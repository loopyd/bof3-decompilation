"""Evidence-gated, atomic type application transactions and candidate accounting."""

from __future__ import annotations

import hashlib
import json
import subprocess
from pathlib import Path
from typing import Any, Callable

from harness.analysis.index import connect
from harness.common import execution as execution_context
from harness.common import promotion as shared_pre
from harness.common.process import run_command
from harness.common.checks import capture_partial_baselines, required_checks
from harness.common.digests import digest
from harness.common.deadlines import bind_deadline, suspend_work_deadline
from harness.common.files import preflight_existing_replacements, read_file
from harness.common.history import validate_application_history
from harness.common.lease import exclude_writers, verify_writer
from harness.common.submodules import protect_artifacts
from harness.common.runtime import APPLICATION_SCHEMA, application_record, apply_changes
from harness.common.directory import validate_repo_path
from harness.common.runtime import changed_paths
from harness.common.paths import file_state, validate_paths
from harness.common.git import git_index_backup
from harness.common.runtime import rollback, run_checks, write_attestation
from harness.common.safeguards import capture_safeguards, verify_restored_state
from harness.common.process import ProcessCleanupError
from harness.common.workspace import workspace_backup, verify_workspace
from harness.common.workspace import adopted_baseline as _adopted_baseline
from harness.common.workspace import workspace_baseline as _workspace_baseline
from harness.common.workspace import workspace_state as _workspace_state
from harness.domain.ids import normalize_target_id
from harness.domain.claims import collect_manifest_source_addresses
from harness.domain.layout import parse_splat_layout
from harness.domain.manifests import load_target_manifests
from harness.domain.registry import resolve_function
from harness.domain import functions as source_functions
from harness.domain.sources import expected_lift_sources, source_expected_key
from harness.domain.tags import count_function_metadata
from harness.types.proofs import private_proofs, validate_shared_header
from harness.types import links
from harness.types.review import artifact_paths, validate_reviewed_candidate
from harness.types.review import candidate_account as _candidate_account

ACCOUNT_SCHEMA = "bof3.type-candidate-account/v1"
REQUEST_SCHEMA = "bof3.type-transaction-request/v2"
MANIFEST_SCHEMA = "bof3.type-transaction/v2"
CONCERNS = frozenset({"alias", "layout", "field", "prototype", "shared"})
workspace_baseline = _workspace_baseline


def _target(value: object, manifests: dict[str, Any]) -> str:
    if not isinstance(value, str):
        raise ValueError("type transaction target must be canonical")
    target = normalize_target_id(value).value
    if target != value or target not in manifests:
        raise ValueError(f"unknown or non-canonical type transaction target: {value}")
    return target


def _header(root: Path, manifest: Any, concern: str, value: object) -> str:
    try:
        header = validate_repo_path(value)
        validate_paths(root, {header})
        exists = file_state(root, {header})[header] is not None
    except (OSError, ValueError):
        header = ""
        exists = False
    if not exists:
        raise ValueError("header must be an existing repo-relative file")
    if concern == "shared":
        relative = Path(header)
        private_headers = set(manifest.headers)
        if (
            not relative.is_relative_to("include")
            or relative.suffix != ".h"
            or relative.name.endswith("_internal.h")
            or header in private_headers
        ):
            raise ValueError(
                "shared type transaction header is not a sanctioned public shared path"
            )
    elif header not in manifest.headers:
        raise ValueError(f"type transaction header is not claimed by target: {header}")
    return header


def _index_rows(
    connection: Any, ids: object, targets: set[str]
) -> list[dict[str, Any]]:
    if (
        not isinstance(ids, list)
        or not ids
        or any(not isinstance(item, str) for item in ids)
    ):
        raise ValueError("index_candidate_ids must be non-empty strings")
    if len({item.casefold() for item in ids}) != len(ids):
        raise ValueError("index_candidate_ids must be unique")
    rows = []
    for candidate_id in ids:
        row = connection.execute(
            "SELECT id,target_id,address,end,kind,evidence_class,width,signedness,status,"
            "representation_status,semantic_status,evidence,blocker FROM type_candidates WHERE id=?",
            (candidate_id,),
        ).fetchone()
        if row is None:
            raise ValueError(f"unknown type candidate lead: {candidate_id}")
        item = dict(row)
        item["evidence"] = json.loads(item["evidence"])
        if item["target_id"] not in targets:
            raise ValueError(
                f"type candidate belongs to another target: {candidate_id}"
            )
        rows.append(item)
    return sorted(rows, key=lambda item: item["id"])


