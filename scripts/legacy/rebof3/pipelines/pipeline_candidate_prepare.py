from __future__ import annotations

"""Pipeline builder for the prepare half of candidate generation."""

from ..lib.pipeline import PipelineTask, pipeline
from ..tasks.candidate import (
    ConfigureStubBuildTask,
    ResolveFunctionTask,
    RunDecompBundleTask,
    SelectCandidateSourceTask,
    WriteCandidateStubTask,
    WriteCandidateWorkspaceTask,
)


def pipeline_candidate_prepare() -> PipelineTask:
    """Resolve a function, seed a stub, and write the candidate workspace."""

    return pipeline(
        ResolveFunctionTask(),
        RunDecompBundleTask(),
        SelectCandidateSourceTask(),
        WriteCandidateStubTask(),
        ConfigureStubBuildTask(),
        WriteCandidateWorkspaceTask(),
        task_name="candidate_prepare",
    )


__all__ = ["pipeline_candidate_prepare"]
