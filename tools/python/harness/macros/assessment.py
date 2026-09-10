"""Pinned scope preparation for unchanged existing macro assessments."""

from __future__ import annotations

from pathlib import Path

from harness.analysis.index import connect
from harness.common.checks import required_checks
from harness.common.digests import digest
from harness.common.paths import file_state, validate_paths
from harness.common.inputs import file_state as inspect_file
from harness.domain.manifests import load_target_manifests
from harness.macros.accounting import candidate_account
from harness.macros.coverage import validate_consumer_coverage
from harness.macros.ranking import require_ranked_candidate
from harness.macros.resolution import describe_candidate
from harness.macros.review import _wrapper_dependencies, reviewed_artifact
from harness.macros.transactions import _functions, _target
from harness.types.transactions import workspace_baseline

REQUEST_SCHEMA = "bof3.macro-existing-request/v1"
MANIFEST_SCHEMA = "bof3.macro-existing-assessment/v1"


def prepare_assessment(root: Path, request: object) -> dict:
    required = {
        "schema",
        "target",
        "targets",
        "candidate_artifact",
        "candidate_id",
        "candidate_fingerprint",
        "definition_ids",
        "affected_functions",
        "adopted_baseline",
        "rationale",
    }
    if (
        not isinstance(request, dict)
        or not required
        <= set(request)
        <= required | {"block_min_instructions", "ranking"}
        or request.get("schema") != REQUEST_SCHEMA
        or not isinstance(request.get("candidate_id"), str)
        or not isinstance(request.get("rationale"), str)
        or not request["rationale"].strip()
    ):
        raise ValueError("invalid existing macro assessment request")
    if not request["candidate_id"].startswith("assembly_block:") and {
        "ranking",
        "block_min_instructions",
    } & set(request):
        raise ValueError("existing assessment has inapplicable block ranking fields")
    manifests = load_target_manifests(root)
    target = _target(request["target"], manifests)
    supplied = request["targets"]
    if not isinstance(supplied, list) or not supplied:
        raise ValueError("existing assessment requires explicit targets")
    targets = sorted({_target(value, manifests) for value in supplied})
    if supplied != targets or target not in targets:
        raise ValueError("existing assessment targets must be canonical and unique")
    options = (
        {"block_min_instructions": request["block_min_instructions"]}
        if "block_min_instructions" in request
        else {}
    )
    account = candidate_account(root, **options)
    descriptor = describe_candidate(
        root,
        request["candidate_id"],
        min_instructions=request.get("block_min_instructions"),
    )
    if descriptor["candidate_fingerprint"] != request["candidate_fingerprint"]:
        raise ValueError("existing assessment candidate fingerprint drifted")
    concern = "shared_template" if len(targets) > 1 else "local_template"
    reviewed = reviewed_artifact(
        root,
        request["candidate_artifact"],
        concern,
        account,
        manifests,
    )
    if (reviewed["candidate_id"], reviewed["candidate_fingerprint"]) != (
        request["candidate_id"],
        request["candidate_fingerprint"],
    ):
        raise ValueError("existing assessment review identifies another candidate")
    if concern == "local_template" and target not in reviewed["declared_targets"]:
        raise ValueError("existing assessment owner is not target-local")
    if request["candidate_id"].startswith("assembly_block:"):
        require_ranked_candidate(
            root,
            request.get("ranking"),
            reviewed,
            request.get("block_min_instructions"),
            target,
        )
    connection = connect(root)
    try:
        selectors = request["affected_functions"]
        if (
            not isinstance(selectors, list)
            or any(not isinstance(item, str) for item in selectors)
            or selectors != sorted(set(selectors))
        ):
            raise ValueError(
                "existing assessment requires distinct canonical selectors"
            )
        functions = _functions(
            root, connection, request["affected_functions"], set(targets)
        )
        if any(function["status"] != "exact" for function in functions):
            raise ValueError("existing assessment requires exact affected functions")
        if {function["target"] for function in functions} != set(targets):
            raise ValueError("existing assessment omits a declared target")
        if len(
            {(function["target"], function["address"]) for function in functions}
        ) != len(functions):
            raise ValueError("existing assessment duplicates a function identity")
        definitions = _validate_binding(root, descriptor, request, reviewed, functions)
        validate_consumer_coverage(
            connection,
            root,
            reviewed["owners"],
            functions,
            manifests,
            definition_ids={row["id"] for row in definitions},
            require_function_identity=True,
        )
    finally:
        connection.close()
    paths = validate_paths(
        root, set(reviewed["owners"]) | {item["source"] for item in functions}
    )
    baseline = workspace_baseline(root)
    if request["adopted_baseline"] != baseline["digest"]:
        raise ValueError("existing assessment must adopt the full current workspace")
    state = file_state(root, paths)
    facts = {
        "schema": MANIFEST_SCHEMA,
        "target": target,
        "targets": targets,
        "participation_limit": len(manifests),
        "concern": "existing_abstraction",
        "reviewed_opportunity": reviewed,
        "opportunity_evidence": {
            "path": reviewed["artifact"],
            "sha256": reviewed["artifact_sha256"],
        },
        "existing_definitions": definitions,
        "affected_functions": functions,
        "allowed_paths": sorted(paths),
        "pre_state": state,
        "pre_state_digest": digest(state),
        "workspace_baseline": baseline,
        "workspace_evidence": [
            {
                "path": name,
                **(inspect_file(root / name) or {"sha256": None, "mode": None}),
            }
            for name in baseline["state"]
        ],
        "required_checks": required_checks(targets, functions),
        "request": request,
    }
    return {**facts, "digest": digest(facts)}


