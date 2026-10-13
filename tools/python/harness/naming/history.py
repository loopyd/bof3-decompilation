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
RECOVERY_CHECKPOINT_SCHEMA = "bof3.naming-inventory-checkpoint/v1"
RECOVERY_PLAN_SCHEMA = "bof3.naming-inventory-recovery/v1"
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
    root: Path, report_path: Path, bundle_path: Path | None, report: dict
) -> dict:
    states = {}

    def capture(value: str, *, expected_state=None, expected_sha256=None) -> Path:
        candidate = Path(value)
        path = (
            candidate
            if candidate.is_absolute()
            else resolve_contained_path(root, value)
        )
        if bundle_path is None:
            from harness.naming.namespace import selected_evidence_root

            selected = selected_evidence_root()
            roots = [root / "out" / "reviews"]
            if selected is not None:
                roots.append(selected)
            if path.resolve() != path or not any(
                path.is_relative_to(base) for base in roots
            ):
                raise ValueError("recovery evidence escapes its approved roots")
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
    if load(report_path) != report:
        raise ValueError("report changed during evidence inventory")
    collect(report)
    if bundle_path is not None:
        capture(str(bundle_path))
        bundle = load(bundle_path)
        collect(bundle)
        parent = load(Path(bundle["review"]["attestation"]))
        collect(parent)
        states.update(read_snapshot_states(Path(parent["snapshot"]["path"])))
    for name, expected in states.items():
        if file_state(Path(name)) != expected:
            raise ValueError("retained evidence changed during inventory")
    return states


def is_unreviewed_initializer(row: dict, replacement: dict) -> bool:
    """Require an open initializer with no authored observations or receipts."""
    if (
        not isinstance(row, dict)
        or not isinstance(replacement, dict)
        or set(row)
        != {
            "authority",
            "ceiling_next_command",
            "initializer_state",
            "interpretation",
            "kind",
            "missing_fact",
            "name",
            "optional_work",
            "outside_payload",
            "partial_used",
            "required_work",
            "rung_status",
            "rungs",
            "smallest_repair",
        }
        or set(row) != set(replacement)
        or row.get("initializer_state") != "bof3.naming-audit-initializer/v1"
        or row.get("rung_status") != "blocked"
        or replacement.get("rung_status") != "blocked"
        or row.get("optional_work") != []
        or replacement.get("optional_work") != []
        or any(
            row.get(key) != replacement.get(key)
            for key in (
                "kind",
                "name",
                "authority",
                "interpretation",
                "initializer_state",
            )
        )
    ):
        return False
    rungs = row.get("rungs")
    expected = replacement.get("rungs")
    if not isinstance(rungs, dict) or not isinstance(expected, dict) or not rungs:
        return False
    for name, rung in rungs.items():
        expected_rung = expected.get(name)
        if (
            expected_rung is None
            and name == "partial_baseline"
            and row.get("partial_used") is True
            and replacement.get("partial_used") is False
        ):
            expected_rung = {
                "status": "open",
                "authority": "live asm-diff, byte-match, source progress metadata, and original bytes",
                "observations": [
                    {
                        "id": f"{row['name']}.partial_baseline.gap",
                        "text": "Evidence Gap: live partial percentage, sizes, first mismatch, residual, and original-byte verification are not yet recorded.",
                    }
                ],
            }
        if (
            not isinstance(rung, dict)
            or not isinstance(expected_rung, dict)
            or set(rung) != {"status", "next_command", "observations", "authority"}
            or rung.get("status") != "open"
            or rung.get("observations") != expected_rung.get("observations")
            or rung.get("authority") != expected_rung.get("authority")
            or expected_rung.get("status") != "open"
            or not isinstance(rung.get("observations"), list)
            or not rung["observations"]
            or any(
                not isinstance(observation, dict)
                or set(observation) != {"id", "text"}
                or observation.get("id") != f"{row['name']}.{name}.gap"
                or not isinstance(observation.get("text"), str)
                or not observation["text"].startswith(
                    ("Evidence gap:", "Evidence Gap:")
                )
                for observation in rung["observations"]
            )
        ):
            return False
    work = row.get("required_work")
    if not isinstance(work, list):
        return False
    return all(
        isinstance(item, dict)
        and set(item) == {"id", "profile", "description", "status"}
        and item.get("status") == "open"
        and all(isinstance(item[key], str) and item[key] for key in item)
        for item in work
    )


