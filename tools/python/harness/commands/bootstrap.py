"""Small, non-interactive setup entry point used only by ``just setup``."""

from __future__ import annotations

import argparse
from pathlib import Path

from ..io import repo_layout, run_command
from ..toolchain.psx import install_canonical_psx_toolchain
from ..toolchain.setup_psyq import stage_psyq_sdk
from ._common import run_main
from .compile_commands import run as write_compile_commands


def run(args: argparse.Namespace) -> int:
    root = args.root.resolve()
    layout = repo_layout(root)
    run_command(["git", "submodule", "update", "--init", "--recursive"], cwd=root)
    install_canonical_psx_toolchain(layout)
    stage_psyq_sdk(dest=layout.psyq_root)
    for source, target in (
        (layout.harness_disk_src, layout.harness_disk_bin.parent.parent),
        (layout.emi_ex_src, layout.emi_ex_bin.parent.parent),
    ):
        run_command(
            [
                "cargo",
                "build",
                "--locked",
                "--release",
                "--manifest-path",
                str(source / "Cargo.toml"),
                "--target-dir",
                str(target),
            ],
            cwd=root,
        )
    write_compile_commands(argparse.Namespace(root=root))
    print("setup: complete")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bootstrap")
    parser.add_argument("--root", type=Path, default=repo_layout().root)
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
