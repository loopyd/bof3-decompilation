"""Public naming-conclusion CLI evidence-root contract."""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
from pathlib import Path

import pytest
from harness.naming import cli as naming_audit_cli


@pytest.mark.parametrize(
    "module",
    ["harness.naming.cli", "harness.naming.cli"],
)
def test_module_help_emits_usage(module: str) -> None:
    root = Path(__file__).resolve().parents[4]
    environment = dict(os.environ)
    environment["PYTHONPATH"] = str(root / "tools/python")
    result = subprocess.run(
        [sys.executable, "-m", module, "--help"],
        cwd=root,
        env=environment,
        capture_output=True,
        text=True,
        check=False,
    )
    assert result.returncode == 0
    assert result.stdout.startswith("usage: bin/harness naming")


@pytest.mark.parametrize("escape", ["outside", "symlink"])
def test_init_rejects_output_escape_before_initialize(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, escape: str
) -> None:
    root = tmp_path / "repo"
    root.mkdir()
    outside = tmp_path / "outside"
    if escape == "outside":
        output = outside / "report.json"
    else:
        outside.mkdir()
        (root / "reports").symlink_to(outside, target_is_directory=True)
        output = root / "reports/report.json"

    called = False

    def fake_initialize(*_args):
        nonlocal called
        called = True
        return {}

    monkeypatch.setattr("harness.naming.audit.initialize", fake_initialize)
    args = argparse.Namespace(root=root, target="exe/test", output=output)
    with pytest.raises(ValueError, match="escapes repository root|symlink escape"):
        naming_audit_cli._run_init(args)
    assert called is False
    assert not (outside / "report.json").exists()


def _arguments(*extra: str) -> list[str]:
    return ["conclude", "exe/test", "report.json", "input.json", *extra]


def test_conclude_preserves_one_raw_evidence_root(tmp_path: Path) -> None:
    value = str(tmp_path.resolve())
    args = naming_audit_cli.build_parser().parse_args(
        _arguments("--evidence-root", value)
    )
    assert args.evidence_root == value


@pytest.mark.parametrize(
    "value",
    [
        "relative",
        "./relative",
        "/",
        "//tmp/evidence",
        "/tmp//evidence",
        "/tmp/./evidence",
        "/tmp/../evidence",
        "/tmp/evidence/",
    ],
)
def test_conclude_rejects_noncanonical_evidence_root_on_run(
    value: str, tmp_path: Path
) -> None:
    args = naming_audit_cli.build_parser().parse_args(
        _arguments("--evidence-root", value)
    )
    args.root = Path.cwd()
    with pytest.raises(ValueError, match="one canonical absolute path"):
        naming_audit_cli._run_conclude(args)


def test_conclude_rejects_repeated_evidence_root(tmp_path: Path) -> None:
    with pytest.raises(SystemExit):
        naming_audit_cli.build_parser().parse_args(
            _arguments(
                "--evidence-root",
                str(tmp_path.resolve()),
                "--evidence-root",
                str((tmp_path / "other").resolve()),
            )
        )


def test_conclude_forwards_evidence_root(monkeypatch, tmp_path: Path) -> None:
    evidence_root = tmp_path.resolve()
    captured = {}

    def fake_import(root, target, report, input_path, *, evidence_root=None):
        captured.update(
            root=root,
            target=target,
            report=report,
            input=input_path,
            evidence_root=evidence_root,
        )
        return {"changed": False}

    monkeypatch.setattr("harness.naming.conclusion.import_conclusion", fake_import)
    args = argparse.Namespace(
        root=Path.cwd(),
        target="exe/test",
        report=tmp_path / "report.json",
        input=tmp_path / "input.json",
        evidence_root=str(evidence_root),
    )
    assert naming_audit_cli._run_conclude(args) == 0
    assert captured["evidence_root"] == evidence_root


def test_conclude_omission_preserves_default_mode(monkeypatch, tmp_path: Path) -> None:
    captured = {}

    def fake_import(root, target, report, input_path, *, evidence_root=None):
        captured["evidence_root"] = evidence_root
        return {"changed": False}

    monkeypatch.setattr("harness.naming.conclusion.import_conclusion", fake_import)
    args = argparse.Namespace(
        root=Path.cwd(),
        target="exe/test",
        report=tmp_path / "report.json",
        input=tmp_path / "input.json",
        evidence_root=None,
    )
    assert naming_audit_cli._run_conclude(args) == 0
    assert captured["evidence_root"] is None
