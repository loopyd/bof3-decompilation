from __future__ import annotations

from pathlib import Path
from typing import Any

from ..config import ROOT


def default_report_output_dir(match_root: Path) -> Path:
    return match_root / "_reports"


def local_status_output_root(profile: str) -> Path:
    return ROOT / "tmp" / "status" / profile / "current"


def tracked_status_output_root() -> Path:
    return ROOT / "reports" / "decomp-status" / "current"


def resolve_status_output_root(*, profile: str, tracked_output: bool) -> Path:
    if tracked_output:
        return tracked_status_output_root()
    return local_status_output_root(profile)


def refresh_status_snapshot(
    *,
    profile: str,
    tracked_output: bool,
    inventory_db: Path,
    match_root: Path,
    source_root: Path,
    artifact_root: Path,
    build_artifact_manifest: Path | None = None,
) -> Path:
    from . import status as status_lib

    output_root = resolve_status_output_root(
        profile=profile,
        tracked_output=tracked_output,
    )
    status_lib.refresh_status_snapshot(
        inventory_db=inventory_db,
        match_root=match_root,
        source_root=source_root,
        artifact_root=artifact_root,
        profile=profile,
        output_root=output_root,
        build_artifact_manifest=build_artifact_manifest,
    )
    return output_root


def refresh_status_snapshot_from_payload(
    *,
    profile: str,
    tracked_output: bool,
    scoreboard_payload: dict[str, Any],
    build_artifact_manifest: Path | None = None,
) -> Path:
    from . import status as status_lib

    output_root = resolve_status_output_root(
        profile=profile,
        tracked_output=tracked_output,
    )
    if build_artifact_manifest is None:
        build_artifact_manifest = status_lib.DEFAULT_BUILD_ARTIFACT_MANIFEST
    artifact_summary = status_lib.load_artifact_manifest_summary(
        build_artifact_manifest
    )
    if artifact_summary is not None:
        scoreboard_payload = {**scoreboard_payload, "artifacts": artifact_summary}
    status_lib.write_status_snapshot(
        scoreboard_payload,
        output_root=output_root,
        profile=profile,
    )
    return output_root


def refresh_report_artifacts_from_payload(
    *,
    profile: str,
    tracked_output: bool,
    match_root: Path,
    scoreboard_payload: dict[str, Any],
    refresh_reports: bool,
    refresh_status: bool,
    build_artifact_manifest: Path | None = None,
) -> dict[str, str]:
    from . import frontier_backlog as frontier_backlog_lib
    from . import enhanced_report as enhanced_report_lib
    from . import import_backlog as import_backlog_lib
    from . import scoreboard as scoreboard_lib

    refreshed: dict[str, str] = {}
    if refresh_reports:
        scoreboard_json, scoreboard_tsv = scoreboard_lib.default_output_paths(
            match_root,
            profile,
        )
        scoreboard_json.parent.mkdir(parents=True, exist_ok=True)
        scoreboard_json.write_text(
            scoreboard_lib.render_summary_json(scoreboard_payload),
            encoding="utf-8",
        )
        scoreboard_tsv.write_text(
            scoreboard_lib.render_tsv(list(scoreboard_payload.get("functions") or [])),
            encoding="utf-8",
        )
        refreshed["scoreboard_json"] = str(scoreboard_json)
        refreshed["scoreboard_tsv"] = str(scoreboard_tsv)

        import_backlog_payload = import_backlog_lib.build_import_backlog_payload(
            scoreboard_payload
        )
        import_backlog_json, import_backlog_tsv = (
            import_backlog_lib.default_output_paths(
                match_root,
                profile,
            )
        )
        import_backlog_json.write_text(
            scoreboard_lib.render_summary_json(import_backlog_payload),
            encoding="utf-8",
        )
        import_backlog_tsv.write_text(
            import_backlog_lib.render_tsv(
                list(import_backlog_payload.get("items") or [])
            ),
            encoding="utf-8",
        )
        refreshed["import_backlog_json"] = str(import_backlog_json)
        refreshed["import_backlog_tsv"] = str(import_backlog_tsv)

        frontier_payload = frontier_backlog_lib.build_frontier_backlog_payload(
            inventory_db=Path(str(scoreboard_payload.get("inventory_db") or "")),
            match_root=match_root,
            source_root=Path(str(scoreboard_payload.get("source_root") or "")),
            artifact_root=Path(
                str(
                    scoreboard_payload.get("artifact_root")
                    or scoreboard_lib.workspace_lib.DEFAULT_GHIDRA_ARTIFACT_ROOT
                )
            ),
        )
        frontier_json, frontier_tsv = frontier_backlog_lib.default_output_paths(
            match_root,
            profile,
        )
        frontier_json.write_text(
            scoreboard_lib.render_summary_json(frontier_payload),
            encoding="utf-8",
        )
        frontier_tsv.write_text(
            frontier_backlog_lib.render_tsv(list(frontier_payload.get("items") or [])),
            encoding="utf-8",
        )
        refreshed["frontier_backlog_json"] = str(frontier_json)
        refreshed["frontier_backlog_tsv"] = str(frontier_tsv)

        enhanced_payload = enhanced_report_lib.build_binary_report_payload(
            scoreboard_payload,
            profile=profile,
        )
        enhanced_json, enhanced_tsv, enhanced_md = (
            enhanced_report_lib.default_output_paths(match_root, profile)
        )
        enhanced_summary_md = enhanced_report_lib.default_summary_md_path(
            match_root,
            profile,
        )
        enhanced_json.write_text(
            scoreboard_lib.render_summary_json(enhanced_payload),
            encoding="utf-8",
        )
        enhanced_tsv.write_text(
            enhanced_report_lib.render_tsv(
                list(enhanced_payload.get("binaries") or [])
            ),
            encoding="utf-8",
        )
        enhanced_md.write_text(
            enhanced_report_lib.render_markdown(enhanced_payload, view="full"),
            encoding="utf-8",
        )
        enhanced_summary_md.write_text(
            enhanced_report_lib.render_markdown(enhanced_payload, view="summary"),
            encoding="utf-8",
        )
        refreshed["enhanced_report_json"] = str(enhanced_json)
        refreshed["enhanced_report_tsv"] = str(enhanced_tsv)
        refreshed["enhanced_report_md"] = str(enhanced_md)
        refreshed["enhanced_report_summary_md"] = str(enhanced_summary_md)
    if refresh_status:
        refreshed["status_root"] = str(
            refresh_status_snapshot_from_payload(
                profile=profile,
                tracked_output=tracked_output,
                scoreboard_payload=scoreboard_payload,
                build_artifact_manifest=build_artifact_manifest,
            )
        )
    return refreshed


def refresh_report_artifacts(
    *,
    profile: str,
    tracked_output: bool,
    inventory_db: Path,
    match_root: Path,
    source_root: Path,
    artifact_root: Path,
    refresh_reports: bool,
    refresh_status: bool,
    build_artifact_manifest: Path | None = None,
) -> dict[str, str]:
    from . import scoreboard as scoreboard_lib

    if not refresh_reports and not refresh_status:
        return {}
    scoreboard_payload = scoreboard_lib.build_scoreboard_payload(
        inventory_db=inventory_db,
        match_root=match_root,
        source_root=source_root,
        artifact_root=artifact_root,
    )
    return refresh_report_artifacts_from_payload(
        profile=profile,
        tracked_output=tracked_output,
        match_root=match_root,
        scoreboard_payload=scoreboard_payload,
        refresh_reports=refresh_reports,
        refresh_status=refresh_status,
        build_artifact_manifest=build_artifact_manifest,
    )
