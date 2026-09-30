"""Selection, canonical query, receipt, and row-commit helpers."""

from __future__ import annotations

import hashlib
import json
from collections.abc import Mapping, Sequence
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from harness.domain.receipts import command_records, write_receipt
from harness.common.deadlines import check_deadline
from harness.naming.evidence import SCHEMA as ANALYZER_VERSION
from harness.naming.journal import commit_entry
from harness.naming.native import OUTPUT_BUDGET, SemanticResult
from harness.naming.plan import row_operation_sequence


@dataclass(frozen=True)
class EvidenceNamespace:
    """Runner-selected artifact directory for one report identity."""

    root: Path
    path: Path
    explicit: bool = False

    def __post_init__(self) -> None:
        root = self.root.resolve()
        path = self.path.resolve()
        if path != self.path or (path.exists() and path.is_symlink()):
            raise ValueError("evidence namespace must be a canonical non-symlink path")
        object.__setattr__(self, "root", root)
        object.__setattr__(self, "path", path)

    @property
    def receipt_root(self) -> Path:
        """Return the root against which receipt paths are resolved."""

        return self.path.parent if self.explicit else self.root

    def resolve_record(self, value: str) -> Path:
        """Resolve one stored path without allowing namespace escape."""

        candidate = Path(value)
        if candidate.is_absolute() != self.explicit:
            raise ValueError("receipt path spelling does not match namespace mode")
        path = (candidate if self.explicit else self.root / candidate).resolve()
        if self.path not in path.parents:
            raise ValueError("evidence artifact escapes the selected namespace")
        return path

    def command_records(self, value: object, field: str) -> list[dict[str, Any]]:
        """Validate command receipts against this namespace."""

        return command_records(
            value,
            field,
            self.root,
            evidence_root=self.path if self.explicit else None,
        )

    def write_receipt(
        self, row: dict[str, Any], name: str, command: dict[str, object]
    ) -> dict[str, Any]:
        """Write one receipt using this namespace's canonical path spelling."""

        check_deadline()
        return write_receipt(
            self.root,
            self.receipt(row, name),
            command,
            evidence_root=self.path if self.explicit else None,
        )

    def artifact(self, row: dict[str, Any], name: str) -> Path:
        """Return one artifact path inside this namespace."""

        path = (self.path / str(row.get("name")) / name).resolve()
        if self.path != path and self.path not in path.parents:
            raise ValueError("evidence artifact escapes the selected namespace")
        return path

    def record_path(self, path: Path) -> str:
        """Return the stable path spelling stored in evidence records."""

        if self.explicit:
            return path.as_posix()
        return path.relative_to(self.root).as_posix()

    def receipt(self, row: dict[str, Any], name: str) -> str:
        """Return one receipt path under the selected namespace."""

        return self.record_path(self.artifact(row, name))


def evidence_namespace(root: Path, report: Path, target: str) -> EvidenceNamespace:
    """Resolve the selected namespace once for all row artifacts."""

    from harness.naming.namespace import artifact_root
    from harness.naming.namespace import selected_evidence_root

    check_deadline()
    return EvidenceNamespace(
        root,
        artifact_root(root, report, target),
        explicit=selected_evidence_root() is not None,
    )


def evidence_namespace_for_digest(
    root: Path, target: str, report_digest: str, evidence_root: Path | None
) -> EvidenceNamespace:
    """Resolve the deterministic namespace for an already collected report."""

    if len(report_digest) != 64 or any(
        character not in "0123456789abcdef" for character in report_digest
    ):
        raise ValueError("report digest must be lowercase SHA-256")
    parent = (
        evidence_root.resolve()
        if evidence_root is not None
        else (root / "out" / "reviews" / "evidence").resolve()
    )
    safe_target = target.replace("/", "__") if evidence_root is not None else target
    path = (parent / f"{safe_target}__{report_digest[:12]}").resolve()
    if parent != path and parent not in path.parents:
        raise ValueError("evidence namespace escapes selected root")
    return EvidenceNamespace(root, path, explicit=evidence_root is not None)


def namespace_identity(
    namespace: EvidenceNamespace, evidence_root: Path | None
) -> dict[str, str]:
    """Return stable provenance for the selected namespace mode."""

    explicit = evidence_root is not None
    return {
        "mode": "explicit" if explicit else "default",
        "root": (
            evidence_root.resolve().as_posix()
            if explicit
            else namespace.path.parent.relative_to(namespace.root).as_posix()
        ),
        "namespace": (
            namespace.path.as_posix()
            if explicit
            else namespace.path.relative_to(namespace.root).as_posix()
        ),
    }


def _bounded_text(text: str) -> str:
    if len(text) <= OUTPUT_BUDGET:
        return text
    marker = " ...[truncated; full evidence in digest-bound file]"
    return text[: OUTPUT_BUDGET - len(marker)] + marker


def _json_lines(payload: dict[str, Any]) -> str:
    return json.dumps(payload, indent=2, sort_keys=True) + "\n"


def _row_key(row: dict[str, Any]) -> str:
    return f"{row.get('kind')}:{row.get('name')}"


