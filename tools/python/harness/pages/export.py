"""Export target-qualified reverse-index counts for the public dashboard."""

from __future__ import annotations

import hashlib
import json
from contextlib import closing
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import quote

from harness.analysis.index import SCHEMA_VERSION, connect_status
from harness.domain.functions import parse_function_records
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.psx import binary_offset_for, payload_for, validate_psx_header
from harness.naming.debt import classify_raw_symbol
from harness.pages.assets import AssetCatalogs
from harness.pages.binary_map import CATEGORIES, IndexedFunction, Span, partition

STATUSES = {
    "exact": "completed",
    "partial": "partial",
    "unlifted": "unlifted",
    "invalid": "unlifted",
}


def collect_targets(root: Path, connection) -> list[dict]:
    targets = {}
    payloads = {}
    for target, binary, load_address in connection.execute(
        "SELECT id, binary, load_address FROM targets ORDER BY id"
    ):
        parts = target.split("/")
        if parts[0] not in {"exe", "emi"}:
            raise ValueError(f"unsupported dashboard target: {target}")
        payloads[target] = payload_for(
            (root / binary).read_bytes(), load_address, binary_name=binary
        )
        targets[target] = {
            "id": target,
            "kind": parts[0],
            "family": parts[1] if parts[0] == "emi" else "executables",
            "archive": "/".join(parts[2:-1]) if parts[0] == "emi" else parts[-1],
            "entry": parts[-1] if parts[0] == "emi" else None,
            "completed": 0,
            "partial": 0,
            "unlifted": 0,
            "invalid": 0,
            "total": 0,
            "symbols": {"named": 0, "raw": 0, "total": 0},
        }
    for target, status, count in connection.execute(
        "SELECT target_id, lift_status, COUNT(*) FROM functions GROUP BY target_id, lift_status"
    ):
        if status not in STATUSES:
            raise ValueError(f"unknown indexed lift status: {status!r}")
        targets[target][STATUSES[status]] += count
        targets[target]["total"] += count
        if status == "invalid":
            targets[target]["invalid"] += count
    for target, address, name in connection.execute(
        "SELECT target_id, address, name FROM symbols"
    ):
        payload = payloads[target]
        if not payload.load_address <= address < payload.payload_end:
            continue
        symbols = targets[target]["symbols"]
        symbols["raw" if classify_raw_symbol(name) else "named"] += 1
        symbols["total"] += 1
    return list(targets.values())


def target_binary(root: Path, manifest, connection, catalogs: AssetCatalogs) -> dict:
    payload = (root / manifest.binary).read_bytes()
    data, spans, source = catalogs.target(manifest, payload)
    header = binary_offset_for(data)
    if header:
        validate_psx_header(data, manifest.load_address, binary_name=manifest.binary)
        spans.append(Span(0, header, "header", "PS-X EXE loader header", 50))
    source_records = {}
    for address, size, reviewed_size, status, source_path in connection.execute(
        "SELECT address, size, reviewed_size, lift_status, source FROM functions WHERE target_id = ?",
        (manifest.id.value,),
    ):
        length = reviewed_size or size
        if not length:
            continue
        start = address - manifest.load_address + header
        category = "code-lifted" if status in {"exact", "partial"} else "code-unlifted"
        source_url = None
        if category == "code-lifted":
            path = (root / source_path).resolve().relative_to(root.resolve())
            if path not in source_records:
                source_records[path] = {
                    record.address: record.line
                    for record in parse_function_records((root / path).read_text())
                }
            line = source_records[path][address]
            source_url = (
                "https://github.com/loopyd/bof3-decompilation/blob/main/"
                f"{quote(path.as_posix())}#L{line}"
            )
        spans.append(
            Span(
                start,
                start + length,
                category,
                "Indexed function",
                10,
                function=IndexedFunction(address, status, source_url),
            )
        )
    layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
    for boundary in layout.boundaries:
        if (
            manifest.kind == "emi"
            and boundary.file_start == 0
            and boundary.kind == "bin"
            and boundary.name == "header"
            and boundary.file_end is not None
            and boundary.file_end >= 4
        ):
            spans.append(Span(0, 4, "header", "Reviewed EMI entry header word", 50))
        if boundary.kind in {"jtbl", ".jtbl"} and boundary.file_end is not None:
            spans.append(
                Span(
                    boundary.file_start + header,
                    boundary.file_end + header,
                    "table",
                    "Reviewed Splat jump table",
                )
            )
    return {
        "size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
        "source": source,
        "spans": partition(data, spans),
    }


def write_snapshot(
    root: Path, output: Path, revision: str, media_root: Path | None = None
) -> dict:
    catalogs = AssetCatalogs(root, media_root or root)
    manifests = load_target_manifests(root)
    with closing(connect_status(root)) as connection:
        targets = collect_targets(root, connection)
        for target in targets:
            target["binary"] = target_binary(
                root, manifests[target["id"]], connection, catalogs
            )
    counts = {
        key: sum(target[key] for target in targets)
        for key in ("completed", "partial", "unlifted", "invalid", "total")
    }
    counts["symbols"] = {
        key: sum(target["symbols"][key] for target in targets)
        for key in ("named", "raw", "total")
    }
    snapshot = {
        "schema": "bof3.decomp-status/v3",
        "binary_categories": CATEGORIES,
        "index_schema": SCHEMA_VERSION,
        "revision": revision,
        "published_at": datetime.now(timezone.utc).isoformat(),
        "counts": counts,
        "targets": targets,
    }
    output.write_text(json.dumps(snapshot, indent=2) + "\n", encoding="utf-8")
    return snapshot
