"""Bounded native naming gates and frozen external receipt consumption."""

from __future__ import annotations

import json
import os
import secrets
from pathlib import Path

from harness.common.inputs import file_state
from harness.common.process import run_bounded
from harness.domain.receipts import command_records, write_receipt
from harness.domain.symbols import format_map, load_map
from harness.io import unique_object
from harness.naming.data import validate_data_shape
from harness.naming.inputs import (
    applied_scope,
    digest,
    frozen,
    index_digest,
    keys,
    load,
    state,
)
from harness.naming.namespace import selected_evidence_root

SCHEMA = "bof3.naming-postapply/v1"
BUNDLE_KEYS = "schema binding implementation_run_id initial_state final_state index_digest normalization_input graph_state gates review"
OUTPUT_LIMIT = 2 * 1024 * 1024


def collect_function_checks(root: Path, row) -> list[dict]:
    from harness.domain.functions import parse_function_records
    from harness.domain.registry import resolve_function

    facts = row["pre_apply"]["facts"]
    start, end = (int(value, 0) for value in facts["unchanged_range"].split(".."))
    checks = [
        {
            "selector": row["identity"]["selector"],
            "address": start,
            "size": end - start,
            "name": row["new_name"],
            "source": facts["destination"],
        }
    ]
    scope = facts["scope"]
    selectors = {checks[0]["selector"]}
    for relative in sorted(set(scope["source_locations"])):
        if relative == scope["definition"] or not relative.endswith(".c"):
            continue
        source = root / relative
        try:
            records = parse_function_records(source.read_text(encoding="utf-8"))
        except ValueError as error:
            raise ValueError(f"invalid caller metadata: {relative}: {error}") from error
        if len(records) != 1 or records[0].status != "exact":
            raise ValueError(f"function identity requires one exact caller: {relative}")
        record = records[0]
        selector = f"{scope['target']}@0x{record.address:08X}"
        resolved = resolve_function(root, selector)
        if (
            resolved.source != source
            or resolved.compiled_symbol != record.spelling
            or selector in selectors
        ):
            raise ValueError(f"function caller ownership is ambiguous: {relative}")
        selectors.add(selector)
        checks.append(
            {
                "selector": selector,
                "address": record.address,
                "size": None,
                "name": record.spelling,
                "source": relative,
            }
        )
    return checks


def plan(root: Path, target: str, row) -> list[list[str]]:
    selectors = (
        [
            consumer["selector"]
            for consumer in validate_data_shape(row["pre_apply"]["facts"].get("data"))[
                "consumers"
            ]
        ]
        if row["kind"] == "data"
        else [check["selector"] for check in collect_function_checks(root, row)]
    )
    commands = [
        ["bin/symbols", "normalize", target, "--write"],
        ["bin/symbols", "check", target],
        ["bin/splat", target],
        ["bin/build", target],
    ]
    for selector in selectors:
        commands.extend(
            [
                ["bin/asm-diff", selector, "--json", "--detail", "full"],
                ["bin/byte-match", selector, "--json"],
            ]
        )
    return commands


def _execute(root: Path, argv: list[str]) -> dict:
    """Run bounded native naming gates through the shared process owner."""
    return run_bounded(root, argv, timeout=120, output_limit=OUTPUT_LIMIT)


def evidence(root: Path, result: dict, row) -> bool:
    if (
        type(result["exit_code"]) is not int
        or result["exit_code"] != 0
        or result["failure"] is not None
    ):
        return False
    tool = result["argv"][0]
    if tool not in {"bin/asm-diff", "bin/byte-match"}:
        return True
    from harness.domain.manifests import load_target_manifests

    try:
        payload = json.loads(result["stdout"], object_pairs_hook=unique_object)
        facts = row["pre_apply"]["facts"]
        if row["kind"] == "data":
            consumers = [
                consumer
                for consumer in validate_data_shape(facts.get("data"))["consumers"]
                if consumer["selector"] == result["argv"][1]
            ]
            if len(consumers) != 1:
                return False
            consumer = consumers[0]
            start = int(consumer["address"], 0)
            size, name, destination = (
                consumer["size"],
                consumer["name"],
                consumer["source"],
            )
        else:
            checks = [
                check
                for check in collect_function_checks(root, row)
                if check["selector"] == result["argv"][1]
            ]
            if len(checks) != 1:
                return False
            check = checks[0]
            start, size, name, destination = (
                check["address"],
                check["size"],
                check["name"],
                check["source"],
            )
        source = Path(payload["source"])
        if source.is_absolute():
            source = source.relative_to(root.resolve())
        return (
            payload["schema"]
            == (
                "harness.asm-diff-one/v2"
                if tool == "bin/asm-diff"
                else "harness.byte-match-one/v1"
            )
            and payload["status"] == "exact_match"
            and payload["exact_match"] is True
            and payload["byte_match"] is True
            and payload["function"] == name
            and int(payload["address"], 0) == start
            and source.as_posix() == destination
            and type(payload["original_size"]) is int
            and payload["original_size"] > 0
            and payload["original_size"] % 4 == 0
            and (size is None or payload["original_size"] == size)
            and type(payload["current_size"]) is int
            and payload["current_size"] == payload["original_size"]
            and type(payload["size_delta"]) is int
            and payload["size_delta"] == 0
            and Path(payload["original_binary"]).resolve()
            == (
                root / load_target_manifests(root)[facts["scope"]["target"]].binary
            ).resolve()
        )
    except (ValueError, TypeError, KeyError, IndexError):
        return False


