"""Combined PsyQ SDK symbol map.

One authored line per SDK symbol lists every ``target@address`` location where
that symbol was verified: the resident executable that ships it and any overlay
that carries its own copy. Splat and the generated weak bindings need a single
address per name inside one image, so the projection keeps the target-local copy
when it exists and the resident one otherwise.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re

from .symbols import MapSymbol

SDK_MAP = "config/sdk/psyq.txt"

_TARGET = r"[A-Za-z0-9_.-]+(?:/[A-Za-z0-9_.-]+)+"
_ENTRY = re.compile(r"^\s*(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*=\s*(?P<groups>\S.*?)\s*$")
_GROUP = re.compile(rf"(?P<target>{_TARGET})@(?P<address>0x[0-9A-Fa-f]{{1,8}})")

#: Resident executable that owns the PsyQ library build for each manifest space.
_SPACE_TARGET = {"slus": "exe/slus_004_22", "logo": "exe/logo"}


@dataclass(frozen=True, order=True)
class SdkSymbol:
    """One verified SDK symbol location: a name, the target, and its address."""

    name: str
    target: str
    address: int


def sdk_map_path(root: Path) -> Path:
    return root / SDK_MAP


def space_target(space: str) -> str:
    try:
        return _SPACE_TARGET[space]
    except KeyError as error:
        raise ValueError(f"unsupported psyq space: {space}") from error


def parse_sdk_map(
    text: str, *, source: str = "sdk map", base: Path | None = None
) -> list[SdkSymbol]:
    """Parse the combined map, rejecting a name that repeats a target."""
    symbols: list[SdkSymbol] = []
    seen: set[tuple[str, str]] = set()
    for number, line in enumerate(text.splitlines(), 1):
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        entry = _ENTRY.match(line)
        if entry is None:
            raise ValueError(f"{source}:{number}: invalid SDK symbol line")
        name = entry.group("name")
        groups = _GROUP.findall(entry.group("groups"))
        remainder = _GROUP.sub("", entry.group("groups")).replace(",", "").strip()
        if not groups or remainder:
            raise ValueError(f"{source}:{number}: invalid SDK symbol locations")
        for target, address in groups:
            if (name, target) in seen:
                raise ValueError(
                    f"{source}:{number}: duplicate location for {name} in {target}"
                )
            seen.add((name, target))
            symbols.append(SdkSymbol(name, target, int(address, 16)))
    return symbols


def format_sdk_map(symbols: list[SdkSymbol]) -> str:
    """Render one line per name, with its locations sorted by target and address."""
    grouped: dict[str, list[SdkSymbol]] = {}
    for symbol in symbols:
        grouped.setdefault(symbol.name, []).append(symbol)
    lines = []
    for name in sorted(grouped):
        locations = sorted(grouped[name], key=lambda row: (row.target, row.address))
        lines.append(
            name
            + " = "
            + ", ".join(f"{row.target}@0x{row.address:08X}" for row in locations)
        )
    return "\n".join(lines) + "\n"


def load_sdk_map(path: Path) -> list[SdkSymbol]:
    """Parse the map, treating an absent file as an empty map."""
    if not path.is_file():
        return []
    return parse_sdk_map(path.read_text(encoding="utf-8"), source=str(path))


def write_sdk_map(path: Path, symbols: list[SdkSymbol]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(format_sdk_map(symbols), encoding="utf-8")


def project_addresses(
    symbols: list[SdkSymbol], *, target: str, space: str
) -> frozenset[int]:
    """Addresses the combined map assigns inside one target's image."""
    return frozenset(
        row.address for row in project_symbols(symbols, target=target, space=space)
    )


def project_symbols(
    symbols: list[SdkSymbol], *, target: str, space: str
) -> list[MapSymbol]:
    """Return one address per name for the image Splat or bindings describe.

    A name's target-local copy wins, so an overlay naming its own PsyQ copy does
    not collide with the resident executable's copy of the same routine.
    """
    resident = space_target(space)
    chosen: dict[str, int] = {}
    for symbol in symbols:
        if symbol.target == target:
            chosen[symbol.name] = symbol.address
        elif symbol.target == resident and symbol.name not in chosen:
            chosen[symbol.name] = symbol.address
    return [MapSymbol(address, name) for name, address in sorted(chosen.items())]
