"""Plan-oracle prefill."""

from .base import _context_profile


_context_profile(
    "oracle",
    paths=("AGENTS.md", "docs/agents/plan-authoring.md"),
    stable_paths=("docs/agents/plan-authoring.md",),
)(lambda request: ())
