"""Frozen-five consumer checks; synthetic proof stubs are not live acceptance."""

from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path
from types import SimpleNamespace

import pytest
from harness.analysis import frozen_queue_accounting as accounting
from harness.domain import receipts
from harness.naming import namespace as evidence_namespace


def _write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value))
    return hashlib.sha256(path.read_bytes()).hexdigest()


@pytest.fixture
def queue(tmp_path, monkeypatch):
    root = tmp_path
    entries = [
        {"id": identity, "route": route, "query": query}
        for identity, route, query in zip(
            accounting.IDS,
            [
                "naming prepare/postapply/verify",
                "naming evidence/validation",
                "type-audit",
                "type-audit",
                "macro-audit",
            ],
            [
                [{"selector": "function"}],
                [{"selector": "data"}],
                {"id": accounting.IDS[2]},
                {"id": accounting.IDS[3]},
                {
                    "id": accounting.IDS[4],
                    "members": [{"function": f"{accounting.TARGET}@800b2218"}],
                },
            ],
        )
    ]
    entries[0]["report_row"] = "function:func_800A3638"
    entries[1]["report_row"] = "data:D_80096994"
    entries[4]["cross_target_report_only"] = copy.deepcopy(entries[4]["query"])
    report = root / accounting.REPORT
    frozen = {
        "entries": entries,
        "report_sha256": _write(report, {"complete": True}),
        "index_sha256": _write(accounting.index.index_path(root), {}),
        "fingerprints": {
            accounting.TARGET: {
                "source_inputs": {"src/test.c": _write(root / "src/test.c", "source")},
                "indexed_target": {
                    "binary": "image.bin",
                    "binary_sha256": _write(root / "image.bin", "image"),
                    "snapshot": "snapshot.json",
                    "snapshot_sha256": _write(root / "snapshot.json", {}),
                },
            }
        },
    }
    pilot = root / "pilot.json"
    pin = _write(pilot, frozen)
    monkeypatch.setattr(accounting, "PILOT_SHA256", pin)
    monkeypatch.setattr(
        accounting.index, "connect", lambda root: SimpleNamespace(close=lambda: None)
    )
    monkeypatch.setattr(
        accounting, "load_target_manifests", lambda root: {accounting.TARGET: None}
    )
    monkeypatch.setattr(
        accounting,
        "describe_payload",
        lambda connection, function, **kwargs: entries[
            0 if function.address == 0x800A3638 else 1
        ]["query"],
    )
    monkeypatch.setattr(
        accounting,
        "type_candidates_payload",
        lambda *args, **kwargs: [e["query"] for e in entries[2:4]],
    )
    monkeypatch.setattr(
        accounting,
        "macro_opportunities_payload",
        lambda *args, **kwargs: [entries[4]["query"]],
    )
    calls = []

    def full(*args, **kwargs):
        assert kwargs["transaction"] is None
        assert evidence_namespace.selected_evidence_root() == root / "evidence"
        assert receipts._RECEIPT_ROOT.get() == root / "evidence"
        calls.append(True)
        raise ValueError("unfiltered FUNCTION binding blocker")

    monkeypatch.setattr(accounting.audit, "validate", full)
    records = [
        {
            "id": identity,
            "claim": "blocked",
            "proof": None,
            "blocker": {
                "owner": "parent",
                "reason": "unaccepted owner evidence",
                "next_action": "separate scoped review",
            },
        }
        for identity in accounting.IDS
    ]

    def run():
        return accounting.account_frozen_queue(
            root,
            pilot,
            pin,
            records=records,
            report_path=report,
            evidence_root=root / "evidence",
        )

    return SimpleNamespace(
        root=root,
        pilot=pilot,
        pin=pin,
        report=report,
        entries=entries,
        records=records,
        run=run,
        calls=calls,
    )


def test_five_blocked_accounted_not_completed_and_dual_root_restored(queue):
    namespace = evidence_namespace.set_evidence_root(queue.root / "previous")
    receipt = receipts.set_receipt_root(queue.root / "previous-receipts")
    try:
        result = queue.run()
        assert result["accounted"] is True
        assert result["counts"] == {
            "accepted": 0,
            "noop": 0,
            "blocked": 5,
            "historical_skip": 1,
        }
        assert [e["id"] for e in result["entries"]] == list(accounting.IDS)
        assert [e["historical_skip"] for e in result["entries"]] == [
            True,
            False,
            False,
            False,
            False,
        ]
        assert result["overlap_groups"][0]["entries"] == list(accounting.IDS[2:4])
        assert result["full_report"] == {
            "complete": False,
            "blocker": "unfiltered FUNCTION binding blocker",
        }
        assert not result["campaign_complete"] and not result["production_complete"]
        assert queue.calls == [True]
        assert evidence_namespace.selected_evidence_root() == queue.root / "previous"
        assert receipts._RECEIPT_ROOT.get() == queue.root / "previous-receipts"
    finally:
        receipts.reset_receipt_root(receipt)
        evidence_namespace.reset_evidence_root(namespace)


