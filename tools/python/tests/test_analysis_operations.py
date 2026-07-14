from __future__ import annotations

import json
from pathlib import Path
import subprocess

import pytest

from harness.analysis import operations
from harness.domain import TargetManifest
from harness.domain.ids import TargetId


def _manifest() -> TargetManifest:
    return TargetManifest(
        id=TargetId("emi/etc/game/01", "BIN/ETC/GAME.EMI#1"),
        disc_id="BIN/ETC/GAME.EMI#1",
        kind="emi",
        source_dir="src/emi/etc/game/01",
        binary="out/binaries/emi/etc/game/01.bin",
        splat="config/splat/emi/etc/game/01.yaml",
        load_address=0x801D0C00,
        profile="native/capcom97",
    )


def _write_inputs(root: Path) -> TargetManifest:
    manifest = _manifest()
    for relative, content in (
        (manifest.binary, b"binary"),
        (manifest.splat, b"subsegments:\n  - [0x4, c, func_801d0c04]\n"),
        ("config/analysis/shared/bof3_objects.h", b"typedef int Example;\n"),
        ("config/analysis/shared/hwregs.r2", b"fs functions\n"),
    ):
        path = root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)
    return manifest


def test_doctor_probes_engine_capabilities_inside_engine(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    monkeypatch.setattr(
        operations.shutil,
        "which",
        lambda name: "/usr/bin/r2" if name == "r2" else None,
    )

    def fake_run(arguments: list[str], **_: object) -> subprocess.CompletedProcess[str]:
        if arguments == ["/usr/bin/r2", "-v"]:
            output = "radare2 6.1.4\n"
        elif "e asm.arch=mips" in arguments:
            output = "mips\n32\nfalse\n"
        elif "aflj" in arguments:
            output = '[]\n{"types": []}\n'
        elif "P?" in arguments:
            output = "Usage: P Project management\n"
        elif "pdg?" in arguments:
            output = "Native Ghidra decompiler plugin\n"
        else:
            raise AssertionError(arguments)
        return subprocess.CompletedProcess(arguments, 0, output, "")

    monkeypatch.setattr(operations.subprocess, "run", fake_run)

    result = operations.doctor()

    assert result["preferred"] == "r2"
    assert result["tools"]["r2"]["version"] == "radare2 6.1.4"
    assert result["tools"]["r2"]["capabilities"] == {
        "mips32_little_endian": True,
        "json": True,
        "projects": True,
        "decompiler": True,
    }


def test_initialize_records_inputs_and_verifies_reopen(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    manifest = _write_inputs(tmp_path)
    calls: list[tuple[list[str], str | None]] = []
    monkeypatch.setattr(operations, "_engine", lambda _: ("r2", Path("/fake/r2")))
    monkeypatch.setattr(operations, "_target", lambda *_: manifest)

    def fake_run(
        _executable: Path,
        _manifest: TargetManifest,
        _root: Path,
        commands: list[str],
        *,
        project_dir: Path,
        project: str | None = None,
        timeout: int = 120,
    ) -> str:
        del project_dir, timeout
        calls.append((commands, project))
        if commands == ["fs *;fj"]:
            sentinel = operations._sentinel(
                operations._analysis_inputs(tmp_path, manifest)
            )
            return json.dumps([{"name": sentinel, "offset": 1}])
        return ""

    monkeypatch.setattr(operations, "_run", fake_run)

    result = operations.initialize_project(tmp_path, manifest.id.value, "r2")

    state_path = tmp_path / result["project"] / "state.json"
    state = json.loads(state_path.read_text())
    assert state["inputs"]["binary_sha256"]
    assert state["inputs"]["types_sha256"]
    assert state["inputs"]["splat_sha256"]
    assert state["reviewed_function_starts"] == 1
    assert calls[0][0][:2] == ["af @ 0x801d0c04", "aar 0x2 @ 0x801d0c04"]
    assert "aaa" not in calls[0][0]
    assert "aar" not in calls[0][0]
    assert calls[1] == (["fs *;fj"], "emi__etc__game__01")
    assert result["verified_reopen"] is True


def test_rizin_project_reference_stays_under_generated_project_directory(
    tmp_path: Path,
) -> None:
    manifest = _manifest()
    project_dir = tmp_path / "out/analysis/projects/rizin/emi__etc__game__01"

    reference = operations._project_reference("rizin", project_dir, manifest)

    assert reference == str(project_dir / "emi__etc__game__01.rzdb")


def test_query_reopens_verified_project_without_reanalysis(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    manifest = _write_inputs(tmp_path)
    engine = "r2"
    executable = Path("/fake/r2")
    project_dir, _ = operations._paths(tmp_path, engine, manifest)
    project_dir.mkdir(parents=True)
    inputs = operations._analysis_inputs(tmp_path, manifest)
    sentinel = operations._sentinel(inputs)
    state = {
        "schema": operations._PROJECT_STATE_SCHEMA,
        "engine": engine,
        "engine_path": str(executable),
        "target": manifest.id.value,
        "project": "emi__etc__game__01",
        "sentinel": sentinel,
        "inputs": inputs,
    }
    (project_dir / "state.json").write_text(json.dumps(state))
    calls: list[tuple[list[str], str | None]] = []
    monkeypatch.setattr(operations, "_engine", lambda _: (engine, executable))
    monkeypatch.setattr(operations, "_target", lambda *_: manifest)

    def fake_run(
        _executable: Path,
        _manifest: TargetManifest,
        _root: Path,
        commands: list[str],
        *,
        project_dir: Path,
        project: str | None = None,
        timeout: int = 120,
    ) -> str:
        del project_dir, timeout
        calls.append((commands, project))
        if commands == ["fs *;fj"]:
            return json.dumps([{"name": sentinel, "offset": 1}])
        return '[{"offset": 2, "name": "second"}, {"offset": 1, "name": "first"}]'

    monkeypatch.setattr(operations, "_run", fake_run)

    result = operations.query_project(tmp_path, manifest.id.value, "functions", engine)

    assert [row["name"] for row in result] == ["first", "second"]
    assert calls == [
        (["fs *;fj"], "emi__etc__game__01"),
        (["aflj"], "emi__etc__game__01"),
    ]


def test_export_writes_full_evidence_but_returns_compact_summary(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    manifest = _manifest()
    monkeypatch.setattr(operations, "_engine", lambda _: ("rizin", Path("/fake/rizin")))
    monkeypatch.setattr(operations, "_target", lambda *_: manifest)

    rows = {
        "functions": [{"offset": 1}],
        "strings": [{"string": "\u00ae\u00af\u00b0\u00b1"}, {"string": "menu"}],
        "xrefs": [{"from": 1, "to": 2}],
    }
    monkeypatch.setattr(
        operations,
        "query_project",
        lambda _root, _target, query, _engine: rows[query],
    )

    result = operations.export_project(tmp_path, manifest.id.value, "rizin")

    assert result == {
        "schema": "bof3.analysis/v1",
        "engine": "rizin",
        "target": manifest.id.value,
        "output": "out/analysis/exports/rizin/emi__etc__game__01/analysis.json",
        "counts": {
            "functions": 1,
            "strings": 2,
            "xrefs": 1,
            "string_classifications": {
                "sequential_table": 1,
                "text_candidate": 1,
            },
        },
    }
    artifact = json.loads((tmp_path / result["output"]).read_text())
    assert artifact["strings"] == rows["strings"]
    assert artifact["string_classifications"] == [
        {"index": 0, "classification": "sequential_table"},
        {"index": 1, "classification": "text_candidate"},
    ]


def test_analysis_paths_isolate_engine_specific_state(tmp_path: Path) -> None:
    manifest = _manifest()

    rizin_paths = operations._paths(tmp_path, "rizin", manifest)
    radare2_paths = operations._paths(tmp_path, "r2", manifest)

    assert rizin_paths != radare2_paths
    assert rizin_paths[1] == (
        tmp_path / "out/analysis/exports/rizin/emi__etc__game__01"
    )
    assert radare2_paths[1] == (tmp_path / "out/analysis/exports/r2/emi__etc__game__01")


@pytest.mark.parametrize(
    ("value", "classification"),
    [
        ("Load Game", "text_candidate"),
        ("\x00\x02", "control_bytes"),
        ("\xff\xff", "repeated_fill"),
        ("\xae\xaf\xb0\xb1", "sequential_table"),
        ("\xae\xaf", "data_pattern"),
        ("+-", "data_pattern"),
        ("ok", "data_pattern"),
        (None, "data_pattern"),
    ],
)
def test_classifies_analyzer_string_guesses_conservatively(
    value: object, classification: str
) -> None:
    assert operations._classify_analyzer_string(value) == classification


def test_query_rejects_stale_inputs(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    manifest = _write_inputs(tmp_path)
    project_dir, _ = operations._paths(tmp_path, "r2", manifest)
    project_dir.mkdir(parents=True)
    state = {
        "schema": operations._PROJECT_STATE_SCHEMA,
        "engine": "r2",
        "engine_path": "/fake/r2",
        "target": manifest.id.value,
        "project": "emi__etc__game__01",
        "sentinel": "harness.sentinel_old",
        "inputs": operations._analysis_inputs(tmp_path, manifest),
    }
    (project_dir / "state.json").write_text(json.dumps(state))
    (tmp_path / manifest.binary).write_bytes(b"changed")
    monkeypatch.setattr(operations, "_engine", lambda _: ("r2", Path("/fake/r2")))
    monkeypatch.setattr(operations, "_target", lambda *_: manifest)

    with pytest.raises(RuntimeError, match="project is stale"):
        operations.query_project(tmp_path, manifest.id.value, "functions", "r2")
