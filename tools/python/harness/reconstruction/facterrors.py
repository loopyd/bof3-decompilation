"""Normalize provenance admission failures at their boundary."""

from __future__ import annotations

import contextlib

from harness.reconstruction import contract


class FactRefused(ValueError):
    """A record, reference or slice failed an admission predicate."""


def require(condition: object, message: str) -> None:
    if not condition:
        raise FactRefused(message)


@contextlib.contextmanager
def refusing():
    """Re-raise a lower owner's refusal as :class:`FactRefused`."""

    try:
        yield
    except FactRefused:
        raise
    except (contract.PreparationRefused, ValueError, KeyError, TypeError) as error:
        raise FactRefused(str(error)) from error
