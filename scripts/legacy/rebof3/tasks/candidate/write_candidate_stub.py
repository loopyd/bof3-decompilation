from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from ...common import relative_to_root, write_text_output
from ...lib.pipeline import (
    PipelineContext,
    PipelineOptions,
    PipelineTask,
    option_logger,
)
from .common import wrap_candidate_source_text
from .options import force_rewrite_source


@dataclass(frozen=True, slots=True)
class WriteCandidateStubTask(PipelineTask):
    """Write the selected candidate source into the disabled-stub tree."""

    task_name = "write_candidate_stub"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        target_path = Path(str(context["candidate_source_file"])).resolve()
        if target_path.exists() and not force_rewrite_source(options):
            context["candidate_source_file"] = target_path
            logger.debug(f"reuse stub {relative_to_root(target_path)}")
            return context

        bundle_payload = dict(context["bundle_payload"])
        function_meta = bundle_payload.get("function") or {}
        final_text = wrap_candidate_source_text(
            str(context["candidate_source_text"]),
            program_path=str(context["program_path"]),
            entry_hex=str(context["entry_hex"]),
            function_name=str(context["function_name"]),
            original_symbol_name=(
                None
                if function_meta.get("name") in (None, "")
                else str(function_meta.get("name"))
            ),
            stub_source_path=target_path,
        )
        write_text_output(target_path, final_text)
        context["candidate_source_file"] = target_path
        logger.debug(f"wrote stub {relative_to_root(target_path)}")
        return context


__all__ = ["WriteCandidateStubTask"]
