"""Shared PRE integration: real sequential owners/Ninja, synthetic BOF3 evidence."""

import copy
import hashlib
import json
import sqlite3

import pytest
import test_macro_transactions as macros
import test_type_transactions as types
from harness.common.digests import digest


def write_json(path, value):
    value["digest"] = digest({k: v for k, v in value.items() if k != "digest"})
    path.write_text(json.dumps(value))


def prepare_inputs(root, owner, proposal):
    (root / "include/shared.h").write_text("/* original shared dependency */\n")
    for target in ("test", "second"):
        path = root / f"src/{target}/func_80100000.c"
        path.write_text('#include "shared.h"\n' + path.read_text())
    if owner == "type":
        connection = types._connect(root)
        connection.execute("UPDATE type_candidates SET kind='aggregate_region'")
        connection.commit()
        connection.close()
        path = root / proposal["candidate_artifacts"][0]
        candidate = json.loads(path.read_text())
        candidate["candidate"]["kind"] = "aggregate"
        candidate["candidate"]["id"] = "aggregate:exe/test@80100000"
        connection = types._connect(root)
        row = dict(
            connection.execute(
                "SELECT * FROM type_candidates WHERE target_id='exe/test'"
            ).fetchone()
        )
        connection.close()
        row["evidence"] = json.loads(row["evidence"])
        candidate["index_row_digest"] = digest(row)
        proposal["concern"] = "layout"
        write_json(path, candidate)
    else:
        connection = sqlite3.connect(root / "index.sqlite")
        # Synthetic reviewed-byte registry; native owner queries remain real.
        connection.execute(
            "INSERT INTO duplicate_groups VALUES (?,8,2,0,2,'exe/test@80100000','executable',2,'[]',0)",
            ("b" * 64,),
        )
        for target in ("test", "second"):
            connection.execute(
                "INSERT INTO duplicate_members VALUES (?,8,?,'exact',?,'func_80100000',1)",
                ("b" * 64, f"exe/{target}@80100000", f"src/{target}/func_80100000.c"),
            )
        for target in ("test", "second"):
            path = f"src/{target}/func_80100000.c"
            connection.execute(
                "UPDATE macro_input_fingerprints SET sha256=? WHERE source_path=?",
                (hashlib.sha256((root / path).read_bytes()).hexdigest(), path),
            )
        connection.commit()
        connection.close()
        report = macros.macro_accounting.candidate_account(root)
        # Actual indexed exact group, not a forged macro account or approval.
        report["rows"] = [row for row in report["rows"] if row["kind"] == "exact_group"]
        assert report["rows"]
        proposal["candidate_artifact"] = macros._artifact(
            root, report, "local_template"
        )
        proposal["concern"] = "local_template"
    return proposal


