from __future__ import annotations

from ...common import run_command
from ..models import SetupContext


def run(context: SetupContext) -> None:
    build_dir = context.layout.build_dir
    run_command(
        [
            "cargo",
            "build",
            "--locked",
            "--release",
            "--manifest-path",
            str(context.layout.harness_disk_src / "Cargo.toml"),
            "--target-dir",
            str(build_dir / "tools" / "rust" / "bof3-disk"),
        ],
        cwd=context.layout.root,
    )
    run_command(
        [
            "cargo",
            "build",
            "--locked",
            "--release",
            "--manifest-path",
            str(context.layout.emi_ex_src / "Cargo.toml"),
            "--target-dir",
            str(build_dir / "tools" / "rust" / "emi-ex"),
        ],
        cwd=context.layout.root,
    )
