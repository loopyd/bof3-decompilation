"""Reviewed, atomic macro and template application transactions."""

from __future__ import annotations

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
from harness.common.files import preflight_existing_replacements
from harness.common.history import validate_application_history
from harness.common.lease import exclude_writers, verify_writer
from harness.common.submodules import protect_artifacts
from harness.common.runtime import application_record
from harness.common.runtime import apply_changes
from harness.common.directory import validate_repo_path
from harness.common.runtime import changed_paths
from harness.common.paths import file_state
from harness.common.git import git_index_backup
from harness.common.runtime import rollback
from harness.common.safeguards import capture_safeguards, verify_restored_state
from harness.common.process import ProcessCleanupError
from harness.common.runtime import run_checks
from harness.common.paths import validate_paths
from harness.common.workspace import workspace_backup, verify_workspace
from harness.common.runtime import write_attestation
from harness.domain.manifests import load_target_manifests
from harness.domain import functions as source_functions
from harness.macros import accounting as macro_accounting
from harness.macros import creation as macro_creation
from harness.macros import owners as macro_owners
from harness.macros.blocks import validate_minimum
from harness.macros.coverage import validate_consumer_coverage
from harness.macros.ranking import require_ranked_candidate
from harness.macros.participation import (
    resolve_targets,
    validate_manifest,
    validate_wrappers,
)
from harness.macros.review import exact_proofs, reviewed_artifact, validate_proof_inputs
from harness.common.workspace import workspace_state as _workspace_state
from harness.types.transactions import workspace_baseline

REQUEST_SCHEMA = "bof3.macro-transaction-request/v1"
MANIFEST_SCHEMA = "bof3.macro-transaction/v1"
RECEIPT_SCHEMA = "bof3.macro-command-receipt/v1"
APPLICATION_SCHEMA = "bof3.macro-application/v1"
ATTESTATION_SCHEMA = "bof3.macro-application-attestation/v1"
CONCERNS = frozenset({"constant", "expression", "local_template", "shared_template"})


def prepare_transaction(root: Path, request: object) -> dict[str, Any]:
    if not isinstance(request, dict) or request.get("schema") != REQUEST_SCHEMA:
        raise ValueError(f"macro transaction request schema must be {REQUEST_SCHEMA}")
    concern = request.get("concern")
    if concern not in CONCERNS:
        raise ValueError(f"unknown macro transaction concern: {concern}")
    options = (
        {"block_min_instructions": validate_minimum(request["block_min_instructions"])}
        if "block_min_instructions" in request
        else {}
    )
    account = macro_accounting.candidate_account(root, **options)
    if account["safe_application_count"] != 0:
        raise ValueError(
            "macro transaction requires zero current automatic applications"
        )
    manifests = load_target_manifests(root)
    target = macro_owners.resolve_target(request.get("target"), manifests)
    targets = resolve_targets(request, target, manifests, macro_owners.resolve_target)
    proof_refs = request.get("exact_function_proofs")
    if concern != "shared_template" and proof_refs not in (None, []):
        raise ValueError("private macro transactions cannot reference shared proofs")
    from harness.macros.application import verify_reviewed_application

    proofs = (
        exact_proofs(
            root,
            proof_refs,
            manifests,
            expected_count=len(targets),
            normalize_target=macro_owners.resolve_target,
            verify_reviewed_application=verify_reviewed_application,
        )
        if concern == "shared_template"
        else []
    )
    if proofs and {item["target"] for item in proofs} != targets:
        raise ValueError("shared template proofs must cover all declared targets")
    reviewed = reviewed_artifact(
        root,
        request.get("candidate_artifact"),
        concern,
        account,
        manifests,
        proofs=proofs,
    )
    if reviewed["candidate_id"].startswith("assembly_block:"):
        require_ranked_candidate(
            root,
            request.get("ranking"),
            reviewed,
            options.get("block_min_instructions"),
            target,
        )
    if concern != "shared_template" and not targets.issubset(
        set(reviewed["declared_targets"])
    ):
        raise ValueError("macro transaction target does not own reviewed paths")
    connection = connect(root)
    try:
        functions = macro_owners.resolve_functions(
            root, connection, request.get("affected_functions"), targets
        )
        validate_consumer_coverage(
            connection, root, reviewed["owners"], functions, manifests
        )
        validate_wrappers(proofs, functions)
    finally:
        connection.close()
    if concern == "local_template" and any(
        item["status"] != "exact" for item in functions
    ):
        raise ValueError("local template wrappers must be independently exact")
    allowed = validate_paths(
        root,
        {
            *(validate_repo_path(owner) for owner in reviewed["owners"]),
            *(validate_repo_path(item["source"]) for item in functions),
        },
    )
    if reviewed.get("creation"):
        allowed.update(
            validate_paths(root, [macro_creation.manifest_path(reviewed["creation"])])
        )
    preflight_existing_replacements(root, allowed)
    pre_state = file_state(root, allowed)
    baseline = workspace_baseline(root)
    adopted = request.get("adopted_baseline")
    if baseline["state"] and adopted != baseline["digest"]:
        raise ValueError(
            "dirty worktree requires adopted_baseline equal to current workspace digest"
        )
    if not baseline["state"] and adopted not in {None, digest({})}:
        raise ValueError("adopted_baseline does not match the clean worktree")
    partial_baselines = capture_partial_baselines(root, functions)
    facts = {
        "schema": MANIFEST_SCHEMA,
        "participation_limit": len(manifests),
        "target": target,
        "targets": sorted(targets),
        "concern": concern,
        "reviewed_opportunity": reviewed,
        "exact_function_proofs": proofs,
        "affected_functions": functions,
        "allowed_paths": sorted(allowed),
        "pre_state": pre_state,
        "pre_state_digest": digest(pre_state),
        "workspace_baseline": baseline,
        "required_checks": required_checks(
            sorted(targets), functions, partial_baselines
        ),
        "request": request,
    }
    if proofs:
        facts.update(shared_pre.freeze(root, facts, proofs))
    return {**facts, "digest": digest(facts)}


