"""Implementation-planner prefill."""

from .base import _context_profile


_context_profile(
    "planner",
    paths=(
        "AGENTS.md",
        "docs/agents/project-context.md",
        "docs/agents/plan-authoring.md",
    ),
    stable_paths=(
        "docs/agents/project-context.md",
        "docs/agents/plan-authoring.md",
    ),
    byte_limit=14_000,
)(lambda request: ())
