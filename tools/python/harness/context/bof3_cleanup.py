"""BOF3 cleanup context."""

from .base import ContextRequest, ContextSection, _context_profile
from .common import FULL_PATHS, selector_sections, target_audit_sections


@_context_profile(
    "cleanup",
    paths=(
        *FULL_PATHS,
        ".pi/skills/bof3-re/references/CLEANUP/RULES.md",
        ".pi/skills/bof3-re/references/CLEANUP/REFACTOR_PLAYBOOK.md",
    ),
    accepts_selector=True,
    accepts_target=True,
    stable_paths=(
        ".pi/skills/bof3-re/references/CLEANUP/RULES.md",
        ".pi/skills/bof3-re/references/CLEANUP/REFACTOR_PLAYBOOK.md",
    ),
    byte_limit=40_000,
    section_limit=16,
)
def cleanup_context(request: ContextRequest) -> list[ContextSection]:
    if request.target is not None:
        return target_audit_sections(request.root, request.target)
    return selector_sections(request.root, request.function, request.mode)
