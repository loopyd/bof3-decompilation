"""Focused tests: setup toolchain task uses shared managed_toolchains factory."""

from __future__ import annotations

from pathlib import Path
from types import SimpleNamespace

from harness.io import repo_layout
from harness.commands import setup


def test_setup_toolchain_uses_managed_toolchains(tmp_path: Path, monkeypatch) -> None:
    """Setup _toolchain task delegates to managed_toolchains()."""
    called_with: list[object] = []

    def fake_lifecycle(layout, *, force=False):
        called_with.extend((layout, force))
        return iter(("demo",))

    monkeypatch.setattr(setup, "managed_lifecycle", fake_lifecycle)

    layout = SimpleNamespace(root=tmp_path)
    state = setup.SetupState(
        root=tmp_path, layout=layout, args=SimpleNamespace(force=False)
    )
    result = setup.TASKS[2].run(state)

    assert result == "demo"
    assert called_with == [layout, False]


def test_setup_managed_toolchains_factory_contains_all(tmp_path: Path) -> None:
    """Setup's managed_toolchains import resolves known ordered toolchains."""
    from harness.toolchain import managed_toolchains  # noqa: PLC0415

    layout = repo_layout(tmp_path)
    toolchains = managed_toolchains(layout)
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


def test_setup_primes_all_host_compatible_catalog_variants(
    tmp_path: Path, monkeypatch
) -> None:
    """Setup primes every host-compatible catalog candidate, selected or not."""
    installed: list[str] = []

    class FakeVariant:
        def __init__(self, cid: str) -> None:
            self.id = cid
            self.label = cid
            self.host = "linux-x86_64"

        def install(self, layout, *, force: bool = False) -> str:
            installed.append(self.id)
            return "installed"

    monkeypatch.setattr(setup, "managed_lifecycle", lambda layout, **kwargs: iter(()))
    monkeypatch.setattr(
        setup,
        "load_variants",
        lambda layout: [FakeVariant("gcc-2.6.3-psx"), FakeVariant("gcc-2.8.1-psx")],
    )

    layout = SimpleNamespace(root=tmp_path)
    state = setup.SetupState(
        root=tmp_path, layout=layout, args=SimpleNamespace(force=False)
    )
    result = setup.TASKS[2].run(state)

    assert installed == ["gcc-2.6.3-psx", "gcc-2.8.1-psx"]
    assert "primed variants: gcc-2.6.3-psx, gcc-2.8.1-psx" in result


def test_setup_skips_host_incompatible_catalog_variant(
    tmp_path: Path, monkeypatch
) -> None:
    """A host-incompatible catalog candidate is skipped and reported."""
    installed: list[str] = []

    class FakeVariant:
        id = "gcc-2.6.3-psx"
        label = "GCC 2.6.3 PSX"
        host = "darwin-x86_64"

        def install(self, layout, *, force: bool = False) -> str:
            installed.append(self.id)
            return "installed"

    monkeypatch.setattr(setup, "managed_lifecycle", lambda layout, **kwargs: iter(()))
    monkeypatch.setattr(setup, "load_variants", lambda layout: [FakeVariant()])

    layout = SimpleNamespace(root=tmp_path)
    state = setup.SetupState(
        root=tmp_path, layout=layout, args=SimpleNamespace(force=False)
    )
    result = setup.TASKS[2].run(state)

    assert installed == []
    assert "primed variants" not in result
    assert "skipped variants: gcc-2.6.3-psx (darwin-x86_64)" in result


def test_setup_empty_catalog_installs_no_variant(tmp_path: Path, monkeypatch) -> None:
    """An empty catalog primes no variant."""
    monkeypatch.setattr(setup, "managed_lifecycle", lambda layout, **kwargs: iter(()))
    monkeypatch.setattr(setup, "load_variants", lambda layout: [])

    layout = SimpleNamespace(root=tmp_path)
    state = setup.SetupState(
        root=tmp_path, layout=layout, args=SimpleNamespace(force=False)
    )
    result = setup.TASKS[2].run(state)

    assert "primed variants" not in result
    assert "skipped variants" not in result
