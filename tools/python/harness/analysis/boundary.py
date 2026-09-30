"""Index-independent evidence for target-owned function/global boundary conflicts."""

from __future__ import annotations

import hashlib
import re
from dataclasses import asdict
from pathlib import Path

from harness.common.files import read_file
from harness.domain.c_context import declaration_records
from harness.domain.claims import (
    collect_manifest_source_addresses,
    manifest_source_paths,
)
from harness.domain.ids import normalize_target_id, parse_function_id
from harness.domain.layout import parse_splat_text
from harness.domain.manifests import load_target_manifests
from harness.domain.psx import (
    payload_for,
    reviewed_range_digest,
    validate_psx_header,
    is_psx_exe,
)
from harness.domain.symbols import load_target_symbols, map_path
from harness.types.inputs import authored_type_headers

from .probe import probe_textbin


class BoundaryInputs:
    """One bounded read sample, rechecked before returning diagnostic evidence."""

    def __init__(self, root: Path):
        self.root = root
        self.contents: dict[str, bytes] = {}

    def read(self, path: Path) -> bytes:
        name = path.relative_to(self.root).as_posix()
        if name not in self.contents:
            content = read_file(self.root, name, max_bytes=32 * 1024 * 1024)
            if content is None:
                raise ValueError(f"missing boundary input: {name}")
            self.contents[name] = content
        return self.contents[name]

    def text(self, path: Path) -> str:
        return self.read(path).decode("utf-8")

    def verify(self) -> dict[str, str]:
        for name, content in self.contents.items():
            if read_file(self.root, name, max_bytes=32 * 1024 * 1024) != content:
                raise ValueError(f"boundary input changed during collection: {name}")
        return {
            name: hashlib.sha256(data).hexdigest()
            for name, data in sorted(self.contents.items())
        }


def _collect_declarations(root, manifest, inputs):
    rows = {}
    for path, provenance in authored_type_headers(root, manifest, include_shared=False):
        for declaration in declaration_records(inputs.text(path)):
            for item in declaration.declarators:
                if item.kind not in {"extern", "prototype"}:
                    continue
                rows.setdefault(item.name, []).append(
                    {
                        "path": path.relative_to(root).as_posix(),
                        "provenance": provenance,
                        "kind": item.kind,
                        "declaration": declaration.canonical,
                        "diagnostic": item.diagnostic or declaration.diagnostic,
                    }
                )
    return rows


def _collect_references(root, paths, inputs, names):
    if not names:
        return {}
    pattern = re.compile(
        r"\b(?:" + "|".join(re.escape(name) for name in names) + r")\b"
    )
    rows = {name: [] for name in names}
    for path in paths:
        for number, line in enumerate(inputs.text(path).splitlines(), 1):
            for name in sorted(set(pattern.findall(line))):
                rows[name].append(
                    {
                        "path": path.relative_to(root).as_posix(),
                        "line": number,
                        "text": line.strip(),
                    }
                )
    return rows


