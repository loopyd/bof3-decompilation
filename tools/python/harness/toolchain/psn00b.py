from __future__ import annotations

import shutil
from .base import Toolchain, ensure_gitkeep
from .helpers import download_file
from .releases import extract_archive, github_release_asset_url


def _make_executable(bin_dir: Path) -> None:
    if bin_dir.is_dir():
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
                github_release_asset_url(
                    repo=PSN00B_REPO, tag=PSN00B_TAG, asset_name=asset
                ),
                archive,
            )
        if force:
            shutil.rmtree(self.layout.psn00b_toolchain_root, ignore_errors=True)
            shutil.rmtree(self.layout.psn00b_sdk_root, ignore_errors=True)
        if not (
            self.layout.psn00b_toolchain_root / "bin" / "mipsel-none-elf-gcc"
        ).is_file():
            shutil.rmtree(self.layout.psn00b_toolchain_root, ignore_errors=True)
            extract_archive(toolchain_archive, self.layout.psn00b_toolchain_root)
        if not (self.layout.psn00b_sdk_root / "PSn00bSDK-0.24-Linux").is_dir():
            shutil.rmtree(self.layout.psn00b_sdk_root, ignore_errors=True)
            extract_archive(sdk_archive, self.layout.psn00b_sdk_root)
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