def _validate_binding(root, descriptor, request, reviewed, functions):
    supplied = request["definition_ids"]
    if (
        not isinstance(supplied, list)
        or not supplied
        or any(not isinstance(value, str) for value in supplied)
        or supplied != sorted(set(supplied))
    ):
        raise ValueError("existing assessment requires distinct definition IDs")
    definitions = [
        row for row in descriptor["existing_definitions"] if row["id"] in supplied
    ]
    if len(definitions) != len(supplied) or any(
        row["source_path"] not in reviewed["owners"] for row in definitions
    ):
        raise ValueError("existing definitions must belong to reviewed owners")
    covered = {(row["target"], row["source"]) for row in functions}
    identities = {(row["target"], int(row["address"], 16)) for row in functions}
    members = descriptor["members"]
    if len(members) < 4:
        raise ValueError("existing assessment requires at least four candidate uses")
    witnessed = set()
    for member in members:
        identity = member["member"]
        target = (
            identity.get("target") or identity.get("function", "").rsplit("@", 1)[0]
        )
        source = member["source_path"]
        if (target, source) not in covered:
            raise ValueError("existing assessment omits a candidate member")
        if (
            identity.get("function")
            and (target, int(identity["function"].rsplit("@", 1)[1], 16))
            not in identities
        ):
            raise ValueError("existing assessment omits a candidate function identity")
        dependencies = _wrapper_dependencies(root, source) | {source}
        matches = {
            use["definition_id"]
            for use in descriptor["existing_macro_uses"]
            if use["target_id"] == target
            and use["source_path"] == source
            and use["definition_id"] in supplied
            and use["use_context"] != "definition_body"
            and any(
                row["id"] == use["definition_id"] and row["source_path"] in dependencies
                for row in definitions
            )
        }
        if not matches:
            raise ValueError("candidate member has no included existing macro use")
        witnessed.update(matches)
    if witnessed != set(supplied):
        raise ValueError("existing assessment includes an unwitnessed definition")
    return definitions


def validate_assessment(root: Path, value: object) -> dict:
    if not isinstance(value, dict) or value.get("schema") != MANIFEST_SCHEMA:
        raise ValueError("invalid existing macro assessment manifest")
    if value != prepare_assessment(root, value.get("request")):
        raise ValueError("existing macro assessment manifest drifted")
    return value
