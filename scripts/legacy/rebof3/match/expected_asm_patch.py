from __future__ import annotations

import argparse
import sys
from pathlib import Path

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import normalize_repo_path, write_text_output
from . import asm_differ_backend, object_slices, pipeline_ready


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "asm-patch"),
        description=(
            "Rewrite expected asm into a diff-ready form with conservative "
            "symbolized hi/lo pairs and call targets."
        ),
    )
    add_logging_args(parser)
    pipeline_ready.add_workspace_resolver_args(parser)
    parser.add_argument(
        "--baseline-asm",
        default=None,
        help="Direct mode input asm path, or '-' to read from stdin.",
    )
    parser.add_argument(
        "--baseline-symbol",
        default=None,
        help="Top-level symbol name in the input asm for direct mode.",
    )
    parser.add_argument(
        "--target-symbol",
        default=None,
        help="Rewritten top-level symbol name for direct mode.",
    )
    parser.add_argument(
        "--resolver-program-path",
        default=None,
        help=(
            "Program path used to resolve func_/DAT_ names in direct mode, "
            "for example /boot/SLUS_004.22."
        ),
    )
    parser.add_argument(
        "-o",
        "--output",
        default="-",
        help="Write patched asm to this path, or '-' for stdout.",
    )
    parser.add_argument(
        "--write-workspace",
        action="store_true",
        help="In workspace mode, also rewrite asm_differ/expected/expected.s in place.",
    )
    parser.add_argument("--dry-run", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def workspace_requested(args: argparse.Namespace) -> bool:
    return bool(getattr(args, "workspace_json", None)) or bool(
        getattr(args, "program", None) and getattr(args, "entry", None)
    )


def resolve_workspace_patch(
    args: argparse.Namespace, logger: object
) -> tuple[str, Path | None, str] | None:
    resolved = pipeline_ready.resolve_workspace(args, logger)
    if resolved is None:
        return None
    workspace_json, workspace_payload = resolved
    state = pipeline_ready.build_workspace_state(workspace_json, workspace_payload)
    state = pipeline_ready.refresh_expected_baseline(state)

    baseline_info = state.workspace_payload.get("expected_baseline") or {}
    baseline_asm_path = normalize_repo_path(baseline_info.get("asm_source"))
    baseline_symbol_name = baseline_info.get("symbol_name")
    source_mapping = state.workspace_payload.get("source_mapping") or {}
    target_symbol_name = source_mapping.get(
        "source_function"
    ) or state.workspace_payload.get("name")
    if baseline_asm_path is None or not baseline_asm_path.exists():
        logger.error("workspace is missing an expected baseline asm source")
        return None
    if not baseline_symbol_name or not target_symbol_name:
        logger.error("workspace is missing baseline/source symbol names")
        return None

    resolver = asm_differ_backend.load_program_symbol_resolver(state.workspace_payload)
    patched = object_slices.patch_expected_asm_text(
        baseline_asm_path.read_text(encoding="utf-8"),
        original_symbol_name=str(baseline_symbol_name),
        target_symbol_name=str(target_symbol_name),
        resolver=resolver,
    )
    workspace_output = (
        state.workspace_dir / "asm_differ" / "expected" / "expected.s"
        if args.write_workspace
        else None
    )
    return patched, workspace_output, str(target_symbol_name)


def resolve_direct_patch(
    args: argparse.Namespace, logger: object
) -> tuple[str, str] | None:
    if not args.baseline_asm or not args.baseline_symbol or not args.target_symbol:
        logger.error(
            "pass --workspace-json or (--program and --entry), or direct-mode "
            "--baseline-asm/--baseline-symbol/--target-symbol"
        )
        return None
    if args.baseline_asm == "-":
        asm_text = sys.stdin.read()
    else:
        asm_path = Path(args.baseline_asm)
        if not asm_path.exists():
            logger.error(f"baseline asm not found: {asm_path}")
            return None
        asm_text = asm_path.read_text(encoding="utf-8")

    resolver = None
    if args.resolver_program_path:
        resolver = asm_differ_backend.load_program_symbol_resolver(
            {
                "inventory_db": str(args.inventory_db),
                "program_path": args.resolver_program_path,
            }
        )

    patched = object_slices.patch_expected_asm_text(
        asm_text,
        original_symbol_name=str(args.baseline_symbol),
        target_symbol_name=str(args.target_symbol),
        resolver=resolver,
    )
    return patched, str(args.target_symbol)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_asm_patch")

    if args.dry_run:
        logger.summary(
            f"workspace_mode={workspace_requested(args)} output={args.output} write_workspace={bool(args.write_workspace)}"
        )
        return 0

    workspace_output: Path | None = None
    if workspace_requested(args):
        resolved = resolve_workspace_patch(args, logger)
        if resolved is None:
            return 1
        patched, workspace_output, target_symbol = resolved
    else:
        direct = resolve_direct_patch(args, logger)
        if direct is None:
            return 1
        patched, target_symbol = direct

    if workspace_output is not None:
        write_text_output(workspace_output, patched)

    if args.output != "-":
        output_path = Path(args.output)
        write_text_output(output_path, patched)
        logger.summary(f"symbol={target_symbol} output={output_path}")
        if workspace_output is not None:
            logger.item(f"workspace_expected={workspace_output}")
        return 0

    if workspace_output is not None:
        logger.summary(f"symbol={target_symbol} workspace_expected={workspace_output}")
        return 0

    sys.stdout.write(patched)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
