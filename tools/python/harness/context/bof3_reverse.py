"""BOF3 reverse-engineering context."""

from .base import ContextRequest, ContextSection, _context_profile
from .common import FULL_PATHS, contract_sections, selector_sections


@_context_profile(
    "reverse",
    paths=(
        *FULL_PATHS,
        ".pi/skills/bof3-re/references/REVERSE/MISSION_PROTOCOL.md",
        "docs/reference/bof3-eu/README.md",
    ),
    accepts_selector=True,
    stable_paths=(
        ".pi/skills/bof3-re/SKILL.md",
        ".pi/skills/bof3-re/references/REVERSE/MISSION_PROTOCOL.md",
    ),
    byte_limit=100_000,
    section_limit=24,
)
def reverse_context(request: ContextRequest) -> list[ContextSection]:
    sections = selector_sections(request.root, request.function, request.mode)
    return (
        [*contract_sections(request.root, request.role), *sections]
        if request.mode == "stable"
        else sections
    )
