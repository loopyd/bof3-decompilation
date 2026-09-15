"""Derive exact function checks from frozen naming transaction scopes."""

from __future__ import annotations

from pathlib import Path

from harness.domain.functions import parse_function_records
from harness.domain.registry import resolve_function


def collect_function_checks(root: Path, row) -> list[dict]:
    facts = row["pre_apply"]["facts"]
    start, end = (int(value, 0) for value in facts["unchanged_range"].split(".."))
    checks = [
        {
            "selector": row["identity"]["selector"],
            "address": start,
            "size": end - start,
            "name": row["new_name"],
            "source": facts["destination"],
        }
    ]
    scope = facts["scope"]
    selectors = {checks[0]["selector"]}
    for relative in sorted(set(scope["source_locations"])):
        if relative == scope["definition"] or not relative.endswith(".c"):
            continue
        source = root / relative
        try:
            records = parse_function_records(source.read_text(encoding="utf-8"))
        except ValueError as error:
            raise ValueError(f"invalid caller metadata: {relative}: {error}") from error
        if len(records) != 1 or records[0].status != "exact":
            raise ValueError(f"function identity requires one exact caller: {relative}")
        record = records[0]
        selector = f"{scope['target']}@0x{record.address:08X}"
        resolved = resolve_function(root, selector)
        if (
            resolved.source != source
            or resolved.compiled_symbol != record.spelling
            or selector in selectors
        ):
            raise ValueError(f"function caller ownership is ambiguous: {relative}")
        selectors.add(selector)
        checks.append(
            {
                "selector": selector,
                "address": record.address,
                "size": None,
                "name": record.spelling,
                "source": relative,
            }
        )
    return checks
