"""Toolchain-specific installers and validation helpers."""

from __future__ import annotations

from pathlib import Path

from ..io import RepoLayout


# Order-sensitive: setup run labels and doctor verify labels share this sequence.
_MANAGED_TOOLCHAIN_TYPES: list[tuple[type, str]] = []


def register_managed_toolchain(toolchain_type: type, constructor_arg: str) -> None:
    """Register a managed toolchain with its constructor argument type.

    ``constructor_arg`` must be ``"root"`` (for ``root: Path``) or ``"layout"``
    (for ``layout: RepoLayout``).
    """
    _MANAGED_TOOLCHAIN_TYPES.append((toolchain_type, constructor_arg))


def _register() -> None:
    """Register managed toolchains in registration order."""
    if _MANAGED_TOOLCHAIN_TYPES:
        return  # already registered
    # Lazy imports to avoid circular dependency at package level.
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

    for tc, arg in (
        (Psn00bToolchain, "layout"),
        (GccToolchain, "layout"),
        (MaspsxToolchain, "root"),
        (RizinToolchain, "layout"),
        (M2cToolchain, "root"),
        (AsmDifferToolchain, "root"),
        (DecompPermuterToolchain, "root"),
        (PsyqSignaturesToolchain, "root"),
        (SplatToolchain, "root"),
        (SpimdisasmToolchain, "root"),
    ):
        register_managed_toolchain(tc, arg)


def managed_toolchains(root: Path, layout: RepoLayout) -> tuple:
    """Return instantiated managed external toolchains in registration order.

    Setup calls ``toolchain.run(force=...)``; doctor calls
    ``toolchain.verify()``.  Both consume this ordered factory so the
    membership list never drifts.
    """
    _register()
    result: list = []
    for toolchain_type, arg in _MANAGED_TOOLCHAIN_TYPES:
        if arg == "layout":
            result.append(toolchain_type(layout))
        elif arg == "root":
            result.append(toolchain_type(root))
        else:
            raise ValueError(f"unknown constructor arg type: {arg!r}")
    return tuple(result)
