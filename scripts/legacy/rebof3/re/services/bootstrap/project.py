from __future__ import annotations

import fcntl
import sqlite3
import subprocess
from pathlib import Path

from . import constants


def default_inventory_db() -> Path:
    return constants.ROOT / "processed" / "inventory" / "inventory.sqlite"


def default_project_dir() -> Path:
    return constants.ROOT / "tmp" / "bof3_ghidra" / "main"


def ensure_project_marker(project_dir: Path, project_name: str) -> Path | None:
    project_gpr = project_dir / f"{project_name}.gpr"
    if project_gpr.is_file():
        return project_gpr

    project_rep = project_dir / f"{project_name}.rep"
    if not project_rep.is_dir():
        return None

    project_gpr.parent.mkdir(parents=True, exist_ok=True)
    project_gpr.touch(exist_ok=True)
    return project_gpr if project_gpr.is_file() else None


def lock_is_active(path: Path) -> bool:
    if not path.exists():
        return False
    with path.open("a+", encoding="utf-8") as handle:
        try:
            fcntl.flock(handle.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            return True
        fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
    return False


def active_project_processes(project_dir: Path) -> list[tuple[int, str]]:
    try:
        result = subprocess.run(
            ["ps", "-eo", "pid=,args="],
            capture_output=True,
            text=True,
            check=False,
        )
    except OSError:
        return []

    target = str(project_dir.resolve())
    matches: list[tuple[int, str]] = []
    for line in result.stdout.splitlines():
        normalized = line.strip()
        if not normalized or target not in normalized:
            continue
        if "analyzeHeadless" not in normalized and "ghidra.Ghidra" not in normalized:
            continue
        pid_text, _, command = normalized.partition(" ")
        try:
            pid = int(pid_text)
        except ValueError:
            continue
        matches.append((pid, command.strip()))
    return matches


def describe_active_project_processes(processes: list[tuple[int, str]]) -> str:
    previews = []
    for pid, command in processes[:3]:
        truncated = command if len(command) <= 140 else f"{command[:137]}..."
        previews.append(f"pid={pid} cmd={truncated}")
    suffix = ""
    if len(processes) > len(previews):
        suffix = f" (+{len(processes) - len(previews)} more)"
    return "; ".join(previews) + suffix


def project_busy_message(project_dir: Path) -> str | None:
    bridge_lock = project_dir / ".ghidra_bridge.session.lock"
    if lock_is_active(bridge_lock):
        return (
            "canonical Ghidra project is busy: the Ghidra bridge holds the session "
            "lock; stop the active bridge session or use a different project directory"
        )

    project_lock = project_dir / ".ghidra_project.lock"
    if lock_is_active(project_lock):
        return (
            "canonical Ghidra project is busy: another Ghidra/headless workflow "
            "currently holds the project lock; wait for it to finish and retry"
        )

    active_processes = active_project_processes(project_dir)
    if active_processes:
        process_details = describe_active_project_processes(active_processes)
        return (
            "canonical Ghidra project is busy: an active Ghidra/headless process "
            f"is already using {project_dir}; wait for it to finish and retry; "
            f"matches: {process_details}"
        )

    return None


def inventory_db_ready() -> tuple[bool, str]:
    inventory_db = default_inventory_db()
    required_tables = (
        "archives",
        "emi_entries",
        "overlay_aliases",
        "overlay_entry_tables",
    )
    if not inventory_db.exists():
        return False, str(inventory_db)

    connection = None
    try:
        connection = sqlite3.connect(inventory_db)
        rows = connection.execute(
            "SELECT name FROM sqlite_master WHERE type IN ('table', 'view')"
        ).fetchall()
    except sqlite3.Error as exc:
        return False, str(exc)
    finally:
        if connection is not None:
            connection.close()

    names = {str(row[0]) for row in rows}
    missing = [name for name in required_tables if name not in names]
    if missing:
        return False, ", ".join(missing)
    return True, str(inventory_db)


def overlay_import_rows() -> list[tuple[str, str]]:
    inventory_db = default_inventory_db()
    if not inventory_db.exists():
        return []

    connection = sqlite3.connect(inventory_db)
    try:
        return connection.execute(
            """
            SELECT archive_id, payload_path
            FROM emi_entries
            WHERE code_candidate = 1
              AND payload_path IS NOT NULL
              AND payload_path != ''
            ORDER BY archive_id, entry_index
            """
        ).fetchall()
    finally:
        connection.close()


__all__ = [
    "active_project_processes",
    "default_inventory_db",
    "default_project_dir",
    "describe_active_project_processes",
    "ensure_project_marker",
    "inventory_db_ready",
    "lock_is_active",
    "overlay_import_rows",
    "project_busy_message",
]
