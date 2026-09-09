"""BOF3 independent-review context."""

from .base import ContextRequest, ContextSection, _context_profile
from .common import FULL_PATHS, contract_sections, selector_sections


@_context_profile(
    "review",
    paths=(
        *FULL_PATHS,
        ".codex/skills/bof3-re/references/REVIEW/REVIEW_CHECKLIST.md",
        ".codex/skills/bof3-re/references/REVIEW/SHARING_NONMATCHES.md",
    ),
    accepts_selector=True,
    stable_paths=(
        ".codex/skills/bof3-re/SKILL.md",
        ".codex/skills/bof3-re/references/REVIEW/REVIEW_CHECKLIST.md",
        ".codex/skills/bof3-re/references/REVIEW/SHARING_NONMATCHES.md",
    ),
    byte_limit=100_000,
    section_limit=24,
)
def review_context(request: ContextRequest) -> list[ContextSection]:
    sections = selector_sections(request.root, request.function, request.mode)
    return (
        [*contract_sections(request.root, request.role), *sections]
        if request.mode == "stable"
        else sections
    )
