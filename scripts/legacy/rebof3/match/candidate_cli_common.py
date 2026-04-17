from __future__ import annotations

import argparse
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, add_program_entry_args, package_prog
from ..common import relative_to_root
from ..inventory.layout import INVENTORY_SQLITE
from ..logger import Rebof3Logger
from ..tasks.candidate.common import (
    DEFAULT_CANDIDATE_BUILD_ROOT,
    DEFAULT_CANDIDATE_WORKSPACE_ROOT,
)
from . import pipeline_ready


def add_prepare_args(
    parser: argparse.ArgumentParser, *, prog: str, description: str
) -> None:
    """Populate a parser with the inputs required to seed a candidate pipeline."""

    parser.prog = prog
    parser.description = description
    add_logging_args(parser)
    add_program_entry_args(parser)
    parser.add_argument(
        "-i",
        "--inventory-db",
        type=Path,
        default=INVENTORY_SQLITE,
        help="Inventory database used to resolve the function row.",
    )
    parser.add_argument(
        "-w",
        "--workspace-root",
        type=Path,
        default=DEFAULT_CANDIDATE_WORKSPACE_ROOT,
        help="Root directory for candidate workspaces.",
    )
    parser.add_argument(
        "-a",
        "--artifacts-dir",
        type=Path,
        default=None,
        help="Override the decomp bundle artifacts directory.",
    )
    parser.add_argument(
        "--build-root",
        type=Path,
        default=DEFAULT_CANDIDATE_BUILD_ROOT,
        help="Build tree used for stub compilation.",
    )
    parser.add_argument(
        "-s",
        "--source",
        dest="source_text",
        default=None,
        help="Optional source override for the Ghidra bundle input.",
    )
    parser.add_argument(
        "--asm-backend",
        choices=("ghidra", "spimdisasm"),
        default="spimdisasm",
        help="Canonical asm backend for the decomp bundle lane.",
    )
    parser.add_argument(
        "--no-spimdisasm",
        action="store_true",
        help="Skip the spimdisasm side artifact lane.",
    )
    parser.add_argument(
        "--no-m2c",
        action="store_true",
        help="Skip the automatic m2c sidecar when generating the bundle.",
    )
    parser.add_argument(
        "--candidate-variant",
        choices=("m2c", "ghidra"),
        default="m2c",
        help="Select which recovered C artifact seeds the candidate stub.",
    )
    parser.add_argument("--force-decomp", action="store_true")
    parser.add_argument("--force-rewrite-source", action="store_true")
    parser.add_argument("--force-reconfigure", action="store_true")


def add_build_args(parser: argparse.ArgumentParser) -> None:
    """Add the workspace selection and build options for existing workspaces."""

    pipeline_ready.add_workspace_resolver_args(parser)
    add_build_option_args(parser)


def add_build_option_args(parser: argparse.ArgumentParser) -> None:
    """Add the compile-profile knobs shared by build/full commands."""

    parser.add_argument(
        "--build-root",
        type=Path,
        default=DEFAULT_CANDIDATE_BUILD_ROOT,
        help="Build tree used for stub compilation.",
    )
    add_profile_arg(parser)


def add_profile_arg(parser: argparse.ArgumentParser) -> None:
    """Add the compile profile knob used by build/full pipelines."""

    parser.add_argument(
        "--profile",
        default="capcom97-bof3",
        help="Compiler profile passed into the candidate build lane.",
    )


def add_permuter_args(parser: argparse.ArgumentParser) -> None:
    """Add the small set of permuter controls we expose through the pipeline CLI."""

    parser.add_argument("--permuter-variant", default="repo")
    parser.add_argument(
        "--permuter-timeout-seconds",
        type=int,
        default=60,
    )
    parser.add_argument(
        "--permuter-threads",
        type=int,
        default=None,
    )
    parser.add_argument(
        "--permuter-arg",
        action="append",
        default=[],
        help="Append one extra raw argument for decomp-permuter.",
    )


def build_prepare_context(args: argparse.Namespace) -> dict[str, Any]:
    """Build the minimal context required by the candidate prepare/full pipelines."""

    return {
        "program_selector": args.program,
        "entry": args.entry,
        "source_text": args.source_text,
        "inventory_db": args.inventory_db,
        "workspace_root": args.workspace_root,
        "artifacts_dir": args.artifacts_dir,
        "build_root": args.build_root,
    }


