"""Implementation-worker prefill."""

from .base import _context_profile


_context_profile(
    "worker",
    paths=(
        "AGENTS.md",
        ".pi/skills/bof3-re/references/tooling-contract.md",
        ".pi/skills/bof3-re/references/target-contract.md",
    ),
    stable_paths=(".pi/skills/bof3-re/references/tooling-contract.md",),
)(lambda request: ())
