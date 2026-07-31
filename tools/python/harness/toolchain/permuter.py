from __future__ import annotations

import subprocess
from collections.abc import Sequence
from pathlib import Path

from .base import ExecutableToolchain, SubmoduleToolchain


class DecompPermuterToolchain(SubmoduleToolchain, ExecutableToolchain):
    label = "decomp-permuter"
    submodule = "third_party/decomp-permuter"

    @property
    def python(self) -> Path:
        return self.root / ".venv" / "bin" / "python"

    @property
    def executable(self) -> Path:
        return self.source / "permuter.py"

    @property
    def working_directory(self) -> Path:
        return self.source

    @property
    def interpreter_flags(self) -> tuple[str, ...]:
        """Flags passed to the Python interpreter before the script."""
        return ("-u",)

    def invocation(self, arguments: Sequence[str] = ()) -> list[str]:
        return [
            str(self.python),
            *self.interpreter_flags,
            str(self.executable),
            *arguments,
        ]

    def install(self, *, force: bool = False) -> str:
        super().install(force=force)
        if not self.python.is_file():
            raise FileNotFoundError(
                f"missing project Python environment: {self.python}"
            )
        command = ["uv", "pip", "install", "--python", str(self.python), "toml"]
        if force:
            command.append("--reinstall")
        subprocess.run(command, cwd=self.root, check=True)
        return self.label

    def verify(self) -> str:
        if not self.executable.is_file():
            raise FileNotFoundError(
                f"missing decomp-permuter executable: {self.executable}"
            )
        result = self.execute(["--help"], quiet=True)
        if result.returncode:
            raise RuntimeError(f"decomp-permuter exited {result.returncode}")
        return self.label
