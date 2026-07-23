"""Common lifecycle for repository-managed toolchains."""

from __future__ import annotations

from abc import ABC, abstractmethod
import os
import subprocess
import sys
from collections.abc import Sequence
from pathlib import Path


def ensure_gitkeep(root: Path) -> None:
    root.mkdir(parents=True, exist_ok=True)
    (root / ".gitkeep").write_text("\n", encoding="utf-8")


class Toolchain(ABC):
    """An installable local dependency with an optional build step."""

    label: str

    @abstractmethod
    def install(self, *, force: bool = False) -> str:
        """Install or stage the dependency and return a short status."""

    def build(self) -> str:
        """Build from staged inputs when required by this toolchain."""
        return ""

    @abstractmethod
    def verify(self) -> str:
        """Verify the installed dependency and return a short status."""

    def run(self, *, force: bool = False) -> str:
        """Run the standard install, build, verify lifecycle."""
        installed = self.install(force=force)
        built = self.build()
        verified = self.verify()
        return verified or built or installed


class SubmoduleToolchain(Toolchain):
    """A pinned source submodule verified through one project-local command."""

    submodule: str
    command: tuple[str, ...]

    def __init__(self, root: Path) -> None:
        self.root = root.resolve()

    @property
    def source(self) -> Path:
        return self.root / self.submodule

    def install(self, *, force: bool = False) -> str:
        subprocess.run(
            ["git", "submodule", "update", "--init", self.submodule],
            cwd=self.root,
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        return ""

    def verify(self) -> str:
        command = [sys.executable, str(self.root / self.command[0]), *self.command[1:]]
        result = subprocess.run(
            command,
            cwd=self.root,
            check=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        if result.returncode:
            raise RuntimeError(f"{' '.join(command)} exited {result.returncode}")
        return self.label


class ExecutableToolchain(Toolchain):
    """A toolchain that owns executable invocation as well as installation."""

    @property
    @abstractmethod
    def executable(self) -> Path:
        """The executable managed by this toolchain."""

    @property
    def working_directory(self) -> Path:
        """The sole working directory used for this tool's invocations."""
        return self.executable.parent

    @property
    def environment(self) -> dict[str, str]:
        """The environment supplied to this tool's invocations."""
        return os.environ.copy()

    def invocation(self, arguments: Sequence[str] = ()) -> list[str]:
        """Build the owned command without exposing executable-path policy."""
        return [str(self.executable), *arguments]

    def execute(
        self,
        arguments: Sequence[str] = (),
        *,
        capture_output: bool = False,
        text: bool = False,
        timeout: float | None = None,
    ) -> subprocess.CompletedProcess[str]:
        """Execute the tool in its owned environment without raising on failure."""
        return subprocess.run(
            self.invocation(arguments),
            cwd=self.working_directory,
            env=self.environment,
            check=False,
            capture_output=capture_output,
            text=text,
            timeout=timeout,
        )


class PythonSubmoduleToolchain(SubmoduleToolchain, ExecutableToolchain):
    """A pinned Python source submodule installed into the project virtual environment."""

    install_target: str

    @property
    def python(self) -> Path:
        return self.root / ".venv" / "bin" / "python"

    @property
    def working_directory(self) -> Path:
        return self.root

    def install(self, *, force: bool = False) -> str:
        super().install(force=force)
        if not self.python.is_file():
            raise FileNotFoundError(f"missing project Python environment: {self.python}")
        command = ["uv", "pip", "install", "--python", str(self.python)]
        if force:
            command.append("--reinstall")
        command.append(str(self.root / self.install_target))
        subprocess.run(command, cwd=self.root, check=True)
        return self.label

    def verify(self) -> str:
        if not self.executable.is_file():
            raise FileNotFoundError(f"missing {self.label} executable: {self.executable}")
        result = self.execute(["--version"])
        if result.returncode:
            raise RuntimeError(f"{self.label} exited {result.returncode}")
        return self.label
