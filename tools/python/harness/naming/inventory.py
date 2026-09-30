"""Blocked-row inventory building and atomic report-set publication."""

from __future__ import annotations

import hashlib
import json
import shutil
import tempfile
from collections.abc import Callable
from pathlib import Path
from typing import Any

from harness.naming.debt import address_of, collect_naming_debt
from harness.naming.campaign import campaign_report_filename
from harness.naming.context import SCHEMA_V3, TargetContext, inventory_expected
from harness.naming.editing import report_mutation
from harness.naming.history import require_replaceable_set


def expected_inventories(
    root: Path, manifests: dict[str, Any]
) -> dict[str, set[tuple[str, str]]]:
    """Collect naming debt once and partition it by target."""

    expected: dict[str, set[tuple[str, str]]] = {target: set() for target in manifests}
    debt = collect_naming_debt(root, manifests)
    for kind, values in (
        ("function", debt.raw_functions),
        ("data", debt.raw_data),
    ):
        for value in values:
            target, name = value.split(":", 1)
            expected[target].add((kind, name))
    return expected


def _report_set_digest(output: Path) -> str | None:
    if not output.exists():
        return None
    digest = hashlib.sha256()
    for path in sorted(item for item in output.rglob("*") if item.is_file()):
        digest.update(path.relative_to(output).as_posix().encode())
        digest.update(b"\0")
        digest.update(path.read_bytes())
    return digest.hexdigest()


def publish_reports(
    staging: Path, output: Path, *, expected_digest: str | None
) -> None:
    """Atomically replace the report set only from its expected state."""

    output.parent.mkdir(parents=True, exist_ok=True)
    with report_mutation(output):
        require_replaceable_set(output)
        if _report_set_digest(output) != expected_digest:
            raise ValueError(
                "report set changed concurrently; stale publication rejected"
            )
        backup = output.parent / f".{output.name}.backup"
        rollback = output.parent / f".{output.name}.rollback"
        if rollback.exists():
            raise FileExistsError(f"unresolved report rollback exists: {rollback}")
        moved = False
        recovery = rollback if backup.exists() else backup
        try:
            if output.exists():
                output.replace(recovery)
                moved = True
            staging.replace(output)
        except BaseException:
            if moved and not output.exists():
                recovery.replace(output)
            raise
        else:
            if rollback.exists():
                shutil.rmtree(rollback)
            if backup.exists():
                shutil.rmtree(backup)


def _blocked_row(
    root: Path, target: str, ctx: TargetContext, kind: str, name: str
) -> dict[str, Any]:
    address = address_of(name)
    selector = f"{target}@0x{address:08X}"
    outside = not ctx.manifest.load_address <= address < ctx.payload_end
    if kind == "function":
        profiles = (
            (
                (
                    "selected_call",
                    "selected caller/callsite instructions and arguments",
                ),
                (
                    "owner_resolution",
                    "runtime owner, range, bytes, boundary, and composition",
                ),
                ("owner_body", "owner body effects, callees, and consumers"),
                (
                    "one_level_beyond",
                    "one independent semantic level beyond the selected call",
                ),
            )
            if outside
            else (
                ("selected_range", "reviewed half-open range and original bytes"),
                (
                    "selected_call",
                    "callsite instructions, arguments, guards, and result use",
                ),
                (
                    "one_level_beyond",
                    "caller/callee/table consumer one semantic level beyond",
                ),
            )
        )
    else:
        profiles = (
            (
                (
                    "selected_access",
                    "selected access instructions and access width",
                ),
                (
                    "owner_resolution",
                    "map/Splat/load-range owner and original bytes",
                ),
                ("owner_data", "initializer, consumers, content class, and layout"),
                (
                    "storage_class",
                    "canonical storage class and extent",
                ),
                (
                    "one_level_beyond",
                    "an independent initializer or consumer",
                ),
            )
            if outside
            else (
                ("selected_range", "reviewed range and original bytes"),
                (
                    "selected_access",
                    "access instructions and access width",
                ),
                (
                    "storage_class",
                    "canonical storage class and extent",
                ),
                (
                    "one_level_beyond",
                    "an independent initializer or consumer",
                ),
            )
        )
    rungs: dict[str, Any] = {}
    for rung, missing in profiles:
        next_command = (
            f"bin/harness analysis query --json owners {selector}"
            if rung == "owner_resolution"
            else f"bin/harness analysis query --json {'describe' if rung in {'selected_range', 'storage_class'} else 'xrefs'} {selector}"
        )
        rungs[rung] = {
            "status": "open",
            "next_command": next_command,
            "observations": [
                {
                    "id": f"{name}.{rung}.gap",
                    "text": f"Evidence gap: {missing}; this initializer records no semantic conclusion.",
                }
            ],
            "authority": "target manifest, reviewed Splat, original image, and fresh reverse index",
        }
    required = [{**item, "status": "open"} for item in ctx.required_work(kind, name)]
    partial = False
    if kind == "function":
        partial = ctx.partial(name)
        if partial:
            rungs["partial_baseline"] = {
                "status": "open",
                "next_command": f"bin/harness lift asm-diff {selector} --json --detail full; bin/harness lift byte-match {selector} --json",
                "observations": [
                    {
                        "id": f"{name}.partial_baseline.gap",
                        "text": "Evidence Gap: live partial percentage, sizes, first mismatch, residual, and original-byte verification are not yet recorded.",
                    }
                ],
                "authority": "live asm-diff, byte-match, source progress metadata, and original bytes",
            }
    next_command = (
        f"bin/harness analysis query --json owners {selector}"
        if outside and kind == "function"
        else f"bin/harness analysis query --json xrefs {selector}; bin/harness analysis rz-project query {target} -c 'axt @ 0x{address:08X}'"
    )
    return {
        "kind": kind,
        "name": name,
        "initializer_state": "bof3.naming-audit-initializer/v1",
        "rung_status": "blocked",
        "outside_payload": outside,
        "partial_used": partial,
        "rungs": rungs,
        "required_work": required,
        "optional_work": [],
        "interpretation": "No semantic name is accepted until the failed typed rungs and generated required work are closed.",
        "authority": "target manifest, target-local map, reviewed Splat, original image, and fresh reverse index",
        "smallest_repair": next_command,
        "missing_fact": "; ".join(missing for _, missing in profiles),
        "ceiling_next_command": next_command,
    }