def shared_pre_checks(root, owner, transactions, review, accepted, runner, calls):
    request = {
        "schema": transactions.REQUEST_SCHEMA,
        "target": "exe/test",
        "shared_targets": ["exe/second", "exe/test"],
        "affected_functions": [
            f"exe/{target}@0x80100000" for target in ("second", "test")
        ],
        "adopted_baseline": transactions.workspace_baseline(root)["digest"],
    }
    pins = []
    for number, envelope in enumerate(accepted):
        target = envelope["revalidation"]["manifest"]["target"]
        name = f"out/reviews/fresh-{number}.json"
        (root / name).write_text(json.dumps(envelope))
        pins.append(
            {
                "path": name,
                "target": target,
                "expected_envelope_digest": envelope["digest"],
            }
        )
    if owner == "type":
        request.update(
            concern="shared",
            header="include/shared.h",
            private_transaction_proofs=pins,
            candidate_artifacts=[],
            index_candidate_ids=[],
        )
        for envelope in sorted(
            accepted, key=lambda e: e["revalidation"]["manifest"]["target"]
        ):
            original = envelope["revalidation"]["prerequisite"]["application"][
                "manifest"
            ]
            candidate = json.loads(
                (root / original["request"]["candidate_artifacts"][0]).read_text()
            )
            candidate["candidate"].update(
                owners=["include/shared.h"],
                locations=["include/shared.h"],
                fingerprints={
                    "include/shared.h": hashlib.sha256(
                        (root / "include/shared.h").read_bytes()
                    ).hexdigest()
                },
            )
            name = f"out/reviews/shared-{len(request['candidate_artifacts'])}.json"
            write_json(root / name, candidate)
            request["candidate_artifacts"].append(name)
            request["index_candidate_ids"].append(candidate["index_id"])
    else:
        for pin in pins:
            pin["selector"] = pin["target"] + "@0x80100000"
        report = macros.macro_accounting.candidate_account(root)
        report["rows"] = [row for row in report["rows"] if row["kind"] == "exact_group"]
        # Avoid overwriting original private proposal evidence.
        original = root / "out/reviews/macro-opportunity.json"
        before = original.read_bytes()
        name = macros._artifact(root, report, "shared_template", ["include/shared.h"])
        (root / "out/reviews/shared-macro.json").write_bytes((root / name).read_bytes())
        original.write_bytes(before)
        request.update(
            concern="shared_template",
            exact_function_proofs=pins,
            candidate_artifact="out/reviews/shared-macro.json",
        )
    manifest = transactions.prepare_transaction(root, request)
    assert manifest["shared_pre"][
        "workspace_baseline"
    ] == transactions.workspace_baseline(root)
    frozen = copy.deepcopy(accepted)
    start = len(calls)
    for options in (
        {},
        {"implementation_run_id": "shared"},
        {"implementation_run_id": "shared", "participating_targets": ["exe/test"]},
    ):
        with pytest.raises(ValueError):
            transactions.run_transaction(
                root, manifest, {"include/shared.h": "bad"}, runner=runner, **options
            )
        assert len(calls) == start
    for name in (
        pins[0]["path"],
        accepted[0]["revalidation"]["receipts"][0]["path"],
        "include/second.h",
        "unrelated.txt",
        "build/cmake/build.ninja",
    ):
        path = root / name
        before = path.read_bytes()
        path.write_bytes(before + b"\nchanged\n")
        with pytest.raises(ValueError):
            transactions.run_transaction(
                root,
                manifest,
                {"include/shared.h": "bad"},
                runner=runner,
                implementation_run_id="shared",
                participating_targets=request["shared_targets"],
            )
        assert len(calls) == start
        assert (
            root / "include/shared.h"
        ).read_text() == "/* original shared dependency */\n"
        path.write_bytes(before)
    from pathlib import Path

    from harness.common.execution import _evidence_paths, capture

    # Every nested original/intervening/fresh reviewer artifact stays mandatory.
    def artifacts(value):
        if isinstance(value, dict):
            if "review_artifact" in value:
                yield Path(value["review_artifact"]["path"])
            for item in value.values():
                yield from artifacts(item)
        elif isinstance(value, list):
            for item in value:
                yield from artifacts(item)

    for path in set(artifacts(accepted)):
        before = path.read_bytes()
        for replacement in (None, b"replaced reviewer evidence"):
            if replacement is None:
                path.unlink()
            else:
                path.write_bytes(replacement)
            with pytest.raises(ValueError):
                transactions.prepare_transaction(root, request)
            with pytest.raises(ValueError):
                transactions.run_transaction(
                    root,
                    manifest,
                    {"include/shared.h": "bad"},
                    runner=runner,
                    implementation_run_id="shared",
                    participating_targets=request["shared_targets"],
                )
            assert len(calls) == start
            path.write_bytes(before)
    # No blanket absolute-path exclusion, even under an arbitrary review_artifact key.
    alien = {
        "review_artifact": {"path": str(root / "include/shared.h"), "sha256": "0" * 64}
    }
    assert str(root / "include/shared.h") in _evidence_paths(alien)
    with pytest.raises(ValueError, match="noncanonical repository input"):
        capture(root, {**manifest, "alien": alien}, request["shared_targets"])
    for field in ("shared_pre",):
        broken = copy.deepcopy(manifest)
        del broken[field]
        broken["digest"] = digest({k: v for k, v in broken.items() if k != "digest"})
        with pytest.raises(ValueError):
            transactions.run_transaction(
                root,
                broken,
                {"include/shared.h": "bad"},
                runner=runner,
                implementation_run_id="shared",
                participating_targets=request["shared_targets"],
            )
        assert len(calls) == start
    bad = copy.deepcopy(request)
    bad_pins = bad[
        "private_transaction_proofs" if owner == "type" else "exact_function_proofs"
    ]
    bad_pins[0]["expected_envelope_digest"] = "v1:" + "0" * 64
    with pytest.raises(ValueError):
        transactions.prepare_transaction(root, bad)
    # A valid original envelope cannot replace one freshly accepted participant.
    first_path = root / pins[0]["path"]
    retained = first_path.read_bytes()
    mixed = copy.deepcopy(request)
    mixed_pins = mixed[
        "private_transaction_proofs" if owner == "type" else "exact_function_proofs"
    ]
    original = accepted[0]["revalidation"]["prerequisite"]
    first_path.write_text(json.dumps(original))
    mixed_pins[0]["expected_envelope_digest"] = original["digest"]
    with pytest.raises(ValueError):
        transactions.prepare_transaction(root, mixed)
    first_path.write_bytes(retained)
    # Rehashing a nested false reviewer pin cannot repair retained evidence.
    tampered = copy.deepcopy(accepted[0])
    tampered["parent_review"]["review_artifact"]["sha256"] = "0" * 64
    write_json(first_path, tampered)
    tampered_request = copy.deepcopy(request)
    tampered_request[
        "private_transaction_proofs" if owner == "type" else "exact_function_proofs"
    ][0]["expected_envelope_digest"] = tampered["digest"]
    with pytest.raises(ValueError):
        transactions.prepare_transaction(root, tampered_request)
    first_path.write_bytes(retained)
    options = {
        "implementation_run_id": "shared",
        "participating_targets": request["shared_targets"],
    }

    # Every native gate and publication failure restores owned bytes/modes/index.
    from unittest.mock import patch

    from harness.common import execution
    from harness.common.review import PRESERVATION, _binding

    header = root / "include/shared.h"
    baseline = (
        header.read_bytes(),
        header.stat().st_mode,
        (root / ".git/index").read_bytes(),
    )
    for failed in range(len(manifest["required_checks"])):
        count = 0

        def fail(argv, **kwargs):
            nonlocal count
            result = runner(argv, **kwargs)
            if count == failed:
                result.returncode = 1
            count += 1
            return result

        with pytest.raises(RuntimeError):
            transactions.run_transaction(
                root,
                manifest,
                {"include/shared.h": "/* attempted */\n"},
                runner=fail,
                **options,
            )
        assert count == failed + 1
        assert (
            header.read_bytes(),
            header.stat().st_mode,
            (root / ".git/index").read_bytes(),
        ) == baseline
    with patch.object(
        execution,
        "write_evidence_output",
        side_effect=RuntimeError("publication failed"),
    ):
        with pytest.raises(RuntimeError, match="publication failed"):
            transactions.run_transaction(
                root,
                manifest,
                {"include/shared.h": "/* attempted */\n"},
                runner=runner,
                output="out/reviews/evidence/shared-failed.json",
                **options,
            )
    assert (
        header.read_bytes(),
        header.stat().st_mode,
        (root / ".git/index").read_bytes(),
    ) == baseline
    application = transactions.run_transaction(
        root,
        manifest,
        {"include/shared.h": "/* shared POST */\n"},
        runner=runner,
        output="out/reviews/evidence/shared-application.json",
        **options,
    )
    assert application["applied"]
    artifact = root / "out/reviews/shared-review.txt"
    artifact.write_text(
        "Synthetic independent shared transition acceptance; not BOF3 approval.\n"
    )
    parent = {
        "schema": f"bof3.{owner}-parent-review/v1",
        "accepted": True,
        "parent_run_id": "shared-parent",
        "implementation_run_id": "shared",
        "reviewer_run_id": "shared-reviewer",
        "review_artifact": {
            "path": str(artifact),
            "sha256": hashlib.sha256(artifact.read_bytes()).hexdigest(),
        },
        "binding": _binding(application),
        "preservation": PRESERVATION,
    }
    envelope = review.review_application(
        root, application, parent, application["digest"]
    )
    external_pin = envelope["digest"]
    start = len(calls)
    assert review.verify_reviewed_application(root, envelope, external_pin)["accepted"]
    assert review.verify_reviewed_application(root, envelope, external_pin)["accepted"]
    with pytest.raises(ValueError):
        review.verify_reviewed_application(root, envelope, "v1:" + "0" * 64)
    for private in accepted:
        with pytest.raises(ValueError):
            review.verify_reviewed_revalidation(root, private, private["digest"])
    for origin in (
        accepted[0]["parent_review"],
        accepted[0]["revalidation"]["prerequisite"]["parent_review"],
    ):
        for key in ("reviewer_run_id", "review_artifact"):
            bad = {**parent, key: origin[key]}
            with pytest.raises(ValueError):
                review.review_application(root, application, bad, application["digest"])
    for path in {
        *(root / p for p in _evidence_paths(manifest)),
        *artifacts(accepted),
        artifact,
        root / "include/shared.h",
        root / "include/second.h",
        root / "unrelated.txt",
        root / "build/cmake/build.ninja",
    }:
        before = path.read_bytes()
        path.write_bytes(before + b"\nchanged\n")
        with pytest.raises((ValueError, json.JSONDecodeError)):
            review.verify_reviewed_application(root, envelope, external_pin)
        path.write_bytes(before)
    from harness.common.revalidation import _verify_record

    for fault in ("receipt", "pre", "build", "baseline", "pin", "history", "scope"):
        record = copy.deepcopy(accepted[0]["revalidation"])
        if fault == "receipt":
            record["receipts"][0]["status"] = "failed"
        elif fault == "pre":
            record["review_context"]["initial_state"]["inputs"]["include/shared.h"] = (
                None
            )
        elif fault == "build":
            record["review_context"]["initial_build"] = {}
        elif fault == "baseline":
            record["review_context"]["adopted_baseline"]["state"] = {}
        elif fault == "pin":
            record["expected_envelope_digest"] = "v1:" + "0" * 64
        elif fault == "history":
            record.pop("intervening", None)
        else:
            record["review_context"]["initial_state"]["participating_targets"] = [
                "exe/test"
            ]
        record["digest"] = digest({k: v for k, v in record.items() if k != "digest"})
        with pytest.raises(ValueError):
            _verify_record(
                root,
                record,
                record["digest"],
                owner=owner,
                verify=None,
                historical=True,
                manifest_validator=transactions._manifest,
            )
    for name in ("include/shared.h", "include/second.h"):
        path = root / name
        mode = path.stat().st_mode
        path.chmod(0o750)
        with pytest.raises(ValueError):
            review.verify_reviewed_application(root, envelope, external_pin)
        path.chmod(mode)
    # Rehash-consistent nested edits cannot replace the retained externally pinned bytes.
    broken = copy.deepcopy(envelope)
    broken["application"]["manifest"]["shared_pre"]["build"] = {}
    broken["digest"] = digest({k: v for k, v in broken.items() if k != "digest"})
    with pytest.raises(ValueError):
        review.verify_reviewed_application(root, broken, broken["digest"])
    assert review.verify_reviewed_application(root, envelope, external_pin)["accepted"]
    assert len(calls) == start
    assert accepted == frozen


@pytest.mark.parametrize("owner", ["type", "macro"])
def test_shared_pre_from_two_fresh_private_owners(tmp_path, monkeypatch, owner):
    from test_private_application_transition import test_sequential_private_common_state

    test_sequential_private_common_state(
        tmp_path, monkeypatch, owner, None, True, shared=True
    )
