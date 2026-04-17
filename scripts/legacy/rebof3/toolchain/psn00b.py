from __future__ import annotations

import argparse
import shutil
import sys
from dataclasses import dataclass
from pathlib import Path

from ..cli import add_logging_args, context_from_args, package_prog
from ..common import run_command
from ..config import (
    DEPS_DOWNLOAD_DIR,
    GCC272_PSX_ROOT,
    PSN00B_SDK_ROOT,
    PSN00B_TOOLCHAIN_ROOT,
)
from .installer import Installer
from .old_gcc_catalog import OLD_GCC_RELEASE_TAG, OLD_GCC_REPO, release_for_compiler
from .releases import download_gh_release_asset, extract_tar_gz


RELEASE_TAG = "v0.24"
TOOLCHAIN_ARCHIVE_NAME = "gcc-mipsel-none-elf-12.3.0-linux.zip"
SDK_ARCHIVE_NAME = "PSn00bSDK-0.24-Linux.zip"
OLD_GCC_ARCHIVE_NAME = release_for_compiler("gcc-2.7.2-psx").asset_name


@dataclass(frozen=True, slots=True)
class Psn00bInstallRequest:
    toolchain_dest: Path
    sdk_dest: Path
    gcc272psx_dest: Path
    force: bool = False


class Psn00bInstaller(Installer):
    installer_name = "psn00b"

    def install(self, request: Psn00bInstallRequest, *, logger) -> int:
        download_dir = DEPS_DOWNLOAD_DIR
        toolchain_archive_path = download_dir / TOOLCHAIN_ARCHIVE_NAME
        sdk_archive_path = download_dir / SDK_ARCHIVE_NAME
        gcc272psx_archive_path = download_dir / OLD_GCC_ARCHIVE_NAME
        if shutil.which("gh") is None or shutil.which("unzip") is None:
            logger.error("required commands missing: gh and unzip are required")
            return 1
        if (
            (request.toolchain_dest / "bin" / "mipsel-none-elf-gcc").exists()
            and (request.gcc272psx_dest / "gcc").exists()
            and (
                request.sdk_dest
                / "PSn00bSDK-0.24-Linux"
                / "lib"
                / "libpsn00b"
                / "build.json"
            ).exists()
            and not request.force
        ):
            logger.summary(
                "toolchain already installed: "
                f"gnu={request.toolchain_dest / 'bin' / 'mipsel-none-elf-gcc'} "
                f"compiler={request.gcc272psx_dest / 'gcc'}"
            )
            return 0
        if request.force:
            shutil.rmtree(request.toolchain_dest, ignore_errors=True)
            shutil.rmtree(request.sdk_dest, ignore_errors=True)
            shutil.rmtree(request.gcc272psx_dest, ignore_errors=True)
            toolchain_archive_path.unlink(missing_ok=True)
            sdk_archive_path.unlink(missing_ok=True)
            gcc272psx_archive_path.unlink(missing_ok=True)
        if not toolchain_archive_path.exists():
            result = download_gh_release_asset(
                repo="Lameguy64/PSn00bSDK",
                release_tag=RELEASE_TAG,
                asset_name=TOOLCHAIN_ARCHIVE_NAME,
                download_dir=download_dir,
            )
            if result.returncode != 0:
                sys.stderr.write(result.stderr)
                return result.returncode
        if not sdk_archive_path.exists():
            result = download_gh_release_asset(
                repo="Lameguy64/PSn00bSDK",
                release_tag=RELEASE_TAG,
                asset_name=SDK_ARCHIVE_NAME,
                download_dir=download_dir,
            )
            if result.returncode != 0:
                sys.stderr.write(result.stderr)
                return result.returncode
        if not gcc272psx_archive_path.exists():
            result = download_gh_release_asset(
                repo=OLD_GCC_REPO,
                release_tag=OLD_GCC_RELEASE_TAG,
                asset_name=OLD_GCC_ARCHIVE_NAME,
                download_dir=download_dir,
            )
            if result.returncode != 0:
                sys.stderr.write(result.stderr)
                return result.returncode
        request.toolchain_dest.mkdir(parents=True, exist_ok=True)
        request.sdk_dest.mkdir(parents=True, exist_ok=True)
        request.gcc272psx_dest.mkdir(parents=True, exist_ok=True)
        for archive, dest in (
            (toolchain_archive_path, request.toolchain_dest),
            (sdk_archive_path, request.sdk_dest),
        ):
            result = run_command(["unzip", "-q", "-o", str(archive), "-d", str(dest)])
            if result.returncode != 0:
                sys.stderr.write(result.stderr)
                return result.returncode
        extract_tar_gz(gcc272psx_archive_path, request.gcc272psx_dest)
        logger.summary(
            "toolchain ready: "
            f"gnu={request.toolchain_dest / 'bin' / 'mipsel-none-elf-gcc'} "
            f"compiler={request.gcc272psx_dest / 'gcc'}"
        )
        return 0


DEFAULT_PSN00B_INSTALLER = Psn00bInstaller()


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("toolchain", "psn00b"),
        description="Download and extract the canonical PSX GNU tools, SDK, and gcc-2.7.2-psx compiler.",
    )
    add_logging_args(parser)
    parser.add_argument("--force", action="store_true")
    parser.add_argument("--toolchain-dest", type=Path, default=PSN00B_TOOLCHAIN_ROOT)
    parser.add_argument("--sdk-dest", type=Path, default=PSN00B_SDK_ROOT)
    parser.add_argument("--gcc272psx-dest", type=Path, default=GCC272_PSX_ROOT)
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    context = context_from_args(args, "toolchain_psn00b")
    return DEFAULT_PSN00B_INSTALLER.install(
        Psn00bInstallRequest(
            toolchain_dest=args.toolchain_dest,
            sdk_dest=args.sdk_dest,
            gcc272psx_dest=args.gcc272psx_dest,
            force=args.force,
        ),
        logger=context.logger,
    )


if __name__ == "__main__":
    raise SystemExit(main())
