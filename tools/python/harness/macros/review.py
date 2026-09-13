"""Validation for reviewed macro opportunities and shared wrapper proofs."""

from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any, Callable

from harness.common.digests import digest
from harness.common.directory import validate_repo_path
from harness.common.paths import file_state, validate_paths
from harness.domain.includes import local_include_files
from harness.domain.receipts import sha256_file
from harness.io import unique_object
from harness.macros import creation as macro_creation
from harness.macros import owners as macro_owners

REVIEW_SCHEMA = "bof3.reviewed-macro-opportunity/v1"
GUARDS = frozenset(
    {
        "evaluation_count",
        "side_effects",
        "integer_promotions",
        "precedence",
        "lvalue",
        "volatile",
        "aliasing",
        "control_flow",
    }
)
OBSERVATIONS = frozenset({"parameter_mapping", "all_use_sites"})
_KINDS = {
    "constant": {"constant"},
    "expression": {"expression_accessor"},
    "local_template": {
        "assembly_block",
        "statement_window",
        "exact_group",
        "parameterized_near_duplicate",
    },
    "shared_template": {
        "exact_group",
        "parameterized_near_duplicate",
        "assembly_block",
    },
}
_ADDRESS = re.compile(r"0x[0-9A-Fa-f]+")
_DIGEST = re.compile(r"v1:[0-9a-f]{64}")


def validate_proof_inputs(root: Path, proofs: object) -> None:
    try:
        proof_drifted = any(
            (root / item["path"]).is_symlink()
            or not (root / item["path"]).is_file()
            or sha256_file(root / item["path"]) != item["sha256"]
            for item in proofs
        )
    except (KeyError, TypeError):
        proof_drifted = True
    if proof_drifted:
        raise ValueError("shared template proof drifted")


def repo_path(root: Path, value: object) -> str:
    try:
        name = validate_repo_path(value)
        validate_paths(root, {name})
        if file_state(root, {name})[name] is None:
            raise ValueError
    except (OSError, ValueError):
        raise ValueError(f"macro transaction path is invalid: {value}") from None
    return name


def _shared_owner_kind(root: Path, name: str, manifests: dict[str, Any]) -> str | None:
    try:
        relative = (root / name).resolve().relative_to(root.resolve())
    except ValueError:
        return None
    if name != relative.as_posix():
        return None
    if relative.is_relative_to("src/shared") and relative.suffix == ".inc":
        return "template"
    private_headers = {
        Path(header).as_posix()
        for manifest in manifests.values()
        for header in manifest.headers
    }
    if (
        relative.is_relative_to("include")
        and relative.suffix == ".h"
        and not relative.name.endswith("_internal.h")
        and name not in private_headers
    ):
        return "public_header"
    return None


def _proof_wrapper_sources(
    proofs: list[dict[str, Any]], manifests: dict[str, Any]
) -> dict[str, str]:
    wrappers = {}
    for proof in proofs:
        target = proof.get("target")
        selector = proof.get("selector")
        manifest = manifests.get(target)
        application_manifest = proof.get("application", {}).get("manifest", {})
        functions = application_manifest.get("affected_functions", [])
        matches = [
            item
            for item in functions
            if item.get("selector") == selector
            and item.get("target") == target
            and item.get("status") == "exact"
            and isinstance(item.get("source"), str)
        ]
        claimed = (
            set(manifest.sources + manifest.support_sources)
            if manifest is not None
            else set()
        )
        if len(matches) != 1 or matches[0]["source"] not in claimed:
            raise ValueError(
                "shared template proof does not identify a target-local exact wrapper"
            )
        wrappers[target] = matches[0]["source"]
    if len(wrappers) < 2 or len(wrappers) != len(proofs):
        raise ValueError(
            "shared template requires distinct target-local exact wrappers"
        )
    return wrappers


def _wrapper_dependencies(root: Path, source: str) -> set[str]:
    path = root / source
    dependencies = local_include_files(
        root,
        [path],
        include_roots=(
            root / "src",
            root / "include",
            root / "toolchains/psyq/4.7/include",
        ),
        include_angle=True,
    )
    return {
        dependency.resolve().relative_to(root.resolve()).as_posix()
        for dependency in dependencies
        if dependency.is_file() and dependency.resolve().is_relative_to(root.resolve())
    }