def _functions(
    root: Path, connection: Any, values: object, targets: set[str]
) -> list[dict[str, str]]:
    if (
        not isinstance(values, list)
        or not values
        or any(not isinstance(item, str) for item in values)
    ):
        raise ValueError("affected_functions must be a non-empty array")
    result = []
    for selector in sorted(set(values)):
        if "@0x" not in selector:
            raise ValueError(f"invalid affected function selector: {selector}")
        target, raw = selector.rsplit("@0x", 1)
        if target not in targets:
            raise ValueError(f"affected function selector has wrong target: {selector}")
        try:
            address = int(raw, 16)
            resolved = resolve_function(root, selector)
        except (OSError, RuntimeError, ValueError) as error:
            raise ValueError(
                f"affected function identity is not canonical: {selector}"
            ) from error
        row = connection.execute(
            "SELECT lift_status FROM functions WHERE target_id=? AND address=?",
            (target, address),
        ).fetchone()
        if (
            row is None
            or row[0] not in {"exact", "partial"}
            or resolved.source is None
            or resolved.compiled_symbol is None
        ):
            raise ValueError(
                f"affected function must be exact/partial and manifest-claimed: {selector}"
            )
        result.append(
            {
                "selector": selector,
                "target": target,
                "address": f"0x{address:08X}",
                "function": resolved.compiled_symbol,
                "status": row[0],
                "source": resolved.source.relative_to(root).as_posix(),
            }
        )
    _validate_function_groups(root, connection, result)
    return result


def _validate_function_groups(
    root: Path, connection: Any, functions: list[dict[str, str]]
) -> None:
    manifests = load_target_manifests(root)
    groups = {}
    for function in functions:
        groups.setdefault((function["target"], function["source"]), []).append(function)
    for (target, name), selected in groups.items():
        raw = read_file(root, name, max_bytes=4 * 1024 * 1024)
        text = raw.decode("utf-8")
        if count_function_metadata(text) <= 1:
            source_functions.validate_single_source_changes({name: text})
            continue
        records = source_functions.parse_function_records(text)
        manifest = manifests[target]
        owners = [
            entry.id.value
            for entry in manifests.values()
            if name in entry.sources + entry.support_sources
        ]
        if owners != [target] or name not in manifest.sources:
            raise ValueError(
                f"grouped type source requires one explicit lift owner: {name}"
            )
        layout = parse_splat_layout(root / manifest.splat, manifest.load_address)
        expected = expected_lift_sources(layout, root / manifest.source_dir)
        addresses = expected.get(
            source_expected_key(root / manifest.source_dir, root / name)
        )
        addresses = set(addresses) if isinstance(addresses, tuple) else {addresses}
        claimed = {
            address
            for path, address in collect_manifest_source_addresses(
                root, manifest, expected_lifts=expected
            )
            if path == root / name
        }
        identities = {
            (record.address, record.spelling, record.status) for record in records
        }
        supplied = {
            (int(function["address"], 16), function["function"], function["status"])
            for function in selected
        }
        indexed = connection.execute(
            "SELECT address, compiled_symbol, lift_status FROM functions "
            "WHERE target_id=? AND source IN (?, ?)",
            (target, name, str(root / name)),
        ).fetchall()
        fingerprint = connection.execute(
            "SELECT sha256 FROM source_fingerprints WHERE target_id=? AND source_path=?",
            (target, name),
        ).fetchone()
        if (
            any(record.kind != "function" for record in records)
            or {record.address for record in records} != addresses
            or claimed != addresses
            or supplied != identities
            or len(selected) != len(records)
            or len(indexed) != len(records)
            or {tuple(row) for row in indexed} != identities
            or fingerprint is None
            or fingerprint[0] != hashlib.sha256(raw).hexdigest()
        ):
            raise ValueError(f"grouped type source membership or index differs: {name}")


