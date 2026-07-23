"""Common lifecycle for repository-managed toolchains."""

from __future__ import annotations

from abc import ABC, abstractmethod
import subprocess
import sys
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
        self.root = root

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
    """A toolchain exposing one repository-owned executable."""

    @property
    @abstractmethod
    def executable(self) -> Path:
        """The executable managed by this toolchain."""
