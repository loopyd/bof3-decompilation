from __future__ import annotations

import argparse
import getpass
import json
import re
import sqlite3
from pathlib import Path
from typing import Any

from ..commands._common import run_main
from ..match.asm_diff import (
    AsmDiffRequest,
    parse_int,
    run_asm_diff_one,
)
from . import state
from .config import HarnessConfig, load_harness_config
from .report import build_report
from .tasks import (
    analysis_function_target_records,
    compact_target_row,
    resolve_function_target_alias,
    source_function_payload,
    source_function_target_id,
    target_matches_module,
)


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


def _target_row(
    conn: sqlite3.Connection, target_id: str
) -> dict[str, Any] | None:
    target = state.target_row(conn, target_id)
    if target is not None:
        return target
    rows = state.list_targets(conn, limit=100000, target_type="function")
    return resolve_function_target_alias(rows, target_id)


def _analysis_db_path(config: HarnessConfig) -> Path:
    return config.root / "output" / "analysis.sqlite3"


def _open_analysis_db(
    config: HarnessConfig,
) -> sqlite3.Connection | None:
    path = _analysis_db_path(config)
    if not path.is_file():
        return None
    conn = sqlite3.connect(str(path))
    conn.row_factory = sqlite3.Row
    return conn


def _analysis_call_edge_counts(
    config: HarnessConfig,
) -> dict[str, int]:
    analysis = _open_analysis_db(config)
    if analysis is None:
        return {}
    try:
        rows = analysis.execute(
            "SELECT from_func AS address, COUNT(*) AS cnt FROM call_edges GROUP BY from_func"
        ).fetchall()
        return {str(row["address"]): int(row["cnt"]) for row in rows}
    except Exception:
        return {}
    finally:
        analysis.close()


def run_claim(args: argparse.Namespace) -> int:
    config = _config(args)
    owner = args.owner or getpass.getuser()

    with state.state_db(config.database) as conn:
        if args.module and not args.target_id:
            edge_counts = _analysis_call_edge_counts(config)
            rows = state.list_targets(
                conn,
                limit=100000,
                status=args.status,
                target_type=args.type,
            )
            rows = [
                row for row in rows
                if target_matches_module(row, args.module)
                and str(row.get("status") or "") in ("queued", "ready", "analyzed")
            ]

            if edge_counts:
                rows.sort(
                    key=lambda r: edge_counts.get(
                        str(r.get("entry_hex") or ""), 0
                    )
                )

            target = None
            for row in rows:
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
            target_id = args.target_id
            if target_id:
                existing = _target_row(conn, target_id)
                if existing is not None:
                    target_id = str(existing["id"])
            target = state.claim_target(
                conn,
                owner=owner,
                target_id=target_id,
                status=args.status,
                target_type=args.type,
                lease_minutes=args.lease_minutes,
            )

    if target is None:
        payload: dict[str, Any] = {"claimed": None}
        emit(args, payload, lines=["claim: none"])
        return 1

    compact = compact_target_row(target)
    payload = {
        "target_id": compact["id"],
        "address": compact.get("entry_hex"),
        "name": compact["summary"],
        "program": compact.get("program_path"),
        "source_hint": compact.get("source_hint"),
        "owner": owner,
    }
    emit(
        args,
        payload,
        lines=[
            f"claimed: {payload['target_id']}",
            f"address: {payload['address']}",
            f"program: {payload['program']}",
            f"source_hint: {payload['source_hint']}",
        ],
    )
    return 0


