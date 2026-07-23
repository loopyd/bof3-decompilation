from __future__ import annotations

import shutil
from pathlib import Path

from ..io import RepoLayout
from .base import Toolchain, ensure_gitkeep
from .releases import download_file, extract_zip, github_release_asset_url


PSN00B_REPO = "Lameguy64/PSn00bSDK"
PSN00B_TAG = "v0.24"
PSN00B_TOOLCHAIN_ASSET = "gcc-mipsel-none-elf-12.3.0-linux.zip"
PSN00B_SDK_ASSET = "PSn00bSDK-0.24-Linux.zip"


def _make_executable(bin_dir: Path) -> None:
    if bin_dir.is_dir():
        for path in bin_dir.iterdir():
            if path.is_file():
                path.chmod(path.stat().st_mode | 0o111)


class Psn00bToolchain(Toolchain):
    label = "PSn00b"

    def __init__(self, layout: RepoLayout) -> None:
        self.layout = layout

    def install(self, *, force: bool = False) -> str:
        toolchain_archive = self.layout.downloads_dir / PSN00B_TOOLCHAIN_ASSET
        sdk_archive = self.layout.downloads_dir / PSN00B_SDK_ASSET
        for asset, archive in (
            (PSN00B_TOOLCHAIN_ASSET, toolchain_archive),
            (PSN00B_SDK_ASSET, sdk_archive),
        ):
            download_file(
                github_release_asset_url(repo=PSN00B_REPO, tag=PSN00B_TAG, asset_name=asset),
                archive,
            )
        if force:
            shutil.rmtree(self.layout.psn00b_toolchain_root, ignore_errors=True)
            shutil.rmtree(self.layout.psn00b_sdk_root, ignore_errors=True)
        if not (self.layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-gcc").is_file():
            shutil.rmtree(self.layout.psn00b_toolchain_root, ignore_errors=True)
            extract_zip(toolchain_archive, self.layout.psn00b_toolchain_root)
        if not (self.layout.psn00b_sdk_root / "PSn00bSDK-0.24-Linux").is_dir():
            shutil.rmtree(self.layout.psn00b_sdk_root, ignore_errors=True)
            extract_zip(sdk_archive, self.layout.psn00b_sdk_root)
        _make_executable(self.layout.psn00b_toolchain_root / "bin")
        _make_executable(self.layout.psn00b_sdk_root / "PSn00bSDK-0.24-Linux" / "bin")
        ensure_gitkeep(self.layout.psn00b_toolchain_root)
        ensure_gitkeep(self.layout.psn00b_sdk_root)
        return ""

    def verify(self) -> str:
        required = (
            self.layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-as",
            self.layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-ld",
        )
        missing = [str(path) for path in required if not path.is_file()]
        if missing:
            raise FileNotFoundError(", ".join(missing))
        return self.label
