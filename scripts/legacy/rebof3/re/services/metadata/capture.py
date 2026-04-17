from __future__ import annotations

import json
import re
import tempfile
from pathlib import Path

from ....config import GHIDRA_MAIN_MODULE, ROOT
from ....inventory.db.connection import inventory_db
from ....inventory.layout import INVENTORY_SQLITE
from ....inventory.repositories.metadata import MetadataRepository
from ....inventory.repositories.programs import ProgramRepository
from ....models.inventory import InventoryFunctionRow, InventoryProgramRow
from ....program_identity import infer_source_hint, slugify
from .ghidra_bridge import ghidra_cli_env
from .planning import KIND_CHOICES, selected_program_paths


GHIDRA_PATH_ENTRY_RE = re.compile(
    r"^(?P<archive>.+)_e(?P<entry>[0-9]+)_(?P<load>[0-9a-fA-F]{8})\.bin$",
    re.IGNORECASE,
)


def disambiguate_program_slugs(programs: list[dict[str, str]]) -> dict[str, str]:
    counts: dict[str, int] = {}
    slugs: dict[str, str] = {}
    for program in sorted(programs, key=lambda item: item["program_path"]):
        base_slug = slugify(program["program_path"])
        next_count = counts.get(base_slug, 0) + 1
        counts[base_slug] = next_count
        slugs[program["program_path"]] = (
            base_slug if next_count == 1 else f"{base_slug}_{next_count}"
        )
    return slugs


def _canonical_programs_by_hint(
    db_path: Path,
) -> tuple[dict[str, dict[str, str]], dict[str, str]]:
    by_hint: dict[str, dict[str, str]] = {}
    by_path: dict[str, str] = {}
    with inventory_db(db_path) as connection:
        rows = connection.execute(
            "SELECT program_path, program_name, folder, source_hint FROM programs ORDER BY program_path"
        ).fetchall()
    for row in rows:
        program_path = str(row["program_path"] or "")
        if not program_path:
            continue
        program_name = str(row["program_name"] or Path(program_path).name)
        folder = str(row["folder"] or "")
        by_path[program_path] = program_path
        source_hint = str(row["source_hint"] or "").strip()
        if source_hint:
            by_hint[source_hint] = {
                "program_path": program_path,
                "program_name": program_name,
                "folder": folder,
                "source_hint": source_hint,
            }
    return by_hint, by_path


def _canonical_program_record_from_ghidra_path(
    ghidra_program_path: str,
    *,
    by_hint: dict[str, dict[str, str]],
    by_path: dict[str, str],
) -> dict[str, str]:
    normalized = "/" + str(ghidra_program_path or "").strip("/")
    if normalized == "/SLUS_004.22" and "/boot/SLUS_004.22" in by_path:
        normalized = "/boot/SLUS_004.22"
    elif normalized == "/LOGO/LOGO.EXE" and "/boot/LOGO/LOGO.EXE" in by_path:
        normalized = "/boot/LOGO/LOGO.EXE"
    canonical_path = by_path.get(normalized)
    if canonical_path is not None:
        source_hint = infer_source_hint(
            canonical_path,
            str(Path(canonical_path).parent).replace("//", "/"),
            Path(canonical_path).name,
        )
        return {
            "program_path": canonical_path,
            "program_name": Path(canonical_path).name,
            "folder": str(Path(canonical_path).parent).replace("//", "/"),
            "source_hint": source_hint or "",
        }
    if normalized in {"/boot/SLUS_004.22", "/boot/LOGO/LOGO.EXE"}:
        folder = str(Path(normalized).parent).replace("//", "/")
        source_hint = infer_source_hint(normalized, folder, Path(normalized).name)
        return {
            "program_path": normalized,
            "program_name": Path(normalized).name,
            "folder": folder,
            "source_hint": source_hint or "",
        }

    folder = str(Path(normalized).parent).replace("//", "/")
    program_name = Path(normalized).name
    source_hint = infer_source_hint(normalized, folder, program_name)
    if source_hint and source_hint in by_hint:
        return dict(by_hint[source_hint])

    match = GHIDRA_PATH_ENTRY_RE.match(program_name)
    if match is not None:
        archive_id = str(Path(folder).as_posix()).strip("/")
        if archive_id.startswith("bins/"):
            archive_id = archive_id[len("bins/") :]
        elif archive_id.startswith("overlays/"):
            archive_id = archive_id[len("overlays/") :]
        fallback_source_hint = (
            source_hint
            if source_hint
            else f"build/extracted/{archive_id}.EMI#{int(match.group('entry'))}"
        )
        if fallback_source_hint in by_hint:
            return dict(by_hint[fallback_source_hint])
        canonical_name = f"{int(match.group('entry'))}.bin"
        canonical_path = f"/bins/{archive_id}/{canonical_name}"
        return {
            "program_path": canonical_path,
            "program_name": canonical_name,
            "folder": f"/bins/{archive_id}",
            "source_hint": fallback_source_hint,
        }

    return {
        "program_path": normalized,
        "program_name": program_name,
        "folder": folder,
        "source_hint": source_hint or "",
    }