@pytest.mark.parametrize(
    "mutation",
    [
        "missing",
        "duplicate",
        "extra-id",
        "wrong-selector",
        "unknown-field",
        "claim",
        "empty-blocker",
        "blocked-proof",
    ],
)
def test_closed_record_membership(queue, mutation):
    row = queue.records[0]
    if mutation == "missing":
        queue.records.pop()
    elif mutation == "duplicate":
        queue.records[-1] = row
    elif mutation == "extra-id":
        row["id"] = "exe/other@800A3638"
    elif mutation == "wrong-selector":
        row["id"] = row["id"].replace("@", "@0x")
    elif mutation == "unknown-field":
        row["accepted"] = True
    elif mutation == "claim":
        row["claim"] = "done"
    elif mutation == "empty-blocker":
        row["blocker"]["owner"] = " "
    else:
        row["proof"] = {}
    with pytest.raises(ValueError):
        queue.run()


@pytest.mark.parametrize(
    "relative",
    [
        "pilot.json",
        accounting.REPORT,
        "out/index/reverse.sqlite",
        "src/test.c",
        "image.bin",
        "snapshot.json",
    ],
)
@pytest.mark.parametrize("during", [False, True])
def test_frozen_bytes_before_and_after(queue, monkeypatch, relative, during):
    def mutate(*args, **kwargs):
        (queue.root / relative).write_text("drift")
        return {"complete": True}

    if during:
        monkeypatch.setattr(accounting.audit, "validate", mutate)
    else:
        mutate()
    with pytest.raises(ValueError, match="drift|pinned"):
        queue.run()
    assert evidence_namespace.selected_evidence_root() is None
    assert receipts._RECEIPT_ROOT.get() is None


def _function(queue, monkeypatch):
    bundle = queue.root / "bundle.json"
    pin = _write(bundle, {"synthetic": True})
    queue.records[0].update(
        claim="accepted",
        proof={"bundle": bundle, "expected_bundle_sha256": pin},
        blocker=None,
    )
    result = {
        "applied": True,
        "rows": 1,
        "target": accounting.TARGET,
        "transaction": "function:func_800A3638",
    }

    def verify(root, target, report, transaction, **kwargs):
        assert root == queue.root and target == accounting.TARGET
        assert transaction == result["transaction"]
        assert kwargs == {"report_path": queue.report, "post_apply_receipts": bundle}
        return result

    monkeypatch.setattr(accounting.audit, "verify", verify)
    return result


def test_function_historical_skip_and_fullreport_are_separate(queue, monkeypatch):
    _function(queue, monkeypatch)
    result = queue.run()
    assert result["counts"] == {
        "accepted": 1,
        "noop": 0,
        "blocked": 4,
        "historical_skip": 1,
    }
    assert result["entries"][0]["historical_skip"]
    assert not result["full_report"]["complete"] and not result["campaign_complete"]


@pytest.mark.parametrize(
    "field,value",
    [
        ("target", "exe/other"),
        ("transaction", "function:wrong"),
        ("rows", True),
        ("applied", 1),
    ],
)
def test_function_boolean_is_not_bound_acceptance(queue, monkeypatch, field, value):
    result = _function(queue, monkeypatch)
    result[field] = value
    monkeypatch.setattr(accounting.audit, "verify", lambda *args, **kwargs: result)
    with pytest.raises(ValueError, match="mismatched"):
        queue.run()


@pytest.mark.parametrize(
    "mutation", ["pin", "relative", "symlink", "unknown", "duplicate-json", "during"]
)
def test_positive_proof_boundary(queue, monkeypatch, mutation):
    result = _function(queue, monkeypatch)
    proof = queue.records[0]["proof"]
    if mutation == "pin":
        proof["expected_bundle_sha256"] = "0" * 64
    elif mutation == "relative":
        proof["bundle"] = Path("bundle.json")
    elif mutation == "symlink":
        alias = queue.root / "alias"
        alias.symlink_to(proof["bundle"])
        proof["bundle"] = alias
    elif mutation == "unknown":
        proof["accepted"] = True
    elif mutation == "duplicate-json":
        proof["bundle"].write_text('{"accepted":false,"accepted":true}')
        proof["expected_bundle_sha256"] = hashlib.sha256(
            proof["bundle"].read_bytes()
        ).hexdigest()
        # Exercise the existing owner's strict parser, not a permissive success stub.
        monkeypatch.setattr(
            accounting.audit,
            "verify",
            lambda *args, **kwargs: accounting.load(proof["bundle"]),
        )
    else:

        def changed(*args, **kwargs):
            proof["bundle"].write_text("changed")
            return result

        monkeypatch.setattr(accounting.audit, "verify", changed)
    with pytest.raises(ValueError):
        queue.run()


