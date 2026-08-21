"""Focused tests for target-qualified lift and M2c toolchain delegation."""

from __future__ import annotations

import argparse
import subprocess
from pathlib import Path
from types import SimpleNamespace

import pytest

from harness.commands import lift
from harness.commands import _common
from harness.commands import _lift_m2c
from harness.commands._lift_m2c import run_m2c
from harness.match._asm_diff_payload import AsmDiffRequest
from harness.toolchain import m2c as m2c_toolchain
from harness.toolchain.m2c import M2cToolchain, render_context


def _target(
    root: Path, *, sources: tuple[str, ...] = ("src/exe/logo/initSelectionState.c",)
) -> None:
    base = root / "include/base/types.h"
    base.parent.mkdir(parents=True, exist_ok=True)
    base.write_text(
        "typedef unsigned char u8;\ntypedef signed int s32;\n", encoding="utf-8"
    )
    target = root / "config" / "targets" / "exe" / "logo" / "target.toml"
    target.parent.mkdir(parents=True)
    lines = [
        'schema = "harness.target/v2"',
        'id = "exe/logo"',
        'kind = "executable"',
        'source_dir = "src/exe/logo"',
        'binary = "out/binaries/exe/logo.bin"',
        'splat = "config/targets/exe/logo/splat.yaml"',
        "load_address = 0x801CE000",
        "sources = [" + ", ".join(f'"{s}"' for s in sources) + "]",
    ]
    target.write_text("\n".join(lines) + "\n", encoding="utf-8")
    for claimed in sources:
        source = root / claimed
        source.parent.mkdir(parents=True, exist_ok=True)
        if not source.exists():
            source.write_text("void placeholder(void) {}\n", encoding="utf-8")


def _layout(root: Path) -> SimpleNamespace:
    return SimpleNamespace(root=root, out_dir=root / "out")


