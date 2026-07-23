from __future__ import annotations

from .base import SubmoduleToolchain


class DecompPermuterToolchain(SubmoduleToolchain):
    label = "decomp-permuter"
    submodule = "third_party/decomp-permuter"
    command = ("third_party/decomp-permuter/permuter.py", "--help")
