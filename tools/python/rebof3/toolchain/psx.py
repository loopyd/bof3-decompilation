from __future__ import annotations

import shutil
from dataclasses import dataclass
from pathlib import Path

from ..paths import RepoLayout
from .releases import (
    download_file,
    extract_tar_gz,
    extract_zip,
    github_release_asset_url,
)


PSN00B_REPO = "Lameguy64/PSn00bSDK"
PSN00B_TAG = "v0.24"
PSN00B_TOOLCHAIN_ASSET = "gcc-mipsel-none-elf-12.3.0-linux.zip"
PSN00B_SDK_ASSET = "PSn00bSDK-0.24-Linux.zip"
OLD_GCC_REPO = "decompals/old-gcc"
OLD_GCC_TAG = "0.13"
OLD_GCC_ASSET = "gcc-2.7.2-psx.tar.gz"


@dataclass(frozen=True)
class CanonicalPsxToolchainResult:
    gcc272_psx: Path
    psn00b_sdk: Path
    psn00b_toolchain: Path


def ensure_gitkeep(root: Path) -> None:
    root.mkdir(parents=True, exist_ok=True)
    (root / ".gitkeep").touch(exist_ok=True)


def ensure_bin_files_executable(bin_dir: Path) -> None:
    if not bin_dir.is_dir():
        return
    for path in bin_dir.iterdir():
        if path.is_file():
            path.chmod(path.stat().st_mode | 0o111)


def install_canonical_psx_toolchain(
    layout: RepoLayout,
    *,
    force: bool = False,
) -> CanonicalPsxToolchainResult:
    layout.downloads_dir.mkdir(parents=True, exist_ok=True)

    toolchain_archive = layout.downloads_dir / PSN00B_TOOLCHAIN_ASSET
    sdk_archive = layout.downloads_dir / PSN00B_SDK_ASSET
    old_gcc_archive = layout.downloads_dir / OLD_GCC_ASSET

    download_file(
        github_release_asset_url(
            repo=PSN00B_REPO,
            tag=PSN00B_TAG,
            asset_name=PSN00B_TOOLCHAIN_ASSET,
        ),
        toolchain_archive,
    )
    download_file(
        github_release_asset_url(
            repo=PSN00B_REPO,
            tag=PSN00B_TAG,
            asset_name=PSN00B_SDK_ASSET,
        ),
        sdk_archive,
    )
    download_file(
        github_release_asset_url(
            repo=OLD_GCC_REPO,
            tag=OLD_GCC_TAG,
            asset_name=OLD_GCC_ASSET,
        ),
        old_gcc_archive,
    )

    if force:
        shutil.rmtree(layout.psn00b_toolchain_root, ignore_errors=True)
        shutil.rmtree(layout.psn00b_sdk_root, ignore_errors=True)
        shutil.rmtree(layout.gcc272_psx_root, ignore_errors=True)

    if not (layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-gcc").exists():
        shutil.rmtree(layout.psn00b_toolchain_root, ignore_errors=True)
        extract_zip(toolchain_archive, layout.psn00b_toolchain_root)
    ensure_bin_files_executable(layout.psn00b_toolchain_root / "bin")
    ensure_gitkeep(layout.psn00b_toolchain_root)

    if not (layout.psn00b_sdk_root / "PSn00bSDK-0.24-Linux").exists():
        shutil.rmtree(layout.psn00b_sdk_root, ignore_errors=True)
        extract_zip(sdk_archive, layout.psn00b_sdk_root)
    ensure_bin_files_executable(layout.psn00b_sdk_root / "PSn00bSDK-0.24-Linux" / "bin")
    ensure_gitkeep(layout.psn00b_sdk_root)

    if not (layout.gcc272_psx_root / "gcc").exists():
        shutil.rmtree(layout.gcc272_psx_root, ignore_errors=True)
        extract_tar_gz(old_gcc_archive, layout.gcc272_psx_root)
    ensure_gitkeep(layout.gcc272_psx_root)

    return CanonicalPsxToolchainResult(
        gcc272_psx=layout.gcc272_psx_root,
        psn00b_sdk=layout.psn00b_sdk_root,
        psn00b_toolchain=layout.psn00b_toolchain_root,
    )
