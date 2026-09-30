"""CLI revalidation transport uses real owner validation and exclusive publication."""

import hashlib
import json
import subprocess

import pytest

from harness.common.commands import tool_id

import test_application_revalidation as checks
import test_macro_transactions as macros
import test_type_transactions as types
from harness.common.revalidation import _review_binding
from harness.macros import cli as macro_audit
from harness.types import cli as type_audit

reviewed_run = checks.reviewed_run


def invoke(cli, root, argv):
    args = cli.build_parser().parse_args(["--root", str(root), *map(str, argv)])
    return args.handler(args)


def put(root, name, value):
    path = root / f"out/reviews/evidence/{name}.json"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value))
    return path


@pytest.mark.parametrize("cli", [type_audit, macro_audit])
def test_run_participating_targets_transport(tmp_path, monkeypatch, cli):
    manifest = put(tmp_path, "manifest", {})
    changes = put(tmp_path, "changes", {})
    calls = []

    def run(root, manifest, changes, **kwargs):
        calls.append(kwargs)
        return dict(
            schema="fixture",
            target="exe/test",
            concern="fixture",
            applied=True,
            digest="pin",
        )

    monkeypatch.setattr(cli, "run_transaction", run)
    argv = ["run", manifest, changes, "out/reviews/evidence/application.json"]
    assert invoke(cli, tmp_path, argv) == 0
    assert calls[-1] == {"output": "out/reviews/evidence/application.json"}
    assert (
        invoke(
            cli,
            tmp_path,
            [
                *argv,
                "--implementation-run-id",
                "implementation",
                "--participating-targets",
                "exe/second",
                "exe/test",
            ],
        )
        == 0
    )
    assert calls[-1] == dict(
        output="out/reviews/evidence/application.json",
        implementation_run_id="implementation",
        participating_targets=["exe/second", "exe/test"],
    )


def test_revalidation_cli_real_owner_checks_and_pins(reviewed_run, monkeypatch):
    root, owner, transactions, review, app, envelope, options = checks._inputs(
        reviewed_run
    )
    cli = type_audit if owner == "type" else macro_audit
    proof = put(root, "prerequisite", envelope)
    output = root / "out/reviews/evidence/cli-revalidation.json"
    native = (types if owner == "type" else macros)._runner()
    original = review.revalidate_application
    calls = []

    def run(argv, **kwargs):
        calls.append(argv)
        if tool_id(argv) == "build":
            subprocess.run(
                ["ninja", "-C", str(root / "build/cmake")],
                check=True,
                capture_output=True,
                timeout=30,
            )
        return native(argv, **kwargs)

    def revalidate(*args, **kwargs):
        return original(*args, runner=run, **kwargs)

    monkeypatch.setattr(review, "revalidate_application", revalidate)
    argv = [
        "revalidate",
        proof,
        output.relative_to(root),
        "--expected-envelope-digest",
        envelope["digest"],
        "--execution-run-id",
        options["execution_run_id"],
        "--adopted-baseline",
        options["adopted_baseline"],
    ]
    before = (root / "include/test.h").read_bytes(), (root / ".git/index").read_bytes()
    assert invoke(cli, root, argv) == 0
    record = json.loads(output.read_text())
    assert len(calls) == len(app["manifest"]["required_checks"])
    assert record["checked"] and "accepted" not in record
    assert (
        invoke(
            cli,
            root,
            [
                "verify-revalidation",
                output,
                "--expected-revalidation-digest",
                record["digest"],
            ],
        )
        == 0
    )
    start = len(calls)
    with pytest.raises(ValueError):
        invoke(
            cli,
            root,
            ["verify-revalidation", output, "--expected-revalidation-digest", "0" * 64],
        )
    with pytest.raises(FileExistsError):
        invoke(cli, root, argv)
    artifact = root / "out/reviews/cli-review.txt"
    artifact.write_text("Synthetic fixture review, not BOF3 approval.\n")
    parent = {
        **envelope["parent_review"],
        "schema": f"bof3.{owner}-revalidation-parent-review/v1",
        "implementation_run_id": options["execution_run_id"],
        "reviewer_run_id": "cli-fresh-reviewer",
        "review_artifact": {
            "path": str(artifact),
            "sha256": hashlib.sha256(artifact.read_bytes()).hexdigest(),
        },
        "binding": _review_binding(record),
    }
    parent_path = put(root, "parent", parent)
    accepted = root / "out/reviews/evidence/cli-accepted.json"
    review_argv = [
        "review-revalidation",
        output,
        accepted.relative_to(root),
        "--expected-revalidation-digest",
        record["digest"],
        "--parent-attestation",
        parent_path,
    ]
    assert invoke(cli, root, review_argv) == 0
    retained = accepted.read_bytes()
    with pytest.raises(FileExistsError):
        invoke(cli, root, review_argv)
    assert accepted.read_bytes() == retained
    pin = json.loads(retained)["digest"]
    replay = ["final-verify-revalidation", accepted, "--expected-envelope-digest", pin]
    assert invoke(cli, root, replay) == 0
    assert invoke(cli, root, replay) == 0
    assert len(calls) == start
    assert before == (
        (root / "include/test.h").read_bytes(),
        (root / ".git/index").read_bytes(),
    )
    artifact.write_text("replaced")
    with pytest.raises(ValueError):
        invoke(cli, root, replay)


