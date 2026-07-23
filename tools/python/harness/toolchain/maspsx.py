from __future__ import annotations

import subprocess
from pathlib import Path

from .base import ExecutableToolchain


class MaspsxToolchain(ExecutableToolchain):
    label = "maspsx"

    def __init__(self, root: Path) -> None:
        self.root = root

    @property
    def executable(self) -> Path:
        return self.root / "bin" / "maspsx"

    def install(self, *, force: bool = False) -> str:
        subprocess.run(
            ["git", "submodule", "update", "--init", "third_party/maspsx"],
            cwd=self.root,
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        return ""

    def verify(self) -> str:
        source = self.root / "third_party" / "maspsx" / "maspsx.py"
        if not source.is_file():
            raise FileNotFoundError(f"missing maspsx source: {source}")
        result = subprocess.run(
            [str(self.executable), "--help"],
            cwd=self.root,
            check=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        if result.returncode:
            raise RuntimeError(f"maspsx exited {result.returncode}")
        return self.label
