"""General independent-review prefill."""

from .base import _context_profile


_context_profile(
    "reviewer",
    paths=("AGENTS.md", ".pi/skills/plans/references/authoring.md"),
    stable_paths=(),
)(lambda request: ())
