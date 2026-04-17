from __future__ import annotations

import argparse

from ..cli import logger_from_args, package_prog
from ..lib import options_with_logger
from ..pipelines.pipeline_candidate_prepare import pipeline_candidate_prepare
from .candidate_cli_common import (
    add_prepare_args,
    build_prepare_context,
    build_prepare_options,
    render_prepare_summary,
)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    add_prepare_args(
        parser,
        prog=package_prog("match", "candidate-prepare"),
        description="Resolve one function, generate a candidate stub, and write workspace.json.",
    )
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "candidate_prepare")
    options = options_with_logger(build_prepare_options(args), logger)
    context = pipeline_candidate_prepare().run(
        build_prepare_context(args),
        options=options,
    )
    render_prepare_summary(logger, context)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
