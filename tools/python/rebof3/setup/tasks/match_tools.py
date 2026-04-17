from __future__ import annotations

import shutil

from ...common import run_command
from ..models import SetupContext


def run(context: SetupContext) -> None:
    if shutil.which("cargo") is None:
        raise RuntimeError("cargo is required to build match tools")
    run_command(
        [
            "cargo",
            "build",
            "--manifest-path",
            str(context.layout.objdiff_src / "Cargo.toml"),
            "--release",
            "-p",
            "objdiff-cli",
        ],
        cwd=context.layout.root,
        env={
            "CARGO_TARGET_DIR": str(
                context.layout.build_dir / "third_party" / "objdiff"
            )
        },
    )
    run_command(
        [
            "cargo",
            "build",
            "--manifest-path",
            str(context.layout.mipsmatch_src / "Cargo.toml"),
            "--release",
        ],
        cwd=context.layout.root,
        env={
            "CARGO_TARGET_DIR": str(
                context.layout.build_dir / "third_party" / "mipsmatch"
            )
        },
    )
