"""Compiler variant catalog inspection and lifecycle commands."""

from ..io import repo_layout
from ..toolchain.gcc_variants import ensure_variant, load_variants, lookup_variant
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
    """Resolve (auto-installing when absent) and print the verified GCC path."""
    layout = repo_layout()
    try:
        variant = lookup_variant(layout, args.id)
        print(ensure_variant(layout, variant))
    except (ValueError, FileNotFoundError, RuntimeError, OSError) as exc:
        print(f"path {args.id}: {exc}", file=sys.stderr)
        return 2
    return 0


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

    return parser
