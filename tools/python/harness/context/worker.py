"""Implementation-worker prefill."""

from .base import _context_profile


_context_profile(
    "worker",
    paths=(
        "AGENTS.md",
        "docs/agents/coding-standards.md",
        "docs/agents/project-context.md",
    ),
    stable_paths=("docs/agents/coding-standards.md",),
)(lambda request: ())