def collect_boundaries(
    root: Path, selectors: list[str], *, probe: bool = False
) -> dict:
    """Collect conflicts (targets) or an exact boundary (TARGET@ADDRESS), never SQL."""
    inputs = BoundaryInputs(root)
    manifests = load_target_manifests(
        root, read_manifest=lambda name: inputs.read(root / name)
    )
    selected: dict[str, set[int] | None] = {}
    for selector in selectors or sorted(manifests):
        identity = parse_function_id(selector) if "@" in selector else None
        target = (
            identity.target.value if identity else normalize_target_id(selector).value
        )
        if target not in manifests:
            raise ValueError(f"unknown target: {target}")
        if identity is None:
            selected[target] = None
        elif target not in selected:
            selected[target] = {identity.address}
        else:
            current = selected[target]
            if current is not None:
                current.add(identity.address)
    rows = []
    for target, addresses in sorted(selected.items()):
        manifest = manifests[target]
        text = inputs.text(root / manifest.splat)
        layout = parse_splat_text(text, manifest.load_address, origin=manifest.splat)
        inputs.read(
            map_path(root, target)
        )  # A missing local map is not an empty inventory.
        symbols = load_target_symbols(
            root, target, psyq_space=manifest.psyq_space, read_file=inputs.read
        )
        declarations = _collect_declarations(root, manifest, inputs)
        paths = manifest_source_paths(root, manifest)
        claims = collect_manifest_source_addresses(
            root, manifest, read_source=inputs.text
        )
        source_addresses = {address for _, address in claims}
        wanted = []
        for symbol in symbols:
            decls = declarations.get(symbol.canonical_name, [])
            boundary = layout.find_boundary_at(symbol.address)
            reasons = []
            if boundary and boundary.is_function:
                reasons.append("reviewed_range")
            if (
                symbol.canonical_name.startswith("func_")
                or symbol.address in source_addresses
            ):
                reasons.append("mapped_entry")
            if any(d["kind"] == "prototype" for d in decls):
                reasons.append("prototype")
            conflict = bool(reasons and any(d["kind"] == "extern" for d in decls))
            if (addresses is None and conflict) or (
                addresses is not None and symbol.address in addresses
            ):
                wanted.append((symbol, boundary, decls, reasons, conflict))
        if addresses is not None:
            missing = addresses - {s.address for s, *_ in wanted}
            if missing:
                raise ValueError(
                    f"{target}: no mapped symbol at {', '.join(hex(a) for a in sorted(missing))}"
                )
        if not wanted:
            continue
        binary = inputs.read(root / manifest.binary)
        if is_psx_exe(binary):
            validate_psx_header(
                binary, manifest.load_address, binary_name=manifest.binary
            )
        payload = payload_for(
            binary, manifest.load_address, binary_name=manifest.binary
        )
        references = _collect_references(
            root, paths, inputs, [s.canonical_name for s, *_ in wanted]
        )
        for symbol, boundary, decls, reasons, conflict in wanted:
            name = symbol.canonical_name
            row = {
                "target": target,
                "address": f"0x{symbol.address:08X}",
                "symbol": name,
                "conflict": conflict,
                "function_evidence": reasons,
                "declarations": decls,
                "source_claims": [
                    p.relative_to(root).as_posix()
                    for p, a in claims
                    if a == symbol.address
                ],
                "source_mentions": references[name],
                "boundary": asdict(boundary) if boundary else None,
                "original": None,
                "probe": None,
                "resolution": "Review conflicting declarations and function evidence; no automatic reclassification.",
            }
            if boundary:
                chunk = b""
                identity = reviewed_range_digest(
                    payload, boundary.virtual_start, boundary.virtual_end, binary=binary
                )
                if identity:
                    offset = (
                        payload.binary_offset + symbol.address - payload.load_address
                    )
                    chunk = binary[offset : offset + identity[1]]
                    row["original"] = {
                        "binary": manifest.binary,
                        "file_offset": offset,
                        "size": identity[1],
                        "sha256": identity[0],
                        "preview_hex": chunk[:64].hex(),
                        "alignment_mod16": symbol.address % 16,
                        "u32_preview": [
                            f"0x{int.from_bytes(chunk[i : i + 4], 'little'):08X}"
                            for i in range(0, min(len(chunk) - len(chunk) % 4, 64), 4)
                        ],
                    }
                if (
                    conflict
                    and boundary.kind == "asm"
                    and reasons == ["reviewed_range"]
                    and not row["source_claims"]
                    and not any(d["diagnostic"] for d in decls)
                ):
                    row["resolution"] = (
                        "Candidate: asm -> textbin preserves raw .text storage without a function boundary. Review original consumers; declarations alone do not prove the entire range is data."
                    )
                    if probe:
                        if identity is None:
                            raise ValueError(
                                f"{target}:{name}: no finite original range for probe"
                            )
                        row["probe"] = probe_textbin(
                            root, manifest, boundary, inputs, chunk
                        )
                elif probe:
                    row["probe"] = {
                        "status": "unsupported",
                        "reason": "requires an unclaimed asm range with no competing prototype/mapped entry",
                    }
            elif probe:
                row["probe"] = {
                    "status": "unsupported",
                    "reason": "no exact Splat boundary",
                }
            rows.append(row)
    return {
        "schema": "bof3.boundary-evidence/v1",
        "targets": sorted(selected),
        "rows": rows,
        "conflicts": sum(row["conflict"] for row in rows),
        "inputs": inputs.verify(),
        "limits": [
            "No reverse index is opened, refreshed or accepted. This is not a full index preflight.",
            "Declarations and lexical source mentions are research leads, not proof of data semantics.",
            "Input endpoints are rechecked; this is not an atomic snapshot or transaction approval.",
            "A split probe is not a whole-image link/byte-match gate. Live source/config remain unchanged.",
        ],
    }
