"""Read-only repository scout prefill."""

from .base import _context_profile


_context_profile("scout", paths=("docs/agents/project-context.md",))(lambda request: ())
