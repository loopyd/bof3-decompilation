from __future__ import annotations

import shutil
from dataclasses import dataclass
from pathlib import Path

from ..paths import RepoLayout
from .releases import download_file, extract_tar_gz, github_release_asset_url


ESA_REPO = "mkst/esa"
ESA_TAG = "psyq-binaries"
ALL_ASPSX_PSYQ_VERSIONS = (
    "psyq3.3",
    "psyq3.5",
    "psyq4.0",
    "psyq4.1",
    "psyq4.3",
    "psyq4.4",
    "psyq4.5",
    "psyq4.6",
)
DEFAULT_ASPSX_PSYQ_VERSIONS = ("psyq4.0",)


@dataclass(frozen=True)
class AspsxBinariesResult:
    root: Path
    versions: tuple[str, ...]


def expected_aspsx_exe(root: Path, version: str) -> Path:
    return root / version / "ASPSX.EXE"


def normalize_aspsx_versions(
    versions: tuple[str, ...] | list[str] | None,
) -> tuple[str, ...]:
    requested = tuple(versions or DEFAULT_ASPSX_PSYQ_VERSIONS)
    if not requested:
        raise ValueError("at least one ASPSX PsyQ version must be requested")

    invalid = sorted(set(requested) - set(ALL_ASPSX_PSYQ_VERSIONS))
    if invalid:
        raise ValueError("unknown ASPSX PsyQ version(s): " + ", ".join(invalid))

    return requested


def aspsx_binaries_ready(
    root: Path,
    *,
    versions: tuple[str, ...] | list[str] | None = None,
) -> bool:
    requested = normalize_aspsx_versions(versions)
    return all(expected_aspsx_exe(root, version).exists() for version in requested)


def sync_compat_root(source_root: Path, compat_root: Path) -> None:
    compat_root.parent.mkdir(parents=True, exist_ok=True)

    if compat_root.exists() or compat_root.is_symlink():
        if compat_root.is_dir():
            shutil.rmtree(compat_root)
        else:
            compat_root.unlink()

    shutil.copytree(source_root, compat_root, dirs_exist_ok=True)


def download_aspsx_binaries(
    layout: RepoLayout,
    *,
    versions: tuple[str, ...] | list[str] | None = None,
    force: bool = False,
) -> AspsxBinariesResult:
    requested_versions = normalize_aspsx_versions(versions)
    downloads_dir = layout.downloads_dir / "aspsx"
    downloads_dir.mkdir(parents=True, exist_ok=True)
    layout.aspsx_psyq_root.mkdir(parents=True, exist_ok=True)

    for version in requested_versions:
        archive_name = f"{version}.tar.gz"
        archive_path = downloads_dir / archive_name
        target_dir = layout.aspsx_psyq_root / version

        if force:
            shutil.rmtree(target_dir, ignore_errors=True)
            archive_path.unlink(missing_ok=True)

        if expected_aspsx_exe(layout.aspsx_psyq_root, version).exists():
            continue

        download_file(
            github_release_asset_url(
                repo=ESA_REPO,
                tag=ESA_TAG,
                asset_name=archive_name,
            ),
            archive_path,
        )
        extract_tar_gz(archive_path, layout.aspsx_psyq_root)

    if not aspsx_binaries_ready(layout.aspsx_psyq_root, versions=requested_versions):
        raise RuntimeError(
            f"ASPSX binary download incomplete under {layout.aspsx_psyq_root}"
        )

    sync_compat_root(layout.aspsx_psyq_root, layout.aspsx_psyq_compat_root)

    return AspsxBinariesResult(
        root=layout.aspsx_psyq_root,
        versions=requested_versions,
    )
