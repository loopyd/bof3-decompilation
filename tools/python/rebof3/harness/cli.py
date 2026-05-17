from __future__ import annotations

import argparse
import getpass
import json
from pathlib import Path
import sys
from typing import Any

from ..commands._common import run_main
from ..core.process import run_process
from ..jsonio import write_json
from ..match.asm_diff import (
    AsmDiffRequest,
    parse_int,
    run_asm_diff_one,
)
from . import state
from .binary import build_binary_diff, resolve_binary_pair
from .catalog import artifact_records, emi_target_records, write_harness_catalog
from .config import HarnessConfig, load_harness_config
from .context import build_context_header, ensure_common_context
from .dashboard import write_dashboard
from .ghidra import build_ghidra_coverage
from .lift import (
    candidate_targets,
    function_report_payload,
    lift_target,
    target_display_id,
    target_payload,
    target_size,
    target_source_path,
    write_lift_batch_report,
)
from .maps import write_binary_map
from .m2c import run_m2c_for_target
from .report import build_report, choose_resume_action, write_report
from .tasks import (
    compact_target_row,
    function_target_records,
    migration_target_records,
    resolve_function_target_alias,
    source_function_payload,
    source_function_target_id,
    target_matches_module,
)
from .tools import run_configured_command, tool_health
from .workspace import initialize_target_workspace


def _config(args: argparse.Namespace) -> HarnessConfig:
    return load_harness_config(args.config)


def emit(
    args: argparse.Namespace, payload: dict[str, Any], *, lines: list[str]
) -> None:
    if bool(getattr(args, "json", False)):
        print(json.dumps(payload, indent=2, sort_keys=True))
        return
    for line in lines:
        print(line)


def ensure_dirs(config: HarnessConfig) -> None:
    for path in (
        config.out_dir,
        config.workspace_dir,
        config.context_dir,
        config.dashboard_dir,
    ):
        path.mkdir(parents=True, exist_ok=True)


def _target_row(conn: Any, target_id: str) -> dict[str, Any] | None:
    target = state.target_row(conn, target_id)
    if target is not None:
        return target
    rows = state.list_targets(conn, limit=100000, target_type="function")
    return resolve_function_target_alias(rows, target_id)


def run_setup(args: argparse.Namespace) -> int:
    config = _config(args)
    ensure_dirs(config)
    with state.state_db(config.database):
        pass
    common = ensure_common_context(config)
    if bool(args.run_existing):
        run_configured_command(config, "setup")
    payload = {
        "database": str(config.database),
        "out_dir": str(config.out_dir),
        "common_context": str(common),
        "next": "bin/harness catalog",
    }
    emit(
        args,
        payload,
        lines=[
            f"harness-db: {config.database}",
            f"harness-out: {config.out_dir}",
            f"context: {common}",
            "next: bin/harness catalog",
        ],
    )
    return 0


def run_status(args: argparse.Namespace) -> int:
    config = _config(args)
    ensure_dirs(config)
    with state.state_db(config.database) as conn:
        payload = build_report(config, conn)
    lines = [
        f"harness-db: {config.database}",
        "targets: "
        + ", ".join(
            f"{key}={value}" for key, value in payload["counts_by_status"].items()
        )
        if payload["counts_by_status"]
        else "targets: none",
        f"active-claims: {len(payload['active_claims'])}",
    ]
    emit(args, payload, lines=lines)
    return 0


def run_catalog(args: argparse.Namespace) -> int:
    config = _config(args)
    ensure_dirs(config)
    catalog, catalog_path = write_harness_catalog(config)
    records = emi_target_records(catalog) + artifact_records(config)
    with state.state_db(config.database) as conn:
        count = state.upsert_targets(conn, records)
        state.add_event(
            conn,
            target_id=None,
            kind="catalog",
            message=f"cataloged {count} targets",
            path=str(catalog_path),
        )
    payload = {
        "catalog": str(catalog_path),
        "entry_count": catalog["entry_count"],
        "target_count": count,
        "emi_kind_counts": catalog["emi_kind_counts"],
        "next": "bin/harness analyze",
    }
    emit(
        args,
        payload,
        lines=[
            f"catalog: {catalog_path}",
            f"entries: {catalog['entry_count']}",
            f"targets: {count}",
            "next: bin/harness analyze",
        ],
    )
    return 0


def run_analyze(args: argparse.Namespace) -> int:
    config = _config(args)
    ensure_dirs(config)
    if bool(args.run_existing):
        run_configured_command(config, "analyze")
    records = function_target_records(config)
    with state.state_db(config.database) as conn:
        count = state.upsert_targets(conn, records)
        pruned = state.prune_stale_targets(
            conn,
            target_type="function",
            keep_ids=[str(record["id"]) for record in records],
        )
        state.add_event(
            conn,
            target_id=None,
            kind="analyze",
            message=f"loaded {count} function targets; pruned {pruned}",
        )
    payload = {
        "function_index": str(config.function_index),
        "target_count": count,
        "pruned_target_count": pruned,
        "next": "bin/harness queue --limit 10",
    }
    emit(
        args,
        payload,
        lines=[
            f"function-index: {config.function_index}",
            f"function-targets: {count}",
            f"pruned-targets: {pruned}",
            "next: bin/harness queue --limit 10",
        ],
    )
    return 0


