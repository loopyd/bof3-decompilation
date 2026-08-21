"""Canonical naming transaction scope, storage, work, and progress facts."""

from __future__ import annotations

import re
import sqlite3
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from ..domain import load_target_manifests, normalize_target_id
from ..domain.claims import manifest_source_paths
from ..domain.layout import parse_splat_layout
from ..domain.naming_debt import address_of
from ..domain.registry import payload_end_for
from ..domain.psx import binary_offset_for
from ..domain.sources import LiftMetadataError, local_include_files
from ..domain.symbols import load_target_symbols, map_path
from ..domain.tags import (
    BEHAVIOR_TAG_RE,
    parse_progress_tags,
    STATUS_TAG_RE,
    parse_source_tag,
    canonical_exact_progress,
)
from .index import connect

_KINDS = {"bss", "rodata", "data"}


def _kind_of(old_name: str) -> str:
    return "function" if old_name.startswith("func_") else "data"


def _owned_files(root: Path, target: str, manifest: Any) -> list[Path]:
    """Every tracked file owned by one target (claims are canonical)."""

    base = root / "config" / "targets" / target
    owned = [
        base / "target.toml",
        map_path(root, target),
        root / manifest.splat,
    ]
    try:
        owned += manifest_source_paths(root, manifest)
    except ValueError:
        pass
    owned += [root / path for path in manifest.headers]
    owned += [base / "reviewed.rz"]
    owned += local_include_files(root, owned)
    return list(dict.fromkeys(owned))


def _cross_target_files(root: Path, target: str, manifest: Any) -> list[Path]:
    files: list[Path] = []
    for other, other_manifest in load_target_manifests(root).items():
        if other == target:
            continue
        base = root / "config" / "targets" / other
        files.append(base / "target.toml")
        files.append(map_path(root, other))
        files.append(root / other_manifest.splat)
        try:
            files += manifest_source_paths(root, other_manifest)
        except ValueError:
            pass
        files += [root / path for path in other_manifest.headers]
        files += [base / "reviewed.rz"]
        files += local_include_files(root, files)
    return list(dict.fromkeys(files))


def _files_containing(root: Path, old_name: str, files: list[Path]) -> list[str]:
    pattern = re.compile(rf"\b{re.escape(old_name)}\b")
    found = []
    for path in files:
        if not path.is_file():
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError):
            continue
        if pattern.search(text):
            found.append(path.relative_to(root).as_posix())
    return sorted(found)


def transaction_scope(
    root: Path,
    target: str,
    old_name: str,
    *,
    address: int | None = None,
    kind: str | None = None,
) -> dict[str, Any]:
    """Derive the exact file set one spelling rename must touch.

    ``binding_locations`` bind the symbol (map entry, WEAK_SYMBOL_AT unit);
    ``source_locations`` define, declare, or reference it (manifest claim,
    definition, headers, same-target callers).  ``cross_target_locations``
    are other targets that share the address: leads, never this
    transaction's scope.
    """

    target = normalize_target_id(target).value
    root = root.resolve()
    manifests = load_target_manifests(root)
    if target not in manifests:
        raise ValueError(f"unknown target: {target}")
    manifest = manifests[target]
    address = address_of(old_name) if address is None else address
    kind = _kind_of(old_name) if kind is None else kind
    if kind not in {"function", "data"}:
        raise ValueError(f"invalid symbol kind: {kind}")
    owned = _owned_files(root, target, manifest)
    owned_rel = set(_files_containing(root, old_name, owned))
    bindings = [path for path in owned_rel if path.endswith("symbols.txt")]
    source_locations = [path for path in owned_rel if path not in bindings]
    manifest_rel = f"config/targets/{target}/target.toml"
    if manifest_rel not in source_locations:
        source_locations.append(manifest_rel)
    source_locations.sort()
    cross = _files_containing(
        root, old_name, _cross_target_files(root, target, manifest)
    )
    try:
        from ..domain.claims import resolve_source_for_paths

        definition = resolve_source_for_paths(
            manifest_source_paths(root, manifest), address
        )
    except (ValueError, OSError):
        definition = None
    map_rel = (
        (root / "config" / "targets" / target / "symbols.txt")
        .relative_to(root)
        .as_posix()
    )
    return {
        "target": target,
        "address": f"0x{address:08X}",
        "kind": kind,
        "old_name": old_name,
        "manifest": manifest_rel,
        "map": map_rel if map_rel in owned_rel else None,
        "splat": manifest.splat if manifest.splat in owned_rel else None,
        "definition": (definition.relative_to(root).as_posix() if definition else None),
        "binding_locations": sorted(bindings),
        "source_locations": source_locations,
        "cross_target_locations": cross,
    }


