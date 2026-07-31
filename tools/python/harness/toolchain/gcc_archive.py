"""Shared GCC archive cache and atomic install lifecycle.

Both the canonical ``GccToolchain`` and every catalog ``CompilerVariantEntry``
resolve their archives through this module.

Cache contract (``inputs/external/private-assets/toolchains/gcc/``):

- The cache root and every cached entry must be a real directory / regular
  file; a symlink or any special entry is rejected, never followed.
- Downloads land in a cache-local temporary file; the SHA-256 digest is
  validated before the temporary file atomically becomes the cache entry.
- A digest mismatch removes the corrupt entry and re-downloads it; a failed
  download removes the temporary file and leaves any prior verified entry
  (or no entry) untouched.

Install contract (``toolchains/...``):

- Extraction goes to a fresh sibling staging directory of the destination.
- The staged executable must be a regular file inside the staging root and its
  ``--version`` output must contain the expected identity before anything is
  swapped into place.
- The swap keeps a backup of any prior install and restores it if the atomic
  rename fails, so a failed network, digest, extraction, or identity check
  preserves a prior verified install.
"""

from __future__ import annotations

import hashlib
import os
import shutil
import subprocess
import tarfile
import tempfile
import urllib.request
from pathlib import Path

from ..io import RepoLayout


def sha256_file(path: Path) -> str:
    """Compute the ``sha256:<hex>`` digest of a file (streaming)."""
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return f"sha256:{digest.hexdigest()}"


def gcc_cache_dir(layout: RepoLayout) -> Path:
    """Digest-verified GCC archive cache root under private assets."""
    return layout.gcc_archive_cache_dir


def _reject_cache_entry(path: Path, *, what: str) -> None:
    if path.is_symlink():
        raise ValueError(f"refusing symlinked {what}: {path}")
    if path.exists() and not path.is_file():
        raise ValueError(f"{what} is not a regular file: {path}")


def _ensure_cache_root(cache_dir: Path) -> None:
    if cache_dir.is_symlink():
        raise ValueError(f"refusing symlinked GCC cache root: {cache_dir}")
    if cache_dir.exists() and not cache_dir.is_dir():
        raise ValueError(f"GCC cache root is not a directory: {cache_dir}")
    cache_dir.mkdir(parents=True, exist_ok=True)


def _download_to_cache(cache_dir: Path, url: str, archive_name: str) -> Path:
    """Download into a cache-local temporary file; the caller validates it."""
    fd, temp_name = tempfile.mkstemp(
        dir=cache_dir, prefix=f".{archive_name}.", suffix=".tmp"
    )
    temp = Path(temp_name)
    try:
        with urllib.request.urlopen(url, timeout=120) as response:
            with os.fdopen(fd, "wb") as handle:
                shutil.copyfileobj(response, handle)
    except BaseException:
        temp.unlink(missing_ok=True)
        raise
    return temp


def ensure_cached_archive(
    layout: RepoLayout, *, archive_name: str, url: str, checksum: str
) -> Path:
    """Return a SHA-256-verified cached archive, downloading when needed.

    A cached entry that is a symlink or not a regular file is rejected. A
    cached entry whose digest does not match the expected checksum is treated
    as corrupt, removed, and re-downloaded. Publication is atomic: the
    validated temporary file replaces the cache entry only after its digest
    matches.
    """
    cache_dir = gcc_cache_dir(layout)
    _ensure_cache_root(cache_dir)
    dest = cache_dir / archive_name
    _reject_cache_entry(dest, what="cached archive")
    if dest.is_file() and sha256_file(dest) == checksum:
        return dest
    if dest.exists():
        dest.unlink()  # corrupt cache entry: recover by re-downloading
    temp = _download_to_cache(cache_dir, url, archive_name)
    try:
        actual = sha256_file(temp)
        if actual != checksum:
            raise ValueError(
                f"SHA-256 mismatch for {archive_name}: expected {checksum}, got {actual}"
            )
        os.replace(temp, dest)
    except BaseException:
        temp.unlink(missing_ok=True)
        raise
    return dest


def _version_output(exe: Path, label: str) -> str:
    result = subprocess.run(
        [str(exe), "--version"],
        capture_output=True,
        text=True,
        timeout=30,
    )
    if result.returncode != 0:
        raise RuntimeError(f"{label}: gcc --version exited {result.returncode}")
    return result.stdout.strip()


