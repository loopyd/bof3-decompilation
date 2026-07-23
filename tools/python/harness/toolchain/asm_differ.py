from __future__ import annotations

from pathlib import Path

from .base import PythonSubmoduleToolchain


class AsmDifferToolchain(PythonSubmoduleToolchain):
    """Install the pinned asm-differ submodule and its Python dependencies."""

    label = "asm-differ"
    submodule = "third_party/asm-differ"

    @property
    def executable(self) -> Path:
        return self.root / ".venv" / "bin" / "asm-differ"

    install_target = "third_party/asm-differ"

    @property
    def working_directory(self) -> Path:
        return self.source

    def verify(self) -> str:
        if not self.executable.is_file():
            raise FileNotFoundError(f"missing asm-differ executable: {self.executable}")
        result = self.execute(["--help"], quiet=True)
        if result.returncode:
            raise RuntimeError(f"asm-differ exited {result.returncode}")
        return self.label
