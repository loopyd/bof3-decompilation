"""Function envelopes in normalized GNU MIPS assembler input."""

from __future__ import annotations

import re
from collections.abc import Iterable

_NAME = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_BOUNDARY = re.compile(r"\s*\.(ent|end)\s+([A-Za-z_][A-Za-z0-9_]*)\s*(?:#.*)?$")
_LABEL = re.compile(r"[A-Za-z_.$][A-Za-z0-9_.$]*:\s*(?:#.*)?$")
_HEADER = {
    ".align",
    ".p2align",
    ".balign",
    ".globl",
    ".global",
    ".weak",
    ".type",
    ".loc",
    ".file",
    ".set",
    ".def",
    ".endef",
    ".verstamp",
    ".stabs",
    ".stabn",
    ".stabd",
    ".ident",
}
_DATA = {".data", ".rdata", ".rodata", ".sdata", ".bss", ".sbss"}


def function_section(name: str) -> str:
    """Return a controlled executable section name for one compiled C symbol."""

    if not _NAME.fullmatch(name):
        raise ValueError(f"unsupported compiled function identifier: {name!r}")
    return f".bof3.text.{name}"


def partition_assembly(text: str, names: Iterable[str]) -> str:
    """Move complete function envelopes into sections after one maspsx translation.

    Instructions, alignment, assembler state and shared storage remain unchanged.
    Unsupported directives reject; this is not source or native acceptance.
    """

    expected = tuple(names)
    if len(expected) < 2 or len(set(expected)) != len(expected):
        raise ValueError("section partition requires at least two distinct functions")
    for name in expected:
        function_section(name)
    active = None
    mode = "text"
    seen: set[str] = set()
    output: list[str] = []
    header: list[str] = []
    for line in text.splitlines(keepends=True):
        stripped = line.strip()
        directive = stripped.split(maxsplit=1)[0] if stripped else ""
        boundary = _BOUNDARY.fullmatch(line.rstrip("\r\n"))
        if directive in {".ent", ".end"}:
            if boundary is None:
                raise ValueError("unsupported assembler function boundary")
            operation, name = boundary.groups()
            if operation == "ent":
                if (
                    active is not None
                    or name in seen
                    or name not in expected
                    or mode != "text"
                ):
                    raise ValueError(
                        f"unowned, duplicate, nested or non-text function: {name}"
                    )
                active = name
                seen.add(name)
                output.append(f'.section {function_section(name)},"ax",@progbits\n')
                output.extend(header)
                header.clear()
            elif active != name or mode != "text":
                raise ValueError(f"unpaired or non-text function end: {name}")
            output.append(line if line.endswith("\n") else line + "\n")
            if operation == "end":
                active = None
                output.append(".text\n")
            continue
        if directive in {
            ".pushsection",
            ".popsection",
            ".previous",
            ".subsection",
            ".size",
        }:
            raise ValueError(f"unsupported section-relative directive: {directive}")
        section = directive if directive == ".text" or directive in _DATA else None
        if directive == ".text" and not re.fullmatch(r"\.text\s*(?:#.*)?", stripped):
            raise ValueError("text subsection operands are unsupported")
        if directive == ".section":
            match = re.fullmatch(
                r"\s*\.section\s+([.A-Za-z_][.A-Za-z0-9_]*)(?:\s*,[^\n]*)?\s*", line
            )
            if match is None:
                raise ValueError("unsupported assembler section directive")
            section = match.group(1)
            if section == ".text" and not re.fullmatch(
                r'\.section\s+\.text(?:\s*,\s*"ax"\s*,\s*@progbits)?\s*(?:#.*)?',
                stripped,
            ):
                raise ValueError("unsupported text section attributes")
            if section != ".text" and not any(
                section == name or section.startswith(name + ".") for name in _DATA
            ):
                raise ValueError(f"unreviewed assembler section: {section}")
        if section is not None:
            output.extend(header)
            header.clear()
            mode = "text" if section == ".text" else "data"
            output.append(
                f'.section {function_section(active)},"ax",@progbits\n'
                if active is not None and mode == "text"
                else line
            )
        elif active is None and mode == "text":
            if (
                stripped
                and not stripped.startswith("#")
                and directive not in _HEADER
                and not _LABEL.fullmatch(stripped)
            ):
                raise ValueError(f"unowned executable text or directive: {stripped}")
            header.append(line)
        else:
            output.append(line)
    if active is not None or seen != set(expected):
        raise ValueError(
            "assembler input does not cover the complete function inventory"
        )
    output.extend(header)
    return "".join(output)
