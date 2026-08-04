"""Single-function mission brief composition and printing."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from ..canonical import load_map, sdk_map_path
from ..domain import (
    load_target_manifests,
    parse_function_id,
)
from ..reverse_index import connect, rows


from ._rev_query_graph import _enrich_graph, _function_metrics, _root


def _print_mission(brief: dict[str, Any]) -> None:
    metrics = brief["metrics"]
    risk = brief["risk"]
    print(f"mission {brief['function']} (space={brief['psyq_space']})")
    print(
        f"  source: {brief['source']} "
        f"(exists={brief['source_exists']}, lifted={brief['lifted']})"
    )
    print(f"  splat asm: {brief['splat_asm']} (exists={brief['splat_asm_exists']})")
    print(
        f"  insn={metrics['instruction_count']} cc={metrics['cyclomatic_complexity']} "
        f"loops={metrics['loops']} bb={metrics['basic_blocks']} "
        f"callers={metrics['unique_callers']} callees={metrics['unique_callees']} "
        f"leaf={metrics['leaf_status']} dup_leverage={metrics['duplicate_leverage']}"
    )
    print(
        f"  risk: unresolved_calls={risk['unresolved_calls']} "
        f"metric_missing={risk['metric_missing']} confidence={risk['confidence_band']}"
    )
    if brief["sdk_callees"]:
        names = ", ".join(f"{c['name']}@{c['address']}" for c in brief["sdk_callees"])
        print(f"  SDK callees: {names}")
    if brief["sdk_unresolved"]:
        names = ", ".join(
            f"{c['name']}@{c['address']}" for c in brief["sdk_unresolved"]
        )
        print(f"  SDK unresolved: {names}")
    if brief["callers"]:
        print(
            f"  callers ({len(brief['callers'])}): "
            + ", ".join(str(c["caller"]) for c in brief["callers"][:12])
        )
    if brief["callees"]:
        print(
            f"  callees ({len(brief['callees'])}): "
            + ", ".join(str(c["callee"]) for c in brief["callees"][:12])
        )
    if brief["duplicate_group"]:
        print(
            f"  duplicate group ({len(brief['duplicate_group'])}): "
            + ", ".join(brief["duplicate_group"])
        )


def run_mission(args: argparse.Namespace) -> int:
    """Compose a single-function lifting brief from indexed evidence."""
    root = _root(args)
    function = parse_function_id(args.function)
    target = function.target.value
    address = function.address
    function_id = str(function)

    manifests = load_target_manifests(root)
    if target not in manifests:
        raise ValueError(f"unknown target: {target}")
    manifest = manifests[target]
    space = manifest.psyq_space

    connection = connect(root)
    try:
        metrics = _function_metrics(connection, target)
        _enrich_graph(connection, metrics)
        row = next((item for item in metrics if item["id"] == function_id), None)
        if row is None:
            raise ValueError(f"function not in reverse index: {function_id}")
        callers = rows(
            connection,
            "SELECT caller, printf('0x%08X', callsite) AS callsite "
            "FROM calls WHERE callee = ? ORDER BY caller, callsite",
            (function_id,),
        )
        callees = rows(
            connection,
            "SELECT callee, printf('0x%08X', callsite) AS callsite "
            "FROM calls WHERE caller = ? ORDER BY callsite",
            (function_id,),
        )
        unresolved = rows(
            connection,
            "SELECT printf('0x%08X', target_address) AS target_address, "
            "printf('0x%08X', callsite) AS callsite, kind "
            "FROM unresolved_calls WHERE caller = ? ORDER BY callsite",
            (function_id,),
        )
        duplicate_group: list[str] = []
        if row["duplicate_members"] > 1:
            duplicate_group = [
                member_id
                for (member_id,) in connection.execute(
                    "SELECT function_id FROM duplicate_members WHERE hash = ? "
                    "ORDER BY function_id",
                    (row["exact_sha256"],),
                )
            ]
    finally:
        connection.close()

    sdk_by_address = {
        symbol.address: symbol.canonical_name
        for symbol in load_map(sdk_map_path(root, space))
    }
    sdk_callees = []
    for callee in callees:
        callee_address = int(str(callee["callee"]).rsplit("@", 1)[1], 16)
        name = sdk_by_address.get(callee_address)
        if name is not None:
            sdk_callees.append({"address": f"0x{callee_address:08X}", "name": name})
    sdk_unresolved = []
    for call in unresolved:
        unresolved_address = int(str(call["target_address"]), 16)
        name = sdk_by_address.get(unresolved_address)
        if name is not None:
            sdk_unresolved.append(
                {"address": f"0x{unresolved_address:08X}", "name": name}
            )

    source = Path(manifest.source_dir) / f"func_{address:08X}.c"
    splat_asm = Path("out") / "splat" / target / "asm" / f"func_{address:08X}.s"
    brief = {
        "schema": "bof3.mission/v1",
        "function": function_id,
        "target": target,
        "address": f"0x{address:08X}",
        "psyq_space": space,
        "source": str(source),
        "source_exists": (root / source).is_file(),
        "lifted": row["lifted"],
        "splat_asm": str(splat_asm),
        "splat_asm_exists": (root / splat_asm).is_file(),
        "metrics": {
            key: row[key]
            for key in (
                "size",
                "instruction_count",
                "basic_blocks",
                "cyclomatic_complexity",
                "loops",
                "unique_callers",
                "unique_callees",
                "leaf_status",
                "duplicate_leverage",
                "metric_missing",
                "confidence_band",
            )
        },
        "risk": {
            "unresolved_calls": row["unresolved_calls"],
            "metric_missing": row["metric_missing"],
            "confidence_band": row["confidence_band"],
        },
        "callers": callers,
        "callees": callees,
        "unresolved_calls": unresolved,
        "sdk_callees": sdk_callees,
        "sdk_unresolved": sdk_unresolved,
        "duplicate_group": duplicate_group,
    }
    if args.json:
        print(json.dumps(brief, indent=2, sort_keys=True))
    else:
        _print_mission(brief)
    return 0