def _source_context(
    text: str, records: tuple[source_functions.FunctionRecord, ...]
) -> str:
    parts, position = [], 0
    for record in records:
        parts.append(text[position : record.implementation_start])
        position = record.implementation_end
    return "".join(parts) + text[position:]


def _validate_source_changes(
    root: Path, manifest: dict[str, Any], changes: object
) -> None:
    if not isinstance(changes, dict):
        return
    for name, content in changes.items():
        if (
            not isinstance(name, str)
            or not name.endswith(".c")
            or not isinstance(content, str)
        ):
            continue
        if name not in manifest["allowed_paths"]:
            source_functions.validate_single_source_changes({name: content})
            continue
        before = read_file(root, name, max_bytes=4 * 1024 * 1024).decode("utf-8")
        if count_function_metadata(before) <= 1:
            source_functions.validate_single_source_changes({name: content})
            continue
        old = source_functions.parse_function_records(before)
        new = source_functions.parse_function_records(content)

        def identity(record):
            return (
                record.address,
                record.metadata,
                record.spelling,
                record.kind,
                record.status,
            )

        if tuple(map(identity, old)) != tuple(map(identity, new)) or _source_context(
            before, old
        ) != _source_context(content, new):
            raise ValueError(
                f"grouped type edit must preserve records and source context: {name}"
            )


def _private_proofs(
    root: Path, values: object, manifests: dict[str, Any]
) -> list[dict[str, Any]]:
    return private_proofs(
        root,
        values,
        manifests,
        normalize_target=_target,
    )


def prepare_transaction(root: Path, request: object) -> dict[str, Any]:
    if not isinstance(request, dict) or request.get("schema") not in {
        REQUEST_SCHEMA,
        links.REQUEST_SCHEMA,
    }:
        raise ValueError(f"type transaction request schema must be {REQUEST_SCHEMA}")
    concern = request.get("concern")
    linked = request["schema"] == links.REQUEST_SCHEMA
    if linked:
        links.validate_request(request)
    elif concern not in CONCERNS:
        raise ValueError(f"unknown type transaction concern: {concern}")
    manifests = load_target_manifests(root)
    target = _target(request.get("target"), manifests)
    header = _header(root, manifests[target], concern, request.get("header"))
    if concern == "shared" and any(header in m.headers for m in manifests.values()):
        raise ValueError(
            "shared type transaction header is not a sanctioned public shared path"
        )
    targets = {target}
    if concern == "shared":
        values = request.get("shared_targets")
        if not isinstance(values, list) or len(values) < 2:
            raise ValueError("shared transaction requires at least two owners")
        targets = {_target(item, manifests) for item in values}
        if target not in targets or len(targets) < 2:
            raise ValueError(
                "shared transaction owners must include the primary target"
            )
    connection = connect(root)
    try:
        rows = _index_rows(connection, request.get("index_candidate_ids"), targets)
        functions = _functions(
            root, connection, request.get("affected_functions"), targets
        )
    finally:
        connection.close()
    paths = [
        validate_repo_path(path)
        for path in artifact_paths(request.get("candidate_artifacts"))
    ]
    validate_paths(root, paths)
    if len(paths) != len(rows):
        raise ValueError("every index lead requires one reviewed candidate artifact")
    reviewed = [
        validate_reviewed_candidate(
            root,
            path,
            ("alias" if row["id"] == links.STORAGE_ID else "layout")
            if linked
            else concern,
            row,
        )
        for path, row in zip(paths, rows)
    ]
    pair = links.capture_pair(root, request, reviewed) if linked else None
    if pair is not None:
        connection = connect(root)
        try:
            links.validate_consumers(root, connection, rows, functions, pair)
        finally:
            connection.close()
    proofs = (
        _private_proofs(root, request.get("private_transaction_proofs"), manifests)
        if concern == "shared"
        else []
    )
    if concern == "shared":
        if any(
            item["application"]["concern"] == "linked"
            or item["application"]["manifest"].get("schema") == links.MANIFEST_SCHEMA
            or "linked_pair" in item["application"]["manifest"]
            for item in proofs
        ):
            raise ValueError("linked type pairs cannot supply shared promotion")
        candidate_targets = {item["candidate"]["target"] for item in reviewed}
        proof_targets = {item["application"]["target"] for item in proofs}
        if candidate_targets != targets or proof_targets != targets:
            raise ValueError("shared promotion must prove every declared owner")
        validate_shared_header(root, header, reviewed, proofs)
    allowed = validate_paths(
        root, {header, *(validate_repo_path(item["source"]) for item in functions)}
    )
    preflight_existing_replacements(root, allowed)
    pre_state = file_state(root, allowed)
    partial_baselines = capture_partial_baselines(root, functions)
    facts = {
        "schema": links.MANIFEST_SCHEMA if linked else MANIFEST_SCHEMA,
        "target": target,
        "targets": sorted(targets),
        "concern": concern,
        "header": header,
        "reviewed_candidates": reviewed,
        "private_transaction_proofs": proofs,
        "affected_functions": functions,
        "allowed_paths": sorted(allowed),
        "pre_state": pre_state,
        "pre_state_digest": digest(pre_state),
        "workspace_baseline": _adopted_baseline(root, request),
        "required_checks": required_checks(
            sorted(targets), functions, partial_baselines
        ),
        "request": request,
    }
    if pair is not None:
        facts["linked_pair"] = pair
    if proofs:
        facts.update(shared_pre.freeze(root, facts, proofs))
    return {**facts, "digest": digest(facts)}


