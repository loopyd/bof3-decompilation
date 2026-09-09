"""Propagate a caller's work cutoff without charging restoration to that cutoff."""

from __future__ import annotations

import math
import time
from contextvars import ContextVar
from functools import wraps
from typing import Callable, TypeVar

_CURRENT: ContextVar[float | None] = ContextVar("harness_work_deadline", default=None)
_Result = TypeVar("_Result")


class DeadlineExpired(RuntimeError):
    """Forward work expired; owner recovery must use the retained cleanup tail."""


def validate_deadline(deadline: float | None) -> None:
    if deadline is None:
        return
    try:
        valid = type(deadline) in {int, float} and math.isfinite(deadline)
    except OverflowError:
        valid = False
    if not valid:
        raise ValueError("work deadline must be a finite monotonic timestamp")


def resolve_deadline(deadline: float | None = None) -> float | None:
    validate_deadline(deadline)
    limits = [value for value in (_CURRENT.get(), deadline) if value is not None]
    return min(limits) if limits else None


def check_deadline() -> None:
    deadline = _CURRENT.get()
    if deadline is not None and time.monotonic() >= deadline:
        raise DeadlineExpired(
            "transaction work deadline expired; cleanup requires its retained tail"
        )


def bind_deadline(function: Callable[..., _Result]) -> Callable[..., _Result]:
    """Bind one keyword deadline through nested preflight and forward operations."""

    @wraps(function)
    def execute(*args, **kwargs):
        token = _CURRENT.set(resolve_deadline(kwargs.get("deadline")))
        try:
            check_deadline()
            return function(*args, **kwargs)
        finally:
            _CURRENT.reset(token)

    return execute
