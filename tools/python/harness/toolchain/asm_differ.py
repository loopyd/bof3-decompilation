from __future__ import annotations

from pathlib import Path

from .base import PythonSubmoduleToolchain


class AsmDifferToolchain(PythonSubmoduleToolchain):
    """Install the pinned asm-differ submodule and its Python dependencies."""

    label = "asm-differ"
    submodule = "third_party/asm-differ"
    install_target = "third_party/asm-differ"
    verify_arguments = ("--help",)

    @property
    def executable(self) -> Path:
        return self.root / ".venv" / "bin" / "asm-differ"

    @property
    def working_directory(self) -> Path:
        return self.source
