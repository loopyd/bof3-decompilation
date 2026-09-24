"""Original inventory and reviewed coverage without compiling or persisting state."""

from __future__ import annotations

from dataclasses import asdict
import hashlib
from pathlib import Path

import yaml

from harness.domain.functions import collect_lift_metadata

from ..domain.ids import normalize_target_id
from ..domain.layout import parse_splat_layout
from ..domain.psx import is_psx_exe, payload_for, validate_psx_header
from ..domain.symbols import parse_map
from ..domain.tags import parse_behavior_tag
from .coverage_index import read_index, reconcile_index
from .coverage_inputs import (
    CoverageInputs,
    inventory_archives,
    read_manifests,
    read_layout_document,
)


def union_intervals(intervals: list[tuple[int, int]]) -> list[tuple[int, int]]:
    result = []
    for lo, hi in sorted(intervals):
        if result and lo <= result[-1][1]:
            result[-1] = (result[-1][0], max(hi, result[-1][1]))
        else:
            result.append((lo, hi))
    return result


def reviewed_coverage(inputs, manifest, binary):
    document = read_layout_document(inputs, manifest.splat)
    # The owner intentionally ignores some top-level shapes. Reject those here
    # rather than presenting a survivor inventory as complete reviewed coverage.
    previous = -1
    segments = document.get("segments", [])
    for index, segment in enumerate(segments):
        if isinstance(segment, list):
            rows = [segment]
            if len(segment) == 1 and index != len(segments) - 1:
                raise ValueError("Splat terminal must be the final segment")
        elif isinstance(segment, dict) and segment.get("subsegments"):
            rows = segment["subsegments"]
            segment_start = segment.get("start", 0)
            segment_start = (
                int(segment_start, 0)
                if isinstance(segment_start, str)
                else segment_start
            )
            first = rows[0][0]
            first = int(first, 0) if isinstance(first, str) else first
            if first != segment_start:
                raise ValueError("unaccounted Splat segment start/subsegment ownership")
        else:
            raise ValueError("unsupported or unaccounted top-level Splat segment")
        for row in rows:
            offset = int(row[0], 0) if isinstance(row[0], str) else row[0]
            if offset <= previous:
                raise ValueError(
                    "Splat starts/terminal must be unique and in raw order"
                )
            previous = offset
    layout = parse_splat_layout(inputs.root / manifest.splat, manifest.load_address)
    payload = payload_for(binary, manifest.load_address, binary_name=manifest.binary)
    ranges, valid, code, errors = [], [], [], []
    start, end = payload.binary_offset, len(binary)
    for boundary in layout.boundaries:
        row = asdict(boundary)
        row["sha256"] = None
        ranges.append(row)
        lo, hi = boundary.file_start, boundary.file_end
        expected = payload.binary_offset + boundary.virtual_start - payload.load_address
        if expected != lo:
            errors.append(
                f"coordinate interpretation ambiguity: file={lo:#x}, payload-derived={expected:#x}, VRAM={boundary.virtual_start:#x}"
            )
            continue
        if hi is None or not start <= lo < hi <= end:
            errors.append(f"non-file-backed/open/invalid range: {lo:#x}..{hi}")
            continue
        if boundary.kind in {"bss", "sbss"}:
            errors.append(f"non-file-backed range excluded: {lo:#x}")
            continue
        if not boundary.is_function and boundary.kind not in {
            "data",
            "rodata",
            "sdata",
            "bin",
            "pad",
        }:
            errors.append(f"opaque/unsupported range kind: {boundary.kind}@{lo:#x}")
            continue
        valid.append((lo, hi))
        if boundary.is_function:
            code.append((lo, hi))
            row["sha256"] = hashlib.sha256(binary[lo:hi]).hexdigest()
        elif boundary.kind not in {"data", "rodata", "sdata", "bin", "pad"}:
            errors.append(f"opaque/unsupported range kind: {boundary.kind}@{lo:#x}")
    ordered = sorted(valid)
    for left, right in zip(ordered, ordered[1:]):
        if left[1] > right[0]:
            errors.append(f"overlapping reviewed ranges: {left}, {right}")
    covered = union_intervals(valid)
    gaps, cursor = [], start
    for lo, hi in covered:
        if lo > cursor:
            gaps.append([cursor, lo])
        cursor = hi
    if cursor < end:
        gaps.append([cursor, end])
    return layout, {
        "ranges": ranges,
        "payload": asdict(payload),
        "reviewed_file_bytes": sum(hi - lo for lo, hi in covered),
        "executable_range_bytes": sum(hi - lo for lo, hi in union_intervals(code)),
        "uncovered_file_intervals": gaps,
        "excluded_header_bytes": start,
        "discrepancies": errors,
    }


