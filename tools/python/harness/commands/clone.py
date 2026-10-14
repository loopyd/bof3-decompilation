"""``bin/harness lift clone``: generate a self-contained clone of a duplicate lift.

Dry-run by default: it verifies that the clone range is byte-identical to the
representative's original range and emits the rewritten, self-contained source
(foreign identifiers replaced by raw address spellings) without writing anything.

``--apply`` writes the source plus the target's splat/map/manifest wiring and runs
``bin/harness lift gate``; the four files are kept only on a gate PASS and are
restored byte-for-byte otherwise.  Byte identity is a candidate, never shared
ownership: the gate plus independent review remain the acceptance path.
"""

from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path

from harness.common.cli import add_example_argument, add_root_argument, run_main
from harness.domain.functions import collect_lift_metadata
from harness.domain.ids import parse_function_id
from harness.domain.layout import parse_splat_text
from harness.domain.manifests import load_target_manifests
from harness.domain.psx import binary_offset_for, is_psx_exe, validate_psx_header
from harness.domain.registry import resolve_function
from harness.match.clone import (
    CloneUnsupported,
    byte_identity,
    generate_clone_source,
    parse_symbol_map,
    wire_manifest_text,
    wire_splat_text,
    wire_symbols_text,
)

EXAMPLE = (
    "bin/harness lift clone emi/bmagic/magic003/03@0x801F0A28 "
    "--from src/bof3/battle/clearSharedWorkCells.c --json"
)
_SHARED_MAP = "config/targets/shared/symbols.txt"


def _resolve_selector(selector: str) -> tuple[str, int]:
    function = parse_function_id(selector)
    return function.target.value, function.address


def _read_range(root: Path, manifest, address: int, size: int) -> bytes:
    image = (root / manifest.binary).read_bytes()
    if is_psx_exe(image):
        validate_psx_header(image, manifest.load_address, binary_name=manifest.binary)
    payload_offset = binary_offset_for(image)
    offset = address - manifest.load_address
    if size <= 0 or offset < 0 or offset + size > len(image) - payload_offset:
        raise CloneUnsupported(
            f"range 0x{address:08X}+{size} is outside {manifest.binary}"
        )
    start = payload_offset + offset
    return image[start : start + size]


def _reviewed_size(root: Path, manifest, address: int) -> int:
    layout = _projected(root, manifest)
    if layout is None:
        raise CloneUnsupported(f"missing reviewed layout: {manifest.splat}")
    boundaries = [
        boundary
        for boundary in layout.boundaries
        if boundary.is_function and boundary.virtual_start == address
    ]
    if len(boundaries) != 1:
        raise CloneUnsupported(f"expected one reviewed function at 0x{address:08X}")
    size = boundaries[0].virtual_size
    if size is None or size <= 0 or address % 4 or size % 4:
        raise CloneUnsupported(f"missing finite aligned range at 0x{address:08X}")
    return size


def _resolve_representative(root: Path, value: str):
    if "@" in value:
        resolved = resolve_function(root, value)
    else:
        source = (root / value).resolve()
        if not source.is_file():
            raise CloneUnsupported(f"representative source does not exist: {source}")
        records = collect_lift_metadata(source.read_text())
        if not records:
            raise CloneUnsupported("representative source has no @source address")
        if len(records) != 1:
            raise CloneUnsupported(
                "use --from TARGET@0xADDRESS for a grouped representative"
            )
        relative = source.relative_to(root).as_posix()
        owners = [
            manifest
            for manifest in load_target_manifests(root).values()
            if relative in manifest.sources + manifest.support_sources
        ]
        if len(owners) != 1:
            raise CloneUnsupported(
                "representative source requires exactly one target owner"
            )
        address = next(iter(records))
        resolved = resolve_function(root, f"{owners[0].id.value}@0x{address:08X}")
        if resolved.source != source:
            raise CloneUnsupported(
                "representative source disagrees with its function claim"
            )
    if resolved.source is None:
        raise CloneUnsupported("representative function has no claimed source")
    return resolved.source, resolved.id.address, resolved.manifest


def _projected(root: Path, manifest):
    splat = root / manifest.splat
    if not splat.is_file():
        return None
    return parse_splat_text(splat.read_text(), manifest.load_address)


def _function_names(root: Path, manifest) -> frozenset[str]:
    projected = _projected(root, manifest)
    if projected is None:
        return frozenset()
    return frozenset(
        boundary.name
        for boundary in projected.boundaries
        if boundary.is_function and boundary.name
    )


