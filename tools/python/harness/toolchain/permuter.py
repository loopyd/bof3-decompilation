from __future__ import annotations

from pathlib import Path

from .base import PythonScriptSubmoduleToolchain


class DecompPermuterToolchain(PythonScriptSubmoduleToolchain):
    label = "decomp-permuter"
    submodule = "third_party/decomp-permuter"
    script = "permuter.py"
    interpreter_flags = ("-u",)
    pip_packages = ("toml",)

    @property
    def working_directory(self) -> Path:
        return self.source