def canonical_storage(root: Path, target: str, address: int) -> dict[str, Any]:
    """Derive storage from reviewed layout and original binary presence."""

    target = normalize_target_id(target).value
    root = root.resolve()
    manifests = load_target_manifests(root)
    if target not in manifests:
        raise ValueError(f"unknown target: {target}")
    manifest = manifests[target]
    binary = root / manifest.binary
    if not binary.is_file():
        raise ValueError(f"target binary not found: {manifest.binary}")
    binary_offset = binary_offset_for(binary.read_bytes())
    payload_end = payload_end_for(root, manifest)
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    boundary = layout.find_containing_boundary(address)
    mapped = manifest.load_address <= address < payload_end
    splat_kind = boundary.kind if boundary is not None else None
    start = address
    symbols = load_target_symbols(root, target)
    later = sorted(symbol.address for symbol in symbols if symbol.address > address)
    boundary_end = boundary.virtual_end if boundary is not None else None
    end = min(
        later + ([boundary_end] if boundary_end is not None else []),
        default=address + 1,
    )
    widths = {"lb": 1, "lbu": 1, "sb": 1, "lh": 2, "lhu": 2, "sh": 2, "lw": 4, "sw": 4}
    try:
        connection = connect(root)
        opcodes = [
            row[0]
            for row in connection.execute(
                "SELECT DISTINCT opcode FROM data_references WHERE target_id = ? AND address = ?",
                (target, address),
            )
            if row[0] in widths
        ]
        connection.close()
        if opcodes:
            end = min(end, address + max(widths[opcode] for opcode in opcodes))
    except (FileNotFoundError, ValueError):
        pass
    if end <= address:
        end = address + 1
    authority = []
    if boundary is not None:
        authority.append("reviewed_splat")
    if splat_kind in _KINDS:
        kind = splat_kind
    elif mapped:
        kind = "data"
    else:
        kind = "unknown"
    if mapped:
        authority.append("original_binary")
    return {
        "kind": kind,
        "start": f"0x{start:08X}",
        "end": f"0x{end:08X}",
        "file_offset": f"0x{binary_offset + address - manifest.load_address:X}"
        if mapped
        else None,
        "present_in_binary": mapped and kind != "bss",
        "authority": authority,
    }


@dataclass(frozen=True)
class RequiredWorkSnapshot:
    """Target-scoped required-work facts loaded in three bounded queries."""

    target: str
    load_address: int
    payload_end: int | None
    callers: dict[int, tuple[str, ...]]
    callees: dict[int, tuple[str, ...]]
    accesses: dict[int, tuple[str, ...]]
    owners: tuple[tuple[str, int, int], ...]

    def items(self, address: int, kind: str) -> list[dict[str, str]]:
        inside = (
            self.payload_end is not None
            and self.load_address <= address < self.payload_end
        )
        items: list[dict[str, str]] = []
        if kind == "function" and inside:
            for caller in self.callers.get(address, ()):
                items.append(
                    {
                        "id": f"caller:{caller}",
                        "profile": "caller_context",
                        "description": f"resolve caller {caller}",
                    }
                )
            for callee in self.callees.get(address, ()):
                items.append(
                    {
                        "id": f"callee:{callee}",
                        "profile": "callee_body",
                        "description": f"resolve callee {callee}",
                    }
                )
        if kind == "data" and inside:
            for function_id in self.accesses.get(address, ()):
                items.append(
                    {
                        "id": f"access:{function_id}",
                        "profile": "access_context",
                        "description": f"resolve access {function_id}",
                    }
                )
        if not inside:
            for owner_target, owner_address, end in self.owners:
                if owner_target != self.target and owner_address <= address < end:
                    items.append(
                        {
                            "id": f"owner:{owner_target}@{owner_address:08X}",
                            "profile": "owner_resolution",
                            "description": (
                                f"prove runtime owner {owner_target}@0x{owner_address:08X}"
                            ),
                        }
                    )
        return sorted(items, key=lambda item: item["id"])


