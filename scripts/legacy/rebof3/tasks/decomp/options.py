from __future__ import annotations

from ...lib.pipeline import PipelineOptions


DEFAULT_ASM_BACKEND = "ghidra"
DEFAULT_EMIT_SPIMDISASM = True


def asm_backend(options: PipelineOptions | None) -> str:
    """Return the canonical asm backend for this pipeline run."""

    if options is None:
        return DEFAULT_ASM_BACKEND
    return str(options.get("asm_backend", DEFAULT_ASM_BACKEND))


def emit_spimdisasm(options: PipelineOptions | None) -> bool:
    """Return whether the optional spimdisasm side artifact should be emitted."""

    if options is None:
        return DEFAULT_EMIT_SPIMDISASM
    return bool(options.get("emit_spimdisasm", DEFAULT_EMIT_SPIMDISASM))


def enable_m2c(options: PipelineOptions | None) -> bool:
    """Return whether the caller wants the `m2c` lane to run."""

    if options is None:
        return True
    return not bool(options.get("no_m2c", False))


__all__ = [
    "DEFAULT_ASM_BACKEND",
    "DEFAULT_EMIT_SPIMDISASM",
    "asm_backend",
    "emit_spimdisasm",
    "enable_m2c",
]
