"""``bin/naming-audit``: one grouped naming-audit command.

``prepare`` is the readiness preflight (with optional safe metadata repair
behind live exact proof); ``validate`` runs the full-report or isolated
pre-apply transaction check and captures the immutable pre-apply digest;
``verify`` proves that captured transaction was applied exactly.  Each
subcommand is one explicit surface: no ``--row`` selector alias and no
boolean ``--post-apply`` mode combinations.
"""

from __future__ import annotations

import json
import shutil
import subprocess
import tempfile
from pathlib import Path
from typing import Any, Mapping

from ..analysis.index import connect as connect_index
from ..analysis.naming import SCHEMA_V2, SCHEMA_V3, TargetContext, inventory_expected
from ..analysis.naming_audit_v3 import validate as validate_v3
from ..analysis.naming_readiness import (
    progress_metadata_findings,
    repair_exact_progress,
    required_work_snapshot,
)
from ..domain import load_target_manifests, normalize_target_id
from ..domain.naming_debt import address_of
from ..analysis.project import status as project_status
from ..domain.claims import manifest_source_paths
from ..domain.tags import parse_progress_tags, parse_source_tag


def _live_exact(root: Path, target: str, address: int) -> tuple[bool, list[str]]:
    selector = f"{target}@0x{address:08X}"
    commands = [
        [str(root / "bin/asm-diff"), selector, "--detail", "normal"],
        [str(root / "bin/byte-match"), selector],
    ]
    results = []
    for command in commands:
        result = subprocess.run(command, cwd=root, text=True, capture_output=True)
        results.append(f"{' '.join(command)}: exit {result.returncode}")
        if result.returncode:
            return False, results
    return True, results


def prepare(root: Path, target: str, *, repair: bool = False) -> dict[str, object]:
    """Read-only readiness preflight; ``repair`` rewrites only proven-exact metadata."""

    target = normalize_target_id(target).value
    manifests = load_target_manifests(root)
    if target not in manifests:
        raise ValueError(f"unknown target: {target}")
    manifest = manifests[target]
    findings = progress_metadata_findings(root, target, manifest)
    repaired = []
    if repair:
        by_address = {}
        try:
            for source in manifest_source_paths(root, manifest):
                if source.suffix == ".c":
                    address = parse_source_tag(source.read_text(encoding="utf-8"))
                    if address is not None:
                        by_address[address] = source
        except ValueError:
            pass
        for finding in findings:
            if finding.get("class") != "safe_metadata_repair":
                continue
            address = int(str(finding["row"]).split("func_", 1)[1], 16)
            exact, commands = _live_exact(root, target, address)
            if not exact:
                finding["class"] = "review_required"
                finding["reason"] = "live exact proof failed; metadata not changed"
                finding["validation"] = commands
                continue
            source = by_address.get(address)
            if source is None:
                continue
            repaired.append(
                {
                    "row": finding["row"],
                    "file": source.relative_to(root).as_posix(),
                    "repair": repair_exact_progress(source),
                    "validation": commands,
                }
            )
        findings = progress_metadata_findings(root, target, manifest)
    blocked = [
        finding for finding in findings if finding.get("class") == "review_required"
    ]
    repairable = [
        finding
        for finding in findings
        if finding.get("class") == "safe_metadata_repair"
    ]
    return {
        "schema": "bof3.naming-preflight/v1",
        "target": target,
        "ready": not findings,
        "findings": findings,
        "repaired": repaired,
        "counts": {"blocked": len(blocked), "repairable": len(repairable)},
    }


def _source_metadata(root: Path, manifest: Any) -> dict[int, tuple[Path, Any]]:
    """Read each claimed lift source once for bulk row validation."""

    metadata: dict[int, tuple[Path, Any]] = {}
    try:
        sources = manifest_source_paths(root, manifest)
    except ValueError:
        return metadata
    for source in sources:
        if source.suffix != ".c":
            continue
        try:
            text = source.read_text(encoding="utf-8")
            address = parse_source_tag(text)
            if address is None:
                continue
            try:
                progress: Any = parse_progress_tags(text)
            except ValueError as error:
                progress = error
            metadata[address] = (source, progress)
        except (OSError, UnicodeError):
            continue
    return metadata


