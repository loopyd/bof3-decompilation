"""Core agent roster context."""

from .base import ContextRequest, ContextSection, _context_profile
from .common import FULL_PATHS, roster_sections, selector_sections


@_context_profile(
    "agents",
    paths=FULL_PATHS,
    stable_paths=(),
)
def agents_context(request: ContextRequest) -> list[ContextSection]:
    return roster_sections(request.root) + selector_sections(
        request.root, request.function, request.mode
    )