def _manifest(root: Path, value: object, *, rederive: bool = False) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ValueError("type transaction manifest must be an object")
    facts = {key: item for key, item in value.items() if key != "digest"}
    if facts.get("schema") not in {MANIFEST_SCHEMA, links.MANIFEST_SCHEMA} or value.get(
        "digest"
    ) != digest(facts):
        raise ValueError("type transaction manifest drifted")
    if facts["schema"] == links.MANIFEST_SCHEMA:
        links.validate_manifest(root, value)
    elif value.get("concern") == "linked" or "linked_pair" in value:
        raise ValueError("linked type manifest cannot use a legacy schema")
    if "participation_limit" in value:
        raise ValueError("type transactions do not support a participation override")
    if rederive:
        try:
            canonical = prepare_transaction(root, value["request"])
        except (KeyError, TypeError, ValueError) as error:
            raise ValueError(
                "type transaction manifest cannot be re-derived"
            ) from error
        if value != canonical:
            raise ValueError("type transaction manifest is not canonical")
    try:
        expected = {
            validate_repo_path(value["header"]),
            *(
                validate_repo_path(item["source"])
                for item in value["affected_functions"]
            ),
        }
        supplied = set(value["allowed_paths"])
    except (KeyError, TypeError, ValueError) as error:
        raise ValueError("type transaction manifest paths are invalid") from error
    if supplied != expected or len(value["allowed_paths"]) != len(expected):
        raise ValueError("type transaction manifest allowed_paths are forged")
    return value


