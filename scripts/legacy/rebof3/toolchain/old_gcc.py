from __future__ import annotations

import argparse
import shutil
import sys
from dataclasses import dataclass
from pathlib import Path

from ..cli import add_logging_args, context_from_args, package_prog
from ..config import DEPS_DOWNLOAD_DIR, OLD_GCC_TOOLCHAINS_ROOT
from .installer import Installer
from .old_gcc_catalog import (
    DEFAULT_OLD_GCC_COMPILER_SET,
    OLD_GCC_RELEASE_TAG,
    OLD_GCC_RELEASES_BY_ID,
    OLD_GCC_REPO,
    OLD_GCC_TESTED_MATRIX_COMPILER_IDS,
    expand_compiler_ids,
    release_for_compiler,
)
from .releases import download_gh_release_asset, extract_tar_gz


@dataclass(frozen=True, slots=True)
class OldGccInstallRequest:
    dest_root: Path
    compiler_ids: tuple[str, ...]
    force: bool = False


def requested_compiler_ids(
    requested_ids: list[str] | None,
    compiler_sets: list[str] | None,
) -> tuple[str, ...]:
    return expand_compiler_ids(
        requested_ids,
        compiler_sets,
        default_ids=OLD_GCC_TESTED_MATRIX_COMPILER_IDS,
    )


class OldGccInstaller(Installer):
    installer_name = "old_gcc"

    def install(self, request: OldGccInstallRequest, *, logger) -> int:
        if shutil.which("gh") is None:
            logger.error("required command missing: gh is required")
            return 1

        download_dir = DEPS_DOWNLOAD_DIR
        request.dest_root.mkdir(parents=True, exist_ok=True)

        ready: list[str] = []
        for compiler_id in request.compiler_ids:
            release = release_for_compiler(compiler_id)
            archive_path = download_dir / release.asset_name
            install_root = release.install_path(request.dest_root)
            gcc_path = install_root / "gcc"

            if request.force:
                shutil.rmtree(install_root, ignore_errors=True)
                archive_path.unlink(missing_ok=True)

            if not archive_path.exists():
                result = download_gh_release_asset(
                    repo=OLD_GCC_REPO,
                    release_tag=OLD_GCC_RELEASE_TAG,
                    asset_name=release.asset_name,
                    download_dir=download_dir,
                )
                if result.returncode != 0:
                    sys.stderr.write(result.stderr)
                    return int(result.returncode)

            if gcc_path.exists():
                ready.append(compiler_id)
                continue

            shutil.rmtree(install_root, ignore_errors=True)
            install_root.mkdir(parents=True, exist_ok=True)
            extract_tar_gz(archive_path, install_root)
            if not gcc_path.exists():
                logger.error(f"compiler archive did not produce {gcc_path}")
                return 1
            ready.append(compiler_id)

        logger.summary(
            "old-gcc ready: "
            f"dest_root={request.dest_root} compilers={', '.join(ready)}"
        )
        return 0


DEFAULT_OLD_GCC_INSTALLER = OldGccInstaller()


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("re", "setup-old-gcc"),
        description="Download and extract optional old-gcc toolchains used for cross-compiler validation.",
    )
    add_logging_args(parser)
    parser.add_argument(
        "--compiler",
        action="append",
        choices=sorted(OLD_GCC_RELEASES_BY_ID),
        help="Compiler id to install. May be passed multiple times.",
    )
    parser.add_argument(
        "--compiler-set",
        action="append",
        choices=(DEFAULT_OLD_GCC_COMPILER_SET,),
        help=(
            "Named compiler set to install. Defaults to "
            f"{DEFAULT_OLD_GCC_COMPILER_SET} when no explicit compilers are provided."
        ),
    )
    parser.add_argument("--dest-root", type=Path, default=OLD_GCC_TOOLCHAINS_ROOT)
    parser.add_argument("--force", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    context = context_from_args(args, "toolchain_old_gcc")
    compiler_ids = requested_compiler_ids(args.compiler, args.compiler_set)
    return DEFAULT_OLD_GCC_INSTALLER.install(
        OldGccInstallRequest(
            dest_root=args.dest_root,
            compiler_ids=compiler_ids,
            force=args.force,
        ),
        logger=context.logger,
    )


if __name__ == "__main__":
    raise SystemExit(main())
