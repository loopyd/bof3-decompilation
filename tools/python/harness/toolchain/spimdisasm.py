from __future__ import annotations

from pathlib import Path

from .base import PythonSubmoduleToolchain


class SpimdisasmToolchain(PythonSubmoduleToolchain):
    label = "spimdisasm"
    submodule = "third_party/spimdisasm"
    install_target = "third_party/spimdisasm"

    @property
    def executable(self) -> Path:
        return self.root / ".venv" / "bin" / "spimdisasm"
