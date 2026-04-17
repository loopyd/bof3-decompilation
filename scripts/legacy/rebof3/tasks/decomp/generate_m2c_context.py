from __future__ import annotations

from dataclasses import dataclass
from ...common import relative_to_root
from ...lib.pipeline import (
    PipelineContext,
    PipelineOptions,
    PipelineTask,
    option_logger,
)
from ...re.services.m2c_context import generate_m2c_context_artifacts


@dataclass(frozen=True, slots=True)
class GenerateM2CContextTask(PipelineTask):
    """Generate the preprocessed context files that improve `m2c` output quality."""

    task_name = "generate_m2c_context"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        if context.get("no_m2c") or context.get("returncode", 0) != 0:
            context["m2c_context_metadata"] = {
                "attempted": False,
                "status": "skipped",
                "path": None,
                "stderr": None,
            }
            context["m2c_context_paths"] = []
            logger.debug("skip: m2c lane disabled or earlier stage failed")
            return context

        metadata = generate_m2c_context_artifacts(
            source_text=context["source_text"],
            requested_address=context["requested_address"],
            selected_asm_text=str(context.get("selected_asm_text") or ""),
            context_source_path=context["m2c_context_source_path"],
            context_preprocessed_path=context["m2c_context_preprocessed_path"],
            program_name=context.get("program_name"),
        )
        context["m2c_context_metadata"] = metadata
        context["m2c_context_paths"] = (
            [context["m2c_context_preprocessed_path"]]
            if metadata["status"] == "ok"
            else []
        )
        if metadata["status"] == "ok":
            logger.debug(
                f"context {relative_to_root(context['m2c_context_preprocessed_path'])}"
            )
        elif metadata.get("stderr"):
            logger.debug(
                f"m2c context status={metadata['status']} stderr={metadata['stderr']}"
            )
        return context


__all__ = ["GenerateM2CContextTask"]
