from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from ...common import relative_to_root, write_json_output, write_text_output
from ...lib.pipeline import PipelineContext, PipelineOptions, PipelineTask, option_logger
from ...match import diff as diff_lib
from ...match import pipeline_ready


@dataclass(frozen=True, slots=True)
class RunDiffTask(PipelineTask):
    """Run asm-differ and objdiff over the prepared candidate workspace."""

    task_name = "run_diff"

    def run(
        self,
        context: PipelineContext,
        *,
        options: PipelineOptions | None = None,
    ) -> PipelineContext:
        logger = option_logger(options, fallback_name=self.name)
        workspace_json = Path(str(context["workspace_json"])).resolve()
        workspace_payload = dict(context["workspace_payload"])
        state = pipeline_ready.refresh_expected_baseline(
            pipeline_ready.build_workspace_state(workspace_json, workspace_payload)
        )
        status, next_steps = pipeline_ready.diff_status(state)
        if status != "ready_for_backend_diff":
            raise RuntimeError(
                f"workspace is not ready for backend diff: {status} ({'; '.join(next_steps)})"
            )

        backend_reports = diff_lib.run_diff_backends(
            state.workspace_dir,
            state.workspace_payload,
        )
        report = {
            "workspace_dir": state.workspace_payload.get("workspace_dir"),
            "program_path": state.workspace_payload.get("program_path"),
            "entry_hex": state.workspace_payload.get("entry_hex"),
            "status": status,
            "ghidra_bundle_json": state.workspace_payload.get("ghidra_decomp_bundle_json"),
            "ghidra_bundle_exists": state.ghidra_bundle_exists,
            "build_status_present": state.build_status is not None,
            "build_status": state.build_status,
            "source_mapping_ready": bool(state.workspace_payload.get("source_mapping_ready")),
            "source_mapping": state.workspace_payload.get("source_mapping"),
            "expected_baseline_ready": bool(state.workspace_payload.get("expected_baseline_ready")),
            "expected_baseline": state.workspace_payload.get("expected_baseline"),
            "backend_ready": True,
            "backend": backend_reports.get("asm-differ"),
            "backends": backend_reports,
            "match_metrics": diff_lib.summarize_match_metrics(backend_reports),
            "next_steps": next_steps,
        }
        report_json = state.workspace_dir / "diff.json"
        report_md = state.workspace_dir / "diff.md"
        write_json_output(report_json, report)
        write_text_output(report_md, diff_lib.render_markdown(report))
        context["diff_report_path"] = report_json
        context["diff_report"] = report
        metrics = report["match_metrics"]
        logger.debug(
            " ".join(
                [
                    f"diff={relative_to_root(report_json)}",
                    f"semantic={metrics['semantic_status']}",
                    f"asm_score={metrics['asm_score']}/{metrics['asm_max_score']}",
                ]
            )
        )
        return context


__all__ = ["RunDiffTask"]
