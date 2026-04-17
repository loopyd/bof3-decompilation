from __future__ import annotations

import argparse
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import normalize_repo_path, write_json_output, write_text_output
from ..models.core import MatchMetrics
from . import (
    history as history_lib,
    pipeline_backend,
    pipeline_ready,
    workspace as workspace_lib,
)


def load_build_status(path: Path) -> dict[str, Any] | None:
    return pipeline_ready.load_build_status(path)


def refresh_expected_baseline(
    workspace_json: Path, workspace_payload: dict[str, Any]
) -> dict[str, Any]:
    state = pipeline_ready.build_workspace_state(workspace_json, workspace_payload)
    return pipeline_ready.refresh_expected_baseline(state).workspace_payload


def diff_status(
    workspace_payload: dict[str, Any],
    *,
    build_status: dict[str, Any] | None,
    ghidra_bundle_exists: bool,
) -> tuple[str, list[str]]:
    state = pipeline_ready.WorkspaceState(
        workspace_json=Path("workspace.json"),
        workspace_dir=Path("."),
        workspace_payload=workspace_payload,
        build_status=build_status,
        ghidra_bundle_path=normalize_repo_path(
            workspace_payload.get("ghidra_decomp_bundle_json")
        ),
        ghidra_bundle_exists=ghidra_bundle_exists,
        refresh_log_path=Path("ghidra_decomp.log"),
    )
    return pipeline_ready.diff_status(state)


def run_diff_backends(
    workspace_dir: Path, workspace_payload: dict[str, Any]
) -> dict[str, Any]:
    return pipeline_backend.run_diff_backends(workspace_dir, workspace_payload)


def summarize_match_metrics(backends: dict[str, Any]) -> dict[str, Any]:
    asm_backend = backends.get("asm-differ") or {}
    asm_summary = asm_backend.get("diff_summary") or {}
    obj_backend = backends.get("objdiff") or {}
    obj_summary = obj_backend.get("diff_summary") or {}
    semantic_backend = backends.get("semantic-diff") or {}
    semantic_summary = semantic_backend.get("diff_summary") or {}
    semantic_counts = semantic_summary.get("category_counts") or {}
    current_slice = asm_backend.get("current_slice") or {}
    size = current_slice.get("size")
    asm_score = asm_summary.get("current_score")
    asm_rows = asm_summary.get("row_count")
    metrics = MatchMetrics(
        asm_score=asm_score,
        asm_max_score=asm_summary.get("max_score"),
        asm_row_count=asm_rows,
        asm_score_per_row=None
        if not asm_rows or asm_score is None
        else round(float(asm_score) / float(asm_rows), 3),
        asm_score_per_byte=None
        if not size or asm_score is None
        else round(float(asm_score) / float(size), 3),
        objdiff_match_percent=obj_summary.get("text_match_percent"),
        objdiff_instruction_count=obj_summary.get("instruction_count"),
        objdiff_mismatch_count=obj_summary.get("mismatch_count"),
        semantic_status=semantic_summary.get("semantic_status"),
        semantic_classified_mismatch_count=semantic_summary.get(
            "classified_mismatch_count"
        ),
        semantic_unclassified_mismatch_count=semantic_summary.get(
            "unclassified_mismatch_count"
        ),
        semantic_move_zero_sugar_count=semantic_counts.get("move_zero_sugar"),
        semantic_li_zero_sugar_count=semantic_counts.get("li_zero_sugar"),
        semantic_branch_zero_sugar_count=semantic_counts.get("branch_zero_sugar"),
        semantic_commutative_swap_count=semantic_counts.get("commutative_swap"),
        semantic_call_target_reloc_count=semantic_counts.get("call_target_reloc"),
        semantic_address_materialization_count=semantic_counts.get(
            "address_materialization"
        ),
        semantic_asm_view_only_noise=semantic_summary.get("asm_view_only_noise"),
    )
    return metrics.as_dict()


