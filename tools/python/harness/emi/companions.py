"""Evidence gate for a caller's declared EMI companion static calls."""

from __future__ import annotations

import re
from pathlib import Path
from typing import Any

from ..domain import parse_function_id
from ..domain.claims import manifest_header_paths
from ..domain.manifests import CompanionOverlay, load_target_manifests
from ..domain.layout import parse_splat_layout
from ..domain.symbols import load_map
from .catalog_verify import verify_declared_companions


def _status(ok: bool, detail: str) -> dict[str, object]:
    return {"status": "verified" if ok else "missing", "detail": detail}


def _declarations(source: str) -> set[tuple[str, ...]]:
    source = re.sub(r"\\\r?\n", "", source)
    source = re.sub(
        r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
        " ",
        source,
        flags=re.DOTALL,
    )
    source = re.sub(r"(?m)^[ \t]*#[^\n]*", " ", source)
    return {
        tuple(re.findall(r"[A-Za-z_]\w*|\d+|[^\s]", statement))
        for statement in source.split(";")
        if statement.strip()
    }


def _declares(source: str, prototype: str) -> bool:
    tokens = tuple(re.findall(r"[A-Za-z_]\w*|\d+|[^\s]", prototype))
    return tokens in _declarations(source)


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
        boundary = layout.find_boundary_at(call.target_address)
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
    declaration = companion.abi is not None and any(
        _declares(header.read_text(encoding="utf-8"), companion.abi.prototype)
        for header in manifest_header_paths(root, manifests[caller])
        if header.is_file()
    )
    consumer = _status(
        declaration,
        "caller manifest header has the reviewed ABI declaration"
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


def build_companion_report(root: Path, selector: str) -> dict[str, Any]:
    """Build the evidence gate report for one caller function selector."""
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
                (
                    boundary := caller_layout.find_containing_boundary(
                        call["caller_address"]
                    )
                )
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