def _symbol_rows(root: Path, manifest) -> dict[str, int]:
    """Compose the representative target's symbols for identifier rewriting."""

    rows: dict[str, int] = {}
    shared = root / _SHARED_MAP
    if shared.is_file():
        rows.update(parse_symbol_map(shared.read_text()))
    projected = _projected(root, manifest)
    for name in projected.symbol_map_paths if projected else ():
        path = root / name
        if path.is_file():
            rows.update(parse_symbol_map(path.read_text()))
    return rows


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    representative, representative_address, representative_manifest = (
        _resolve_representative(root, args.source)
    )
    representative_text = representative.read_text()

    target, clone_address = _resolve_selector(args.selector)
    manifests = load_target_manifests(root)
    clone_manifest = manifests.get(target)
    if clone_manifest is None:
        raise CloneUnsupported(f"unknown clone target: {target}")
    if clone_manifest.binary == representative_manifest.binary:
        raise CloneUnsupported("clone target is the representative's own target")

    size = _reviewed_size(root, representative_manifest, representative_address)
    if _reviewed_size(root, clone_manifest, clone_address) != size:
        raise CloneUnsupported(
            "clone and representative complete ranges differ in size"
        )
    if not byte_identity(
        _read_range(root, representative_manifest, representative_address, size),
        _read_range(root, clone_manifest, clone_address, size),
    ):
        raise CloneUnsupported(
            "clone range is not byte-identical to the representative range"
        )

    plan = generate_clone_source(
        representative_text,
        representative_address=representative_address,
        clone_address=clone_address,
        clone_name=args.name,
        symbols=_symbol_rows(root, representative_manifest),
        function_names=_function_names(root, representative_manifest)
        | _function_names(root, clone_manifest),
    )
    report = {
        "schema": "bof3.clone/v1",
        "status": "cloneable",
        "representative": str(representative),
        "representative_address": f"0x{representative_address:08X}",
        "clone_target": target,
        "clone_address": f"0x{clone_address:08X}",
        "size": size,
        "byte_identical": True,
        "clone_name": plan.clone_name,
        "rewrites": [{"from": old, "to": new} for old, new in plan.rewrites],
        "source_text": plan.source_text,
        "limits": (
            "Byte identity is a candidate, not shared ownership: a clone still needs "
            "its own live lift gate PASS and independent review."
        ),
    }
    if not args.apply:
        if args.output:
            Path(args.output).write_text(plan.source_text)
            report["source_path"] = args.output
        if args.json:
            print(json.dumps(report, indent=2, sort_keys=True))
        else:
            print(f"clone {target}@0x{clone_address:08X} name={plan.clone_name}")
            print(f"  byte-identical to {representative}: {size} bytes")
            for old, new in plan.rewrites:
                print(f"  rewrite {old} -> {new}")
            print(report["limits"])
        return 0
    return _apply(
        root, representative, target, clone_manifest, clone_address, plan, report, args
    )


def _apply(
    root, representative, target, clone_manifest, clone_address, plan, report, args
) -> int:
    """Write the clone wiring, gate it, and keep only on PASS."""

    manifest_path = root / "config" / "targets" / target / "target.toml"
    splat_path = root / "config" / "targets" / target / "splat.yaml"
    symbols_path = root / "config" / "targets" / target / "symbols.txt"
    offset = clone_address - clone_manifest.load_address
    source_path = representative.parent / f"{plan.clone_name}.c"
    if source_path.exists():
        raise CloneUnsupported(f"clone source already exists: {source_path}")
    previous = {
        source_path: None,
        splat_path: splat_path.read_text(),
        symbols_path: symbols_path.read_text(),
        manifest_path: manifest_path.read_text(),
    }
    try:
        source_path.write_text(plan.source_text)
        splat_path.write_text(
            wire_splat_text(previous[splat_path], offset, plan.clone_name)
        )
        symbols_path.write_text(
            wire_symbols_text(previous[symbols_path], clone_address, plan.clone_name)
        )
        relative = source_path.relative_to(root).as_posix()
        manifest_path.write_text(wire_manifest_text(previous[manifest_path], relative))
        gate = subprocess.run(
            [str(root / "bin" / "harness"), "lift", "gate", args.selector, target],
            cwd=root,
            check=False,
            capture_output=True,
            text=True,
        )
        report["gate_returncode"] = gate.returncode
        report["gate_tail"] = (gate.stdout + gate.stderr).strip()[-2000:]
        if gate.returncode == 0:
            report["status"] = "exact"
            report["source_path"] = relative
            if args.json:
                print(json.dumps(report, indent=2, sort_keys=True))
            else:
                print(f"clone {plan.clone_name}: gate PASS (exact) -> {relative}")
            return 0
        _restore(previous)
        report["status"] = "reverted"
        print(
            json.dumps(report, indent=2, sort_keys=True)
            if args.json
            else f"clone {plan.clone_name}: gate FAILED, reverted byte-for-byte"
        )
        return 1
    except Exception:
        _restore(previous)
        raise


def _restore(previous: dict[Path, str | None]) -> None:
    for path, text in previous.items():
        if text is None:
            path.unlink(missing_ok=True)
        else:
            path.write_text(text)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness lift clone", description=__doc__)
    parser.add_argument("selector", help="clone selector TARGET@0xADDRESS")
    parser.add_argument(
        "--from",
        dest="source",
        required=True,
        help="representative lifted source or TARGET@0xADDRESS; grouped sources require the selector",
    )
    parser.add_argument("--name", help="clone function name (default func_<ADDR>)")
    parser.add_argument(
        "--apply",
        action="store_true",
        help="write the wiring and keep it only on a gate PASS",
    )
    parser.add_argument("--json", action="store_true")
    parser.add_argument("-o", "--output", help="write the generated source here")
    add_root_argument(parser)
    add_example_argument(parser, EXAMPLE)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)
