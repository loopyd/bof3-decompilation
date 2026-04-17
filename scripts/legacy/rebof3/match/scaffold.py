from __future__ import annotations

import argparse
import fnmatch
import json
import shutil
from pathlib import Path
from typing import Any

from ..cli import add_logging_args, logger_from_args, package_prog
from ..common import (
    ROOT,
    default_artifacts_dir,
    format_hex,
    parse_hexish,
    parse_source_spec,
    relative_to_root,
    write_json_output,
    write_text_output,
)
from ..config import DEFAULT_GHIDRA_DECOMP_ROOT, DEFAULT_MATCH_ROOT, DEFAULT_PSX_PROFILE
from ..re.services.ghidra.decomp_runtime import run_decomp_bundle
from ..stubs import sync as stub_sync
from . import report_refresh
from . import scoreboard as scoreboard_lib


def default_output_path(match_root: Path, profile: str) -> Path:
    return match_root / "_reports" / f"scaffold_{profile.replace('-', '_')}.json"


def normalize_entry_hex(value: Any) -> str:
    return format_hex(parse_hexish(str(value or "0")))


def preferred_repo_function_path(row: dict[str, Any]) -> Path | None:
    source_file = str(row.get("source_file") or "")
    if source_file:
        return Path(source_file)

    program_path = str(row.get("program_path") or "")
    entry_hex = normalize_entry_hex(row.get("entry_hex"))
    if not program_path.startswith("/bins/BIN/"):
        return None

    promoted_rel = stub_sync.promoted_target_path(program_path, entry_hex)
    if (ROOT / promoted_rel).exists():
        return promoted_rel
    return stub_sync.stub_target_path(program_path, entry_hex)


def stub_creation_path(row: dict[str, Any]) -> Path | None:
    if row.get("source_file"):
        return None

    program_path = str(row.get("program_path") or "")
    entry_hex = normalize_entry_hex(row.get("entry_hex"))
    if entry_hex not in stub_sync.CONFIRMED_STUB_ENTRY_HEXES:
        return None
    if not program_path.startswith("/bins/BIN/"):
        return None

    promoted_rel = stub_sync.promoted_target_path(program_path, entry_hex)
    if (ROOT / promoted_rel).exists():
        return None
    return stub_sync.stub_target_path(program_path, entry_hex)


def asm_target_paths(
    repo_function_path: Path, *, asm_root: Path = Path("bof3/asm")
) -> dict[str, Path] | None:
    repo_parts = repo_function_path.parts
    if repo_parts[:2] == ("bof3", "src"):
        relative_function = repo_function_path.relative_to(Path("bof3/src"))
    elif repo_parts[:2] == ("bof3", "stubs"):
        relative_function = repo_function_path.relative_to(Path("bof3/stubs"))
    else:
        return None

    base = asm_root / relative_function.with_suffix("")
    return {
        "asm": base.with_suffix(".s"),
        "m2c": Path(str(base) + ".m2c.c"),
        "ghidra_c": Path(str(base) + ".ghidra.c"),
    }


def artifacts_dir_for_row(row: dict[str, Any], artifact_root: Path) -> Path | None:
    source_hint = str(row.get("source_hint") or "")
    if not source_hint:
        return None
    source_spec = parse_source_spec(source_hint)
    source_path = source_spec.path
    if not source_path.is_absolute():
        source_path = (ROOT / source_path).resolve()
    return default_artifacts_dir(
        artifact_root,
        source_path,
        parse_hexish(str(row.get("entry_hex") or row.get("entry") or "0")),
        source_spec.entry_index,
    )


def bundle_file_paths(artifacts_dir: Path) -> dict[str, Path]:
    return {
        "json": artifacts_dir / "func.json",
        "asm": artifacts_dir / "func.s",
        "m2c": artifacts_dir / "func.m2c.c",
        "ghidra_c": artifacts_dir / "func.ghidra.c",
    }


def bundle_ready(artifacts_dir: Path, *, include_m2c: bool) -> bool:
    bundle_files = bundle_file_paths(artifacts_dir)
    if not bundle_files["json"].exists() or not bundle_files["asm"].exists():
        return False
    if include_m2c and not bundle_files["m2c"].exists():
        return False
    return True


def path_matches(path: Path, pattern: str | None) -> bool:
    if not pattern:
        return True
    return fnmatch.fnmatch(path.as_posix(), pattern)


def path_excluded(path: Path, patterns: list[str] | None) -> bool:
    if not patterns:
        return False
    return any(fnmatch.fnmatch(path.as_posix(), pattern) for pattern in patterns)


