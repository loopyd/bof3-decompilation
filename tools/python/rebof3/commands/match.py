from __future__ import annotations

import argparse
from pathlib import Path

from ..match import (
    initialize_workspace,
    run_match_build,
    run_match_diff,
    write_match_report,
)
from ..match.workspace import (
    find_function_row,
    load_function_rows,
    resolve_workspace_path,
)
from ..paths import repo_layout
from ._common import run_main


def default_match_root() -> Path:
    return repo_layout().out_dir / "match"


def run_init(args: argparse.Namespace) -> int:
    rows = load_function_rows(args.function_index)
    row = find_function_row(rows, program=args.program, entry=args.entry)
    workspace_path = initialize_workspace(
        row,
        function_index_path=args.function_index,
        workspace_root=args.workspace_root,
        build_command=args.build_command,
        build_cwd=args.build_cwd,
        expected_artifact=args.expected_artifact,
        actual_artifact=args.actual_artifact,
        source_file=args.source_file,
    )
    print(f"workspace: {workspace_path}")
    return 0


def run_build(args: argparse.Namespace) -> int:
    workspace_path = resolve_workspace_path(
        workspace=args.workspace,
        function_index=args.function_index,
        workspace_root=args.workspace_root,
        program=args.program,
        entry=args.entry,
    )
    returncode, log_path, status_path = run_match_build(
        workspace_path,
        build_command=args.build_command,
        build_cwd=args.build_cwd,
    )
    print(f"build-log: {log_path}")
    print(f"build-status: {status_path}")
    return returncode


def run_diff(args: argparse.Namespace) -> int:
    workspace_path = resolve_workspace_path(
        workspace=args.workspace,
        function_index=args.function_index,
        workspace_root=args.workspace_root,
        program=args.program,
        entry=args.entry,
    )
    payload, diff_json_path, diff_markdown_path = run_match_diff(
        workspace_path,
        expected_artifact=args.expected_artifact,
        actual_artifact=args.actual_artifact,
    )
    print(f"status: {payload['status']}")
    print(f"diff-json: {diff_json_path}")
    print(f"diff-md: {diff_markdown_path}")
    return 0


def run_report(args: argparse.Namespace) -> int:
    payload = write_match_report(
        match_root=args.match_root,
        output_json=args.output_json,
        output_tsv=args.output_tsv,
    )
    print(f"report-json: {args.output_json}")
    print(f"report-tsv: {args.output_tsv}")
    print(f"rows: {payload['count']}")
    return 0


def add_workspace_selector_arguments(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--workspace", type=Path)
    parser.add_argument(
        "--function-index",
        type=Path,
        default=repo_layout().inventory_ghidra_function_index_path,
    )
    parser.add_argument("--workspace-root", type=Path, default=default_match_root())
    parser.add_argument("--program")
    parser.add_argument("--entry")


def configure_init_parser(parser: argparse.ArgumentParser) -> None:
    parser.add_argument(
        "--function-index",
        type=Path,
        default=repo_layout().inventory_ghidra_function_index_path,
    )
    parser.add_argument("--workspace-root", type=Path, default=default_match_root())
    parser.add_argument("--program", required=True)
    parser.add_argument("--entry", required=True)
    parser.add_argument("--build-command")
    parser.add_argument("--build-cwd", type=Path)
    parser.add_argument("--expected-artifact", type=Path)
    parser.add_argument("--actual-artifact", type=Path)
    parser.add_argument("--source-file", type=Path)
    parser.set_defaults(handler=run_init)


def configure_build_parser(parser: argparse.ArgumentParser) -> None:
    add_workspace_selector_arguments(parser)
    parser.add_argument("--build-command")
    parser.add_argument("--build-cwd", type=Path)
    parser.set_defaults(handler=run_build)


def configure_diff_parser(parser: argparse.ArgumentParser) -> None:
    add_workspace_selector_arguments(parser)
    parser.add_argument("--expected-artifact", type=Path)
    parser.add_argument("--actual-artifact", type=Path)
    parser.set_defaults(handler=run_diff)


def configure_report_parser(parser: argparse.ArgumentParser) -> None:
    default_root = default_match_root()
    parser.add_argument("--match-root", type=Path, default=default_root)
    parser.add_argument(
        "--output-json", type=Path, default=default_root / "report.json"
    )
    parser.add_argument("--output-tsv", type=Path, default=default_root / "report.tsv")
    parser.set_defaults(handler=run_report)


def configure_root_parser(parser: argparse.ArgumentParser) -> None:
    subparsers = parser.add_subparsers(required=True)

    init = subparsers.add_parser("init")
    configure_init_parser(init)

    build = subparsers.add_parser("build")
    configure_build_parser(build)

    diff = subparsers.add_parser("diff")
    configure_diff_parser(diff)

    report = subparsers.add_parser("report")
    configure_report_parser(report)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="match")
    configure_root_parser(parser)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
