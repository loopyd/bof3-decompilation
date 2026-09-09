"""Bounded receipt-backed naming evidence runner."""

from __future__ import annotations

import hashlib
import json
import time
from collections.abc import Sequence
from pathlib import Path
from typing import Any

from harness.analysis.index import index_path
from harness.domain.ids import normalize_target_id, parse_function_id
from harness.naming.capabilities import PRODUCTION_EXACT_CAPABILITIES
from harness.naming.client import (
    IndexWorker,
    _validate_report_shape,
    _validate_run_arguments,
)
from harness.naming.collection import (
    _commit_row,
    _receipts_for,
    _select_rows,
    evidence_namespace,
    terminalize_report,
    write_failed_receipt,
)
from harness.naming.evidence import _RUNNER_TOKEN, _analyze_validated_records
from harness.naming.journal import (
    JournalInputs,
    journal_is_fresh,
    load_journal,
    operation_digest,
    rotate_journal,
)
from harness.naming.journal import write_manifest as write_journal_manifest
from harness.naming.native import NativeByteOps
from harness.naming.plan import (
    collection_row,
    indexed_operation_plan,
    semantic_operation_plan,
    validate_report_operations,
)
from harness.naming.proposal import canonical_report_path
from harness.naming.semantics import RizinSession
from harness.naming.state import (
    evidence_dir,
    manifest_path,
)
from harness.naming.telemetry import RunTelemetry, artifact_bytes, write_telemetry

SHARD_ROWS = 10
DEFAULT_CONCURRENCY = 1
MAX_CONCURRENCY = 1
DEFAULT_DEADLINE = 120
SHARD_WALL_CLOCK = 600
DEFAULT_ROWS_BUDGET = 128


def _row_key(row: dict[str, Any]) -> str:
    return f"{row.get('kind')}:{row.get('name')}"


def _report_digest(report: Path) -> str:
    return hashlib.sha256(report.read_bytes()).hexdigest()


def _operations(
    raw: list[dict[str, Any]], target: str, row: dict[str, Any]
) -> list[tuple[dict[str, Any], str, str, Any, bool]]:
    result = []
    for item in raw:
        if (
            not isinstance(item, dict)
            or item.get("operation")
            not in {
                "calls",
                "access",
                "owner",
                "describe",
                "xrefs",
                "data_dispatch_consumer",
            }
            or (
                item.get("operation") == "data_dispatch_consumer"
                and not isinstance(item.get("payload"), dict)
            )
            or (
                item.get("operation") != "data_dispatch_consumer"
                and not isinstance(item.get("payload"), list)
            )
            or not isinstance(item.get("supplemental"), bool)
            or (
                item.get("supplemental")
                and item.get("role")
                not in {
                    "selected_data_describe",
                    "access_function_describe",
                    "access_function_xrefs",
                    "data_dispatch_consumer",
                }
            )
            or (not item.get("supplemental") and item.get("role") is not None)
        ):
            raise ValueError("invalid indexed operation response")
        try:
            selector_target = parse_function_id(str(item["selector"])).target.value
        except ValueError as error:
            raise ValueError(
                "indexed operation target does not match report"
            ) from error
        item_id = str(item.get("item", {}).get("id", ""))
        external_owner = (
            item["operation"] == "owner"
            and not item["supplemental"]
            and item_id.startswith("owner:")
            and item_id.split(":", 1)[1] == item["selector"]
            and row.get("outside_payload") is True
        )
        if selector_target != target and not external_owner:
            raise ValueError("indexed operation target does not match report")
        record = dict(item["item"])
        if item.get("role") is not None:
            record["role"] = item["role"]
        result.append(
            (
                record,
                item["operation"],
                item["selector"],
                item["payload"],
                item["supplemental"],
            )
        )
    return result


def _semantic_succeeded(
    root: Path, row: dict[str, Any], operation: dict[str, Any], result: Any
) -> bool:
    """Accept exit 1 only when strict JSON proves this row's partial lift."""

    if result.killed:
        return False
    if result.exit == 0:
        return True
    if row.get("partial_used") is not True or operation["kind"] not in {
        "asm-diff",
        "byte-match",
    }:
        return False
    try:
        from harness.common.checks import validate_partial_evidence
        from harness.domain.registry import resolve_function

        resolved = resolve_function(root, operation["target"])
        if resolved.source is None or resolved.compiled_symbol is None:
            return False
        validate_partial_evidence(
            root,
            tool=operation["kind"],
            target=resolved.target.id.value,
            selector=operation["target"],
            function=resolved.compiled_symbol,
            source=resolved.source.relative_to(root).as_posix(),
            exit_code=result.exit,
            output=result.raw,
        )
    except (OSError, ValueError):
        return False
    return True


