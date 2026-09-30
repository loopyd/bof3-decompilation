"""Boundary kind evidence read from a target's own generated listing.

Kind must never be inferred from a symbol's *spelling*.  A data range that the
target's own Splat run emitted as ``dlabel``/``enddlabel`` is data no matter what
its map row is called, and a code range that emitted ``glabel`` is code no matter
whether its name says ``func_`` or ``D_``.

The listing under ``out/splat/<target>/asm/<name>.s`` is the reviewed-layout /
original-image artifact for that boundary: it is produced by the target's own
Splat run, so this evidence is index-independent and reproducible from the
repository the same way the reviewed ``splat.yaml`` is.

Historically the kind was decided by spelling alone, which is how 39 proven data
ranges came to be named ``func_<ADDR>`` and booked as *function* naming debt:

* the untracked, locally generated target configs label every code-region
  subsegment ``func_<ADDR>`` by address arithmetic (the fallback in
  ``analysis/carve.py``), without asking Splat whether the range is code;
* ``naming/debt.py::classify_raw_symbol`` then treats ``func_`` as "function"
  and ``D_`` as "data";
* ``domain/layout.py::SplatBoundary.is_function`` treats every ``asm``
  subsegment as a function.

Callers that need a kind for reporting, accounting or candidate selection should
prefer these helpers and fall back to the spelling only when no listing exists.
"""

from __future__ import annotations

import re
from pathlib import Path

#: Directives are matched at line start.  A substring test is wrong here:
#: ``endlabel`` (code) contains ``dlabel``, and ``enddlabel`` (data) contains
#: ``dlabel`` too, so loose matching makes every code listing look like data.
_CODE_DIRECTIVE = re.compile(r"(?m)^\s*glabel\s")
_DATA_DIRECTIVE = re.compile(r"(?m)^\s*(?:dlabel|enddlabel)\s")


def listing_path(root: Path, target: str, name: str) -> Path | None:
    """Return the generated listing for one boundary name, or ``None``.

    Splat writes the listing under the boundary's own symbol name, so the name's
    hex digits keep whatever case the map row used.  A few historical listings
    only exist under the address-derived ``func_<ADDRESS>`` spelling, so both
    forms are tried.
    """

    directory = root / "out" / "splat" / target / "asm"
    candidates = [directory / f"{name}.s"]
    address = _address_of_name(name)
    if address is not None:
        candidates += [
            directory / f"func_{address:08X}.s",
            directory / f"D_{address:08X}.s",
            directory / f"func_{address:x}.s",
            directory / f"D_{address:x}.s",
        ]
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    return None


def _address_of_name(name: str) -> int | None:
    """Return the trailing ``_<HEX8>`` address anchor of a symbol name, if any.

    Handles both ``func_801F7288`` and semantic spellings such as
    ``armEffectBankAndAdvanceStateScenarioScena1100_801F72C4``, so a renamed
    boundary can still fall back to a listing left under its address spelling.
    """

    tail = name.rsplit("_", 1)[-1]
    if len(tail) < 6 or len(tail) > 8:
        return None
    try:
        return int(tail, 16)
    except ValueError:
        return None


def boundary_kind_evidence(root: Path, target: str, name: str) -> str | None:
    """Return ``"code"``, ``"data"`` or ``None`` for one boundary name.

    ``None`` means no listing was found or it carried no decisive directive, so
    the caller must keep its existing fallback rather than guessing.
    """

    path = listing_path(root, target, name)
    if path is None:
        return None
    text = path.read_text(encoding="utf-8", errors="ignore")
    is_function = _CODE_DIRECTIVE.search(text) is not None
    is_data = _DATA_DIRECTIVE.search(text) is not None
    if is_function and not is_data:
        return "code"
    if is_data and not is_function:
        return "data"
    return None