def run_release(args: argparse.Namespace) -> int:
    config = _config(args)
    owner = args.owner or getpass.getuser()

    with state.state_db(config.database) as conn:
        target = _target_row(conn, args.target_id)
        if target is None:
            raise LookupError(f"unknown target: {args.target_id}")
        target_id = str(target["id"])
        claim = state.active_claim_for_target(conn, target_id)
        if claim is None:
            payload = {"released": None, "target_id": target_id}
            emit(
                args,
                payload,
                lines=[
                    f"target: {target_id}",
                    "status: no active claim",
                ],
            )
            return 1

        if str(claim["owner"]) != owner:
            payload = {
                "released": None,
                "target_id": args.target_id,
                "current_owner": claim["owner"],
            }
            emit(
                args,
                payload,
                lines=[
                    f"target: {target_id}",
                    f"status: held by {claim['owner']}",
                ],
            )
            return 1

        conn.execute(
            "UPDATE claims SET status = 'closed' WHERE target_id = ? AND status = 'active'",
            (target_id,),
        )
        state.add_event(
            conn,
            target_id=target_id,
            kind="release",
            message=f"released by {owner}",
        )

    payload = {"released": target_id, "owner": owner}
    emit(
        args,
        payload,
        lines=[f"released: {target_id}", f"owner: {owner}"],
    )
    return 0


def run_diff(args: argparse.Namespace) -> int:
    config = _config(args)
    target_id_or_source = Path(args.target_id_or_source_path)

    with state.state_db(config.database) as conn:
        source_path: Path | None = None
        address: int | None = None
        size: int | None = None
        binary_path: Path | None = None
        load_address: int | None = None
        target_id: str | None = None

        if target_id_or_source.suffix == ".c":
            source_path = (
                target_id_or_source
                if target_id_or_source.is_absolute()
                else config.root / target_id_or_source
            )
            source_payload = source_function_payload(config, source_path)
            address = (
                parse_int(str(source_payload["entry_hex"]))
                if source_payload.get("entry_hex")
                else None
            )
            size = source_payload.get("size")
            binary_path = (
                config.root / str(source_payload["binary_path"])
                if source_payload.get("binary_path")
                else None
            )
            load_address = source_payload.get("load_address")
            target_id = source_function_target_id(config, source_path)
        else:
            target = _target_row(conn, str(target_id_or_source))
            if target is None:
                raise LookupError(
                    f"unknown target: {target_id_or_source}"
                )
            target_id = str(target["id"])
            raw_payload: Any = target.get("payload")
            payload_data: dict[str, Any] = (
                raw_payload if isinstance(raw_payload, dict) else {}
            )
            source_str: str | None = payload_data.get("source_path")
            if not source_str:
                raise ValueError(
                    f"target {target_id} has no source_path in payload"
                )
            source_path = config.root / source_str
            address = (
                parse_int(str(target["entry_hex"]))
                if target.get("entry_hex")
                else None
            )
            size_raw: Any = payload_data.get("size")
            size = int(size_raw) if size_raw is not None else None
            bin_path_str: str | None = payload_data.get("binary_path")
            binary_path = config.root / bin_path_str if bin_path_str else None
            load_address_raw: Any = payload_data.get("load_address")
            load_address = int(load_address_raw) if load_address_raw is not None else None

        if source_path is None:
            raise ValueError("could not resolve source path")

        result = run_asm_diff_one(
            AsmDiffRequest(
                source_path=source_path,
                address=address,
                size=size,
                binary_path=binary_path,
                load_address=load_address,
            )
        )
        outputs = result["outputs"]

        if target_id:
            match_percent = result["instruction_count"]["match_percent"]
            state.add_event(
                conn,
                target_id=target_id,
                kind="function-diff",
                message=(
                    f"status={result['status']} "
                    f"exact={result['exact_match']} "
                    f"match_pct={match_percent:.2f}"
                ),
                path=str(outputs["summary_json"]),
            )

    emit(
        args,
        {**result, "target_id": target_id},
        lines=[
            f"status: {result['status']}",
            f"function: {result['function']} {result['address']}",
            f"match: {result['instruction_count']['match_percent']:.2f}%",
            f"summary: {outputs['summary_json']}",
            f"diff: {outputs['diff']}",
        ],
    )
    return 0 if result["exact_match"] else 1


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
    }
    emit(
        args,
        payload,
        lines=[
            f"target: {target_id}",
            f"status: {args.status}",
        ],
    )
    return 0


