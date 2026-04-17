from __future__ import annotations

"""Pipeline builder for the full candidate flow, including permuter setup."""

from ..lib.pipeline import PipelineTask, pipeline
from ..tasks.candidate import RunPermuterTask
from .pipeline_candidate_build import pipeline_candidate_build
from .pipeline_candidate_prepare import pipeline_candidate_prepare


def pipeline_candidate_full() -> PipelineTask:
    """Run prepare, build, and one permuter lane as a single composed pipeline."""

    return pipeline(
        pipeline_candidate_prepare(),
        pipeline_candidate_build(),
        RunPermuterTask(),
        task_name="candidate_full",
    )


__all__ = ["pipeline_candidate_full"]
