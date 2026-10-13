"""General independent-review prefill."""

from .base import _context_profile


_context_profile(
    "reviewer",
    paths=("AGENTS.md",),
    stable_paths=(),
)(lambda request: ())