def _normalize_captured_row(
    row: dict[str, object],
    *,
    program_record: dict[str, str],
) -> dict[str, object]:
    payload = dict(row)
    payload["program_path"] = program_record["program_path"]
    source_hint = program_record.get("source_hint")
    if source_hint and not payload.get("source"):
        payload["source"] = source_hint
    return payload


def _row_address_key(row: dict[str, object]) -> str:
    address_value = (
        row.get("address") or row.get("entry") or row.get("path") or row.get("name")
    )
    return str(address_value or "").strip().lower()


def _row_address_int(row: dict[str, object]) -> int | None:
    address_text = str(row.get("address") or row.get("entry") or "").strip()
    if not address_text:
        return None
    try:
        return int(address_text, 16)
    except ValueError:
        return None


def _row_extra(row: dict[str, object]) -> dict[str, object]:
    extra: dict[str, object] = {}
    for key, value in row.items():
        if key in {
            "kind",
            "address",
            "entry",
            "path",
            "program_path",
            "name",
            "comment",
            "repeatable_comment",
            "type_spec",
            "source",
            "confidence",
            "tags",
        }:
            continue
        extra[key] = value
    return extra


def _persist_rows(
    *,
    db_path: Path,
    rows: list[dict[str, object]],
    program_records: dict[str, dict[str, str]],
) -> dict[str, int]:
    program_slugs = disambiguate_program_slugs(list(program_records.values()))
    metadata_count = 0
    function_count = 0
    with inventory_db(db_path) as connection:
        programs = ProgramRepository(connection)
        metadata = MetadataRepository(connection)
        for program_record in sorted(
            program_records.values(), key=lambda item: item["program_path"]
        ):
            programs.upsert_program(
                InventoryProgramRow(
                    program_slug=program_slugs[program_record["program_path"]],
                    program_name=program_record["program_name"],
                    program_path=program_record["program_path"],
                    folder=program_record["folder"],
                    source_hint=program_record.get("source_hint") or None,
                )
            )
        for row in rows:
            program_path = str(row.get("program_path") or "")
            address_key = _row_address_key(row)
            row_kind = str(row.get("kind") or "")
            metadata.upsert_row(
                row_key=f"{program_path}|{row_kind}|{address_key}",
                program_path=program_path,
                kind=row_kind,
                address_key=address_key or None,
                address=_row_address_int(row),
                entry_text=(
                    None if row.get("entry") is None else str(row.get("entry"))
                ),
                path=None if row.get("path") is None else str(row.get("path")),
                name=None if row.get("name") is None else str(row.get("name")),
                comment=(
                    None if row.get("comment") is None else str(row.get("comment"))
                ),
                repeatable_comment=(
                    None
                    if row.get("repeatable_comment") is None
                    else str(row.get("repeatable_comment"))
                ),
                type_spec=(
                    None if row.get("type_spec") is None else str(row.get("type_spec"))
                ),
                source=(None if row.get("source") is None else str(row.get("source"))),
                confidence=(
                    None
                    if row.get("confidence") is None
                    else str(row.get("confidence"))
                ),
                tags=[str(tag) for tag in row.get("tags", []) if str(tag).strip()],
                extra=_row_extra(row),
            )
            metadata_count += 1
            if row_kind != "function":
                continue
            address_int = _row_address_int(row)
            if address_int is None:
                continue
            programs.upsert_function(
                InventoryFunctionRow(
                    program_slug=program_slugs[program_path],
                    entry_address=address_int,
                    entry_hex=f"0x{address_int:08x}",
                    name=str(row.get("name") or f"0x{address_int:08x}"),
                    signature=(
                        None
                        if row.get("type_spec") is None
                        else str(row.get("type_spec"))
                    ),
                    body_min=(
                        None
                        if row.get("body_min") in {None, ""}
                        else int(str(row.get("body_min")), 16)
                    ),
                    body_max=(
                        None
                        if row.get("body_max") in {None, ""}
                        else int(str(row.get("body_max")), 16)
                    ),
                    comment=(
                        None if row.get("comment") is None else str(row.get("comment"))
                    ),
                    repeatable_comment=(
                        None
                        if row.get("repeatable_comment") is None
                        else str(row.get("repeatable_comment"))
                    ),
                    namespace=(
                        None
                        if row.get("namespace") is None
                        else str(row.get("namespace"))
                    ),
                    name_source=(
                        None
                        if row.get("name_source") is None
                        else str(row.get("name_source"))
                    ),
                    is_thunk=bool(row.get("is_thunk", False)),
                    source_hint=program_records[program_path].get("source_hint")
                    or None,
                )
            )
            function_count += 1
    return {
        "metadata_rows": metadata_count,
        "function_rows": function_count,
        "program_rows": len(program_records),
    }


