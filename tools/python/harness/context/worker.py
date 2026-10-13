"""Implementation-worker prefill."""

from .base import _context_profile


_context_profile(
    "worker",
    paths=(
        "AGENTS.md",
        ".agents/skills/bof3-re/references/tooling-contract.md",
        ".agents/skills/bof3-re/references/target-contract.md",
    ),
    stable_paths=(".agents/skills/bof3-re/references/tooling-contract.md",),
)(lambda request: ())