def directory(root: Path) -> Path:
    from harness.naming.namespace import canonical_evidence_root

    parent = canonical_evidence_root(
        selected_evidence_root() or root / "out/reviews/evidence"
    )
    path = parent / ("postapply-" + secrets.token_hex(16))
    path.mkdir(parents=True, exist_ok=False)
    return path


def write_json(path: Path, payload) -> None:
    with path.open("x", encoding="utf-8") as stream:
        stream.write(json.dumps(payload, indent=2, sort_keys=True) + "\n")


def receipt(root: Path, path: Path, command: str, target: str, selector, output: str):
    selected = selected_evidence_root()
    return write_receipt(
        root,
        str(path) if selected else path.relative_to(root).as_posix(),
        {
            "command": command,
            "target": target,
            "selector": selector,
            "status": "passed",
            "output": output,
        },
        evidence_root=selected,
    )


def _graph(root: Path, row) -> None:
    # ponytail: Ninja is the supported graph; add Makefile parsing only on demand.
    graph = root / "build/cmake/build.ninja"
    text = graph.read_text()
    if row["kind"] == "data":
        consumers = validate_data_shape(row["pre_apply"]["facts"].get("data"))[
            "consumers"
        ]
        if any(str(root / consumer["source"]) not in text for consumer in consumers):
            raise ValueError("generated Ninja graph omits data consumers")
        return
    old = row["pre_apply"]["facts"]["scope"]["definition"]
    new = row["pre_apply"]["facts"]["destination"]
    if str(root / new) not in text or (old != new and str(root / old) in text):
        raise ValueError("generated Ninja graph does not reflect source migration")


def recheck(root: Path, bundle, row) -> None:
    binding = bundle["binding"]
    if (
        file_state(root / binding["report"]) != binding["report_state"]
        or state(root, binding["target"], row) != bundle["final_state"]
        or index_digest(root) != bundle["index_digest"]
        or (
            bundle["graph_state"] is not None
            and file_state(root / "build/cmake/build.ninja") != bundle["graph_state"]
        )
    ):
        raise ValueError("report, authoritative inputs or staged index drifted")


def produce(
    root: Path,
    target: str,
    report_path: Path,
    transaction: str,
    implementation_run_id: str,
) -> Path:
    if not isinstance(implementation_run_id, str) or not implementation_run_id.strip():
        raise ValueError("implementation run ID is required")
    root = root.resolve()
    _, row, binding = frozen(root, target, report_path, transaction)
    import shutil

    if shutil.which("ninja") is None:
        raise ValueError("native postapply requires Ninja")
    applied_scope(root, target, row)
    if any(
        name in os.environ
        for name in (
            "PSX_CC_DRIVER",
            "PSX_GCC",
            "PSX_AS",
            "PSX_MASPSX",
            "MASPSX_PYTHON",
            "ASPSX_VERSION",
            "CFLAGS",
            "CPPFLAGS",
            "LDFLAGS",
        )
    ):
        raise ValueError(
            "native postapply does not support environment toolchain overrides"
        )
    initial = state(root, target, row)
    index = index_digest(root)
    output = directory(root)
    bundle = {
        "schema": SCHEMA,
        "binding": binding,
        "implementation_run_id": implementation_run_id,
        "initial_state": initial,
        "final_state": initial,
        "index_digest": index,
        "graph_state": None,
        "normalization_input": (
            root / row["pre_apply"]["facts"]["scope"]["map"]
        ).read_text(),
        "gates": [],
        "review": None,
    }
    map_name = row["pre_apply"]["facts"]["scope"]["map"]
    normalized = format_map(load_map(root / map_name)).encode()
    import hashlib

    expected = {
        **initial,
        map_name: {
            "sha256": hashlib.sha256(normalized).hexdigest(),
            "mode": initial[map_name]["mode"],
        },
    }
    try:
        if row["kind"] == "data" and expected != initial:
            raise ValueError("data postapply requires an already normalized map")
        for number, argv in enumerate(plan(root, target, row)):
            recheck(root, bundle, row)
            before = digest(bundle["final_state"])
            result = _execute(root, argv)
            after = state(root, target, row)
            result.update(before=before, after=digest(after))
            write_json(output / f"execution-{number}.json", result)
            if after != (expected if number == 0 else bundle["final_state"]):
                raise ValueError(
                    "native gate changed authoritative inputs outside normalization"
                )
            bundle["final_state"] = after
            if not evidence(root, result, row):
                raise ValueError(
                    f"native gate failed: {' '.join(argv)}; retained {output}"
                )
            record = receipt(
                root,
                output / f"receipt-{number}.json",
                " ".join(argv),
                target,
                argv[1] if argv[0] in {"bin/asm-diff", "bin/byte-match"} else None,
                result["stdout"] + result["stderr"],
            )
            bundle["gates"].append(
                {
                    "execution": str(output / f"execution-{number}.json"),
                    "execution_state": file_state(output / f"execution-{number}.json"),
                    "receipt": record,
                }
            )
        applied_scope(root, target, row)
        _graph(root, row)
        bundle["graph_state"] = file_state(root / "build/cmake/build.ninja")
        recheck(root, bundle, row)
        write_json(output / "gates.json", bundle)
    except BaseException:
        write_json(output / "failed.json", bundle)
        raise
    return output / "gates.json"


