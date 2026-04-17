from __future__ import annotations

from dataclasses import dataclass
from ...common import relative_to_root
from ...lib.pipeline import (
    PipelineContext,
    PipelineOptions,
    PipelineTask,
    option_logger,
)
from ...re.services.ghidra.decomp_helpers import load_program_symbol_resolver
from ...re.services.spimdisasm_backend import run_spimdisasm_function_asm
from .options import asm_backend, emit_spimdisasm


@dataclass(frozen=True, slots=True)
class SpimdisasmAsmTask(PipelineTask):
    """Render a binary-backed spimdisasm alternative for the selected function."""

    task_name = "spimdisasm_asm"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        backend = asm_backend(options)
        should_emit = emit_spimdisasm(options) or backend == "spimdisasm"
        if not should_emit or context.get("function_payload") is None:
            context["spimdisasm_metadata"] = {
                "attempted": False,
                "status": "skipped",
                "path": None,
                "stderr": None,
            }
            logger.debug("skip: spimdisasm lane disabled or function payload missing")
            return context

        resolver = context.get("resolver")
        if resolver is None:
            resolver = load_program_symbol_resolver(context["source_text"])
            context["resolver"] = resolver

        logger.debug(f"render {relative_to_root(context['spim_asm_path'])}")
        metadata = run_spimdisasm_function_asm(
            source_text=context["source_text"],
            function_payload=context["function_payload"],
            output_path=context["spim_asm_path"],
            resolver=resolver,
        )
        metadata["attempted"] = True
        metadata["path"] = metadata.get("output_path")
        if metadata["status"] == "ok":
            context["spim_asm_text"] = context["spim_asm_path"].read_text(
                encoding="utf-8"
            )
            logger.debug(f"wrote spim asm {relative_to_root(context['spim_asm_path'])}")
        elif metadata.get("stderr"):
            logger.debug(
                f"spimdisasm status={metadata['status']} stderr={metadata['stderr']}"
            )
        context["spimdisasm_metadata"] = metadata
        return context


__all__ = ["SpimdisasmAsmTask"]
