from __future__ import annotations

import argparse

from ..cli import add_logging_args, logger_from_args, package_prog
from ..lib import options_with_logger
from ..pipelines.pipeline_candidate_build import pipeline_candidate_build
from .candidate_cli_common import (
    add_build_args,
    build_build_options,
    render_build_summary,
    resolve_existing_workspace_context,
)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "candidate-build"),
        description="Compile and diff one prepared candidate workspace.",
    )
    add_logging_args(parser)
    add_build_args(parser)
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "candidate_build")
    context = resolve_existing_workspace_context(args, logger=logger)
    if context is None:
        return 1
    result = pipeline_candidate_build().run(
        context,
        options=options_with_logger(build_build_options(args), logger),
    )
    render_build_summary(logger, result)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
