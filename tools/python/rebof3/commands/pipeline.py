from __future__ import annotations

import argparse
from collections.abc import Mapping
from typing import Any

from ..core import Pipeline
from ..pipelines.registry import PipelineRegistry, build_default_registry
from ._common import run_main


def print_pipeline_list(registry: PipelineRegistry) -> None:
    for registration in registry.list():
        print(f"{registration.name}\t{registration.description}")


def format_pipeline_help(registry: PipelineRegistry) -> str:
    lines = ["pipelines:"]
    for registration in registry.list():
        lines.append(f"  {registration.name:<18} {registration.description}")
    return "\n".join(lines)


def print_pipeline_plan(pipeline: Pipeline) -> None:
    print(f"{pipeline.name}: {pipeline.description}")
    for index, task in enumerate(pipeline.plan(), start=1):
        print(f"{index}. {task.name}: {task.description}")


def print_pipeline_result(result: Any) -> None:
    if result is None:
        return
    if isinstance(result, Mapping):
        for key, value in result.items():
            print(f"{key}: {value}")
        return
    print(result)


def run_pipeline_command(args: argparse.Namespace) -> int:
    registry: PipelineRegistry = args.registry
    if args.list:
        print_pipeline_list(registry)
        return 0

    if args.name is None:
        args.parser.error("expected --list or NAME")

    try:
        registration = registry.get(args.name)
    except KeyError:
        choices = ", ".join(registry.names()) or "(none)"
        args.parser.error(f"unknown pipeline {args.name!r}; available: {choices}")
    pipeline = registration.build()

    if args.plan:
        print_pipeline_plan(pipeline)
        return 0

    print_pipeline_result(pipeline.run())
    return 0


def build_parser(
    registry: PipelineRegistry | None = None,
) -> argparse.ArgumentParser:
    resolved_registry = registry or build_default_registry()
    parser = argparse.ArgumentParser(
        prog="pipeline",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=format_pipeline_help(resolved_registry),
    )
    parser.add_argument("--list", action="store_true", help="list registered pipelines")
    parser.add_argument("name", nargs="?", metavar="NAME", help="pipeline to run")
    parser.add_argument("--plan", action="store_true", help="print the task plan")
    parser.set_defaults(
        handler=run_pipeline_command,
        parser=parser,
        registry=resolved_registry,
    )
    return parser


def main(
    argv: list[str] | None = None,
    registry: PipelineRegistry | None = None,
) -> int:
    return run_main(lambda: build_parser(registry), argv)


if __name__ == "__main__":
    raise SystemExit(main())
