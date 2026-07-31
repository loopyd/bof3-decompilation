"""Compiler variant catalog inspection and resolution commands."""

from __future__ import annotations

import argparse
import json
import sys

from ..compiler_config import resolve_compiler_variant, set_environment_for_variant
from ..io import repo_layout
from ..toolchain.gcc_variants import load_variants, lookup_variant, sha256_file
from ._common import run_main


def _cmd_list(args: argparse.Namespace) -> int:
    layout = repo_layout()
    try:
        candidates = load_variants(layout, validate=args.validate)
        if not candidates:
            print('{"schema":"harness.compiler-variants/v1","candidates":[],"status":"empty"}')
            return 0
        payload = {"schema": "harness.compiler-variants/v1", "candidates": [
            {"id": v.id, "label": v.label} for v in candidates
        ]}
        print(json.dumps(payload, indent=2))
    except ValueError as exc:
        print(f"compiler-variants: schema validation error: {exc}", file=sys.stderr)
        return 2
    return 0


def _cmd_install(args: argparse.Namespace) -> int:
    layout = repo_layout()
    try:
        variant = lookup_variant(layout, args.id)
        status = variant.install(layout, force=args.force)
        print(status)
    except (ValueError, FileNotFoundError, RuntimeError, OSError) as exc:
        print(f"install {args.id}: {exc}", file=sys.stderr)
        return 2
    return 0


def _cmd_verify(args: argparse.Namespace) -> int:
    layout = repo_layout()
    try:
        variant = lookup_variant(layout, args.id)
        status = variant.verify(layout)
        identity = variant.verify_identity(layout)
        print(f"{args.id}: {status}")
        print(f"version: {identity.split(chr(10))[0]}")
    except (ValueError, FileNotFoundError, RuntimeError) as exc:
        print(f"verify {args.id}: {exc}", file=sys.stderr)
        return 2
    return 0


def _cmd_path(args: argparse.Namespace) -> int:
    """Resolve and print the verified GCC path for a compiler ID."""
    layout = repo_layout()
    try:
        variant = lookup_variant(layout, args.id)
        variant.verify(layout)
        exe = variant.install_path(layout) / variant.executable_relpath
        if not exe.is_file():
            print(f"path {args.id}: compiler not found at {exe}", file=sys.stderr)
            return 2
        print(exe.resolve())
    except (ValueError, FileNotFoundError, RuntimeError) as exc:
        print(f"path {args.id}: {exc}", file=sys.stderr)
        return 2
    return 0


def _cmd_resolve(args: argparse.Namespace) -> int:
    layout = repo_layout()
    variant = resolve_compiler_variant(layout)
    print(variant.id)
    return 0


def _cmd_env(args: argparse.Namespace) -> int:
    layout = repo_layout()
    variant = resolve_compiler_variant(layout)
    env = set_environment_for_variant(layout, variant)
    for key, value in sorted(env.items()):
        if value:
            print(f"export {key}={value!r}")
        else:
            print(f"unset {key}")
    return 0


def _cmd_sha256(args: argparse.Namespace) -> int:
    layout = repo_layout()
    try:
        candidates = load_variants(layout, validate=True)
    except ValueError as exc:
        print(f"sha256: schema validation error: {exc}", file=sys.stderr)
        return 2

    if not candidates:
        print("sha256: empty catalog")
        return 0

    failed = 0
    for v in candidates:
        archive = layout.downloads_dir / v.archive_name
        if archive.is_file():
            computed = sha256_file(archive)
            match = "OK" if computed == v.checksum else "MISMATCH"
            if match == "MISMATCH":
                failed += 1
            print(f"{v.id}: {computed} ({match})")
        else:
            print(f"{v.id}: {v.checksum} (not downloaded)")
    return 2 if failed else 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="compiler-variants")
    subparsers = parser.add_subparsers(dest="command")

    p = subparsers.add_parser("list", help="Show catalog entries")
    p.add_argument("--no-validate", dest="validate", action="store_false",
                   default=True, help="Skip full schema validation")
    p.set_defaults(handler=_cmd_list)

    p = subparsers.add_parser("install", help="Download and install a variant")
    p.add_argument("id", help="Catalog ID to install")
    p.add_argument("--force", action="store_true", help="Re-install even if present")
    p.set_defaults(handler=_cmd_install)

    p = subparsers.add_parser("verify", help="Verify installed variant")
    p.add_argument("id", help="Catalog ID to verify")
    p.set_defaults(handler=_cmd_verify)

    p = subparsers.add_parser("path", help="Print verified GCC path for CMake")
    p.add_argument("id", help="Catalog ID")
    p.set_defaults(handler=_cmd_path)

    p = subparsers.add_parser("resolve", help="Print resolved variant ID")
    p.set_defaults(handler=_cmd_resolve)

    p = subparsers.add_parser("env", help="Print environment exports")
    p.set_defaults(handler=_cmd_env)

    p = subparsers.add_parser("sha256", help="Compute SHA-256 of downloaded archives")
    p.set_defaults(handler=_cmd_sha256)

    return parser


def run(args: argparse.Namespace) -> int:
    cmd = getattr(args, "command", None)
    if cmd is None:
        # Default: resolve
        return _cmd_resolve(args)
    handler = getattr(args, "handler", None)
    if handler is None:
        print(f"compiler-variants: unknown command {cmd!r}", file=sys.stderr)
        return 2
    return handler(args)


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