def target_coverage(inputs, target, manifest):
    row = {
        "target": target,
        "shipped_identity": manifest.disc_id,
        "load_address": manifest.load_address,
        "discrepancies": [],
    }
    errors = row["discrepancies"]
    try:
        binary = inputs.read(manifest.binary)
        if is_psx_exe(binary):
            validate_psx_header(
                binary, manifest.load_address, binary_name=manifest.binary
            )
        payload = payload_for(
            binary, manifest.load_address, binary_name=manifest.binary
        )
        row["image"] = {
            **inputs.files[manifest.binary],
            "path": manifest.binary,
            "header_bearing": is_psx_exe(binary),
            "payload": asdict(payload),
        }
        if manifest.kind == "executable":
            original = inputs.read("out/extracted/" + manifest.disc_id)
            if not is_psx_exe(original):
                raise ValueError("original executable lacks PS-X magic")
            validate_psx_header(
                original, manifest.load_address, binary_name=manifest.disc_id
            )
            if binary != original and binary != original[0x800:]:
                errors.append("normalized executable differs from original payload")
        layout, reviewed = reviewed_coverage(inputs, manifest, binary)
        errors.extend(reviewed.pop("discrepancies"))
        row.update(reviewed)
        maps = []
        for path in layout.symbol_map_paths:
            symbols = parse_map(inputs.read(path).decode(), source=path)
            maps.extend(
                {
                    "path": path,
                    "address": symbol.address,
                    "name": symbol.name,
                    "kind": "raw-function"
                    if symbol.name.startswith("func_")
                    else "raw-data"
                    if symbol.is_raw
                    else "unresolved",
                }
                for symbol in symbols
            )
        if not layout.symbol_map_paths:
            errors.append("no selected symbol maps")
        row["map_rows"] = maps
        starts = [b.virtual_start for b in layout.boundaries if b.is_function]
        row["reviewed_starts"] = starts
        authored = []
        for path in manifest.sources:
            text = inputs.read(path).decode()
            records = collect_lift_metadata(text)
            if not records:
                errors.append(f"missing authored function metadata: {path}")
                continue
            for address, metadata in records.items():
                behavior = parse_behavior_tag(metadata)
                if behavior is None:
                    errors.append(
                        f"missing authored function metadata: {path}@0x{address:08X}"
                    )
                    continue
                authored.append(
                    {"address": address, "source": path, "behavior": behavior}
                )
        row["authored"] = authored
        row["excluded_support_sources"] = list(manifest.support_sources)
        if not manifest.has_explicit_sources:
            errors.append(
                "legacy source inventory unavailable: explicit claims required"
            )
        addresses = [item["address"] for item in authored]
        for address in sorted(set(addresses + starts)):
            identity = f"{target}@0x{address:08X}"
            if addresses.count(address) > 1 or starts.count(address) > 1:
                errors.append(f"duplicate authored/reviewed identity: {identity}")
            if address in addresses and address not in starts:
                errors.append(f"authored start not reviewed: {identity}")
            if not any(symbol["address"] == address for symbol in maps):
                errors.append(f"authored/reviewed start missing map: {identity}")
        raw = {
            item["address"]
            for item in maps
            if item["kind"] == "raw-function"
            and item["path"] == f"config/targets/{target}/symbols.txt"
        }
        errors.extend(
            f"raw function not reviewed: {target}@0x{a:08X}"
            for a in sorted(raw - set(starts))
        )
    except (OSError, ValueError, KeyError, TypeError, yaml.YAMLError) as exc:
        errors.append(str(exc))
    return row


