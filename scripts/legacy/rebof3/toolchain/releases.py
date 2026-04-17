from __future__ import annotations

import subprocess
import tarfile
from pathlib import Path

from ..common import run_command


def download_gh_release_asset(
    *,
    repo: str,
    release_tag: str,
    asset_name: str,
    download_dir: Path,
) -> subprocess.CompletedProcess[str]:
    download_dir.mkdir(parents=True, exist_ok=True)
    return run_command(
        [
            "gh",
            "release",
            "download",
            release_tag,
            "-R",
            repo,
            "-p",
            asset_name,
            "-D",
            str(download_dir),
        ]
    )


def extract_tar_gz(archive_path: Path, dest: Path) -> None:
    with tarfile.open(archive_path, "r:gz") as archive:
        dest_resolved = dest.resolve()
        for member in archive.getmembers():
            member_path = (dest / member.name).resolve()
            if member_path != dest_resolved and dest_resolved not in member_path.parents:
                raise RuntimeError(
                    f"refusing to extract path outside destination: {member.name}"
                )
        archive.extractall(dest)
