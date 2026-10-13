"""Validate closed original-proof values shared by calling and memory contracts."""

from __future__ import annotations

import re


class CallContractRefused(RuntimeError):
    """Native calling proof failed a closed schema or reference predicate."""


def _require(condition: object, message: str) -> None:
    if not condition:
        raise CallContractRefused(message)


def _keys(value: object, expected: set, subject: str) -> dict:
    _require(type(value) is dict and set(value) == expected, f"closed {subject} shape")
    return value


def _rows(value: object, limit: int, subject: str) -> list:
    _require(type(value) is list and len(value) <= limit, f"{subject} count")
    return value


def _integer(value, maximum=(1 << 32), *, optional=False):
    _require(
        (optional and value is None) or (type(value) is int and 0 <= value <= maximum),
        "original contract integer",
    )


def _references(value, maximum, minimum=0):
    values = _rows(value, 4096, "original proof references")
    for entry in values:
        _integer(entry, maximum)
        _require(entry >= minimum, "original proof reference minimum")
    _require(len(set(values)) == len(values), "duplicate original proof reference")
    return values


def _reasons(value, *, nonempty=False):
    values = _rows(value, 4096, "original proof reasons")
    _require(not nonempty or bool(values), "missing original proof reason")
    _require(
        all(
            type(reason) is str and 0 < len(reason.encode()) <= 128 for reason in values
        ),
        "original proof reason text",
    )
    return values


def _complete(value):
    fact = _keys(value, {"kind", "remaining"}, "original fact")
    _require(
        type(fact["kind"]) is str
        and fact["kind"] in {"unknown", "partial", "complete"},
        "original fact kind",
    )
    remaining = _reasons(fact["remaining"], nonempty=fact["kind"] != "complete")
    _require(
        fact["kind"] != "complete" or not remaining,
        "complete original fact has remainder",
    )
    return fact["kind"] == "complete"


def _address(value: object, *, refusal=CallContractRefused) -> int | None:
    if value is None or value == "":
        return None
    if type(value) is not str or not re.fullmatch(r"0x[0-9a-fA-F]{1,9}", value):
        raise refusal("configuration address spelling")
    result = int(value, 16)
    if result > 1 << 32:
        raise refusal("configuration address bound")
    return result
