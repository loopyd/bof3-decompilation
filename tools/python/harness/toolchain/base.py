"""Common lifecycle for repository-managed toolchains."""

from __future__ import annotations

from abc import ABC, abstractmethod
import os
import subprocess
import sys
from collections.abc import Sequence
from pathlib import Path

from ..io import RepoLayout


def ensure_gitkeep(root: Path) -> None:
    root.mkdir(parents=True, exist_ok=True)
    (root / ".gitkeep").write_text("\n", encoding="utf-8")


class Toolchain(ABC):
    """An installable local dependency with an optional build step."""

    label: str

    def __init__(self, layout: RepoLayout) -> None:
        self.layout = layout

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

    def __init__(self, layout: RepoLayout) -> None:
        super().__init__(layout)
        self.root = layout.root

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
        quiet: bool = False,
    ) -> subprocess.CompletedProcess[str]:
        """Execute the tool in its owned environment without raising on failure.

        *quiet* — redirect stdout/stderr to DEVNULL (ignores *capture_output*
        when True to keep verification quiet while normal capture or streaming
        callers pass quiet=False, the default).
        """
        if quiet:
            return subprocess.run(
                self.invocation(arguments),
                cwd=self.working_directory,
                env=self.environment,
                check=False,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                timeout=timeout,
            )
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

    install_target: str | None = None
    pip_packages: tuple[str, ...] = ()
    verify_arguments: tuple[str, ...] = ("--version",)

    @property
    def python(self) -> Path:
        return self.root / ".venv" / "bin" / "python"

    @property
    def working_directory(self) -> Path:
        return self.root

    def install(self, *, force: bool = False) -> str:
        super().install(force=force)
        targets: list[str] = []
        if self.install_target is not None:
            targets.append(str(self.root / self.install_target))
        targets.extend(self.pip_packages)
        if not targets:
            return self.label
        if not self.python.is_file():
            raise FileNotFoundError(
                f"missing project Python environment: {self.python}"
            )
        command = ["uv", "pip", "install", "--python", str(self.python)]
        if force:
            command.append("--reinstall")
        command.extend(targets)
        subprocess.run(command, cwd=self.root, check=True)
        return self.label

    def verify(self) -> str:
        if not self.executable.is_file():
            raise FileNotFoundError(
                f"missing {self.label} executable: {self.executable}"
            )
        result = self.execute(self.verify_arguments, quiet=True)
        if result.returncode:
            raise RuntimeError(f"{self.label} exited {result.returncode}")
        return self.label


class PythonScriptSubmoduleToolchain(PythonSubmoduleToolchain):
    """A pinned Python script submodule run through the project interpreter."""

    script: str
    interpreter_flags: tuple[str, ...] = ()
    verify_arguments: tuple[str, ...] = ("--help",)

    @property
    def executable(self) -> Path:
        return self.source / self.script

    @property
    def environment(self) -> dict[str, str]:
        environment = os.environ.copy()
        environment["PYTHONPATH"] = os.pathsep.join(
            (str(self.source), str(self.root / "tools/python"))
        )
        environment["PYTHONSAFEPATH"] = "1"
        return environment

    def invocation(self, arguments: Sequence[str] = ()) -> list[str]:
        return [
            str(self.python),
            "-P",
            *self.interpreter_flags,
            str(self.executable),
            *arguments,
        ]

    def verify(self) -> str:
        if not self.executable.is_file():
            raise FileNotFoundError(f"missing {self.label} source: {self.executable}")
        if not self.python.is_file():
            raise FileNotFoundError(
                f"missing project Python environment: {self.python}"
            )
        result = self.execute(self.verify_arguments, quiet=True)
        if result.returncode:
            raise RuntimeError(f"{self.label} exited {result.returncode}")
        return self.label
