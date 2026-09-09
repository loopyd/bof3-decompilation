"""Real private owner transactions followed by check-only native revalidation."""

import copy
import json
import os
import subprocess

import pytest
import test_application_review
import test_macro_transactions as macro_tests
import test_type_transactions as type_tests
from harness.common.digests import digest

reviewed_run = test_application_review.reviewed_run


def _inputs(reviewed_run):
    root, owner, transactions, review, app, parent, environment = reviewed_run
    os.environ["PYTEST_CURRENT_TEST"] = environment
    envelope = review.review_application(root, app, parent, app["digest"])
    options = {
        "execution_run_id": "new-check-only-run",
        "adopted_baseline": transactions.workspace_baseline(root)["digest"],
    }
    return root, owner, transactions, review, app, envelope, options


def test_check_only_revalidation_executes_new_gates_without_reapplication(reviewed_run):
    root, owner, transactions, review, app, envelope, options = _inputs(reviewed_run)
    original = copy.deepcopy(envelope)
    before = (root / "include/test.h").read_bytes()
    index = (root / ".git/index").read_bytes()
    retained = {r["path"]: (root / r["path"]).read_bytes() for r in app["receipts"]}
    native = (type_tests if owner == "type" else macro_tests)._runner()
    commands = []

    def run(argv, **kwargs):
        commands.append(argv)
        if argv[0] == "bin/build":
            subprocess.run(
                ["ninja", "-C", str(root / "build/cmake")],
                check=True,
                capture_output=True,
                timeout=30,
            )
        return native(argv, **kwargs)

    value = review.revalidate_application(
        root,
        envelope,
        envelope["digest"],
        runner=run,
        output="out/reviews/evidence/check-only.json",
        **options,
    )
    assert commands == [c["argv"] for c in app["manifest"]["required_checks"]]
    assert (
        value["checked"] is True and "applied" not in value and "accepted" not in value
    )
    assert review.verify_revalidation(root, value, value["digest"])["checked"]
    assert (
        json.loads((root / "out/reviews/evidence/check-only.json").read_text()) == value
    )
    assert not {r["path"] for r in value["receipts"]} & set(retained)
    assert all((root / p).read_bytes() == b for p, b in retained.items())
    assert envelope == original
    assert (root / "include/test.h").read_bytes() == before
    assert (root / "include/test.h").stat().st_mode & 0o7777 == 0o750
    assert (root / ".git/index").read_bytes() == index
    assert transactions.verify_application(root, app, app["digest"])["applied"]
    # Fresh check-only evidence is not independent acceptance or a shared pin.
    with pytest.raises(ValueError):
        review.verify_reviewed_application(root, value, value["digest"])
    for fault in ("pin", "unknown", "receipt", "context", "owner"):
        changed = copy.deepcopy(value)
        pin = value["digest"]
        if fault == "pin":
            pin = "0" * 64
        elif fault == "unknown":
            changed["extra"] = True
        elif fault == "receipt":
            changed["receipts"][0]["sha256"] = "0" * 64
        elif fault == "context":
            changed["review_context"]["implementation_run_id"] = (
                "implementation-actual-run"
            )
        else:
            changed["schema"] = "bof3.wrong-application-revalidation/v1"
        if fault != "pin":
            pin = changed["digest"] = digest(
                {k: v for k, v in changed.items() if k != "digest"}
            )
        with pytest.raises(ValueError):
            review.verify_revalidation(root, changed, pin)
    (root / value["receipts"][0]["path"]).write_text("replaced")
    with pytest.raises(ValueError):
        review.verify_revalidation(root, value, value["digest"])


@pytest.mark.parametrize(
    "fault", ["run", "adoption", "pin", "drift", "gate", "mutation"]
)
def test_check_only_revalidation_rejects_before_publication(reviewed_run, fault):
    root, owner, transactions, review, app, envelope, options = _inputs(reviewed_run)
    before = (root / "include/test.h").read_bytes()
    original = copy.deepcopy(envelope)
    pin = envelope["digest"]
    if fault == "run":
        options["execution_run_id"] = "independent-reviewer-run"
    elif fault == "adoption":
        options["adopted_baseline"] = "0" * 64
    elif fault == "pin":
        pin = "0" * 64
    elif fault == "drift":
        (root / "unrelated.txt").write_text("later unreviewed private work\n")
        options["adopted_baseline"] = transactions.workspace_baseline(root)["digest"]
    calls = []
    native = (type_tests if owner == "type" else macro_tests)._runner()

    def run(argv, **kwargs):
        calls.append(argv)
        result = native(argv, **kwargs)
        if fault == "gate":
            result.returncode = 1
        if fault == "mutation":
            (root / "include/test.h").write_text("unexpected gate mutation\n")
        return result

    with pytest.raises((ValueError, RuntimeError)):
        review.revalidate_application(
            root,
            envelope,
            pin,
            runner=run,
            output="out/reviews/evidence/rejected-check.json",
            **options,
        )
    assert len(calls) == (1 if fault in {"gate", "mutation"} else 0)
    assert not (root / "out/reviews/evidence/rejected-check.json").exists()
    assert envelope == original
    if fault == "mutation":
        assert (root / "include/test.h").read_text() == "unexpected gate mutation\n"
    else:
        assert (root / "include/test.h").read_bytes() == before


