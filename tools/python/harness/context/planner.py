"""Implementation-planner prefill."""

from .base import _context_profile


_context_profile(
    "planner",
    paths=(
        "AGENTS.md",
        ".pi/skills/bof3-re/references/target-contract.md",
        ".pi/skills/plans/references/authoring.md",
    ),
    stable_paths=(
        ".pi/skills/bof3-re/references/target-contract.md",
        ".pi/skills/plans/references/authoring.md",
    ),
    byte_limit=14_000,
)(lambda request: ())
