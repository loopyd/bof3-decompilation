"""Admission and retained policy for reviewed type-and-reconstruction changes."""

from __future__ import annotations

import copy
import hashlib
import json
import re
from pathlib import Path

from harness.common.commands import tool_id
from harness.common.digests import digest
from harness.common.directory import validate_repo_path
from harness.common.files import read_file
from harness.domain import functions
from harness.domain.tags import count_function_metadata, parse_progress_tags
from harness.io import unique_object
from harness.match.evidence import comparison_digest

REQUEST_SCHEMA = "bof3.type-transaction-request/v4"
MANIFEST_SCHEMA = "bof3.type-transaction/v4"
PROPOSAL_SCHEMA = "bof3.type-reconstruction-proposal/v1"
_SHA = re.compile(r"[0-9a-f]{64}\Z")
_REVIEW_ITEMS = {
    "semantics",
    "abi",
    "range",
    "memory_events",
    "control_flow",
    "frame",
    "aids",
}


def _keys(value: object, names: str) -> None:
    if not isinstance(value, dict) or set(value) != set(names.split()):
        raise ValueError("invalid reconstruction record fields")


def _hash(value: object) -> None:
    if not isinstance(value, str) or not _SHA.fullmatch(value):
        raise ValueError("invalid reconstruction SHA-256")


def _name(value: object) -> None:
    if not isinstance(value, str) or not value.strip() or value != value.strip():
        raise ValueError("invalid reconstruction review identity or finding")


def request_binding(request: dict) -> str:
    # The proposal reference and adopted dirty baseline are transport bindings,
    # not part of the independently reviewed semantic request.
    return digest(
        {k: v for k, v in request.items() if k not in {"proposal", "adopted_baseline"}}
    )


def validate_request(request: dict) -> None:
    required = {
        "schema",
        "target",
        "concern",
        "type_concern",
        "header",
        "index_candidate_ids",
        "candidate_artifacts",
        "affected_functions",
        "reconstruction_functions",
        "proposal",
    }
    if set(request) not in (required, required | {"adopted_baseline"}):
        raise ValueError("invalid reconstruction request fields")
    if request["schema"] != REQUEST_SCHEMA or request["concern"] != "reconstruction":
        raise ValueError("reconstruction requires the V4 request")
    if request["type_concern"] not in {"alias", "layout", "field", "prototype"}:
        raise ValueError("reconstruction excludes shared and linked changes")
    selected = request["reconstruction_functions"]
    if (
        not isinstance(selected, list)
        or not selected
        or any(not isinstance(v, str) or not v for v in selected)
        or selected != sorted(set(selected))
    ):
        raise ValueError(
            "reconstruction functions must be a sorted nonempty identity set"
        )
    _keys(request["proposal"], "path sha256")
    validate_repo_path(request["proposal"]["path"])
    _hash(request["proposal"]["sha256"])


def _read_proposal(root: Path, reference: dict) -> dict:
    raw = read_file(root, reference["path"], max_bytes=4 * 1024 * 1024)
    if raw is None or hashlib.sha256(raw).hexdigest() != reference["sha256"]:
        raise ValueError("reconstruction proposal bytes drifted")
    value = json.loads(raw, object_pairs_hook=unique_object)
    _keys(value, "schema request_digest pre_state images members review digest")
    if value["schema"] != PROPOSAL_SCHEMA or value["digest"] != digest(
        {k: v for k, v in value.items() if k != "digest"}
    ):
        raise ValueError("reconstruction proposal digest drifted")
    return value


