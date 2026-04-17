from __future__ import annotations

from dataclasses import dataclass
from ...common import relative_to_root, run_command, write_text_output
from ...lib.pipeline import (
    PipelineContext,
    PipelineOptions,
    PipelineTask,
    option_logger,
)
from ...re.services.m2c_runner import build_m2c_command


@dataclass(frozen=True, slots=True)
class RunM2CTask(PipelineTask):
    """Run `m2c` on the normalized asm and capture both output and metadata."""

    task_name = "run_m2c"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        if context.get("no_m2c"):
            context["m2c_metadata"] = {
                "attempted": False,
                "path": None,
                "status": "skipped",
                "stderr": None,
            }
            logger.debug("skip: m2c disabled")
            return context

        context_paths = list(context.get("m2c_context_paths") or [])
        logger.debug(
            " ".join(
                [
                    f"asm={relative_to_root(context['m2c_asm_path'])}",
                    f"contexts={len(context_paths)}",
                ]
            )
        )
        result = run_command(
            build_m2c_command(context["m2c_asm_path"], context_paths=context_paths)
        )
        metadata: dict[str, Any] = {
            "attempted": True,
            "path": None,
            "status": "pending",
            "stderr": None,
            "input_backend": context.get("selected_asm_backend"),
            "context_paths": [relative_to_root(path) for path in context_paths],
        }
        if result.returncode == 0:
            write_text_output(context["m2c_c_path"], result.stdout)
            metadata["path"] = relative_to_root(context["m2c_c_path"])
            metadata["status"] = "ok"
            logger.debug(f"wrote m2c {metadata['path']}")
        else:
            metadata["status"] = "failed"
            metadata["stderr"] = (result.stderr or result.stdout).strip() or None
            logger.debug(f"m2c failed: {metadata['stderr']}")
        context["m2c_metadata"] = metadata
        return context


__all__ = ["RunM2CTask"]
