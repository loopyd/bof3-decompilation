"""Change-review prefill."""

from .base import _context_profile


_context_profile(
    "oracle",
    paths=("AGENTS.md",),
    stable_paths=(),
)(lambda request: ())
