from __future__ import annotations

import argparse
import json

from ...cli import add_logging_args, package_prog
from ...cli import logger_from_args
from ...models.metadata import MetadataSyncFromRequest, MetadataSyncToRequest
from .command import Command
from ..services import metadata as metadata_service


def _add_shared_scope_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--db", type=metadata_service.Path, default=metadata_service.INVENTORY_SQLITE
    )
    parser.add_argument("--owner")
    parser.add_argument("--program", action="append", default=[])
    parser.add_argument("--kind", default="all", choices=metadata_service.KIND_CHOICES)
    parser.add_argument(
        "--project-dir",
        type=metadata_service.Path,
        default=metadata_service.Path("tmp/bof3_ghidra/main"),
    )
    parser.add_argument("--project-name", default="bof3_main")
    parser.add_argument("--output", type=metadata_service.Path)
    parser.add_argument("--log-path", type=metadata_service.Path)
    parser.add_argument("--json", action="store_true")


class MetadataCommand(Command):
    command_name = "metadata"

    @classmethod
    def build_parser(cls) -> argparse.ArgumentParser:
        parser = argparse.ArgumentParser(
            prog=package_prog("re", "metadata"),
            description="Metadata workflows for syncing canonical SQLite state to and from Ghidra.",
        )
        add_logging_args(parser)
        commands = parser.add_subparsers(dest="command", required=True)

        sync_parser = commands.add_parser(
            "sync", help="sync metadata to or from Ghidra"
        )
        add_logging_args(sync_parser)
        directions = sync_parser.add_subparsers(dest="direction", required=True)

        to_parser = directions.add_parser(
            "to", help="sync metadata from SQLite into Ghidra"
        )
        add_logging_args(to_parser)
        _add_shared_scope_args(to_parser)
        to_parser.add_argument(
            "--mode",
            choices=("preflight", "apply", "report"),
            default="apply",
        )

        from_parser = directions.add_parser(
            "from", help="capture metadata from Ghidra into SQLite"
        )
        add_logging_args(from_parser)
        _add_shared_scope_args(from_parser)
        from_parser.add_argument(
            "--mode",
            choices=("preflight", "capture", "report"),
            default="capture",
        )
        from_parser.add_argument("--input", type=metadata_service.Path)
        from_parser.add_argument("--user-defined-only", action="store_true")
        from_parser.add_argument(
            "--include-default",
            action=argparse.BooleanOptionalAction,
            default=True,
        )
        return parser

    @classmethod
    def execute(cls, args: argparse.Namespace) -> int:
        if args.command != "sync":
            raise RuntimeError(f"unsupported metadata command: {args.command}")
        if args.direction == "to":
            return execute_to(args)
        if args.direction == "from":
            return execute_from(args)
        raise RuntimeError(f"unsupported metadata sync direction: {args.direction}")


def build_parser() -> argparse.ArgumentParser:
    return MetadataCommand.build_parser()


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return MetadataCommand.parse_args(argv)


def execute_to(args: argparse.Namespace) -> int:
    logger = logger_from_args(args, "re_metadata_sync")
    request = MetadataSyncToRequest(
        db_path=args.db,
        mode=args.mode,
        owner=args.owner,
        selectors=tuple(args.program or ()),
        kind=args.kind,
        project_dir=args.project_dir,
        project_name=args.project_name,
        output_path=args.output,
        log_path=args.log_path,
    )
    exit_code, payload, plan = (
        metadata_service.DEFAULT_METADATA_SYNC_SERVICE.execute_to(request)
    )
    if payload.get("known_types_error"):
        logger.info(f"known-type query failed: {payload['known_types_error']}")
    quiet = bool(getattr(args, "quiet", False))
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    elif not quiet:
        print(
            metadata_service.render_to_report(
                plan,
                ghidra_result=(
                    payload.get("ghidra_result")
                    if args.mode in {"preflight", "apply"}
                    else None
                ),
            )
        )
    return exit_code


def execute_from(args: argparse.Namespace) -> int:
    quiet = bool(getattr(args, "quiet", False))
    request = MetadataSyncFromRequest(
        db_path=args.db,
        mode=args.mode,
        owner=args.owner,
        selectors=tuple(args.program or ()),
        kind=args.kind,
        project_dir=args.project_dir,
        project_name=args.project_name,
        include_default=bool(args.include_default),
        user_defined_only=bool(args.user_defined_only),
        output_path=args.output,
        log_path=args.log_path,
        input_path=args.input,
    )
    report = metadata_service.DEFAULT_METADATA_SYNC_SERVICE.execute_from(request)
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
    elif not quiet:
        print(metadata_service.render_from_report(report))
    return 0


def execute(args: argparse.Namespace) -> int:
    return MetadataCommand.execute(args)


def main(argv: list[str] | None = None) -> int:
    return MetadataCommand.main(argv)
