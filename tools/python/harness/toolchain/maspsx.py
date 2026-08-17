from __future__ import annotations

from pathlib import Path

from .base import PythonScriptSubmoduleToolchain


class MaspsxToolchain(PythonScriptSubmoduleToolchain):
    label = "maspsx"
    submodule = "third_party/maspsx"
    script = "maspsx.py"

    @property
    def working_directory(self) -> Path:
        return self.source