def required_work_snapshot(
    root: Path,
    target: str,
    manifest: Any,
    connection: sqlite3.Connection,
) -> RequiredWorkSnapshot:
    """Load all required-work inputs for one target from one open index."""

    target = normalize_target_id(target).value
    prefix = f"{target}@"
    callers: dict[int, list[str]] = {}
    callees: dict[int, list[str]] = {}
    for caller, callee in connection.execute(
        "SELECT DISTINCT caller, callee FROM calls "
        "WHERE caller LIKE ? OR callee LIKE ? ORDER BY caller, callee",
        (f"{prefix}%", f"{prefix}%"),
    ):
        if str(callee).startswith(prefix):
            callers.setdefault(int(str(callee).rsplit("@", 1)[1], 16), []).append(
                str(caller)
            )
        if str(caller).startswith(prefix):
            callees.setdefault(int(str(caller).rsplit("@", 1)[1], 16), []).append(
                str(callee)
            )
    accesses: dict[int, list[str]] = {}
    for address, function_id in connection.execute(
        "SELECT DISTINCT address, function_id FROM data_references "
        "WHERE target_id = ? AND function_id IS NOT NULL ORDER BY address, function_id",
        (target,),
    ):
        accesses.setdefault(int(address), []).append(str(function_id))
    owners = tuple(
        (str(owner_target), int(address), int(end))
        for owner_target, address, end in connection.execute(
            "SELECT target_id, address, end FROM function_candidates "
            "WHERE end IS NOT NULL ORDER BY target_id, address"
        )
    )
    payload_end = (
        payload_end_for(root, manifest) if (root / manifest.binary).is_file() else None
    )
    return RequiredWorkSnapshot(
        target,
        manifest.load_address,
        payload_end,
        {key: tuple(values) for key, values in callers.items()},
        {key: tuple(values) for key, values in callees.items()},
        {key: tuple(values) for key, values in accesses.items()},
        owners,
    )


def required_work_items(
    root: Path, target: str, address: int, kind: str
) -> list[dict[str, str]]:
    """Compatibility one-row lookup; bulk callers must reuse a snapshot."""

    target = normalize_target_id(target).value
    manifests = load_target_manifests(root)
    manifest = manifests.get(target)
    if manifest is None:
        raise ValueError(f"unknown target: {target}")
    connection = connect(root)
    try:
        return required_work_snapshot(root, target, manifest, connection).items(
            address, kind
        )
    finally:
        connection.close()


def repair_exact_progress(source: Path) -> str:
    """Rewrite one live-proven exact lift's progress block."""

    text = source.read_text(encoding="utf-8")
    if not STATUS_TAG_RE.search(text):
        return "no-status-tags"
    fixed, changed = canonical_exact_progress(text)
    if not changed:
        return "already-canonical"
    source.write_text(fixed, encoding="utf-8")
    return "repaired"


def progress_metadata_findings(
    root: Path, target: str, manifest: Any
) -> list[dict[str, Any]]:
    """Classify lift metadata debt for one target's claimed sources.

    ``safe_metadata_repair``: exact-status lifts missing canonical progress
    metadata, repairable after live byte-match proof.  Everything else is
    ``review_required``.
    """

    findings: list[dict[str, Any]] = []
    try:
        sources = [
            path
            for path in manifest_source_paths(root, manifest)
            if path.suffix == ".c"
        ]
    except ValueError:
        return [
            {
                "row": "target",
                "class": "review_required",
                "reason": "target has no explicit source claims",
            }
        ]
    from ..domain.tags import MATCH_TAG_RE

    for source in sources:
        row = {
            "row": "target",
            "class": "review_required",
            "file": source.relative_to(root).as_posix(),
        }
        try:
            text = source.read_text(encoding="utf-8")
            address = parse_source_tag(text)
            if address is None:
                if BEHAVIOR_TAG_RE.search(text):
                    row["reason"] = "function metadata has @behavior but no @source"
                    findings.append(row)
                continue
            row["row"] = f"function:func_{address:08X}"
            try:
                parse_progress_tags(text)
            except ValueError:
                status = STATUS_TAG_RE.search(text)
                match = MATCH_TAG_RE.search(text)
                exact_claim = (
                    status is not None
                    and status.group(1) == "exact"
                    and match is not None
                    and match.group(1) in {"100", "100.00"}
                )
                row["reason"] = (
                    "malformed progress metadata (@status/@match/@residual incomplete)"
                )
                if exact_claim:
                    row["class"] = "safe_metadata_repair"
                    row["repair"] = (
                        f"bin/naming-audit prepare {target} --repair; "
                        "live byte-match must prove exact before the rewrite"
                    )
                findings.append(row)
        except LiftMetadataError as error:
            row["reason"] = str(error)
            findings.append(row)
    return findings


__all__ = [
    "canonical_storage",
    "progress_metadata_findings",
    "repair_exact_progress",
    "required_work_items",
    "transaction_scope",
]
