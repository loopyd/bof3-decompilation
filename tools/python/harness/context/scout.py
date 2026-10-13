"""Read-only repository scout prefill."""

from .base import _context_profile


_context_profile(
    "scout", paths=(".agents/skills/bof3-re/references/target-contract.md",)
)(lambda request: ())
