from __future__ import annotations

import tarfile
import urllib.request
import zipfile
from pathlib import Path

from ..common import ensure_parent


def github_release_asset_url(*, repo: str, tag: str, asset_name: str) -> str:
    return f"https://github.com/{repo}/releases/download/{tag}/{asset_name}"


def download_file(url: str, dest: Path) -> Path:
    ensure_parent(dest)
    if dest.exists():
        return dest
    with urllib.request.urlopen(url) as response, dest.open("wb") as handle:
        handle.write(response.read())
    return dest


def extract_zip(archive_path: Path, dest: Path) -> None:
    dest.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(archive_path) as archive:
        archive.extractall(dest)


def extract_tar_gz(archive_path: Path, dest: Path) -> None:
    dest.mkdir(parents=True, exist_ok=True)
    with tarfile.open(archive_path, "r:gz") as archive:
        dest_resolved = dest.resolve()
        for member in archive.getmembers():
            member_path = (dest / member.name).resolve()
            if (
                member_path != dest_resolved
                and dest_resolved not in member_path.parents
            ):
                raise RuntimeError(
                    f"refusing to extract path outside destination: {member.name}"
                )
        archive.extractall(dest)