def _payload(member: dict, tool: str, check: dict, root: Path) -> dict:
    from harness.common.checks import _evidence, _exact_evidence

    item = member["expected"][tool]
    _keys(item, "exit_code payload identity_digest")
    payload = item["payload"]
    if not isinstance(payload, dict):
        raise ValueError("invalid expected comparison payload")
    raw = json.dumps(payload)
    exact = payload.get("byte_match") is True
    if type(item["exit_code"]) is not int or item["exit_code"] != (0 if exact else 1):
        raise ValueError("expected comparison exit code disagrees")
    if exact:
        if not _exact_evidence(check, raw, root):
            raise ValueError("invalid expected exact evidence")
    else:
        _evidence(check, raw, root)
    if comparison_digest(payload, root) != item["identity_digest"]:
        raise ValueError("expected comparison identity drifted")
    from harness.common.checks import _canonical_source
    from harness.domain.manifests import load_target_manifests
    from harness.match._asm_resolve import extract_original_bytes

    target = load_target_manifests(root)[check["target"]]
    if (
        _canonical_source(payload["original_binary"], target.binary, root) is None
        or type(payload["original_size"]) is not int
        or payload["original_size"] <= 0
        or payload["original_size"] % 4
        or payload["current_size"] <= 0
        or payload["current_size"] % 4
    ):
        raise ValueError("reconstruction comparison range or binary differs")
    original = extract_original_bytes(
        root / target.binary,
        address=int(payload["address"], 16),
        size=payload["original_size"],
        load_address=target.load_address,
    )
    if (
        hashlib.sha256(original).hexdigest()
        != payload["function_image"]["original_sha256"]
    ):
        raise ValueError("reconstruction original function-image differs")
    if exact != (
        payload["original_size"] == payload["current_size"]
        and payload["function_image"]["original_sha256"]
        == payload["function_image"]["current_sha256"]
    ):
        raise ValueError(
            "reconstruction exact status and function-image hashes disagree"
        )
    if tool == "asm-diff" and (
        payload["instruction_count"]["original"] * 4 != payload["original_size"]
        or payload["instruction_count"]["current"] * 4 != payload["current_size"]
    ):
        raise ValueError(
            "reconstruction instruction counts and function sizes disagree"
        )
    return payload


