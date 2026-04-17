from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import relative_to_root, write_json_output, write_text_output
from ..config import DEFAULT_MATCH_ROOT
from . import history as history_lib
from . import report_refresh
from . import scoreboard as scoreboard_lib


def score_row(payload: dict[str, Any]) -> tuple[float, str, str]:
    metrics = payload.get("match_metrics") or {}
    match_percent = float(metrics.get("objdiff_match_percent") or 0.0)
    asm_score = float(metrics.get("asm_score") or 0.0)
    program = str(payload.get("program_path") or "")
    entry = str(payload.get("entry_hex") or "")
    return match_percent, -asm_score, f"{program}:{entry}"


def match_bucket(metrics: dict[str, Any]) -> str:
    percent = float(metrics.get("objdiff_match_percent") or 0.0)
    if percent >= 90.0:
        return "excellent"
    if percent >= 60.0:
        return "strong"
    if percent >= 25.0:
        return "promising"
    if percent > 0.0:
        return "weak"
    return "unmatched"


def row_from_diff_payload(
    payload: dict[str, Any], *, report_path: str
) -> dict[str, Any]:
    source_mapping = payload.get("source_mapping") or {}
    workspace_dir_text = str(payload.get("workspace_dir") or "")
    history_summary = payload.get("history_summary")
    if not isinstance(history_summary, dict):
        diff_path = Path(report_path)
        if diff_path.is_absolute():
            history_summary = history_lib.summarize_entries(
                history_lib.load_entries_from_path(diff_path.with_name("history.jsonl"))
            )
        else:
            history_summary = {}
    return {
        "workspace_dir": workspace_dir_text,
        "program_path": payload.get("program_path"),
        "entry_hex": payload.get("entry_hex"),
        "status": payload.get("status"),
        "source_file": source_mapping.get("source_file"),
        "source_function": source_mapping.get("source_function"),
        "match_metrics": payload.get("match_metrics") or {},
        "match_bucket": match_bucket(payload.get("match_metrics") or {}),
        "attempt_count": int(history_summary.get("attempt_count") or 0),
        "diff_attempt_count": int(history_summary.get("diff_attempt_count") or 0),
        "permuter_attempt_count": int(
            history_summary.get("permuter_attempt_count") or 0
        ),
        "non_improving_scored_attempts": int(
            history_summary.get("non_improving_scored_attempts") or 0
        ),
        "best_objdiff_match_percent": history_summary.get("best_objdiff_match_percent"),
        "best_asm_score": history_summary.get("best_asm_score"),
        "stalled": bool(history_summary.get("stalled")),
        "report_path": report_path,
    }


def collect_reports(match_root: Path) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for path in sorted(match_root.glob("**/diff.json")):
        payload = json.loads(path.read_text(encoding="utf-8"))
        rows.append(row_from_diff_payload(payload, report_path=str(path)))
    rows.sort(key=score_row, reverse=True)
    return rows


def render_tsv(rows: list[dict[str, Any]]) -> str:
    header = [
        "program_path",
        "entry_hex",
        "source_file",
        "source_function",
        "status",
        "match_bucket",
        "objdiff_match_percent",
        "asm_score",
        "asm_score_per_byte",
        "attempt_count",
        "diff_attempt_count",
        "permuter_attempt_count",
        "non_improving_scored_attempts",
        "best_objdiff_match_percent",
        "best_asm_score",
        "stalled",
        "workspace_dir",
    ]
    lines = ["\t".join(header)]
    for row in rows:
        metrics = row.get("match_metrics") or {}
        lines.append(
            "\t".join(
                [
                    str(row.get("program_path") or ""),
                    str(row.get("entry_hex") or ""),
                    str(row.get("source_file") or ""),
                    str(row.get("source_function") or ""),
                    str(row.get("status") or ""),
                    str(row.get("match_bucket") or ""),
                    str(metrics.get("objdiff_match_percent") or ""),
                    str(metrics.get("asm_score") or ""),
                    str(metrics.get("asm_score_per_byte") or ""),
                    str(row.get("attempt_count") or ""),
                    str(row.get("diff_attempt_count") or ""),
                    str(row.get("permuter_attempt_count") or ""),
                    str(row.get("non_improving_scored_attempts") or ""),
                    str(row.get("best_objdiff_match_percent") or ""),
                    str(row.get("best_asm_score") or ""),
                    "yes" if bool(row.get("stalled")) else "no",
                    str(row.get("workspace_dir") or ""),
                ]
            )
        )
    return "\n".join(lines) + "\n"


def _format_metric(value: Any) -> str:
    if value is None:
        return "n/a"
    return f"{float(value):.3f}"


def render_brief_rows(rows: list[dict[str, Any]], *, limit: int = 5) -> list[str]:
    lines: list[str] = []
    for row in rows[: max(limit, 0)]:
        metrics = row.get("match_metrics") or {}
        source_function = (
            row.get("source_function") or row.get("entry_hex") or "<unknown>"
        )
        lines.append(
            f"{source_function}: {row.get('status') or 'unknown'}, "
            f"bucket {row.get('match_bucket') or 'unknown'}, "
            f"match {_format_metric(metrics.get('objdiff_match_percent'))}, "
            f"asm {_format_metric(metrics.get('asm_score'))}, "
            f"attempts {int(row.get('attempt_count') or 0)}, "
            f"stalled {'yes' if bool(row.get('stalled')) else 'no'}"
        )
    return lines


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "report"),
        description="Rank collected match diff reports by current match quality.",
    )
    add_logging_args(parser)
    parser.add_argument("--match-root", type=Path, default=DEFAULT_MATCH_ROOT)
    parser.add_argument(
        "--inventory-db",
        type=Path,
        default=scoreboard_lib.DEFAULT_INVENTORY_DB,
    )
    parser.add_argument(
        "--source-root",
        type=Path,
        default=scoreboard_lib.DEFAULT_SOURCE_ROOT,
    )
    parser.add_argument(
        "--artifact-root",
        type=Path,
        default=scoreboard_lib.workspace_lib.DEFAULT_GHIDRA_ARTIFACT_ROOT,
    )
    parser.add_argument("--output-json", type=Path)
    parser.add_argument("--output-tsv", type=Path)
    parser.add_argument("--refresh-status", action="store_true")
    parser.add_argument("--tracked-output", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_report")
    rows = collect_reports(args.match_root)
    payload = {"match_root": str(args.match_root), "count": len(rows), "rows": rows}
    if args.output_json is not None:
        write_json_output(args.output_json, payload)
    if args.output_tsv is not None:
        write_text_output(args.output_tsv, render_tsv(rows))
    output_parts = []
    if args.output_json is not None:
        output_parts.append(f"json={relative_to_root(args.output_json)}")
    if args.output_tsv is not None:
        output_parts.append(f"tsv={relative_to_root(args.output_tsv)}")
    logger.summary(
        " ".join(
            [
                f"reports={len(rows)}",
                *output_parts,
            ]
        )
    )
    if args.refresh_status:
        status_root = report_refresh.refresh_status_snapshot(
            profile=scoreboard_lib.DEFAULT_PSX_PROFILE,
            tracked_output=bool(args.tracked_output),
            inventory_db=args.inventory_db,
            match_root=args.match_root,
            source_root=args.source_root,
            artifact_root=args.artifact_root,
        )
        logger.item(f"status {relative_to_root(status_root)}")
    return 0
