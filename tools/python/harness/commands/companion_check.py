"""Evidence gate for a caller's declared EMI companion static calls."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from ..canonical import load_map
from ..domain import FUNCTION_ID_HELP, parse_function_id
from ..domain.manifests import CompanionOverlay, load_target_manifests
from ..emi.catalog import verify_declared_companions
from ..io import repo_layout
from ..layout import parse_splat_layout

from ._common import run_main


def _status(ok: bool, detail: str) -> dict[str, object]:
    return {"status": "verified" if ok else "missing", "detail": detail}


def _companion_report(
    root: Path,
    manifests: dict[str, Any],
    caller: str,
    companion: CompanionOverlay,
) -> dict[str, Any]:
    target = manifests[companion.target.value]
    layout = parse_splat_layout(root / target.splat, target.load_address)
    symbols = {
        symbol.address: symbol.canonical_name
        for symbol in load_map(
            root / "config" / "targets" / target.id.value / "symbols.txt"
        )
    }
    boundaries = []
    binding_ok = True
    for call in companion.static_calls:
        boundary = layout.boundary_starting_at(call.target_address)
        mapped = symbols.get(call.target_address)
        if boundary is None or not boundary.is_function:
            boundaries.append(
                {"address": f"0x{call.target_address:08X}", "status": "missing"}
            )
            binding_ok = False
            continue
        boundaries.append(
            {
                "address": f"0x{call.target_address:08X}",
                "status": "verified",
                "name": boundary.name,
                "kind": boundary.kind,
            }
        )
        if mapped != boundary.name:
            binding_ok = False
    abi = _status(
        companion.abi is not None,
        "no reviewed ABI declaration"
        if companion.abi is None
        else companion.abi.prototype,
    )
    binding = _status(
        binding_ok,
        "target-local map owns every reviewed boundary"
        if binding_ok
        else "target-local map/boundary mismatch",
    )
    header = root / manifests[caller].source_dir / "internal.h"
    declaration = (
        companion.abi is not None
        and header.is_file()
        and companion.abi.prototype + ";" in header.read_text(encoding="utf-8")
    )
    consumer = _status(
        declaration,
        "caller internal.h has the reviewed ABI declaration"
        if companion.abi is not None
        else "no reviewed ABI declaration",
    )
    ready = all(item["status"] == "verified" for item in (abi, binding, consumer))
    return {
        "target": companion.target.value,
        "static_calls": [
            {
                "caller_address": f"0x{call.caller_address:08X}",
                "target_address": f"0x{call.target_address:08X}",
            }
            for call in companion.static_calls
        ],
        "boundary": boundaries,
        "abi": abi,
        "companion_binding": binding,
        "consumer_declaration": consumer,
        "ready_to_lift": ready,
    }


def build_report(root: Path, selector: str) -> dict[str, Any]:
    function = parse_function_id(selector)
    manifests = load_target_manifests(root)
    caller = manifests.get(function.target.value)
    if caller is None:
        raise ValueError(f"unknown target: {function.target.value}")
    relations = verify_declared_companions(root, caller)
    caller_layout = parse_splat_layout(root / caller.splat, caller.load_address)
    companions = [
        companion
        for companion in caller.companions
        if any(
            relation["caller"] == caller.id.value
            and relation["companion"] == companion.target.value
            and any(
                (boundary := caller_layout.boundary_containing(call["caller_address"]))
                is not None
                and boundary.virtual_start == function.address
                for call in relation["static_calls"]
            )
            for relation in relations
        )
    ]
    reports = [
        _companion_report(root, manifests, caller.id.value, companion)
        for companion in companions
    ]
    return {
        "schema": "harness.companion-check/v1",
        "caller": f"{function.target.value}@0x{function.address:08X}",
        "companions": reports,
        "ready_to_lift": all(item["ready_to_lift"] for item in reports),
    }


def run(args: argparse.Namespace) -> int:
    report = build_report(args.root.resolve(), args.function)
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["ready_to_lift"] else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="bin/companion-check",
        description="report whether companion evidence makes one lift safe",
    )
    parser.add_argument("function", help=FUNCTION_ID_HELP)
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