def validate_policy(root: Path, manifest: dict) -> dict:
    """Validate immutable admission bindings without recapturing historical PRE."""
    request = manifest["request"]
    validate_request(request)
    _keys(manifest["reconstruction"], "proposal reference preservation_checks")
    if manifest["reconstruction"]["reference"] != request["proposal"]:
        raise ValueError("reconstruction proposal reference differs")
    proposal = manifest["reconstruction"]["proposal"]
    if proposal != _read_proposal(root, request["proposal"]):
        raise ValueError("retained reconstruction proposal differs")
    if proposal["request_digest"] != request_binding(request):
        raise ValueError("reconstruction proposal request binding differs")
    if proposal["pre_state"] != manifest["pre_state"]:
        raise ValueError("reconstruction PRE binding differs")
    if manifest.get("expected_post_state") != {
        **proposal["pre_state"],
        **proposal["images"],
    }:
        raise ValueError("reconstruction expected POST state differs")
    allowed = set(manifest["allowed_paths"])
    images = proposal["images"]
    if not isinstance(images, dict) or not images or not set(images) <= allowed:
        raise ValueError("reconstruction images exceed owned scope")
    if manifest["header"] not in images:
        raise ValueError("reconstruction proposal must bind its header image")
    if any(name != manifest["header"] and not name.endswith(".c") for name in images):
        raise ValueError("reconstruction image is not the selected header or a source")
    for name, sha in images.items():
        validate_repo_path(name)
        _hash(sha)
        if sha == proposal["pre_state"].get(name):
            raise ValueError("reconstruction proposal contains an unchanged image")
    review = proposal["review"]
    _keys(review, "accepted author_run_id reviewer_run_id artifact findings")
    if review["accepted"] is not True:
        raise ValueError("reconstruction review is not accepted")
    for key in ("author_run_id", "reviewer_run_id"):
        _name(review[key])
    if review["author_run_id"] == review["reviewer_run_id"]:
        raise ValueError("reconstruction requires independent review")
    _keys(review["artifact"], "path sha256")
    artifact = validate_repo_path(review["artifact"]["path"])
    _hash(review["artifact"]["sha256"])
    raw = read_file(root, artifact, max_bytes=4 * 1024 * 1024)
    if not raw or hashlib.sha256(raw).hexdigest() != review["artifact"]["sha256"]:
        raise ValueError("reconstruction review artifact drifted")
    attestation = json.loads(raw, object_pairs_hook=unique_object)
    expected_review = {
        "schema": "bof3.type-reconstruction-review/v1",
        "accepted": True,
        "author_run_id": review["author_run_id"],
        "reviewer_run_id": review["reviewer_run_id"],
        "request_digest": proposal["request_digest"],
        "pre_state_digest": digest(proposal["pre_state"]),
        "images_digest": digest(proposal["images"]),
        "members_digest": digest(proposal["members"]),
        "findings": review["findings"],
    }
    if attestation != expected_review:
        raise ValueError(
            "independent reconstruction review does not bind this proposal"
        )
    if (
        not isinstance(review["findings"], dict)
        or set(review["findings"]) != _REVIEW_ITEMS
    ):
        raise ValueError("reconstruction review omits semantic findings")
    for finding in review["findings"].values():
        _name(finding)
    selected = request["reconstruction_functions"]
    members = proposal["members"]
    if (
        not isinstance(members, list)
        or [m.get("selector") for m in members] != selected
    ):
        raise ValueError("reconstruction membership differs")
    by_id = {f["selector"]: f for f in manifest["affected_functions"]}
    if len(by_id) != len(manifest["affected_functions"]) or any(
        f["target"] != request["target"] or f["status"] not in {"partial", "exact"}
        for f in by_id.values()
    ):
        raise ValueError("invalid reconstruction consumer identities")
    if set(images) != {
        manifest["header"],
        *(by_id[s]["source"] for s in selected if s in by_id),
    }:
        raise ValueError(
            "reconstruction images must exactly cover selected sources and header"
        )
    checks = manifest["required_checks"]
    from harness.common.checks import required_checks

    preserved = manifest["reconstruction"]["preservation_checks"]
    if not isinstance(preserved, list):
        raise ValueError("missing original preservation checks")
    baselines = {}
    for function in by_id.values():
        pair = [c for c in preserved if c.get("selector") == function["selector"]]
        if len(pair) != 2:
            raise ValueError("incomplete reconstruction consumer checks")
        if function["status"] == "partial":
            baselines[function["selector"]] = [c.get("partial_baseline") for c in pair]
            for baseline in baselines[function["selector"]]:
                if not isinstance(baseline, dict) or baseline.get("digest") != digest(
                    {k: v for k, v in baseline.items() if k != "digest"}
                ):
                    raise ValueError("original preservation baseline drifted")
    if preserved != required_checks(
        manifest["targets"], manifest["affected_functions"], baselines
    ):
        raise ValueError("original reconstruction consumer checks differ")
    if [c for c in checks if c.get("selector") not in selected] != [
        c for c in preserved if c.get("selector") not in selected
    ]:
        raise ValueError("unselected consumer preservation checks differ")
    if manifest["concern"] != "reconstruction" or manifest["targets"] != [
        request["target"]
    ]:
        raise ValueError("reconstruction requires one target and its own concern")
    if not set(selected) <= set(by_id):
        raise ValueError("reconstruction selection exceeds consumer scope")
    for check in checks:
        if check["selector"] not in selected and (
            "reconstruction" in check or "--image-evidence" in check["argv"]
        ):
            raise ValueError("unselected consumer has reconstruction policy")
    for member in members:
        _keys(member, "selector pre expected metadata findings")
        if (
            not isinstance(member["findings"], dict)
            or set(member["findings"]) != _REVIEW_ITEMS
        ):
            raise ValueError("reconstruction member omits semantic review")
        for finding in member["findings"].values():
            _name(finding)
        function = by_id.get(member["selector"])
        if function is None or function["source"] not in images:
            raise ValueError("reconstruction member lacks owned proposed source")
        _keys(member["pre"], "asm-diff byte-match")
        _keys(member["expected"], "asm-diff byte-match")
        pair = [c for c in checks if c["selector"] == member["selector"]]
        if len(pair) != 2 or [tool_id(c["argv"]) for c in pair] != [
            "asm-diff",
            "byte-match",
        ]:
            raise ValueError("reconstruction member lacks complete native checks")
        payloads = []
        pre_payloads = []
        for tool, check in zip(("asm-diff", "byte-match"), pair):
            from harness.common.checks import _check

            expected_check = _check(
                tool,
                function["target"],
                function["selector"],
                function["function"],
                function["source"],
            )
            expected_check["argv"].append("--image-evidence")
            expected_check["command"] = " ".join(expected_check["argv"])
            if {
                k: v for k, v in check.items() if k != "reconstruction"
            } != expected_check:
                raise ValueError("reconstruction check command or identity differs")
            pre = member["pre"][tool]
            _keys(pre, "exit_code payload identity_digest")
            original_member = {"expected": {tool: pre}}
            pre_payload = _payload(original_member, tool, check, root)
            post_payload = _payload(member, tool, check, root)
            if (
                pre_payload["original_size"] != post_payload["original_size"]
                or pre_payload["function_image"]["original_sha256"]
                != post_payload["function_image"]["original_sha256"]
                or abs(post_payload["size_delta"]) > abs(pre_payload["size_delta"])
                or (function["status"] == "exact" and not post_payload["byte_match"])
                or pre["exit_code"] != (0 if function["status"] == "exact" else 1)
            ):
                raise ValueError(
                    "reconstruction changes original range or regresses preservation"
                )
            if tool == "asm-diff":
                before, after = (
                    pre_payload["instruction_count"],
                    post_payload["instruction_count"],
                )
                if (
                    before["original"] != after["original"]
                    or after["matching"] < before["matching"]
                ):
                    raise ValueError("reconstruction instruction match regressed")
            expected_policy = {
                "expected": member["expected"][tool],
                "proposal_digest": proposal["digest"],
            }
            if (
                check.get("reconstruction") != expected_policy
                or "partial_baseline" in check
            ):
                raise ValueError("reconstruction native policy drifted")
            payloads.append(post_payload)
            pre_payloads.append(pre_payload)
        if any(
            payloads[0][k] != payloads[1][k]
            for k in (
                "source",
                "function",
                "address",
                "original_size",
                "current_size",
                "byte_match",
                "function_image",
                "size_delta",
            )
        ):
            raise ValueError("reconstruction comparison pair differs")
        if any(
            pre_payloads[0][k] != pre_payloads[1][k]
            for k in (
                "source",
                "function",
                "address",
                "original_size",
                "current_size",
                "byte_match",
                "function_image",
                "size_delta",
            )
        ):
            raise ValueError("reconstruction PRE comparison pair differs")
        metadata = member["metadata"]
        _keys(metadata, "status match residual")
        exact = payloads[0]["byte_match"]
        percent = payloads[0]["instruction_count"]["match_percent"]
        if (
            metadata["status"] != ("exact" if exact else "partial")
            or metadata["match"] != f"{percent:.2f}"
        ):
            raise ValueError("reconstruction metadata measurement differs")
        if (
            not isinstance(metadata["residual"], str)
            or not metadata["residual"].strip()
            or (exact and metadata["residual"] != "none")
            or (not exact and (metadata["residual"] == "none" or percent == 100))
        ):
            raise ValueError("reconstruction residual is inconsistent")
    return proposal


