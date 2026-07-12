from __future__ import annotations

import shutil
import subprocess
import tarfile
import zipfile
from pathlib import Path


SUPPORTED_ARCHIVE_SUFFIXES = (".7z", ".zip", ".tar.gz", ".tgz")


def archive_path_looks_valid(path: Path) -> bool:
    return (
        path.exists()
        and path.is_file()
        and path.name.lower().endswith(SUPPORTED_ARCHIVE_SUFFIXES)
    )


def extract_archive(archive_path: Path, dest: Path) -> None:
    archive_name = archive_path.name.lower()
    dest.mkdir(parents=True, exist_ok=True)
    if archive_name.endswith(".zip"):
        with zipfile.ZipFile(archive_path, "r") as archive:
            archive.extractall(dest)
        return
    if archive_name.endswith((".tar.gz", ".tgz")):
        with tarfile.open(archive_path, "r:gz") as archive:
            archive.extractall(dest)
        return
    if archive_name.endswith(".7z"):
        subprocess.run(["7z", "x", str(archive_path), f"-o{dest}"], check=True)
        return
    raise ValueError(f"unsupported archive type: {archive_path}")


def archive_stem(path: Path) -> str:
    lowered = path.name.lower()
    for suffix in SUPPORTED_ARCHIVE_SUFFIXES:
        if lowered.endswith(suffix):
            return path.name[: -len(suffix)]
    return path.stem


def sync_archive_into_store(source: Path, dest: Path, *, force: bool) -> Path:
    dest.parent.mkdir(parents=True, exist_ok=True)
    if source.resolve() == dest.resolve():
        return dest
    if force or not dest.exists():
        shutil.copy2(source, dest)
    return dest
