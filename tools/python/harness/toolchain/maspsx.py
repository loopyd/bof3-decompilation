from __future__ import annotations

from .base import PythonScriptSubmoduleToolchain


class MaspsxToolchain(PythonScriptSubmoduleToolchain):
    label = "maspsx"
    submodule = "third_party/maspsx"
    script = "maspsx.py"
