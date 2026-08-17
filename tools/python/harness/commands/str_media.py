"""Inspect, validate, or convert one extracted STR stream."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import shutil

from ..media.str_media import convert_str, inspect_str, validate_str
from ._common import add_example_argument, add_root_argument, run_main


def _source(args: argparse.Namespace) -> Path:
    return args.source if args.source.is_absolute() else args.root / args.source


def _output(args: argparse.Namespace) -> Path:
    return args.output_dir or args.root / "out" / "str-media" / _source(args).stem


def _print(payload: object, as_json: bool) -> None:
    if as_json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    elif isinstance(payload, dict):
        print(
            " ".join(
                f"{key}={value}"
                for key, value in payload.items()
                if not isinstance(value, (list, dict))
            )
        )


def run_inspect(args: argparse.Namespace) -> int:
    _print(inspect_str(_source(args).read_bytes()), args.json)
    return 0


def run_validate(args: argparse.Namespace) -> int:
    result = validate_str(
        _source(args),
        _output(args),
        expected_fps=args.expected_fps,
        ffprobe=shutil.which("ffprobe"),
    )
    _print(result, args.json)
    return 1 if result["status"] == "fail" else 0


def run_convert(args: argparse.Namespace) -> int:
    result = convert_str(_source(args), _output(args), fps=args.fps, output=args.out)
    _print(result, args.json)
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="str-media")
    add_root_argument(parser)
    add_example_argument(parser, "bin/str-media inspect out/extracted/INTRO.STR")
    sub = parser.add_subparsers(dest="command")
    for name, handler in (
        ("inspect", run_inspect),
        ("validate", run_validate),
        ("convert", run_convert),
    ):
        command = sub.add_parser(name)
        command.add_argument("source", type=Path)
        command.add_argument("--output-dir", type=Path)
        command.add_argument("--json", action="store_true")
        if name == "validate":
            command.add_argument("--expected-fps", type=float)
        if name == "convert":
            command.add_argument("--fps", required=True, type=float)
            command.add_argument("-o", "--out", type=Path)
        command.set_defaults(handler=handler)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
