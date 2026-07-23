from __future__ import annotations

import subprocess
from collections.abc import Sequence
from pathlib import Path

from .base import ExecutableToolchain, SubmoduleToolchain


class M2cToolchain(SubmoduleToolchain, ExecutableToolchain):
    label = "m2c"
    submodule = "third_party/m2c"

    @property
    def python(self) -> Path:
        return self.root / ".venv" / "bin" / "python"

    @property
    def executable(self) -> Path:
        return self.root / "third_party" / "m2c" / "m2c.py"

    @property
    def working_directory(self) -> Path:
        return self.root

    def invocation(self, arguments: Sequence[str] = ()) -> list[str]:
        return [str(self.python), str(self.executable), *arguments]

    def install(self, *, force: bool = False) -> str:
        subprocess.run(
            ["git", "submodule", "update", "--init", self.submodule],
            cwd=self.root,
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        return self.label

    def verify(self) -> str:
        if not self.executable.is_file():
            raise FileNotFoundError(f"missing m2c source: {self.executable}")
        if not self.python.is_file():
            raise FileNotFoundError(f"missing project Python environment: {self.python}")
        result = self.execute(["--help"], quiet=True)
        if result.returncode:
            raise RuntimeError(f"m2c exited {result.returncode}")
        return self.label
