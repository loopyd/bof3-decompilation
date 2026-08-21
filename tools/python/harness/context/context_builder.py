"""Implementation context-builder prefill."""

from .base import _context_profile


_context_profile(
    "context-builder",
    paths=("AGENTS.md", "docs/agents/project-context.md"),
    stable_paths=("docs/agents/project-context.md",),
)(lambda request: ())
