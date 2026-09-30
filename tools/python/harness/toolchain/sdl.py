"""Download and build the approved SDL3 reference-emulator dependency locally."""

from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import tempfile
import urllib.request

from harness.common.process import run_bounded
from harness.io import file_sha256, write_json
from .base import Toolchain
from .releases import extract_archive

VERSION = "3.4.16"
ARCHIVE_SHA256 = "7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68"
ARCHIVE_URL = f"https://www.libsdl.org/release/SDL3-{VERSION}.tar.gz"


def source_digest(directory: Path) -> str:
    digest = hashlib.sha256()
    for path in sorted(directory.rglob("*")):
        if path.is_symlink():
            raise ValueError(f"source tree contains symlink: {path}")
        if path.is_file():
            digest.update(str(path.relative_to(directory)).encode() + b"\0")
            digest.update(bytes.fromhex(file_sha256(path)))
    return digest.hexdigest()


def build_command(root: Path, argv: list[str], log: Path, *, env=None) -> None:
    result = run_bounded(
        root, argv, timeout=1800, output_limit=32 * 1024 * 1024, env=env
    )
    log.parent.mkdir(parents=True, exist_ok=True)
    write_json(log, result)
    if result["failure"] or result["exit_code"]:
        raise RuntimeError(f"reference build failed; inspect {log}")


class SdlToolchain(Toolchain):
    label = "SDL3"

    @property
    def prefix(self) -> Path:
        return self.layout.toolchains_dir / f"pcsx-redux/sdl3-{VERSION}"

    @property
    def source(self) -> Path:
        return self.layout.toolchains_dir / f"pcsx-redux/source/SDL3-{VERSION}"

    @property
    def library(self) -> Path:
        return self.prefix / "lib/libSDL3.so.0"

    @property
    def receipt(self) -> Path:
        return self.layout.out_dir / "setup/sdl3.json"

    def install(self, *, force: bool = False) -> str:
        archive = self.layout.downloads_dir / f"SDL3-{VERSION}.tar.gz"
        archive.parent.mkdir(parents=True, exist_ok=True)
        if archive.is_symlink():
            raise ValueError("SDL3 download cache must not be a symlink")
        if force or not archive.exists():
            with tempfile.NamedTemporaryFile(
                dir=archive.parent, delete=False
            ) as staged:
                temporary = Path(staged.name)
                try:
                    with urllib.request.urlopen(ARCHIVE_URL, timeout=60) as response:
                        remaining = 64 * 1024 * 1024
                        while chunk := response.read(min(1024 * 1024, remaining + 1)):
                            remaining -= len(chunk)
                            if remaining < 0:
                                raise ValueError("SDL3 download exceeds 64 MiB")
                            staged.write(chunk)
                    staged.close()
                    if file_sha256(temporary) != ARCHIVE_SHA256:
                        raise ValueError("SDL3 archive SHA-256 mismatch")
                    os.replace(temporary, archive)
                finally:
                    temporary.unlink(missing_ok=True)
        if file_sha256(archive) != ARCHIVE_SHA256:
            raise ValueError("SDL3 archive SHA-256 mismatch")
        self.source.parent.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=self.source.parent) as temporary:
            extracted = Path(temporary) / "archive"
            extract_archive(archive, extracted)
            source = extracted / f"SDL3-{VERSION}"
            expected = source_digest(source)
            if self.source.exists():
                if self.source.is_symlink() or source_digest(self.source) != expected:
                    raise ValueError(
                        "SDL3 source differs from approved archive; preserve local changes"
                    )
            else:
                source.rename(self.source)
        return expected

    def build(self) -> str:
        build = self.layout.toolchains_dir / f"pcsx-redux/build/sdl3-{VERSION}"
        arguments = [
            "cmake",
            "-S",
            str(self.source),
            "-B",
            str(build),
            "-DCMAKE_BUILD_TYPE=Release",
            f"-DCMAKE_INSTALL_PREFIX={self.prefix}",
            "-DCMAKE_INSTALL_LIBDIR=lib",
            "-DSDL_SHARED=ON",
            "-DSDL_STATIC=OFF",
            "-DSDL_TESTS=OFF",
            "-DSDL_TEST_LIBRARY=OFF",
            "-DSDL_EXAMPLES=OFF",
        ]
        before = source_digest(self.source)
        logs = self.layout.out_dir / "setup"
        for label, command in (
            ("configure", arguments),
            ("build", ["cmake", "--build", str(build), "--parallel", "6"]),
            ("install", ["cmake", "--install", str(build)]),
        ):
            build_command(self.layout.root, command, logs / f"sdl3-{label}.json")
        if source_digest(self.source) != before:
            raise ValueError("SDL3 source changed during build")
        write_json(
            self.receipt,
            {
                "version": VERSION,
                "archive_sha256": ARCHIVE_SHA256,
                "source_sha256": before,
                "library_sha256": file_sha256(self.library),
                "configuration": arguments,
            },
        )
        return str(self.library)

    def verify(self) -> str:
        receipt = json.loads(self.receipt.read_text())
        if (
            receipt.get("version") != VERSION
            or receipt.get("archive_sha256") != ARCHIVE_SHA256
        ):
            raise ValueError("SDL3 build receipt does not match approved release")
        if receipt.get("source_sha256") != source_digest(self.source):
            raise ValueError("SDL3 source differs from build receipt")
        if receipt.get("library_sha256") != file_sha256(self.library):
            raise ValueError("SDL3 binary differs from build receipt")
        return f"SDL3 {VERSION}; source and built library hashes verified"
