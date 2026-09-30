"""Rewrite a byte-identical duplicate's source for another target (clone cascade).

Two ranges with identical original bytes need the same C, but the representative's
source names its *own* target's symbols, which the clone target cannot use.  This
owner generates a self-contained clone instead:

* the metadata block's ``@source`` moves to the clone address, and the function is
  renamed to an address-anchored spelling by default;
* every foreign identifier is rewritten to a raw address spelling
  (``func_XXXXXXXX`` / ``D_XXXXXXXX``), which the harness linker resolver binds by
  name-encoded address, and its local declaration is renamed with it;
* a representative that includes a target-local ``*_internal.h`` or calls an
  identifier that is neither declared nor mapped is **declined** with that reason -
  an unverifiable rewrite is never emitted.

The core functions are pure text/byte transforms so they can be unit-tested; the
``bin/harness lift clone`` adapter performs resolution, writing and the
keep-or-revert native gate.  Byte identity is a candidate, never shared ownership:
each clone still needs its own live ``lift gate`` PASS and independent review.
"""

from __future__ import annotations

import re
from dataclasses import dataclass

#: ``name = 0xADDR;`` rows of a composed target symbol map.
_MAP_ROW = re.compile(r"^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;\s*$", re.MULTILINE)
_SOURCE_TAG = re.compile(r"@source\s+0x([0-9A-Fa-f]+)")
_INTERNAL_INCLUDE = re.compile(r'#include\s+["<][^">]*_internal\.h[">]')
_CALL = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
_DECLARATION = re.compile(
    r"^\s*(?:extern\s+)?[A-Za-z_][\w \t*]*(?:\b([A-Za-z_]\w*))\s*(?:\([^;]*\)|\[[^\]]*\])?\s*;",
    re.MULTILINE,
)


class CloneUnsupported(ValueError):
    """The representative cannot be rewritten into a self-contained clone."""


@dataclass(frozen=True)
class ClonePlan:
    """Generated clone text plus the rewrites that produced it."""

    source_text: str
    clone_name: str
    rewrites: tuple[tuple[str, str], ...]


def parse_symbol_map(text: str) -> dict[str, int]:
    """Return ``name -> address`` for every ``name = 0xADDR;`` row in a map."""

    return {match.group(1): int(match.group(2), 16) for match in _MAP_ROW.finditer(text)}


def raw_spelling(name: str, address: int, *, is_function: bool) -> str:
    """Raw address spelling the linker resolver binds by name."""

    prefix = "func_" if is_function else "D_"
    return f"{prefix}{address:08X}"


def byte_identity(representative: bytes, clone: bytes) -> bool:
    """True when both original ranges are the same size and byte-for-byte equal."""

    return len(representative) == len(clone) and representative == clone


def _source_address(text: str) -> int:
    match = _SOURCE_TAG.search(text)
    if match is None:
        raise CloneUnsupported("representative source has no @source address")
    return int(match.group(1), 16)


def generate_clone_source(
    representative_text: str,
    *,
    clone_address: int,
    clone_name: str | None = None,
    symbols: dict[str, int] | None = None,
    function_names: frozenset[str] | None = None,
) -> ClonePlan:
    """Rewrite a representative source into a self-contained clone.

    ``symbols`` maps the representative target's identifiers to addresses and
    ``function_names`` marks which of those are functions; both default to empty,
    which only supports representatives that are already self-contained.
    """

    if _INTERNAL_INCLUDE.search(representative_text):
        raise CloneUnsupported(
            "representative includes a target-local *_internal.h header"
        )
    representative_address = _source_address(representative_text)
    representative_name = _representative_name(representative_text, representative_address)
    if clone_name is None:
        clone_name = f"func_{clone_address:08X}"
    if not re.fullmatch(r"[A-Za-z_]\w*", clone_name):
        raise CloneUnsupported("clone name must be a C identifier")

    symbols = dict(symbols or {})
    functions = function_names or frozenset()
    symbols.pop(representative_name, None)
    symbols.pop(clone_name, None)

    rewrites: list[tuple[str, str]] = []
    text = representative_text.replace(
        f"0x{representative_address:08X}", f"0x{clone_address:08X}"
    )
    # Rename the definition (and any self-recursion) before touching externals.
    text = re.sub(
        rf"\b{re.escape(representative_name)}\b", clone_name, text
    )
    for name, address in sorted(symbols.items()):
        if not re.search(rf"\b{re.escape(name)}\b", text):
            continue
        replacement = raw_spelling(name, address, is_function=name in functions)
        text = re.sub(rf"\b{re.escape(name)}\b", replacement, text)
        rewrites.append((name, replacement))
    _require_self_contained(text, clone_name)
    return ClonePlan(source_text=text, clone_name=clone_name, rewrites=tuple(rewrites))


