from __future__ import annotations

from .base import SubmoduleToolchain


class AsmDifferToolchain(SubmoduleToolchain):
    label = "asm-differ"
    submodule = "third_party/asm-differ"
    command = ("third_party/asm-differ/diff.py", "--help")