def test_target_qualified_lift_resolves_only_its_owner(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _target(tmp_path, sources=("src/exe/logo/initSelectionState.c",))
    source = tmp_path / "src/exe/logo/initSelectionState.c"
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text(
        "/* @source 0x801CE758 @behavior stages selection */\n", encoding="utf-8"
    )
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(_common, "repo_layout", lambda: _layout(tmp_path))

    function, manifest, resolved = lift.resolve_function_selector("exe/logo@0x801CE758")

    assert function.address == 0x801CE758
    assert manifest.id.value == "exe/logo"
    assert resolved == source


def test_lift_commands_explain_missing_source(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    _target(tmp_path)
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(_common, "repo_layout", lambda: _layout(tmp_path))

    assert lift.main("asm-diff", ["exe/logo@0x801CE758"]) == 2
    assert "lifted source does not exist for exe/logo@0x801CE758" in (
        capsys.readouterr().err
    )


def test_context_uses_registry_and_bootstrap_fallback_is_explicit(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _target(tmp_path)
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(_common, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(m2c_toolchain, "repo_layout", lambda: _layout(tmp_path))
    function, manifest, _ = lift.resolve_function_selector("exe/logo@0x801CE758")

    context = render_context(function, manifest)

    assert (
        "WARNING: reverse type index unavailable during explicit bootstrap" in context
    )
    assert "typedef unsigned char u8;" in context

    placeholder = tmp_path / "out/index/reverse.sqlite"
    placeholder.parent.mkdir(parents=True)
    placeholder.write_bytes(b"bootstrap placeholder")
    context = render_context(function, manifest)
    assert (
        "WARNING: reverse type index unavailable during explicit bootstrap" in context
    )


def test_context_keeps_symbols_target_local(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _target(tmp_path, sources=("src/exe/logo/initSelectionState.c",))
    symbols = tmp_path / "config" / "targets" / "exe" / "logo"
    symbols.mkdir(parents=True, exist_ok=True)
    (symbols / "symbols.txt").write_text(
        "func_801CE758 = 0x801CE758;\nD_801D0000 = 0x801D0000;\n",
        encoding="utf-8",
    )
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(_common, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(m2c_toolchain, "repo_layout", lambda: _layout(tmp_path))
    function, manifest, _ = lift.resolve_function_selector("exe/logo@0x801CE758")

    context = render_context(function, manifest)

    assert "extern void func_801CE758();" in context
    assert "extern u8 D_801D0000[];" in context
    assert "other target" not in context


def test_context_includes_nested_private_header(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """m2ctx records the resolved source's claimed private header."""
    _target(tmp_path, sources=("src/exe/logo/runtime/initSelectionState.c",))
    manifest = tmp_path / "config/targets/exe/logo/target.toml"
    manifest.write_text(
        manifest.read_text(encoding="utf-8")
        + 'headers = ["src/exe/logo/runtime/internal.h"]\n',
        encoding="utf-8",
    )
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(_common, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(m2c_toolchain, "repo_layout", lambda: _layout(tmp_path))
    source_dir = tmp_path / "src" / "exe" / "logo"
    nested = source_dir / "runtime" / "initSelectionState.c"
    nested.parent.mkdir(parents=True, exist_ok=True)
    nested.write_text("/* @source 0x801CE758 @behavior x */\n", encoding="utf-8")
    (nested.parent / "internal.h").write_text(
        "/* runtime-private */\n", encoding="utf-8"
    )
    function, manifest, _ = lift.resolve_function_selector("exe/logo@0x801CE758")

    context = render_context(function, manifest)

    assert "owning declaration source: src/exe/logo/runtime/internal.h" in context


def test_context_consumes_manifest_header_paths(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    """m2ctx records the target's claimed private headers even when they
    live outside ``source_dir`` (semantic ``include/bof3/`` placement)."""
    _target(tmp_path, sources=("src/bof3/ui/selectUiMode14.c",))
    manifest = tmp_path / "config/targets/exe/logo/target.toml"
    manifest.write_text(
        manifest.read_text(encoding="utf-8")
        + 'headers = ["include/bof3/ui/commu00_internal.h"]\n',
        encoding="utf-8",
    )
    source = tmp_path / "src/bof3/ui/selectUiMode14.c"
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text("/* @source 0x801CE758 @behavior x */\n", encoding="utf-8")
    header = tmp_path / "include/bof3/ui/commu00_internal.h"
    header.parent.mkdir(parents=True)
    header.write_text("/* claimed private header */\n", encoding="utf-8")
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(_common, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(m2c_toolchain, "repo_layout", lambda: _layout(tmp_path))
    function, manifest, _ = lift.resolve_function_selector("exe/logo@0x801CE758")

    context = render_context(function, manifest)

    assert "owning declaration source: include/bof3/ui/commu00_internal.h" in context


def _m2c_stubs(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    """Set up one isolated target, assembly artifact, and project layout."""
    _target(tmp_path, sources=("src/exe/logo/func_801CE758.c",))
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(_common, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(_lift_m2c, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(m2c_toolchain, "repo_layout", lambda: _layout(tmp_path))
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
    assert call_args[:6] == [
        "-t",
        "mipsel-gcc-c",
        "-f",
        "func_801CE758",
        "--globals",
        "used",
    ]
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


def _map_infra(root: Path) -> None:
    _target(root)
    (root / "config/targets/shared").mkdir(parents=True)
    (root / "config/targets/shared/symbols.txt").write_text("")
    (root / "config/sdk").mkdir(parents=True)
    (root / "config/sdk/psyq-slus.txt").write_text("")
    symbols = root / "config/targets/exe/logo/symbols.txt"
    symbols.parent.mkdir(parents=True, exist_ok=True)
    symbols.write_text("func_801CE758 = 0x801CE758;\nD_801D0000 = 0x801D0000;\n")


def test_run_match_passes_bindings_without_rewriting_identical_file(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    _map_infra(tmp_path)
    monkeypatch.setattr(lift, "repo_layout", lambda: _layout(tmp_path))
    monkeypatch.setattr(_common, "repo_layout", lambda: _layout(tmp_path))
    source = tmp_path / "src/exe/logo/func_801CE758.c"
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text(
        "// @source 0x801CE758\n// @behavior stages selection\n", encoding="utf-8"
    )
    # claim the lift source so registry resolution finds it
    manifest_path = tmp_path / "config/targets/exe/logo/target.toml"
    manifest_path.write_text(
        manifest_path.read_text(encoding="utf-8").replace(
            'sources = ["src/exe/logo/initSelectionState.c"]',
            'sources = ["src/exe/logo/func_801CE758.c"]',
        ),
        encoding="utf-8",
    )
    bindings = tmp_path / "out/bindings/exe/logo/symbols.c"
    bindings.parent.mkdir(parents=True, exist_ok=True)
    bindings.write_text(
        lift.weak_bindings_c(lift.load_target_symbols(tmp_path, "exe/logo"))
    )
    before = bindings.stat().st_mtime_ns
    captured: list[AsmDiffRequest] = []
    monkeypatch.setattr(
        lift,
        "run_asm_diff_one",
        lambda request: captured.append(request) or {"byte_match": True},
    )

    function, manifest, _ = lift.resolve_function_selector("exe/logo@0x801CE758")
    lift._run_match(function, manifest, source, diagnostics=False)

    assert bindings.stat().st_mtime_ns == before
    assert captured[0].canonical_bindings == {
        "func_801CE758": 0x801CE758,
        "D_801D0000": 0x801D0000,
    }
