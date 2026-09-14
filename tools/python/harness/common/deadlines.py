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


def check_reserve(seconds: float | None) -> None:
    """Require a positive remaining-time reserve without extending the work cutoff."""
    check_deadline()
    if seconds is None:
        return
    try:
        valid = type(seconds) in {int, float} and math.isfinite(seconds) and seconds > 0
    except OverflowError:
        valid = False
    if not valid:
        raise ValueError("remaining-time reserve must be finite and positive")
    deadline = resolve_deadline()
    if deadline is None:
        raise ValueError("remaining-time reserve requires a bound work deadline")
    if deadline - time.monotonic() < seconds:
        raise DeadlineExpired(
            "insufficient remaining work time for the reserved "
            f"{seconds:g} seconds; defer before source application"
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


@contextmanager
def use_deadline(deadline: float | None = None) -> Iterator[None]:
    """Bind the stricter inherited or supplied cutoff and restore it on every exit."""
    token = _CURRENT.set(resolve_deadline(deadline))
    try:
        check_deadline()
        yield
    finally:
        _CURRENT.reset(token)


def bind_deadline(
    function: Callable[..., _Result], *, argument: str = "deadline"
) -> Callable[..., _Result]:
    """Bind one keyword deadline through nested preflight and forward operations."""

    @wraps(function)
    def execute(*args, **kwargs):
        with use_deadline(kwargs.get(argument)):
            return function(*args, **kwargs)

    return execute
