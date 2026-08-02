"""Shared release downloads and safe archive extraction."""

import os
import shutil
import stat
import subprocess
import tempfile
from pathlib import Path, PurePosixPath, PureWindowsPath
SUPPORTED_ARCHIVE_SUFFIXES = (".7z", ".zip", ".tar.gz", ".tgz")
        not path.is_symlink()
def _validate_member_name(name: str) -> None:
    posix = PurePosixPath(name)
    windows = PureWindowsPath(name)
    if posix.is_absolute() or windows.is_absolute() or windows.drive:
        raise ValueError(f"archive contains absolute path {name!r}; rejecting")
    if ".." in posix.parts or ".." in windows.parts:
        raise ValueError(f"archive contains path with '..' {name!r}; rejecting")


def _extract_zip(archive_path: Path, dest: Path) -> None:
        for member in archive.infolist():
            _validate_member_name(member.filename)
            if stat.S_ISLNK(member.external_attr >> 16):
                raise ValueError(
                    f"archive contains link entry {member.filename!r}; rejecting"
                )
def _extract_tar_gz(archive_path: Path, dest: Path) -> None:
            _validate_member_name(member.name)
            if member.issym() or member.islnk():
                raise ValueError(
                    f"archive contains link entry {member.name!r}; rejecting"
                )
                member.isdev()
                or member.isfifo()
                or not (member.isfile() or member.isdir())
                raise ValueError(
                    f"archive contains device entry {member.name!r}; rejecting"
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
