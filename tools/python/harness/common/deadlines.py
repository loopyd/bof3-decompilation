"""Propagate a caller's work cutoff without charging restoration to that cutoff."""

from __future__ import annotations

import math
from pathlib import Path
import time
from contextlib import contextmanager
from contextvars import ContextVar
from functools import wraps
from typing import Callable, Iterator, TypeVar

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


def capture_work_clock() -> dict:
    check_deadline()
    deadline = resolve_deadline()
    if deadline is None:
        raise ValueError("retained work clock requires an original cutoff")
    return {
        "boot_id": Path("/proc/sys/kernel/random/boot_id").read_text().strip(),
        "deadline": deadline,
    }


def validate_work_clock(clock: object) -> float:
    if (
        not isinstance(clock, dict)
        or set(clock) != {"boot_id", "deadline"}
        or clock["boot_id"]
        != Path("/proc/sys/kernel/random/boot_id").read_text().strip()
        or clock["deadline"] is None
    ):
        raise ValueError("retained work clock is invalid or belongs to another boot")
    validate_deadline(clock["deadline"])
    return clock["deadline"]


@contextmanager
def suspend_work_deadline() -> Iterator[None]:
    """Exclude recovery from the work cutoff; the owner retains its cleanup hard-stop."""
    token = _CURRENT.set(None)
    try:
        yield
    finally:
        _CURRENT.reset(token)


def bind_deadline(
    function: Callable[..., _Result], *, argument: str = "deadline"
) -> Callable[..., _Result]:
    """Bind one keyword deadline through nested preflight and forward operations."""

    @wraps(function)
    def execute(*args, **kwargs):
        token = _CURRENT.set(resolve_deadline(kwargs.get(argument)))
        try:
            check_deadline()
            return function(*args, **kwargs)
        finally:
            _CURRENT.reset(token)

    return execute
