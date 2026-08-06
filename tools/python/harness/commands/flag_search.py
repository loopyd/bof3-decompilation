"""Search the reviewed compiler flag catalog for one target-qualified function."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from ..domain import FUNCTION_ID_HELP, load_target_manifests, parse_function_id
from ..io import repo_layout
from ..match.flag_search import search_flags
from ..toolchain.gcc_variants import EmptyCatalog, lookup_variant
from ._common import add_example_argument, run_main


def run(args: argparse.Namespace) -> int:
    layout = repo_layout()
    function = parse_function_id(args.function)
    manifest = load_target_manifests(layout.root).get(function.target.value)
    if manifest is None:
        raise ValueError(f"unknown target: {function.target.value}")
    from ._lift_m2c import resolve_function

    _, _, source = resolve_function(args.function)
    if not source.is_file():
        raise FileNotFoundError(f"lifted source does not exist: {source}")

    # Resolve optional compiler variant before search.
    compiler_id = args.compiler
    if compiler_id is not None:
        variant = lookup_variant(layout, compiler_id)
        if isinstance(variant, EmptyCatalog):
            raise ValueError(
                f"compiler variant {compiler_id!r} not available (empty catalog)"
            )
        variant.verify(layout)

    payload = search_flags(
        layout=layout,
        source=source,
        catalog_path=args.catalog
        or layout.root / "config" / "compiler" / "flag-catalog.json",
        compiler_id=compiler_id,
    )
    text = json.dumps(payload, indent=2, sort_keys=True) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0 if payload["exact_matches"] else 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="flag-search")
    parser.add_argument("function", help=FUNCTION_ID_HELP)
    parser.add_argument("--catalog", type=Path)
    parser.add_argument(
        "--compiler", type=str, help="catalog ID for a historical GCC variant"
    )
    parser.add_argument("-o", "--out", type=Path)
    add_example_argument(parser, "bin/flag-search exe/logo@0x801CE758")
    parser.set_defaults(handler=run)
    return parser


def main(argv: list[str] | None = None) -> int:
    return run_main(build_parser, argv)


if __name__ == "__main__":
    raise SystemExit(main())
