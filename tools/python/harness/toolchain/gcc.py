"""Canonical GCC 2.7.2 PSX installer (decompals/old-gcc 0.13)."""

from __future__ import annotations

from ..io import RepoLayout
from .base import Toolchain, ensure_gitkeep
from .gcc_archive import install_archive, verify_installed
from .releases import github_release_asset_url


# Observed SHA-256 of the 0.13 release archive; identity is the `gcc --version`
# first line. Both canonical GCC and catalog variants share the same
# digest-verified cache lifecycle.
OLD_GCC_URL = github_release_asset_url(
    repo=OLD_GCC_REPO, tag=OLD_GCC_TAG, asset_name=OLD_GCC_ASSET
)
OLD_GCC_SHA256 = (
    "sha256:aca64479041aa2d645dc52ebcaace276c0aa06f258c0e3f190ccf6d76701ffbc"
)
OLD_GCC_IDENTITY = "2.7.2"


class GccToolchain(Toolchain):
    label = "GCC 2.7.2"

    def __init__(self, layout: RepoLayout) -> None:
        self.layout = layout

    def install(self, *, force: bool = False) -> str:
        status = install_archive(
            self.layout,
            archive_name=OLD_GCC_ASSET,
            url=OLD_GCC_URL,
            checksum=OLD_GCC_SHA256,
            dest=self.layout.gcc272_psx_root,
            executable_relpath="gcc",
            expected_identity=OLD_GCC_IDENTITY,
            label=self.label,
            force=force,
        )
        ensure_gitkeep(self.layout.gcc272_psx_root)
        return status

    def verify(self) -> str:
        return verify_installed(
            dest=self.layout.gcc272_psx_root,
            executable_relpath="gcc",
            expected_identity=OLD_GCC_IDENTITY,
            label=self.label,
        )
