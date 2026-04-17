from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from ..common import utc_now

HISTORY_BASENAME = "history.jsonl"
STALL_THRESHOLD = 3


def history_path(workspace_dir: Path) -> Path:
    return workspace_dir / HISTORY_BASENAME


def load_entries_from_path(path: Path) -> list[dict[str, Any]]:
    if not path.exists():
        return []
    entries: list[dict[str, Any]] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        try:
            loaded = json.loads(line)
        except json.JSONDecodeError:
            continue
        if isinstance(loaded, dict):
            entries.append(dict(loaded))
    return entries


def load_entries(workspace_dir: Path) -> list[dict[str, Any]]:
    return load_entries_from_path(history_path(workspace_dir))


def append_entry(workspace_dir: Path, entry: dict[str, Any]) -> dict[str, Any]:
    path = history_path(workspace_dir)
    path.parent.mkdir(parents=True, exist_ok=True)
    normalized = dict(entry)
    normalized.setdefault("ran_at", utc_now())
    with path.open("a", encoding="utf-8") as handle:
        handle.write(json.dumps(normalized, sort_keys=True) + "\n")
    return normalized


def _optional_float(value: Any) -> float | None:
    if value in (None, ""):
        return None
    try:
        return float(value)
    except (TypeError, ValueError):
        return None


def _latest_field(entries: list[dict[str, Any]], key: str) -> Any:
    for entry in reversed(entries):
        value = entry.get(key)
        if value not in (None, ""):
            return value
    return None


def summarize_entries(
    entries: list[dict[str, Any]], *, stall_threshold: int = STALL_THRESHOLD
) -> dict[str, Any]:
    attempt_count = len(entries)
    build_attempt_count = 0
    diff_attempt_count = 0
    permuter_attempt_count = 0
    timed_out_permuter_attempt_count = 0
    scored_attempt_count = 0
    non_improving_scored_attempts = 0
    best_match: float | None = None
    best_asm_score: float | None = None
    latest_match: float | None = None
    latest_asm_score: float | None = None
    last_scored_improved: bool | None = None

    for entry in entries:
        event = str(entry.get("event") or "")
        if event == "build":
            build_attempt_count += 1
            continue
        if event == "permuter":
            permuter_attempt_count += 1
            timed_out_permuter_attempt_count += int(bool(entry.get("timed_out")))
            continue
        if event != "diff":
            continue

        diff_attempt_count += 1
        metrics = entry.get("match_metrics") or {}
        match_value = _optional_float(metrics.get("objdiff_match_percent"))
        asm_value = _optional_float(metrics.get("asm_score"))
        if match_value is None and asm_value is None:
            continue

        scored_attempt_count += 1
        latest_match = match_value
        latest_asm_score = asm_value

        improved = False
        if match_value is not None and (best_match is None or match_value > best_match):
            improved = True
        if asm_value is not None and (
            best_asm_score is None or asm_value < best_asm_score
        ):
            improved = True

        if improved:
            non_improving_scored_attempts = 0
        else:
            non_improving_scored_attempts += 1
        last_scored_improved = improved

        if match_value is not None and (best_match is None or match_value > best_match):
            best_match = match_value
        if asm_value is not None and (
            best_asm_score is None or asm_value < best_asm_score
        ):
            best_asm_score = asm_value

    return {
        "program_path": _latest_field(entries, "program_path"),
        "entry_hex": _latest_field(entries, "entry_hex"),
        "history_path": None,
        "attempt_count": attempt_count,
        "build_attempt_count": build_attempt_count,
        "diff_attempt_count": diff_attempt_count,
        "permuter_attempt_count": permuter_attempt_count,
        "timed_out_permuter_attempt_count": timed_out_permuter_attempt_count,
        "scored_attempt_count": scored_attempt_count,
        "best_objdiff_match_percent": best_match,
        "best_asm_score": best_asm_score,
        "latest_objdiff_match_percent": latest_match,
        "latest_asm_score": latest_asm_score,
        "last_scored_improved": last_scored_improved,
        "non_improving_scored_attempts": non_improving_scored_attempts,
        "stall_threshold": stall_threshold,
        "stalled": scored_attempt_count > 0
        and non_improving_scored_attempts >= stall_threshold,
    }


def summarize_workspace(
    workspace_dir: Path, *, stall_threshold: int = STALL_THRESHOLD
) -> dict[str, Any]:
    path = history_path(workspace_dir)
    summary = summarize_entries(
        load_entries_from_path(path),
        stall_threshold=stall_threshold,
    )
    summary["history_path"] = str(path)
    return summary
