"""External-research prefill."""

from .base import _context_profile


_context_profile(
    "researcher", paths=(".agents/skills/bof3-re/references/target-contract.md",)
)(lambda request: ())
