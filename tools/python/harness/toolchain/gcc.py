"""Canonical GCC 2.7.2 PSX installer (decompals/old-gcc 0.13)."""

from __future__ import annotations

from collections.abc import Mapping

from ..common.artifacts import FileIdentity
from ..io import RepoLayout
from .base import Toolchain, ensure_gitkeep
from .gcc_archive import install_archive, verify_installed
from .releases import github_release_asset_url


OLD_GCC_REPO = "decompals/old-gcc"
OLD_GCC_TAG = "0.13"
OLD_GCC_ASSET = "gcc-2.7.2-psx.tar.gz"
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
CANONICAL_INTEGER_RULE = "gcc-2.7.2-psx-int32/v1"
CANONICAL_INTEGER_COMPONENTS = (
    (
        "driver",
        816856,
        "4eb1d9e0335ef61aa484afe1c671e807f54a2526c483a48ee6ae9e5e98f408c6",
    ),
    (
        "cc1",
        2832972,
        "11f093cbb18b9ef132782bd2f37a50320069fa452acaf9e74e6a32cb28676cdb",
    ),
    (
        "cpp",
        1022964,
        "7b1da3e118fe4c015e71bdf31b6a5d2ca916f6ddf86918d42088433e7840aa03",
    ),
)


def inspect_canonical_int_bits(components: Mapping[str, FileIdentity]) -> int | None:
    if set(components) != {name for name, _, _ in CANONICAL_INTEGER_COMPONENTS}:
        return None
    return (
        32
        if all(
            (components[name].bytes, components[name].sha256) == (size, sha256)
            for name, size, sha256 in CANONICAL_INTEGER_COMPONENTS
        )
        else None
    )


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