def initialize_with_context(
    root: Path,
    target: str,
    ctx: TargetContext,
    manifests: dict[str, Any],
    *,
    expected: set[tuple[str, str]] | None = None,
) -> dict[str, Any]:
    inventory = (
        inventory_expected(root, target, manifests) if expected is None else expected
    )
    rows = [
        _blocked_row(root, target, ctx, kind, name) for kind, name in sorted(inventory)
    ]
    return {"schema": SCHEMA_V3, "target": target, "complete": not rows, "rows": rows}


def _initialize_all(
    root: Path,
    output: Path,
    *,
    load_manifests: Callable[[Path], dict[str, Any]],
    connect: Callable[..., Any],
    build_context: Callable[..., TargetContext],
    validate_report: Callable[..., dict[str, Any]],
) -> dict[str, Any]:
    """Build one validated report set using collaborators supplied by its owner."""

    require_replaceable_set(output)
    manifests = load_manifests(root)
    expected_by_target = expected_inventories(root, manifests)
    output.parent.mkdir(parents=True, exist_ok=True)
    initial_digest = _report_set_digest(output)
    staging = Path(
        tempfile.mkdtemp(prefix=f".{output.name}.", suffix=".tmp", dir=output.parent)
    )
    connection = None
    try:
        connection = connect(root, manifests=manifests)
        targets = []
        total_rows = 0
        for target in sorted(manifests):
            ctx = build_context(
                root,
                target,
                bulk_work=True,
                manifests=manifests,
                connection=connection,
            )
            expected = expected_by_target[target]
            report = initialize_with_context(
                root, target, ctx, manifests, expected=expected
            )
            result = validate_report(root, target, report, ctx, expected=expected)
            seen = [(str(row["kind"]), str(row["name"])) for row in report["rows"]]
            if len(seen) != len(set(seen)) or set(seen) != expected:
                raise ValueError(f"all-target accounting mismatch for {target}")
            name = campaign_report_filename(target)
            _write_json(staging / name, report)
            total_rows += len(seen)
            targets.append(
                {
                    "target": target,
                    "rows": len(seen),
                    "complete": result["complete"],
                    "report": (output / name).as_posix(),
                }
            )
        summary = {
            "schema": "bof3.naming-audit-account/v1",
            "targets": targets,
            "target_count": len(targets),
            "row_count": total_rows,
        }
        _write_json(staging / "summary.json", summary)
        publish_reports(staging, output, expected_digest=initial_digest)
        return summary
    finally:
        if connection is not None:
            connection.close()
        shutil.rmtree(staging, ignore_errors=True)


def _write_json(path: Path, payload: dict[str, Any]) -> None:
    path.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )


__all__ = ["expected_inventories", "initialize_with_context", "publish_reports"]
