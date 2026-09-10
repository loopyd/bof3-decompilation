"""Read-only transport for unified BOF3 Markdown operations and edit preparation."""

from __future__ import annotations

import argparse
import json

from harness.common.cli import (
    add_example_argument,
    add_root_argument,
    resolved_root,
    run_main,
)
from harness.docs.documents import (
    build_context,
    prepare_edit,
    read_documents,
    search_documents,
)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="docs", description=__doc__)
    add_root_argument(parser)
    add_example_argument(parser, "bin/docs compact docs/agents/documentation.md")
    commands = parser.add_subparsers(dest="command", required=True)
    for name, help_text in (
        ("context", "read numbered excerpts from explicit Markdown paths"),
        ("search", "search a literal in explicit Markdown paths"),
        ("aggregate", "collect complete documents without merging or writing them"),
        ("edit", "prepare complete snapshots for a scoped Markdown edit"),
        ("repair", "prepare complete snapshots for authority-backed repair"),
        ("compact", "prepare one complete document for reviewed AI compaction"),
    ):
        command = commands.add_parser(name, help=help_text)
        if name == "search":
            command.add_argument("pattern")
            command.add_argument("--ignore-case", action="store_true")
            command.add_argument("--limit", type=int, default=20)
        command.add_argument("paths", nargs=1 if name == "compact" else "+")
        command.add_argument("--max-bytes", type=int, default=16_384)
        if name == "context":
            command.add_argument("--start-line", type=int, default=1)
            command.add_argument("--lines", type=int, default=80)
        if name in {"edit", "repair", "compact"}:
            command.add_argument("--expected-sha256")
        command.set_defaults(handler=run_documents)
    return parser


def run_documents(args: argparse.Namespace) -> int:
    if not 512 <= args.max_bytes <= 1_048_576:
        raise ValueError("docs max-bytes must be between 512 and 1048576")
    root = resolved_root(args)
    documents = read_documents(root, args.paths)
    if args.command == "search":
        facts = search_documents(
            documents, args.pattern, ignore_case=args.ignore_case, limit=args.limit
        )
    elif args.command == "context":
        facts = {
            "documents": build_context(
                documents, start_line=args.start_line, lines=args.lines
            )
        }
    else:
        facts = {
            "documents": prepare_edit(documents, getattr(args, "expected_sha256", None))
        }
    payload = {
        "schema": "bof3.docs/v1",
        "operation": args.command,
        "root": str(root),
        "write_authorized": False,
        **facts,
    }
    rendered = json.dumps(payload, indent=2, ensure_ascii=False) + "\n"
    if len(rendered.encode("utf-8")) > args.max_bytes:
        raise ValueError(
            "docs output exceeds max-bytes; narrow scope or explicitly increase the bound"
        )
    print(rendered, end="")
    return 0


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