def select_items(
    scoreboard_payload: dict[str, Any],
    *,
    families: set[str] | None,
    program_kinds: set[str] | None,
    path_glob: str | None,
    exclude_globs: list[str] | None,
    limit: int | None,
) -> list[dict[str, Any]]:
    items: list[dict[str, Any]] = []
    for row in scoreboard_payload.get("functions") or []:
        family = str(row.get("family") or "")
        if families and family not in families:
            continue

        program_kind = str(row.get("program_kind") or "")
        if program_kinds and program_kind not in program_kinds:
            continue
        if (
            parse_hexish(str(row.get("entry_hex") or row.get("entry") or "0"))
            < 0x80000000
        ):
            continue

        repo_path = preferred_repo_function_path(row)
        if repo_path is None or not path_matches(repo_path, path_glob):
            continue
        if path_excluded(repo_path, exclude_globs):
            continue

        item = {
            "family": family,
            "program_kind": program_kind,
            "program_path": str(row.get("program_path") or ""),
            "entry_hex": normalize_entry_hex(row.get("entry_hex")),
            "source_hint": row.get("source_hint"),
            "source_file": row.get("source_file"),
            "repo_path": repo_path.as_posix(),
            "stub_path": None,
        }
        stub_path = stub_creation_path(row)
        if stub_path is not None:
            item["stub_path"] = stub_path.as_posix()
        items.append(item)

    items.sort(
        key=lambda item: (
            item["family"],
            item["program_path"],
            parse_hexish(item["entry_hex"]),
        )
    )
    if limit is not None:
        return items[: max(limit, 0)]
    return items


def create_stub(
    item: dict[str, Any], *, dry_run: bool, requested: bool
) -> tuple[str, str | None]:
    stub_path_text = item.get("stub_path")
    if not requested:
        return "disabled", None
    if not stub_path_text:
        return "not_applicable", None

    stub_path = ROOT / str(stub_path_text)
    if stub_path.exists():
        return "existing", relative_to_root(stub_path)
    if dry_run:
        return "would_create", relative_to_root(stub_path)

    write_text_output(
        stub_path,
        stub_sync.stub_file_text(str(item.get("entry_hex") or "")),
    )
    return "created", relative_to_root(stub_path)


def mirror_artifacts(
    *,
    repo_path: Path,
    asm_root: Path,
    artifacts_dir: Path,
    dry_run: bool,
    include_m2c: bool,
    export_planned: bool,
) -> dict[str, Any]:
    targets = asm_target_paths(repo_path, asm_root=asm_root)
    if targets is None:
        return {"status": "unsupported_repo_path"}

    mirrored: dict[str, Any] = {"status": "ok"}
    source_paths = bundle_file_paths(artifacts_dir)
    for key in ("asm", "m2c"):
        if key == "m2c" and not include_m2c:
            mirrored[key] = {"status": "disabled", "path": None}
            continue

        source_path = source_paths[key]
        target_path = ROOT / targets[key]
        target_rel = relative_to_root(target_path)
        if not source_path.exists():
            mirrored[key] = {
                "status": "would_copy"
                if dry_run and export_planned
                else "missing_source",
                "path": target_rel,
            }
            continue
        if dry_run:
            mirrored[key] = {"status": "would_copy", "path": target_rel}
            continue

        target_path.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source_path, target_path)
        mirrored[key] = {"status": "copied", "path": target_rel}
    return mirrored


