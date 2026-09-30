"""Owner integrations for parent-reviewed applications, never synthetic acceptance pins."""

import copy
import hashlib
import json
import os

import pytest

from harness.common.commands import tool_id

import test_macro_transactions as macro_tests
import test_type_transactions as type_tests
from harness.common.digests import digest
from harness.macros import application as macro_application_review
from harness.macros import cli as macro_audit
from harness.types import application as type_application_review
from harness.types import cli as type_audit
from transaction_native_fixture import execution_inputs, native_build


@pytest.fixture(params=["type", "macro"])
def reviewed_run(tmp_path, monkeypatch, request):
    owner = request.param
    tests = type_tests if owner == "type" else macro_tests
    transactions = (
        type_tests.transactions if owner == "type" else macro_tests.macro_transactions
    )
    review = type_application_review if owner == "type" else macro_application_review
    if owner == "type":
        type_tests._repo(tmp_path, git=True)
        monkeypatch.setattr(transactions, "connect", type_tests._connect)
        proposal = type_tests._request(tmp_path)
    else:
        report = macro_tests._setup(tmp_path, monkeypatch, git=True)
        proposal = macro_tests._request(tmp_path, report)
    execution_inputs(tmp_path)
    with (tmp_path / ".gitignore").open("a") as stream:
        stream.write("\nbuild/\n")
    (tmp_path / "unrelated.txt").write_text("adopted dirty work\n")
    (tmp_path / "include/test.h").chmod(0o750)
    proposal["adopted_baseline"] = transactions.workspace_baseline(tmp_path)["digest"]
    manifest = transactions.prepare_transaction(tmp_path, proposal)
    native = tests._runner()

    def run(argv, **kwargs):
        if tool_id(argv) == "build":
            native_build(tmp_path)
        return native(argv, **kwargs)

    application = transactions.run_transaction(
        tmp_path,
        manifest,
        {"include/test.h": "/* reviewed replacement */\n"},
        runner=run,
        implementation_run_id="implementation-actual-run",
    )
    artifact = tmp_path / "out/reviews/accepted-review.md"
    artifact.parent.mkdir(parents=True, exist_ok=True)
    artifact.write_text(
        "Independent reviewer inspected actual application and native gates.\n"
    )
    context = application["review_context"]
    parent = {
        "schema": f"bof3.{owner}-parent-review/v1",
        "accepted": True,
        "parent_run_id": "supervising-parent-run",
        "implementation_run_id": "implementation-actual-run",
        "reviewer_run_id": "independent-reviewer-run",
        "review_artifact": {
            "path": str(artifact),
            "sha256": hashlib.sha256(artifact.read_bytes()).hexdigest(),
        },
        "binding": {
            "application_digest": application["digest"],
            "application_proof_digest": digest(application),
            "manifest_digest": manifest["digest"],
            "request_digest": digest(manifest["request"]),
            "review_context_digest": digest(context),
            "post_state_digest": application["post_state_digest"],
            "native_receipts_digest": digest(application["receipts"]),
            "adopted_baseline_digest": digest(context["adopted_baseline"]),
        },
        "preservation": dict.fromkeys(
            ("scope", "body", "abi", "range", "index", "adopted_baseline"), True
        ),
    }
    # pytest changes PYTEST_CURRENT_TEST between fixture setup and test call.
    captured_environment = os.environ["PYTEST_CURRENT_TEST"]
    return (
        tmp_path,
        owner,
        transactions,
        review,
        application,
        parent,
        captured_environment,
    )


def test_owner_parent_review_and_cli_final_verify(reviewed_run):
    root, owner, transactions, review, app, parent, environment = reviewed_run
    os.environ["PYTEST_CURRENT_TEST"] = environment
    assert transactions.verify_application(root, app, app["digest"])["applied"]
    envelope = review.review_application(root, app, parent, app["digest"])
    pin = envelope["digest"]
    assert review.verify_reviewed_application(root, envelope, pin)["accepted"] is True
    cli = type_audit if owner == "type" else macro_audit
    proof = root / "out/reviews/application.json"
    attestation = root / "out/reviews/parent.json"
    proof.write_text(json.dumps(app))
    attestation.write_text(json.dumps(parent))
    output = "out/reviews/evidence/reviewed.json"
    args = cli.build_parser().parse_args(
        [
            "--root",
            str(root),
            "review",
            str(proof),
            output,
            "--parent-attestation",
            str(attestation),
            "--expected-application-digest",
            app["digest"],
        ]
    )
    assert args.handler(args) == 0
    args = cli.build_parser().parse_args(
        [
            "--root",
            str(root),
            "final-verify",
            str(root / output),
            "--expected-envelope-digest",
            pin,
        ]
    )
    assert args.handler(args) == 0
    assert (root / "unrelated.txt").read_text() == "adopted dirty work\n"
    assert (root / "include/test.h").stat().st_mode & 0o7777 == 0o750
    # Review/final verification are read-only replay, not application execution.
    assert review.verify_reviewed_application(root, envelope, pin)["digest"] == pin


