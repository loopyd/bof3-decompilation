"""Immutable naming report generations and checkpoint integrity."""

from __future__ import annotations

import hashlib
import json
import os
import re
from pathlib import Path

from harness.common.inputs import file_state, load, relative
from harness.domain.ids import normalize_target_id
from harness.io import unique_object

CHECKPOINT_SCHEMA = "bof3.naming-report-checkpoint/v1"
PLAN_SCHEMA = "bof3.naming-report-finalization/v1"
_SHA256 = re.compile(r"[0-9a-f]{64}").fullmatch


def encode(value: object) -> bytes:
    return (json.dumps(value, indent=2, sort_keys=True) + "\n").encode()


def compute_sha256(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def require_pin(value: str) -> None:
    if not isinstance(value, str) or _SHA256(value) is None:
        raise ValueError("finalization requires lowercase SHA-256 pins")


def require_keys(value: object, keys: str) -> None:
    if not isinstance(value, dict) or set(value) != set(keys.split()):
        raise ValueError("noncanonical naming checkpoint record")


def read_state(path: Path) -> dict:
    state = file_state(path)
    if state is None or path.stat().st_nlink != 1:
        raise ValueError(f"missing or multiply linked checkpoint input: {path}")
    return state


def resolve_contained_path(root: Path, value: str) -> Path:
    path = root / relative(value)
    for ancestor in (path, *path.parents):
        if ancestor.is_symlink():
            raise ValueError("checkpoint path contains a symlink")
    return path


def resolve_transition_paths(root: Path, request: dict) -> tuple[Path, Path]:
    identity = compute_sha256(encode(request))
    predecessor = resolve_contained_path(root, request["report"])
    target = normalize_target_id(request["target"]).value
    stem = target.replace("/", "__")
    return (
        predecessor.parent / f"{stem}.generation-{identity}.json",
        predecessor.parent / f"{stem}.checkpoint-{identity}.json",
    )


def select_target_entry(summary: dict, target: str) -> dict:
    if (
        not isinstance(summary, dict)
        or summary.get("schema") != "bof3.naming-audit-account/v1"
        or not isinstance(summary.get("targets"), list)
    ):
        raise ValueError("canonical naming campaign summary is malformed")
    entries = [
        entry
        for entry in summary["targets"]
        if isinstance(entry, dict) and entry.get("target") == target
    ]
    if len(entries) != 1:
        raise ValueError("canonical naming campaign target is missing or ambiguous")
    return entries[0]


def resolve_recorded_path(root: Path, value: str) -> Path:
    if not isinstance(value, str):
        raise ValueError("invalid campaign report path")
    candidate = Path(value)
    if candidate.is_absolute():
        try:
            value = candidate.relative_to(root).as_posix()
        except ValueError as error:
            raise ValueError("campaign report escapes repository") from error
    return resolve_contained_path(root, value)


def read_snapshot_states(snapshot: Path) -> dict:
    retained = load(snapshot)
    require_keys(retained, "files frozen")
    result = {}
    for item in retained["files"]:
        if type(item.get("exists")) is not bool:
            raise ValueError("snapshot existence must be boolean")
        require_keys(
            item, "path exists sha256 mode" if item["exists"] else "path exists"
        )
        backup = resolve_contained_path(snapshot.parent, item["path"])
        name = str(backup)
        if name in result:
            raise ValueError("duplicate retained snapshot file")
        expected = (
            {"sha256": item["sha256"], "mode": item["mode"]} if item["exists"] else None
        )
        if expected is not None:
            require_pin(expected["sha256"])
            if type(expected["mode"]) is not int:
                raise ValueError("snapshot mode must be an integer")
        if file_state(backup) != expected:
            raise ValueError(f"retained snapshot differs from declared PRE: {backup}")
        result[name] = expected
    return result


def collect_evidence_states(
    root: Path, report_path: Path, bundle_path: Path, report: dict
) -> dict:
    states = {}

    def capture(value: str, *, expected_state=None, expected_sha256=None) -> Path:
        candidate = Path(value)
        path = (
            candidate
            if candidate.is_absolute()
            else resolve_contained_path(root, value)
        )
        state = file_state(path)
        if state is None:
            raise ValueError(f"missing retained naming evidence: {path}")
        if expected_state is not None and state != expected_state:
            raise ValueError(f"retained evidence differs from embedded state: {path}")
        if expected_sha256 is not None:
            require_pin(expected_sha256)
            if state["sha256"] != expected_sha256:
                raise ValueError(
                    f"retained evidence differs from embedded hash: {path}"
                )
        states[str(path)] = state
        return path

    def collect(value: object) -> None:
        if isinstance(value, list):
            for item in value:
                collect(item)
        elif isinstance(value, dict):
            for key, item in value.items():
                if key in {"execution", "attestation"} and isinstance(item, str):
                    expected = value.get(key + "_state")
                    require_keys(expected, "sha256 mode")
                    capture(item, expected_state=expected)
                elif key in {"receipt", "evidence"} and isinstance(item, str):
                    pin = value.get("sha256" if key == "receipt" else "evidence_sha256")
                    require_pin(pin)
                    capture(item, expected_sha256=pin)
                elif key in {"review_artifact", "snapshot"}:
                    require_keys(item, "path sha256")
                    capture(item["path"], expected_sha256=item["sha256"])
                elif isinstance(item, (dict, list)):
                    collect(item)

    capture(str(report_path))
    capture(str(bundle_path))
    if load(report_path) != report:
        raise ValueError("report changed during evidence inventory")
    bundle = load(bundle_path)
    collect(report)
    collect(bundle)
    parent = load(Path(bundle["review"]["attestation"]))
    collect(parent)
    states.update(read_snapshot_states(Path(parent["snapshot"]["path"])))
    for name, expected in states.items():
        if file_state(Path(name)) != expected:
            raise ValueError("retained evidence changed during inventory")
    return states


def update_summary(summary: dict, record: dict, reference: dict) -> dict:
    updated = json.loads(json.dumps(summary))
    entry = select_target_entry(updated, record["request"]["target"])
    rows = entry.get("rows")
    total = updated.get("row_count")
    if type(rows) is not int or rows < 1 or type(total) is not int or total < 1:
        raise ValueError("checkpoint requires positive selected and global row counts")
    if rows != record["validation"]["rows"] + 1:
        raise ValueError("selected summary row count differs from predecessor")
    entry["rows"] = rows - 1
    entry["complete"] = record["validation"]["complete"]
    entry["report"] = record["successor"]["path"]
    entry["history"] = [*entry.get("history", []), reference]
    updated["row_count"] = total - 1
    return updated


def validate_record(root: Path, path: Path, expected_sha256: str) -> dict:
    require_pin(expected_sha256)
    before = read_state(path)
    if before["sha256"] != expected_sha256:
        raise ValueError("naming checkpoint digest changed")
    record = load(path)
    require_keys(
        record,
        "schema request predecessor bundle summary successor verification validation artifacts",
    )
    request = record["request"]
    require_keys(
        request,
        "schema root target transaction report report_sha256 bundle bundle_sha256 summary_sha256 evidence_root",
    )
    if (
        record["schema"] != CHECKPOINT_SCHEMA
        or request["schema"] != PLAN_SCHEMA
        or request["root"] != str(root)
        or normalize_target_id(request["target"]).value != request["target"]
    ):
        raise ValueError("checkpoint root, target or schema mismatch")
    for field in ("report_sha256", "bundle_sha256", "summary_sha256"):
        require_pin(request[field])
    successor, checkpoint = resolve_transition_paths(root, request)
    if checkpoint != path or successor.parent != path.parent:
        raise ValueError("checkpoint is not its deterministic owner path")
    for name in ("predecessor", "bundle", "successor"):
        require_keys(record[name], "path state")
        require_keys(record[name]["state"], "sha256 mode")
        require_pin(record[name]["state"]["sha256"])
        if type(record[name]["state"]["mode"]) is not int:
            raise ValueError("invalid checkpoint file mode")
    if (
        record["predecessor"]["path"] != request["report"]
        or record["predecessor"]["state"]["sha256"] != request["report_sha256"]
        or record["bundle"]["path"] != request["bundle"]
        or record["bundle"]["state"]["sha256"] != request["bundle_sha256"]
        or record["successor"]["path"] != successor.relative_to(root).as_posix()
        or record["successor"]["state"]["mode"]
        != record["predecessor"]["state"]["mode"]
    ):
        raise ValueError("checkpoint input or successor binding mismatch")
    predecessor = resolve_contained_path(root, request["report"])
    if read_state(predecessor) != record["predecessor"]["state"]:
        raise ValueError("retired naming report changed")
    report = load(predecessor)
    require_keys(report, "schema target complete rows")
    selected = [
        row
        for row in report["rows"]
        if f"{row.get('kind')}:{row.get('name')}" == request["transaction"]
    ]
    if (
        report["schema"] != "bof3.naming-audit/v3"
        or report["target"] != request["target"]
        or len(selected) != 1
        or selected[0].get("rung_status") != "proposed"
        or sum(row.get("rung_status") == "proposed" for row in report["rows"]) != 1
    ):
        raise ValueError("checkpoint requires exactly one applied proposal")
    expected_report = dict(report)
    expected_report["rows"] = [row for row in report["rows"] if row is not selected[0]]
    expected_report["complete"] = not any(
        row.get("rung_status") == "blocked" for row in expected_report["rows"]
    )
    if (
        compute_sha256(encode(expected_report))
        != record["successor"]["state"]["sha256"]
    ):
        raise ValueError("checkpoint changed surviving rows")
    if (
        record["verification"]
        != {
            "schema": "bof3.naming-audit/v3",
            "target": request["target"],
            "transaction": request["transaction"],
            "applied": True,
            "rows": 1,
        }
        or record["verification"].get("applied") is not True
    ):
        raise ValueError("checkpoint lacks selected public verification")
    if record["validation"] != {
        "schema": "bof3.naming-audit/v3",
        "target": request["target"],
        "rows": len(expected_report["rows"]),
        "complete": expected_report["complete"],
    }:
        raise ValueError("checkpoint successor validation mismatch")
    require_keys(record["summary"], "state text")
    if (
        compute_sha256(record["summary"]["text"].encode()) != request["summary_sha256"]
        or record["summary"]["state"]["sha256"] != request["summary_sha256"]
    ):
        raise ValueError("checkpoint summary pin mismatch")
    summary = json.loads(record["summary"]["text"], object_pairs_hook=unique_object)
    entry = select_target_entry(summary, request["target"])
    if resolve_recorded_path(root, entry["report"]) != predecessor:
        raise ValueError("checkpoint predecessor is not the recorded active report")
    artifacts = record["artifacts"]
    if not isinstance(artifacts, dict) or not artifacts:
        raise ValueError("checkpoint evidence inventory is missing")
    if artifacts.get(str(predecessor)) != record["predecessor"]["state"] or (
        artifacts.get(request["bundle"]) != record["bundle"]["state"]
    ):
        raise ValueError("checkpoint evidence omits predecessor or bundle")
    for name, state in artifacts.items():
        artifact = Path(name)
        if not artifact.is_absolute() or str(artifact.resolve()) != name:
            raise ValueError("noncanonical retained evidence path")
        if file_state(artifact) != state:
            raise ValueError(f"retained naming evidence changed: {name}")
    if (
        collect_evidence_states(root, predecessor, Path(request["bundle"]), report)
        != artifacts
    ):
        raise ValueError(
            "checkpoint evidence inventory differs from immutable references"
        )
    if read_state(path) != before:
        raise ValueError("checkpoint changed during validation")
    return record


def _validate_active(root: Path, path: Path, record: dict) -> None:
    before = read_state(path)
    if before == record["successor"]["state"]:
        return
    if before["mode"] != record["successor"]["state"]["mode"]:
        raise ValueError("active naming generation mode changed")
    predecessor = load(resolve_contained_path(root, record["predecessor"]["path"]))
    baseline = [
        row
        for row in predecessor["rows"]
        if f"{row.get('kind')}:{row.get('name')}" != record["request"]["transaction"]
    ]
    active = load(path)
    require_keys(active, "schema target complete rows")
    if not isinstance(active["rows"], list) or len(active["rows"]) != len(baseline):
        raise ValueError("active naming generation inventory changed")
    for original, current in zip(baseline, active["rows"]):
        if current == original:
            continue
        if (
            not isinstance(current, dict)
            or original.get("initializer_state") != "bof3.naming-audit-initializer/v1"
            or original.get("rung_status") != "blocked"
            or (original.get("kind"), original.get("name"))
            != (current.get("kind"), current.get("name"))
        ):
            raise ValueError(
                "active generation changed retained inventory or conclusions"
            )
        selector = f"{current['kind']}:{current['name']}"
        if current.get("rung_status") == "proposed":
            from harness.naming.proposal import (
                require_provenance,
                validate_authored_digests,
            )

            provenance = require_provenance(current, selector)
            validate_authored_digests(active, current, provenance)
            if provenance["report"] != path.relative_to(root).as_posix() or any(
                current.get(key) != provenance[key] for key in ("pre_apply", "manifest")
            ):
                raise ValueError("active generation proposal binding differs")
        elif current.get("rung_status") == "exhausted":
            from harness.naming.provenance import require_canonical_provenance

            provenance = require_canonical_provenance(current, selector)
            evidence = Path(provenance["evidence"])
            evidence = evidence if evidence.is_absolute() else root / evidence
            if read_state(evidence)["sha256"] != provenance["evidence_sha256"]:
                raise ValueError("active generation exhaustion evidence changed")
        else:
            raise ValueError("active generation contains an unauthenticated row edit")
    if (
        active["schema"] != predecessor["schema"]
        or active["target"] != predecessor["target"]
        or active["complete"]
        is not (not any(row.get("rung_status") == "blocked" for row in active["rows"]))
        or read_state(path) != before
    ):
        raise ValueError("active generation identity, completeness or state changed")


def validate_history(root: Path, target: str, directory: Path, history: list) -> Path:
    if not isinstance(history, list) or not history:
        raise ValueError("generation requires a nonempty checkpoint chain")
    expected = directory / f"{target.replace('/', '__')}.json"
    seen = set()
    for number, reference in enumerate(history):
        require_keys(reference, "path sha256")
        path = resolve_contained_path(root, reference["path"])
        if path.parent != directory or path in seen:
            raise ValueError("checkpoint chain escapes directory or repeats")
        seen.add(path)
        record = validate_record(root, path, reference["sha256"])
        if record["request"]["target"] != target or (
            resolve_contained_path(root, record["predecessor"]["path"]) != expected
        ):
            raise ValueError("checkpoint predecessor chain mismatch")
        prior = json.loads(record["summary"]["text"], object_pairs_hook=unique_object)
        if select_target_entry(prior, target).get("history", []) != history[:number]:
            raise ValueError("checkpoint history prefix mismatch")
        update_summary(prior, record, reference)
        expected = resolve_contained_path(root, record["successor"]["path"])
    _validate_active(root, expected, record)
    return expected


def require_replaceable_set(directory: Path) -> None:
    if any(directory.glob("*.checkpoint-*.json")) or any(
        directory.glob("*.generation-*.json")
    ):
        raise ValueError(
            "report set contains retained naming history; replacement refused"
        )
    summary = directory / "summary.json"
    if summary.is_file():
        value = load(summary)
        if isinstance(value, dict) and any(
            isinstance(entry, dict) and entry.get("history")
            for entry in value.get("targets", [])
        ):
            raise ValueError("report set accounts for retained naming history")


def require_mutable_report(path: Path) -> None:
    directory = path.parent
    records = list(directory.glob("*.checkpoint-*.json"))
    generations = list(directory.glob("*.generation-*.json"))
    if not records and not generations:
        return
    if path.name == "summary.json" or ".checkpoint-" in path.name:
        raise ValueError("naming history may only advance through finalization")
    for checkpoint in records:
        record = load(checkpoint)
        if record.get("schema") != CHECKPOINT_SCHEMA:
            raise ValueError("invalid retained naming checkpoint")
        if Path(record["predecessor"]["path"]).name == path.name:
            raise ValueError("retired or pending naming report is immutable")
    if ".generation-" in path.name:
        from harness.naming.campaign import resolve_campaign_report

        if directory.parts[-3:] != ("out", "reviews", "plan-audit-naming"):
            raise ValueError("generation is outside canonical campaign")
        root = directory.parents[2]
        target = load(path).get("target")
        if resolve_campaign_report(root, target) != path:
            raise ValueError("unpublished naming generation is immutable")


def write_exclusive(path: Path, content: bytes, mode: int) -> None:
    if path.exists() or path.is_symlink():
        if read_state(path) != {"sha256": compute_sha256(content), "mode": mode}:
            raise ValueError(
                f"retained publication differs; refusing overwrite: {path}"
            )
        return
    descriptor = os.open(
        path, os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW, mode
    )
    with os.fdopen(descriptor, "wb") as stream:
        stream.write(content)
        stream.flush()
        os.fchmod(stream.fileno(), mode)
        os.fsync(stream.fileno())
    sync_directory(path.parent)


def sync_directory(path: Path) -> None:
    descriptor = os.open(path, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(descriptor)
    finally:
        os.close(descriptor)
