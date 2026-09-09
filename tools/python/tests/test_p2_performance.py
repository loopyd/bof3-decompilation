"""P2 cache, probe-free status, and bounded telemetry contracts."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.analysis import engine
from harness.commands import rz_project
from harness.domain.repository_layout import load_repository_layout
from harness.macros.index import macro_input_rows
from harness.naming.telemetry import RunTelemetry, write_telemetry
from harness.types.inputs import type_input_rows


def test_engine_identity_cache_invalidates_on_executable_generation(
    tmp_path: Path, monkeypatch
) -> None:
    executable = tmp_path / "rizin"
    executable.write_text("one")
    # Test the owning cache directly so no fake repository layout is needed.
    calls: list[Path] = []
    monkeypatch.setattr(engine, "_get_version", lambda path: calls.append(path) or "v")
    monkeypatch.setattr(
        engine,
        "_probe_capabilities",
        lambda path: {"mips32_little_endian": True, "json": True},
    )
    engine._verified_engine.cache_clear()
    stat = executable.stat()
    first = engine._verified_engine(executable, stat.st_mtime_ns, stat.st_size)
    second = engine._verified_engine(executable, stat.st_mtime_ns, stat.st_size)
    executable.write_text("generation-two")
    stat = executable.stat()
    third = engine._verified_engine(executable, stat.st_mtime_ns, stat.st_size)
    assert first is second
    assert third is not first
    assert len(calls) == 2


def test_rz_status_never_probes_engine(monkeypatch, capsys) -> None:
    monkeypatch.setattr(rz_project, "_root", lambda args: Path("/repo"))
    monkeypatch.setattr(
        rz_project,
        "status",
        lambda root, target: {"target": target, "fresh": True, "snapshot": "x"},
    )
    monkeypatch.setattr(
        rz_project,
        "find_engine",
        lambda *args, **kwargs: (_ for _ in ()).throw(AssertionError("probe")),
    )
    args = argparse.Namespace(target="exe/logo", json=True)
    assert rz_project.run_status(args) == 0
    assert json.loads(capsys.readouterr().out)["fresh"] is True


def test_repository_layout_precomputes_canonical_fingerprint_rows() -> None:
    repo_root = Path(__file__).resolve().parents[3]
    layout = load_repository_layout(repo_root)
    for target, manifest in layout.manifests.items():
        assert list(layout.type_input_rows[target]) == type_input_rows(
            repo_root, manifest
        )
        assert list(layout.macro_input_rows[target]) == macro_input_rows(
            repo_root, target, manifest
        )
    assert layout.files.contents.keys() == layout.files.digests.keys()
    assert layout.files.contents.keys() == layout.files.stats.keys()


def test_telemetry_is_bounded_and_reconciles(tmp_path: Path) -> None:
    telemetry = RunTelemetry()
    telemetry.rows_planned = 10
    telemetry.rows_completed = 7
    telemetry.rows_skipped = 3
    telemetry.phase("prepare")
    payload = telemetry.payload("abc")
    path = tmp_path / "telemetry.json"
    write_telemetry(path, payload)
    assert path.stat().st_size <= 4096
    assert abs(payload["wall_seconds"] - payload["accounted_seconds"]) <= 0.01
    assert payload["rows"]["completed"] + payload["rows"]["skipped"] == 10
