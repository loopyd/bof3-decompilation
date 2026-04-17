from __future__ import annotations

from pathlib import Path

from ....common import run_command
from ....inventory.layout import INVENTORY_SQLITE
from .capture import (
    capture_into_inventory,
    disambiguate_program_slugs,
    preflight_capture,
    slugify,
)
from .ghidra_bridge import (
    ghidra_cli_env,
    ghidra_metadata_payload as _ghidra_metadata_payload,
    load_known_type_names,
    run_ghidra_metadata,
)
from .parser import build_parser, execute, main, parse_args
from .planning import (
    KIND_CHOICES,
    build_plan,
    selected_program_paths,
    selected_program_selectors,
)
from .reporting import render_from_report, render_to_report
from .service import DEFAULT_METADATA_SYNC_SERVICE, MetadataSyncService


__all__ = [
    "DEFAULT_METADATA_SYNC_SERVICE",
    "INVENTORY_SQLITE",
    "KIND_CHOICES",
    "MetadataSyncService",
    "Path",
    "_ghidra_metadata_payload",
    "build_parser",
    "build_plan",
    "capture_into_inventory",
    "disambiguate_program_slugs",
    "execute",
    "ghidra_cli_env",
    "load_known_type_names",
    "main",
    "parse_args",
    "preflight_capture",
    "render_from_report",
    "render_to_report",
    "run_command",
    "run_ghidra_metadata",
    "selected_program_paths",
    "selected_program_selectors",
    "slugify",
]
