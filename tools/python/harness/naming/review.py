"""Explicit local parent review attestation for completed native naming gates."""

from __future__ import annotations

import copy
import json
from pathlib import Path

from harness.common.inputs import file_state
from harness.io import unique_object
from harness.naming.application import (
    _execute,
    directory,
    receipt,
    recheck,
    validate_bundle,
    write_json,
)
from harness.naming.inputs import digest
from harness.naming.inputs import keys
from harness.naming.inputs import load
from harness.common.inputs import relative
from harness.naming.inputs import transaction_paths

ATTESTATION_KEYS = "schema accepted implementation_run_id reviewer_run_id binding final_state_digest gates_digest review_artifact snapshot baseline_index_digest preservation"
PRESERVATION = {"scope": True, "body": True, "abi": True, "range": True, "index": True}


def _artifact(ref) -> Path:
    keys(ref, "path sha256")
    path = Path(ref["path"])
    if not path.is_absolute() or str(path.resolve()) != str(path):
        raise ValueError("attested artifact path must be canonical absolute")
    current = file_state(path)
    if current is None or current["sha256"] != ref["sha256"]:
        raise ValueError("attested artifact replaced")
    return path


def attestation(root: Path, bundle, row, value) -> None:
    keys(value, ATTESTATION_KEYS)
    origins = (value["implementation_run_id"], value["reviewer_run_id"])
    if (
        value["schema"] != "bof3.naming-parent-review/v1"
        or value["accepted"] is not True
        or any(
            not isinstance(x, str) or not x.strip() or x != x.strip() for x in origins
        )
        or origins[0] == origins[1]
        or origins[0] != bundle["implementation_run_id"]
    ):
        raise ValueError(
            "explicit accepted parent attestation with distinct origins required"
        )
    if (
        value["binding"] != bundle["binding"]
        or value["final_state_digest"] != digest(bundle["final_state"])
        or value["gates_digest"] != digest(bundle["gates"])
        or value["baseline_index_digest"] != bundle["index_digest"]
        or value["preservation"] != PRESERVATION
        or any(v is not True for v in value["preservation"].values())
    ):
        raise ValueError("parent attestation does not bind finalized preservation")
    review = _artifact(value["review_artifact"])
    if not review.read_bytes().strip():
        raise ValueError("accepted review artifact is empty")
    snapshot = _artifact(value["snapshot"])
    retained = load(snapshot)
    keys(retained, "files frozen")
    paths = transaction_paths(row)
    if not isinstance(retained["files"], list) or len(retained["files"]) != len(paths):
        raise ValueError("snapshot must cover exact transaction paths")
    seen = set()
    snapshot_files = {}
    for item in retained["files"]:
        keys(
            item,
            "path exists sha256 mode" if item.get("exists") is True else "path exists",
        )
        name = relative(item["path"])
        if name not in paths or name in seen or type(item["exists"]) is not bool:
            raise ValueError("snapshot path or absence is invalid")
        seen.add(name)
        snapshot_files[name] = item
        actual = file_state(snapshot.parent / name)
        expected = (
            {"sha256": item["sha256"], "mode": item["mode"]} if item["exists"] else None
        )
        if actual != expected:
            raise ValueError("retained snapshot bytes/mode/absence changed")
    facts = row["pre_apply"]["facts"]
    old, new = facts["scope"]["definition"], facts["destination"]
    if row["kind"] == "data":
        for name, item in snapshot_files.items():
            captured = facts["data"]["files"][name]
            if (
                item.get("exists") is not True
                or item.get("sha256") != captured["before"]
                or item.get("mode") != captured["mode"]
                or bundle["final_state"].get(name)
                != {"sha256": captured["after"], "mode": captured["mode"]}
            ):
                raise ValueError(
                    "snapshot does not bind data preapply and postapply images"
                )
    elif not snapshot_files[old]["exists"] or (
        old != new and snapshot_files[new]["exists"]
    ):
        raise ValueError("snapshot is not a preapply source migration baseline")
    for name, item in snapshot_files.items():
        destination = new if name == old else name
        if item["exists"] and (
            bundle["final_state"].get(destination) is None
            or bundle["final_state"][destination]["mode"] != item["mode"]
        ):
            raise ValueError("applied file mode differs from cleaner snapshot")
    binding = bundle["binding"]
    expected_frozen = {
        binding["report"]: binding["report_state"]["sha256"],
        **{
            p: bundle["final_state"][p]["sha256"]
            for p in row["pre_apply"]["facts"]["reviewed"]
        },
        ".git/index": retained["frozen"].get(".git/index"),
    }
    if retained["frozen"] != expected_frozen:
        raise ValueError("snapshot frozen bindings changed")
    # Legacy cleaner snapshot stores raw index SHA-256, while native state also
    # binds mode. Match its raw digest to the live index without changing it.
    import os
    import subprocess

    path = subprocess.run(
        ["git", "rev-parse", "--path-format=absolute", "--git-path", "index"],
        cwd=root,
        capture_output=True,
        check=True,
        timeout=120,
    ).stdout.rstrip(b"\n")
    if (
        file_state(Path(os.fsdecode(path)))["sha256"]
        != retained["frozen"][".git/index"]
    ):
        raise ValueError("staged index differs from cleaner snapshot")


