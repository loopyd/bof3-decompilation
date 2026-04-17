from __future__ import annotations

import argparse
from pathlib import Path
from typing import Any

from ..cli import (
    add_logging_args,
    add_program_entry_args,
    logger_from_args,
    package_prog,
)
from ..common import ROOT, relative_to_root
from ..config import DEFAULT_GHIDRA_DECOMP_ROOT, DEFAULT_MATCH_ROOT
from ..inventory.layout import INVENTORY_SQLITE
from ..program_identity import normalize_program_selector, slugify
from . import source_map
from .target import (
    build_synthetic_function_row,
    bundle_supports_entry,
    find_function_row,
    find_program_row,
    infer_function_row_for_mapping,
    infer_function_row_for_program,
    infer_function_row_from_program_selector,
    load_function_rows,
    load_program_rows,
    row_matches_program,
)
from .workspace_store import (
    build_workspace_payload,
    find_workspace_json,
    ghidra_decomp_command,
    load_workspace_payload,
    refresh_workspace_json,
    suggested_artifacts_dir,
    workspace_dir,
    workspace_json_path,
    write_workspace_payload,
)


DEFAULT_INVENTORY_DB = INVENTORY_SQLITE
DEFAULT_WORKSPACE_ROOT = DEFAULT_MATCH_ROOT
DEFAULT_GHIDRA_ARTIFACT_ROOT = DEFAULT_GHIDRA_DECOMP_ROOT
LEGACY_WORKSPACE_INIT_SENTINEL = "__workspace_init_compat__"


def build_parser(*, prog: str) -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=prog,
        description="Create one function-matching workspace from the durable Ghidra symbol inventory.",
    )
    init_parser = parser
    add_logging_args(init_parser)
    add_program_entry_args(init_parser)
    init_parser.add_argument(
        "-i", "--inventory-db", type=Path, default=DEFAULT_INVENTORY_DB
    )
    init_parser.add_argument(
        "-w", "--workspace-root", type=Path, default=DEFAULT_WORKSPACE_ROOT
    )
    init_parser.add_argument(
        "-a",
        "--artifact-root",
        type=Path,
        default=DEFAULT_GHIDRA_ARTIFACT_ROOT,
        help="Root used for per-function ghidra_decomp bundles",
    )
    init_parser.add_argument(
        "-s",
        "--source",
        help="Optional source path override used to derive the ghidra_decomp bundle path",
    )
    init_parser.add_argument("-n", "--dry-run", action="store_true")
    return init_parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    args = list(argv or [])
    if args and args[0] == LEGACY_WORKSPACE_INIT_SENTINEL:
        return build_parser(prog=package_prog("match", "workspace-init")).parse_args(
            args[1:]
        )
    return build_parser(prog=package_prog("match", "init")).parse_args(args)


def init_main(args: argparse.Namespace) -> int:
    logger = logger_from_args(args, "match_workspace")
    inventory_db = args.inventory_db
    if not inventory_db.exists():
        logger.error(f"inventory db not found: {args.inventory_db}")
        return 1

    rows = load_function_rows(inventory_db)
    program_rows = load_program_rows(inventory_db)
    try:
        row = find_function_row(
            rows,
            program=args.program,
            entry=args.entry,
            program_rows=program_rows,
            artifact_root=args.artifact_root,
            source_root=source_map.DEFAULT_SOURCE_ROOT,
        )
    except LookupError as exc:
        logger.error(str(exc))
        return 1

    dir_path, payload = build_workspace_payload(
        row,
        inventory_db=inventory_db,
        workspace_root=args.workspace_root,
        artifact_root=args.artifact_root,
        source_override=args.source,
    )
    workspace_json = dir_path / "workspace.json"

    if args.dry_run:
        logger.summary(
            f"workspace={relative_to_root(dir_path)} program={payload['program_path']} entry={payload['entry_hex']}"
        )
        return 0

    write_workspace_payload(workspace_json, payload)
    logger.summary(
        f"workspace={relative_to_root(dir_path)} program={payload['program_path']} entry={payload['entry_hex']}"
    )
    return 0


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    return init_main(args)
