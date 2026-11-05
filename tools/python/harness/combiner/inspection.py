"""Read current source membership and profiles or check complete native consumers."""

from __future__ import annotations

from pathlib import Path
import sqlite3

from harness.build.compiler import (
    build_compiler_arguments,
    resolve_compiler_settings,
    parse_object_flags,
    parse_object_compilers,
    validate_object_configuration,
)
from harness.common.files import read_file
from harness.decomp import metadata
from harness.decomp.consumers import identifier_consumers
from harness.domain.ids import normalize_target_id, parse_function_id
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.registry import resolve_function
from .opportunities import relationship_rows


def _target(root: Path, target: str) -> str:
    target = normalize_target_id(target).value
    if target not in load_target_manifests(root):
        raise ValueError(f"unknown target: {target}")
    return target


def _profiles(root: Path, records: list[dict]) -> tuple[dict, list[dict]]:
    profiles, findings = {}, []
    try:
        configuration = read_file(
            root, "config/compiler/object-flags.cmake", missing_ok=True
        )
        text = configuration.decode("utf-8") if configuration is not None else ""
        validate_object_configuration(text)
        flags, compilers = parse_object_flags(text), parse_object_compilers(text)
    except (OSError, UnicodeError, ValueError) as error:
        return {}, [
            {"status": "unavailable", "code": "configuration", "message": str(error)}
        ]
    for source in sorted({row["source"] for row in records}):
        try:
            settings = resolve_compiler_settings(
                source, (root / source).read_text(encoding="utf-8"), flags, compilers
            )
            profiles[source] = {
                "compiler_id": settings.compiler_id or "gcc-2.7.2-psx",
                "arguments": build_compiler_arguments(root, settings.flags),
            }
        except (OSError, UnicodeError, ValueError) as error:
            findings.append(
                {
                    "status": "failed",
                    "code": "profile",
                    "source": source,
                    "message": str(error),
                }
            )
    return profiles, findings


def list_report(root: Path, target: str, *, limit: int | None = None) -> dict:
    root = root.resolve()
    target = _target(root, target)
    if limit is not None and limit < 1:
        raise ValueError("listing requires a positive limit")
    result = metadata.report(root, target, check=False)
    result["operation"] = "combiner"
    profiles, findings = _profiles(root, result["records"])
    facts = {}
    try:
        facts = relationship_rows(root, target)
        manifest = load_target_manifests(root)[target]
        layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
        boundaries = {row.virtual_start: row for row in layout.boundaries}
        for row in result["records"]:
            row["kind"] = "source_member"
            row["profile"] = profiles.get(row["source"])
            address = int(row["address"], 16) if row["address"] else None
            row["relationships"] = facts.get(address)
            boundary = boundaries.get(address)
            row["reviewed_size"] = boundary.virtual_size if boundary else None
            if address is not None and address not in facts:
                findings.append(
                    {
                        "status": "unavailable",
                        "code": "index_member",
                        "source": row["source"],
                        "message": "authored function is absent from the current index",
                    }
                )
    except (OSError, UnicodeError, RuntimeError, ValueError, sqlite3.Error) as error:
        findings.append(
            {"status": "unavailable", "code": "relationships", "message": str(error)}
        )
    result["records"] = (
        result["records"] if limit is None else result["records"][:limit]
    )
    result["truncated"] = limit is not None and result["total"] > limit
    result["findings"].extend(findings)
    result["incomplete"] = any(
        row["status"] == "unavailable" for row in result["findings"]
    )
    result["passed"] = not result["findings"]
    return result


def check_report(root: Path, target: str, selectors: tuple[str, ...] = ()) -> dict:
    root = root.resolve()
    target = _target(root, target)
    selected = {parse_function_id(value) for value in selectors}
    if any(value.target.value != target for value in selected):
        raise ValueError("every selector must belong to the selected target")
    scope = metadata.report(root, target, check=False, selectors=selectors)
    consumers = set(selectors)
    findings = []
    if selected:
        try:
            names = set()
            manifests = load_target_manifests(root)
            for row in scope["records"]:
                if row["selector"]:
                    resolved = resolve_function(
                        root, row["selector"], manifests=manifests
                    )
                    if resolved.compiled_symbol is None:
                        raise ValueError(
                            f"unresolved compiled symbol: {row['selector']}"
                        )
                    names.add(resolved.compiled_symbol)
            discovered, coverage = identifier_consumers(root, target, names)
            consumers.update(discovered)
            findings.extend(coverage)
        except (OSError, UnicodeError, RuntimeError, ValueError) as error:
            findings.append(
                {"status": "unavailable", "code": "consumers", "message": str(error)}
            )
    # Native evidence is requested per selector: without one there is no bounded
    # member set to verify, and the alternative - comparing every record in the
    # target, one build each - cost minutes for a purely mechanical answer.
    result = (
        metadata.report(root, target, check=True, selectors=tuple(sorted(consumers)))
        if selected
        else {
            "operation": "metadata",
            "action": "check",
            "scope": {"target": target, "selectors": [], "whole_target": True},
            "consumers": [],
            "total": 0,
            "truncated": False,
            "records": [],
            "findings": [],
            "incomplete": False,
            "passed": True,
            "live": "not measured here; pass the members as selectors or run bin/harness lift gate",
        }
    )
    result["operation"] = "combiner"
    profiles, profile_findings = _profiles(root, result["records"])
    result["profiles"] = profiles
    result["findings"].extend([*findings, *profile_findings])
    result["scope"]["selectors"] = sorted(str(value) for value in selected)
    result["scope"]["whole_target"] = not bool(selected)
    result["incomplete"] = any(
        row["status"] == "unavailable" for row in result["findings"]
    )
    result["passed"] = not result["findings"]
    return result