def capture(root: Path, manifest: dict, runner) -> dict:
    from harness.common.checks import _check

    proposal = _read_proposal(root, manifest["request"]["proposal"])
    preservation_checks = copy.deepcopy(manifest["required_checks"])
    for member in proposal["members"]:
        function = next(
            (
                f
                for f in manifest["affected_functions"]
                if f["selector"] == member["selector"]
            ),
            None,
        )
        if function is None:
            raise ValueError("unknown reconstruction member")
        if count_function_metadata(read_file(root, function["source"]).decode()) != 1:
            raise ValueError("reconstruction excludes grouped sources")
        for tool in ("asm-diff", "byte-match"):
            check = _check(
                tool,
                function["target"],
                function["selector"],
                function["function"],
                function["source"],
            )
            check["argv"].append("--image-evidence")
            result = runner(check["argv"], cwd=root, text=True, capture_output=True)
            payload = json.loads(result.stdout, object_pairs_hook=unique_object)
            expected = member["pre"][tool]
            if (
                result.returncode != expected["exit_code"]
                or comparison_digest(payload, root) != expected["identity_digest"]
            ):
                raise ValueError("independent live reconstruction PRE differs")
            for existing in manifest["required_checks"]:
                if (
                    existing["selector"] == member["selector"]
                    and tool_id(existing["argv"]) == tool
                ):
                    existing.pop("partial_baseline", None)
                    existing["argv"].append("--image-evidence")
                    existing["command"] = " ".join(existing["argv"])
                    existing["reconstruction"] = {
                        "expected": member["expected"][tool],
                        "proposal_digest": proposal["digest"],
                    }
    manifest["expected_post_state"] = {**proposal["pre_state"], **proposal["images"]}
    manifest["reconstruction"] = {
        "proposal": proposal,
        "reference": manifest["request"]["proposal"],
        "preservation_checks": preservation_checks,
    }
    validate_policy(root, manifest)
    return manifest["reconstruction"]


