"""Wording-only classifier context."""

from .base import _context_profile


_context_profile("classifier")(lambda request: ())
