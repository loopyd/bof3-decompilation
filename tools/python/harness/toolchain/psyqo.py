"""Build the Linux MIPS toolchain used by Nugget's PSYQo SDK."""

from __future__ import annotations

import shutil
import subprocess
import time
from pathlib import Path

from harness.common.process import capture
from harness.common.resources import select_build_jobs
from harness.io import file_sha256

from .base import Toolchain


class PsyqoToolchain(Toolchain):
    label = "PSYQo"
    _rebuild_sdk = False

    @property
    def script(self) -> Path:
        return (
            self.layout.third_party_dir
            / "pcsx-redux-agentic/tools/linux-mips/spawn-compiler.sh"
        )

    @property
    def stamp(self) -> Path:
        return self.layout.psyqo_toolchain_root / ".compiler-source.sha256"

    @property
    def directory(self) -> Path:
        return self.layout.build_dir / "third_party/psyqo-toolchain"

    def _clear_build_sources(self) -> None:
        for child in self.directory.iterdir():
            if child.is_dir():
                shutil.rmtree(child)

    def install(self, *, force: bool = False) -> str:
        self._rebuild_sdk = False
        if not (self.layout.psyqo_sdk_root / "Makefile").is_file():
            subprocess.run(
                [
                    "git",
                    "submodule",
                    "update",
                    "--init",
                    "--recursive",
                    "third_party/pcsx-redux-agentic",
                ],
                cwd=self.layout.root,
                check=True,
            )
        if not force:
            try:
                return self._verify_compiler()
            except (OSError, RuntimeError, subprocess.SubprocessError):
                pass
        source_hash = file_sha256(self.script)
        directory = self.directory
        directory.mkdir(parents=True, exist_ok=True)
        self._clear_build_sources()
        prefix = self.layout.psyqo_toolchain_root
        if prefix.exists():
            shutil.rmtree(prefix)
        self._execute(["/bin/sh", str(self.script)], "build")
        self._clear_build_sources()
        self.stamp.write_text(source_hash + "\n", encoding="ascii")
        self._rebuild_sdk = True
        return self.label

    def _execute(self, argv: list[str], name: str) -> None:
        directory = self.directory
        directory.mkdir(parents=True, exist_ok=True)
        for suffix in ("stdout.bin", "stderr.bin", "json"):
            (directory / f"{name}.{suffix}").unlink(missing_ok=True)
        timeout = 14400
        record = capture(
            directory,
            argv,
            environment={
                "PATH": "/usr/bin:/bin",
                "LC_ALL": "C",
                "PREFIX": str(self.layout.psyqo_toolchain_root),
                "JOBS": str(select_build_jobs()),
                "MAKEINFO": "true",
            },
            deadline=time.monotonic() + timeout,
            timeout=timeout,
            output_limit=128 << 20,
            name=name,
        )
        if record["failure"] is not None or record["raw_exit"] != 0:
            raise RuntimeError(
                f"PSYQo {name} failed; see {directory / (name + '.stderr.bin')}"
            )

    def build(self) -> str:
        if self._rebuild_sdk:
            self._execute(
                ["make", "-C", str(self.layout.psyqo_sdk_root), "clean"], "sdk-clean"
            )
        self._execute(
            [
                "make",
                "-C",
                str(self.layout.psyqo_sdk_root),
                f"PREFIX={self.layout.psyqo_toolchain_root / 'bin/mipsel-none-elf'}",
                f"-j{select_build_jobs()}",
            ],
            "sdk",
        )
        return self.label

    def verify(self) -> str:
        if not (self.layout.psyqo_sdk_root / "psyqo.mk").is_file():
            raise FileNotFoundError(f"missing PSYQo SDK: {self.layout.psyqo_sdk_root}")
        if not (self.layout.psyqo_sdk_root / "libpsyqo.a").is_file():
            raise FileNotFoundError(
                "missing PSYQo library; run setup --component psyqo"
            )
        return self._verify_compiler()

    def _verify_compiler(self) -> str:
        if self.stamp.read_text(encoding="ascii").strip() != file_sha256(self.script):
            raise RuntimeError(
                "PSYQo compiler build script changed; run setup --component psyqo"
            )
        binaries = self.layout.psyqo_toolchain_root / "bin"
        for name in (
            "gcc",
            "g++",
            "as",
            "ld",
            "ar",
            "nm",
            "objcopy",
            "objdump",
            "ranlib",
            "strip",
        ):
            subprocess.run(
                [str(binaries / f"mipsel-none-elf-{name}"), "--version"],
                check=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                timeout=10,
            )
        target = subprocess.check_output(
            [str(binaries / "mipsel-none-elf-gcc"), "-dumpmachine"],
            text=True,
            timeout=10,
        ).strip()
        if target != "mipsel-none-elf":
            raise RuntimeError(f"unexpected PSYQo compiler target: {target}")
        return self.label
