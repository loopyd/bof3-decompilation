"""Capture complete original function ranges and target bindings from pinned inputs."""

from __future__ import annotations

from pathlib import Path

from harness.common.deadlines import check_deadline
from harness.common.inputs import InputBatch
from harness.common.paths import leaf_stat
from harness.domain.layout import parse_splat_text
from harness.domain.manifests import _parse_manifest
from harness.domain.psx import is_psx_exe, payload_for, validate_psx_header
from harness.domain.symbols import parse_map, validate_symbols

_LIMIT = 64 * 1024 * 1024


def capture_members(root: Path, record: dict, preserved: dict) -> dict:
    """Read only preserved paths; the caller retains the dispatch watch and lease."""
    check_deadline()
    profile = record["profile"]
    if (
        record["fingerprint"] != preserved["preservation_fingerprint"]
        or profile["target"] != preserved["target"]
        or profile["destination"] != preserved["destination"]
    ):
        raise ValueError("comparison members differ from preserved identity")
    with InputBatch(root) as batch:

        def read(name: str, *, required: bool = True) -> bytes:
            if name not in preserved["inputs"]:
                raise ValueError(f"comparison input lacks a preservation pin: {name}")
            before = leaf_stat(root, name)
            if before is not None and before.st_nlink != 1:
                raise ValueError(f"comparison input has multiple links: {name}")
            state, content = batch.read(root / name, max_bytes=_LIMIT)
            after = leaf_stat(root, name)
            if after is not None and after.st_nlink != 1:
                raise ValueError(f"comparison input has multiple links: {name}")
            if state != preserved["inputs"][name] or (required and content is None):
                raise ValueError(f"comparison input differs from preservation: {name}")
            return content if content is not None else b""

        manifest = _parse_manifest(root / record["manifest"], read(record["manifest"]))
        if (
            manifest.id.value != preserved["target"]
            or manifest.splat != record["layout"]
        ):
            raise ValueError("comparison manifest differs from preserved ownership")
        binary = read(manifest.binary)
        if is_psx_exe(binary):
            validate_psx_header(
                binary, manifest.load_address, binary_name=manifest.binary
            )
        payload = payload_for(
            binary, manifest.load_address, binary_name=manifest.binary
        )
        if payload.payload_end > 0x100000000:
            raise ValueError("comparison target payload overflows 32-bit addresses")
        layout = parse_splat_text(
            read(manifest.splat).decode("utf-8"),
            manifest.load_address,
            origin=root / manifest.splat,
        )
        by_address = {}
        local = []
        for name in (
            "config/targets/shared/symbols.txt",
            f"config/sdk/psyq-{manifest.psyq_space}.txt",
            f"config/targets/{manifest.id.value}/symbols.txt",
        ):
            symbols = parse_map(
                read(name, required=name != "config/targets/shared/symbols.txt").decode(
                    "utf-8"
                ),
                source=name,
            )
            by_address.update({symbol.address: symbol for symbol in symbols})
            local = symbols
        validate_symbols(list(by_address.values()), source="comparison target bindings")
        local_by_address = {symbol.address: symbol.canonical_name for symbol in local}
        functions = []
        selectors, names = set(), set()
        for member in profile["members"]:
            for function in member["functions"]:
                check_deadline()
                selector, symbol = function["selector"], function["symbol"]
                target, raw_address = selector.rsplit("@", 1)
                address = int(raw_address, 16)
                if (
                    target != manifest.id.value
                    or selector in selectors
                    or symbol in names
                    or local_by_address.get(address) != symbol
                ):
                    raise ValueError(
                        "comparison member identity is duplicate or unbound"
                    )
                boundaries = [
                    boundary
                    for boundary in layout.boundaries
                    if boundary.virtual_start == address
                ]
                if len(boundaries) != 1:
                    raise ValueError(
                        "comparison member requires one exact reviewed boundary"
                    )
                boundary = boundaries[0]
                size = boundary.virtual_size
                if (
                    boundary.kind != "c"
                    or boundary.source != profile["destination"]
                    or size is None
                    or size < 8
                    or size % 4
                    or address % 4
                    or boundary.file_size != size
                    or not payload.load_address
                    <= address
                    < address + size
                    <= payload.payload_end
                    or boundary.file_start
                    != payload.binary_offset + address - payload.load_address
                    or boundary.file_end != boundary.file_start + size
                ):
                    raise ValueError(
                        "comparison member has an invalid original byte range"
                    )
                original = binary[boundary.file_start : boundary.file_end]
                if len(original) != size:
                    raise ValueError("comparison original bytes are truncated")
                selectors.add(selector)
                names.add(symbol)
                functions.append(
                    {
                        "selector": selector,
                        "symbol": symbol,
                        "address": address,
                        "file_start": boundary.file_start,
                        "original": original,
                    }
                )
        if len(functions) < 2 or sorted(selectors) != preserved["selectors"]:
            raise ValueError("comparison must cover every preserved group member")
    check_deadline()
    return {
        "target": manifest.id.value,
        "binary": manifest.binary,
        "load_address": manifest.load_address,
        "bindings": {
            symbol.canonical_name: symbol.address for symbol in by_address.values()
        },
        "functions": sorted(functions, key=lambda member: member["selector"]),
    }
