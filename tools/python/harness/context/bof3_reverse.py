"""BOF3 reverse-engineering context."""

from .base import ContextRequest, ContextSection, _context_profile
from .common import BOF3_PATHS, contract_sections, selector_sections


@_context_profile(
    "reverse",
    paths=(
        *BOF3_PATHS,
        ".pi/skills/bof3-re/references/reverse/mission-protocol.md",
        ".pi/skills/bof3-re/references/regional-leads.md",
    ),
    accepts_selector=True,
    stable_paths=(
        ".pi/skills/bof3-re/SKILL.md",
        ".pi/skills/bof3-re/references/reverse/mission-protocol.md",
    ),
    byte_limit=160_000,
    section_limit=32,
)
def reverse_context(request: ContextRequest) -> list[ContextSection]:
    sections = selector_sections(request.root, request.function, request.mode)
    return (
        [*contract_sections(request.root, request.role), *sections]
        if request.mode == "stable"
        else sections
    )
