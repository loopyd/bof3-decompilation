"""Stable P1 command performance contracts."""

from __future__ import annotations

from pathlib import Path

from harness.analysis.project import status
from harness.commands import analysis_readiness, symbols


def test_readiness_reuses_loaded_manifests(monkeypatch, tmp_path: Path) -> None:
    manifest = object()
    calls: list[object] = []
    monkeypatch.setattr(
        analysis_readiness,
        "load_target_manifests",
        lambda _root: {"exe/test": manifest},
    )
    monkeypatch.setattr(
        analysis_readiness,
        "project_status",
        lambda _root, _target, *, manifest=None: (
            calls.append(manifest) or {"fresh": True}
        ),
    )
    monkeypatch.setattr(analysis_readiness, "_summaries", lambda *_args: [])

    analysis_readiness.readiness(tmp_path, "exe/test", False)

    assert calls == [manifest]


def test_symbols_composes_maps_without_reloading_manifests(
    monkeypatch, tmp_path: Path
) -> None:
    """The global symbols hot loop must pass its already-loaded PsyQ space."""

    source = Path(symbols.__file__).read_text(encoding="utf-8")
    assert "psyq_space=manifest.psyq_space" in source


def test_project_status_accepts_manifest_for_bulk_callers() -> None:
    defaults = status.__kwdefaults__
    assert defaults is not None and "manifest" in defaults
