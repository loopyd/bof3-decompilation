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

from harness.domain.functions import collect_lift_metadata, parse_function_records

#: ``name = 0xADDR;`` rows of a composed target symbol map.
_MAP_ROW = re.compile(
    r"^\s*([A-Za-z_]\w*)\s*=\s*(0x[0-9A-Fa-f]+)\s*;\s*$", re.MULTILINE
)
_SOURCE_TAG = re.compile(r"@source\s+0x([0-9A-Fa-f]+)")
_INTERNAL_INCLUDE = re.compile(r'#include\s+["<][^">]*_internal\.h[">]')
_CALL = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
#: Spans that are not code: block/line comments and string or character literals.
_NON_CODE = re.compile(
    r"/\*.*?\*/|//[^\n]*|\"(?:\\.|[^\"\\\\])*\"|'(?:\\.|[^'\\\\])*'",
    re.DOTALL,
)
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

    return {
        match.group(1): int(match.group(2), 16) for match in _MAP_ROW.finditer(text)
    }


def raw_spelling(name: str, address: int, *, is_function: bool) -> str:
    """Raw address spelling the linker resolver binds by name."""

    prefix = "func_" if is_function else "D_"
    return f"{prefix}{address:08X}"


def byte_identity(representative: bytes, clone: bytes) -> bool:
    """True when both original ranges are the same size and byte-for-byte equal."""

    return len(representative) == len(clone) and representative == clone


def _select_representative(text: str, address: int | None) -> tuple[str, int, str]:
    records = collect_lift_metadata(text)
    if not records:
        raise CloneUnsupported("representative source has no @source address")
    if address is None:
        if len(records) != 1:
            raise CloneUnsupported(
                "grouped representative requires an explicit address"
            )
        address = next(iter(records))
    if address not in records:
        raise CloneUnsupported(f"representative has no function at 0x{address:08X}")
    if len(records) == 1:
        return text, address, _representative_name(text, address)
    pieces: list[str] = []
    cursor = 0
    name = ""
    for record in parse_function_records(text):
        pieces.append(text[cursor : record.metadata_start])
        if record.address == address:
            if record.kind != "function":
                raise CloneUnsupported("grouped clone requires an ordinary function")
            name = record.spelling
            pieces.append(text[record.metadata_start : record.implementation_end])
        cursor = record.implementation_end
    pieces.append(text[cursor:])
    return "".join(pieces), address, name


def generate_clone_source(
    representative_text: str,
    *,
    representative_address: int | None = None,
    clone_address: int,
    clone_name: str | None = None,
    symbols: dict[str, int] | None = None,
    function_names: frozenset[str] | None = None,
) -> ClonePlan:
    """Rewrite a representative source into a self-contained clone.

    ``symbols`` maps the representative target's identifiers to addresses and
    ``function_names`` marks which of those are functions; both default to empty,
    which only supports representatives that are already self-contained.
    Grouped representatives require an explicit address and retain only that
    attached function plus the source's common declarations and includes.
    """

    if _INTERNAL_INCLUDE.search(representative_text):
        raise CloneUnsupported(
            "representative includes a target-local *_internal.h header"
        )
    representative_text, representative_address, representative_name = (
        _select_representative(representative_text, representative_address)
    )
    if clone_name is None:
        clone_name = f"func_{clone_address:08X}"
    if not re.fullmatch(r"[A-Za-z_]\w*", clone_name):
        raise CloneUnsupported("clone name must be a C identifier")

    symbols = dict(symbols or {})
    functions = function_names or frozenset()
    symbols.pop(representative_name, None)
    symbols.pop(clone_name, None)

    rewrites: list[tuple[str, str]] = []
    tags = [
        match
        for match in _SOURCE_TAG.finditer(representative_text)
        if int(match.group(1), 16) == representative_address
    ]
    if len(tags) != 1:
        raise CloneUnsupported("representative requires one unambiguous source tag")
    tag = tags[0]
    text = (
        representative_text[: tag.start(1)]
        + f"{clone_address:08X}"
        + representative_text[tag.end(1) :]
    )
    # Rename the definition (and any self-recursion) before touching externals.
    name_pattern = re.compile(rf"\b{re.escape(representative_name)}\b")
    text = _substitute_in_code(text, name_pattern, clone_name)
    for name, address in sorted(symbols.items()):
        pattern = re.compile(rf"\b{re.escape(name)}\b")
        if not _matches_in_code(text, pattern):
            continue
        replacement = raw_spelling(name, address, is_function=name in functions)
        text = _substitute_in_code(text, pattern, replacement)
        rewrites.append((name, replacement))
    _require_self_contained(text, clone_name)
    return ClonePlan(source_text=text, clone_name=clone_name, rewrites=tuple(rewrites))


