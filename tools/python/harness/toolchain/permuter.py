from __future__ import annotations

from .base import PythonScriptSubmoduleToolchain


class DecompPermuterToolchain(PythonScriptSubmoduleToolchain):
    label = "decomp-permuter"
    submodule = "third_party/decomp-permuter"
    script = "permuter.py"
    interpreter_flags = ("-u",)
    pip_packages = ("toml",)
