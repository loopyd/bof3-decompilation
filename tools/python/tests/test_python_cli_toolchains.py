from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

from harness.toolchain.asm_differ import AsmDifferToolchain
from harness.toolchain.permuter import DecompPermuterToolchain
from harness.toolchain.splat import SplatToolchain
from harness.toolchain.spimdisasm import SpimdisasmToolchain


@pytest.mark.parametrize(
    ("toolchain_type", "submodule", "install_target", "executable"),
    (
        (SplatToolchain, "third_party/splat", "third_party/splat[mips]", "splat"),
        (SpimdisasmToolchain, "third_party/spimdisasm", "third_party/spimdisasm", "spimdisasm"),
    ),
)
def test_python_submodule_toolchain_installs_and_verifies(
    tmp_path: Path,
    monkeypatch: pytest.MonkeyPatch,
    toolchain_type: type,
    submodule: str,
    install_target: str,
    executable: str,
) -> None:
    toolchain = toolchain_type(tmp_path)
    calls: list[list[str]] = []

    monkeypatch.setattr(
        subprocess,
        "run",
        lambda command, **kwargs: calls.append(command) or subprocess.CompletedProcess(command, 0),
    )
    toolchain.python.parent.mkdir(parents=True)
    toolchain.python.touch()
    toolchain.executable.touch()

    assert toolchain.install() == toolchain.label
    assert toolchain.verify() == toolchain.label
    assert calls == [
        ["git", "submodule", "update", "--init", submodule],
        ["uv", "pip", "install", "--python", str(tmp_path / ".venv/bin/python"), str(tmp_path / install_target)],
        [str(tmp_path / ".venv/bin" / executable), "--version"],
    ]


def test_decomp_permuter_toolchain_installs_toml(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    toolchain = DecompPermuterToolchain(tmp_path)
    calls: list[list[str]] = []

    monkeypatch.setattr(
        subprocess,
        "run",
        lambda command, **kwargs: calls.append(command) or subprocess.CompletedProcess(command, 0),
    )
    toolchain.python.parent.mkdir(parents=True)
    toolchain.python.touch()
    toolchain.executable.parent.mkdir(parents=True)
    toolchain.executable.touch()

    assert toolchain.install() == "decomp-permuter"
    assert toolchain.verify() == "decomp-permuter"
    assert calls == [
        ["git", "submodule", "update", "--init", "third_party/decomp-permuter"],
        ["uv", "pip", "install", "--python", str(toolchain.python), "toml"],
        [str(toolchain.python), str(toolchain.executable), "--help"],
    ]


def test_asm_differ_toolchain_installs_pinned_submodule(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    toolchain = AsmDifferToolchain(tmp_path)
    calls: list[list[str]] = []

    monkeypatch.setattr(
        subprocess,
        "run",
        lambda command, **kwargs: calls.append(command) or subprocess.CompletedProcess(command, 0),
    )
    toolchain.python.parent.mkdir(parents=True)
    toolchain.python.touch()
    toolchain.source.mkdir(parents=True)
    toolchain.executable.touch()

    assert toolchain.install() == "asm-differ"
    assert toolchain.verify() == "asm-differ"
    assert calls == [
        ["git", "submodule", "update", "--init", "third_party/asm-differ"],
        ["uv", "pip", "install", "--python", str(toolchain.python), str(toolchain.source)],
        [str(toolchain.executable), "--help"],
    ]
