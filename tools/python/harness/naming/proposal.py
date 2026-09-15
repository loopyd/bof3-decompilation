"""Closed provenance for prepared naming transactions."""

from __future__ import annotations

import hashlib
import json
import os
import re
import stat
import tempfile
from pathlib import Path
from typing import TYPE_CHECKING, Any

from harness.common.deadlines import check_deadline

if TYPE_CHECKING:
    from harness.naming.context import TargetContext

KIND = "proposed-function-transaction/v1"
DATA_KIND = "proposed-data-transaction/v1"
PROVENANCE_FIELD = "conclusion_provenance"
_TOOL_FIELDS = {PROVENANCE_FIELD, "pre_apply", "manifest", "post_apply_receipts"}
_SHA256 = re.compile(r"[0-9a-f]{64}").fullmatch
_KEYS = {"kind", "report", "report_sha256", "row_sha256", "pre_apply", "manifest"}


def _canonical_json(value: object) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":")).encode("utf-8")


def _digest(value: object) -> str:
    return hashlib.sha256(_canonical_json(value)).hexdigest()


def _authored_row(row: dict[str, Any]) -> dict[str, Any]:
    return {key: value for key, value in row.items() if key not in _TOOL_FIELDS}


def _authored_report(report: dict[str, Any]) -> dict[str, Any]:
    authored = dict(report)
    authored["rows"] = [
        _authored_row(row) if isinstance(row, dict) else row
        for row in report.get("rows", [])
    ]
    return authored


def canonical_report_path(root: Path, report_path: Path) -> Path:
    """Return a contained report path after validating its caller spelling."""
    lexical_root = Path(os.path.abspath(root))
    lexical_path = Path(
        os.path.abspath(
            report_path if report_path.is_absolute() else root / report_path
        )
    )
    try:
        relative = lexical_path.relative_to(lexical_root)
    except ValueError as error:
        raise ValueError("proposal report path escapes repository root") from error
    if not relative.parts:
        raise ValueError("proposal report path must not be the repository root")

    resolved_root = lexical_root.resolve()
    current = resolved_root
    for component in relative.parts:
        current = (current / component).resolve()
        try:
            current.relative_to(resolved_root)
        except ValueError as error:
            raise ValueError("proposal report path has a symlink escape") from error
    return current


def _relative_report(root: Path, report_path: Path) -> str:
    return (
        canonical_report_path(root, report_path).relative_to(root.resolve()).as_posix()
    )


def replace_initializer(
    report: dict[str, Any],
    original: bytes,
    target: str,
    transaction: str,
    candidate: dict[str, Any],
    expected_sha256: str,
) -> None:
    """Replace one frozen initializer in memory, never importing tool records."""
    from .conclusion import INITIALIZER_STATE

    if (
        not isinstance(expected_sha256, str)
        or _SHA256(expected_sha256) is None
        or hashlib.sha256(original).hexdigest() != expected_sha256
    ):
        raise ValueError("report is stale or expected SHA-256 mismatches")
    if report.get("schema") != "bof3.naming-audit/v3" or report.get("target") != target:
        raise ValueError("candidate requires the selected v3 target report")
    if (
        not isinstance(candidate, dict)
        or candidate.get("kind") not in {"function", "data"}
        or candidate.get("rung_status") != "proposed"
        or transaction != f"{candidate.get('kind')}:{candidate.get('name')}"
        or (_TOOL_FIELDS | {"initializer_state"}) & candidate.keys()
    ):
        raise ValueError(
            "candidate must be an unprepared proposed FUNCTION or DATA row"
        )
    rows = report.get("rows")
    if not isinstance(rows, list):
        raise ValueError("report rows must be a list")
    matches = [
        i
        for i, row in enumerate(rows)
        if isinstance(row, dict)
        and f"{row.get('kind')}:{row.get('name')}" == transaction
    ]
    if len(matches) != 1:
        raise ValueError("candidate requires exactly one matching report row")
    current = rows[matches[0]]
    if (
        current.get("initializer_state") != INITIALIZER_STATE
        or current.get("rung_status") != "blocked"
    ):
        raise ValueError("candidate replacement requires a blocked initializer")
    rows[matches[0]] = json.loads(json.dumps(candidate))
    report["complete"] = not any(
        isinstance(row, dict) and row.get("rung_status") == "blocked" for row in rows
    )


def build_provenance(
    root: Path,
    report_path: Path,
    report: dict[str, Any],
    row: dict[str, Any],
    binding: dict[str, Any],
    manifest: dict[str, Any],
) -> dict[str, Any]:
    """Build the closed proposal receipt from production-derived records."""
    return {
        "kind": DATA_KIND if row["kind"] == "data" else KIND,
        "report": _relative_report(root, report_path),
        "report_sha256": _digest(_authored_report(report)),
        "row_sha256": _digest(_authored_row(row)),
        "pre_apply": binding,
        "manifest": manifest,
    }


