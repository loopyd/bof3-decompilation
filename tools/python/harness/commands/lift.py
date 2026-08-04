"""Target-qualified entry points for the one-function lifting loop.

These commands deliberately resolve a ``TARGET@0xADDRESS`` before touching a
source file.  This prevents an identically-addressed function in another EMI
entry from accidentally becoming the build or match target.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess
import sys

from ..canonical import load_target_symbols, weak_bindings_c
from ..domain import FUNCTION_ID_HELP, FunctionId
from ..domain.manifests import TargetManifest
from ..io import repo_layout
from ..match._asm_diff_payload import AsmDiffRequest
from ..match.asm_diff import run_asm_diff_one
from ..match.asm_differ import write_bundle
from ..output import add_detail_argument, resolve_detail
from ._common import add_example_argument
from ._asm_diff_output import format_asm_diff_llm, format_asm_diff_summary

from ._lift_m2c import resolve_function, run_m2c, run_m2ctx

def _example(command: str) -> str:
    if command == "m2c":
        return "bin/m2c exe/logo@0x801CE758 -o candidate.c"
    return f"bin/{command} exe/logo@0x801CE758"

def _run_match(
    function: FunctionId,
    manifest: TargetManifest,
    source: Path,
    *,
    diagnostics: bool,
) -> dict[str, object]:
    root = repo_layout().root
    bindings = root / "out" / "bindings" / function.target.value / "symbols.c"
    symbols = load_target_symbols(root, function.target.value)
    binding_text = weak_bindings_c(symbols)
    bindings.parent.mkdir(parents=True, exist_ok=True)
    if not bindings.is_file() or bindings.read_text(encoding="utf-8") != binding_text:
        bindings.write_text(binding_text, encoding="utf-8")
    payload = run_asm_diff_one(
        AsmDiffRequest(
            source_path=source,
            address=function.address,
            binary_path=root / manifest.binary,
            load_address=manifest.load_address,
            output_root=root / "out" / "asm-diff",
            symbols_c_path=bindings,
            canonical_bindings={
                symbol.canonical_name: symbol.address for symbol in symbols
            },
            section_placements=manifest.section_placements.get(function.address, ()),
            diagnostics=diagnostics,
        )
    )
    if diagnostics:
        write_bundle(root, payload, target=function.target.value)
    return payload

def _print_match(
    payload: dict[str, object],
    *,
    json_output: bool,
    bytes_only: bool,
    detail: str | None = None,
) -> int:
    exact = bool(payload["byte_match"])
    resolved = resolve_detail(requested=detail, json_output=json_output)
    if json_output:
        if bytes_only:
            print(
                json.dumps(
                    {
                        key: payload[key]
                        for key in (
                            "function",
                            "address",
                            "original_size",
                            "current_size",
                            "byte_match",
                            "outputs",
                        )
                    },
                    sort_keys=True,
                )
            )
        else:
            projected = payload
            if resolved == "minimal":
                projected = {
                    key: payload[key]
                    for key in (
                        "schema",
                        "function",
                        "address",
                        "status",
                        "byte_match",
                        "instruction_count",
                    )
                }
            elif resolved == "normal":
                projected = {
                    key: payload[key]
                    for key in (
                        "schema",
                        "function",
                        "address",
                        "status",
                        "byte_match",
                        "instruction_count",
                        "original_size",
                        "current_size",
                        "size_delta",
                        "first_mismatch",
                    )
                }
                outputs = payload["outputs"]
                if not isinstance(outputs, dict):
                    raise ValueError("invalid asm-diff payload outputs")
                projected["diff"] = outputs["diff"]
            print(json.dumps(projected, indent=2, sort_keys=True))
    elif bytes_only:
        status = "MATCH" if exact else "DIFFER"
        print(f"{status} {payload['function']}@{payload['address']} bytes")
    else:
        root = repo_layout().root
        if resolved == "minimal":
            print(format_asm_diff_summary(payload, root=root))
        elif resolved == "normal":
            print(format_asm_diff_llm(payload, root=root))
        else:
            outputs = payload["outputs"]
            if not isinstance(outputs, dict):
                raise ValueError("invalid asm-diff payload outputs")
            diff = Path(outputs["diff"])
            print(format_asm_diff_summary(payload, root=root))
            if diff.is_file() and diff.stat().st_size:
                print(diff.read_text(encoding="utf-8"), end="")
    return 0 if exact else 1

def _require_lifted_source(function: FunctionId, source: Path) -> None:
    if not source.is_file():
        raise FileNotFoundError(
            f"lifted source does not exist: {source}; generate and review it with "
            f"bin/m2c {function.target.value}@0x{function.address:08X} -o {source}"
        )

def run_asm_diff(args: argparse.Namespace) -> int:
    function, manifest, source = resolve_function(args.function)
    _require_lifted_source(function, source)
    return _print_match(
        _run_match(function, manifest, source, diagnostics=True),
        json_output=args.json,
        bytes_only=False,
        detail=args.detail,
    )

def run_byte_match(args: argparse.Namespace) -> int:
    function, manifest, source = resolve_function(args.function)
    _require_lifted_source(function, source)
    return _print_match(
        _run_match(function, manifest, source, diagnostics=False),
        json_output=args.json,
        bytes_only=True,
    )

def run_promote(args: argparse.Namespace) -> int:
    function, manifest, source = resolve_function(args.function)
    candidate = Path(args.candidate).resolve()
    if not candidate.is_file():
        raise FileNotFoundError(f"candidate source does not exist: {candidate}")
    # Promotion intentionally never copies a candidate.  Exact matching is
    # target-local, so a candidate must first be installed at its owned path by
    # the maintainer, then this command validates it.
    if candidate != source.resolve():
        raise ValueError(
            "validate-only promotion requires the candidate at its owned source path: "
            f"{source.relative_to(repo_layout().root)}"
        )
    format_check = subprocess.run(
        ["clang-format", "--dry-run", "--Werror", str(candidate)],
        capture_output=True,
        text=True,
    )
    if format_check.returncode:
        raise RuntimeError(
            f"candidate is not clang-format clean: {format_check.stderr}"
        )
    payload = _run_match(function, manifest, source, diagnostics=True)
    result = _print_match(
        payload, json_output=args.json, bytes_only=False, detail=args.detail
    )
    if result == 0 and not args.json:
        print(
            "validated; manually retain the source, Splat boundary, and symbol-map edits"
        )
    return result

def _parser(command: str) -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog=f"bin/{command}")
    add_example_argument(parser, _example(command))
    return parser

def main(command: str, argv: list[str] | None = None) -> int:
    parser = _parser(command)
    if command == "m2ctx":
        parser.add_argument("function", nargs="?", help=FUNCTION_ID_HELP)
        parser.add_argument("-o", "--out")
        parser.add_argument("--json", action="store_true")
        parser.set_defaults(handler=run_m2ctx)
    elif command == "m2c":
        parser.add_argument("function", nargs="?", help=FUNCTION_ID_HELP)
        parser.add_argument("--void", action="store_true")
        parser.add_argument("-o", "--out")
        parser.add_argument("-c", "--context", action="append", default=[])
        parser.set_defaults(handler=run_m2c)
    elif command in {"asm-diff", "byte-match"}:
        parser.add_argument("function", nargs="?", help=FUNCTION_ID_HELP)
        parser.add_argument("--json", action="store_true")
        if command == "asm-diff":
            add_detail_argument(parser)
        parser.set_defaults(
            handler=run_asm_diff if command == "asm-diff" else run_byte_match
        )
    elif command == "promote":
        parser.add_argument("function", nargs="?", help=FUNCTION_ID_HELP)
        parser.add_argument("candidate", nargs="?")
        parser.add_argument("--json", action="store_true")
        add_detail_argument(parser)
        parser.set_defaults(handler=run_promote)
    else:
        raise ValueError(f"unknown lift command: {command}")
    args = parser.parse_args(argv)
    if args.example:
        print(args.example_text)
        return 0
    if not getattr(args, "function", None):
        parser.error(f"{FUNCTION_ID_HELP} is required")
    if command == "promote" and not args.candidate:
        parser.error("candidate.c is required")
    try:
        return args.handler(args)
    except (FileNotFoundError, RuntimeError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2