def _representative_name(text: str, address: int) -> str:
    """The representative's compiled symbol: the function *definition* name.

    A declaration such as ``void helper(void);`` must not win over the definition,
    so the match requires the parameter list to be followed by a body brace.
    """

    code = _code_only(text)
    definitions: set[str] = set()
    for match in _CALL.finditer(code):
        name = match.group(1)
        if name in {"if", "for", "while", "switch", "return", "sizeof"}:
            continue
        definition = re.compile(
            rf"^[A-Za-z_][\w \t*]*\b{re.escape(name)}\s*\([^;{{}}]*\)\s*\{{",
            re.MULTILINE,
        )
        if definition.search(code):
            definitions.add(name)
    if len(definitions) == 1:
        return next(iter(definitions))
    raise CloneUnsupported(
        f"representative source requires one unambiguous function definition for 0x{address:08X}"
    )


def _substitute_in_code(text: str, pattern: re.Pattern[str], repl: str) -> str:
    """Rewrite identifier matches in code only, never in comments or literals.

    Symbol names are ordinary English words often enough (``read``, ``size``) that a
    blanket substitution also edits prose in comments and data in string literals.
    Non-code spans keep their original text; the metadata ``@source`` address has
    its own deliberate edit.
    """

    pieces: list[str] = []
    cursor = 0
    for span in _NON_CODE.finditer(text):
        pieces.append(pattern.sub(repl, text[cursor : span.start()]))
        pieces.append(span.group(0))
        cursor = span.end()
    pieces.append(pattern.sub(repl, text[cursor:]))
    return "".join(pieces)


def _matches_in_code(text: str, pattern: re.Pattern[str]) -> bool:
    """Whether ``pattern`` matches outside comments and literals."""

    cursor = 0
    for span in _NON_CODE.finditer(text):
        if pattern.search(text[cursor : span.start()]):
            return True
        cursor = span.end()
    return pattern.search(text[cursor:]) is not None


def _code_only(text: str) -> str:
    """Blank out comments and literals so structural scans read code alone.

    A comment mentioning a helper, or a string holding call-like text, otherwise
    looks like a declaration or a call to the scans below.
    """

    pieces: list[str] = []
    cursor = 0
    for span in _NON_CODE.finditer(text):
        pieces.append(text[cursor : span.start()])
        pieces.append(" " * (span.end() - span.start()))
        cursor = span.end()
    pieces.append(text[cursor:])
    return "".join(pieces)


def _require_self_contained(text: str, clone_name: str) -> None:
    """Every call target must be declared in the file or a raw mapped spelling."""

    code = _code_only(text)
    declared = set(_DECLARATION.findall(code))
    declared.add(clone_name)
    for match in _CALL.finditer(code):
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
        body_match = re.compile(
            rf"^{key}\s*=\s*\[(.*?)\]", re.MULTILINE | re.DOTALL
        ).search(text)
        if key == "sources" and (body_match is None or not body_match.group(1).strip()):
            continue
        insert_at = match.end()
        return text[:insert_at] + f'\n    "{source_path}",' + text[insert_at:]
    raise CloneUnsupported("manifest has no usable source list")