def validate_application(value: dict) -> None:
    manifest = value["manifest"]
    if manifest["schema"] != MANIFEST_SCHEMA:
        return
    proposal = manifest["reconstruction"]["proposal"]
    expected = {**proposal["pre_state"], **proposal["images"]}
    if (
        value["post_state"] != expected
        or value["changed_paths"] != sorted(proposal["images"])
        or value.get("target") != manifest["target"]
        or value.get("concern") != "reconstruction"
    ):
        raise ValueError("reconstruction application image binding differs")


def validate_changes(root: Path, manifest: dict, changes: dict) -> None:
    proposal = validate_policy(root, manifest)
    if not isinstance(changes, dict) or set(changes) != set(proposal["images"]):
        raise ValueError("reconstruction changes differ from pinned images")
    selected = {m["selector"]: m for m in proposal["members"]}
    for name, text in changes.items():
        if (
            not isinstance(text, str)
            or hashlib.sha256(text.encode()).hexdigest() != proposal["images"][name]
        ):
            raise ValueError("reconstruction replacement image drifted")
        if not name.endswith(".c"):
            continue
        old = functions.parse_function_records(read_file(root, name).decode())
        new = functions.parse_function_records(text)
        function = next(
            f for f in manifest["affected_functions"] if f["source"] == name
        )
        member = selected.get(function["selector"])
        if member is None or len(old) != 1 or len(new) != 1:
            raise ValueError("unselected or grouped reconstruction source edit")
        before, after = old[0], new[0]
        if (
            before.address != after.address
            or before.spelling != after.spelling
            or before.kind != after.kind
        ):
            raise ValueError("reconstruction source identity differs")
        metadata = member["metadata"]
        if parse_progress_tags(after.metadata) != (
            metadata["status"],
            float(metadata["match"]),
            metadata["residual"],
        ):
            raise ValueError("reconstruction source metadata differs")

        def without_progress(value: str) -> str:
            value = re.sub(r"@residual\b(?:(?!@\w|\*/).)*", "", value, flags=re.DOTALL)
            return re.sub(r"@(?:status|match)\b[^\n]*", "", value)

        if without_progress(before.metadata) != without_progress(after.metadata):
            raise ValueError("reconstruction changed unrelated metadata")
        old_text = read_file(root, name).decode()
        if (
            old_text[: before.metadata_start] != text[: after.metadata_start]
            or old_text[before.metadata_end : before.implementation_start]
            != text[after.metadata_end : after.implementation_start]
            or old_text[before.implementation_end :] != text[after.implementation_end :]
        ):
            raise ValueError("reconstruction changed unrelated source context")
        old_signature = old_text[
            before.implementation_start : before.implementation_end
        ].split("{", 1)[0]
        new_signature = text[
            after.implementation_start : after.implementation_end
        ].split("{", 1)[0]
        if old_signature != new_signature:
            raise ValueError("reconstruction changed the function ABI declaration")
