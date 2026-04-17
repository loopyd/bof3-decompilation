from __future__ import annotations

from ...common import run_command
from ..models import SetupContext


def run(context: SetupContext) -> None:
    build_dir = context.layout.build_dir
    run_command(
        [
            "cmake",
            "-S",
            str(context.layout.bof3_disk_src),
            "-B",
            str(build_dir / "third_party" / "bof3-disk"),
            "-DCMAKE_BUILD_TYPE=Release",
        ],
        cwd=context.layout.root,
    )
    run_command(
        ["cmake", "--build", str(build_dir / "third_party" / "bof3-disk")],
        cwd=context.layout.root,
    )
    run_command(
        [
            "cmake",
            "-S",
            str(context.layout.emi_ex_src),
            "-B",
            str(build_dir / "tools" / "emi-ex-v2"),
            "-DCMAKE_BUILD_TYPE=Release",
            "-DEMI_BUILD_CLI=ON",
            "-DEMI_BUILD_TESTS=ON",
        ],
        cwd=context.layout.root,
    )
    run_command(
        ["cmake", "--build", str(build_dir / "tools" / "emi-ex-v2")],
        cwd=context.layout.root,
    )
