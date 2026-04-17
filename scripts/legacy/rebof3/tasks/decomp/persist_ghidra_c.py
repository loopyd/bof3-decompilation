from __future__ import annotations

from dataclasses import dataclass
from ...common import relative_to_root, write_text_output
from ...lib.pipeline import PipelineContext, PipelineOptions, PipelineTask, option_logger


@dataclass(frozen=True, slots=True)
class PersistGhidraCArtifactTask(PipelineTask):
    """Persist the raw Ghidra decompiler output when it is available."""

    task_name = "persist_ghidra_c"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        export_payload = context.get("export_payload")
        if not export_payload:
            logger.debug("skip: export payload unavailable")
            return context

        ghidra_c = export_payload.get("ghidra_c") or ""
        if ghidra_c:
            write_text_output(context["ghidra_c_path"], ghidra_c)
            logger.debug(f"wrote ghidra c {relative_to_root(context['ghidra_c_path'])}")
        else:
            logger.debug("skip: ghidra decompiler produced no C artifact")
        context["ghidra_c"] = ghidra_c or None
        return context


__all__ = ["PersistGhidraCArtifactTask"]
