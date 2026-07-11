from __future__ import annotations

import argparse
from pathlib import Path

from ..setup import (
    SetupOptions,
    plan_setup_tasks,
    run_named_setup_task,
    run_setup_workspace,
    setup_task_names,
)
from ._common import run_main


def add_setup_option_flags(
    parser: argparse.ArgumentParser,
    *,
    include_force: bool = False,
    include_psyq_inputs: bool = False,
    include_skip_flags: bool = False,
) -> None:
    if include_psyq_inputs:
        parser.add_argument("--psyq-version")
        parser.add_argument("--psyq-source-root", type=Path)
        parser.add_argument("--psyq-archive", type=Path)
        parser.add_argument("--disc-archive", type=Path)
    if include_force:
        parser.add_argument("--force", action="store_true")
    if include_skip_flags:
        parser.add_argument("--with-ghidra-plan", action="store_true")
        parser.add_argument("--skip-psyq", action="store_true")
        parser.add_argument("--skip-extract", action="store_true")
        parser.add_argument("--skip-ghidra-plan", action="store_true")


def build_setup_options(args: argparse.Namespace) -> SetupOptions:
    skip_psyq = bool(getattr(args, "skip_psyq", False))
    skip_extract = bool(getattr(args, "skip_extract", False))
    skip_ghidra_plan = bool(getattr(args, "skip_ghidra_plan", False))

    if bool(getattr(args, "open_setup", False)):
        skip_psyq = True
        skip_extract = True
        skip_ghidra_plan = True

    return SetupOptions(
        force=bool(getattr(args, "force", False)),
        include_psyq=not skip_psyq,
        include_extract=not skip_extract,
        include_ghidra_plan=bool(getattr(args, "with_ghidra_plan", False))
        and not skip_ghidra_plan,
        psyq_version=getattr(args, "psyq_version", None),
        psyq_source_root=getattr(args, "psyq_source_root", None),
        psyq_archive=getattr(args, "psyq_archive", None),
        disc_archive=getattr(args, "disc_archive", None),
    )


def run_plan(args: argparse.Namespace) -> int:
    options = build_setup_options(args)
    for task in plan_setup_tasks(options):
        print(f"{task.name}: {task.description}")
    return 0


def run_workspace(args: argparse.Namespace) -> int:
    options = build_setup_options(args)
    run_setup_workspace(options)
    print("workspace setup complete")
    return 0


def run_task(args: argparse.Namespace) -> int:
    options = build_setup_options(args)
    run_named_setup_task(args.task_name, options)
    print(f"setup task complete: {args.task_name}")
    return 0


def run_flat_command(args: argparse.Namespace) -> int:
    if bool(args.plan):
        return run_plan(args)
    if args.task_name is not None:
        return run_task(args)
    return run_workspace(args)


def configure_flat_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--plan", action="store_true")
    parser.add_argument("--open", dest="open_setup", action="store_true")
    parser.add_argument("--task", dest="task_name", choices=setup_task_names())
    add_setup_option_flags(
        parser,
        include_force=True,
        include_psyq_inputs=True,
        include_skip_flags=True,
    )
    parser.set_defaults(handler=run_flat_command)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="setup")
    configure_flat_parser(parser)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
