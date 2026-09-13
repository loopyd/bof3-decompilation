"""Trust-boundary tests for naming evidence roots and worker environments."""

from __future__ import annotations

import os
from pathlib import Path

import pytest
from harness.naming import client as index_client
from harness.naming.namespace import canonical_evidence_root
from harness.naming.namespace import reset_evidence_root
from harness.naming.namespace import set_evidence_root
from harness.naming.runner import build_parser


def test_explicit_root_requires_canonical_spelling(tmp_path: Path) -> None:
    canonical = tmp_path / "safe" / "new"
    token = set_evidence_root(canonical)
    reset_evidence_root(token)
    malformed = (
        "relative",
        "/",
        f"//{canonical.as_posix().lstrip('/')}",
        f"{tmp_path}/safe//new",
        f"{tmp_path}/safe/./new",
        f"{tmp_path}/safe/../safe/new",
        f"{canonical}/",
    )
    for spelling in malformed:
        with pytest.raises(ValueError, match="one canonical absolute path"):
            canonical_evidence_root(spelling)


def test_collection_parser_preserves_raw_root_and_rejects_repeat(
    tmp_path: Path,
) -> None:
    canonical = str(tmp_path / "evidence")
    args = build_parser().parse_args(
        ["exe/test", "report.json", "--evidence-root", canonical]
    )
    assert args.evidence_root == canonical
    with pytest.raises(SystemExit):
        build_parser().parse_args(
            [
                "exe/test",
                "report.json",
                "--evidence-root",
                canonical,
                "--evidence-root",
                canonical,
            ]
        )


def test_explicit_root_rejects_symlink_ancestor_and_final(tmp_path: Path) -> None:
    real = tmp_path / "real"
    real.mkdir()
    link = tmp_path / "link"
    link.symlink_to(real, target_is_directory=True)
    for path in (link, link / "new"):
        with pytest.raises(ValueError, match="symlink"):
            set_evidence_root(path)


def _worker_environment(
    monkeypatch, tmp_path: Path, selected: Path | None
) -> dict[str, str]:
    captured = {}

    def popen(*_args, **kwargs):
        captured.update(kwargs["env"])
        return object()

    monkeypatch.setattr(index_client, "owned_popen", popen)
    monkeypatch.setattr(index_client.IndexWorker, "_send", lambda *_args: None)
    monkeypatch.setenv("BOF3_NAMING_EVIDENCE_ROOT", "/tmp/ambient-untrusted")
    token = set_evidence_root(selected)
    try:
        index_client.IndexWorker(tmp_path, "exe/test", {})
    finally:
        reset_evidence_root(token)
    return captured


def test_default_worker_scrubs_ambient_evidence_root(
    monkeypatch, tmp_path: Path
) -> None:
    assert "BOF3_NAMING_EVIDENCE_ROOT" not in _worker_environment(
        monkeypatch, tmp_path, None
    )


def test_explicit_worker_overrides_ambient_evidence_root(
    monkeypatch, tmp_path: Path
) -> None:
    selected = tmp_path / "evidence"
    assert _worker_environment(monkeypatch, tmp_path, selected)[
        "BOF3_NAMING_EVIDENCE_ROOT"
    ] == os.fspath(selected)
