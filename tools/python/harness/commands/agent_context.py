"""Emit bounded, role-specific BOF3 context in one read-only command."""

from __future__ import annotations

import argparse
from pathlib import Path

from ..context import parse_cleanup_request, profile_names, render_context
from ..domain.ids import FUNCTION_ID_HELP, parse_function_id
from ._common import add_root_argument, run_main


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="agent-context", description=__doc__)
    parser.add_argument(
        "role", nargs="?", default="agents", choices=sorted(profile_names())
    )
    parser.add_argument(
        "scope",
        nargs="*",
        metavar="SCOPE",
        help=f"cleanup request tokens or {FUNCTION_ID_HELP}",
    )
    parser.add_argument(
        "->",
        "--rename-to",
        dest="rename_to",
        nargs=1,
        metavar="NEW",
        help="shell-safe transport for cleanup symbol/type OLD -> NEW",
    )
    parser.add_argument(
        "--target",
        help="historical cleanup target form; normalized to audit-target",
    )
    parser.add_argument(
        "--parent-compatibility",
        action="store_true",
        help="allow temporary parent-only old audit normalization",
    )
    parser.add_argument(
        "--mode",
        choices=("stable", "compatibility"),
        default="stable",
        help="bounded tracked prefill (default) or legacy-compatible full context",
    )
    add_root_argument(parser, default=_repository_root())
    parser.add_argument("--example", action="store_true", help=argparse.SUPPRESS)
    parser.set_defaults(
        handler=_run,
        argument_validator=_validate_arguments,
        error_prefix="",
        example_text="bin/agent-context worker",
    )
    return parser


def _validate_arguments(
    parser: argparse.ArgumentParser, args: argparse.Namespace
) -> None:
    args.function = None
    args.cleanup = None
    if args.rename_to is not None:
        args.scope.extend(("->", args.rename_to[0]))
    if args.target is not None:
        if args.role != "cleanup" or args.scope:
            parser.error("--target requires cleanup role and no request tokens")
        if args.mode == "compatibility":
            args.target_compatibility = args.target
        else:
            args.target_compatibility = None
            args.cleanup = parse_cleanup_request(
                ("audit-target", args.target), root=args.root
            )
        return
    args.target_compatibility = None
    if args.mode == "compatibility" and args.role == "agents":
        if len(args.scope) > 1:
            parser.error(f"unrecognized arguments: {' '.join(args.scope[1:])}")
        if args.scope:
            try:
                args.function = _selector(args.scope[0])
            except argparse.ArgumentTypeError as error:
                parser.error(str(error))
        return
    if args.role == "cleanup":
        if args.mode == "compatibility" and len(args.scope) == 1:
            try:
                args.function = _selector(args.scope[0])
            except argparse.ArgumentTypeError:
                pass
            else:
                return
        try:
            args.cleanup = parse_cleanup_request(
                args.scope,
                parent_compatibility=args.parent_compatibility,
                root=args.root,
            )
        except ValueError as error:
            parser.error(str(error))
        return
    if args.parent_compatibility:
        parser.error("--parent-compatibility requires cleanup role")
    if args.rename_to is not None:
        parser.error("-> transport requires cleanup symbol or type mode")
    if args.mode == "compatibility" and args.role not in {
        "reverse",
        "review",
        "cleanup",
    }:
        if len(args.scope) > 1:
            parser.error(f"unrecognized arguments: {' '.join(args.scope[1:])}")
        if args.scope:
            try:
                _selector(args.scope[0])
            except argparse.ArgumentTypeError as error:
                parser.error(str(error))
        return
    if args.role in {"reverse", "review"}:
        if len(args.scope) != 1:
            parser.error(f"{args.role} requires a function selector")
        try:
            args.function = _selector(args.scope[0])
        except argparse.ArgumentTypeError as error:
            parser.error(str(error))
    elif args.scope and args.mode == "stable":
        parser.error(f"{args.role} does not accept a function selector")


def _selector(value: str):
    try:
        return parse_function_id(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError(f"expected {FUNCTION_ID_HELP}") from error


def _run(args: argparse.Namespace) -> int:
    print(
        render_context(
            args.root.resolve(),
            args.role,
            args.function,
            target=args.target_compatibility,
            mode=args.mode,
            cleanup=args.cleanup,
        ),
        end="",
    )
    return 0


def _repository_root() -> Path:
    return Path(__file__).resolve().parents[4]


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
