"""Conservative PsyQ header/type/value extraction for the evidence graph."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import re
from typing import Any


PROTOTYPE_RE = re.compile(
    r"^\s*([A-Za-z_][\w\s\*]+?)\s+([A-Za-z_]\w*)\s*\(([^;]*)\);", re.M
)
DEFINE_RE = re.compile(
    r"^\s*#define\s+([A-Za-z_]\w*)\s+([-+]?0x[0-9A-Fa-f]+|[-+]?\d+)\s*$", re.M
)
STRUCT_RE = re.compile(
    r"typedef\s+struct\s*([A-Za-z_]\w*)?\s*\{(?P<body>.*?)\}\s*([A-Za-z_]\w*)\s*;", re.S
)
FIELD_RE = re.compile(
    r"^\s*([A-Za-z_]\w*(?:\s+\*)?)\s+([A-Za-z_]\w*)(?:\[([0-9]+)\])?\s*;", re.M
)


def _layout_hash(fields: list[dict[str, Any]]) -> str:
    normalized = json.dumps(fields, sort_keys=True, separators=(",", ":")).encode()
    return hashlib.sha256(normalized).hexdigest()


def parse_headers(include_root: Path) -> dict[str, Any]:
    """Parse declarations without compiling or executing SDK code."""

    declarations: list[dict[str, Any]] = []
    types: list[dict[str, Any]] = []
    fields: list[dict[str, Any]] = []
    values: list[dict[str, Any]] = []
    for path in sorted(include_root.rglob("*")):
        if not path.is_file() or path.suffix.lower() not in {".h", ".inc"}:
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        source = str(path.relative_to(include_root))
        for name, value in DEFINE_RE.findall(text):
            values.append(
                {
                    "id": f"{source}:{name}",
                    "name": name,
                    "value": value,
                    "source": source,
                }
            )
        for return_type, name, parameters in PROTOTYPE_RE.findall(text):
            declarations.append(
                {
                    "id": f"{source}:{name}",
                    "name": name,
                    "kind": "function",
                    "return_type": " ".join(return_type.split()),
                    "parameters": parameters.strip(),
                    "source": source,
                }
            )
        for struct_name, body, alias in STRUCT_RE.findall(text):
            name = alias or struct_name
            parsed_fields: list[dict[str, Any]] = []
            offset = 0
            for field_type, field_name, array_size in FIELD_RE.findall(body):
                count = int(array_size or "1")
                field = {
                    "id": f"{source}:{name}.{field_name}",
                    "type_id": f"{source}:{name}",
                    "name": field_name,
                    "offset": offset,
                    "field_type": " ".join(field_type.split()),
                    "array": count,
                }
                parsed_fields.append(field)
                fields.append(field)
                offset += 4 * count
            types.append(
                {
                    "id": f"{source}:{name}",
                    "name": name,
                    "size": offset,
                    "layout_hash": _layout_hash(parsed_fields),
                    "source": source,
                }
            )
    return {
        "schema": "harness.psyq-headers/v1",
        "declarations": declarations,
        "types": types,
        "fields": fields,
        "values": values,
    }


def index_headers(root: Path, version: str) -> Path:
    """Write a deterministic header graph artifact and return its path."""

    include_root = root / "toolchains" / "psyq" / version / "include"
    payload = (
        parse_headers(include_root)
        if include_root.is_dir()
        else {
            "schema": "harness.psyq-headers/v1",
            "declarations": [],
            "types": [],
            "fields": [],
            "values": [],
        }
    )
    output = root / "out" / "index" / "psyq" / version / "headers.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    return output