def validate_bundle(
    root: Path, target: str, report_path: Path, transaction: str, path: Path
):
    root = root.resolve()
    report, row, binding = frozen(root, target, report_path, transaction)
    bundle = load(path)
    keys(bundle, BUNDLE_KEYS)
    if (
        bundle["schema"] != SCHEMA
        or bundle["binding"] != binding
        or not isinstance(bundle["implementation_run_id"], str)
        or not bundle["implementation_run_id"].strip()
    ):
        raise ValueError("native bundle binding or origin mismatch")
    recheck(root, bundle, row)
    if bundle["graph_state"] is None:
        raise ValueError("native build graph evidence missing")
    commands = plan(root, target, row)
    if not isinstance(bundle["gates"], list) or len(bundle["gates"]) != len(commands):
        raise ValueError("native bundle requires every planned gate")
    initial, final = bundle["initial_state"], bundle["final_state"]
    if not isinstance(initial, dict) or set(initial) != set(final):
        raise ValueError("native initial scope differs from final closure")
    if row["kind"] == "data" and initial != final:
        raise ValueError("data native gates must preserve exact postapply inputs")
    map_name = row["pre_apply"]["facts"]["scope"]["map"]
    if (
        any(initial[p] != final[p] for p in initial if p != map_name)
        or initial[map_name]["mode"] != final[map_name]["mode"]
    ):
        raise ValueError("unsupported normalization transition")
    import hashlib

    from harness.domain.symbols import parse_map

    keys(initial[map_name], "sha256 mode")
    normalized = format_map(parse_map(bundle["normalization_input"]))
    if (
        hashlib.sha256(bundle["normalization_input"].encode()).hexdigest()
        != initial[map_name]["sha256"]
        or hashlib.sha256(normalized.encode()).hexdigest() != final[map_name]["sha256"]
    ):
        raise ValueError("normalization is not the native map formatting transition")
    records = []
    for number, (gate, argv) in enumerate(zip(bundle["gates"], commands)):
        keys(gate, "execution execution_state receipt")
        execution = Path(gate["execution"])
        if (
            execution.parent != path.resolve().parent
            or file_state(execution) != gate["execution_state"]
        ):
            raise ValueError("native execution artifact replaced")
        result = load(execution)
        keys(result, "argv exit_code failure stdout stderr before after")
        if (
            result["argv"] != argv
            or not evidence(root, result, row)
            or result["before"] != digest(initial if number == 0 else final)
            or result["after"] != digest(final)
        ):
            raise ValueError("native execution result mismatches gate/state")
        record = gate["receipt"]
        keys(record, "command status target selector output receipt sha256")
        if (
            record["command"] != " ".join(argv)
            or record["status"] != "passed"
            or record["target"] != target
            or record["selector"]
            != (argv[1] if argv[0] in {"bin/asm-diff", "bin/byte-match"} else None)
            or record["output"] != result["stdout"] + result["stderr"]
        ):
            raise ValueError("receipt differs from native execution")
        records.append(record)
    command_records(records, "native postapply gates", root)
    applied_scope(root, target, row)
    _graph(root, row)
    return report, row, bundle, records
