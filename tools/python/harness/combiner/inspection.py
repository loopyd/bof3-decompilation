"""Read-only per-function source inspection for consolidation planning."""

from __future__ import annotations

import hashlib
from pathlib import Path

from harness.domain.functions import parse_function_records


def inspect_source(root: Path, value: str) -> dict[str, object]:
    """Report lexical members of an explicitly named repository C file."""

    relative = Path(value)
    if (
        relative.is_absolute()
        or relative.as_posix() != value
        or ".." in relative.parts
        or relative.suffix != ".c"
        or relative.parts[:2] != ("src", "bof3")
    ):
        raise ValueError("source must be a canonical src/bof3/ C path")
    root = root.resolve()
    source = root / relative
    if (
        source.resolve() != source
        or not source.is_file()
        or source.stat().st_nlink != 1
    ):
        raise ValueError("source must be a regular single-link repository file")
    data = source.read_bytes()
    try:
        text = data.decode("utf-8")
    except UnicodeError as error:
        raise ValueError("source must be UTF-8 text") from error
    records = parse_function_records(text)
    return {
        "schema": "bof3.combiner-source/v1",
        "source": value,
        "sha256": hashlib.sha256(data).hexdigest(),
        "write_authorized": False,
        "native_verified": False,
        "coverage_verified": False,
        "functions": [
            {
                "address": f"0x{record.address:08X}",
                "kind": record.kind,
                "spelling": record.spelling,
                "line": record.line,
                "status": record.status,
                "metadata_range": [record.metadata_start, record.metadata_end],
                "implementation_range": [
                    record.implementation_start,
                    record.implementation_end,
                ],
            }
            for record in records
        ],
    }
