from __future__ import annotations

import re
from pathlib import Path


_WEAK_SYMBOL_RE = re.compile(r"WEAK_SYMBOL_AT\((\w+),\s*(0x[0-9a-fA-F]+)\)")


def parse_weak_symbol_bindings(text: str) -> dict[str, int]:
    """Parse target-local weak address bindings and reject conflicts."""

    bindings: dict[str, int] = {}
    for match in _WEAK_SYMBOL_RE.finditer(text):
        name = match.group(1)
        address = int(match.group(2), 16)
        previous = bindings.get(name)
        if previous is not None and previous != address:
            raise ValueError(
                f"conflicting weak bindings for {name}: {previous:#x} and {address:#x}"
            )
        bindings[name] = address
    return bindings


def load_weak_symbol_bindings(path: Path) -> dict[str, int]:
    if not path.is_file():
        return {}
    return parse_weak_symbol_bindings(path.read_text(encoding="utf-8"))
