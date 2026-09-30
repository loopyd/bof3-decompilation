"""CLI adapter for argument and local-variable naming commands."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from harness.common.cli import resolved_root


def _print(payload: dict[str, Any]) -> None:
    print(json.dumps(payload, indent=2, sort_keys=True))


def add_source_commands(sub: argparse._SubParsersAction) -> None:
    identifiers = sub.add_parser(
        "source-identifiers",
        help="list read-only argument and local-variable naming leads",
    )
    identifiers.add_argument("target")
    identifiers.set_defaults(handler=run_source_identifiers)
    identifier = sub.add_parser(
        "describe-source-identifier",
        help="inspect one exact argument/local lead without mutation",
    )
    identifier.add_argument("target")
    identifier.add_argument("id")
    identifier.add_argument("--expected-fingerprint")
    identifier.set_defaults(handler=run_source_identifiers)
    prepare = sub.add_parser(
        "prepare-source-transaction",
        help="bind one reviewed argument/local rename to exact PRE source state",
    )
    prepare.add_argument("target")
    prepare.add_argument("--selector", required=True)
    prepare.add_argument(
        "--transaction", required=True, help="argument:NAME or local:NAME"
    )
    prepare.add_argument("--new-name", required=True)
    prepare.add_argument("--output", type=Path, required=True)
    prepare.set_defaults(handler=run_prepare_source_transaction)
    apply = sub.add_parser(
        "apply-source-transaction", help="apply one prepared argument/local rename"
    )
    apply.add_argument("target")
    apply.add_argument("receipt", type=Path)
    apply.set_defaults(handler=run_apply_source_transaction)
    verify = sub.add_parser(
        "verify-source-transaction",
        help="prove an applied argument/local rename is the only change",
    )
    verify.add_argument("target")
    verify.add_argument("receipt", type=Path)
    verify.add_argument(
        "--no-native",
        action="store_true",
        help="skip the native byte-identity gate (diagnostics only)",
    )
    verify.set_defaults(handler=run_verify_source_transaction)
    rollback = sub.add_parser(
        "rollback-source-transaction", help="restore PRE bytes after an applied rename"
    )
    rollback.add_argument("target")
    rollback.add_argument("receipt", type=Path)
    rollback.set_defaults(handler=run_rollback_source_transaction)


def run_source_identifiers(args: argparse.Namespace) -> int:
    from harness.naming.source_identifiers import (
        collect_source_identifiers,
        describe_source_identifier,
    )

    root = resolved_root(args)
    if args.command == "source-identifiers":
        print(
            json.dumps(
                collect_source_identifiers(root, args.target), indent=2, sort_keys=True
            )
        )
        return 0
    _print(
        describe_source_identifier(
            root, args.target, args.id, expected_fingerprint=args.expected_fingerprint
        )
    )
    return 0


def run_prepare_source_transaction(args: argparse.Namespace) -> int:
    from harness.naming.source_identifiers import prepare_source_transaction

    kind, _, name = args.transaction.partition(":")
    if not kind or not name:
        raise ValueError("--transaction must be KIND:NAME")
    _print(
        prepare_source_transaction(
            resolved_root(args),
            args.target,
            args.selector,
            kind=kind,
            old_name=name,
            new_name=args.new_name,
            output=args.output,
        )
    )
    return 0


def run_apply_source_transaction(args: argparse.Namespace) -> int:
    from harness.naming.source_identifiers import apply_source_transaction

    _print(apply_source_transaction(resolved_root(args), args.receipt))
    return 0


def run_verify_source_transaction(args: argparse.Namespace) -> int:
    from harness.naming.source_identifiers import verify_source_transaction

    _print(
        verify_source_transaction(
            resolved_root(args), args.receipt, native=not args.no_native
        )
    )
    return 0


def run_rollback_source_transaction(args: argparse.Namespace) -> int:
    from harness.naming.source_identifiers import rollback_source_transaction

    _print(rollback_source_transaction(resolved_root(args), args.receipt))
    return 0
