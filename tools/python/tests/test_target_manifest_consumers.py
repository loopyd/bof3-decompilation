"""Focused tests for the four harness callers migrated to lookup_target_manifest.

Covers canonical-target propagation and the retained unknown-target wording
for analyzer.write_target_snapshot, rizin_project.prepare_target,
commands.build.run, and commands.splat.run.
"""

from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

from harness import analyzer
from harness.commands import build as build_cmd
from harness.commands import splat as splat_cmd
from harness.io import repo_layout
from harness.rizin_project import prepare_target
from harness.snapshot import snapshot_path
from harness.toolchain.splat import SplatToolchain

CANONICAL = "emi/battle/batl_end/00"
SHIPPED = "BIN/BATTLE/BATL_END.EMI#0"


def test_snapshot_path_is_flat_injective_and_rejects_traversal(
    tmp_path: Path,
) -> None:
    assert snapshot_path(tmp_path, "exe/slus_004_22") == (
        tmp_path / "out/reverse/snapshots/exe--slus_004_22.json"
    )
    assert snapshot_path(tmp_path, "emi/a--b/00").name == "emi--a%2D%2Db--00.json"
    assert snapshot_path(tmp_path, "emi/a/b--00").name != snapshot_path(
        tmp_path, "emi/a--b/00"
    ).name
    for target_id in ("", "../exe/logo", "exe//logo", "exe/./logo"):
        with pytest.raises(ValueError, match="invalid target ID"):
            snapshot_path(tmp_path, target_id)


def _write_target(
    root: Path,
    target_id: str,
    *,
    binary: bool = True,
    sources: tuple[str, ...] = (),
) -> None:
    """Create a minimal schema-v2 target without game media."""
    target_dir = root / "config" / "targets" / target_id
    target_dir.mkdir(parents=True)
    (target_dir / "target.toml").write_text(
        "schema = 'harness.target/v2'\n"
        f"id = '{target_id}'\n"
        "kind = 'emi'\n"
        f"source_dir = 'src/{target_id}'\n"
        f"binary = 'out/binaries/{target_id}.bin'\n"
        f"splat = 'config/targets/{target_id}/splat.yaml'\n"
        "load_address = 0x80100000\n",
        encoding="utf-8",
    )
    if binary:
        binary_path = root / "out/binaries" / f"{target_id}.bin"
        binary_path.parent.mkdir(parents=True)
        binary_path.write_bytes(b"\0" * 32)
    (target_dir / "splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n", encoding="utf-8"
    )
    (target_dir / "symbols.txt").write_text(
        "func_80100000 = 0x80100000;\n", encoding="utf-8"
    )
    for source in sources:
        source_path = root / "src" / target_id / source
        source_path.parent.mkdir(parents=True)
        source_path.write_text("void func_80100000(void) {}\n", encoding="utf-8")
    splat_exe = root / ".venv" / "bin" / "splat"
    splat_exe.parent.mkdir(parents=True)
    splat_exe.touch()


# --- analyzer.write_target_snapshot ------------------------------------------


def test_analyzer_unknown_target_keeps_normalized_message(tmp_path: Path) -> None:
    with pytest.raises(ValueError) as exc:
        analyzer.write_target_snapshot(tmp_path, "BIN/FOO/BAR.EMI#3")
    assert str(exc.value) == "unknown target: emi/foo/bar/03"


def test_analyzer_missing_binary_still_fails(tmp_path: Path) -> None:
    _write_target(tmp_path, CANONICAL, binary=False)
    with pytest.raises(FileNotFoundError, match="target binary not found"):
        analyzer.write_target_snapshot(tmp_path, SHIPPED)


