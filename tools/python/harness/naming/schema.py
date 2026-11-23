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
    "role",
)


def is_raw_name(name: str) -> bool:
    """True for a placeholder whose only claim is its address."""
    return bool(RAW_NAME_RE.match(name))


def name_tokens(name: str) -> tuple[str, ...]:
    """Split one camelCase name into ordered tokens; digits stay in their token."""
    return tuple(TOKEN_RE.findall(name))


def target_token(target: str) -> str | None:
    """Return the composed family token a target's names must not carry.

    Only overlay targets carry one: ``emi/bmagic/magic001/00`` composes
    ``BmagicMagic00100``. Executable targets have no composed token.
    """
    parts = target.split("/")
    if len(parts) != 4 or parts[0] != "emi":
        return None
    return "".join(part.capitalize() for part in parts[1:])


def token_index(targets) -> dict[str, str]:
    """Map every overlay target to the token it composes."""
    rows = {}
    for target in targets:
        token = target_token(target)
        if token is not None:
            rows[target] = token
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
    tokens_by_target: dict[str, str] | None = None,
    kind: str | None = None,
) -> NameReport:
    """Report grammar facts for one semantic name.

    ``kind`` is ``"data"`` only where the caller proved it; the role rule is
    skipped for an unresolved kind rather than guessed.
    """
    if is_raw_name(name):
        return NameReport(tokens=name_tokens(name), length=len(name), violations=())

    # The address tail is an identity claim, not part of the description.
    tokens = name_tokens(ADDRESS_SUFFIX_RE.sub("", name))

    found = set()
    if name[:1].isupper():
        found.add("case")
    if len(name) > MAX_LENGTH:
        found.add("length")
    if len(tokens) > MAX_TOKENS:
        found.add("tokens")
    if tokens and tokens[0].casefold() in DENIED_CATEGORIES:
        found.add("category")
    if ADDRESS_SUFFIX_RE.search(name):
        found.add("address")
    folded = name.casefold()
    for owner, token in sorted((tokens_by_target or {}).items()):
        if token.casefold() in folded:
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
