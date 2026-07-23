from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path

from ..analyzer import find_engine
from ..io import RepoLayout
from .base import ExecutableToolchain


class RizinToolchain(ExecutableToolchain):
    label = "Rizin"

    def __init__(self, layout: RepoLayout) -> None:
        self.layout = layout
        self.source = layout.third_party_dir / "rizin"
        self.build_dir = layout.build_dir / "third_party" / "rizin"
        self.prefix = layout.toolchains_dir / "rizin"

    @property
    def executable(self) -> Path:
        return self.prefix / "bin" / "rizin"

    def install(self, *, force: bool = False) -> str:
        subprocess.run(
            ["git", "submodule", "update", "--init", "third_party/rizin"],
            cwd=self.layout.root,
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        if not (self.source / "meson.build").is_file():
            raise FileNotFoundError(f"missing Rizin source: {self.source}")
        if force:
            shutil.rmtree(self.build_dir, ignore_errors=True)
            shutil.rmtree(self.prefix, ignore_errors=True)
        return ""

    def build(self) -> str:
        if not self.build_dir.is_dir():
            subprocess.run(
                [
                    "meson",
                    "setup",
                    "--buildtype=release",
                    f"--prefix={self.prefix}",
                    str(self.build_dir),
                    str(self.source),
                ],
                cwd=self.layout.root,
                check=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
        subprocess.run(
            ["meson", "compile", "-C", str(self.build_dir)],
            cwd=self.layout.root,
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        subprocess.run(
            ["meson", "install", "-C", str(self.build_dir)],
            cwd=self.layout.root,
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
        return ""

    def verify(self) -> str:
        if not self.executable.is_file():
            raise FileNotFoundError(f"missing Rizin executable: {self.executable}")
        result = subprocess.run(
            [str(self.executable), "-V"],
            capture_output=True,
            text=True,
            check=False,
            timeout=10,
        )
        if result.returncode:
            raise RuntimeError(f"Rizin version check failed: {result.stderr.strip()}")
        original_path = os.environ.get("PATH", "")
        try:
            os.environ["PATH"] = f"{self.executable.parent}:{original_path}"
            find_engine(root=self.layout.root)
        finally:
            os.environ["PATH"] = original_path
        return self.label
