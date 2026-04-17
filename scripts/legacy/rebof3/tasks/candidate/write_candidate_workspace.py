from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from ...common import relative_to_root, write_json_output
from ...lib.pipeline import (
    PipelineContext,
    PipelineOptions,
    PipelineTask,
    option_logger,
)
from .common import build_candidate_workspace_payload


@dataclass(frozen=True, slots=True)
class WriteCandidateWorkspaceTask(PipelineTask):
    """Write the canonical workspace.json consumed by compile/diff tools."""

    task_name = "write_candidate_workspace"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        workspace_json, payload = build_candidate_workspace_payload(
            dict(context["function_row"]),
            inventory_db=Path(str(context["inventory_db"])).resolve(),
            workspace_root=Path(str(context["workspace_root"])).resolve(),
            build_root=Path(str(context["build_root"])).resolve(),
            source_file=Path(str(context["candidate_source_file"])).resolve(),
            bundle_payload=dict(context["bundle_payload"]),
        )
        write_json_output(workspace_json, payload)
        context["workspace_json"] = workspace_json
        context["workspace_payload"] = payload
        logger.debug(f"workspace {relative_to_root(workspace_json)}")
        return context


__all__ = ["WriteCandidateWorkspaceTask"]
