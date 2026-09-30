"""BOF3 independent-review context."""

from .base import ContextRequest, ContextSection, _context_profile
from .common import BOF3_PATHS, contract_sections, selector_sections


@_context_profile(
    "review",
    paths=(
        *BOF3_PATHS,
        ".pi/skills/bof3-re/references/review/review-checklist.md",
        ".pi/skills/bof3-re/references/review/sharing-nonmatches.md",
    ),
    accepts_selector=True,
    stable_paths=(
        ".pi/skills/bof3-re/SKILL.md",
        ".pi/skills/bof3-re/references/review/review-checklist.md",
        ".pi/skills/bof3-re/references/review/sharing-nonmatches.md",
    ),
    byte_limit=160_000,
    section_limit=32,
)
def review_context(request: ContextRequest) -> list[ContextSection]:
    sections = selector_sections(request.root, request.function, request.mode)
    return (
        [*contract_sections(request.root, request.role), *sections]
        if request.mode == "stable"
        else sections
    )