def require_provenance(row: dict[str, Any], selector: str) -> dict[str, Any]:
    provenance = row.get(PROVENANCE_FIELD)
    if not isinstance(provenance, dict) or set(provenance) != _KEYS:
        raise ValueError(f"{selector} proposal provenance has a non-canonical shape")
    if provenance.get("kind") != (DATA_KIND if row.get("kind") == "data" else KIND):
        raise ValueError(f"{selector} proposal provenance kind is unsupported")
    for key in ("report_sha256", "row_sha256"):
        value = provenance.get(key)
        if not isinstance(value, str) or _SHA256(value) is None:
            raise ValueError(
                f"{selector} proposal provenance requires lowercase SHA-256 ({key})"
            )
    if not isinstance(provenance.get("report"), str):
        raise ValueError(f"{selector} proposal provenance report is invalid")
    binding = provenance.get("pre_apply")
    manifest = provenance.get("manifest")
    if not isinstance(binding, dict) or set(binding) != {"version", "digest", "facts"}:
        raise ValueError(f"{selector} proposal provenance pre_apply is non-canonical")
    digest = binding.get("digest")
    if (
        binding.get("version") != 1
        or not isinstance(digest, str)
        or not re.fullmatch(r"v1:[0-9a-f]{64}", digest)
    ):
        raise ValueError(f"{selector} proposal provenance pre_apply is invalid")
    if not isinstance(binding.get("facts"), dict) or not isinstance(manifest, dict):
        raise ValueError(f"{selector} proposal provenance records are invalid")
    return provenance


def validate_authored_digests(
    report: dict[str, Any], row: dict[str, Any], provenance: dict[str, Any]
) -> None:
    """Verify the prepared authored projection, excluding tool-added receipts."""
    if provenance["row_sha256"] != _digest(_authored_row(row)):
        raise ValueError("proposal row digest drifted")
    if provenance["report_sha256"] != _digest(_authored_report(report)):
        raise ValueError("proposal report digest drifted")


def validate_proposal(
    root: Path,
    report_path: Path,
    report: dict[str, Any],
    row: dict[str, Any],
    ctx: TargetContext,
) -> None:
    """Recompute one prepared proposal from current repository state."""
    from harness.naming.context import naming_manifest, pre_apply

    selector = f"{row.get('kind')}:{row.get('name')}"
    if (
        row.get("kind") not in {"function", "data"}
        or row.get("rung_status") != "proposed"
    ):
        raise ValueError("proposal provenance requires a proposed FUNCTION or DATA row")
    provenance = require_provenance(row, selector)
    relative = _relative_report(root, report_path)
    if provenance["report"] != relative:
        raise ValueError("proposal provenance report path mismatch")
    validate_authored_digests(report, row, provenance)
    name = row.get("name")
    if not isinstance(name, str):
        raise ValueError("proposal row name is invalid")
    binding = pre_apply(
        ctx, row["kind"], name, row, data_proposal=row["kind"] == "data"
    )
    manifest = naming_manifest(ctx, row["kind"], name, row, binding)
    if provenance["pre_apply"] != binding or row.get("pre_apply") != binding:
        raise ValueError("proposal pre_apply drifted")
    if provenance["manifest"] != manifest or row.get("manifest") != manifest:
        raise ValueError("proposal manifest drifted")
    identity = row.get("identity")
    inventory = manifest.get("inventory")
    if not isinstance(identity, dict) or not isinstance(inventory, dict):
        raise ValueError("proposal identity is invalid")
    if (
        identity.get("selector") != inventory.get("selector")
        or manifest.get("transaction") != selector
    ):
        raise ValueError("proposal transaction identity mismatch")


def atomic_write_if_unchanged(
    path: Path, expected: bytes, payload: dict[str, Any]
) -> None:
    """Atomically replace path only when its bytes still match the read snapshot."""
    check_deadline()
    if path.read_bytes() != expected:
        raise ValueError("proposal report changed concurrently")
    rendered = (json.dumps(payload, indent=2, sort_keys=True) + "\n").encode()
    mode = stat.S_IMODE(path.stat().st_mode)
    descriptor, temporary = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(rendered)
            stream.flush()
            os.fsync(stream.fileno())
        os.chmod(temporary, mode)
        if path.read_bytes() != expected:
            raise ValueError("proposal report changed concurrently")
        check_deadline()
        os.replace(temporary, path)
    except BaseException:
        Path(temporary).unlink(missing_ok=True)
        raise