def run_seed(args: argparse.Namespace) -> int:
    config = _config(args)
    analysis = _open_analysis_db(config)
    if analysis is None:
        payload = {"seeded": False, "analysis_db": str(_analysis_db_path(config))}
        emit(
            args,
            payload,
            lines=[f"analysis DB not found: {_analysis_db_path(config)}"],
        )
        return 1
    try:
        rows = [
            dict(row)
            for row in analysis.execute(
                """
                SELECT functions.*, programs.name AS program_name
                FROM functions
                JOIN programs ON programs.path = functions.program_path
                WHERE functions.is_thunk = 0
                ORDER BY functions.program_path, functions.address
                """
            ).fetchall()
        ]
    finally:
        analysis.close()

    records = analysis_function_target_records(config, rows)
    with state.state_db(config.database) as conn:
        count = state.upsert_targets(conn, records)
        pruned = 0
        if args.prune:
            pruned = state.prune_stale_targets(
                conn,
                target_type="function",
                keep_ids=[str(record["id"]) for record in records],
            )

    payload = {
        "database": str(config.database),
        "analysis_db": str(_analysis_db_path(config)),
        "upserted": count,
        "pruned": pruned,
    }
    emit(
        args,
        payload,
        lines=[
            f"database: {config.database}",
            f"analysis DB: {_analysis_db_path(config)}",
            f"function targets upserted: {count}",
            f"stale queued targets pruned: {pruned}",
        ],
    )
    return 0


def _status_section(
    lines: list[str], heading: str, entries: list[str]
) -> None:
    if not entries:
        return
    lines.append(heading)
    lines.append("-" * len(heading))
    lines.extend(entries)
    lines.append("")


def run_status(args: argparse.Namespace) -> int:
    config = _config(args)

    with state.state_db(config.database) as conn:
        report = build_report(config, conn)

    counts = report["counts_by_status"]
    total = sum(counts.values())
    claims = report["active_claims"]

    lines: list[str] = []
    lines.append(f"database: {config.database}")
    lines.append("")

    lines.append(f"total targets: {total}")
    for status_key in sorted(counts):
        lines.append(f"  {status_key}: {counts[status_key]}")
    lines.append("")

    if claims:
        lines.append(f"active claims: {len(claims)}")
        for claim in claims:
            lines.append(
                f"  {claim['target_id']}  owner={claim['owner']}"
            )
        lines.append("")

    per_module: dict[str, dict[str, int]] = {}
    with state.state_db(config.database) as conn:
        all_targets = state.list_targets(
            conn, limit=100000, target_type="function"
        )
    for target in all_targets:
        module = (
            target.get("source_hint")
            or target.get("program_path")
            or "unknown"
        )
        module = str(module)
        if args.module and not target_matches_module(target, args.module):
            continue
        st = str(target.get("status") or "unknown")
        per_module.setdefault(module, {}).setdefault(st, 0)
        per_module[module][st] += 1

    if per_module:
        lines.append("per-module progress:")
        lines.append("-" * 22)
        for module_name in sorted(per_module):
            mod_counts = per_module[module_name]
            mod_total = sum(mod_counts.values())
            done = mod_counts.get("done", 0)
            pct = (done / mod_total * 100) if mod_total else 0
            parts = [f"  done={done}/{mod_total} ({pct:.0f}%)"]
            for st_key in sorted(mod_counts):
                if st_key != "done":
                    parts.append(f"{st_key}={mod_counts[st_key]}")
            lines.append(f"{module_name}:")
            lines.append(" ".join(parts))
        lines.append("")

    analysis = _open_analysis_db(config)
    if analysis:
        try:
            func_count = analysis.execute(
                "SELECT COUNT(*) AS cnt FROM functions"
            ).fetchone()
            if func_count:
                lines.append(
                    f"analysis DB functions: {func_count['cnt']}"
                )
        except Exception:
            pass
        finally:
            analysis.close()

    payload: dict[str, Any] = {
        "database": str(config.database),
        "counts_by_status": counts,
        "total_targets": total,
        "active_claims": [
            {"target_id": c["target_id"], "owner": c["owner"]}
            for c in claims
        ],
        "per_module": {
            m: {**c} for m, c in per_module.items()
        },
    }
    emit(args, payload, lines=lines)
    return 0


