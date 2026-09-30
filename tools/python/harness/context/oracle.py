"""Plan-oracle prefill."""

from .base import _context_profile


_context_profile(
    "oracle",
    paths=("AGENTS.md", ".pi/skills/plans/references/authoring.md"),
    stable_paths=(".pi/skills/plans/references/authoring.md",),
)(lambda request: ())
