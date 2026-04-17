from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..program_identity import classify_program_kind
from . import pipeline_ready


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "target"),
        description="Resolve one canonical function target and show its workspace identity.",
    )
    add_logging_args(parser)
    pipeline_ready.add_workspace_resolver_args(parser)
    parser.add_argument(
        "--json", action="store_true", help="Emit machine-readable JSON."
    )
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def build_payload(
    workspace_json: Path, workspace_payload: dict[str, Any]
) -> dict[str, Any]:
    source_mapping = workspace_payload.get("source_mapping") or {}
    source_hint = workspace_payload.get("source_hint")
    program_path = str(workspace_payload.get("program_path") or "")
    return {
        "workspace_json": str(workspace_json),
        "workspace_dir": workspace_payload.get("workspace_dir"),
        "program_path": program_path,
        "program_kind": classify_program_kind(program_path, str(source_hint or "")),
        "entry_hex": workspace_payload.get("entry_hex"),
        "source_file": source_mapping.get("source_file"),
        "source_function": source_mapping.get("source_function"),
        "source_hint": source_hint,
        "ghidra_bundle_json": workspace_payload.get("ghidra_decomp_bundle_json"),
    }


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_target")
    resolved = pipeline_ready.resolve_workspace(args, logger)
    if resolved is None:
        return 1
    workspace_json, workspace_payload = resolved
    payload = build_payload(workspace_json, workspace_payload)
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
        return 0
    logger.summary(
        f"{payload['program_path']} {payload['entry_hex']} -> {payload['workspace_dir']}"
    )
    if payload.get("source_file"):
        logger.item(f"source {payload['source_file']}:{payload['source_function']}")
    if payload.get("ghidra_bundle_json"):
        logger.item(f"bundle {payload['ghidra_bundle_json']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