def _manifest(root: Path, value: object, *, rederive: bool = False) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ValueError("macro transaction manifest must be an object")
    facts = {key: item for key, item in value.items() if key != "digest"}
    if facts.get("schema") != MANIFEST_SCHEMA or value.get("digest") != digest(facts):
        raise ValueError("macro transaction manifest drifted")
    validate_manifest(value)
    if rederive:
        try:
            canonical = prepare_transaction(root, value["request"])
        except (KeyError, TypeError, ValueError) as error:
            raise ValueError(
                "macro transaction manifest cannot be re-derived"
            ) from error
        if value != canonical:
            raise ValueError("macro transaction manifest is not canonical")
    try:
        expected = {
            *(
                validate_repo_path(name)
                for name in value["reviewed_opportunity"]["owners"]
            ),
            *(
                validate_repo_path(item["source"])
                for item in value["affected_functions"]
            ),
        }
        if value["reviewed_opportunity"].get("creation"):
            creation = macro_creation.validate_contract(
                value["reviewed_opportunity"]["creation"]
            )
            macro_creation.validate_manifest_state(value, creation)
            expected.add(macro_creation.manifest_path(creation))
        supplied = set(value["allowed_paths"])
    except (KeyError, TypeError, ValueError) as error:
        raise ValueError("macro transaction manifest paths are invalid") from error
    if supplied != expected or len(value["allowed_paths"]) != len(expected):
        raise ValueError("macro transaction manifest allowed_paths are forged")
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
    source_functions.validate_single_source_changes(changes)
    macro_creation.validate_changes(root, manifest, changes)
    allowed = validate_paths(root, manifest["allowed_paths"])
    validate_proof_inputs(root, manifest["exact_function_proofs"])
    if (
        file_state(root, allowed) != manifest["pre_state"]
        or _workspace_state(root) != manifest["workspace_baseline"]["state"]
    ):
        raise ValueError("macro transaction base drifted")
    validate_paths(root, allowed)
    context = execution_context.begin(
        root, manifest, implementation_run_id, participating_targets
    )
    shared_pre.validate_entry(manifest, context)
    full_backup = workspace_backup(root)
    validate_paths(root, allowed)
    backup: dict[str, bytes | None] = {}
    quarantines: dict[str, dict[str, Any]] = {}
    index_backup = git_index_backup(root)
    safeguards = capture_safeguards(root, set(), full_backup, index_backup)
    try:
        backup, quarantines = apply_changes(
            root,
            changes,
            allowed,
            recovery={
                "owner": "macro",
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
            raise ValueError("macro transaction changed-path set is incomplete")
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
                receipt_schema=RECEIPT_SCHEMA,
                evidence_prefix="macro",
            )
            receipts.extend(current)
            current_index = git_index_backup(root)
            if current_index != index_backup:
                raise ValueError("macro transaction validation changed the Git index")
            execution_context.checked(root, manifest, context, check, current[0])
            if file_state(root, allowed) != post:
                raise ValueError("macro transaction validation mutated an allowed path")
            if not passed:
                break
        state = _workspace_state(root)
        outside = {name: state[name] for name in state.keys() - allowed}
        baseline = manifest["workspace_baseline"]["state"]
        if outside != {name: baseline[name] for name in baseline.keys() - allowed}:
            raise ValueError("macro transaction changed an unrelated path")
        if not passed:
            raise RuntimeError("macro transaction validation failed")
        application = application_record(
            manifest["digest"],
            manifest["pre_state"],
            post,
            changed,
            receipts,
            schema=APPLICATION_SCHEMA,
        )
        opportunity = manifest["reviewed_opportunity"]
        application.update(
            target=manifest["target"],
            concern=manifest["concern"],
            semantic_guards=opportunity["semantic_guards"],
            observations=opportunity["observations"],
            exact_function_proofs=(
                [
                    item["selector"]
                    for item in manifest["affected_functions"]
                    if item["status"] == "exact"
                ]
                if manifest["concern"] == "local_template"
                else [item["selector"] for item in manifest["exact_function_proofs"]]
            ),
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
        application["attestation"] = write_attestation(
            root,
            application,
            schema=ATTESTATION_SCHEMA,
            prefix="macro",
        )
        current_index = git_index_backup(root)
        if current_index != index_backup:
            raise ValueError("macro transaction changed the Git index before proof")
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
                raise RuntimeError("macro transaction rollback failed") from error
            try:
                verify_restored_state(root, manifest, index_backup, safeguards)
            except ProcessCleanupError:
                raise
            except BaseException as error:
                raise RuntimeError(
                    "macro transaction rollback failed or is unverified; "
                    "external state preserved; parent review required"
                ) from error
        raise


def verify_application(
    root: Path, value: object, expected_application_digest: str
) -> dict[str, Any]:
    manifest = validate_application_history(
        root, value, expected_application_digest, "macro", _manifest
    )
    allowed = validate_paths(root, manifest["allowed_paths"])
    if file_state(root, allowed) != value["post_state"]:
        raise ValueError("macro application post-state drifted")
    execution_context.validate_context(root, value)
    if "shared_pre" in manifest:
        shared_pre.validate_post(root, value, "macro", manifest_validator=_manifest)
    return {
        "schema": APPLICATION_SCHEMA,
        "target": value["target"],
        "concern": value["concern"],
        "applied": True,
        "digest": value["digest"],
    }