def test_analyzer_passes_canonical_target_after_lookup(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _write_target(tmp_path, CANONICAL)
    calls: list[tuple] = []
    monkeypatch.setattr(
        "harness.rizin_project.analyze_project",
        lambda root, target, timeout=120: calls.append((root, target, timeout)),
    )
    monkeypatch.setattr(
        "harness.analyzer.snapshot_path",
        lambda root, target: root / "out/reverse" / target / "snapshot.json",
    )
    output = analyzer.write_target_snapshot(tmp_path, SHIPPED)
    assert calls == [(tmp_path, CANONICAL, 120)]
    assert output == tmp_path / "out/reverse" / CANONICAL / "snapshot.json"


# --- rizin_project.prepare_target --------------------------------------------


def test_rizin_unknown_target_keeps_raw_message(tmp_path: Path) -> None:
    with pytest.raises(ValueError) as exc:
        prepare_target(tmp_path, "SLUS_004.22")
    assert str(exc.value) == "unknown target: SLUS_004.22"


def test_rizin_canonical_identity_propagated(tmp_path: Path) -> None:
    _write_target(tmp_path, CANONICAL)
    project = prepare_target(tmp_path, SHIPPED)
    assert project.target == CANONICAL
    assert project.snapshot == snapshot_path(tmp_path, CANONICAL)
    assert "afn func_80100000 0x80100000" in project.replay


# --- commands.build.run -------------------------------------------------------


def test_build_unknown_target_keeps_normalized_message(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    monkeypatch.setattr(build_cmd, "repo_layout", lambda: repo_layout(tmp_path))
    with pytest.raises(ValueError) as exc:
        build_cmd.run(build_cmd.build_parser().parse_args(["BIN/FOO/BAR.EMI#3"]))
    assert str(exc.value) == "unknown target: emi/foo/bar/03"


def test_build_no_authored_sources_uses_canonical_id(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys
) -> None:
    _write_target(tmp_path, CANONICAL)
    monkeypatch.setattr(build_cmd, "repo_layout", lambda: repo_layout(tmp_path))
    rc = build_cmd.run(build_cmd.build_parser().parse_args([SHIPPED]))
    assert rc == 0
    captured = capsys.readouterr()
    assert captured.out.strip() == f"{CANONICAL}: no authored sources"


def test_build_passes_canonical_source_dir_to_cmake(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _write_target(tmp_path, CANONICAL, sources=("func_80100000.c",))
    monkeypatch.setattr(build_cmd, "repo_layout", lambda: repo_layout(tmp_path))
    recorded: dict[str, object] = {}

    def fake_cmake_target(directory: str) -> str:
        recorded["dir"] = directory
        return "target_x"

    monkeypatch.setattr(build_cmd, "cmake_target_for_directory", fake_cmake_target)
    def fake_build(root: Path, target: str) -> subprocess.CompletedProcess[str]:
        recorded["built"] = (root, target)
        return subprocess.CompletedProcess([], 0, "", "")

    monkeypatch.setattr(build_cmd, "build", fake_build)
    rc = build_cmd.run(build_cmd.build_parser().parse_args([SHIPPED]))
    assert rc == 0
    assert recorded["dir"] == f"src/{CANONICAL}"
    assert recorded["built"] == (tmp_path, "target_x")


# --- commands.splat.run -------------------------------------------------------


def test_splat_unknown_target_keeps_raw_message(tmp_path: Path) -> None:
    with pytest.raises(ValueError) as exc:
        splat_cmd.run(
            splat_cmd.build_parser().parse_args(["--root", str(tmp_path), "SLUS_004.22"])
        )
    assert str(exc.value) == "unknown target: SLUS_004.22"


def test_splat_success_line_uses_canonical_id(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys
) -> None:
    _write_target(tmp_path, CANONICAL)
    calls: list[list[str]] = []

    def fake_execute(self, args, **kwargs):
        calls.append(args)
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    rc = splat_cmd.run(
        splat_cmd.build_parser().parse_args(["--root", str(tmp_path), SHIPPED])
    )
    assert rc == 0
    captured = capsys.readouterr()
    assert captured.out.strip() == f"{CANONICAL}: splat OK"
    assert captured.err == ""
    assert any("splat.yaml" in str(arg) for arg in calls[0])
