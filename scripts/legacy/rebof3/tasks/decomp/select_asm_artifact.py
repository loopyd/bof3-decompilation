from __future__ import annotations

from dataclasses import dataclass
from ...common import relative_to_root, write_text_output
from ...lib.pipeline import (
    PipelineContext,
    PipelineOptions,
    PipelineTask,
    option_logger,
)
from .options import asm_backend


@dataclass(frozen=True, slots=True)
class SelectAsmArtifactTask(PipelineTask):
    """Choose the canonical asm artifact that downstream stages should consume."""

    task_name = "select_asm_artifact"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        backend = asm_backend(options)
        if backend == "spimdisasm":
            selected_asm_text = context.get("spim_asm_text")
            if not selected_asm_text:
                context["returncode"] = int(context.get("returncode") or 1) or 1
                context["selected_asm_error"] = (
                    "selected asm backend spimdisasm produced no output"
                )
                logger.debug(context["selected_asm_error"])
                return context
        else:
            selected_asm_text = context.get("ghidra_asm_text")
            if not selected_asm_text:
                context["returncode"] = int(context.get("returncode") or 1) or 1
                context["selected_asm_error"] = (
                    "selected asm backend ghidra produced no output"
                )
                logger.debug(context["selected_asm_error"])
                return context

        write_text_output(context["asm_path"], selected_asm_text)
        context["selected_asm_backend"] = backend
        context["selected_asm_text"] = selected_asm_text
        logger.debug(
            f"selected {backend} asm -> {relative_to_root(context['asm_path'])}"
        )
        return context


__all__ = ["SelectAsmArtifactTask"]