def run_split(args: argparse.Namespace) -> int:
    config = _config(args)
    records = migration_target_records(config)
    if bool(args.create_source_dirs):
        for target in config.migration_targets:
            target.source_dir.mkdir(parents=True, exist_ok=True)
    with state.state_db(config.database) as conn:
        count = state.upsert_targets(conn, records)
        state.add_event(
            conn,
            target_id=None,
            kind="split",
            message=f"recorded {count} migration targets",
        )
    payload = {
        "target_count": count,
        "created_source_dirs": bool(args.create_source_dirs),
        "next": "bin/harness queue --type migration",
    }
    emit(
        args,
        payload,
        lines=[
            f"migration-targets: {count}",
            f"created-source-dirs: {bool(args.create_source_dirs)}",
            "next: bin/harness queue --type migration",
        ],
    )
    return 0


def run_queue(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        scan_limit = 100000 if args.module else args.limit
        rows = state.list_targets(
            conn,
            limit=scan_limit,
            status=args.status,
            target_type=args.type,
        )
        rows = [row for row in rows if target_matches_module(row, args.module)][
            : args.limit
        ]
        payload = {"targets": [compact_target_row(row) for row in rows]}
    lines = [
        f"{'priority':>8}  {'status':<9}  {'type':<10}  target",
    ]
    for row in payload["targets"]:
        display_id = row.get("alias") or row["id"]
        lines.append(
            f"{row['priority']:>8}  {row['status']:<9}  {row['type']:<10}  {display_id}  {row['summary']}"
        )
    emit(args, payload, lines=lines)
    return 0


def run_candidates(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        rows = state.list_targets(
            conn,
            limit=100000,
            status=args.status,
            target_type="function",
        )
    targets = candidate_targets(
        rows,
        module=args.module,
        min_size=args.min_size,
        source=args.source,
        largest=bool(args.largest),
        limit=args.limit,
    )
    payload = {"targets": [compact_target_row(row) | {"size": target_size(row)} for row in targets]}
    lines = [f"{'bytes':>8}  {'status':<9}  {'source':<8}  target"]
    for row in targets:
        source_state = "source" if target_source_path(row) else "missing"
        lines.append(
            f"{(target_size(row) or 0):>8}  {row['status']:<9}  "
            f"{source_state:<8}  {target_display_id(row)}  {row['summary']}"
        )
    emit(args, payload, lines=lines)
    return 0


def run_claim(args: argparse.Namespace) -> int:
    config = _config(args)
    owner = args.owner or getpass.getuser()
    with state.state_db(config.database) as conn:
        if args.module and not args.target_id:
            target = None
            rows = state.list_targets(
                conn,
                limit=100000,
                status=args.status,
                target_type=args.type,
            )
            for row in rows:
                if not target_matches_module(row, args.module):
                    continue
                target = state.claim_target(
                    conn,
                    owner=owner,
                    target_id=str(row["id"]),
                    status=args.status,
                    target_type=args.type,
                    lease_minutes=args.lease_minutes,
                )
                if target is not None:
                    break
        else:
            target = state.claim_target(
                conn,
                owner=owner,
                target_id=args.target_id,
                status=args.status,
                target_type=args.type,
                lease_minutes=args.lease_minutes,
            )
    if target is None:
        payload = {"claimed": None, "next": "bin/harness resume"}
        emit(args, payload, lines=["claim: none", "next: bin/harness resume"])
        return 1
    payload = {
        "claimed": compact_target_row(target),
        "owner": owner,
        "next": f"bin/harness target init {target['id']}",
    }
    emit(
        args,
        payload,
        lines=[
            f"claimed: {target['id']}",
            f"owner: {owner}",
            f"next: bin/harness target init {target['id']}",
        ],
    )
    return 0


def run_lift(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        target = _target_row(conn, args.target_id)
        if target is None:
            raise LookupError(f"unknown target: {args.target_id}")
        result = lift_target(
            config,
            conn,
            target,
            extra_args=list(args.extra_args or []),
        )
    emit(
        args,
        result,
        lines=[
            f"status: {result['status']}",
            f"target: {result['display_id']}",
            f"function: {result['function']} {result['address']}",
            f"workspace: {result['workspace']}",
            f"context: {result['context']}",
            f"asm: {result['asm']}",
            f"m2c: {result['m2c']}",
            f"next: {result['next_action']}",
        ],
    )
    return 0 if result["status"] == "ok" else 1


def run_lift_batch(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        rows = state.list_targets(
            conn,
            limit=100000,
            status=args.status,
            target_type="function",
        )
        targets = candidate_targets(
            rows,
            module=args.module,
            min_size=args.min_size,
            source=args.source,
            largest=bool(args.largest),
            limit=args.limit,
        )
        results = [
            lift_target(config, conn, target, extra_args=list(args.extra_args or []))
            for target in targets
        ]
    json_path, md_path = write_lift_batch_report(config, results)
    payload = {"count": len(results), "report_json": str(json_path), "report_md": str(md_path), "results": results}
    lines = [
        f"lifted: {len(results)}",
        f"report-json: {json_path}",
        f"report-md: {md_path}",
    ]
    for result in results:
        lines.append(
            f"{result['status']:<8} {result['display_id']} {result['function']} {result['m2c']}"
        )
    emit(args, payload, lines=lines)
    return 0 if all(result["status"] == "ok" for result in results) else 1


def run_target_show(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        target = _target_row(conn, args.target_id)
        if target is None:
            raise LookupError(f"unknown target: {args.target_id}")
        claim = state.active_claim_for_target(conn, str(target["id"]))
    payload = {"active_claim": claim, "target": target}
    lines = [
        f"target: {target['id']}",
        f"type: {target['type']}",
        f"status: {target['status']}",
        f"priority: {target['priority']}",
        f"summary: {target['summary']}",
        f"source: {target.get('source_hint') or ''}",
        f"program: {target.get('program_path') or ''}",
        f"entry: {target.get('entry_hex') or ''}",
        "claim: "
        + (
            "none" if claim is None else f"{claim['owner']} until {claim['expires_at']}"
        ),
    ]
    emit(args, payload, lines=lines)
    return 0


def run_target_init(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        target = _target_row(conn, args.target_id)
        if target is None:
            raise LookupError(f"unknown target: {args.target_id}")
        workspace_path = initialize_target_workspace(config, target)
        state.add_event(
            conn,
            target_id=str(target["id"]),
            kind="workspace",
            message="initialized target workspace",
            path=str(workspace_path),
        )
    payload = {
        "workspace": str(workspace_path),
        "next": f"bin/harness context build {target['id']}",
    }
    emit(
        args,
        payload,
        lines=[
            f"workspace: {workspace_path}",
            f"next: bin/harness context build {target['id']}",
        ],
    )
    return 0


def run_lock_acquire(args: argparse.Namespace) -> int:
    config = _config(args)
    owner = args.owner or getpass.getuser()
    with state.state_db(config.database) as conn:
        acquired = state.acquire_lock(
            conn,
            name=args.name,
            owner=owner,
            lease_minutes=args.lease_minutes,
        )
        lock = state.lock_row(conn, args.name)
    payload = {"acquired": acquired, "lock": lock}
    if acquired:
        emit(args, payload, lines=[f"lock: {args.name}", f"owner: {owner}"])
        return 0
    current_owner = "unknown" if lock is None else str(lock["owner"])
    emit(
        args,
        payload,
        lines=[f"lock: {args.name}", "status: held", f"owner: {current_owner}"],
    )
    return 1


def run_lock_release(args: argparse.Namespace) -> int:
    config = _config(args)
    owner = args.owner or getpass.getuser()
    with state.state_db(config.database) as conn:
        state.release_lock(conn, name=args.name, owner=owner)
    emit(
        args, {"released": args.name, "owner": owner}, lines=[f"released: {args.name}"]
    )
    return 0


def run_context_build(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        target = _target_row(conn, args.target_id)
        if target is None:
            raise LookupError(f"unknown target: {args.target_id}")
        path = build_context_header(config, target)
        state.add_event(
            conn,
            target_id=str(target["id"]),
            kind="context",
            message="built context header",
            path=str(path),
        )
    payload = {"context": str(path), "next": f"bin/harness diff {target['id']}"}
    emit(
        args,
        payload,
        lines=[f"context: {path}", f"next: bin/harness diff {target['id']}"],
    )
    return 0


def run_m2c(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        target = _target_row(conn, args.target_id)
        if target is None:
            raise LookupError(f"unknown target: {args.target_id}")
        payload, path = run_m2c_for_target(
            config,
            target,
            extra_args=list(args.extra_args or []),
        )
        state.add_event(
            conn,
            target_id=str(target["id"]),
            kind="m2c",
            message=str(payload["status"]),
            path=str(path),
        )
    outputs = payload["outputs"]
    emit(
        args,
        payload,
        lines=[
            f"status: {payload['status']}",
            f"target: {target['id']}",
            f"function: {payload['function']} {payload['address']}",
            f"asm: {outputs['original_asm']}",
            f"m2c: {outputs['m2c_c']}",
            f"context: {outputs['context']}",
            f"next: {payload['next_action']}",
        ],
    )
    return 0 if payload["status"] == "ok" else 1


def run_diff(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        target = _target_row(conn, args.target_id)
        if target is None:
            raise LookupError(f"unknown target: {args.target_id}")
        target_id = str(target["id"])
        payload = (
            target.get("payload") if isinstance(target.get("payload"), dict) else {}
        )
        source_path = payload.get("source_path")
        if target.get("type") == "function" and source_path:
            result = run_asm_diff_one(
                AsmDiffRequest(
                    source_path=config.root / str(source_path),
                    address=parse_int(str(target["entry_hex"]))
                    if target.get("entry_hex")
                    else None,
                    size=payload.get("size"),
                    binary_path=config.root / str(payload["binary_path"])
                    if payload.get("binary_path")
                    else None,
                    load_address=payload.get("load_address"),
                )
            )
            outputs = result["outputs"]
            state.add_event(
                conn,
                target_id=target_id,
                kind="function-diff",
                message=str(result["status"]),
                path=str(outputs["summary_json"]),
            )
            emit(
                args,
                {**result, "target_id": target_id},
                lines=[
                    f"status: {result['status']}",
                    f"target: {target_id}",
                    f"function: {result['function']} {result['address']}",
                    f"match: {result['instruction_count']['match_percent']:.2f}%",
                    f"summary: {outputs['summary_json']}",
                    f"diff: {outputs['diff']}",
                    f"asm: {outputs['original_extracted_asm']} -> {outputs['current_compiler_asm']}",
                    "next: inspect function diff"
                    if not result["exact_match"]
                    else "next: bin/harness finish "
                    + target_id
                    + " --status done --message matched",
                ],
            )
            return 0 if result["exact_match"] else 1
        if bool(args.record_blocker):
            state.finish_target(
                conn,
                target_id=target_id,
                status="blocked",
                message="diff adapter needs a target-specific match workspace",
            )
        else:
            state.add_event(
                conn,
                target_id=target_id,
                kind="diff",
                message="diff adapter needs a target-specific match workspace",
            )
    payload = {
        "status": "needs_match_workspace",
        "target_id": target_id,
        "next": "bin/harness target init " + target_id,
    }
    emit(
        args,
        payload,
        lines=[
            "status: needs_match_workspace",
            "why: diff adapter needs a target-specific match workspace",
            f"next: bin/harness target init {target_id}",
        ],
    )
    return 1


def _binary_targets(
    conn: Any,
    config: HarnessConfig,
    args: argparse.Namespace,
) -> list[dict[str, Any]]:
    scan_limit = 100000 if bool(args.compiled_only) else args.limit
    rows = state.list_targets(
        conn,
        limit=scan_limit,
        status=args.status,
        target_type=args.type,
    )
    if bool(args.compiled_only):
        rows = [
            row
            for row in rows
            if (pair := resolve_binary_pair(config, row)).compiled is not None
            and pair.compiled.is_file()
        ]
    return rows[: args.limit]


def binary_diff_exit_code(statuses: list[str], *, allow_different: bool) -> int:
    if allow_different:
        return (
            0
            if all(status in {"exact_match", "different"} for status in statuses)
            else 1
        )
    return 0 if all(status == "exact_match" for status in statuses) else 1


def run_binary_diff(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        if bool(args.all):
            rows = _binary_targets(conn, config, args)
            results = []
            exact_matches = 0
            blocking_failures = 0
            for target in rows:
                result, path = build_binary_diff(
                    config,
                    target,
                    output_root=config.out_dir / "binary-diff",
                )
                state.add_event(
                    conn,
                    target_id=str(target["id"]),
                    kind="binary-diff",
                    message=str(result["status"]),
                    path=str(path),
                )
                exact_matches += int(result["status"] == "exact_match")
                if result["status"] not in {"exact_match", "different"}:
                    blocking_failures += 1
                results.append(
                    {
                        "target_id": target["id"],
                        "status": result["status"],
                        "report": str(path),
                    }
                )
            payload = {
                "count": len(results),
                "exact_matches": exact_matches,
                "blocking_failures": blocking_failures,
                "allow_different": bool(args.allow_different),
                "results": results,
            }
            lines = [
                f"binary-diff: {len(results)} target(s)",
                f"exact-matches: {exact_matches}",
                f"blocking-failures: {blocking_failures}",
            ]
            if bool(args.allow_different):
                lines.append("mode: allow-different")
            for result in results[: args.limit]:
                lines.append(
                    f"{result['status']:<20} {result['target_id']} {result['report']}"
                )
            emit(args, payload, lines=lines)
            if bool(args.allow_different):
                return binary_diff_exit_code(
                    [str(result["status"]) for result in results],
                    allow_different=True,
                )
            return binary_diff_exit_code(
                [str(result["status"]) for result in results],
                allow_different=False,
            )

        if not args.target_id:
            raise ValueError("verify binary needs <target-id> or --all")
        target = state.target_row(conn, args.target_id)
        if target is None:
            raise LookupError(f"unknown target: {args.target_id}")
        payload, path = build_binary_diff(
            config,
            target,
            output_root=config.out_dir / "binary-diff",
        )
        state.add_event(
            conn,
            target_id=args.target_id,
            kind="binary-diff",
            message=str(payload["status"]),
            path=str(path),
        )
    emit(
        args,
        payload,
        lines=[
            f"status: {payload['status']}",
            f"original-bin: {payload.get('original_bin') or ''}",
            f"compiled-bin: {payload.get('compiled_bin') or ''}",
            f"report: {path}",
            f"next: {payload['next_action']}",
        ],
    )
    return binary_diff_exit_code(
        [str(payload["status"])], allow_different=bool(args.allow_different)
    )


def run_binary_map(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        if bool(args.all):
            rows = _binary_targets(conn, config, args)
            results = []
            totals = {"functions": 0, "symbols": 0, "xrefs": 0}
            for target in rows:
                result, path = write_binary_map(
                    config,
                    target,
                    output_root=config.out_dir / "binary-maps",
                )
                state.record_binary_map(conn, result)
                state.add_event(
                    conn,
                    target_id=str(target["id"]),
                    kind="binary-map",
                    message=(
                        f"functions={result['function_count']} "
                        f"symbols={result['symbol_count']} "
                        f"xrefs={result['xref_count']}"
                    ),
                    path=str(path),
                )
                totals["functions"] += int(result["function_count"])
                totals["symbols"] += int(result["symbol_count"])
                totals["xrefs"] += int(result["xref_count"])
                results.append(
                    {
                        "target_id": target["id"],
                        "functions": result["function_count"],
                        "symbols": result["symbol_count"],
                        "xrefs": result["xref_count"],
                        "map": str(path),
                    }
                )
            payload = {"count": len(results), "totals": totals, "results": results}
            lines = [
                f"binary-maps: {len(results)} target(s)",
                f"functions: {totals['functions']}",
                f"symbols: {totals['symbols']}",
                f"xrefs: {totals['xrefs']}",
            ]
            for result in results[: args.limit]:
                lines.append(
                    f"{result['functions']:>4} fn  {result['symbols']:>4} sym  "
                    f"{result['xrefs']:>4} xref  {result['target_id']}"
                )
            emit(args, payload, lines=lines)
            return 0

        if not args.target_id:
            raise ValueError("binary map needs <target-id> or --all")
        target = state.target_row(conn, args.target_id)
        if target is None:
            raise LookupError(f"unknown target: {args.target_id}")
        payload, path = write_binary_map(
            config,
            target,
            output_root=config.out_dir / "binary-maps",
        )
        state.record_binary_map(conn, payload)
        state.add_event(
            conn,
            target_id=args.target_id,
            kind="binary-map",
            message=(
                f"functions={payload['function_count']} "
                f"symbols={payload['symbol_count']} xrefs={payload['xref_count']}"
            ),
            path=str(path),
        )
    emit(
        args,
        payload,
        lines=[
            f"map: {path}",
            f"functions: {payload['function_count']}",
            f"symbols: {payload['symbol_count']}",
            f"xrefs: {payload['xref_count']}",
        ],
    )
    return 0


def run_verify_binary(args: argparse.Namespace) -> int:
    return run_binary_diff(args)


def run_verify_function(args: argparse.Namespace) -> int:
    config = _config(args)
    source_payload = source_function_payload(config, args.source)
    binary_path = args.binary
    if binary_path is None and source_payload.get("binary_path"):
        binary_path = Path(str(source_payload["binary_path"]))
    load_address = (
        args.load_address
        if args.load_address is not None
        else source_payload.get("load_address")
    )
    payload = run_asm_diff_one(
        AsmDiffRequest(
            source_path=args.source,
            address=args.address,
            size=args.size if args.size is not None else source_payload.get("size"),
            binary_path=binary_path,
            load_address=load_address,
            output_root=args.output_root,
        )
    )
    outputs = payload["outputs"]
    next_action = (
        "claim/finish the function target or run the containing binary parity gate"
        if payload["exact_match"]
        else "inspect the function diff and update the source"
    )
    payload["next_action"] = next_action
    target_id = source_function_target_id(config, args.source)
    with state.state_db(config.database) as conn:
        state.add_event(
            conn,
            target_id=target_id
            if target_id and state.target_row(conn, target_id)
            else None,
            kind="function-diff",
            message=f"{payload['status']} {payload['function']}",
            path=str(outputs["summary_json"]),
        )
    emit(
        args,
        payload,
        lines=[
            f"status: {payload['status']}",
            f"function: {payload['function']} {payload['address']}",
            f"match: {payload['instruction_count']['match_percent']:.2f}%",
            f"summary: {outputs['summary_json']}",
            f"diff: {outputs['diff']}",
            f"asm: {outputs['original_extracted_asm']} -> {outputs['current_compiler_asm']}",
            f"next: {next_action}",
        ],
    )
    return 0 if payload["exact_match"] else 1


def run_verify_module(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        rows = state.list_targets(
            conn,
            limit=100000,
            status=args.status,
            target_type="function",
        )
        targets = candidate_targets(
            rows,
            module=args.module,
            min_size=args.min_size,
            source="existing",
            largest=bool(args.largest),
            limit=args.limit,
        )
        results: list[dict[str, Any]] = []
        for target in targets:
            payload = target_payload(target)
            source_path = payload.get("source_path")
            if not source_path:
                continue
            try:
                result = run_asm_diff_one(
                    AsmDiffRequest(
                        source_path=config.root / str(source_path),
                        address=parse_int(str(target["entry_hex"]))
                        if target.get("entry_hex")
                        else None,
                        size=payload.get("size"),
                        binary_path=config.root / str(payload["binary_path"])
                        if payload.get("binary_path")
                        else None,
                        load_address=payload.get("load_address"),
                    )
                )
                outputs = result["outputs"]
                state.add_event(
                    conn,
                    target_id=str(target["id"]),
                    kind="function-diff",
                    message=f"{result['status']} {result['function']}",
                    path=str(outputs["summary_json"]),
                )
                results.append(
                    {
                        "target_id": str(target["id"]),
                        "display_id": target_display_id(target),
                        "source": str(source_path),
                        "function": result["function"],
                        "status": result["status"],
                        "match_percent": result["instruction_count"]["match_percent"],
                        "summary": outputs["summary_json"],
                        "diff": outputs["diff"],
                    }
                )
            except Exception as exc:
                state.add_event(
                    conn,
                    target_id=str(target["id"]),
                    kind="function-diff-error",
                    message=str(exc),
                )
                results.append(
                    {
                        "target_id": str(target["id"]),
                        "display_id": target_display_id(target),
                        "source": str(source_path),
                        "function": Path(str(source_path)).stem,
                        "status": "error",
                        "match_percent": 0.0,
                        "error": str(exc),
                    }
                )
    exact = sum(1 for result in results if result["status"] == "exact_match")
    payload = {
        "module": args.module,
        "count": len(results),
        "exact_matches": exact,
        "allow_different": bool(args.allow_different),
        "results": results,
    }
    lines = [
        f"module: {args.module}",
        f"verified: {len(results)}",
        f"exact-matches: {exact}",
        f"{'match':>8}  {'status':<12}  function",
    ]
    for result in results:
        lines.append(
            f"{float(result['match_percent']):>7.2f}%  "
            f"{result['status']:<12}  {result['function']}  {result.get('diff', '')}"
        )
    emit(args, payload, lines=lines)
    if bool(args.allow_different):
        return 0 if all(result["status"] != "error" for result in results) else 1
    return 0 if results and exact == len(results) else 1


def run_finish(args: argparse.Namespace) -> int:
    config = _config(args)
    with state.state_db(config.database) as conn:
        target = _target_row(conn, args.target_id)
        if target is None:
            raise LookupError(f"unknown target: {args.target_id}")
        target_id = str(target["id"])
        state.finish_target(
            conn,
            target_id=target_id,
            status=args.status,
            message=args.message,
            path=str(args.path) if args.path else None,
        )
    payload = {
        "target_id": target_id,
        "status": args.status,
        "next": "bin/harness resume",
    }
    emit(
        args,
        payload,
        lines=[
            f"target: {target_id}",
            f"status: {args.status}",
            "next: bin/harness resume",
        ],
    )
    return 0


def run_report(args: argparse.Namespace) -> int:
    config = _config(args)
    ensure_dirs(config)
    if getattr(args, "report_kind", None) == "function":
        if not args.target_or_source:
            raise ValueError("report function needs <target-id-or-source>")
        target_or_source = str(args.target_or_source)
        source = Path(target_or_source)
        with state.state_db(config.database) as conn:
            target = None if source.suffix == ".c" else _target_row(conn, target_or_source)
        payload = function_report_payload(
            config,
            target,
            source=source if target is None else None,
        )
        lines = [
            f"target: {payload['display_id']}",
            f"function: {payload['function']}",
            f"source: {payload.get('source') or ''}",
            f"binary: {payload.get('binary_path') or ''}",
            f"load-address: {payload.get('load_address') or ''}",
            f"size: {payload.get('size') or ''}",
            f"asm: {payload.get('original_asm') or ''}",
            f"m2c: {payload.get('m2c_draft') or ''}",
            f"asm-diff: {payload.get('asm_diff_summary') or ''}",
            f"next: {payload['next_action']}",
        ]
        emit(args, payload, lines=lines)
        return 0
    if getattr(args, "report_kind", None) not in {None, "summary"}:
        raise ValueError(f"unknown report kind: {args.report_kind}")
    with state.state_db(config.database) as conn:
        payload, json_path, md_path = write_report(config, conn)
    emit(
        args,
        payload,
        lines=[
            f"report-json: {json_path}",
            f"report-md: {md_path}",
            f"active-claims: {len(payload['active_claims'])}",
        ],
    )
    return 0


def run_resume(args: argparse.Namespace) -> int:
    config = _config(args)
    ensure_dirs(config)
    with state.state_db(config.database) as conn:
        action = choose_resume_action(config, conn)
    emit(
        args,
        action,
        lines=[
            f"next: {action['command']}",
            f"why: {action['reason']}",
        ],
    )
    return 0


def run_checkpoint(args: argparse.Namespace) -> int:
    config = _config(args)
    ensure_dirs(config)
    with state.state_db(config.database) as conn:
        payload, json_path, md_path = write_report(config, conn)
    checkpoint_path = config.out_dir / "checkpoint.json"
    write_json(checkpoint_path, payload)
    emit(
        args,
        {"checkpoint": str(checkpoint_path), "report": str(json_path)},
        lines=[
            f"checkpoint: {checkpoint_path}",
            f"report-json: {json_path}",
            f"report-md: {md_path}",
        ],
    )
    return 0


def run_dashboard(args: argparse.Namespace) -> int:
    config = _config(args)
    ensure_dirs(config)
    with state.state_db(config.database) as conn:
        payload = build_report(config, conn)
    path = write_dashboard(payload, config.dashboard_dir)
    emit(args, {"dashboard": str(path)}, lines=[f"dashboard: {path}"])
    return 0


def run_bootstrap(args: argparse.Namespace) -> int:
    config = _config(args)
    if bool(args.plan):
        payload = {"command": config.commands.get("bootstrap", ""), "run": False}
        emit(args, payload, lines=[f"bootstrap: {payload['command']}", "run: false"])
        return 0
    run_configured_command(config, "bootstrap")
    emit(
        args,
        {"run": True, "next": "bin/harness catalog"},
        lines=["bootstrap: complete", "next: bin/harness catalog"],
    )
    return 0


def run_tool_health(args: argparse.Namespace) -> int:
    config = _config(args)
    payload = {"tools": [item.to_dict() for item in tool_health(config)]}
    lines = [
        f"{item['status']:<10} {item['name']:<24} {item['detail']}"
        for item in payload["tools"]
    ]
    emit(args, payload, lines=lines)
    return 0


def run_ghidra_coverage(args: argparse.Namespace) -> int:
    config = _config(args)
    payload = build_ghidra_coverage(config)
    missing = payload["missing_programs"]
    lines = [
        f"expected-programs: {payload['expected_program_count']}",
        f"exported-programs: {payload['exported_program_count']}",
        f"matched-programs: {payload['matched_program_count']}",
        f"missing-programs: {payload['missing_program_count']}",
    ]
    for program in missing[: args.limit]:
        lines.append(f"missing: {program}")
    emit(args, payload, lines=lines)
    return 0 if payload["complete"] or args.allow_partial else 1


def run_ghidra_locked_command(args: argparse.Namespace) -> int:
    config = _config(args)
    owner = args.owner or getpass.getuser()
    command = [str(config.root / "bin" / args.command_name), *list(args.extra_args or [])]
    with state.state_db(config.database) as conn:
        acquired = state.acquire_lock(
            conn,
            name="ghidra",
            owner=owner,
            lease_minutes=args.lease_minutes,
        )
        lock = state.lock_row(conn, "ghidra")
        if not acquired:
            current_owner = "unknown" if lock is None else str(lock["owner"])
            payload = {
                "acquired": False,
                "command": command,
                "owner": owner,
                "current_owner": current_owner,
                "next": "wait for the active Ghidra command or release stale lock",
            }
            emit(
                args,
                payload,
                lines=[
                    "lock: ghidra held",
                    f"owner: {current_owner}",
                    "next: wait for the active Ghidra command or release stale lock",
                ],
            )
            return 1
        state.add_event(
            conn,
            target_id=None,
            kind="ghidra-lock",
            message=f"{args.command_name} locked by {owner}",
        )
    result = None
    try:
        result = run_process(command, cwd=config.root, stream=True, check=False)
        return_code = result.returncode
    finally:
        with state.state_db(config.database) as conn:
            state.release_lock(conn, name="ghidra", owner=owner)
            state.add_event(
                conn,
                target_id=None,
                kind="ghidra-unlock",
                message=f"{args.command_name} released by {owner}",
            )
    payload = {
        "acquired": True,
        "command": command,
        "owner": owner,
        "returncode": return_code,
    }
    emit(
        args,
        payload,
        lines=[
            "lock: ghidra acquired",
            f"owner: {owner}",
            f"command: {' '.join(command)}",
            f"returncode: {return_code}",
        ],
    )
    return return_code


def configure_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--config", type=Path)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--verbose", action="store_true")
    subparsers = parser.add_subparsers(required=True)

    setup = subparsers.add_parser("setup")
    setup.add_argument("--run-existing", action="store_true")
    setup.set_defaults(handler=run_setup)

    status = subparsers.add_parser("status")
    status.set_defaults(handler=run_status)

    bootstrap = subparsers.add_parser("bootstrap")
    bootstrap.add_argument("--plan", action="store_true")
    bootstrap.set_defaults(handler=run_bootstrap)

    catalog = subparsers.add_parser("catalog")
    catalog.set_defaults(handler=run_catalog)

    analyze = subparsers.add_parser("analyze")
    analyze.add_argument("--run-existing", action="store_true")
    analyze.set_defaults(handler=run_analyze)

    split = subparsers.add_parser("split")
    split.add_argument("--create-source-dirs", action="store_true")
    split.set_defaults(handler=run_split)

    queue = subparsers.add_parser("queue")
    queue.add_argument("--limit", type=int, default=20)
    queue.add_argument("--status")
    queue.add_argument("--type")
    queue.add_argument("--module", help="filter by EMI/module target, e.g. emi:BATTLE/BATTLE#15")
    queue.set_defaults(handler=run_queue)

    candidates = subparsers.add_parser("candidates")
    candidates.add_argument("--limit", type=int, default=20)
    candidates.add_argument("--module")
    candidates.add_argument("--min-size", type=parse_int, default=0)
    candidates.add_argument("--status", default="queued")
    candidates.add_argument(
        "--source",
        choices=("missing", "existing", "all"),
        default="missing",
        help="select functions without source, with source, or both",
    )
    candidates.add_argument("--largest", action="store_true", default=True)
    candidates.set_defaults(handler=run_candidates)

    claim = subparsers.add_parser("claim")
    claim.add_argument("target_id", nargs="?")
    claim.add_argument("--owner")
    claim.add_argument("--lease-minutes", type=int, default=120)
    claim.add_argument("--status")
    claim.add_argument("--type")
    claim.add_argument("--module", help="claim the next target from an EMI/module filter")
    claim.set_defaults(handler=run_claim)

    target = subparsers.add_parser("target")
    target_subparsers = target.add_subparsers(required=True)
    target_show = target_subparsers.add_parser("show")
    target_show.add_argument("target_id")
    target_show.set_defaults(handler=run_target_show)
    target_init = target_subparsers.add_parser("init")
    target_init.add_argument("target_id")
    target_init.set_defaults(handler=run_target_init)

    lock = subparsers.add_parser("lock")
    lock_subparsers = lock.add_subparsers(required=True)
    lock_acquire = lock_subparsers.add_parser("acquire")
    lock_acquire.add_argument("name")
    lock_acquire.add_argument("--owner")
    lock_acquire.add_argument("--lease-minutes", type=int, default=60)
    lock_acquire.set_defaults(handler=run_lock_acquire)
    lock_release = lock_subparsers.add_parser("release")
    lock_release.add_argument("name")
    lock_release.add_argument("--owner")
    lock_release.set_defaults(handler=run_lock_release)

    context = subparsers.add_parser("context")
    context_subparsers = context.add_subparsers(required=True)
    context_build = context_subparsers.add_parser("build")
    context_build.add_argument("target_id")
    context_build.set_defaults(handler=run_context_build)

    m2c = subparsers.add_parser("m2c")
    m2c.add_argument("target_id")
    m2c.add_argument("extra_args", nargs=argparse.REMAINDER)
    m2c.set_defaults(handler=run_m2c)

    lift = subparsers.add_parser("lift")
    lift.add_argument("target_id")
    lift.add_argument("extra_args", nargs=argparse.REMAINDER)
    lift.set_defaults(handler=run_lift)

    lift_batch = subparsers.add_parser("lift-batch")
    lift_batch.add_argument("--limit", type=int, default=5)
    lift_batch.add_argument("--module")
    lift_batch.add_argument("--min-size", type=parse_int, default=0)
    lift_batch.add_argument("--status", default="queued")
    lift_batch.add_argument(
        "--source",
        choices=("missing", "existing", "all"),
        default="missing",
    )
    lift_batch.add_argument("--largest", action="store_true", default=True)
    lift_batch.add_argument("extra_args", nargs=argparse.REMAINDER)
    lift_batch.set_defaults(handler=run_lift_batch)

    diff = subparsers.add_parser("diff")
    diff.add_argument("target_id")
    diff.add_argument("--record-blocker", action="store_true")
    diff.set_defaults(handler=run_diff)

    binary = subparsers.add_parser("binary")
    binary_subparsers = binary.add_subparsers(required=True)
    binary_diff = binary_subparsers.add_parser("diff")
    binary_diff.add_argument("target_id", nargs="?")
    binary_diff.add_argument("--all", action="store_true")
    binary_diff.add_argument("--limit", type=int, default=200)
    binary_diff.add_argument("--compiled-only", action="store_true")
    binary_diff.add_argument(
        "--allow-different",
        action="store_true",
        help="return success when compared binaries differ but diff artifacts were written",
    )
    binary_diff.add_argument("--status")
    binary_diff.add_argument("--type", default="emi")
    binary_diff.set_defaults(handler=run_binary_diff)
    binary_map = binary_subparsers.add_parser("map")
    binary_map.add_argument("target_id", nargs="?")
    binary_map.add_argument("--all", action="store_true")
    binary_map.add_argument("--limit", type=int, default=200)
    binary_map.add_argument("--compiled-only", action="store_true")
    binary_map.add_argument("--status")
    binary_map.add_argument("--type", default="emi")
    binary_map.set_defaults(handler=run_binary_map)

    verify = subparsers.add_parser("verify")
    verify_subparsers = verify.add_subparsers(required=True)
    verify_binary = verify_subparsers.add_parser("binary")
    verify_binary.add_argument("target_id", nargs="?")
    verify_binary.add_argument("--all", action="store_true")
    verify_binary.add_argument("--limit", type=int, default=200)
    verify_binary.add_argument("--compiled-only", action="store_true")
    verify_binary.add_argument(
        "--allow-different",
        action="store_true",
        help="return success when compared binaries differ but diff artifacts were written",
    )
    verify_binary.add_argument("--status")
    verify_binary.add_argument("--type", default="emi")
    verify_binary.set_defaults(handler=run_verify_binary)
    verify_function = verify_subparsers.add_parser("function")
    verify_function.add_argument("source", type=Path)
    verify_function.add_argument(
        "--address",
        type=parse_int,
        help="original function address; inferred from @source or func_XXXXXXXX when omitted",
    )
    verify_function.add_argument(
        "--size",
        type=parse_int,
        help="original function byte size; inferred from the next sibling source when omitted",
    )
    verify_function.add_argument(
        "--binary",
        type=Path,
        help="original PS-X EXE or raw overlay binary; core sources default to SLUS",
    )
    verify_function.add_argument(
        "--load-address",
        type=parse_int,
        help="load address for raw binaries; PS-X EXE headers are read automatically",
    )
    verify_function.add_argument(
        "--output-root",
        type=Path,
        help="directory for asm diff outputs; defaults to out/asm-diff",
    )
    verify_function.set_defaults(handler=run_verify_function)
    verify_module = verify_subparsers.add_parser("module")
    verify_module.add_argument("module")
    verify_module.add_argument("--limit", type=int, default=200)
    verify_module.add_argument("--min-size", type=parse_int, default=0)
    verify_module.add_argument("--status")
    verify_module.add_argument("--largest", action="store_true", default=False)
    verify_module.add_argument("--allow-different", action="store_true")
    verify_module.set_defaults(handler=run_verify_module)

    finish = subparsers.add_parser("finish")
    finish.add_argument("target_id")
    finish.add_argument(
        "--status", choices=("done", "blocked", "queued"), default="done"
    )
    finish.add_argument("--message", default="finished")
    finish.add_argument("--path", type=Path)
    finish.set_defaults(handler=run_finish)

    report = subparsers.add_parser("report")
    report.add_argument("report_kind", nargs="?", default="summary")
    report.add_argument("target_or_source", nargs="?")
    report.set_defaults(handler=run_report)

    resume = subparsers.add_parser("resume")
    resume.set_defaults(handler=run_resume)

    checkpoint = subparsers.add_parser("checkpoint")
    checkpoint.set_defaults(handler=run_checkpoint)

    dashboard = subparsers.add_parser("dashboard")
    dashboard.set_defaults(handler=run_dashboard)

    tools = subparsers.add_parser("tools")
    tools.set_defaults(handler=run_tool_health)

    ghidra = subparsers.add_parser("ghidra")
    ghidra_subparsers = ghidra.add_subparsers(required=True)
    ghidra_coverage = ghidra_subparsers.add_parser("coverage")
    ghidra_coverage.add_argument("--allow-partial", action="store_true")
    ghidra_coverage.add_argument("--limit", type=int, default=20)
    ghidra_coverage.set_defaults(handler=run_ghidra_coverage)
    ghidra_import = ghidra_subparsers.add_parser("import-project")
    ghidra_import.add_argument("--owner")
    ghidra_import.add_argument("--lease-minutes", type=int, default=240)
    ghidra_import.add_argument("extra_args", nargs=argparse.REMAINDER)
    ghidra_import.set_defaults(
        handler=run_ghidra_locked_command,
        command_name="ghidra-import-project",
    )
    ghidra_export = ghidra_subparsers.add_parser("export-symbols")
    ghidra_export.add_argument("--owner")
    ghidra_export.add_argument("--lease-minutes", type=int, default=120)
    ghidra_export.add_argument("extra_args", nargs=argparse.REMAINDER)
    ghidra_export.set_defaults(
        handler=run_ghidra_locked_command,
        command_name="ghidra-export-symbols",
    )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="harness")
    configure_parser(parser)
    return parser


def main(argv: list[str] | None = None) -> int:
    try:
        return run_main(build_parser, argv)
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        print("next: bin/harness resume", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