def build_prepare_options(args: argparse.Namespace) -> dict[str, Any]:
    """Translate prepare/full CLI args into pipeline options."""

    return {
        "asm_backend": args.asm_backend,
        "emit_spimdisasm": not args.no_spimdisasm,
        "no_m2c": args.no_m2c,
        "candidate_variant": args.candidate_variant,
        "force_decomp": args.force_decomp,
        "force_rewrite_source": args.force_rewrite_source,
        "force_reconfigure": args.force_reconfigure,
    }


def build_build_options(args: argparse.Namespace) -> dict[str, Any]:
    """Translate build/full CLI args into pipeline options."""

    options: dict[str, Any] = {
        "profile": args.profile,
    }
    if getattr(args, "permuter_variant", None) is not None:
        options["permuter_variant"] = args.permuter_variant
    if getattr(args, "permuter_timeout_seconds", None) is not None:
        options["permuter_timeout_seconds"] = args.permuter_timeout_seconds
    if getattr(args, "permuter_threads", None) is not None:
        options["permuter_threads"] = args.permuter_threads
    if getattr(args, "permuter_arg", None):
        options["permuter_args"] = list(args.permuter_arg)
    return options


def resolve_existing_workspace_context(
    args: argparse.Namespace,
    *,
    logger: Rebof3Logger,
) -> dict[str, Any] | None:
    """Resolve an existing workspace and build the context used by build/permuter lanes."""

    resolved = pipeline_ready.resolve_workspace(args, logger)
    if resolved is None:
        return None
    workspace_json, workspace_payload = resolved
    return {
        "workspace_json": workspace_json,
        "workspace_payload": workspace_payload,
        "build_root": args.build_root,
    }


def render_prepare_summary(logger: Rebof3Logger, context: dict[str, Any]) -> None:
    """Render prepare-stage results through the shared logger interface."""

    workspace_json = relative_to_root(Path(str(context["workspace_json"])))
    candidate_source = relative_to_root(Path(str(context["candidate_source_file"])))
    bundle_payload = dict(context["bundle_payload"])
    bundle_files = bundle_payload.get("files") or {}

    logger.summary(f"workspace={workspace_json} candidate={candidate_source}")
    logger.item(f"bundle {bundle_files['json']}")
    logger.detail(f"candidate variant {context.get('candidate_source_variant')}")
    logger.detail(f"asm backend {bundle_payload.get('asm_backend')}")
    if bundle_payload.get("artifacts_dir"):
        logger.detail(f"artifacts {bundle_payload['artifacts_dir']}")
    if bundle_files.get("ghidra_asm"):
        logger.detail(f"ghidra asm {bundle_files['ghidra_asm']}")
    if bundle_files.get("spim_asm"):
        logger.detail(f"spim asm {bundle_files['spim_asm']}")
    if bundle_files.get("m2c_c"):
        logger.detail(f"m2c c {bundle_files['m2c_c']}")


def render_build_summary(logger: Rebof3Logger, context: dict[str, Any]) -> None:
    """Render compile/diff results through the shared logger interface."""

    metrics = context["diff_report"]["match_metrics"]
    workspace_json = relative_to_root(Path(str(context["workspace_json"])))
    diff_report_path = relative_to_root(Path(str(context["diff_report_path"])))

    logger.summary(
        " ".join(
            [
                f"workspace={workspace_json}",
                f"semantic={metrics['semantic_status']}",
                f"asm_score={metrics['asm_score']}/{metrics['asm_max_score']}",
            ]
        )
    )
    logger.item(f"diff {diff_report_path}")
    logger.detail(f"objdiff match {metrics['objdiff_match_percent']:.1f}%")
    logger.detail(f"asm rows {metrics['asm_row_count']}")
    build_status = context.get("build_status") or {}
    if build_status.get("log_path"):
        logger.detail(f"build log {build_status['log_path']}")
    if build_status.get("object_path"):
        logger.detail(f"object {build_status['object_path']}")


def render_permuter_summary(logger: Rebof3Logger, context: dict[str, Any]) -> None:
    """Render permuter results through the shared logger interface."""

    permuter = context.get("permuter") or {}
    if not permuter:
        return
    logger.item(f"permuter log {permuter.get('log_path')}")
    logger.detail(f"permuter returncode {permuter.get('returncode')}")
    logger.detail(f"permuter timed out {permuter.get('timed_out')}")


__all__ = [
    "add_build_args",
    "add_build_option_args",
    "add_permuter_args",
    "add_profile_arg",
    "add_prepare_args",
    "build_build_options",
    "build_prepare_context",
    "build_prepare_options",
    "render_build_summary",
    "render_permuter_summary",
    "render_prepare_summary",
    "resolve_existing_workspace_context",
]
