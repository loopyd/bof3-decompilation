"""Synchronize lift progress tags from a decomp-status report."""

from __future__ import annotations

import re
from pathlib import Path
from typing import Any

from harness.domain.tags import parse_progress_tags

_PROGRESS_RE = re.compile(r"\n\s*\* @(?:status|match|residual) [^\n]*")
_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)


def sync_lift_metadata(root: Path, report: dict[str, Any]) -> int:
    """Write current status tags to every lift in a decomp-status report."""

    changed = 0
    for target in report["targets"]:
        for row in target["functions"]:
            source = root / row["source"]
            text = source.read_text(encoding="utf-8")
            current = parse_progress_tags(text)
            status = row["status"]
            match = None if row["match_percent"] is None else float(row["match_percent"])
            if current is not None and current[0] == status and current[1] == match:
                continue
            counts = row["instruction_count"]
            if status == "exact":
                residual = "none; live audit is instruction- and byte-exact."
            elif status == "partial":
                residual = (
                    f"non-exact live audit: {counts['matching']}/{counts['original']} instructions; "
                    f"{row['original_size']} original bytes versus {row['current_size']} current."
                )
            else:
                residual = f"invalid live audit: {row['reason']}"
            match_tag = "unavailable" if match is None else f"{match:.2f}"
            cleaned = _PROGRESS_RE.sub("", text)
            tags = (
                f"\n * @status {status}\n"
                f" * @match {match_tag}\n"
                f" * @residual {residual}"
            )
            source_tag = f"@source {row['address']}"
            matches = [
                match for match in _COMMENT_RE.finditer(cleaned) if source_tag in match.group(0)
            ]
            if len(matches) != 1:
                raise ValueError(f"expected one function @source block in {source}")
            block = matches[0]
            replacement = block.group(0)[:-2].rstrip() + tags + "\n */"
            updated = cleaned[: block.start()] + replacement + cleaned[block.end() :]
            source.write_text(updated, encoding="utf-8")
            changed += 1
    return changed


__all__ = ["sync_lift_metadata"]