def _capture_command(
    *,
    db_path: Path,
    selectors: tuple[str, ...],
    kind: str,
    include_default: bool,
    user_defined_only: bool,
    project_dir: Path,
    project_name: str,
    output_path: Path,
    log_path: Path | None,
) -> list[str]:
    command = [
        "python3",
        "-m",
        GHIDRA_MAIN_MODULE,
        "metadata",
        "capture",
        "--db",
        str(db_path),
        "--kind",
        kind,
        "--project-dir",
        str(project_dir),
        "--project-name",
        project_name,
        "--output",
        str(output_path),
    ]
    if include_default:
        command.append("--include-default")
    if user_defined_only:
        command.append("--user-defined-only")
    for selector in selectors:
        command.extend(["--program", selector])
    if not selectors:
        command.append("--all-programs")
    if log_path is not None:
        command.extend(["--log-path", str(log_path)])
    return command


def capture_into_inventory(
    *,
    db_path: Path = INVENTORY_SQLITE,
    owner: str | None = None,
    selectors: tuple[str, ...] = (),
    kind: str = "all",
    project_dir: Path,
    project_name: str,
    include_default: bool = True,
    user_defined_only: bool = False,
    output_path: Path | None = None,
    log_path: Path | None = None,
) -> dict[str, object]:
    from . import run_command

    if kind not in KIND_CHOICES:
        raise ValueError(f"unsupported metadata kind: {kind}")
    selected = selected_program_paths(db_path=db_path, owner=owner, selectors=selectors)
    with tempfile.TemporaryDirectory(
        prefix="rebof3_metadata_sync_from_", dir=ROOT / "tmp"
    ) as temp_dir:
        temp_root = Path(temp_dir)
        capture_db = temp_root / "capture.sqlite"
        capture_output = temp_root / "capture.json"
        command = _capture_command(
            db_path=capture_db,
            selectors=selected,
            kind=kind,
            include_default=include_default,
            user_defined_only=user_defined_only,
            project_dir=project_dir,
            project_name=project_name,
            output_path=capture_output,
            log_path=log_path,
        )
        result = run_command(command, cwd=ROOT, env=ghidra_cli_env())
        if result.returncode != 0:
            raise RuntimeError("ghidra metadata capture failed")
        payload = json.loads(capture_output.read_text(encoding="utf-8"))

    by_hint, by_path = _canonical_programs_by_hint(db_path)
    normalized_rows: list[dict[str, object]] = []
    program_records: dict[str, dict[str, str]] = {}
    ghidra_programs: set[str] = set()
    for row in payload.get("rows", []):
        if not isinstance(row, dict):
            continue
        ghidra_program_path = str(row.get("program_path") or "")
        program_record = _canonical_program_record_from_ghidra_path(
            ghidra_program_path,
            by_hint=by_hint,
            by_path=by_path,
        )
        normalized_row = _normalize_captured_row(row, program_record=program_record)
        normalized_rows.append(normalized_row)
        program_records[program_record["program_path"]] = program_record
        if ghidra_program_path:
            ghidra_programs.add(ghidra_program_path)
    persisted = _persist_rows(
        db_path=db_path,
        rows=normalized_rows,
        program_records=program_records,
    )
    report = {
        "schema": "bof3.metadata.sync_from/v1",
        "db": str(db_path),
        "kind": kind,
        "owner": owner,
        "selectors": list(selected),
        "project_dir": str(project_dir),
        "project_name": project_name,
        "include_default": include_default,
        "user_defined_only": user_defined_only,
        "ghidra_program_count": len(ghidra_programs),
        "canonical_program_count": len(program_records),
        "row_count": len(normalized_rows),
        "persisted": persisted,
        "rows": normalized_rows,
    }
    if output_path is not None:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(
            json.dumps(report, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
    return report


def preflight_capture(
    *,
    db_path: Path = INVENTORY_SQLITE,
    owner: str | None = None,
    selectors: tuple[str, ...] = (),
    kind: str = "all",
    project_dir: Path,
    project_name: str,
    include_default: bool = True,
    user_defined_only: bool = False,
    output_path: Path | None = None,
    log_path: Path | None = None,
) -> dict[str, object]:
    from . import run_command

    if kind not in KIND_CHOICES:
        raise ValueError(f"unsupported metadata kind: {kind}")
    selected = selected_program_paths(db_path=db_path, owner=owner, selectors=selectors)
    with tempfile.TemporaryDirectory(
        prefix="rebof3_metadata_sync_from_", dir=ROOT / "tmp"
    ) as temp_dir:
        temp_root = Path(temp_dir)
        capture_db = temp_root / "capture.sqlite"
        capture_output = temp_root / "capture.json"
        command = _capture_command(
            db_path=capture_db,
            selectors=selected,
            kind=kind,
            include_default=include_default,
            user_defined_only=user_defined_only,
            project_dir=project_dir,
            project_name=project_name,
            output_path=capture_output,
            log_path=log_path,
        )
        result = run_command(command, cwd=ROOT, env=ghidra_cli_env())
        if result.returncode != 0:
            raise RuntimeError("ghidra metadata capture failed")
        payload = json.loads(capture_output.read_text(encoding="utf-8"))

    by_hint, by_path = _canonical_programs_by_hint(db_path)
    normalized_rows: list[dict[str, object]] = []
    program_records: dict[str, dict[str, str]] = {}
    ghidra_programs: set[str] = set()
    for row in payload.get("rows", []):
        if not isinstance(row, dict):
            continue
        ghidra_program_path = str(row.get("program_path") or "")
        program_record = _canonical_program_record_from_ghidra_path(
            ghidra_program_path,
            by_hint=by_hint,
            by_path=by_path,
        )
        normalized_row = _normalize_captured_row(row, program_record=program_record)
        normalized_rows.append(normalized_row)
        program_records[program_record["program_path"]] = program_record
        if ghidra_program_path:
            ghidra_programs.add(ghidra_program_path)

    report = {
        "schema": "bof3.metadata.sync_from/v1",
        "mode": "preflight",
        "db": str(db_path),
        "kind": kind,
        "owner": owner,
        "selectors": list(selected),
        "project_dir": str(project_dir),
        "project_name": project_name,
        "include_default": include_default,
        "user_defined_only": user_defined_only,
        "ghidra_program_count": len(ghidra_programs),
        "canonical_program_count": len(program_records),
        "row_count": len(normalized_rows),
        "persisted": {
            "program_rows": 0,
            "function_rows": 0,
            "metadata_rows": 0,
        },
        "rows": normalized_rows,
    }
    if output_path is not None:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        output_path.write_text(
            json.dumps(report, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
    return report
