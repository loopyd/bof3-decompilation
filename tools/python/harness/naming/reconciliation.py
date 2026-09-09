"""Add missing canonical inventory rows without replacing retained evidence."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path

from harness.domain.ids import normalize_target_id
from harness.io import unique_object
from harness.naming.audit import initialize, validate
from harness.naming.campaign import resolve_campaign_report
from harness.naming.editing import report_mutation
from harness.naming.proposal import atomic_write_if_unchanged


def reconcile(root: Path, target: str, *, apply: bool = False) -> dict:
    """Preview or atomically append only missing blocked rows to one report."""

    target = normalize_target_id(target).value
    path = resolve_campaign_report(root, target)
    with report_mutation(path, report=True):
        if resolve_campaign_report(root, target) != path or path.stat().st_nlink != 1:
            raise ValueError("report identity changed or has multiple hard links")
        original = path.read_bytes()
        report = json.loads(original, object_pairs_hook=unique_object)
        if not isinstance(report, dict) or not isinstance(report.get("rows"), list):
            raise ValueError("report must contain a rows array")
        fresh = initialize(root, target)
        expected = {(row["kind"], row["name"]): row for row in fresh["rows"]}
        seen = set()
        for row in report["rows"]:
            if not isinstance(row, dict):
                raise ValueError("rows must be objects")
            key = (str(row.get("kind")), str(row.get("name")))
            if key in seen or key not in expected:
                raise ValueError(f"duplicate or extra inventory row: {key}")
            seen.add(key)
        missing = sorted(expected.keys() - seen)
        candidate = dict(report)
        candidate["rows"] = report["rows"] + [expected[key] for key in missing]
        if missing:
            candidate["complete"] = False
        # Validate retained evidence too; never refresh its facts or conclusions.
        validation = validate(root, target, candidate, report_path=path)
        if apply and missing:
            atomic_write_if_unchanged(path, original, candidate)
        return {
            "target": target,
            "report": str(path.relative_to(root)),
            "added": [f"{kind}:{name}" for kind, name in missing],
            "applied": bool(apply and missing),
            "before_sha256": hashlib.sha256(original).hexdigest(),
            "after_sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
            "validation": validation,
        }