def _context(
    root: Path,
    target: str,
    *,
    bulk_work: bool = False,
    manifests: Mapping[str, Any] | None = None,
    connection: Any = None,
) -> TargetContext:
    target = normalize_target_id(target).value
    loaded = load_target_manifests(root) if manifests is None else manifests
    if target not in loaded:
        raise ValueError(f"unknown target: {target}")
    manifest = loaded[target]
    if not bulk_work:
        return TargetContext(root, target, manifest)
    owned_connection = connection is None
    if owned_connection:
        connection = connect_index(root, manifests=loaded)
    try:
        work = required_work_snapshot(root, target, manifest, connection)
    finally:
        if owned_connection:
            connection.close()
    return TargetContext(
        root,
        target,
        manifest,
        work_snapshot=work,
        source_metadata=_source_metadata(root, manifest),
        payload_end=work.payload_end,
    )


def _blocked_row(
    root: Path, target: str, ctx: TargetContext, kind: str, name: str
) -> dict[str, Any]:
    address = address_of(name)
    selector = f"{target}@0x{address:08X}"
    outside = not ctx.manifest.load_address <= address < ctx.payload_end
    if kind == "function":
        profiles = (
            (
                (
                    "selected_call",
                    "selected caller/callsite instructions and arguments",
                ),
                (
                    "owner_resolution",
                    "runtime owner, range, bytes, boundary, and composition",
                ),
                ("owner_body", "owner body effects, callees, and consumers"),
                (
                    "one_level_beyond",
                    "one independent semantic level beyond the selected call",
                ),
            )
            if outside
            else (
                ("selected_range", "reviewed half-open range and original bytes"),
                (
                    "selected_call",
                    "callsite instructions, arguments, guards, and result use",
                ),
                (
                    "one_level_beyond",
                    "caller/callee/table consumer one semantic level beyond",
                ),
            )
        )
    else:
        profiles = (
            (
                ("selected_access", "selected access instructions and access width"),
                ("owner_resolution", "map/Splat/load-range owner and original bytes"),
                ("owner_data", "initializer, consumers, content class, and layout"),
                ("storage_class", "canonical storage class and extent"),
                ("one_level_beyond", "an independent initializer or consumer"),
            )
            if outside
            else (
                ("selected_range", "reviewed range and original bytes"),
                ("selected_access", "access instructions and access width"),
                ("storage_class", "canonical storage class and extent"),
                ("one_level_beyond", "an independent initializer or consumer"),
            )
        )
    rungs: dict[str, Any] = {}
    for rung, missing in profiles:
        next_command = (
            f"bin/rev-query --json owners {selector}"
            if rung == "owner_resolution"
            else f"bin/rev-query --json {'describe' if rung in {'selected_range', 'storage_class'} else 'xrefs'} {selector}"
        )
        rungs[rung] = {
            "status": "open",
            "next_command": next_command,
            "observations": [
                {
                    "id": f"{name}.{rung}.gap",
                    "text": f"Evidence gap: {missing}; this initializer records no semantic conclusion.",
                }
            ],
            "authority": "target manifest, reviewed Splat, original image, and fresh reverse index",
        }
    required = [{**item, "status": "open"} for item in ctx.required_work(kind, name)]
    partial = False
    if kind == "function":
        partial = ctx.partial(name)
        if partial:
            rungs["partial_baseline"] = {
                "status": "open",
                "next_command": f"bin/asm-diff {selector} --detail normal; bin/byte-match {selector}",
                "observations": [
                    {
                        "id": f"{name}.partial_baseline.gap",
                        "text": "Evidence gap: live partial percentage, sizes, first mismatch, residual, and original-byte verification are not yet recorded.",
                    }
                ],
                "authority": "live asm-diff, byte-match, source progress metadata, and original bytes",
            }
    next_command = (
        f"bin/rev-query --json owners {selector}; bin/rz-project query OWNER -c 'pdf @ 0x{address:08X}'"
        if outside and kind == "function"
        else f"bin/rev-query --json xrefs {selector}; bin/rz-project query {target} -c 'axt @ 0x{address:08X}'"
    )
    return {
        "kind": kind,
        "name": name,
        "rung_status": "blocked",
        "outside_payload": outside,
        "partial_used": partial,
        "rungs": rungs,
        "required_work": required,
        "optional_work": [],
        "interpretation": "No semantic name is accepted until the failed typed rungs and generated required work are closed.",
        "authority": "target manifest, target-local map, reviewed Splat, original image, and fresh reverse index",
        "smallest_repair": next_command,
        "missing_fact": "; ".join(missing for _, missing in profiles),
        "ceiling_next_command": next_command,
    }


