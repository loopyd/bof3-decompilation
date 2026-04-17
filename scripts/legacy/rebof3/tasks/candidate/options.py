from __future__ import annotations

from ...config import DEFAULT_PSX_PROFILE
from ...lib.pipeline import PipelineOptions
from ...match import permuter as permuter_lib


DEFAULT_CANDIDATE_VARIANT = "m2c"
DEFAULT_PERMUTER_VARIANT = "repo"
DEFAULT_PERMUTER_TIMEOUT_SECONDS = 60
DEFAULT_ASM_BACKEND = "spimdisasm"
DEFAULT_EMIT_SPIMDISASM = True


def candidate_variant(options: PipelineOptions | None) -> str:
    if options is None:
        return DEFAULT_CANDIDATE_VARIANT
    return str(options.get("candidate_variant", DEFAULT_CANDIDATE_VARIANT))


def force_reconfigure(options: PipelineOptions | None) -> bool:
    return bool(options and options.get("force_reconfigure"))


def force_decomp(options: PipelineOptions | None) -> bool:
    return bool(options and options.get("force_decomp"))


def force_rewrite_source(options: PipelineOptions | None) -> bool:
    return bool(options and options.get("force_rewrite_source"))


def profile(options: PipelineOptions | None) -> str:
    if options is None:
        return DEFAULT_PSX_PROFILE
    return str(options.get("profile", DEFAULT_PSX_PROFILE))


def permuter_timeout_seconds(options: PipelineOptions | None) -> int:
    if options is None:
        return DEFAULT_PERMUTER_TIMEOUT_SECONDS
    return int(
        options.get("permuter_timeout_seconds", DEFAULT_PERMUTER_TIMEOUT_SECONDS)
    )


def permuter_variant(options: PipelineOptions | None) -> str:
    if options is None:
        return DEFAULT_PERMUTER_VARIANT
    return str(options.get("permuter_variant", DEFAULT_PERMUTER_VARIANT))


def permuter_threads(options: PipelineOptions | None) -> int:
    default_threads = max(permuter_lib.default_threads(), 1)
    if options is None:
        return default_threads
    return int(options.get("permuter_threads", default_threads))


def asm_backend(options: PipelineOptions | None) -> str:
    if options is None:
        return DEFAULT_ASM_BACKEND
    return str(options.get("asm_backend", DEFAULT_ASM_BACKEND))


def emit_spimdisasm(options: PipelineOptions | None) -> bool:
    if options is None:
        return DEFAULT_EMIT_SPIMDISASM
    return bool(options.get("emit_spimdisasm", DEFAULT_EMIT_SPIMDISASM))


def enable_m2c(options: PipelineOptions | None) -> bool:
    if options is None:
        return True
    return not bool(options.get("no_m2c", False))


def permuter_args(options: PipelineOptions | None) -> list[str]:
    if options is None:
        return []
    return [str(value) for value in options.get("permuter_args", [])]


__all__ = [
    "DEFAULT_ASM_BACKEND",
    "DEFAULT_CANDIDATE_VARIANT",
    "DEFAULT_EMIT_SPIMDISASM",
    "DEFAULT_PERMUTER_TIMEOUT_SECONDS",
    "DEFAULT_PERMUTER_VARIANT",
    "asm_backend",
    "candidate_variant",
    "emit_spimdisasm",
    "enable_m2c",
    "force_decomp",
    "force_reconfigure",
    "force_rewrite_source",
    "permuter_args",
    "permuter_threads",
    "permuter_timeout_seconds",
    "permuter_variant",
    "profile",
]