def render_markdown(report: dict[str, Any]) -> str:
    lines = [
        "# Match Diff Report",
        "",
        f"- Program: `{report['program_path']}`",
        f"- Entry: `{report['entry_hex']}`",
        f"- Status: `{report['status']}`",
        f"- Ghidra bundle exists: {report['ghidra_bundle_exists']}",
        f"- Build status present: {report['build_status_present']}",
        f"- Source mapping ready: {report['source_mapping_ready']}",
        f"- Expected baseline ready: {report.get('expected_baseline_ready', False)}",
        f"- Backend ready: {report.get('backend_ready', False)}",
        f"- asm-differ ran: {bool((report.get('backends') or {}).get('asm-differ'))}",
        f"- objdiff ran: {bool((report.get('backends') or {}).get('objdiff'))}",
        f"- semantic-diff ran: {bool((report.get('backends') or {}).get('semantic-diff'))}",
        "",
    ]
    semantic_metrics = report.get("match_metrics") or {}
    semantic_status = semantic_metrics.get("semantic_status")
    if semantic_status is not None:
        lines.extend(
            [
                "## Semantic Sidecar",
                "",
                f"- Status: `{semantic_status}`",
                f"- Classified mismatches: {semantic_metrics.get('semantic_classified_mismatch_count')}",
                f"- Unclassified mismatches: {semantic_metrics.get('semantic_unclassified_mismatch_count')}",
                f"- move/zero sugar: {semantic_metrics.get('semantic_move_zero_sugar_count')}",
                f"- li sugar: {semantic_metrics.get('semantic_li_zero_sugar_count')}",
                f"- branch sugar: {semantic_metrics.get('semantic_branch_zero_sugar_count')}",
                f"- commutative swaps: {semantic_metrics.get('semantic_commutative_swap_count')}",
                f"- call target relocations: {semantic_metrics.get('semantic_call_target_reloc_count')}",
                f"- address materialization: {semantic_metrics.get('semantic_address_materialization_count')}",
                "",
            ]
        )
    lines.extend(
        [
        "## Next Steps",
        "",
        ]
    )
    for step in report["next_steps"]:
        lines.append(f"- {step}")
    return "\n".join(lines) + "\n"


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "diff"),
        description="Assess one match workspace and optionally run backend diffs.",
    )
    add_logging_args(parser)
    pipeline_ready.add_workspace_resolver_args(parser)
    parser.add_argument(
        "--refresh-ghidra-bundle",
        action="store_true",
        help="Run the recorded ghidra_decomp command when func.json is missing",
    )
    parser.add_argument(
        "--run-backend",
        action="store_true",
        help="Prepare and run asm-differ and objdiff backends when the workspace is ready",
    )
    parser.add_argument("--dry-run", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def record_diff_attempt(
    workspace_dir: Path,
    workspace_payload: dict[str, Any],
    *,
    status: str,
    report_path: Path | None,
    run_backend: bool,
    refresh_ghidra_bundle: bool,
    backend_ready: bool,
    succeeded: bool,
    returncode: int,
    error: str | None = None,
    match_metrics: dict[str, Any] | None = None,
) -> dict[str, Any]:
    history_lib.append_entry(
        workspace_dir,
        {
            "event": "diff",
            "program_path": workspace_payload.get("program_path"),
            "entry_hex": workspace_payload.get("entry_hex"),
            "status": status,
            "run_backend": bool(run_backend),
            "refresh_ghidra_bundle": bool(refresh_ghidra_bundle),
            "backend_ready": bool(backend_ready),
            "succeeded": bool(succeeded),
            "returncode": int(returncode),
            "error": error,
            "report_path": None
            if report_path is None
            else workspace_lib.relative_to_root(report_path),
            "match_metrics": dict(match_metrics or {}),
        },
    )
    return history_lib.summarize_workspace(workspace_dir)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_diff")
    resolved = pipeline_ready.resolve_workspace(args, logger)
    if resolved is None:
        return 1
    workspace_json, workspace_payload = resolved
    state = pipeline_ready.build_workspace_state(workspace_json, workspace_payload)

    if args.dry_run:
        logger.summary(
            f"workspace={state.workspace_payload.get('workspace_dir')} refresh_ghidra_bundle={bool(args.refresh_ghidra_bundle)} run_backend={bool(args.run_backend)}"
        )
        return 0

    refresh_result = None
    if not state.ghidra_bundle_exists and args.refresh_ghidra_bundle:
        state, refresh_result = pipeline_ready.maybe_refresh_ghidra_bundle(
            state,
            refresh=True,
        )
        if refresh_result is not None and refresh_result.returncode != 0:
            logger.error(
                "ghidra_decomp refresh failed; see "
                f"{workspace_lib.relative_to_root(state.refresh_log_path)}"
            )
            return refresh_result.returncode

    state = pipeline_ready.refresh_expected_baseline(state)

    status, next_steps = pipeline_ready.diff_status(state)
    source_mapping_ready = bool(state.workspace_payload.get("source_mapping_ready"))
    expected_baseline_ready = bool(
        state.workspace_payload.get("expected_baseline_ready")
    )
    backend_reports: dict[str, Any] = {}
    backend_ready = status == "ready_for_backend_diff"
    report_json = state.workspace_dir / "diff.json"
    report_md = state.workspace_dir / "diff.md"
    if backend_ready and args.run_backend:
        try:
            backend_reports = run_diff_backends(
                state.workspace_dir,
                state.workspace_payload,
            )
        except pipeline_backend.BackendFailure as exc:
            history_summary = record_diff_attempt(
                state.workspace_dir,
                state.workspace_payload,
                status="backend_failed",
                report_path=None,
                run_backend=bool(args.run_backend),
                refresh_ghidra_bundle=bool(args.refresh_ghidra_bundle),
                backend_ready=backend_ready,
                succeeded=False,
                returncode=exc.returncode,
                error=str(exc),
                match_metrics=None,
            )
            error_report = {
                "workspace_dir": state.workspace_payload.get("workspace_dir"),
                "program_path": state.workspace_payload.get("program_path"),
                "entry_hex": state.workspace_payload.get("entry_hex"),
                "status": "backend_failed",
                "ghidra_bundle_json": state.workspace_payload.get(
                    "ghidra_decomp_bundle_json"
                ),
                "ghidra_bundle_exists": state.ghidra_bundle_exists,
                "build_status_present": state.build_status is not None,
                "build_status": state.build_status,
                "source_mapping_ready": source_mapping_ready,
                "source_mapping": state.workspace_payload.get("source_mapping"),
                "expected_baseline_ready": expected_baseline_ready,
                "expected_baseline": state.workspace_payload.get("expected_baseline"),
                "backend_ready": backend_ready,
                "backend": None,
                "backends": backend_reports,
                "match_metrics": summarize_match_metrics(backend_reports),
                "next_steps": [str(exc)],
                "ghidra_decomp_refresh_log": None
                if refresh_result is None
                else workspace_lib.relative_to_root(state.refresh_log_path),
                "history_path": workspace_lib.relative_to_root(
                    Path(str(history_summary.get("history_path") or ""))
                ),
                "history_summary": history_summary,
            }
            write_json_output(report_json, error_report)
            write_text_output(report_md, render_markdown(error_report))
            logger.error(str(exc))
            return exc.returncode

    history_summary = record_diff_attempt(
        state.workspace_dir,
        state.workspace_payload,
        status=status,
        report_path=report_json,
        run_backend=bool(args.run_backend),
        refresh_ghidra_bundle=bool(args.refresh_ghidra_bundle),
        backend_ready=backend_ready,
        succeeded=True,
        returncode=0,
        match_metrics=summarize_match_metrics(backend_reports),
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
        "source_mapping_ready": source_mapping_ready,
        "source_mapping": state.workspace_payload.get("source_mapping"),
        "expected_baseline_ready": expected_baseline_ready,
        "expected_baseline": state.workspace_payload.get("expected_baseline"),
        "backend_ready": backend_ready,
        "backend": backend_reports.get("asm-differ"),
        "backends": backend_reports,
        "match_metrics": summarize_match_metrics(backend_reports),
        "next_steps": next_steps,
        "ghidra_decomp_refresh_log": None
        if refresh_result is None
        else workspace_lib.relative_to_root(state.refresh_log_path),
        "history_path": workspace_lib.relative_to_root(
            Path(str(history_summary.get("history_path") or ""))
        ),
        "history_summary": history_summary,
    }

    write_json_output(report_json, report)
    write_text_output(report_md, render_markdown(report))
    logger.summary(
        "workspace="
        f"{state.workspace_payload.get('workspace_dir')} "
        f"status={status} "
        f"report={workspace_lib.relative_to_root(report_json)}"
    )
    return 0