def _shared_targets(
    root: Path,
    owners: list[str],
    proofs: list[dict[str, Any]],
    manifests: dict[str, Any],
) -> set[str]:
    kinds = {owner: _shared_owner_kind(root, owner, manifests) for owner in owners}
    if not all(kinds.values()):
        raise ValueError("shared template owner is not a sanctioned shared path")
    if not proofs:
        return set()
    wrappers = _proof_wrapper_sources(proofs, manifests)
    for target, source in wrappers.items():
        dependencies = _wrapper_dependencies(root, source)
        claims = set(manifests[target].sources + manifests[target].support_sources)
        if any(
            owner not in dependencies
            and not (kinds[owner] == "public_header" and owner in claims)
            for owner in owners
        ):
            raise ValueError(
                "shared template owner is not a proven dependency of every exact wrapper"
            )
    return set(wrappers)


def candidate_concerns(kind: str) -> list[str]:
    """Return supported review concerns without authorizing a candidate."""
    return sorted(concern for concern, kinds in _KINDS.items() if kind in kinds)


def candidate_observations(kind: str) -> frozenset[str]:
    return OBSERVATIONS | {"human_value"} if kind == "assembly_block" else OBSERVATIONS


def reviewed_artifact(
    root: Path,
    value: object,
    concern: str,
    account: dict[str, Any],
    manifests: dict[str, Any],
    *,
    proofs: list[dict[str, Any]] | None = None,
) -> dict[str, Any]:
    name = repo_path(root, value)
    try:
        artifact = json.loads(
            (root / name).read_text(encoding="utf-8"), object_pairs_hook=unique_object
        )
    except (json.JSONDecodeError, UnicodeDecodeError) as error:
        raise ValueError("reviewed macro opportunity is not JSON") from error
    if not isinstance(artifact, dict):
        raise ValueError("reviewed macro opportunity must be an object")
    facts = {key: item for key, item in artifact.items() if key != "digest"}
    required = {
        "schema",
        "candidate_id",
        "candidate_fingerprint",
        "concern",
        "owners",
        "owner_fingerprints",
        "semantic_guards",
        "observations",
        "review",
        "digest",
    }
    creating = artifact.get("schema") == macro_creation.REVIEW_SCHEMA
    creation = None
    if creating:
        required.add("creation")
        if concern != "local_template":
            raise ValueError("header creation requires a local template review")
        creation = macro_creation.validate_admission(
            root, artifact.get("creation"), manifests
        )
    if (
        set(artifact) != required
        or artifact.get("schema") not in {REVIEW_SCHEMA, macro_creation.REVIEW_SCHEMA}
        or artifact.get("digest") != digest(facts)
        or artifact.get("concern") != concern
    ):
        raise ValueError("reviewed macro opportunity artifact drifted")
    row = {item["id"]: item for item in account["rows"]}.get(
        artifact.get("candidate_id")
    )
    if (
        row is None
        or row["kind"] not in _KINDS[concern]
        or row["candidate_fingerprint"] != artifact.get("candidate_fingerprint")
    ):
        raise ValueError("reviewed macro opportunity fingerprint drifted")
    owners = artifact.get("owners")
    fingerprints = artifact.get("owner_fingerprints")
    if (
        not isinstance(owners, list)
        or not owners
        or len(set(owners)) != len(owners)
        or not isinstance(fingerprints, dict)
        or set(fingerprints) != set(owners)
    ):
        raise ValueError("reviewed macro opportunity owners are invalid")
    for owner in owners:
        if creation is not None:
            if owners != [creation["header"]] or fingerprints[owner] is not None:
                raise ValueError("creation requires its sole absent header owner")
            continue
        path = repo_path(root, owner)
        if sha256_file(root / path) != fingerprints[owner]:
            raise ValueError("reviewed macro opportunity owner fingerprint drifted")
    guards = artifact.get("semantic_guards")
    observations = artifact.get("observations")
    if (
        not isinstance(guards, dict)
        or set(guards) != GUARDS
        or any(
            not isinstance(item, dict)
            or item.get("status") not in {"resolved", "not_applicable"}
            or not isinstance(item.get("evidence"), str)
            or not item["evidence"].strip()
            for item in guards.values()
        )
        or not isinstance(observations, dict)
        or set(observations) != candidate_observations(row["kind"])
        or any(
            not isinstance(item, str) or not item.strip()
            for item in observations.values()
        )
    ):
        raise ValueError("reviewed macro opportunity has unresolved semantic guards")
    if concern == "shared_template":
        contract = json.dumps(
            {"semantic_guards": guards, "observations": observations},
            sort_keys=True,
        )
        if any(
            int(address, 16) >= 0x80000000 for address in _ADDRESS.findall(contract)
        ):
            raise ValueError("shared template review contract contains an address leak")
    review = artifact.get("review")
    if (
        not isinstance(review, dict)
        or review.get("verdict") != "accepted"
        or not isinstance(review.get("reviewer"), str)
        or not review["reviewer"].strip()
    ):
        raise ValueError("reviewed macro opportunity is not accepted")
    owner_set = set(owners)
    if creation is not None:
        declared_targets = {creation["target"]}
    elif concern == "shared_template":
        declared_targets = _shared_targets(root, owners, proofs or [], manifests)
    else:
        declared_targets = {
            target
            for target, manifest in manifests.items()
            if owner_set <= macro_owners.target_owned_paths(root, target, manifest)
        }
        if not declared_targets:
            raise ValueError(
                "private macro opportunity paths are not all owned by one target"
            )
    return {
        "artifact": name,
        "artifact_sha256": sha256_file(root / name),
        "candidate_id": artifact["candidate_id"],
        "candidate_fingerprint": artifact["candidate_fingerprint"],
        "owners": owners,
        "owner_fingerprints": fingerprints,
        "semantic_guards": guards,
        "observations": observations,
        "review": review,
        "declared_targets": sorted(declared_targets),
        **({"creation": creation} if creation is not None else {}),
    }