def _report_digest(report: Path) -> str:
    return hashlib.sha256(report.read_bytes()).hexdigest()


def _select_rows(
    report_rows: Sequence[dict[str, Any]],
    *,
    rows: Sequence[str] | None,
    all_rows: bool,
    shard: int,
    checkpoint: Mapping[str, dict[str, Any]],
) -> list[dict[str, Any]]:
    """Deterministic, bounded row selection; completed rows never replay."""

    if rows is not None:
        selected: list[dict[str, Any]] = []
        for selector in rows:
            kind, _, name = str(selector).partition(":")
            for row in report_rows:
                if (
                    str(row.get("kind")) == kind
                    and str(row.get("name")) == name
                    and _row_key(row) not in checkpoint
                ):
                    selected.append(row)
                    break
        return selected
    if all_rows:
        return [row for row in report_rows if _row_key(row) not in checkpoint]
    by_key = {
        _row_key(row): row
        for row in report_rows
        if row.get("rung_status") == "blocked" and _row_key(row) not in checkpoint
    }
    return [by_key[key] for key in sorted(by_key)][:shard]


def _item_queries(
    row: dict[str, Any], source: Any
) -> list[tuple[dict[str, Any], str, str, list[dict[str, Any]]]]:
    """Route each open work item to its canonical indexed query.

    Returns ``(item, operation, selector, payload)``; un-routable items
    stay open with a machine-visible note instead of a fabricated payload.
    """

    results: list[tuple[dict[str, Any], str, str, list[dict[str, Any]]]] = []
    for item in row.get("required_work", []):
        if not (isinstance(item, dict) and item.get("status") == "open"):
            continue
        item_id = str(item.get("id"))
        if item_id.startswith("callee:") or item_id.startswith("caller:"):
            function_id = item_id.split(":", 1)[1]
            payload = source.canonical("calls_payload", function_id)
            results.append((item, "calls", function_id, payload))
        elif item_id.startswith("access:"):
            function_id = item_id.split(":", 1)[1]
            target, _, addr_text = function_id.partition("@")
            access_site = int(addr_text, 16)
            payload = source.execute(
                "SELECT source, address, function_id, access_kind, opcode "
                "FROM data_references WHERE target_id = ? AND source = ? "
                "AND function_id IS NOT NULL ORDER BY address",
                (target, access_site),
            )
            results.append((item, "access", function_id, payload))
        elif item_id.startswith("owner:"):
            owner_selector = item_id.split(":", 1)[1]
            owner_target, _, owner_address = owner_selector.partition("@")
            start = int(owner_address, 16)
            payload = source.canonical("owners_payload", f"{owner_target}@{start:08X}")
            results.append((item, "owner", owner_selector, payload))
        else:
            item["note"] = (
                "unsupported generated work profile; left open for the owning validator"
            )
            results.append((item, "open", _row_key(row), []))
    return results


def _commit_row(
    namespace: EvidenceNamespace,
    report: Path,
    target: str,
    row: dict[str, Any],
    operations: list[tuple[dict[str, Any], str, str, list[dict[str, Any]], bool]],
    receipts: list[dict[str, Any]],
    semantic: list[dict[str, Any]],
    derived: dict[str, Any],
) -> dict[str, Any]:
    """Persist the validated collection state, then commit the checkpoint."""

    payload: dict[str, Any] = {
        "schema": "bof3.naming-evidence/v2",
        "target": target,
        "row": _row_key(row),
        "items": [
            {
                "id": item.get("id"),
                "status": "open",
                "operation": op,
                "selector": selector,
                "payload": payload_rows,
                "note": item.get("note"),
                "supplemental": supplemental,
                **({"role": item["role"]} if item.get("role") else {}),
            }
            for item, op, selector, payload_rows, supplemental in operations
        ],
        "semantic": semantic,
        "commands": receipts,
    }
    raw_path = namespace.artifact(row, "payload.json")
    raw_file = namespace.record_path(raw_path)
    check_deadline()
    raw_path.parent.mkdir(parents=True, exist_ok=True)
    for index, result in enumerate(semantic):
        stdout = str(result.pop("raw", result.get("output", "")))
        stderr = str(result.pop("stderr", ""))
        path = raw_path.parent / f"semantic-{index}.stdout.raw"
        error_path = raw_path.parent / f"semantic-{index}.stderr.raw"
        check_deadline()
        path.write_text(stdout, encoding="utf-8")
        check_deadline()
        error_path.write_text(stderr, encoding="utf-8")
        result["raw_file"] = namespace.record_path(path)
        result["raw_sha256"] = hashlib.sha256(stdout.encode()).hexdigest()
        result["raw_size"] = path.stat().st_size
        result["stderr_file"] = namespace.record_path(error_path)
        result["stderr_sha256"] = hashlib.sha256(stderr.encode()).hexdigest()
        result["stderr_size"] = error_path.stat().st_size
    raw_text = _json_lines(payload)
    check_deadline()
    raw_path.write_text(raw_text, encoding="utf-8")
    derived_path = raw_path.parent / "derived.json"
    derived_text = _json_lines(derived)
    check_deadline()
    derived_path.write_text(derived_text, encoding="utf-8")
    entry: dict[str, Any] = {
        "row": _row_key(row),
        "status": "committed",
        "evidence": raw_file,
        "evidence_sha256": hashlib.sha256(raw_text.encode()).hexdigest(),
        "operations": row_operation_sequence(row, target),
        "derived": namespace.record_path(derived_path),
        "derived_sha256": hashlib.sha256(derived_text.encode()).hexdigest(),
        "derived_size": len(derived_text.encode()),
        "analyzer_version": ANALYZER_VERSION,
    }
    commit_entry(namespace, report, target, entry)
    check_deadline()
    failed_path = raw_path.parent / "failed.json"
    if failed_path.exists():
        failed_path.unlink()
    return entry