def run_export(args: argparse.Namespace) -> int:
    config = _config(args)

    with state.state_db(config.database) as conn:
        target = _target_row(conn, args.target_id)
        if target is None:
            raise LookupError(f"unknown target: {args.target_id}")

        target_id = str(target["id"])
        payload_data = (
            target.get("payload")
            if isinstance(target.get("payload"), dict)
            else {}
        )
        entry_hex = str(target.get("entry_hex") or "")
        program_path = str(target.get("program_path") or "")
        summary = str(target.get("summary") or "")

    address_int = None
    if entry_hex:
        try:
            address_int = parse_int(entry_hex)
        except (ValueError, TypeError):
            pass

    result: dict[str, Any] = {
        "target_id": target_id,
        "address": entry_hex,
        "name": summary,
        "program": program_path,
        "source_hint": target.get("source_hint"),
        "status": target.get("status"),
        "priority": target.get("priority"),
        "payload": payload_data,
    }

    analysis = _open_analysis_db(config)
    if analysis:
        try:
            if address_int is not None:
                rows = analysis.execute(
                    "SELECT * FROM functions WHERE address = ? AND program_path = ?",
                    (entry_hex, program_path),
                ).fetchall()
                if not rows:
                    rows = analysis.execute(
                        "SELECT * FROM functions WHERE address = ?",
                        (entry_hex,),
                    ).fetchall()

                if rows:
                    row = dict(rows[0])
                    result["signature"] = row.get("signature")
                    result["body_min"] = row.get("body_min")
                    result["body_max"] = row.get("body_max")
                    result["call_edges"] = row.get("call_edges")
                    result["name"] = row.get("name") or result["name"]

            callers = []
            callees = []
            xrefs_to = []
            xrefs_from = []
            duplicates = []

            if address_int is not None:
                caller_rows = analysis.execute(
                    "SELECT * FROM call_edges WHERE to_func = ?",
                    (entry_hex,),
                ).fetchall()
                callers = [dict(r) for r in caller_rows]

                callee_rows = analysis.execute(
                    "SELECT * FROM call_edges WHERE from_func = ?",
                    (entry_hex,),
                ).fetchall()
                callees = [dict(r) for r in callee_rows]

                xref_to_rows = analysis.execute(
                    "SELECT * FROM xrefs WHERE to_address = ?",
                    (entry_hex,),
                ).fetchall()
                xrefs_to = [dict(r) for r in xref_to_rows]

                xref_from_rows = analysis.execute(
                    "SELECT * FROM xrefs WHERE from_address = ?",
                    (entry_hex,),
                ).fetchall()
                xrefs_from = [dict(r) for r in xref_from_rows]

                dup_rows = analysis.execute(
                    "SELECT * FROM duplicates"
                ).fetchall()
                for dup_row in dup_rows:
                    duplicate = dict(dup_row)
                    entries = json.loads(str(duplicate.get("entries_json") or "[]"))
                    if any(
                        isinstance(entry, dict)
                        and entry.get("address") == entry_hex
                        and entry.get("program_path") == program_path
                        for entry in entries
                    ):
                        duplicate["entries"] = entries
                        duplicates.append(duplicate)

            result["callers"] = callers
            result["callees"] = callees
            result["xrefs_to"] = xrefs_to
            result["xrefs_from"] = xrefs_from
            result["duplicate_info"] = (
                duplicates[0] if duplicates else {}
            )

            const_rows = (
                analysis.execute(
                    "SELECT * FROM constants WHERE address = ?",
                    (entry_hex,),
                ).fetchall()
                if address_int is not None
                else []
            )
            result["constants_in_range"] = [dict(r) for r in const_rows]
        except Exception:
            pass
        finally:
            analysis.close()

    emit(args, result, lines=[json.dumps(result, indent=2, sort_keys=True, default=str)])
    return 0


