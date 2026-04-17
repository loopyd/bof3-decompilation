from __future__ import annotations

import argparse
from pathlib import Path

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import relative_to_root
from ..config import DEFAULT_PSX_PROFILE
from . import report_refresh
from . import scoreboard as scoreboard_lib
from . import status as status_lib


def refresh_outputs(
    *,
    inventory_db: Path,
    match_root: Path,
    source_root: Path,
    artifact_root: Path,
    profile: str = DEFAULT_PSX_PROFILE,
    tracked_output: bool = False,
    refresh_reports: bool = True,
    refresh_status: bool = True,
    build_artifact_manifest: Path | None = status_lib.DEFAULT_BUILD_ARTIFACT_MANIFEST,
) -> dict[str, Path]:
    refreshed = report_refresh.refresh_report_artifacts(
        profile=profile,
        tracked_output=tracked_output,
        inventory_db=inventory_db,
        match_root=match_root,
        source_root=source_root,
        artifact_root=artifact_root,
        refresh_reports=refresh_reports,
        refresh_status=refresh_status,
        build_artifact_manifest=build_artifact_manifest,
    )
    return {name: Path(path) for name, path in refreshed.items()}


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "refresh"),
        description="Refresh scoreboard, backlog, and status outputs.",
    )
    add_logging_args(parser)
    parser.add_argument(
        "-i",
        "--inventory-db",
        type=Path,
        default=scoreboard_lib.DEFAULT_INVENTORY_DB,
    )
    parser.add_argument(
        "-m",
        "--match-root",
        type=Path,
        default=scoreboard_lib.DEFAULT_MATCH_ROOT,
    )
    parser.add_argument(
        "-s",
        "--source-root",
        type=Path,
        default=scoreboard_lib.DEFAULT_SOURCE_ROOT,
    )
    parser.add_argument(
        "-a",
        "--artifact-root",
        type=Path,
        default=scoreboard_lib.workspace_lib.DEFAULT_GHIDRA_ARTIFACT_ROOT,
    )
    parser.add_argument("-P", "--profile", default=DEFAULT_PSX_PROFILE)
    parser.add_argument(
        "--build-artifact-manifest",
        type=Path,
        default=status_lib.DEFAULT_BUILD_ARTIFACT_MANIFEST,
    )
    parser.add_argument("-t", "--tracked-output", action="store_true")
    parser.add_argument("--no-status", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_refresh")
    refreshed = refresh_outputs(
        inventory_db=args.inventory_db,
        match_root=args.match_root,
        source_root=args.source_root,
        artifact_root=args.artifact_root,
        profile=args.profile,
        tracked_output=bool(args.tracked_output),
        refresh_reports=True,
        refresh_status=not bool(args.no_status),
        build_artifact_manifest=args.build_artifact_manifest,
    )
    logger.summary(
        " ".join(
            [
                f"artifacts={len(refreshed)}",
                f"reports={relative_to_root(report_refresh.default_report_output_dir(args.match_root))}",
                *(
                    [f"status={relative_to_root(refreshed['status_root'])}"]
                    if "status_root" in refreshed
                    else []
                ),
            ]
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