@pytest.mark.parametrize(
    "fault",
    [
        "self-review",
        "parent-self",
        "wrong-run",
        "rejected",
        "unknown",
        "binding",
        "preservation",
        "artifact",
        "empty-artifact",
        "wrong-owner",
        "legacy",
    ],
)
def test_owner_parent_review_rejects_invalid_acceptance(reviewed_run, fault):
    root, owner, transactions, review, app, parent, environment = reviewed_run
    os.environ["PYTEST_CURRENT_TEST"] = environment
    app, parent = copy.deepcopy(app), copy.deepcopy(parent)
    if fault == "self-review":
        parent["reviewer_run_id"] = parent["implementation_run_id"]
    elif fault == "parent-self":
        parent["parent_run_id"] = parent["reviewer_run_id"]
    elif fault == "wrong-run":
        parent["implementation_run_id"] = "invented-run"
    elif fault == "rejected":
        parent["accepted"] = False
    elif fault == "unknown":
        parent["binding"]["extra"] = True
    elif fault == "binding":
        parent["binding"]["manifest_digest"] = "0" * 64
    elif fault == "preservation":
        parent["preservation"]["adopted_baseline"] = 1
    elif fault in {"artifact", "empty-artifact"}:
        from pathlib import Path

        Path(parent["review_artifact"]["path"]).write_text(
            "replaced" if fault == "artifact" else ""
        )
    elif fault == "wrong-owner":
        parent["schema"] = (
            f"bof3.{'macro' if owner == 'type' else 'type'}-parent-review/v1"
        )
    else:
        del app["review_context"]
        app["digest"] = digest(
            {k: v for k, v in app.items() if k not in {"digest", "attestation"}}
        )
        kwargs = (
            {}
            if owner == "type"
            else {"schema": transactions.ATTESTATION_SCHEMA, "prefix": "macro"}
        )
        app["attestation"] = transactions.write_attestation(root, app, **kwargs)
        assert transactions.verify_application(root, app, app["digest"])["applied"]
    with pytest.raises(ValueError):
        review.review_application(root, app, parent, app["digest"])


@pytest.mark.parametrize(
    "fault",
    [
        "pin",
        "rehash",
        "unknown",
        "artifact",
        "receipt",
        "retained-receipt",
        "attestation",
        "source",
        "mode",
        "tooling",
        "catalog",
        "index",
        "unrelated",
        "context",
    ],
)
def test_owner_final_verify_rejects_drift_and_rebinding(reviewed_run, fault):
    root, owner, transactions, review, app, parent, environment = reviewed_run
    os.environ["PYTEST_CURRENT_TEST"] = environment
    envelope = review.review_application(root, app, parent, app["digest"])
    pin = envelope["digest"]
    if fault == "pin":
        pin = "0" * 64
    elif fault == "rehash":
        envelope["parent_review"]["reviewer_run_id"] = "replacement-reviewer"
        envelope["digest"] = digest(
            {k: v for k, v in envelope.items() if k != "digest"}
        )
    elif fault == "unknown":
        envelope["extra"] = True
    elif fault == "context":
        envelope["application"]["review_context"]["implementation_run_id"] = (
            "replacement"
        )
    else:
        from pathlib import Path

        paths = {
            "artifact": Path(parent["review_artifact"]["path"]),
            "attestation": root / app["attestation"]["path"],
            "retained-receipt": root / app["receipts"][0]["path"],
            "source": root / "include/test.h",
            "mode": root / "include/test.h",
            "tooling": root / "CMakeLists.txt",
            "catalog": root / "out/catalog/emi.json",
            "index": root / ".git/index",
            "unrelated": root / "unrelated.txt",
        }
        if fault == "receipt":
            envelope["application"]["receipts"][0]["digest"] = "0" * 64
        elif fault == "mode":
            paths[fault].chmod(0o600)
        else:
            paths[fault].parent.mkdir(parents=True, exist_ok=True)
            paths[fault].write_text("drift\n")
    with pytest.raises((ValueError, OSError)):
        review.verify_reviewed_application(root, envelope, pin)


@pytest.mark.parametrize("owner", [type_audit, macro_audit])
@pytest.mark.parametrize("command", ["review", "final-verify"])
def test_review_cli_rejects_duplicate_json(tmp_path, owner, command):
    path = tmp_path / "duplicate.json"
    path.write_text('{"schema":"first","schema":"second"}')
    argv = ["--root", str(tmp_path), command, str(path)]
    argv += (
        [
            "out/reviews/evidence/result.json",
            "--parent-attestation",
            str(path),
            "--expected-application-digest",
            "0" * 64,
        ]
        if command == "review"
        else ["--expected-envelope-digest", "0" * 64]
    )
    args = owner.build_parser().parse_args(argv)
    with pytest.raises(ValueError, match="duplicate"):
        args.handler(args)