def run_list(args: argparse.Namespace) -> int:
    config = _config(args)
    kind = args.kind  # "programs" or "functions"

    if kind == "programs":
        return _run_list_programs(args, config)
    elif kind == "functions":
        return _run_list_functions(args, config)
    else:
        raise ValueError(f"unknown list kind: {kind}")


def _run_list_programs(args: argparse.Namespace, config: HarnessConfig) -> int:
    programs: list[dict[str, Any]] = []
    analysis = _open_analysis_db(config)

    if analysis:
        try:
            rows = analysis.execute(
                """
                SELECT programs.path, programs.name, COUNT(functions.id) AS func_count
                FROM programs
                LEFT JOIN functions ON functions.program_path = programs.path
                  AND functions.is_thunk = 0
                GROUP BY programs.path, programs.name
                ORDER BY programs.path
                """
            ).fetchall()
            for row in rows:
                programs.append({
                    "program_path": row["path"],
                    "program_name": row["name"],
                    "analysis_funcs": row["func_count"],
                })
        finally:
            analysis.close()

    with state.state_db(config.database) as conn:
        targets = state.counts_by_status(conn)
        all_rows = state.list_targets(conn, limit=100000, target_type="function")

    prog_targets: dict[str, dict[str, int]] = {}
    prog_events: dict[str, list[float]] = {}
    for t in all_rows:
        pp = str(t.get("program_path") or "unknown")
        st = str(t.get("status") or "unknown")
        prog_targets.setdefault(pp, {}).setdefault(st, 0)
        prog_targets[pp][st] += 1

    with state.state_db(config.database) as conn:
        event_rows = conn.execute(
            "SELECT target_id, kind, message FROM events WHERE kind='function-diff'"
        ).fetchall()
    for ev in event_rows:
        msg = ev["message"]
        try:
            m = re.search(r"match_pct=([\d.]+)", msg)
            if m:
                prog_events.setdefault(ev["target_id"], []).append(float(m.group(1)))
        except (ValueError, AttributeError):
            pass

    lines: list[str] = []
    lines.append(f"{'program':<40} {'funcs':>6} {'done':>6} {'queued':>7} {'avg_match':>9}")
    lines.append("-" * 72)

    payload_programs: list[dict[str, Any]] = []
    for prog in sorted(programs, key=lambda p: str(p["program_path"])):
        pp = str(prog["program_path"])
        pt = prog_targets.get(pp, {})
        total = sum(pt.values())
        done = pt.get("done", 0)
        queued = pt.get("queued", 0)
        pct = f"{done/total*100:.0f}%" if total else "-"

        match_pcts = prog_events.get(pp, [])
        avg = f"{sum(match_pcts)/len(match_pcts):.1f}%" if match_pcts else "-"

        if args.module and not target_matches_module({"program_path": pp}, args.module):
            continue

        lines.append(
            f"{pp:<40} {total or prog['analysis_funcs']:>6} {done:>6} {queued:>7} {avg:>9}"
        )
        payload_programs.append({
            "program_path": pp,
            "name": prog["program_name"],
            "funcs": total or prog["analysis_funcs"],
            "done": done,
            "queued": queued,
            "completion_pct": pct,
            "avg_match": avg,
        })

    payload = {"programs": payload_programs}
    emit(args, payload, lines=lines)
    return 0


