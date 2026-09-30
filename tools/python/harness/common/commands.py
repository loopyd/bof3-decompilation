"""Canonical ``bin/harness`` command tokens.

One owner for the mapping between a logical tool id (used by schema
discriminators, receipts and reports) and the canonical ``bin/harness
<domain> <action>`` token sequence that invokes it.  Owners build real
invocations with :func:`command` and read them back with :func:`tool_id`
instead of hard-coding paths.  Every id resolves to an invokable
``<domain> <action>`` route; a retired wrapper that covered several actions
becomes one id per canonical action instead of a dead bare-domain alias.
"""

from __future__ import annotations

from collections.abc import Iterable, Sequence
from pathlib import Path

HARNESS = "bin/harness"

#: Logical tool id -> tokens after ``bin/harness``.  Retained build adapters
#: (``cc``, ``as``, ``ld``, ``ar``, ``nm``, ``objcopy``, ``objdump``,
#: ``ranlib``, ``strip``, ``maspsx``, ``python-env``) are deliberately absent.
TOOL_TOKENS: dict[str, tuple[str, ...]] = {
    "agent-context": ("agent", "context"),
    "analysis-readiness": ("analysis", "readiness"),
    "asm-diff": ("lift", "asm-diff"),
    "bof3-disk": ("media", "disc"),
    "build": ("build",),
    "byte-match": ("lift", "byte-match"),
    "campaign-consolidate": ("lift", "campaign"),
    "combiner": ("combiner",),
    "companion-check": ("lift", "companion-check"),
    "compiler-variants": ("build", "variants"),
    "data-scan": ("analysis", "scan"),
    "decomp-audit": ("decomp", "audit"),
    "decomp-diagnose": ("decomp", "diagnose"),
    "decomp-status": ("lift", "status"),
    "emi-ex": ("emi", "archive"),
    "emi-target": ("emi", "target"),
    "flag-search": ("lift", "flag-search"),
    "index": ("analysis", "index"),
    "lift-gate": ("lift", "gate"),
    "m2c": ("lift", "m2c"),
    "m2ctx": ("lift", "m2ctx"),
    "macro-audit": ("macros",),
    "naming-audit": ("naming",),
    "naming-evidence-run": ("naming", "evidence"),
    "next-lift": ("lift", "next"),
    "package-psx-audio": ("audio", "package"),
    "permute": ("lift", "permute"),
    "plans": ("plans",),
    "promote": ("lift", "promote"),
    "psx-audio": ("audio", "build"),
    "psyq-calls": ("psyq", "calls"),
    "psyq-import": ("psyq", "import"),
    "psyq-proposal": ("psyq", "proposal"),
    "psyq-scan": ("psyq", "scan"),
    "rev-query": ("analysis", "query"),
    "rizin": ("analysis", "rizin"),
    "rz-project": ("analysis", "rz-project"),
    "scratchpad": ("analysis", "scratchpad"),
    "shape-sweep": ("lift", "sweep"),
    "spimdisasm": ("analysis", "spimdisasm"),
    "splat": ("source", "splat"),
    "str-media": ("media", "str"),
    "symbols": ("source", "symbols"),
    "type-audit": ("types",),
}


def tokens(tool: str) -> tuple[str, ...]:
    """Canonical tokens after ``bin/harness`` for one logical tool id."""

    return TOOL_TOKENS[tool]


def command(tool: str, *arguments: object) -> list[str]:
    """Build the canonical argv for one logical tool id."""

    return [HARNESS, *TOOL_TOKENS[tool], *(str(argument) for argument in arguments)]


def command_at(root: Path, tool: str, *arguments: object) -> list[str]:
    """Build an absolute-root canonical argv for one logical tool id."""

    return [
        str(root / "bin" / "harness"),
        *TOOL_TOKENS[tool],
        *(str(argument) for argument in arguments),
    ]


def command_text(tool: str, *arguments: object) -> str:
    """Build the canonical space-joined command text for one tool id."""

    return " ".join(command(tool, *arguments))


def tool_id(argv: Sequence[object]) -> str | None:
    """Return the logical tool id encoded by a canonical argv, if any.

    The longest matching token prefix wins, so ``build variants`` resolves to
    ``compiler-variants`` and ``naming evidence`` to ``naming-evidence-run``.
    """

    items = [str(item) for item in argv]
    if not items or items[0] != HARNESS:
        return None
    rest = items[1:]
    best: str | None = None
    best_length = -1
    for tool, names in TOOL_TOKENS.items():
        if rest[: len(names)] == list(names) and len(names) > best_length:
            best = tool
            best_length = len(names)
    return best


def is_tool(argv: Sequence[object], tool: str) -> bool:
    """True when a canonical argv invokes exactly ``tool``."""

    return tool_id(argv) == tool


def trailing_arguments(argv: Sequence[object]) -> list[str]:
    """Arguments after the canonical ``bin/harness`` prefix."""

    items = [str(item) for item in argv]
    tool = tool_id(items)
    if tool is None:
        return []
    return items[1 + len(TOOL_TOKENS[tool]) :]


def all_tools() -> Iterable[str]:
    """Every logical tool id, in stable declaration order."""

    return TOOL_TOKENS
