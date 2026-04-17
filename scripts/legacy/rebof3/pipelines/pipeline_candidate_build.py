from __future__ import annotations

"""Pipeline builder for the compile/diff half of candidate generation."""

from ..lib.pipeline import PipelineTask, pipeline
from ..tasks.candidate import CompileWorkspaceTask, RunDiffTask


def pipeline_candidate_build() -> PipelineTask:
    """Compile one prepared candidate workspace and record diff artifacts."""

    return pipeline(
        CompileWorkspaceTask(),
        RunDiffTask(),
        task_name="candidate_build",
    )


__all__ = ["pipeline_candidate_build"]