@bind_deadline
@protect_artifacts
@exclude_writers
def run_transaction(
    root: Path,
    manifest_value: object,
    changes: object,
    *,
    runner: Callable[..., subprocess.CompletedProcess[str]] = run_command,
    implementation_run_id: str | None = None,
    participating_targets: list[str] | None = None,
    output: str | None = None,
    deadline: float | None = None,
) -> dict[str, Any]:
    manifest = _manifest(root, manifest_value, rederive=True)
    links.validate_participation(manifest, implementation_run_id, participating_targets)
    _validate_source_changes(root, manifest, changes)
    allowed = validate_paths(root, manifest["allowed_paths"])
    if (
        file_state(root, allowed) != manifest["pre_state"]
        or _workspace_state(root) != manifest["workspace_baseline"]["state"]
    ):
        raise ValueError("type transaction base drifted")
    validate_paths(root, allowed)
    context = execution_context.begin(
        root, manifest, implementation_run_id, participating_targets
    )
    shared_pre.validate_entry(manifest, context)
    full_backup = workspace_backup(root)
    validate_paths(root, allowed)
    backup: dict[str, bytes | None] = {}
    quarantines = {}
    index_backup = git_index_backup(root)
    safeguards = capture_safeguards(root, set(), full_backup, index_backup)
    try:
        backup, quarantines = apply_changes(
            root,
            changes,
            allowed,
            recovery={
                "owner": "type",
                "manifest": manifest,
                "implementation_run_id": implementation_run_id,
                "output": output,
            },
            workspace=full_backup,
            index=index_backup,
        )
        execution_context.applied(root, manifest, context, changes)
        post = file_state(root, allowed)
        changed = changed_paths(manifest["pre_state"], post)
        if not changed or set(changed) != set(backup):
            raise ValueError("type transaction changed-path set is incomplete")
        post_digest = digest(post)
        receipts = []
        passed = True
        for check in manifest["required_checks"]:
            execution_context.recheck(root, manifest, context)
            current, passed = run_checks(
                root,
                [check],
                post_digest,
                runner=runner,
                start_index=len(receipts),
            )
            receipts.extend(current)
            if git_index_backup(root) != index_backup:
                raise ValueError("type transaction validation changed the Git index")
            execution_context.checked(root, manifest, context, check, current[0])
            if file_state(root, allowed) != post:
                raise ValueError("type transaction validation mutated an allowed path")
            if not passed:
                break
        state = _workspace_state(root)
        baseline = manifest["workspace_baseline"]["state"]
        outside = {name: state[name] for name in state.keys() - allowed}
        if outside != {name: baseline[name] for name in baseline.keys() - allowed}:
            raise ValueError("type transaction changed an unrelated path")
        if not passed:
            raise RuntimeError("type transaction validation failed")
        application = application_record(
            manifest["digest"], manifest["pre_state"], post, changed, receipts
        )
        candidate = manifest["reviewed_candidates"][0]
        summaries = (
            links.summaries(manifest)
            if manifest["schema"] == links.MANIFEST_SCHEMA
            else {
                "representation": candidate["representation"],
                "semantics": candidate["semantics"],
            }
        )
        application.update(
            target=manifest["target"],
            concern=manifest["concern"],
            **summaries,
            manifest=manifest,
            receipts=receipts,
            quarantine_paths={
                name: record["quarantine"]
                for name, record in quarantines.items()
                if record["quarantine"] is not None
            },
            applied=True,
        )
        if context is not None:
            application["review_context"] = context
        execution_context.recheck(root, manifest, context)
        application["digest"] = digest(
            {key: item for key, item in application.items() if key != "digest"}
        )
        application["attestation"] = write_attestation(root, application)
        if git_index_backup(root) != index_backup:
            raise ValueError("type transaction changed the Git index before proof")
        execution_context.recheck(root, manifest, context)
        verify_workspace(root, full_backup)
        execution_context.publish(root, application, output)
        return application
    except ProcessCleanupError:
        raise
    except BaseException:
        with suspend_work_deadline():
            try:
                verify_writer(root)
                rollback(root, backup, quarantines)
            except ProcessCleanupError:
                raise
            except BaseException as error:
                raise RuntimeError("type transaction rollback failed") from error
            try:
                verify_restored_state(root, manifest, index_backup, safeguards)
            except ProcessCleanupError:
                raise
            except BaseException as error:
                raise RuntimeError(
                    "type transaction rollback failed or is unverified; "
                    "external state preserved; parent review required"
                ) from error
        raise


def verify_application(
    root: Path, value: object, expected_application_digest: str
) -> dict[str, Any]:
    manifest = validate_application_history(
        root, value, expected_application_digest, "type", _manifest
    )
    links.validate_application(value)
    allowed = validate_paths(root, manifest["allowed_paths"])
    if file_state(root, allowed) != value["post_state"]:
        raise ValueError("type application post-state drifted")
    execution_context.validate_context(root, value)
    if "shared_pre" in manifest:
        shared_pre.validate_post(root, value, "type", manifest_validator=_manifest)
    return {
        "schema": APPLICATION_SCHEMA,
        "target": value["target"],
        "concern": value["concern"],
        "applied": True,
        "digest": value["digest"],
    }


def candidate_account(root: Path) -> dict[str, Any]:
    return _candidate_account(root, connect)


def validate_account(root: Path, report: object) -> dict[str, Any]:
    current = candidate_account(root)
    if report != current:
        raise ValueError("type candidate account is stale, incomplete, or duplicated")
    return current
