from __future__ import annotations

from ....models.metadata import MetadataSyncPlan


def render_to_report(
    plan: MetadataSyncPlan, *, ghidra_result: dict[str, object] | None = None
) -> str:
    lines = [
        f"mode: {plan.mode}",
        f"db: {plan.db_path}",
        f"rows: {plan.total_rows}",
    ]
    if plan.selector_scope:
        lines.append(f"selectors: {', '.join(plan.selector_scope)}")
    counts: dict[str, int] = {}
    for row in plan.row_plans:
        counts[row.classification] = counts.get(row.classification, 0) + 1
    for key in sorted(counts):
        lines.append(f"{key}: {counts[key]}")
    if ghidra_result is not None:
        failures = []
        for row in ghidra_result.get("rows", []):
            if isinstance(row, dict) and str(row.get("status") or "") != "ok":
                failures.append(row)
        for program in ghidra_result.get("programs", []):
            if not isinstance(program, dict):
                continue
            for row in program.get("rows", []):
                if isinstance(row, dict) and str(row.get("status") or "") != "ok":
                    failures.append(row)
        lines.append(f"ghidra_failures: {len(failures)}")
    return "\n".join(lines)


def render_from_report(report: dict[str, object]) -> str:
    lines = [
        "direction: from",
        f"db: {report.get('db')}",
        f"kind: {report.get('kind')}",
        f"row_count: {report.get('row_count')}",
        f"canonical_program_count: {report.get('canonical_program_count')}",
        f"ghidra_program_count: {report.get('ghidra_program_count')}",
    ]
    selectors = report.get("selectors") or []
    if selectors:
        lines.append(f"selectors: {', '.join(str(item) for item in selectors)}")
    persisted = report.get("persisted") or {}
    if isinstance(persisted, dict):
        for key in ("program_rows", "function_rows", "metadata_rows"):
            if key in persisted:
                lines.append(f"{key}: {persisted[key]}")
    return "\n".join(lines)