def validate_review(root: Path, bundle, row):
    review = bundle["review"]
    keys(review, "attestation attestation_state receipt readiness")
    path = Path(review["attestation"])
    if file_state(path) != review["attestation_state"]:
        raise ValueError("parent attestation replaced")
    value = load(path)
    attestation(root, bundle, row, value)
    record = review["receipt"]
    keys(record, "command status target selector output receipt sha256")
    from harness.domain.receipts import command_records

    command_records([record], "parent independent review", root)
    if record["command"] != "independent review " + row["identity"][
        "selector"
    ] or record["output"] != digest(value):
        raise ValueError("review receipt does not bind parent attestation")
    readiness = review["readiness"]
    keys(readiness, "argv exit_code failure stdout stderr")
    import json

    if (
        readiness["argv"] != ["bin/analysis-readiness", bundle["binding"]["target"]]
        or readiness["exit_code"] != 0
        or readiness["failure"] is not None
        or json.loads(readiness["stdout"], object_pairs_hook=unique_object).get("ready")
        is not True
    ):
        raise ValueError("missing native readiness checkpoint")
    return record


def ingest(
    root: Path,
    target: str,
    report_path: Path,
    transaction: str,
    gates: Path,
    parent: Path,
) -> Path:
    _, row, bundle, _ = validate_bundle(root, target, report_path, transaction, gates)
    if bundle["review"] is not None:
        raise ValueError("native bundle already has a review")
    original = file_state(parent)
    value = load(parent)
    attestation(root, bundle, row, value)
    readiness = _execute(root, ["bin/analysis-readiness", target])
    hygiene = _execute(root, ["git", "diff", "--check"])
    output = directory(root)
    write_json(output / "readiness.json", readiness)
    write_json(output / "diff-check.json", hygiene)
    if hygiene["exit_code"] != 0 or hygiene["failure"] is not None:
        raise ValueError(f"diff hygiene failed; retained {output}")
    # Keep bundle beside its original gate execution files; review artifacts may
    # live in a new native directory, but no existing evidence is overwritten.
    retained = output / "parent-attestation.json"
    write_json(retained, value)
    bundle["review"] = {
        "attestation": str(retained),
        "attestation_state": file_state(retained),
        "readiness": readiness,
        "receipt": receipt(
            root,
            output / "review-receipt.json",
            "independent review " + row["identity"]["selector"],
            target,
            row["identity"]["selector"],
            digest(value),
        ),
    }
    validate_review(root, bundle, row)
    recheck(root, bundle, row)
    if file_state(parent) != original:
        raise ValueError("parent attestation changed during ingestion")
    destination = gates.resolve().parent / ("reviewed-" + output.name + ".json")
    write_json(destination, bundle)
    return destination


def verify_external(
    root: Path, target: str, report_path: Path, transaction: str, path: Path, validate
):
    original_bundle = file_state(path)
    report, row, bundle, records = validate_bundle(
        root, target, report_path, transaction, path
    )
    records.append(validate_review(root, bundle, row))
    copied = copy.deepcopy(report)
    selected = next(
        r for r in copied["rows"] if f"{r.get('kind')}:{r.get('name')}" == transaction
    )
    selected["post_apply_receipts"] = records
    result = validate(copied)
    recheck(root, bundle, row)
    validate_review(root, bundle, row)
    for argv in (["bin/analysis-readiness", target], ["git", "diff", "--check"]):
        check = _execute(root, argv)
        if (
            check["exit_code"] != 0
            or check["failure"] is not None
            or (
                argv[0] == "bin/analysis-readiness"
                and json.loads(check["stdout"]).get("ready") is not True
            )
        ):
            raise ValueError("final readiness/diff hygiene failed")
    validate_bundle(root, target, report_path, transaction, path)
    validate_review(root, bundle, row)
    recheck(root, bundle, row)
    if file_state(path) != original_bundle:
        raise ValueError("external bundle changed during final verification")
    return result
