"""Command transport for isolated PCSX-Redux evidence missions."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from harness.common.cli import (
    add_example_argument,
    add_root_argument,
    add_work_deadline_argument,
    resolved_root,
    run_main,
)
from harness.io import repo_layout
from harness.runtime.session import run_session
from harness.toolchain.pcsx import PcsxReduxToolchain


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="bin/harness runtime", description=__doc__)
    add_root_argument(parser)
    add_example_argument(parser, "bin/harness runtime status")
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser(
        "status", help="inspect pinned source, emulator and SDL identities"
    ).set_defaults(handler=run_runtime)
    command = commands.add_parser(
        "run", help="execute a trusted Lua mission with bounded process ownership"
    )
    for name in ("bios", "script", "output"):
        command.add_argument(f"--{name}", type=Path, required=True)
    command.add_argument("--executable", type=Path, help="optional PS-X EXE to load")
    command.add_argument(
        "--origin",
        type=Path,
        help="verify saved-state capture receipt and tool/media identities",
    )
    command.add_argument(
        "--disc", type=Path, help="CUE with BINARY tracks; pin and mount declared media"
    )
    command.add_argument(
        "--module",
        action="append",
        default=[],
        metavar="NAME=PATH",
        help="stage and hash a Lua module (repeatable)",
    )
    command.add_argument(
        "--input",
        action="append",
        default=[],
        metavar="NAME=PATH",
        help="stage and hash a mission input (repeatable)",
    )
    command.add_argument(
        "--argument",
        action="append",
        default=[],
        metavar="NAME=VALUE",
        help="mission argument exposed as PSX_RUNTIME_ARG_NAME (repeatable)",
    )
    command.add_argument("--timeout", type=float, default=60)
    command.add_argument("--output-limit", type=int, default=2 * 1024 * 1024)
    add_work_deadline_argument(command)
    command.set_defaults(handler=run_runtime)
    return parser


def run_runtime(args: argparse.Namespace) -> int:
    toolchain = PcsxReduxToolchain(repo_layout(resolved_root(args)))
    if args.command == "status":
        toolchain.verify()
        report = {"schema": "psx.runtime-status/v1", **toolchain.runtime_identity()}
    else:
        report = run_session(
            toolchain,
            bios=args.bios,
            executable=args.executable,
            script=args.script,
            output=args.output,
            timeout=args.timeout,
            output_limit=args.output_limit,
            deadline=args.work_deadline,
            arguments=args.argument,
            modules=args.module,
            files=args.input,
            disc=args.disc,
            origin=args.origin,
        )
    print(json.dumps(report, indent=2))
    return int(report.get("status") == "failed")


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)
