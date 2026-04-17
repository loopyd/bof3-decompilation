from __future__ import annotations

from dataclasses import dataclass
from ...common import ROOT
from ...lib.pipeline import PipelineContext, PipelineOptions, PipelineTask, option_logger
from .options import candidate_variant


@dataclass(frozen=True, slots=True)
class SelectCandidateSourceTask(PipelineTask):
    """Choose which recovered C seed becomes the candidate stub source."""

    task_name = "select_candidate_source"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        if context.get("candidate_source_text") is not None:
            logger.debug("reuse candidate source already present in context")
            return context

        variant = candidate_variant(options)
        bundle_payload = dict(context["bundle_payload"])
        files = bundle_payload.get("files") or {}
        path_text = files.get("ghidra_c") if variant == "ghidra" else files.get("m2c_c")
        if not path_text:
            raise FileNotFoundError(f"missing {variant} candidate source in bundle")

        source_path = ROOT / str(path_text)
        if not source_path.exists():
            raise FileNotFoundError(source_path)

        context["candidate_source_variant"] = variant
        context["candidate_source_path"] = source_path
        context["candidate_source_text"] = source_path.read_text(encoding="utf-8")
        logger.debug(f"selected {variant} source {source_path}")
        return context


__all__ = ["SelectCandidateSourceTask"]