def recovery_successor(report: dict, delta: dict) -> dict:
    """Replay an explicit inventory delta without changing retained conclusions."""
    require_keys(report, "schema target complete rows")
    require_keys(delta, "added refreshed retired")
    if any(not isinstance(delta[key], list) for key in delta):
        raise ValueError("recovery delta fields must be arrays")
    if report["schema"] != "bof3.naming-audit/v3" or not isinstance(
        report["rows"], list
    ):
        raise ValueError("recovery requires a v3 row inventory")
    original = {}
    for row in report["rows"]:
        if not isinstance(row, dict):
            raise ValueError("recovery inventory rows must be objects")
        selector = f"{row.get('kind')}:{row.get('name')}"
        if selector in original:
            raise ValueError("recovery cannot resolve duplicate inventory")
        original[selector] = row
    retired = set()
    for item in delta["retired"]:
        require_keys(item, "selector current_binding")
        require_keys(item["current_binding"], "name address")
        selector = item["selector"]
        if selector not in original or selector in retired:
            raise ValueError("invalid historical inventory disposition")
        binding = item["current_binding"]
        from harness.naming.debt import address_of

        if (
            binding["address"] != f"0x{address_of(original[selector]['name']):08X}"
            or not isinstance(binding["name"], str)
            or not binding["name"]
            or binding["name"].startswith(("func_", "D_"))
        ):
            raise ValueError("historical inventory requires a current named binding")
        retired.add(selector)
    refreshed = {}
    for item in delta["refreshed"]:
        require_keys(item, "selector reason row")
        selector = item["selector"]
        if (
            selector not in original
            or selector in retired
            or selector in refreshed
            or not isinstance(item["reason"], str)
            or not item["reason"]
            or not is_unreviewed_initializer(original[selector], item["row"])
        ):
            raise ValueError("recovery may refresh only unreviewed initializers")
        refreshed[selector] = item["row"]
    rows = [
        refreshed.get(selector, row)
        for selector, row in original.items()
        if selector not in retired
    ]
    added = set()
    for row in delta["added"]:
        if not isinstance(row, dict) or not is_unreviewed_initializer(row, row):
            raise ValueError("new inventory must be an open owner initializer")
        selector = f"{row['kind']}:{row['name']}"
        if selector in original or selector in added:
            raise ValueError("duplicate recovery addition")
        added.add(selector)
        rows.append(row)
    return {
        **report,
        "rows": rows,
        "complete": not any(row.get("rung_status") == "blocked" for row in rows),
    }


def update_summary(summary: dict, record: dict, reference: dict) -> dict:
    updated = json.loads(json.dumps(summary))
    entry = select_target_entry(updated, record["request"]["target"])
    rows = entry.get("rows")
    total = updated.get("row_count")
    if type(rows) is not int or rows < 1 or type(total) is not int or total < 1:
        raise ValueError("checkpoint requires positive selected and global row counts")
    if record["schema"] == RECOVERY_CHECKPOINT_SCHEMA:
        if (
            any(
                not isinstance(item, dict)
                or type(item.get("rows")) is not int
                or item["rows"] < 0
                for item in updated["targets"]
            )
            or sum(item["rows"] for item in updated["targets"]) != total
        ):
            raise ValueError("recovery account totals differ from cached rows")
        entry["rows"] = record["validation"]["rows"]
        entry["complete"] = record["validation"]["complete"]
        entry["report"] = record["successor"]["path"]
        entry["history"] = [*entry.get("history", []), reference]
        updated["row_count"] = total + entry["rows"] - rows
        return updated
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
    if record.get("schema") == RECOVERY_CHECKPOINT_SCHEMA:
        return _validate_recovery_record(root, path, record, before)
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
    require_keys(record["summary"]["state"], "sha256 mode")
    if (
        not isinstance(record["summary"]["text"], str)
        or type(record["summary"]["state"]["mode"]) is not int
    ):
        raise ValueError("invalid retained recovery account")
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


