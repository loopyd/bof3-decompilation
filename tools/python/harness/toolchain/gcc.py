from __future__ import annotations

import shutil

from ..io import RepoLayout
from .base import Toolchain, ensure_gitkeep
from .releases import download_file, extract_tar_gz, github_release_asset_url


OLD_GCC_REPO = "decompals/old-gcc"
OLD_GCC_TAG = "0.13"
OLD_GCC_ASSET = "gcc-2.7.2-psx.tar.gz"


class GccToolchain(Toolchain):
    label = "GCC 2.7.2"

    def __init__(self, layout: RepoLayout) -> None:
        self.layout = layout

    def install(self, *, force: bool = False) -> str:
        archive = self.layout.downloads_dir / OLD_GCC_ASSET
        download_file(
            github_release_asset_url(
                repo=OLD_GCC_REPO, tag=OLD_GCC_TAG, asset_name=OLD_GCC_ASSET
            ),
            archive,
        )
        if force:
            shutil.rmtree(self.layout.gcc272_psx_root, ignore_errors=True)
        if not (self.layout.gcc272_psx_root / "gcc").is_file():
            shutil.rmtree(self.layout.gcc272_psx_root, ignore_errors=True)
            extract_tar_gz(archive, self.layout.gcc272_psx_root)
        ensure_gitkeep(self.layout.gcc272_psx_root)
        return ""

    def verify(self) -> str:
        if not (self.layout.gcc272_psx_root / "gcc").is_file():
            raise FileNotFoundError(f"missing GCC: {self.layout.gcc272_psx_root / 'gcc'}")
        return self.label
