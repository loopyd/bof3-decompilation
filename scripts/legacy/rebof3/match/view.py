from __future__ import annotations

import argparse

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import normalize_repo_path
from . import (
    asm_differ_backend,
    diff as diff_lib,
    pipeline_ready,
    workspace as workspace_lib,
)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "view"),
        description=(
            "Prepare one asm-differ workspace and launch the interactive "
            "side-by-side viewer."
        ),
    )
    add_logging_args(parser)
    pipeline_ready.add_workspace_resolver_args(parser)
    parser.add_argument(
        "--refresh-ghidra-bundle",
        action="store_true",
        help="Run the recorded ghidra_decomp command when func.json is missing",
    )
    parser.add_argument("--dry-run", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_view")
    resolved = pipeline_ready.resolve_workspace(args, logger)
    if resolved is None:
        return 1
    workspace_json, workspace_payload = resolved
    state = pipeline_ready.build_workspace_state(workspace_json, workspace_payload)

    if args.dry_run:
        logger.summary(
            f"workspace={state.workspace_payload.get('workspace_dir')} refresh_ghidra_bundle={bool(args.refresh_ghidra_bundle)}"
        )
        return 0

    refresh_result = None
    if not state.ghidra_bundle_exists and args.refresh_ghidra_bundle:
        state, refresh_result = pipeline_ready.maybe_refresh_ghidra_bundle(
            state,
            refresh=True,
        )
        if refresh_result is not None and refresh_result.returncode != 0:
            logger.error(
                "ghidra_decomp refresh failed; see "
                f"{workspace_lib.relative_to_root(state.refresh_log_path)}"
            )
            return refresh_result.returncode

    refreshed_payload = diff_lib.refresh_expected_baseline(
        state.workspace_json, state.workspace_payload
    )
    state = pipeline_ready.build_workspace_state(
        state.workspace_json, refreshed_payload
    )
    status, next_steps = diff_lib.diff_status(
        state.workspace_payload,
        build_status=state.build_status,
        ghidra_bundle_exists=state.ghidra_bundle_exists,
    )
    if status != "ready_for_backend_diff":
        logger.error(f"workspace is not ready for asm-differ view: {status}")
        for step in next_steps:
            logger.item(f"- {step}")
        return 1

    try:
        prepared = asm_differ_backend.prepare_backend(
            state.workspace_dir, state.workspace_payload
        )
    except (FileNotFoundError, ValueError) as exc:
        logger.error(str(exc))
        return 1

    logger.summary(
        "workspace="
        f"{state.workspace_payload.get('workspace_dir')} "
        f"asm_differ_dir={prepared['backend_dir']}"
    )
    result = asm_differ_backend.run_viewer(prepared)
    if result.returncode != 0:
        logger.error(f"asm-differ viewer exited with code {result.returncode}")
    return int(result.returncode)


if __name__ == "__main__":
    raise SystemExit(main())
