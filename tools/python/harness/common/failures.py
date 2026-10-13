"""Passive operation-local failure origins, priority and ordered facts."""

from __future__ import annotations

from collections.abc import Sequence
from typing import Literal

Origin = Literal["cleanup", "ordinary", "policy", "unknown"]


class PolicyFailure(ValueError):
    pass


FailureEvent = tuple[Origin, str, str, Exception]


def retain(
    events: list[FailureEvent],
    error: Exception,
    *,
    phase: str,
    path: object,
    origin: Origin = "unknown",
) -> None:
    if not any(event[3] is error for event in events):
        events.append((origin, phase, str(path), error))


def primary(events: Sequence[FailureEvent]) -> Exception | None:
    for origins in ({"cleanup"}, {"ordinary"}, {"policy", "unknown"}):
        for event in events:
            if event[0] in origins:
                return event[3]
    return None


def facts(events: Sequence[FailureEvent]) -> list[dict]:
    return [
        dict(
            origin=origin,
            phase=phase,
            path=path,
            type=type(error).__name__,
            message=str(error),
            capture_failures=getattr(error, "capture_failures", ()),
        )
        for origin, phase, path, error in events
    ]
