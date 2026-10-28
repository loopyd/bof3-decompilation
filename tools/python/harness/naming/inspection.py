"""Current naming debt, source spellings and mechanical preservation checks."""

from __future__ import annotations

from pathlib import Path
import re

from harness.decomp import metadata
from harness.decomp.consumers import identifier_consumers
from harness.domain.claims import manifest_header_paths, manifest_source_paths
from harness.domain.ids import normalize_target_id, parse_function_id
from harness.domain.manifests import load_target_manifests
from harness.domain.registry import resolve_function
from harness.domain.symbols import load_map, map_path

from .debt import collect_naming_debt, collect_symbol_debt
from .map_checks import symbol_findings
from .source_identifiers import collect_source_identifiers
from harness.common.lexicon import mask_c_text

KINDS = ("debt", "symbols", "identifiers", "references", "files", "all")
_IDENTIFIER = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")


def _target(root: Path, target: str):
    target = normalize_target_id(target).value
    manifests = load_target_manifests(root)
    if target not in manifests:
        raise ValueError(f"unknown target: {target}")
    return target, manifests[target]


def _locations(root: Path, manifest, names: set[str]) -> list[dict]:
    rows = []
    for path in sorted(
        set(
            manifest_source_paths(root, manifest)
            + manifest_header_paths(root, manifest)
        )
    ):
        text = mask_c_text(path.read_text(encoding="utf-8"))
        for match in _IDENTIFIER.finditer(text):
            if match.group() in names:
                rows.append(
                    {
                        "kind": "reference",
                        "target": manifest.id.value,
                        "name": match.group(),
                        "source": path.relative_to(root).as_posix(),
                        "line": text.count("\n", 0, match.start()) + 1,
                    }
                )
    return rows


def list_report(
    root: Path, target: str, *, kind: str = "debt", limit: int | None = None
) -> dict:
    root = root.resolve()
    target, manifest = _target(root, target)
    if kind not in KINDS or limit is not None and limit < 1:
        raise ValueError("listing requires a supported kind and a positive limit")
    rows, findings = [], []
    try:
        path = map_path(root, target)
        symbols = load_map(path)
        functions, data = collect_symbol_debt(root, {target: manifest})
        lines = {
            match.group(): number
            for number, text in enumerate(path.read_text().splitlines(), 1)
            if (match := _IDENTIFIER.search(text))
        }
        if kind in {"debt", "symbols", "all"}:
            for symbol in symbols:
                key = f"{target}:{symbol.name}"
                debt_kind = (
                    "function" if key in functions else "data" if key in data else None
                )
                if kind == "debt" and debt_kind is None:
                    continue
                rows.append(
                    {
                        "kind": "symbol",
                        "target": target,
                        "name": symbol.name,
                        "address": f"0x{symbol.address:08X}",
                        "debt_kind": debt_kind,
                        "debt_is_semantic_proof": False,
                        "source": path.relative_to(root).as_posix(),
                        "line": lines.get(symbol.name),
                        "alias_of": symbol.alias_of or None,
                    }
                )
        if kind in {"identifiers", "all"}:
            rows.extend(collect_source_identifiers(root, target))
        if kind in {"references", "all"}:
            rows.extend(_locations(root, manifest, {row.name for row in symbols}))
        if kind in {"debt", "files", "all"}:
            debt_files = collect_naming_debt(root, {target: manifest})
            for path in manifest_source_paths(root, manifest):
                if not path.is_file():
                    raise FileNotFoundError(f"missing claimed source: {path}")
                relative = path.relative_to(root).as_posix()
                debt_kind = (
                    "raw_function_file"
                    if relative in debt_files.raw_function_files
                    else "invalid_semantic_file"
                    if relative in debt_files.invalid_semantic_files
                    else None
                )
                if kind == "debt" and debt_kind is None:
                    continue
                rows.append(
                    {
                        "kind": "source_file",
                        "debt_kind": debt_kind,
                        "target": target,
                        "source": path.relative_to(root).as_posix(),
                        "line": 1,
                        "name": path.name,
                    }
                )
    except (OSError, UnicodeError, ValueError) as error:
        findings.append({"status": "unavailable", "message": str(error)})
    rows.sort(
        key=lambda row: (row["kind"], row["source"], row.get("line") or 0, row["name"])
    )
    return {
        "operation": "naming",
        "action": "list",
        "scope": {"target": target, "kind": kind},
        "total": len(rows),
        "truncated": limit is not None and len(rows) > limit,
        "records": rows if limit is None else rows[:limit],
        "findings": findings,
        "incomplete": bool(findings),
        "passed": not findings,
    }


def check_report(root: Path, target: str, selectors: tuple[str, ...] = ()) -> dict:
    root = root.resolve()
    target, manifest = _target(root, target)
    selected = {parse_function_id(value) for value in selectors}
    if any(value.target.value != target for value in selected):
        raise ValueError("every selector must belong to the selected target")
    consumers = set(selectors)
    coverage_findings = []
    try:
        names = set()
        if selected:
            for value in selected:
                resolved = resolve_function(root, value)
                if resolved.source is None or resolved.compiled_symbol is None:
                    raise ValueError(
                        f"selector has no reviewed authored function: {value}"
                    )
                names.add(resolved.compiled_symbol)
        else:
            names = {
                symbol.canonical_name for symbol in load_map(map_path(root, target))
            }
        discovered, coverage_findings = identifier_consumers(root, target, names)
        consumers.update(discovered)
    except (OSError, UnicodeError, RuntimeError, ValueError) as error:
        coverage_findings.append(
            {"status": "unavailable", "code": "consumer_inputs", "message": str(error)}
        )
    result = metadata.report(
        root, target, check=True, selectors=tuple(sorted(consumers)) if selected else ()
    )
    result["findings"].extend(coverage_findings)
    result["operation"] = "naming"
    result["scope"]["selectors"] = sorted(str(value) for value in selected)
    result["scope"]["whole_target"] = not bool(selected)
    try:
        errors = symbol_findings(root, target, reviewed=True)
        result["findings"].extend(
            {"status": "failed", "code": "symbols", "message": error}
            for error in errors
        )
    except (OSError, UnicodeError, ValueError) as error:
        result["findings"].append(
            {"status": "unavailable", "code": "symbols", "message": str(error)}
        )
    result["incomplete"] = any(
        row["status"] == "unavailable" for row in result["findings"]
    )
    result["passed"] = not result["findings"]
    return result
