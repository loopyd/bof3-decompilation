from __future__ import annotations

from pathlib import Path
from types import SimpleNamespace

from harness.commands import lift


def _target(root: Path) -> None:
    target = root / "config" / "targets" / "exe" / "logo.toml"
    target.parent.mkdir(parents=True)
    target.write_text(
        "\n".join(
            (
                'schema = "harness.target/v2"',
                'id = "exe/logo"',
                'kind = "executable"',
                'source_dir = "src/exe/logo"',
                'binary = "out/binaries/exe/logo.bin"',
                'splat = "config/splat/exe/logo.yaml"',
                "load_address = 0x801CE000",
                'profile = "compat/capcom97"',
            )
        )
        + "\n",
        encoding="utf-8",
    )


def test_target_qualified_lift_resolves_only_its_owner(
    tmp_path: Path, monkeypatch
) -> None:
    _target(tmp_path)
    monkeypatch.setattr(
        lift,
        "repo_layout",
        lambda: SimpleNamespace(root=tmp_path, out_dir=tmp_path / "out"),
    )

    function, manifest, source = lift.resolve_function("exe/logo@0x801CE758")

    assert function.address == 0x801CE758
    assert manifest.id.value == "exe/logo"
    assert source == tmp_path / "src/exe/logo/func_801ce758.c"


def test_context_keeps_symbols_target_local(tmp_path: Path, monkeypatch) -> None:
    _target(tmp_path)
    symbols = tmp_path / "config" / "symbols" / "exe"
    symbols.mkdir(parents=True)
    (symbols / "logo.txt").write_text(
        "func_801CE758 = 0x801CE758;\nD_801D0000 = 0x801D0000;\n",
        encoding="utf-8",
    )
    monkeypatch.setattr(
        lift,
        "repo_layout",
        lambda: SimpleNamespace(root=tmp_path, out_dir=tmp_path / "out"),
    )
    function, manifest, _ = lift.resolve_function("exe/logo@0x801CE758")

    context = lift.render_context(function, manifest)

    assert "extern void func_801CE758();" in context
    assert "extern u8 D_801D0000[];" in context
    assert "other target" not in context
