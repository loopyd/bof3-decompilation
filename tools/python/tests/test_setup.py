"""Focused tests: setup toolchain task uses shared managed_toolchains factory."""

from __future__ import annotations

from pathlib import Path
from types import SimpleNamespace

from harness.io import RepoLayout
from harness.commands import setup


def test_setup_toolchain_uses_managed_toolchains(tmp_path: Path, monkeypatch) -> None:
    """Setup _toolchain task delegates to managed_toolchains()."""
    called_with: list[object] = []
    fake_toolchains = tuple()

    def fake_managed(root, layout):
        called_with.append(root)
        called_with.append(layout)
        return fake_toolchains

    monkeypatch.setattr(setup, "managed_toolchains", fake_managed)

    layout = SimpleNamespace(root=tmp_path)
    state = setup.SetupState(root=tmp_path, layout=layout, args=SimpleNamespace(force=False))
    result = setup.TASKS[2].run(state)

    assert result == "PSn00b, GCC 2.7.2, maspsx, Rizin, m2c, asm-differ, decomp-permuter, splat, spimdisasm"
    assert called_with == [tmp_path, layout]


def test_setup_managed_toolchains_factory_contains_all(tmp_path: Path) -> None:
    """Setup's managed_toolchains import resolves known ordered toolchains."""
    from harness.toolchain import managed_toolchains  # noqa: PLC0415

    resolved = tmp_path.resolve()
    layout = RepoLayout(
        root=resolved,
        build_dir=resolved / "build",
        out_dir=resolved / "out",
        toolchains_dir=resolved / "toolchains",
        third_party_dir=resolved / "third_party",
        inputs_dir=resolved / "inputs",
        downloads_dir=resolved / "toolchains" / "downloads",
        private_assets_dir=resolved / "inputs" / "external" / "private-assets",
        harness_disk_src=resolved / "tools" / "rust" / "bof3-disk",
        emi_ex_src=resolved / "tools" / "rust" / "emi-ex",
        harness_disk_bin=resolved / "toolchains" / "bof3-disk" / "bof3-disk",
        emi_ex_bin=resolved / "toolchains" / "emi-ex" / "emi-ex",
        psn00b_toolchain_root=resolved / "toolchains" / "psn00b_toolchain",
        psn00b_sdk_root=resolved / "toolchains" / "psn00bsdk",
        gcc272_psx_root=resolved / "toolchains" / "gcc-2.7.2-psx",
        gcc_variants_root=resolved / "toolchains" / "gcc-variants",
        psyq_root=resolved / "toolchains" / "psyq",
    )
    toolchains = managed_toolchains(resolved, layout)
    labels = [t.label for t in toolchains]
    assert labels == [
        "PSn00b",
        "GCC 2.7.2",
        "maspsx",
        "Rizin",
        "m2c",
        "asm-differ",
        "decomp-permuter",
        "PsyQ signatures",
        "splat",
        "spimdisasm",
    ]