def _initialize_with_context(
    root: Path,
    target: str,
    ctx: TargetContext,
    manifests: dict[str, Any],
    *,
    expected: set[tuple[str, str]] | None = None,
) -> dict[str, Any]:
    inventory = (
        inventory_expected(root, target, manifests) if expected is None else expected
    )
    rows = [
        _blocked_row(root, target, ctx, kind, name) for kind, name in sorted(inventory)
    ]
    return {"schema": SCHEMA_V3, "target": target, "complete": not rows, "rows": rows}


def initialize(root: Path, target: str) -> dict[str, Any]:
    """Create a complete v3 inventory whose unreviewed rows are explicit gaps."""

    target = normalize_target_id(target).value
    snapshot = project_status(root, target)
    if not snapshot.get("fresh"):
        raise ValueError(
            f"analysis snapshot is stale for {target}; run bin/index --recover"
        )
    manifests = load_target_manifests(root)
    return _initialize_with_context(
        root, target, _context(root, target, bulk_work=True), manifests
    )


def validate(
    root: Path,
    target: str,
    report: dict[str, Any],
    *,
    transaction: str | None = None,
) -> dict[str, Any]:
    """Pre-apply check: full report, or one isolated transaction selection.

    A ready isolated proposal receives the captured immutable ``pre_apply``
    digest record; ``verify`` is the only post-apply surface.
    """

    if not isinstance(report, dict):
        raise ValueError("report must be an object")
    if report.get("schema") == SCHEMA_V2:
        raise ValueError(
            "bof3.naming-audit/v2 is retired; regenerate as bof3.naming-audit/v3"
        )
    if report.get("schema") != SCHEMA_V3:
        raise ValueError(f"report schema must be {SCHEMA_V3}")
    return validate_v3(
        root,
        normalize_target_id(target).value,
        report,
        _context(root, target),
        transaction=transaction,
    )


def verify(
    root: Path,
    target: str,
    report: dict[str, Any],
    transaction: str,
) -> dict[str, Any]:
    """Post-apply proof: the captured transaction applied exactly.

    Requires the selected row's versioned ``pre_apply`` digest record; the
    recorded facts must still hold against the current repository.
    """

    if not isinstance(report, dict):
        raise ValueError("report must be an object")
    if transaction is None:
        raise ValueError("verify requires --transaction KIND:NAME")
    if report.get("schema") == SCHEMA_V2:
        raise ValueError(
            "bof3.naming-audit/v2 is retired; regenerate as bof3.naming-audit/v3"
        )
    if report.get("schema") != SCHEMA_V3:
        raise ValueError(f"report schema must be {SCHEMA_V3}")
    return validate_v3(
        root,
        normalize_target_id(target).value,
        report,
        _context(root, target),
        transaction=transaction,
        post_apply=True,
    )


def _write_json(path: Path, payload: dict[str, Any]) -> None:
    path.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )


def initialize_all(root: Path, output: Path) -> dict[str, Any]:
    """Build one validated report set from one repository/index snapshot."""

    from .naming_audit_bulk import expected_inventories, publish_reports

    manifests = load_target_manifests(root)
    expected_by_target = expected_inventories(root, manifests)
    output.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(
        tempfile.mkdtemp(prefix=f".{output.name}.", suffix=".tmp", dir=output.parent)
    )
    connection = None
    try:
        connection = connect_index(root, manifests=manifests)
        targets = []
        total_rows = 0
        for target in sorted(manifests):
            ctx = _context(
                root,
                target,
                bulk_work=True,
                manifests=manifests,
                connection=connection,
            )
            expected = expected_by_target[target]
            report = _initialize_with_context(
                root, target, ctx, manifests, expected=expected
            )
            result = validate_v3(root, target, report, ctx, expected=expected)
            seen = [(str(row["kind"]), str(row["name"])) for row in report["rows"]]
            if len(seen) != len(set(seen)) or set(seen) != expected:
                raise ValueError(f"all-target accounting mismatch for {target}")
            name = f"{target.replace('/', '__')}.json"
            _write_json(staging / name, report)
            total_rows += len(seen)
            targets.append(
                {
                    "target": target,
                    "rows": len(seen),
                    "complete": result["complete"],
                    "report": (output / name).as_posix(),
                }
            )
        summary = {
            "schema": "bof3.naming-audit-account/v1",
            "targets": targets,
            "target_count": len(targets),
            "row_count": total_rows,
        }
        _write_json(staging / "summary.json", summary)
        publish_reports(staging, output)
        return summary
    finally:
        if connection is not None:
            connection.close()
        shutil.rmtree(staging, ignore_errors=True)


def main(argv: list[str] | None = None) -> int:
    from .naming_audit_cli import main as cli_main

    return cli_main(argv)


if __name__ == "__main__":
    raise SystemExit(main())
