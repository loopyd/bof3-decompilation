"""External-research prefill."""

from .base import _context_profile


_context_profile("researcher", paths=("docs/agents/project-context.md",))(
    lambda request: ()
)