@pytest.mark.parametrize("position,claim", [(0, "noop"), (1, "accepted"), (4, "noop")])
def test_unsupported_terminal_routes(queue, position, claim):
    queue.records[position].update(claim=claim, proof={}, blocker=None)
    with pytest.raises(ValueError, match="unsupported"):
        queue.run()


def test_selected_terminal_noop_does_not_hide_full_report(queue, monkeypatch):
    parent = queue.root / "parent.json"
    _write(parent, {})
    proof = {"parent": parent, "expected_parent_digest": "a" * 64}
    queue.records[1].update(claim="noop", proof=proof, blocker=None)

    def verify(root, target, report, transaction, path, pin):
        assert (root, target, report, transaction, path, pin) == (
            queue.root,
            accounting.TARGET,
            queue.report,
            "data:D_80096994",
            parent,
            "a" * 64,
        )
        return {
            "selected_row_accepted": True,
            "target": target,
            "transaction": transaction,
            "parent_digest": pin,
        }

    monkeypatch.setattr(accounting.terminal_review, "verify_terminal", verify)
    result = queue.run()
    assert result["counts"]["noop"] == 1 and not result["full_report"]["complete"]


def _pair(queue, monkeypatch):
    manifest = {
        "target": accounting.TARGET,
        "targets": [accounting.TARGET],
        "request": {"index_candidate_ids": list(accounting.IDS[2:4])},
        "reviewed_candidates": [
            {
                "index_id": identity,
                "candidate": {"target": accounting.TARGET, "address": 0x1F800044},
            }
            for identity in accounting.IDS[2:4]
        ],
    }
    envelope = queue.root / "envelope.json"
    _write(envelope, {"application": {"manifest": manifest}})
    proof = {"envelope": envelope, "expected_envelope_digest": "v1:" + "a" * 64}
    for row in queue.records[2:4]:
        row.update(claim="accepted", proof=proof.copy(), blocker=None)
    monkeypatch.setattr(
        accounting.type_application_review,
        "verify_reviewed_application",
        lambda root, value, pin: {
            "accepted": True,
            "target": accounting.TARGET,
            "digest": pin,
        },
    )
    return manifest, envelope


def test_linked_pair_is_two_entries_one_group(queue, monkeypatch):
    _pair(queue, monkeypatch)
    result = queue.run()
    assert result["counts"]["accepted"] == 2 and result["counts"]["blocked"] == 3
    assert len(result["overlap_groups"]) == 1
    assert result["overlap_groups"][0]["outcome"] == "accepted"


@pytest.mark.parametrize(
    "mutation",
    [
        "separate",
        "one",
        "missing-id",
        "wrong-target",
        "wrong-address",
        "expanded-targets",
    ],
)
def test_linked_type_binding_rejects(queue, monkeypatch, mutation):
    manifest, envelope = _pair(queue, monkeypatch)
    if mutation == "separate":
        other = queue.root / "other.json"
        other.write_bytes(envelope.read_bytes())
        queue.records[3]["proof"]["envelope"] = other
    elif mutation == "one":
        queue.records[3].update(
            claim="blocked",
            proof=None,
            blocker={"owner": "parent", "reason": "blocked", "next_action": "review"},
        )
    elif mutation == "missing-id":
        manifest["reviewed_candidates"].pop()
    elif mutation == "wrong-target":
        manifest["reviewed_candidates"][0]["candidate"]["target"] = "exe/other"
    elif mutation == "wrong-address":
        manifest["reviewed_candidates"][0]["candidate"]["address"] += 4
    else:
        manifest["targets"].append("exe/other")
    _write(envelope, {"application": {"manifest": manifest}})
    with pytest.raises(ValueError):
        queue.run()


def test_unfiltered_nonboolean_complete_rejects_completion(queue, monkeypatch):
    monkeypatch.setattr(
        accounting.audit, "validate", lambda *args, **kwargs: {"complete": "true"}
    )
    assert queue.run()["full_report"]["complete"] is False