def summarize_counts(items: list[dict[str, Any]]) -> dict[str, int]:
    counts = {
        "stub_created": 0,
        "bundle_exported": 0,
        "bundle_failed": 0,
        "asm_copied": 0,
        "m2c_copied": 0,
    }
    for item in items:
        if str(item.get("stub_action") or "") == "created":
            counts["stub_created"] += 1
        if str(item.get("bundle_action") or "") == "exported":
            counts["bundle_exported"] += 1
        if str(item.get("bundle_action") or "") == "failed":
            counts["bundle_failed"] += 1
        if ((item.get("mirror") or {}).get("asm") or {}).get("status") == "copied":
            counts["asm_copied"] += 1
        if ((item.get("mirror") or {}).get("m2c") or {}).get("status") == "copied":
            counts["m2c_copied"] += 1
    return counts


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=package_prog("match", "scaffold"),
        description=(
            "Batch-create conservative stubs and mirror asm/m2c evidence into "
            "bof3/asm for functions with derivable repo paths."
        ),
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
        default=DEFAULT_MATCH_ROOT,
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
        default=DEFAULT_GHIDRA_DECOMP_ROOT,
    )
    parser.add_argument("-f", "--family", action="append")
    parser.add_argument(
        "-k",
        "--program-kind",
        action="append",
        choices=("bin", "boot", "logo", "other"),
    )
    parser.add_argument("--path-glob")
    parser.add_argument("--exclude-glob", action="append")
    parser.add_argument("--limit", type=int)
    parser.add_argument("--asm-root", type=Path, default=Path("bof3/asm"))
    parser.add_argument("--no-stubs", action="store_true")
    parser.add_argument("--no-asm", action="store_true")
    parser.add_argument("--no-m2c", action="store_true")
    parser.add_argument("--refresh-bundles", action="store_true")
    parser.add_argument("-n", "--dry-run", action="store_true")
    parser.add_argument("-o", "--output-json", type=Path)
    parser.add_argument("-r", "--refresh-status", action="store_true")
    parser.add_argument("-t", "--tracked-output", action="store_true")
    return parser


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    return build_parser().parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    logger = logger_from_args(args, "match_scaffold")
    if not args.inventory_db.exists():
        logger.error(f"inventory db not found: {args.inventory_db}")
        return 1

    scoreboard_payload = scoreboard_lib.build_scoreboard_payload(
        inventory_db=args.inventory_db,
        match_root=args.match_root,
        source_root=args.source_root,
        artifact_root=args.artifact_root,
    )
    families = None if not args.family else {str(family) for family in args.family}
    program_kinds = (
        None if not args.program_kind else {str(kind) for kind in args.program_kind}
    )
    items = select_items(
        scoreboard_payload,
        families=families,
        program_kinds=program_kinds,
        path_glob=args.path_glob,
        exclude_globs=args.exclude_glob,
        limit=args.limit,
    )

    processed: list[dict[str, Any]] = []
    failed = False
    for item in items:
        repo_path = Path(str(item["repo_path"]))
        artifacts_dir = artifacts_dir_for_row(item, args.artifact_root)
        bundle_action = "skipped_missing_source_hint"
        bundle_payload: dict[str, Any] | None = None
        bundle_paths = (
            None if artifacts_dir is None else bundle_file_paths(artifacts_dir)
        )
        bundle_error = None

        stub_action, created_stub_path = create_stub(
            item,
            dry_run=bool(args.dry_run),
            requested=not bool(args.no_stubs),
        )

        if artifacts_dir is not None:
            should_export = bool(args.refresh_bundles) or not bundle_ready(
                artifacts_dir, include_m2c=not bool(args.no_m2c)
            )
            if should_export:
                returncode, bundle_payload = run_decomp_bundle(
                    source_text=str(item.get("source_hint") or ""),
                    address_text=str(item.get("entry_hex") or ""),
                    artifacts_dir=artifacts_dir,
                    no_m2c=bool(args.no_m2c),
                    dry_run=bool(args.dry_run),
                )
                if args.dry_run:
                    bundle_action = "dry_run"
                elif returncode == 0 and bundle_payload is not None:
                    bundle_action = "exported"
                else:
                    bundle_action = "failed"
                    bundle_error = f"ghidra_decomp failed with returncode={returncode}"
                    failed = True
            else:
                bundle_action = "reused"
                if bundle_paths is not None and bundle_paths["json"].exists():
                    try:
                        bundle_payload = json.loads(
                            bundle_paths["json"].read_text(encoding="utf-8")
                        )
                    except (OSError, UnicodeDecodeError, json.JSONDecodeError):
                        bundle_payload = None

        mirror = {"status": "disabled"}
        if not args.no_asm and artifacts_dir is not None:
            mirror = mirror_artifacts(
                repo_path=repo_path,
                asm_root=args.asm_root,
                artifacts_dir=artifacts_dir,
                dry_run=bool(args.dry_run),
                include_m2c=not bool(args.no_m2c),
                export_planned=bundle_action in {"dry_run", "exported"},
            )

        processed.append(
            {
                **item,
                "repo_path": repo_path.as_posix(),
                "stub_action": stub_action,
                "created_stub_path": created_stub_path,
                "artifacts_dir": None
                if artifacts_dir is None
                else relative_to_root(artifacts_dir),
                "bundle_json": None
                if bundle_paths is None
                else relative_to_root(bundle_paths["json"]),
                "bundle_action": bundle_action,
                "bundle_error": bundle_error,
                "bundle_payload": None
                if bundle_payload is None or args.dry_run
                else {
                    "files": bundle_payload.get("files"),
                    "m2c": bundle_payload.get("m2c"),
                },
                "mirror": mirror,
            }
        )

    counts = summarize_counts(processed)
    report = {
        "inventory_db": relative_to_root(args.inventory_db),
        "source_root": relative_to_root(args.source_root),
        "artifact_root": relative_to_root(args.artifact_root),
        "dry_run": bool(args.dry_run),
        "families": sorted(families) if families else None,
        "program_kinds": sorted(program_kinds) if program_kinds else None,
        "path_glob": args.path_glob,
        "exclude_globs": list(args.exclude_glob or []),
        "asm_root": relative_to_root(args.asm_root),
        "item_count": len(processed),
        **counts,
        "items": processed,
    }
    output_json = args.output_json or default_output_path(
        args.match_root, DEFAULT_PSX_PROFILE
    )
    write_json_output(output_json, report)

    logger.summary(
        " ".join(
            [
                f"items={len(processed)}",
                f"stubs={counts['stub_created']}",
                f"bundles={counts['bundle_exported']}",
                f"asm={counts['asm_copied']}",
                f"m2c={counts['m2c_copied']}",
                f"failed={counts['bundle_failed']}",
                f"json={relative_to_root(output_json)}",
            ]
        )
    )
    if args.refresh_status:
        status_root = report_refresh.refresh_status_snapshot(
            profile=DEFAULT_PSX_PROFILE,
            tracked_output=bool(args.tracked_output),
            inventory_db=args.inventory_db,
            match_root=args.match_root,
            source_root=args.source_root,
            artifact_root=args.artifact_root,
        )
        logger.item(f"status {relative_to_root(status_root)}")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
