"""Focused tests for bin/splat delegation through SplatToolchain."""

from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

from harness.commands import splat
from harness.toolchain.splat import SplatToolchain


def _setup_target(tmp_path: Path) -> Path:
    """Create a minimal exe/logo target and venv so the command can parse."""
    root = tmp_path
    target_dir = root / "config" / "targets" / "exe" / "logo"
    target_dir.mkdir(parents=True)
    (target_dir / "manifest.toml").write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/logo"\n'
        'kind = "executable"\n'
        'source_dir = "src/exe/logo"\n'
        'binary = "out/binaries/exe/logo.bin"\n'
        'splat = "config/targets/exe/logo/splat.yaml"\n'
        "load_address = 0x80100000\n",
        encoding="utf-8",
    )
    (root / "out/binaries/exe/logo.bin").parent.mkdir(parents=True)
    (root / "out/binaries/exe/logo.bin").write_bytes(b"\x00" * 0x200000)
    # Create .venv/bin/splat so SplatToolchain.executable resolves
    splat_exe = root / ".venv" / "bin" / "splat"
    splat_exe.parent.mkdir(parents=True)
    splat_exe.touch()
    return root


def test_missing_executable_fails(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    root = _setup_target(tmp_path)
    root.joinpath(".venv/bin/splat").unlink()
    with pytest.raises(FileNotFoundError, match="missing Splat executable"):
        splat.run(
            splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
        )


def test_non_verbose_captures_output(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    root = _setup_target(tmp_path)
    calls: list[tuple] = []

    def fake_execute(self, args, **kwargs):
        calls.append((args, kwargs))
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    rc = splat.run(
        splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
    )
    assert rc == 0
    assert len(calls) == 1
    args_passed, kwargs = calls[0]
    assert kwargs.get("capture_output") is True
    assert kwargs.get("text") is True


def test_verbose_streams_output(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    root = _setup_target(tmp_path)
    calls: list[tuple] = []

    def fake_execute(self, args, **kwargs):
        calls.append((args, kwargs))
        return subprocess.CompletedProcess(args, 0)

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    rc = splat.run(
        splat.build_parser().parse_args(
            ["--root", str(root), "exe/logo", "--verbose"]
        )
    )
    assert rc == 0
    assert len(calls) == 1
    _, kwargs = calls[0]
    assert kwargs.get("capture_output") is False
    assert kwargs.get("text") is False


def test_non_verbose_failure_prints_to_stderr(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys
) -> None:
    root = _setup_target(tmp_path)

    def fake_execute(self, args, **kwargs):
        return subprocess.CompletedProcess(
            args, 7, stdout="stdout-msg", stderr="stderr-msg"
        )

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    rc = splat.run(
        splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
    )
    assert rc == 7
    captured = capsys.readouterr()
    assert captured.out == ""
    assert "stdout-msg" in captured.err
    assert "stderr-msg" in captured.err


def test_non_verbose_success_prints_ok(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys
) -> None:
    root = _setup_target(tmp_path)

    def fake_execute(self, args, **kwargs):
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    rc = splat.run(
        splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
    )
    assert rc == 0
    captured = capsys.readouterr()
    assert "splat OK" in captured.out
    assert captured.err == ""


def test_splat_path_in_arguments(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    root = _setup_target(tmp_path)
    calls: list[tuple] = []

    def fake_execute(self, args, **kwargs):
        calls.append((args, kwargs))
        return subprocess.CompletedProcess(args, 0, stdout="", stderr="")

    monkeypatch.setattr(SplatToolchain, "execute", fake_execute)
    splat.run(
        splat.build_parser().parse_args(["--root", str(root), "exe/logo"])
    )
    assert len(calls) == 1
    args_passed = calls[0][0]
    assert "split" in args_passed
    assert "--make-full-disasm-for-code" in args_passed
    assert any("splat.yaml" in a for a in args_passed)
