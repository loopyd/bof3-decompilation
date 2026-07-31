"""Shared release downloads and safe archive extraction."""

from __future__ import annotations

import os
import shutil
import stat
import subprocess
import tarfile
import tempfile
import zipfile
from pathlib import Path, PurePosixPath, PureWindowsPath

SUPPORTED_ARCHIVE_SUFFIXES = (".7z", ".zip", ".tar.gz", ".tgz")


def github_release_asset_url(*, repo: str, tag: str, asset_name: str) -> str:
    return f"https://github.com/{repo}/releases/download/{tag}/{asset_name}"


def archive_path_looks_valid(path: Path) -> bool:
    return (
        not path.is_symlink()
        and path.is_file()
        and path.name.lower().endswith(SUPPORTED_ARCHIVE_SUFFIXES)
    )


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


def _validate_member_name(name: str) -> None:
    posix = PurePosixPath(name)
    windows = PureWindowsPath(name)
    if posix.is_absolute() or windows.is_absolute() or windows.drive:
        raise ValueError(f"archive contains absolute path {name!r}; rejecting")
    if ".." in posix.parts or ".." in windows.parts:
        raise ValueError(f"archive contains path with '..' {name!r}; rejecting")


def _extract_zip(archive_path: Path, dest: Path) -> None:
    with zipfile.ZipFile(archive_path) as archive:
        for member in archive.infolist():
            _validate_member_name(member.filename)
            if stat.S_ISLNK(member.external_attr >> 16):
                raise ValueError(
                    f"archive contains link entry {member.filename!r}; rejecting"
                )
        archive.extractall(dest)


def _extract_tar_gz(archive_path: Path, dest: Path) -> None:
    with tarfile.open(archive_path, "r:gz") as archive:
        for member in archive.getmembers():
            _validate_member_name(member.name)
            if member.issym() or member.islnk():
                raise ValueError(
                    f"archive contains link entry {member.name!r}; rejecting"
                )
            if (
                member.isdev()
                or member.isfifo()
                or not (member.isfile() or member.isdir())
            ):
                raise ValueError(
                    f"archive contains device entry {member.name!r}; rejecting"
                )
        archive.extractall(dest, filter="data")


def _extract_7z(archive_path: Path, dest: Path) -> None:
    listing = subprocess.run(
        ["7z", "l", "-slt", str(archive_path)],
        capture_output=True,
        check=True,
        text=True,
    ).stdout.partition("----------")[2]
    for record in listing.split("\n\n"):
        fields = dict(
            line.split(" = ", 1) for line in record.splitlines() if " = " in line
        )
        if path := fields.get("Path"):
            _validate_member_name(path)
            attributes = fields.get("Attributes", "").split()
            if len(attributes) > 1 and attributes[1].startswith("l"):
                raise ValueError(f"archive contains link entry {path!r}; rejecting")
    subprocess.run(
        ["7z", "x", "-y", str(archive_path), f"-o{dest}"],
        capture_output=True,
        check=True,
        text=True,
    )


def _reject_extracted_links(root: Path) -> None:
    for parent, directories, files in os.walk(root, followlinks=False):
        for name in [*directories, *files]:
            path = Path(parent) / name
            if path.is_symlink():
                raise ValueError(
                    f"archive extracted link entry {path.relative_to(root)!r}; rejecting"
                )


def extract_archive(archive_path: Path, dest: Path) -> None:
    """Safely extract one supported archive into an empty destination.

    Extraction always happens in a sibling staging directory. Member names and
    link-like entries are rejected before publication, and the verified staging
    tree is atomically renamed into place.
    """
    if not archive_path_looks_valid(archive_path):
        raise ValueError(f"unsupported or missing archive: {archive_path}")
    if dest.is_symlink() or (dest.exists() and not dest.is_dir()):
        raise ValueError(f"destination is not a real directory: {dest}")
    if dest.exists() and any(dest.iterdir()):
        raise ValueError(f"destination is not empty: {dest}")

    dest.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=f".{dest.name}.extract-", dir=dest.parent))
    try:
        suffix = archive_path.name.lower()
        if suffix.endswith(".zip"):
            _extract_zip(archive_path, staging)
        elif suffix.endswith((".tar.gz", ".tgz")):
            _extract_tar_gz(archive_path, staging)
        else:
            _extract_7z(archive_path, staging)
        _reject_extracted_links(staging)
        if dest.exists():
            dest.rmdir()
        os.replace(staging, dest)
    except (
        OSError,
        subprocess.SubprocessError,
        tarfile.TarError,
        zipfile.BadZipFile,
    ) as exc:
        raise ValueError(f"cannot extract {archive_path.name}: {exc}") from exc
    finally:
        shutil.rmtree(staging, ignore_errors=True)