def run_evidence(
    root: Path,
    target: str,
    report: Path,
    *,
    rows: Sequence[str] | None = None,
    all_rows: bool = False,
    shard: int = SHARD_ROWS,
    concurrency: int = DEFAULT_CONCURRENCY,
    deadline: int = DEFAULT_DEADLINE,
    rows_budget: int = DEFAULT_ROWS_BUDGET,
    rizin_executable: str | Path | None = None,
    terminalize: bool = False,
    registry=PRODUCTION_EXACT_CAPABILITIES,
) -> dict[str, Any]:
    telemetry = RunTelemetry()
    report = canonical_report_path(root, report)
    root = root.resolve()
    target = normalize_target_id(target).value
    if not report.is_file():
        raise ValueError(f"report must be an existing repository file: {report}")
    report_payload = json.loads(report.read_text(encoding="utf-8"))
    report_rows = _validate_report_shape(report_payload, target)
    validate_report_operations(report_rows, target)
    _validate_run_arguments(
        rows, all_rows, shard, concurrency, deadline, rows_budget, report_rows
    )
    index = index_path(root)
    if not index.is_file():
        raise ValueError("reverse index is missing; run bin/index --recover")
    inputs = JournalInputs(
        target,
        _report_digest(report),
        hashlib.sha256(index.read_bytes()).hexdigest(),
        operation_digest(target, report_payload),
        {
            path.relative_to(root).as_posix(): hashlib.sha256(
                path.read_bytes()
            ).hexdigest()
            for path in sorted(root.glob("config/targets/**/target.toml"))
        },
    )
    loaded, corrupt = load_journal(root, report, target)
    fresh = bool(loaded) and journal_is_fresh(
        root, report, target, inputs, registry=registry
    )
    telemetry.rows_stale = len(loaded) if loaded and not fresh else 0
    reusable = loaded if fresh else {}
    if corrupt and not loaded:
        rotate_journal(root, report, target, "corrupt")
    elif loaded and not fresh:
        rotate_journal(root, report, target, "stale")
    namespace = evidence_namespace(root, report, target)
    selected = _select_rows(
        report_rows, rows=rows, all_rows=all_rows, shard=shard, checkpoint=reusable
    )
    checkpoint = dict(reusable)
    telemetry.rows_planned = len(selected)
    telemetry.rows_skipped = len(reusable)
    telemetry.phase("prepare")
    shard_end = time.monotonic() + SHARD_WALL_CLOCK
    worker = IndexWorker(
        root,
        target,
        report_payload,
        min(deadline, max(0, shard_end - time.monotonic())),
    )
    telemetry.external_processes += 1
    telemetry.index_opens += 1
    rizin = RizinSession(
        root, target, command_timeout=deadline, executable=rizin_executable
    )
    byte_ops = NativeByteOps(root, deadline)
    executed = 0
    errors: list[str] = []
    try:
        worker.receive(min(deadline, max(0, shard_end - time.monotonic())))
        for request_id, report_row in enumerate(selected):
            key = _row_key(report_row)
            row = collection_row(report_row, target, registry=registry)
            remaining = shard_end - time.monotonic()
            if remaining <= 0:
                worker.kill()
                errors.append(f"{key}: shard wall clock exceeded")
                break
            try:
                semantic: list[dict[str, Any]] = []
                for op in semantic_operation_plan(row, target):
                    if op["target"] != target and not op["target"].startswith(
                        f"{target}@"
                    ):
                        raise ValueError(
                            f"semantic target does not match run target: {op['target']}"
                        )
                    remaining = shard_end - time.monotonic()
                    if remaining <= 0:
                        raise TimeoutError("shard wall clock exceeded")
                    instruction = {}
                    if op["kind"] == "instructions-v1":
                        from harness.naming.instructions import (
                            resolve_instructions,
                            validate_instructions,
                        )

                        binding, original = resolve_instructions(root, op["target"])
                        rizin.queue(binding["rizin"], op["target"])
                        result = rizin.run_queued(min(deadline, remaining))[-1]
                        if result.killed or result.exit != 0:
                            raise ValueError("instruction operation failed")
                        validate_instructions(result.raw, binding, original)
                        instruction = {
                            "instruction_id": op["command"],
                            "binding": binding,
                        }
                        op = {**op, "command": binding["command"]}
                    elif op["kind"] == "rz-project":
                        rizin.queue(op["rizin"], op["target"])
                        result = rizin.run_queued(min(deadline, remaining))[-1]
                    else:
                        result = byte_ops.run(op, min(deadline, remaining))
                    semantic.append(
                        {
                            **instruction,
                            "command": op["command"],
                            "selector": op["target"],
                            "exit": result.exit,
                            "killed": result.killed,
                            "output": result.output,
                            "raw": result.raw,
                            "stderr": result.stderr,
                            "semantic_success": _semantic_succeeded(
                                root, row, op, result
                            ),
                        }
                    )
                    if not semantic[-1]["semantic_success"]:
                        raise RuntimeError(
                            f"semantic operation failed: {op['command']}"
                        )
                operations = _operations(
                    worker.request(
                        request_id,
                        row,
                        min(deadline, max(0, shard_end - time.monotonic())),
                    ),
                    target,
                    row,
                )
                expected = indexed_operation_plan(row, target, registry=registry)
                expected_by_id = {str(item["id"]): item for item in expected}
                actual = [
                    {
                        "id": str(operation[0].get("id")),
                        "operation": operation[1],
                        "selector": operation[2],
                        "command": expected_by_id.get(
                            str(operation[0].get("id")), {}
                        ).get("command", ""),
                        "supplemental": operation[4],
                        **(
                            {"role": operation[0]["role"]}
                            if operation[0].get("role")
                            else {}
                        ),
                    }
                    for operation in operations
                ]
                if actual != expected:
                    raise ValueError(
                        "indexed operations do not match the exact row plan"
                    )
                operation_records = tuple(
                    {
                        "item": operation[0],
                        "operation": operation[1],
                        "selector": operation[2],
                        "payload": operation[3],
                        "supplemental": operation[4],
                        **(
                            {"role": operation[0]["role"]}
                            if operation[0].get("role")
                            else {}
                        ),
                    }
                    for operation in operations
                )
                derived = _analyze_validated_records(
                    _RUNNER_TOKEN,
                    target=target,
                    report_digest=inputs.report_digest,
                    row=row,
                    operations=operation_records,
                    registry=registry,
                )
                receipts = _receipts_for(namespace, target, row, operations, semantic)
                checkpoint[key] = _commit_row(
                    namespace,
                    report,
                    target,
                    report_row,
                    operations,
                    receipts,
                    semantic,
                    derived,
                )
                write_journal_manifest(root, report, target, inputs, checkpoint)
                executed += 1
                telemetry.rows_completed += 1
            except Exception as error:
                worker.kill()
                write_failed_receipt(namespace, target, row, error)
                errors.append(f"{key}: {error}")
                break
    finally:
        rizin.close()
        worker.close()
    telemetry.phase("collect")
    entries = load_journal(root, report, target)[0]
    manifest = write_journal_manifest(root, report, target, inputs, entries)
    telemetry.receipt_bytes, telemetry.stdout_bytes = artifact_bytes(
        root, namespace, entries
    )
    telemetry_path = evidence_dir(root, report, target) / "telemetry.json"
    summary = telemetry.payload(manifest["sha256"])
    write_telemetry(telemetry_path, summary)
    terminalized = (
        terminalize
        and not errors
        and terminalize_report(root, report, target, report_rows, entries)
    )
    return {
        "schema": "bof3.naming-evidence-run/v2",
        "target": target,
        "report": report.relative_to(root).as_posix(),
        "rows": [_row_key(row) for row in selected],
        "executed": executed,
        "resumed": len(checkpoint),
        "errors": errors,
        "terminalized": terminalized,
        "manifest": namespace.record_path(manifest_path(root, report, target)),
        "manifest_sha256": manifest["sha256"],
        "telemetry": namespace.record_path(telemetry_path),
        "telemetry_summary": summary,
    }


__all__ = [
    "DEFAULT_CONCURRENCY",
    "DEFAULT_DEADLINE",
    "DEFAULT_ROWS_BUDGET",
    "MAX_CONCURRENCY",
    "SHARD_ROWS",
    "SHARD_WALL_CLOCK",
    "run_evidence",
]