def test_shared_consumers_use_real_reviewed_owner_envelopes(reviewed_run, monkeypatch):
    from harness.domain.manifests import load_target_manifests
    from harness.macros import owners as macro_owners
    from harness.macros import review as macro_transaction_review
    from harness.types import proofs as type_shared_proofs

    root, owner, transactions, review, app, parent, environment = reviewed_run
    os.environ["PYTEST_CURRENT_TEST"] = environment
    envelope = review.review_application(root, app, parent, app["digest"])
    path = root / "out/reviews/private-reviewed.json"
    path.write_text(json.dumps(envelope))
    pin = {
        "path": path.relative_to(root).as_posix(),
        "target": app["target"],
        "expected_envelope_digest": envelope["digest"],
    }
    if owner == "macro":
        pin["selector"] = macro_tests.SELECTOR
    consumer = (
        type_shared_proofs.private_proofs
        if owner == "type"
        else macro_transaction_review.exact_proofs
    )
    verified = []

    def verify(root, value, expected):
        result = review.verify_reviewed_application(root, value, expected)
        verified.append(result)
        return result

    def consume():
        return consumer(
            root,
            [pin, dict(pin)],
            load_target_manifests(root),
            normalize_target=(
                transactions._target if owner == "type" else macro_owners.resolve_target
            ),
            verify_reviewed_application=verify,
        )

    # Real envelope verification succeeds before existing distinct-target/type
    # constraints reject. This is NOT positive two-private/shared closure.
    with pytest.raises(ValueError, match="paths must be unique|exact wrapper"):
        consume()
    assert len(verified) == (2 if owner == "type" else 1)
    verified.clear()
    external_digest = pin.pop("expected_envelope_digest")
    pin["expected_application_digest"] = app["digest"]
    with pytest.raises(ValueError, match="proof pin is invalid"):
        consume()
    assert not verified
    del pin["expected_application_digest"]
    pin["expected_envelope_digest"] = external_digest
    path.write_text(
        path.read_text().replace('"schema":', '"schema":"duplicate","schema":', 1)
    )
    with pytest.raises(ValueError, match="duplicate JSON key"):
        consume()
    path.write_text(json.dumps(envelope))

    # A genuine later private transaction, not a rehash/rebinding of the first.
    proposal = (
        type_tests._request(root)
        if owner == "type"
        else macro_tests._request(
            root, macro_tests.macro_accounting.candidate_account(root)
        )
    )
    proposal["adopted_baseline"] = transactions.workspace_baseline(root)["digest"]
    manifest = transactions.prepare_transaction(root, proposal)
    second = transactions.run_transaction(
        root,
        manifest,
        {"include/test.h": "/* later private replacement */\n"},
        runner=(type_tests if owner == "type" else macro_tests)._runner(),
        implementation_run_id="later-private-run",
    )
    assert transactions.verify_application(root, second, second["digest"])["applied"]
    with pytest.raises(ValueError):
        consume()
    assert not verified
    assert json.loads(path.read_text()) == envelope
    assert pin["expected_envelope_digest"] == external_digest


def test_shared_parent_acceptance_is_closed_before_publication(reviewed_run):
    root, owner, transactions, review, app, parent, environment = reviewed_run
    os.environ["PYTEST_CURRENT_TEST"] = environment
    app = copy.deepcopy(app)
    app["concern"] = "shared" if owner == "type" else "shared_template"
    # Otherwise valid parent/envelope structure must never bypass the ceiling.
    from harness.common.review import _binding

    parent["binding"] = _binding(app)
    with pytest.raises(ValueError, match="shared reviewed transition"):
        review.review_application(root, app, parent, app["digest"])
    facts = {
        "schema": f"bof3.{owner}-reviewed-application/v1",
        "application": app,
        "parent_review": parent,
    }
    envelope = {**facts, "digest": digest(facts)}
    with pytest.raises(ValueError, match="shared reviewed transition"):
        review.verify_reviewed_application(root, envelope, envelope["digest"])
    proof = root / "out/reviews/shared-application.json"
    attestation = root / "out/reviews/shared-parent.json"
    proof.write_text(json.dumps(app))
    attestation.write_text(json.dumps(parent))
    output = "out/reviews/evidence/shared-reviewed.json"
    cli = type_audit if owner == "type" else macro_audit
    args = cli.build_parser().parse_args(
        [
            "--root",
            str(root),
            "review",
            str(proof),
            output,
            "--parent-attestation",
            str(attestation),
            "--expected-application-digest",
            app["digest"],
        ]
    )
    with pytest.raises(ValueError, match="shared reviewed transition"):
        args.handler(args)
    assert not (root / output).exists()