@pytest.mark.parametrize(
    "mutation", [None, "fingerprint", "member", "owner", "target", "id", "verifier"]
)
def test_macro_verified_envelope_binds_global_fingerprint_selected_members(
    queue, monkeypatch, mutation
):
    entry = queue.entries[4]
    # The actual owner prepares from global accounting, not the selected subset.
    entry["cross_target_report_only"] = {
        **entry["query"],
        "target_scope": "cross_target",
    }
    frozen = accounting.load(queue.pilot)
    frozen["entries"][4] = entry
    queue.pin = _write(queue.pilot, frozen)
    monkeypatch.setattr(accounting, "PILOT_SHA256", queue.pin)
    monkeypatch.setattr(
        accounting,
        "macro_opportunities_payload",
        lambda *args, **kwargs: [
            entry["query"] if kwargs["target"] else entry["cross_target_report_only"]
        ],
    )
    source = queue.root / "src/test.c"
    monkeypatch.setattr(
        accounting, "resolve_function", lambda *args: SimpleNamespace(source=source)
    )
    monkeypatch.setattr(accounting, "manifest_header_paths", lambda *args: [])
    manifest = {
        "target": accounting.TARGET,
        "targets": [accounting.TARGET],
        "reviewed_opportunity": {
            "candidate_id": entry["id"],
            "candidate_fingerprint": accounting.digest(
                entry["cross_target_report_only"]
            ),
            "declared_targets": [accounting.TARGET],
            "owners": ["src/test.c"],
        },
        "affected_functions": [{"selector": f"{accounting.TARGET}@0x800B2218"}],
        "allowed_paths": ["src/test.c"],
    }
    if mutation == "fingerprint":
        manifest["reviewed_opportunity"]["candidate_fingerprint"] = accounting.digest(
            entry["query"]
        )
    elif mutation == "member":
        manifest["affected_functions"].append(
            {"selector": "emi/etc/game/00@0x801996FC"}
        )
    elif mutation == "owner":
        manifest["reviewed_opportunity"]["owners"] = ["src/other.c"]
    elif mutation == "target":
        manifest["targets"].append("emi/etc/game/00")
    elif mutation == "id":
        manifest["reviewed_opportunity"]["candidate_id"] = "exact_group:other"
    envelope = queue.root / "macro.json"
    _write(envelope, {"application": {"manifest": manifest}})
    pin = "v1:" + "a" * 64
    queue.records[4].update(
        claim="accepted",
        proof={"envelope": envelope, "expected_envelope_digest": pin},
        blocker=None,
    )
    monkeypatch.setattr(
        accounting.macro_application_review,
        "verify_reviewed_application",
        lambda root, value, pin: {
            "accepted": mutation != "verifier",
            "target": accounting.TARGET,
            "digest": pin,
        },
    )

    def run():
        return accounting.account_frozen_queue(
            queue.root,
            queue.pilot,
            queue.pin,
            records=queue.records,
            report_path=queue.report,
            evidence_root=queue.root / "evidence",
        )

    if mutation:
        with pytest.raises(ValueError):
            run()
    else:
        assert run()["counts"]["accepted"] == 1


def test_actual_owner_rejects_malformed_positive_envelope(queue, monkeypatch):
    from harness.types import application as type_application_review

    verifier = type_application_review.verify_reviewed_application
    _, envelope = _pair(queue, monkeypatch)
    monkeypatch.setattr(
        type_application_review, "verify_reviewed_application", verifier
    )
    envelope.write_text('{"accepted": true}')
    with pytest.raises(ValueError):
        queue.run()


def test_full_report_cannot_change_previously_verified_reference(queue, monkeypatch):
    _function(queue, monkeypatch)

    def changed(*args, **kwargs):
        queue.records[0]["proof"]["bundle"].write_text("changed after verification")
        return {"complete": True}

    monkeypatch.setattr(accounting.audit, "validate", changed)
    with pytest.raises(ValueError, match="during accounting"):
        queue.run()


@pytest.mark.parametrize("position", [0, 1, 2, 4])
def test_actual_owners_reject_malformed_positive_references(queue, position):
    path = queue.root / "malformed.json"
    pin = _write(path, {"accepted": True})
    if position == 0:
        proof, claim = {"bundle": path, "expected_bundle_sha256": pin}, "accepted"
    elif position == 1:
        proof, claim = {"parent": path, "expected_parent_digest": pin}, "noop"
    else:
        proof, claim = (
            {"envelope": path, "expected_envelope_digest": "v1:" + pin},
            "accepted",
        )
    queue.records[position].update(claim=claim, proof=proof, blocker=None)
    if position == 2:
        queue.records[3].update(claim=claim, proof=proof.copy(), blocker=None)
    with pytest.raises(ValueError):
        queue.run()
