"""Build the Linux MIPS toolchain used by Nugget's PSYQo SDK."""

from __future__ import annotations

import os
import secrets
import shutil
import stat
import subprocess
from collections.abc import Iterator
from pathlib import Path

from harness.common.files import read_file
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

    def _installed_files(self) -> Iterator[tuple[Path, os.stat_result]]:
        root = self.layout.psyqo_toolchain_root
        if root.is_symlink():
            raise ValueError(f"PSYQo install root is symlinked: {root}")
        if not root.exists():
            return
        if not root.is_dir():
            raise ValueError(f"PSYQo install root is not a directory: {root}")

        def fail(error: OSError) -> None:
            raise error

        for directory, _, names in os.walk(root, followlinks=False, onerror=fail):
            for name in sorted(names):
                path = Path(directory) / name
                status = path.lstat()
                if stat.S_ISREG(status.st_mode):
                    yield path, status

    def _detach_hardlinks(self) -> int:
        """Replace installer-created aliases with independent, identical files."""
        count = 0
        root = self.layout.psyqo_toolchain_root
        for path, before in self._installed_files():
            if before.st_nlink == 1:
                continue
            content = read_file(root, path.relative_to(root).as_posix())
            temporary = path.with_name(f".{path.name}.unlinked-{secrets.token_hex(12)}")
            owned = None
            try:
                with temporary.open("xb") as stream:
                    status = os.fstat(stream.fileno())
                    owned = (status.st_dev, status.st_ino)
                    stream.write(content)
                    stream.flush()
                    if before.st_mode & 0o111:
                        os.fchmod(
                            stream.fileno(), os.fstat(stream.fileno()).st_mode | 0o111
                        )
                    os.fsync(stream.fileno())
                after = path.lstat()
                fields = (
                    "st_dev",
                    "st_ino",
                    "st_nlink",
                    "st_size",
                    "st_mtime_ns",
                    "st_ctime_ns",
                )
                if any(getattr(before, key) != getattr(after, key) for key in fields):
                    raise RuntimeError(
                        f"PSYQo file changed while detaching hardlink: {path}"
                    )
                os.replace(temporary, path)
                count += 1
            finally:
                if owned is not None:
                    try:
                        status = temporary.lstat()
                    except FileNotFoundError:
                        pass
                    else:
                        if (status.st_dev, status.st_ino) == owned:
                            temporary.unlink()
        return count

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
            self._detach_hardlinks()
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
        self._detach_hardlinks()
        self.stamp.write_text(source_hash + "\n", encoding="ascii")
        self._rebuild_sdk = True
        return self.label

    def _execute(self, argv: list[str], name: str) -> None:
        directory = self.directory
        directory.mkdir(parents=True, exist_ok=True)
        for suffix in ("stdout.bin", "stderr.bin", "json"):
            (directory / f"{name}.{suffix}").unlink(missing_ok=True)
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
        for path, status in self._installed_files():
            if status.st_nlink != 1:
                raise RuntimeError(
                    f"PSYQo install contains a hardlink: {path}; run setup --component psyqo"
                )
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
            )
        target = subprocess.check_output(
            [str(binaries / "mipsel-none-elf-gcc"), "-dumpmachine"],
            text=True,
        ).strip()
        if target != "mipsel-none-elf":
            raise RuntimeError(f"unexpected PSYQo compiler target: {target}")
        return self.label