def _run_list_functions(args: argparse.Namespace, config: HarnessConfig) -> int:
    module_filter = args.module
    if not module_filter:
        raise ValueError("--module is required for `list functions`")

    with state.state_db(config.database) as conn:
        all_rows = state.list_targets(conn, limit=100000, target_type="function")
        rows = [r for r in all_rows if target_matches_module(r, module_filter)]

    match_pcts: dict[str, float] = {}
    with state.state_db(config.database) as conn:
        event_rows = conn.execute(
            "SELECT target_id, message FROM events WHERE kind='function-diff'"
        ).fetchall()
    for ev in event_rows:
        try:
            m = re.search(r"match_pct=([\d.]+)", ev["message"])
            if m:
                match_pcts[ev["target_id"]] = float(m.group(1))
        except (ValueError, AttributeError):
            pass

    lines: list[str] = []
    lines.append(f"{'status':<8} {'match':>8} {'address':>12}  function")
    lines.append("-" * 64)

    payload_funcs: list[dict[str, Any]] = []
    for row in rows:
        tid = str(row["id"])
        st = str(row.get("status") or "?")
        entry = str(row.get("entry_hex") or "")
        summary = str(row.get("summary") or "")
        mp = match_pcts.get(tid)
        match_str = f"{mp:.1f}%" if mp is not None else "-"

        lines.append(
            f"{st:<8} {match_str:>8} {entry:>12}  {summary}"
        )
        payload_funcs.append({
            "target_id": tid,
            "status": st,
            "address": entry,
            "name": summary,
            "match_pct": mp,
        })

    payload = {"functions": payload_funcs, "total": len(rows)}
    emit(args, payload, lines=lines)
    return 0


def _match_program(program_path: str, module_filter: str) -> bool:
    return target_matches_module({"program_path": program_path}, module_filter)


def run_report(args: argparse.Namespace) -> int:
    config = _config(args)
    program_filter = args.program

    with state.state_db(config.database) as conn:
        all_rows = state.list_targets(conn, limit=100000, target_type="function")
        rows = [r for r in all_rows if _match_program(str(r.get("program_path") or ""), program_filter)]

    if not rows:
        print(f"no targets found for: {program_filter}")
        return 1

    statuses: dict[str, int] = {}
    match_pcts: dict[str, list[float]] = {}
    for row in rows:
        st = str(row.get("status") or "?")
        statuses[st] = statuses.get(st, 0) + 1

    with state.state_db(config.database) as conn:
        for row in rows:
            tid = str(row["id"])
            events = conn.execute(
                "SELECT message FROM events WHERE target_id=? AND kind='function-diff' ORDER BY created_at DESC LIMIT 1",
                (tid,),
            ).fetchall()
            for ev in events:
                try:
                    m = re.search(r"match_pct=([\d.]+)", ev["message"])
                    if m:
                        match_pcts.setdefault(tid, []).append(float(m.group(1)))
                except (ValueError, AttributeError):
                    pass

    total = len(rows)
    done = statuses.get("done", 0)
    all_matches = [v[0] for v in match_pcts.values() if v]
    avg_match = sum(all_matches) / len(all_matches) if all_matches else 0.0

    lines: list[str] = []
    lines.append(f"program: {program_filter}")
    lines.append("")
    lines.append(f"  total functions: {total}")
    lines.append(f"  done: {done}/{total} ({done/total*100:.1f}%)" if total else "  done: 0")
    for st_key in sorted(statuses):
        if st_key != "done":
            lines.append(f"  {st_key}: {statuses[st_key]}")
    lines.append("")

    if all_matches:
        lines.append(f"  functions with match data: {len(all_matches)}")
        lines.append(f"  average match: {avg_match:.2f}%")
        lines.append(f"  100% matches: {sum(1 for m in all_matches if m >= 100.0)}")
        lines.append("")

    lines.append(f"{'status':<8} {'match':>8} {'address':>12}  function")
    lines.append("-" * 64)
    for row in sorted(rows, key=lambda r: str(r.get("entry_hex") or "")):
        tid = str(row["id"])
        st = str(row.get("status") or "?")
        entry = str(row.get("entry_hex") or "")
        summary = str(row.get("summary") or "")
        mp_list = match_pcts.get(tid, [])
        mp = f"{mp_list[0]:.1f}%" if mp_list else "-"
        lines.append(f"{st:<8} {mp:>8} {entry:>12}  {summary}")

    status_counts = {k: v for k, v in statuses.items()}
    payload = {
        "program": program_filter,
        "total": total,
        "done": done,
        "statuses": status_counts,
        "avg_match": round(avg_match, 2),
        "funcs_with_match": len(all_matches),
        "perfect_matches": sum(1 for m in all_matches if m >= 100.0),
    }
    emit(args, payload, lines=lines)
    return 0
