"""Semantic-name grammar shared by naming listings, checks and renames.

A semantic name describes work. Identity lives in the target, map row, Splat
boundary and layout, so a name that spells the target token or the address is
refused. Raw ``func_<ADDRESS>``/``D_<ADDRESS>`` placeholders are not semantic
names and stay legal.
"""

from __future__ import annotations

import re
from dataclasses import dataclass

MAX_TOKENS = 4
MAX_LENGTH = 48

RAW_NAME_RE = re.compile(r"^(?:func|D)_[0-9A-F]{8}$")
ADDRESS_SUFFIX_RE = re.compile(r"_[0-9A-F]{8}$")
TOKEN_RE = re.compile(r"[A-Z]+(?=[A-Z][a-z]|$)|[A-Z]?[a-z]+[0-9]*|[0-9]+")

DENIED_CATEGORIES = frozenset({"func", "helper", "sub", "temp", "misc", "unknown"})
# Camel tokens that only exist because a harvested name was cut off mid-target:
# the family or index spelling of an overlay, never a word a description needs.
# A family word carrying its index (`World04`, `Battle15`) is the target token cut
# short, even though the bare word (`World`, `Battle`) is a word a description needs.
FAMILY_INDEX_RE = re.compile(r"^(?:World|Battle|Boss|Magic|Area|Scena|Plchar|Plp|Bmagic|Commu)\d{2,}$")
ARTIFACT_ROOTS = (
    "Bmagic",
    "Scena",
    "Plchar",
    "Batl",
    "Commu",
    "Sisyou",
    "Shisu",
    "Plp",
    "Area",
)
DATA_ROLE_WORDS = frozenset({"table", "strings", "state"})

# Recorded order of violation codes; the report keeps it stable.
VIOLATION_CODES = (
    "case",
    "length",
    "tokens",
    "category",
    "address",
    "own-token",
    "foreign-token",
    "family-fragment",
    "role",
)


def is_raw_name(name: str) -> bool:
    """True for a placeholder whose only claim is its address.

    A placeholder is still judged: it carries an address, so `name_report` reports
    `address` and `role` for it. This predicate only labels the row for reporting.
    """
    return bool(RAW_NAME_RE.match(name))


def name_tokens(name: str) -> tuple[str, ...]:
    """Split one camelCase name into ordered tokens; digits stay in their token."""
    return tuple(TOKEN_RE.findall(name))


def _camel(text: str) -> str:
    return "".join(part.capitalize() for part in text.split("_"))


def target_tokens(target: str) -> tuple[str, ...]:
    """Return the tokens a target's names must not carry, most specific first.

    Only overlay targets carry one: ``emi/bmagic/magic001/00`` composes
    ``Magic00100`` inside ``BmagicMagic00100``, and ``emi/battle/batl_end/00``
    composes ``BatlEnd00``. Executable targets carry none.
    """
    parts = target.split("/")
    if len(parts) != 4 or parts[0] != "emi":
        return ()
    index = _camel(parts[2]) + parts[3]
    return (index, _camel(parts[1]) + index)


def target_token(target: str) -> str | None:
    """Return the distinctive index-plus-slot token, or None for an executable."""
    tokens = target_tokens(target)
    return tokens[0] if tokens else None


def token_index(targets) -> dict[str, tuple[str, ...]]:
    """Map every overlay target to the tokens it composes."""
    rows = {}
    for target in targets:
        tokens = target_tokens(target)
        if tokens:
            rows[target] = tokens
    return rows


@dataclass(frozen=True)
class NameReport:
    """Schema facts for one name."""

    tokens: tuple[str, ...]
    length: int
    violations: tuple[str, ...]

    @property
    def passed(self) -> bool:
        return not self.violations

    def as_dict(self) -> dict:
        return {
            "tokens": len(self.tokens),
            "length": self.length,
            "violations": list(self.violations),
        }


def name_report(
    name: str,
    *,
    target: str | None = None,
    tokens_by_target: dict[str, tuple[str, ...]] | None = None,
    kind: str | None = None,
) -> NameReport:
    """Report grammar facts for one semantic name.

    ``kind`` is ``"data"`` only where the caller proved it; the role rule is
    skipped for an unresolved kind rather than guessed.
    """
    # The address tail is an identity claim, not part of the description.
    tokens = name_tokens(ADDRESS_SUFFIX_RE.sub("", name))

    found = set()
    # Underscore spellings are the data/API style; camelCase is the lift style.
    if "_" not in name and name[:1].isupper():
        found.add("case")
    if len(name) > MAX_LENGTH:
        found.add("length")
    if len(tokens) > MAX_TOKENS:
        found.add("tokens")
    if tokens and tokens[0].casefold() in DENIED_CATEGORIES:
        found.add("category")
    if ADDRESS_SUFFIX_RE.search(name):
        found.add("address")
    if any(token.startswith(ARTIFACT_ROOTS) for token in tokens) or any(
        FAMILY_INDEX_RE.match(token) for token in tokens
    ):
        found.add("family-fragment")
    folded = name.casefold()
    for owner, composition in sorted((tokens_by_target or {}).items()):
        if any(token.casefold() in folded for token in composition):
            found.add("own-token" if owner == target else "foreign-token")
    if kind == "data" and (not tokens or tokens[-1].casefold() not in DATA_ROLE_WORDS):
        found.add("role")
    return NameReport(
        tokens=tokens,
        length=len(name),
        violations=tuple(code for code in VIOLATION_CODES if code in found),
    )


def describe(report: NameReport) -> str:
    """One-line reason for a refusal, used in findings and refusals."""
    return ",".join(report.violations) or "none"