def exact_proofs(
    root: Path,
    values: object,
    manifests: dict[str, Any],
    *,
    expected_count: int = 2,
    normalize_target: Callable[[object, dict[str, Any]], str],
    verify_reviewed_application: Callable[[Path, object, str], dict[str, Any]],
) -> list[dict[str, Any]]:
    if (
        expected_count < 2
        or not isinstance(values, list)
        or len(values) != expected_count
    ):
        raise ValueError(
            "shared template requires one independent exact proof per declared target"
        )
    proofs = []
    for value in values:
        if not isinstance(value, dict) or set(value) != {
            "path",
            "target",
            "selector",
            "expected_envelope_digest",
        }:
            raise ValueError("shared template proof pin is invalid")
        name = repo_path(root, value["path"])
        proof_path = (root / name).resolve()
        if not proof_path.is_relative_to((root / "out/reviews").resolve()):
            raise ValueError("shared template proof path is invalid")
        expected = value["expected_envelope_digest"]
        if not isinstance(expected, str) or not _DIGEST.fullmatch(expected):
            raise ValueError("shared template proof digest pin is invalid")
        proof = json.loads(
            (root / name).read_text(encoding="utf-8"), object_pairs_hook=unique_object
        )
        from harness.macros.transactions import _manifest
        from harness.common.promotion import private_application

        envelope = proof
        proof, verified = private_application(
            root,
            envelope,
            expected,
            "macro",
            verify_reviewed_application,
            manifest_validator=_manifest,
        )
        target = normalize_target(value["target"], manifests)
        if (
            verified["target"] != target
            or proof["concern"] != "local_template"
            or value["selector"] not in proof["exact_function_proofs"]
        ):
            raise ValueError("shared template proof is not a pinned exact wrapper")
        proofs.append(
            {
                "path": name,
                "sha256": sha256_file(root / name),
                "target": target,
                "selector": value["selector"],
                "expected_envelope_digest": expected,
                "application": proof,
                "reviewed_envelope": envelope,
            }
        )
    uniqueness = ("path", "target", "selector", "expected_envelope_digest")
    if any(len({item[key] for item in proofs}) != expected_count for key in uniqueness):
        raise ValueError("shared template proofs must be independently pinned")
    contracts = {
        json.dumps(
            {
                "semantic_guards": item["application"]["semantic_guards"],
                "observations": item["application"]["observations"],
            },
            sort_keys=True,
        )
        for item in proofs
    }
    if len(contracts) != 1 or any(
        int(address, 16) >= 0x80000000
        for address in _ADDRESS.findall(next(iter(contracts)))
    ):
        raise ValueError(
            "shared template proof contracts differ or contain address leaks"
        )
    return sorted(proofs, key=lambda item: (item["target"], item["path"]))


__all__ = [
    "GUARDS",
    "OBSERVATIONS",
    "REVIEW_SCHEMA",
    "exact_proofs",
    "reviewed_artifact",
    "validate_proof_inputs",
]