def terminalize_report(
    root: Path,
    report: Path,
    target: str,
    rows: list[dict[str, Any]],
    checkpoint: Mapping[str, dict[str, Any]],
) -> bool:
    """Leave collected rows blocked until a semantic analyzer authors conclusions.

    Collection receipts prove only that commands ran.  The runner has no typed
    semantic-conclusion format capable of proving v3 exhaustion, so it must not
    mutate initializer or authored rows.
    """
    return False


def write_failed_receipt(
    namespace: EvidenceNamespace,
    target: str,
    row: dict[str, Any],
    error: Exception,
    result: SemanticResult | None = None,
) -> dict[str, Any]:
    """Persist one honest forensic failure without committing the row."""

    output = {"error": str(error)}
    if result is not None:
        output["last_operation"] = {
            "command": result.command,
            "selector": result.selector,
            "exit": result.exit,
            "killed": result.killed,
            "raw": result.raw,
            "stderr": result.stderr,
        }
    return namespace.write_receipt(
        row,
        "failed.json",
        {
            "command": "harness:naming-evidence-run row",
            "status": "failed",
            "target": target,
            "selector": _row_key(row),
            "output": _json_lines(output),
        },
    )


def _receipts_for(
    namespace: EvidenceNamespace,
    target: str,
    row: dict[str, Any],
    operations: list[tuple[dict[str, Any], str, str, list[dict[str, Any]], bool]],
    semantic: list[dict[str, Any]],
    captures: tuple[dict, ...] = (),
) -> list[dict[str, Any]]:
    """Honest receipts for exactly the operations that were executed.

    Indexed receipts record the canonical native operation and the exact
    selector queried; semantic receipts record the executed Rizin/native
    command, its real exit status, and its bounded output.  No receipt
    claims a CLI command that the runner did not execute.
    """

    receipts: list[dict[str, Any]] = []
    for item, op, selector, payload, supplemental in operations:
        if op not in {"calls", "access", "owner", "describe", "xrefs"}:
            continue
        item_id = str(item.get("id")).replace(":", "_").replace("/", "_")
        command: dict[str, object] = {
            "command": (
                f"harness:rev-query {op} {selector} (work item {item.get('id')})"
            ),
            "status": "passed",
            "target": target,
            "selector": selector,
            "output": _bounded_text(
                f"{len(payload)} rows captured for {item.get('id')}"
            ),
        }
        record = namespace.write_receipt(row, f"{item_id}.json", command)
        record["item"] = str(item.get("id"))
        record["operation"] = op
        record["supplemental"] = supplemental
        if item.get("role"):
            record["role"] = item["role"]
        receipts.append(record)
    for index, result in enumerate(semantic):
        command = dict(result)
        command_digest = hashlib.sha256(
            str(result.get("command")).encode()
        ).hexdigest()[:12]
        record = namespace.write_receipt(
            row,
            f"semantic-{index}-{command_digest}.json",
            {
                "command": str(result.get("command")),
                "status": "passed" if result.get("semantic_success") else "failed",
                "target": (
                    str(result["selector"]).split("@", 1)[0]
                    if result.get("instruction_id")
                    else target
                ),
                "selector": str(result.get("selector")),
                "output": (
                    str(result["raw"])
                    if result.get("instruction_id")
                    else _bounded_text(str(result.get("output")))
                ),
            },
        )
        record["operation"] = "semantic"
        capture = next(
            (item for item in captures if item["selector"] == result.get("selector")),
            None,
        )
        if capture is not None and row.get("kind") == "function":
            from harness.naming.evidence import (
                _RUNNER_TOKEN,
                instruction_observations,
            )

            observations = instruction_observations(_RUNNER_TOKEN, target, row, capture)
            path = namespace.resolve_record(record["receipt"])
            receipt = json.loads(path.read_text())
            receipt.update(observations)
            check_deadline()
            text = _json_lines(receipt)
            path.write_text(text, encoding="utf-8")
            record["sha256"] = hashlib.sha256(text.encode()).hexdigest()
        receipts.append(record)
    if not receipts:
        receipts.append(
            namespace.write_receipt(
                row,
                "collection.json",
                {
                    "command": "harness:naming-evidence-run bounded collection",
                    "status": "passed",
                    "target": target,
                    "selector": _row_key(row),
                    "output": "No generated indexed or semantic operations remained for this row.",
                },
            )
        )
    namespace.command_records(receipts, f"{_row_key(row)}.commands")
    return receipts
