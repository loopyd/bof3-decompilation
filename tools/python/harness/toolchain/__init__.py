"""Ordered registry for repository-managed toolchains."""

from __future__ import annotations

from collections.abc import Iterator

from ..io import RepoLayout
from .base import ExecutableToolchain, Toolchain


# Order-sensitive: setup and doctor consume the same membership and labels.
def _managed_types() -> tuple[tuple[str, type[Toolchain]], ...]:
    from .asm_differ import AsmDifferToolchain  # noqa: PLC0415
    from .gcc import GccToolchain  # noqa: PLC0415
    from .m2c import M2cToolchain  # noqa: PLC0415
    from .maspsx import MaspsxToolchain  # noqa: PLC0415
    from .permuter import DecompPermuterToolchain  # noqa: PLC0415
    from .psn00b import Psn00bToolchain  # noqa: PLC0415
    from .rizin import RizinToolchain  # noqa: PLC0415
    from .signatures import PsyqSignaturesToolchain  # noqa: PLC0415
    from .splat import SplatToolchain  # noqa: PLC0415
    from .spimdisasm import SpimdisasmToolchain  # noqa: PLC0415

    registry = (
        ("psn00b", Psn00bToolchain),
        ("gcc", GccToolchain),
        ("maspsx", MaspsxToolchain),
        ("rizin", RizinToolchain),
        ("m2c", M2cToolchain),
        ("asm-differ", AsmDifferToolchain),
        ("decomp-permuter", DecompPermuterToolchain),
        ("psyq-signatures", PsyqSignaturesToolchain),
        ("splat", SplatToolchain),
        ("spimdisasm", SpimdisasmToolchain),
    )
    keys = tuple(key for key, _ in registry)
    if len(keys) != len(set(keys)):
        raise ValueError("duplicate managed toolchain registry key")
    return registry


def managed_toolchains(layout: RepoLayout) -> tuple[Toolchain, ...]:
    """Instantiate every managed toolchain in stable lifecycle order."""
    return tuple(toolchain_type(layout) for _, toolchain_type in _managed_types())


def managed_toolchain(layout: RepoLayout, key: str) -> ExecutableToolchain:
    """Return one executable managed toolchain by its stable registry key."""
    for registered_key, toolchain_type in _managed_types():
        if registered_key != key:
            continue
        toolchain = toolchain_type(layout)
        if isinstance(toolchain, ExecutableToolchain):
            return toolchain
        break
    raise ValueError(f"unknown toolchain executable: {key}")


def managed_lifecycle(
    layout: RepoLayout, *, force: bool = False, verify_only: bool = False
) -> Iterator[str]:
    """Run the base-owned lifecycle in registry order and yield actual labels."""
    for toolchain in managed_toolchains(layout):
        yield toolchain.verify() if verify_only else toolchain.run(force=force)
