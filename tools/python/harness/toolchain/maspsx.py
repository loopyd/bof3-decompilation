from __future__ import annotations

import subprocess
from collections.abc import Sequence
from pathlib import Path

from .base import ExecutableToolchain


class MaspsxToolchain(ExecutableToolchain):
    label = "maspsx"

    def __init__(self, root: Path) -> None:
        self.root = root.resolve()

    @property
    def python(self) -> Path:
        return self.root / ".venv" / "bin" / "python"

    @property
    def executable(self) -> Path:
        return self.root / "third_party" / "maspsx" / "maspsx.py"

    def invocation(self, arguments: Sequence[str] = ()) -> list[str]:
        return [str(self.python), str(self.executable), *arguments]

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
        if not self.executable.is_file():
            raise FileNotFoundError(f"missing maspsx source: {self.executable}")
        if not self.python.is_file():
            raise FileNotFoundError(f"missing project Python environment: {self.python}")
        result = self.execute(["--help"])
        if result.returncode:
            raise RuntimeError(f"maspsx exited {result.returncode}")
        return self.label