@pytest.mark.parametrize("target", ["receipt", "application", "envelope", "output"])
@pytest.mark.parametrize("alias", ["direct", "symlink", "hardlink"])
def test_revalidation_existing_output_preserves_prerequisites(
    reviewed_run, target, alias
):
    root, owner, transactions, review, app, envelope, options = _inputs(reviewed_run)
    application = root / "out/reviews/evidence/original-application.json"
    application.write_text(json.dumps(app))
    reviewed = root / "out/reviews/evidence/original-envelope.json"
    reviewed.write_text(json.dumps(envelope))
    output = root / "out/reviews/evidence/existing-output.json"
    output.write_text("retained prior output\n")
    paths = [
        application,
        reviewed,
        output,
        *(root / r["path"] for r in app["receipts"]),
    ]
    for path in paths:
        path.chmod(0o750)
    retained = {p: (p.read_bytes(), p.stat().st_mode) for p in paths}
    destination = {
        "receipt": root / app["receipts"][0]["path"],
        "application": application,
        "envelope": reviewed,
        "output": output,
    }[target]
    if alias != "direct":
        link = root / "out/reviews/evidence/alias.json"
        if alias == "symlink":
            link.symlink_to(destination)
        else:
            os.link(destination, link)
        destination = link

    def unexpected_gate(*args, **kwargs):
        pytest.fail("existing destination must reject before native gates")

    with pytest.raises(FileExistsError):
        review.revalidate_application(
            root,
            envelope,
            envelope["digest"],
            runner=unexpected_gate,
            output=destination.relative_to(root).as_posix(),
            **options,
        )
    assert all(
        (p.read_bytes(), p.stat().st_mode) == state for p, state in retained.items()
    )
    assert transactions.verify_application(root, app, app["digest"])["applied"]
    assert review.verify_reviewed_application(root, envelope, envelope["digest"])[
        "accepted"
    ]


@pytest.mark.parametrize("alias", ["direct", "symlink", "hardlink"])
def test_revalidation_late_output_creation_never_clobbers(
    reviewed_run, monkeypatch, alias
):
    from harness.common import files as transaction_files

    root, owner, transactions, review, app, envelope, options = _inputs(reviewed_run)
    output = "out/reviews/evidence/late-output.json"
    destination = root / output
    retained = {
        root / r["path"]: (
            (root / r["path"]).read_bytes(),
            (root / r["path"]).stat().st_mode,
        )
        for r in app["receipts"]
    }
    original = copy.deepcopy(envelope)
    publish = transaction_files._rename_noreplace
    collided = []

    def create_at_publication(source, leaf, **kwargs):
        if leaf == destination.name:
            if alias == "direct":
                destination.write_bytes(b"concurrent output\n")
                destination.chmod(0o750)
            elif alias == "symlink":
                destination.symlink_to(root / app["receipts"][0]["path"])
            else:
                os.link(root / app["receipts"][0]["path"], destination)
            collided.append((destination.read_bytes(), destination.stat().st_mode))
        return publish(source, leaf, **kwargs)

    monkeypatch.setattr(transaction_files, "_rename_noreplace", create_at_publication)
    native = (type_tests if owner == "type" else macro_tests)._runner()
    commands = []

    def run(argv, **kwargs):
        commands.append(argv)
        if argv[0] == "bin/build":
            subprocess.run(
                ["ninja", "-C", str(root / "build/cmake")],
                check=True,
                capture_output=True,
                timeout=30,
            )
        return native(argv, **kwargs)

    with pytest.raises(RuntimeError, match="without replacing"):
        review.revalidate_application(
            root,
            envelope,
            envelope["digest"],
            runner=run,
            output=output,
            **options,
        )
    assert commands == [c["argv"] for c in app["manifest"]["required_checks"]]
    assert collided == [(destination.read_bytes(), destination.stat().st_mode)]
    assert all(
        (p.read_bytes(), p.stat().st_mode) == state for p, state in retained.items()
    )
    assert envelope == original
    assert transactions.verify_application(root, app, app["digest"])["applied"]
    assert review.verify_reviewed_application(root, envelope, envelope["digest"])[
        "accepted"
    ]


