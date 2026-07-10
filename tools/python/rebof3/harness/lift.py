from __future__ import annotations

from pathlib import Path
import time
from typing import Any

from ..jsonio import read_json, write_json
from ..match.asm_diff import parse_source_address, source_function_name
from . import state
from .config import HarnessConfig
from .context import build_context_header
from .m2c import resolve_function_input, run_m2c_for_target
from .tasks import (
    compact_target_row,
    source_function_payload,
    source_function_target_id,
    target_matches_module,
)
from .workspace import initialize_target_workspace, safe_name


def target_payload(row: dict[str, Any]) -> dict[str, Any]:
    payload = row.get("payload")
    return payload if isinstance(payload, dict) else {}


def target_size(row: dict[str, Any]) -> int | None:
    payload = target_payload(row)
    for key in ("size", "original_size"):
        if payload.get(key) is not None:
            return int(payload[key])
    body_min = str(payload.get("body_min") or "").lower().removeprefix("0x")
    body_max = str(payload.get("body_max") or "").lower().removeprefix("0x")
    if body_min and body_max:
        return int(body_max, 16) - int(body_min, 16) + 1
    return None


def target_source_path(row: dict[str, Any]) -> str | None:
    source_path = target_payload(row).get("source_path")
    return str(source_path) if source_path else None


def target_display_id(row: dict[str, Any]) -> str:
    compact = compact_target_row(row)
    return str(compact.get("alias") or compact["id"])


def candidate_targets(
    rows: list[dict[str, Any]],
    *,
    module: str | None = None,
    min_size: int = 0,
    source: str = "missing",
    largest: bool = True,
    limit: int = 20,
) -> list[dict[str, Any]]:
    selected: list[dict[str, Any]] = []
    for row in rows:
        if row.get("type") != "function":
            continue
        if not target_matches_module(row, module):
            continue
        size = target_size(row)
        if size is None and min_size > 0:
            continue
        if size is not None and size < min_size:
            continue
        has_source = target_source_path(row) is not None
        if source == "missing" and has_source:
            continue
        if source == "existing" and not has_source:
            continue
        selected.append(row)
    if largest:
        selected.sort(
            key=lambda row: (target_size(row) or 0, str(row["id"])),
            reverse=True,
        )
    else:
        selected.sort(key=lambda row: (int(row.get("priority", 100)), str(row["id"])))
    return selected[:limit]


def lift_target(
    config: HarnessConfig,
    conn: Any,
    target: dict[str, Any],
    *,
    extra_args: list[str] | None = None,
) -> dict[str, Any]:
    workspace_path = initialize_target_workspace(config, target)
    state.add_event(
        conn,
        target_id=str(target["id"]),
        kind="workspace",
        message="initialized target workspace",
        path=str(workspace_path),
    )
    context_path = build_context_header(config, target)
    state.add_event(
        conn,
        target_id=str(target["id"]),
        kind="context",
        message="built context header",
        path=str(context_path),
    )
    m2c_payload, m2c_path = run_m2c_for_target(
        config,
        target,
        extra_args=extra_args or [],
    )
    state.add_event(
        conn,
        target_id=str(target["id"]),
        kind="m2c",
        message=str(m2c_payload["status"]),
        path=str(m2c_path),
    )
    outputs = m2c_payload["outputs"]
    return {
        "target_id": str(target["id"]),
        "display_id": target_display_id(target),
        "status": m2c_payload["status"],
        "function": m2c_payload["function"],
        "address": m2c_payload["address"],
        "size": target_size(target),
        "workspace": str(workspace_path),
        "context": str(context_path),
        "asm": outputs["original_asm"],
        "m2c": outputs["m2c_c"],
        "next_action": m2c_payload["next_action"],
    }


def write_lift_batch_report(
    config: HarnessConfig, results: list[dict[str, Any]]
) -> tuple[Path, Path]:
    report_dir = config.out_dir / "function-reports"
    report_dir.mkdir(parents=True, exist_ok=True)
    stamp = time.strftime("%Y%m%d-%H%M%S")
    json_path = report_dir / f"lift-batch-{stamp}.json"
    md_path = report_dir / f"lift-batch-{stamp}.md"
    payload = {
        "schema": "rebof3-simple.harness-lift-batch/v1",
        "generated_at": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
        "results": results,
    }
    write_json(json_path, payload)
    lines = [
        "# Lift Batch Report",
        "",
        "| target | bytes | status | original asm | m2c draft |",
        "|---|---:|---|---|---|",
    ]
    for result in results:
        size = "" if result.get("size") is None else str(result["size"])
        lines.append(
            f"| `{result['display_id']}` | {size} | {result['status']} | "
            f"`{result['asm']}` | `{result['m2c']}` |"
        )
    lines.append("")
    lines.append(f"JSON: `{json_path}`")
    lines.append("")
    md_path.write_text("\n".join(lines), encoding="utf-8")
    return json_path, md_path


def function_report_payload(
    config: HarnessConfig,
    target: dict[str, Any] | None,
    *,
    source: Path | None = None,
) -> dict[str, Any]:
    if source is not None:
        source_path = source if source.is_absolute() else config.root / source
        address = parse_source_address(source_path)
        function = source_function_name(source_path, address)
        source_payload = source_function_payload(config, source_path)
        target_id = source_function_target_id(config, source_path)
        workspace = None
        report_id = target_id or f"source:{source}"
        binary_path = source_payload.get("binary_path")
        load_address = source_payload.get("load_address")
        size = source_payload.get("size")
        source_text = str(source_path.relative_to(config.root))
    else:
        if target is None:
            raise ValueError("function report needs a target or source")
        function_input = resolve_function_input(config, target)
        function = function_input.function_name
        report_id = str(target["id"])
        workspace = config.workspace_dir / safe_name(str(target["id"]))
        binary_path = str(function_input.binary_path)
        load_address = function_input.load_address
        size = function_input.size
        source_text = target_source_path(target)

    asm_diff_summary = config.root / "out/asm-diff" / function / "summary.json"
    asm_diff = read_json(asm_diff_summary) if asm_diff_summary.is_file() else None
    workspace_path = None if workspace is None else Path(workspace)
    original_asm = None
    m2c_draft = None
    context = None
    if workspace_path is not None:
        candidate_original = workspace_path / "original.s"
        candidate_m2c = workspace_path / "func.m2c.c"
        candidate_context = config.context_dir / safe_name(report_id) / "context.h"
        original_asm = str(candidate_original) if candidate_original.is_file() else None
        m2c_draft = str(candidate_m2c) if candidate_m2c.is_file() else None
        context = str(candidate_context) if candidate_context.is_file() else None
    if source_text:
        next_action = "run bin/harness verify function <source>"
    elif m2c_draft:
        next_action = "create src/... source from the m2c draft, then verify"
    else:
        next_action = "run bin/harness lift <target>, then create source and verify"
    return {
        "schema": "rebof3-simple.harness-function-report/v1",
        "target_id": report_id,
        "display_id": report_id if target is None else target_display_id(target),
        "function": function,
        "source": source_text,
        "binary_path": binary_path,
        "load_address": None if load_address is None else f"0x{int(load_address):08x}",
        "size": size,
        "original_asm": original_asm,
        "m2c_draft": m2c_draft,
        "context": context,
        "asm_diff_summary": (
            str(asm_diff_summary) if asm_diff_summary.is_file() else None
        ),
        "asm_diff": asm_diff,
        "next_action": next_action,
    }