def build_coverage(root: Path, target_ids=()) -> dict:
    inputs = CoverageInputs(root)
    report = {
        "schema": "bof3.decomp-coverage/v1",
        "targets": [],
        # Computed after the target loop from target-owned reviewed Splat boundaries;
        # ``denominator_scope`` states what it covers and what remains unknown.
        "full_original_function_denominator": None,
        "denominator_scope": {},
        "denominator_reason": (
            "Configured-target denominator unavailable: coverage collection did not "
            "complete, so no target-owned boundary count can be stated. Residual "
            "unknown: every configured and unconfigured slot lacks accepted coverage "
            "evidence in this report."
        ),
        "blockers": [],
    }
    try:
        manifests = read_manifests(inputs)
        selected = (
            sorted({normalize_target_id(t).value for t in target_ids})
            if target_ids
            else sorted(manifests)
        )
        unknown = set(selected) - manifests.keys()
        if unknown:
            raise ValueError(f"unknown target: {sorted(unknown)[0]}")
        report["scope"] = {
            "selected_targets": selected,
            "configured_targets": sorted(manifests),
            "original_inventory": "all existing out/extracted/BIN declarations; live hashes require accepted reproduction binding",
        }
        try:
            original = inventory_archives(inputs)
        except (OSError, ValueError) as exc:
            original = {
                "archives": None,
                "archive_count": None,
                "manifest_count": None,
                "slots": [],
                "declared_slot_count": None,
                "discrepancies": [str(exc)],
            }
        report["original"] = original
        configured = {m.disc_id for m in manifests.values() if m.kind == "emi"}
        slots = {s["identity"]: s for s in original["slots"]}
        original["configured_slots"] = sorted(configured & slots.keys())
        original["unconfigured_slots"] = sorted(slots.keys() - configured)
        original["missing_configured_slots"] = sorted(configured - slots.keys())
        report["blockers"].extend(original["discrepancies"])
        for target in selected:
            manifest = manifests[target]
            row = target_coverage(inputs, target, manifest)
            slot = slots.get(manifest.disc_id)
            if manifest.kind == "emi":
                if slot is None:
                    row["discrepancies"].append("configured shipped slot missing")
                elif (
                    slot.get("sha256") != row.get("image", {}).get("sha256")
                    or slot["load_address"] != manifest.load_address
                ):
                    row["discrepancies"].append(
                        "configured slot binary/load disagreement"
                    )
            report["targets"].append(row)
        boundaries = sum(
            len(row.get("reviewed_starts") or [])
            for row in report["targets"]
            if "reviewed_starts" in row
        )
        with_boundaries = sum(
            1 for row in report["targets"] if "reviewed_starts" in row
        )
        unreadable = len(report["targets"]) - with_boundaries
        selected_count = len(report["targets"])
        configured_count = len(manifests)
        inventory_complete = original.get("declarations_complete") is True
        unconfigured = len(original.get("unconfigured_slots") or [])
        scope = (
            f"over {selected_count} configured targets"
            if selected_count == configured_count
            else f"over {selected_count} selected targets of {configured_count} configured"
        )
        report["full_original_function_denominator"] = boundaries
        report["denominator_scope"] = {
            "basis": (
                "declared target-owned reviewed Splat function boundaries; opaque "
                "bin/data ranges inside readable targets are not counted as functions"
            ),
            "targets": selected_count,
            "configured_targets": configured_count,
            "targets_with_boundaries": with_boundaries,
            "function_boundaries": boundaries,
            "unconfigured_slots": unconfigured if inventory_complete else None,
        }
        report["denominator_reason"] = (
            f"Configured-target denominator: {boundaries} declared function "
            f"boundaries {scope}. "
            + (
                f"{unreadable} configured target(s) could not be read, so their "
                "boundaries are not counted. "
                if unreadable
                else ""
            )
            + (
                "No readable configured target declared a function boundary, so the "
                "count is 0 rather than unknown. "
                if not with_boundaries
                else ""
            )
            + (
                f"Residual unknown: {unconfigured} unconfigured slots have no "
                "target-owned evidence and are outside this denominator."
                if inventory_complete
                else "Residual unknown: the archive declarations are incomplete, so "
                "the unconfigured-slot count cannot be stated."
            )
        )
        report["index"] = read_index(inputs, selected)
        if not report["index"]["available"]:
            report["blockers"].append("index unavailable: " + report["index"]["reason"])
        for row in report["targets"]:
            row["discrepancies"].extend(reconcile_index(row, report["index"]))
        inputs.verify()
    except (OSError, ValueError, KeyError, TypeError) as exc:
        report["blockers"].append(str(exc))
        if report["full_original_function_denominator"] is not None:
            report["blockers"].append(
                "coverage collection did not complete after the denominator was "
                "computed; the reported count is partial and unverified"
            )
    report["inputs"] = dict(sorted(inputs.files.items()))
    if report["denominator_reason"]:
        report["blockers"].append(report["denominator_reason"])
    return report


def render_coverage(report: dict, detail: str) -> str:
    lines = ["decompilation coverage: incomplete", *report["blockers"]]
    for target in report["targets"]:
        lines.append(
            f"{target['target']}: reviewed={len(target.get('reviewed_starts', []))} authored={len(target.get('authored', []))}"
        )
        lines.extend("  " + error for error in target["discrepancies"])
    if detail == "full":
        lines.append(f"input fingerprints: {len(report['inputs'])}")
    return "\n".join(lines)
