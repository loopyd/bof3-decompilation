"""Read-only per-function source inspection for consolidation planning."""

from __future__ import annotations

from pathlib import Path

from harness.common.inputs import InputBatch
from harness.domain.functions import parse_function_records

_LIMIT = 16 * 1024 * 1024


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
    try:
        root = root.resolve()
        with InputBatch(root) as inputs:
            state, data = inputs.read(
                root / relative, max_bytes=_LIMIT, single_link=True
            )
            if data is None:
                raise ValueError("source must be a regular single-link repository file")
            try:
                text = data.decode("utf-8")
            except UnicodeError as error:
                raise ValueError("source must be UTF-8 text") from error
            records = parse_function_records(text)
    except OSError as error:
        raise ValueError(f"source capture failed: {value}: {error}") from error
    return {
        "schema": "bof3.combiner-source/v1",
        "source": value,
        "sha256": state["sha256"],
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
