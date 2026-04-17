from __future__ import annotations

import argparse

from ..cli import logger_from_args, package_prog
from ..lib import options_with_logger
from ..pipelines.pipeline_candidate_full import pipeline_candidate_full
from .candidate_cli_common import (
    add_permuter_args,
    add_profile_arg,
    add_prepare_args,
    build_build_options,
    build_prepare_context,
    build_prepare_options,
    render_build_summary,
    render_permuter_summary,
    render_prepare_summary,
)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    add_prepare_args(
        parser,
        prog=package_prog("match", "candidate-full"),
        description="Run the full candidate pipeline for one function, including permuter setup.",
    )
    add_profile_arg(parser)
    add_permuter_args(parser)
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "candidate_full")
    options = build_prepare_options(args)
    options.update(build_build_options(args))
    options = dict(options_with_logger(options, logger))
    context = pipeline_candidate_full().run(
        build_prepare_context(args),
        options=options,
    )
    render_prepare_summary(logger, context)
    render_build_summary(logger, context)
    render_permuter_summary(logger, context)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
