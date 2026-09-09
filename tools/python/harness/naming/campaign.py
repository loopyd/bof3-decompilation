"""Canonical naming-campaign report identity and resolution."""

from __future__ import annotations

import json
from pathlib import Path

from ..domain.ids import normalize_target_id

CAMPAIGN_REPORT_DIRECTORY = Path("out/reviews/plan-audit-naming")
CAMPAIGN_ACCOUNT_SCHEMA = "bof3.naming-audit-account/v1"
_CAMPAIGN_REPAIR = "rerun bin/naming-audit init-all out/reviews/plan-audit-naming"


def campaign_report_filename(target: str) -> str:
    """Return the single campaign filename for a normalized target ID."""
    return f"{normalize_target_id(target).value.replace('/', '__')}.json"


def _lexical_campaign_path(root: Path, relative: Path) -> Path:
    """Return a fixed campaign path only when no lexical component is a symlink."""
    if relative.is_absolute() or ".." in relative.parts:
        raise ValueError(
            f"canonical naming campaign path escapes repository; {_CAMPAIGN_REPAIR}"
        )
    path = root.absolute() / relative
    current = root.absolute()
    for part in relative.parts:
        current /= part
        if current.is_symlink():
            raise ValueError(
                f"canonical naming campaign path contains a symlink; {_CAMPAIGN_REPAIR}"
            )
    return path


def _recorded_campaign_path(root: Path, recorded: str) -> Path:
    """Require the manifest spelling to name the fixed lexical campaign path."""
    path = Path(recorded)
    return path if path.is_absolute() else root.absolute() / path


def resolve_campaign_report(root: Path, target: str) -> Path:
    """Resolve one manifest-accounted report from the fixed campaign directory."""
    normalized = str(normalize_target_id(target))
    filename = campaign_report_filename(normalized)
    expected = _lexical_campaign_path(root, CAMPAIGN_REPORT_DIRECTORY / filename)
    summary_path = _lexical_campaign_path(
        root, CAMPAIGN_REPORT_DIRECTORY / "summary.json"
    )
    try:
        summary = json.loads(summary_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise ValueError(
            f"canonical naming campaign summary is unavailable; {_CAMPAIGN_REPAIR}"
        ) from error
    entries = summary.get("targets")
    if summary.get("schema") != CAMPAIGN_ACCOUNT_SCHEMA or not isinstance(
        entries, list
    ):
        raise ValueError(
            f"canonical naming campaign summary is malformed; {_CAMPAIGN_REPAIR}"
        )
    matches = [
        entry
        for entry in entries
        if isinstance(entry, dict) and entry.get("target") == normalized
    ]
    if len(matches) != 1:
        state = "missing" if not matches else "ambiguous"
        raise ValueError(
            f"canonical naming campaign target entry is {state}; {_CAMPAIGN_REPAIR}"
        )
    recorded = matches[0].get("report")
    if (
        not isinstance(recorded, str)
        or _recorded_campaign_path(root, recorded) != expected
    ):
        raise ValueError(
            f"canonical naming campaign report path mismatch; {_CAMPAIGN_REPAIR}"
        )
    if not expected.is_file():
        raise ValueError(
            f"canonical naming campaign report is missing: "
            f"{CAMPAIGN_REPORT_DIRECTORY.as_posix()}/{filename}; {_CAMPAIGN_REPAIR}"
        )
    return expected