def _representative_name(text: str, address: int) -> str:
    """The representative's compiled symbol: the function *definition* name.

    A declaration such as ``void helper(void);`` must not win over the definition,
    so the match requires the parameter list to be followed by a body brace.
    """

    for match in _CALL.finditer(text):
        name = match.group(1)
        if name in {"if", "for", "while", "switch", "return", "sizeof"}:
            continue
        definition = re.compile(
            rf"^[A-Za-z_][\w \t*]*\b{re.escape(name)}\s*\([^;{{}}]*\)\s*\{{",
            re.MULTILINE,
        )
        if definition.search(text):
            return name
    raise CloneUnsupported(
        f"representative source has no function definition for 0x{address:08X}"
    )


def _require_self_contained(text: str, clone_name: str) -> None:
    """Every call target must be declared in the file or a raw mapped spelling."""

    declared = set(_DECLARATION.findall(text))
    declared.add(clone_name)
    for match in _CALL.finditer(text):
        name = match.group(1)
        if name in {"if", "for", "while", "switch", "return", "sizeof"}:
            continue
        if name in declared or re.fullmatch(r"(?:func|D)_[0-9A-F]{8}", name):
            continue
        raise CloneUnsupported(
            f"unresolved external {name!r}: not declared, not in the target map"
        )


def wire_splat_text(text: str, offset: int, name: str) -> str:
    """Flip one ``asm`` subsegment at ``offset`` to ``c`` and rename it."""

    pattern = re.compile(
        rf"^(\s*- - {offset}\n\s*- )asm(\n\s*- )([A-Za-z_]\w*)$", re.MULTILINE
    )
    updated, count = pattern.subn(rf"\g<1>c\g<2>{name}", text)
    if count != 1:
        raise CloneUnsupported(
            f"expected exactly one asm subsegment at offset {offset}, found {count}"
        )
    return updated


def wire_symbols_text(text: str, address: int, name: str) -> str:
    """Rename the map row for ``address`` to ``name``."""

    pattern = re.compile(
        rf"^([A-Za-z_]\w*) = 0x{address:08X};$", re.MULTILINE | re.IGNORECASE
    )
    updated, count = pattern.subn(f"{name} = 0x{address:08X};", text)
    if count != 1:
        raise CloneUnsupported(
            f"expected exactly one map row for 0x{address:08X}, found {count}"
        )
    return updated


def wire_manifest_text(text: str, source_path: str) -> str:
    """Claim ``source_path`` in the manifest's source list."""

    if f'"{source_path}"' in text:
        return text
    for key in ("sources", "support_sources"):
        pattern = re.compile(rf"^{key}\s*=\s*\[", re.MULTILINE)
        match = pattern.search(text)
        if match is None:
            continue
        # Prefer a non-empty `sources` list; otherwise use `support_sources`.
        body_match = re.compile(rf"^{key}\s*=\s*\[(.*?)\]", re.MULTILINE | re.DOTALL).search(text)
        if key == "sources" and (body_match is None or not body_match.group(1).strip()):
            continue
        insert_at = match.end()
        return text[:insert_at] + f'\n    "{source_path}",' + text[insert_at:]
    raise CloneUnsupported("manifest has no usable source list")