@pytest.mark.parametrize("cli", [type_audit, macro_audit])
def test_revalidation_cli_strict_intervening_transport(tmp_path, monkeypatch, cli):
    owner = (
        cli.type_application_review
        if cli is type_audit
        else cli.macro_application_review
    )
    proof = put(tmp_path, "proof", {})
    pins = [{"envelope": {"fixture": True}, "expected_envelope_digest": "external"}]
    intervening = put(tmp_path, "intervening", pins)
    calls = []

    def revalidate(*args, **kwargs):
        calls.append((args, kwargs))
        return dict(schema="fixture", digest="pin")

    monkeypatch.setattr(owner, "revalidate_application", revalidate)
    argv = [
        "revalidate",
        proof,
        "out/reviews/evidence/new.json",
        "--expected-envelope-digest",
        "external",
        "--execution-run-id",
        "fresh",
        "--adopted-baseline",
        "adopted",
        "--intervening",
        intervening,
    ]
    assert invoke(cli, tmp_path, argv) == 0
    assert calls[0] == (
        (tmp_path, {}, "external"),
        dict(
            execution_run_id="fresh",
            adopted_baseline="adopted",
            intervening=pins,
            output="out/reviews/evidence/new.json",
        ),
    )
    for path in (proof, intervening):
        before = path.read_bytes()
        path.write_text('{"pin":1,"pin":2}')
        with pytest.raises(ValueError, match="duplicate"):
            invoke(cli, tmp_path, argv)
        assert len(calls) == 1
        path.write_bytes(before)


@pytest.mark.parametrize("cli", [type_audit, macro_audit])
def test_review_revalidation_late_output_never_clobbers(tmp_path, monkeypatch, cli):
    from harness.common import verification as transport

    owner = (
        cli.type_application_review
        if cli is type_audit
        else cli.macro_application_review
    )
    proof = put(tmp_path, "record", {})
    parent = put(tmp_path, "parent", {})
    output = tmp_path / "out/reviews/evidence/late.json"

    def review(*args):
        output.write_bytes(b"concurrent evidence\n")
        return {"digest": "fixture"}

    monkeypatch.setattr(owner, "review_revalidation", review)
    with pytest.raises(FileExistsError):
        invoke(
            cli,
            tmp_path,
            [
                "review-revalidation",
                proof,
                output.relative_to(tmp_path),
                "--parent-attestation",
                parent,
                "--expected-revalidation-digest",
                "pin",
            ],
        )
    assert output.read_bytes() == b"concurrent evidence\n"
    assert transport._read(proof) == {}


# Removed proven duplicate (2026-09-20):
# `test_distinct_target_shared_sequence_cli[type]`/`[macro]` was pure delegation
# to test_private_application_transition.test_sequential_private_common_state(
# tmp_path, monkeypatch, owner, None, True, shared=True, cli=True) -- the same
# shared-application scenario already asserted by test_shared_application_pre.
# test_shared_pre_from_two_fresh_private_owners (shared=True, cli=False). The
# only delta was the OwnerCLI wrapper, whose transport/argument handling is
# covered by test_run_participating_targets_transport and
# test_revalidation_cli_strict_intervening_transport. Collected count 1923 -> 1921.
