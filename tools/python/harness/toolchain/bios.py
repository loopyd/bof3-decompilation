"""Prepare the US SCPH-5501 BIOS and find it by content during doctor checks."""

from __future__ import annotations

import hashlib
import os
import shutil
import tempfile
import urllib.request
from pathlib import Path

from .base import Toolchain
from .releases import extract_archive
from ..io import file_sha256

BIOS_SHA256 = "11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef"
BIOS_SIZE = 524288
BIOS_FILENAME = "scph5501.bin"
ARCHIVE_SHA1 = "68d72d383a0f9d2c5a70b7020c6384eae6b1c40e"
ARCHIVE_SIZE = 196765
ARCHIVE_URL = (
    "https://archive.org/download/sony-playstation-biosimages242016-10-21/"
    "Sony%20-%20PlayStation%20-%20BIOS%20Images/ps-30a.7z"
)


class BiosToolchain(Toolchain):
    label = "US PlayStation BIOS"

    @property
    def directory(self) -> Path:
        return self.layout.external_inputs_dir / "bios"

    def _find_rom(self) -> Path | None:
        root = self.directory
        if root.is_symlink():
            raise ValueError("BIOS input directory must not be a symlink")
        if not root.exists():
            return None
        if not root.is_dir():
            raise ValueError("BIOS input location must be a directory")
        errors = []
        for directory, folders, filenames in os.walk(
            root, followlinks=False, onerror=errors.append
        ):
            folders[:] = sorted(
                name for name in folders if not (Path(directory) / name).is_symlink()
            )
            for name in sorted(filenames):
                path = Path(directory) / name
                try:
                    if path.is_symlink() or not path.is_file():
                        continue
                    if (
                        path.stat().st_size == BIOS_SIZE
                        and file_sha256(path) == BIOS_SHA256
                    ):
                        return path
                except OSError as exc:
                    errors.append(exc)
        if errors:
            raise OSError(f"BIOS scan could not read all candidates: {errors[0]}")
        return None

    @staticmethod
    def _check_archive(path: Path) -> None:
        if path.is_symlink() or path.stat().st_size != ARCHIVE_SIZE:
            raise ValueError("US BIOS archive size/type mismatch")
        if hashlib.sha1(path.read_bytes()).hexdigest() != ARCHIVE_SHA1:
            raise ValueError("US BIOS archive SHA-1 mismatch")

    def _download(self, *, force: bool) -> Path:
        cache = self.layout.downloads_dir / "bios"
        cache.mkdir(parents=True, exist_ok=True)
        target = cache / "ps-30a.7z"
        if target.is_symlink():
            raise ValueError("BIOS cache must not be a symlink")
        if target.exists() and not force:
            self._check_archive(target)
            return target
        with tempfile.NamedTemporaryFile(
            dir=cache, prefix=".download-", delete=False
        ) as output:
            temporary = Path(output.name)
            try:
                with urllib.request.urlopen(ARCHIVE_URL, timeout=60) as response:
                    output.write(response.read(ARCHIVE_SIZE + 1))
                output.close()
                self._check_archive(temporary)
                os.replace(temporary, target)
            finally:
                temporary.unlink(missing_ok=True)
        return target

    def install(self, *, force: bool = False) -> str:
        existing = self._find_rom()
        if existing is not None and not force:
            return str(existing)
        root = self.directory
        root.mkdir(parents=True, exist_ok=True)
        target = root / BIOS_FILENAME
        if target.is_symlink() or (
            target.exists()
            and (not target.is_file() or file_sha256(target) != BIOS_SHA256)
        ):
            raise ValueError(
                f"existing {BIOS_FILENAME} differs; refusing to overwrite it"
            )
        staging = Path(tempfile.mkdtemp(prefix=".bios-", dir=root))
        try:
            archive = self._download(force=force)
            extracted = staging / "extracted"
            extract_archive(archive, extracted)
            files = list(extracted.iterdir())
            if len(files) != 1 or files[0].is_symlink() or not files[0].is_file():
                raise ValueError("expected one US BIOS ROM in ps-30a.7z")
            rom = files[0]
            if rom.stat().st_size != BIOS_SIZE or file_sha256(rom) != BIOS_SHA256:
                raise ValueError("US BIOS ROM size/SHA-256 mismatch")
            if not target.exists():
                # Publish without replacing a file created concurrently.
                os.link(rom, target)
            elif target.is_symlink() or file_sha256(target) != BIOS_SHA256:
                raise ValueError(f"{BIOS_FILENAME} changed during download")
        finally:
            shutil.rmtree(staging)
        return str(target)

    def verify(self) -> str:
        rom = self._find_rom()
        if rom is None:
            raise FileNotFoundError(
                f"no US SCPH-5501 BIOS with SHA-256 {BIOS_SHA256} "
                f"in {self.directory}; run just setup --component bios"
            )
        return f"US SCPH-5501 BIOS SHA-256 verified: {rom}"
