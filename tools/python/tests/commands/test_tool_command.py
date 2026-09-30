from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

from harness.commands import tool


class FakeToolchain:
    label = "fake"

    def __init__(self, executable: Path, python: Path | None = None) -> None:
        self.executable = executable
        self.python = python
        self.calls: list[tuple[str, ...]] = []

    def execute(self, arguments: list[str]) -> subprocess.CompletedProcess[str]:
        self.calls.append(tuple(arguments))
        return subprocess.CompletedProcess(arguments, 7)


def test_tool_command_delegates_to_owned_invocation(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    executable = tmp_path / "tool"
    executable.touch()
    fake = FakeToolchain(executable)
    monkeypatch.setattr(tool, "managed_toolchain", lambda layout, name: fake)

    assert tool.main(["--root", str(tmp_path), "rizin", "--", "-V"]) == 7
    assert fake.calls == [("-V",)]


def test_tool_command_requires_project_python_when_owned(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    executable = tmp_path / "tool"
    executable.touch()
    fake = FakeToolchain(executable, tmp_path / ".venv/bin/python")
    monkeypatch.setattr(tool, "managed_toolchain", lambda layout, name: fake)

    with pytest.raises(FileNotFoundError, match="missing project Python environment"):
        tool.run(tool.build_parser().parse_args(["--root", str(tmp_path), "maspsx"]))