def _validate_recovery_record(
    root: Path, path: Path, record: dict, before: dict
) -> dict:
    require_keys(
        record,
        "schema request predecessor summary successor delta validation artifacts",
    )
    request = record["request"]
    require_keys(
        request, "schema root target report report_sha256 summary_sha256 evidence_root"
    )
    if (
        request["schema"] != RECOVERY_PLAN_SCHEMA
        or request["root"] != str(root)
        or normalize_target_id(request["target"]).value != request["target"]
    ):
        raise ValueError("recovery checkpoint root, target or schema mismatch")
    for key in ("report_sha256", "summary_sha256"):
        require_pin(request[key])
    successor, checkpoint = resolve_transition_paths(root, request)
    if checkpoint != path:
        raise ValueError("recovery checkpoint is not its deterministic owner path")
    for key in ("predecessor", "successor"):
        require_keys(record[key], "path state")
        require_keys(record[key]["state"], "sha256 mode")
        require_pin(record[key]["state"]["sha256"])
        if type(record[key]["state"]["mode"]) is not int:
            raise ValueError("invalid recovery file mode")
    predecessor = resolve_contained_path(root, request["report"])
    if (
        record["predecessor"]["path"] != request["report"]
        or record["predecessor"]["state"]["sha256"] != request["report_sha256"]
        or read_state(predecessor) != record["predecessor"]["state"]
        or record["successor"]["path"] != successor.relative_to(root).as_posix()
        or record["successor"]["state"]["mode"]
        != record["predecessor"]["state"]["mode"]
    ):
        raise ValueError("recovery predecessor or successor binding changed")
    report = load(predecessor)
    if report.get("target") != request["target"]:
        raise ValueError("recovery predecessor target changed")
    candidate = recovery_successor(report, record["delta"])
    if compute_sha256(encode(candidate)) != record["successor"]["state"]["sha256"]:
        raise ValueError("recovery changed surviving rows or initializer delta")
    if record["validation"] != {
        "schema": "bof3.naming-audit/v3",
        "target": request["target"],
        "rows": len(candidate["rows"]),
        "complete": candidate["complete"],
    }:
        raise ValueError("recovery validation differs from successor inventory")
    require_keys(record["summary"], "state text")
    if (
        record["summary"]["state"]["sha256"] != request["summary_sha256"]
        or compute_sha256(record["summary"]["text"].encode())
        != request["summary_sha256"]
    ):
        raise ValueError("recovery original account changed")
    summary = json.loads(record["summary"]["text"], object_pairs_hook=unique_object)
    if (
        resolve_recorded_path(
            root, select_target_entry(summary, request["target"])["report"]
        )
        != predecessor
    ):
        raise ValueError("recovery predecessor was not the recorded active report")
    if collect_evidence_states(root, predecessor, None, report) != record["artifacts"]:
        raise ValueError("recovery retained evidence changed")
    if read_state(path) != before:
        raise ValueError("recovery checkpoint changed during validation")
    return record


def _validate_active(root: Path, path: Path, record: dict) -> None:
    before = read_state(path)
    if before == record["successor"]["state"]:
        return
    if before["mode"] != record["successor"]["state"]["mode"]:
        raise ValueError("active naming generation mode changed")
    predecessor = load(resolve_contained_path(root, record["predecessor"]["path"]))
    if record["schema"] == RECOVERY_CHECKPOINT_SCHEMA:
        baseline = recovery_successor(predecessor, record["delta"])["rows"]
    else:
        baseline = [
            row
            for row in predecessor["rows"]
            if f"{row.get('kind')}:{row.get('name')}"
            != record["request"]["transaction"]
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
        if record.get("schema") not in {CHECKPOINT_SCHEMA, RECOVERY_CHECKPOINT_SCHEMA}:
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
