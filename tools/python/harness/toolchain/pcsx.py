"""Prepare the pinned PCSX-Redux source reference and its nested submodules."""

from __future__ import annotations

import json
import os
import shutil
from pathlib import Path
import subprocess

from ..io import file_sha256, write_json
from .base import ExecutableToolchain
from ..runtime.patches import (
    inspect_redux_target,
    prepare_redux_target,
)
from .sdl import SdlToolchain, build_command

PACKAGES = (
    "libcapstone-dev",
    "libcapstone4",
    "libavcodec-dev",
    "libavformat-dev",
    "libavutil-dev",
    "libswresample-dev",
    "libcurl4-openssl-dev",
    "libuv1-dev",
)


class PcsxReduxToolchain(ExecutableToolchain):
    label = "PCSX-Redux"
    submodule = "third_party/pcsx-redux"

    @property
    def executable(self) -> Path:
        return self.layout.root / self.submodule / "pcsx-redux"

    @property
    def library(self) -> Path:
        return SdlToolchain(self.layout).library

    @property
    def environment(self) -> dict[str, str]:
        environment = os.environ.copy()
        environment["LD_LIBRARY_PATH"] = str(self.library.parent)
        return environment

    def source_identity(self, *, allow_pristine: bool = False) -> dict:
        """Identify pinned recursive sources and the exact approved customization."""
        return inspect_redux_target(self.layout, allow_pristine=allow_pristine)

    def runtime_identity(self) -> dict:
        """Inspect a prepared binary without installing or writing a setup receipt."""
        identity = self.source_identity()
        if not self.executable.is_file() or not os.access(self.executable, os.X_OK):
            raise FileNotFoundError(f"PCSX-Redux binary not built: {self.executable}")
        if not self.library.is_file():
            raise FileNotFoundError(f"missing approved SDL3 build: {self.library}")
        return {
            **identity,
            "binary": str(self.executable),
            "binary_sha256": file_sha256(self.executable),
            "sdl_library": str(self.library.resolve()),
            "sdl_sha256": file_sha256(self.library),
        }

    def prerequisites(self) -> dict:
        tools = {}
        for name in (
            "git",
            "cmake",
            "make",
            "gcc",
            "g++",
            "pkg-config",
            "dpkg-query",
            "ldd",
        ):
            path = shutil.which(name)
            if path is None:
                raise FileNotFoundError(
                    f"PCSX-Redux build prerequisite missing: {name}"
                )
            tools[name] = path
        result = subprocess.run(
            [
                "dpkg-query",
                "-W",
                "-f=${Package}\t${db:Status-Status}\n",
                *PACKAGES,
            ],
            capture_output=True,
            text=True,
            check=False,
            timeout=30,
        )
        rows = [row.split("\t") for row in result.stdout.splitlines()]
        installed = sorted(
            row[0] for row in rows if len(row) == 2 and row[1] == "installed"
        )
        if result.returncode or installed != sorted(PACKAGES):
            raise ValueError(
                "PCSX-Redux build packages missing; see docs/reference/audio-runtime-setup.md"
            )
        result = subprocess.run(
            [
                "pkg-config",
                "--exists",
                "capstone",
                "freetype2",
                "libavcodec",
                "libavformat",
                "libavutil",
                "libswresample",
                "libcurl",
                "libuv",
                "zlib",
                "gl",
                "x11",
                "xcb",
            ],
            capture_output=True,
            text=True,
            check=False,
            timeout=30,
        )
        if result.returncode:
            raise ValueError(
                f"PCSX-Redux development metadata missing: {result.stderr.strip()}"
            )
        return {
            "packages": installed,
            "tools": tools,
        }

    def build(self) -> str:
        dependencies = self.prerequisites()
        prepare_redux_target(self.layout)
        before = self.source_identity()
        sdl = SdlToolchain(self.layout)
        sdl.run()
        environment = self.environment
        environment["PKG_CONFIG_PATH"] = str(sdl.prefix / "lib/pkgconfig")
        command = [
            "make",
            "-C",
            str(self.layout.root / self.submodule),
            "-j6",
            "BUILD=Release",
        ]
        build_command(
            self.layout.root,
            command,
            self.layout.out_dir / "setup/pcsx-redux-build.json",
            env=environment,
        )
        after = self.source_identity()
        # LuaJIT generates a fixed named set of sources during its own build.
        # Their resulting hashes enter the build receipt and read-only doctor.
        if {k: v for k, v in after.items() if k != "generated_sources"} != {
            k: v for k, v in before.items() if k != "generated_sources"
        } or self.prerequisites() != dependencies:
            raise ValueError("PCSX-Redux build inputs changed")
        identity = self.runtime_identity()
        write_json(
            self.layout.out_dir / "setup/pcsx-redux.json",
            {
                "schema": "bof3.redux-build/v1",
                "identity": identity,
                "dependencies": dependencies,
                "command": command,
            },
        )
        return str(self.executable)

    def _git(self, *arguments: str) -> str:
        result = subprocess.run(
            ["git", *arguments],
            cwd=self.layout.root,
            check=False,
            capture_output=True,
            text=True,
        )
        if result.returncode:
            raise RuntimeError(
                f"PCSX-Redux git command failed: {result.stderr.strip()}"
            )
        return result.stdout.strip()

    def _revision(self) -> str:
        fields = self._git("ls-files", "--stage", "--", self.submodule).split()
        if len(fields) != 4 or fields[0] != "160000" or fields[2] != "0":
            raise ValueError(f"{self.submodule} must be a registered Git submodule")
        return fields[1]

    def install(self, *, force: bool = False) -> str:
        revision = self._revision()
        source = self.layout.root / self.submodule
        if (source / ".git").exists():
            if self._git("-C", self.submodule, "rev-parse", "HEAD") != revision:
                raise ValueError(
                    "PCSX-Redux checkout differs from the gitlink; preserve local work before setup"
                )
            inspect_redux_target(self.layout, allow_pristine=True)
        status = self._git("submodule", "status", "--recursive", "--", self.submodule)
        if any(row.startswith(("-", "+", "U")) for row in status.splitlines()):
            self._git(
                "submodule", "update", "--init", "--recursive", "--", self.submodule
            )
        return revision

    def verify(self) -> str:
        SdlToolchain(self.layout).verify()
        identity = self.runtime_identity()
        receipt = json.loads(
            (self.layout.out_dir / "setup/pcsx-redux.json").read_text()
        )
        if (
            receipt.get("schema") != "bof3.redux-build/v1"
            or receipt.get("identity") != identity
        ):
            raise ValueError("PCSX-Redux build receipt is missing or stale; run setup")
        if receipt.get("dependencies") != self.prerequisites():
            raise ValueError("PCSX-Redux build dependencies changed; run setup")
        linked = subprocess.run(
            ["ldd", str(self.executable)],
            env=self.environment,
            capture_output=True,
            text=True,
            check=False,
            timeout=30,
        )
        if (
            linked.returncode
            or "not found" in linked.stdout
            or str(self.library.parent) not in linked.stdout
        ):
            raise ValueError(
                "PCSX-Redux linkage is missing the approved SDL3 or another library"
            )
        return f"{identity['revision']}; built executable, dependencies, SDL3 and linkage verified"