def verify_installed(
    *,
    dest: Path,
    executable_relpath: str,
    expected_identity: str,
    label: str,
) -> str:
    """Verify an installed GCC: regular file, inside dest, ``--version`` identity.

    A symlinked install root or symlinked executable is rejected before any
    ``is_file``/``resolve`` so a symlink is never followed.
    """
    if dest.is_symlink():
        raise ValueError(f"{label}: refusing symlinked install root: {dest}")
    if not dest.is_dir():
        raise ValueError(f"{label}: install root is not a directory: {dest}")
    exe = dest / executable_relpath
    if exe.is_symlink():
        raise ValueError(f"{label}: refusing symlinked executable: {exe}")
    if not exe.is_file():
        raise FileNotFoundError(f"{label}: missing executable: {exe}")
    resolved = exe.resolve()
    root = dest.resolve()
    if not (resolved == root or root in resolved.parents):
        raise ValueError(
            f"{label}: executable path {resolved} escapes install root {root}"
        )
    version = _version_output(exe, label)
    if expected_identity and expected_identity not in version:
        raise ValueError(
            f"{label}: --version output does not contain expected identity "
            f"{expected_identity!r}"
        )
    return label


def _safe_extract_tar_gz(archive_path: Path, dest: Path) -> None:
    """Extract tar.gz rejecting absolute, traversal, device/FIFO, and link entries."""
    dest.mkdir(parents=True, exist_ok=True)
    try:
        archive = tarfile.open(archive_path, "r:gz")
    except (tarfile.TarError, OSError) as exc:
        raise ValueError(f"cannot open GCC archive {archive_path.name}: {exc}") from exc
    with archive:
        try:
            for member in archive.getmembers():
                if member.issym() or member.islnk():
                    raise ValueError(
                        f"archive contains link entry {member.name!r}; rejecting for safety"
                    )
                if member.isdev() or member.isfifo():
                    raise ValueError(
                        f"archive contains device entry {member.name!r}; rejecting"
                    )
                name = Path(member.name)
                if name.is_absolute():
                    raise ValueError(
                        f"archive contains absolute path {member.name!r}; rejecting"
                    )
                if ".." in name.parts:
                    raise ValueError(
                        f"archive contains path with '..' {member.name!r}; rejecting"
                    )
                archive.extract(member, dest, filter="data")
        except tarfile.TarError as exc:
            raise ValueError(
                f"cannot extract GCC archive {archive_path.name}: {exc}"
            ) from exc


def _atomic_replace_dir(staging: Path, dest: Path) -> None:
    """Swap staging into dest, keeping a backup of any prior install."""
    backup = dest.parent / f".{dest.name}.backup-{os.getpid()}"
    shutil.rmtree(backup, ignore_errors=True)
    moved_old = False
    if dest.is_symlink() or dest.exists():
        os.replace(dest, backup)
        moved_old = True
    try:
        os.replace(staging, dest)
    except BaseException:
        if moved_old:
            os.replace(backup, dest)
        raise
    if moved_old:
        shutil.rmtree(backup, ignore_errors=True)


def install_archive(
    layout: RepoLayout,
    *,
    archive_name: str,
    url: str,
    checksum: str,
    dest: Path,
    executable_relpath: str,
    expected_identity: str,
    label: str,
    force: bool = False,
) -> str:
    """Ensure the digest-verified archive is cached, then install into dest.

    A verified existing install is left untouched (unless *force*); a missing
    or corrupt install is rebuilt from the verified cache entry. Extraction
    happens in a fresh sibling staging directory and the destination is
    swapped only after the staged ``--version`` identity check passes.
    """
    archive = ensure_cached_archive(
        layout, archive_name=archive_name, url=url, checksum=checksum
    )
    if not force:
        try:
            verify_installed(
                dest=dest,
                executable_relpath=executable_relpath,
                expected_identity=expected_identity,
                label=label,
            )
            return f"{label}: already installed"
        except (FileNotFoundError, ValueError, RuntimeError):
            pass  # missing or corrupt install: rebuild from the verified cache
    dest.parent.mkdir(parents=True, exist_ok=True)
    staging = dest.parent / f".{dest.name}.staging-{os.getpid()}"
    shutil.rmtree(staging, ignore_errors=True)
    try:
        _safe_extract_tar_gz(archive, staging)
        verify_installed(
            dest=staging,
            executable_relpath=executable_relpath,
            expected_identity=expected_identity,
            label=label,
        )
        _atomic_replace_dir(staging, dest)
    finally:
        shutil.rmtree(staging, ignore_errors=True)
    return f"{label}: installed and verified"