def test_fresh_parent_acceptance_of_no_drift_revalidation(reviewed_run):
    import hashlib

    root, owner, transactions, review, app, original, options = _inputs(reviewed_run)
    native = (type_tests if owner == "type" else macro_tests)._runner()
    commands = []

    def run(argv, **kwargs):
        commands.append(argv)
        if argv[0] == "bin/build":
            subprocess.run(
                ["ninja", "-C", str(root / "build/cmake")],
                check=True,
                capture_output=True,
                timeout=30,
            )
        return native(argv, **kwargs)

    record = review.revalidate_application(
        root, original, original["digest"], runner=run, **options
    )
    assert commands == [c["argv"] for c in app["manifest"]["required_checks"]]
    artifact = root / "out/reviews/fresh-review.md"
    artifact.write_text(
        "Fresh independent inspection of new native check-only evidence.\n"
    )
    parent = {
        "schema": f"bof3.{owner}-revalidation-parent-review/v1",
        "accepted": True,
        "parent_run_id": "supervising-parent-run",
        "implementation_run_id": options["execution_run_id"],
        "reviewer_run_id": "fresh-reviewer-run",
        "review_artifact": {
            "path": str(artifact),
            "sha256": hashlib.sha256(artifact.read_bytes()).hexdigest(),
        },
        "binding": {
            "revalidation_digest": record["digest"],
            "prerequisite_envelope_digest": original["digest"],
            "manifest_digest": digest(record["manifest"]),
            "review_context_digest": digest(record["review_context"]),
            "pre_state_digest": record["manifest"]["pre_state_digest"],
            "native_receipts_digest": digest(record["receipts"]),
            "adopted_baseline_digest": digest(
                record["review_context"]["adopted_baseline"]
            ),
        },
        "preservation": copy.deepcopy(original["parent_review"]["preservation"]),
    }
    retained = {
        root / r["path"]: (
            (root / r["path"]).read_bytes(),
            (root / r["path"]).stat().st_mode,
        )
        for r in app["receipts"] + record["receipts"]
    }
    source = root / "include/test.h"
    retained[source] = (source.read_bytes(), source.stat().st_mode)
    index = (root / ".git/index").read_bytes()
    frozen = copy.deepcopy((original, record, parent))
    envelope = review.review_revalidation(root, record, parent, record["digest"])
    assert envelope["revalidation"] == record
    for _ in range(2):
        assert review.verify_reviewed_revalidation(root, envelope, envelope["digest"])[
            "accepted"
        ]
    assert review.verify_reviewed_application(root, original, original["digest"])[
        "accepted"
    ]
    assert transactions.verify_application(root, app, app["digest"])["applied"]
    with pytest.raises(ValueError):
        review.verify_reviewed_application(root, envelope, envelope["digest"])
    with pytest.raises(ValueError):
        review.review_revalidation(root, record, parent, app["digest"])
    with pytest.raises(ValueError):
        review.verify_reviewed_revalidation(root, envelope, "0" * 64)

    for field in parent["binding"]:
        bad = copy.deepcopy(parent)
        bad["binding"][field] = "0" * 64
        with pytest.raises(ValueError):
            review.review_revalidation(root, record, bad, record["digest"])
    for fault in (
        "unknown",
        "owner",
        "accepted",
        "self",
        "old-reviewer",
        "old-artifact",
        "implementation",
        "preservation",
        "artifact",
    ):
        bad = copy.deepcopy(parent)
        if fault == "unknown":
            bad["unknown"] = True
        elif fault == "owner":
            bad["schema"] = "bof3.wrong-revalidation-parent-review/v1"
        elif fault == "accepted":
            bad["accepted"] = False
        elif fault == "self":
            bad["reviewer_run_id"] = options["execution_run_id"]
        elif fault == "old-reviewer":
            bad["reviewer_run_id"] = original["parent_review"]["reviewer_run_id"]
        elif fault == "old-artifact":
            bad["review_artifact"] = original["parent_review"]["review_artifact"]
        elif fault == "implementation":
            bad["implementation_run_id"] = "other-execution"
        elif fault == "preservation":
            bad["preservation"]["scope"] = 1
        else:
            bad["review_artifact"]["sha256"] = "0" * 64
        with pytest.raises(ValueError):
            review.review_revalidation(root, record, bad, record["digest"])
        forged = copy.deepcopy(envelope)
        forged["parent_review"] = bad
        forged["digest"] = digest({k: v for k, v in forged.items() if k != "digest"})
        with pytest.raises(ValueError):
            review.verify_reviewed_revalidation(root, forged, forged["digest"])

    assert (original, record, parent) == frozen
    assert (root / ".git/index").read_bytes() == index
    assert all(
        (p.read_bytes(), p.stat().st_mode) == state for p, state in retained.items()
    )
    # Retained reviewer and original/new native evidence remain live prerequisites.
    for path in (
        artifact,
        root / app["receipts"][0]["path"],
        root / record["receipts"][0]["path"],
    ):
        before = path.read_bytes()
        path.write_text("replaced evidence\n")
        with pytest.raises(ValueError):
            review.verify_reviewed_revalidation(root, envelope, envelope["digest"])
        path.write_bytes(before)
    (root / "unrelated.txt").write_text("later private mutation\n")
    with pytest.raises(ValueError):
        review.verify_reviewed_revalidation(root, envelope, envelope["digest"])
