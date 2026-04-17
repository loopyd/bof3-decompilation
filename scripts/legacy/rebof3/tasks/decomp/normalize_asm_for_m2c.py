from __future__ import annotations

from dataclasses import dataclass
from ...common import relative_to_root, write_text_output
from ...lib.pipeline import (
    PipelineContext,
    PipelineOptions,
    PipelineTask,
    option_logger,
)
from ...re.services.ghidra.decomp_helpers import (
    load_program_symbol_resolver,
    rewrite_asm_for_m2c,
)
from .options import asm_backend


@dataclass(frozen=True, slots=True)
class NormalizeAsmForM2CTask(PipelineTask):
    """Apply the minimal compatibility rewrites needed before invoking `m2c`."""

    task_name = "normalize_asm_for_m2c"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        selected_asm_text = context.get("selected_asm_text")
        if not selected_asm_text or context.get("returncode", 0) != 0:
            logger.debug("skip: selected asm unavailable or earlier stage failed")
            return context

        resolver = context.get("resolver")
        if resolver is None:
            resolver = load_program_symbol_resolver(context["source_text"])
            context["resolver"] = resolver

        backend = asm_backend(options)
        rewritten_asm = (
            selected_asm_text
            if backend == "spimdisasm"
            else rewrite_asm_for_m2c(selected_asm_text, resolver=resolver)
        )
        write_text_output(context["m2c_asm_path"], rewritten_asm)
        context["rewritten_asm"] = rewritten_asm
        logger.debug(
            " ".join(
                [
                    f"backend={backend}",
                    f"asm={relative_to_root(context['m2c_asm_path'])}",
                ]
            )
        )
        return context


__all__ = ["NormalizeAsmForM2CTask"]
