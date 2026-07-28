"""Focused tests for target-qualified lift and M2c toolchain delegation."""

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path
from types import SimpleNamespace

import pytest

from harness.commands import lift
from harness.commands.lift import run_m2c
from harness.toolchain.m2c import M2cToolchain


def _target(root: Path) -> None:
    target = root / "config" / "targets" / "exe" / "logo" / "target.toml"
    target.parent.mkdir(parents=True)
    target.write_text(
        "\n".join(
            (
                'schema = "harness.target/v2"',
                'id = "exe/logo"',
                'kind = "executable"',
                'source_dir = "src/exe/logo"',
                'binary = "out/binaries/exe/logo.bin"',
                'splat = "config/targets/exe/logo/splat.yaml"',
                "load_address = 0x801CE000",
            )
        )
        + "\n",
        encoding="utf-8",
    )


def _layout(root: Path) -> SimpleNamespace:
    return SimpleNamespace(root=root, out_dir=root / "out")


def test_target_qualified_lift_resolves_only_its_owner(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _target(tmp_path)
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))

    function, manifest, source = lift.resolve_function("exe/logo@0x801CE758")

    assert function.address == 0x801CE758
    assert manifest.id.value == "exe/logo"
    assert source == tmp_path / "src/exe/logo/func_801CE758.c"


def test_lift_commands_explain_missing_source(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    _target(tmp_path)
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))

    assert lift.main("asm-diff", ["exe/logo@0x801CE758"]) == 2
    assert "bin/m2c exe/logo@0x801CE758 -o" in capsys.readouterr().err


def test_context_keeps_symbols_target_local(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _target(tmp_path)
    symbols = tmp_path / "config" / "targets" / "exe" / "logo"
    symbols.mkdir(parents=True, exist_ok=True)
    (symbols / "symbols.txt").write_text(
        "func_801CE758 = 0x801CE758;\nD_801D0000 = 0x801D0000;\n",
        encoding="utf-8",
    )
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))
    function, manifest, _ = lift.resolve_function("exe/logo@0x801CE758")

    context = lift.render_context(function, manifest)

    assert "extern void func_801CE758();" in context
    assert "extern u8 D_801D0000[];" in context
    assert "other target" not in context


def _m2c_stubs(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    """Set up one isolated target, assembly artifact, and project layout."""
    _target(tmp_path)
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))
    assembly = tmp_path / "out" / "splat" / "exe" / "logo" / "asm" / "func_801CE758.s"
    assembly.parent.mkdir(parents=True)
    assembly.write_text("glabel func_801CE758\n", encoding="utf-8")


def _m2c_args(**overrides: object) -> argparse.Namespace:
    values: dict[str, object] = {
        "function": "exe/logo@0x801CE758",
        "context": [],
        "void": False,
        "out": None,
    }
    values.update(overrides)
    return argparse.Namespace(**values)


def test_run_m2c_delegates_to_owning_toolchain(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _m2c_stubs(tmp_path, monkeypatch)
    executed: list[tuple[Path, list[str], dict[str, object]]] = []

    def fake_execute(self, arguments, **kwargs):
        executed.append((self.root, arguments, kwargs))
        return subprocess.CompletedProcess(
            arguments, 0, stdout="void test(void) { }", stderr=""
        )

    monkeypatch.setattr(M2cToolchain, "execute", fake_execute)

    assert run_m2c(_m2c_args()) == 0
    call_root, call_args, call_kwargs = executed[0]
    assert call_root == tmp_path
    assert call_args[:6] == ["-t", "mipsel-gcc-c", "-f", "func_801CE758", "--globals", "used"]
    assert call_kwargs == {"capture_output": True, "text": True}


def test_run_m2c_preserves_flags_exit_code_and_output(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _m2c_stubs(tmp_path, monkeypatch)
    out_file = tmp_path / "candidate.c"
    executed: list[tuple[Path, list[str]]] = []

    def fake_execute(self, arguments, **kwargs):
        executed.append((self.root, arguments))
        return subprocess.CompletedProcess(
            arguments, 1, stdout="void func_801ce758(void) { }", stderr="syntax error"
        )

    monkeypatch.setattr(M2cToolchain, "execute", fake_execute)

    assert run_m2c(_m2c_args(void=True, out=str(out_file))) == 1
    call_root, call_args = executed[0]
    assert call_root == tmp_path
    assert "--void" in call_args
    assert out_file.read_text(encoding="utf-8") == "void func_801CE758(void) { }"
