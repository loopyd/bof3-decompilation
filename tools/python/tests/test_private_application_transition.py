"""Sequential real private owner transactions; native Ninja, synthetic BOF3 payloads."""

import copy
import hashlib
import json
import os
import shutil
import sqlite3
import subprocess

import pytest
import test_macro_transactions as macros
import test_type_transactions as types
from harness.common.digests import digest
from harness.common.revalidation import _review_binding
from harness.common.review import PRESERVATION, _binding
from harness.macros import application as macro_application_review
from harness.types import application as type_application_review
from transaction_native_fixture import execution_inputs, native_build


@pytest.mark.parametrize("owner", ["type", "macro"])
@pytest.mark.parametrize(
    "distinct,gap",
    [
        (distinct, gap)
        for distinct in (False, True)
        for gap in (None, "workspace", "input", "build", "mode")
    ]
    + [(True, "missing_scope"), (True, "mismatched_scope")],
)
def test_sequential_private_common_state(
    tmp_path, monkeypatch, owner, gap, distinct, shared=False, cli=False
):
    root = tmp_path
    if owner == "type":
        types._repo(root, git=True)
        transactions = types.transactions
        monkeypatch.setattr(transactions, "connect", types._connect)
        proposal = types._request(root)
        review = type_application_review
    else:
        report = macros._setup(root, monkeypatch, git=True)
        transactions = macros.macro_transactions
        proposal = macros._request(root, report)
        review = macro_application_review
    if cli:
        from transaction_cli_fixture import OwnerCLI

        transactions = review = OwnerCLI(root, owner, transactions, review)
    config = root / "config/targets/exe/test/target.toml"
    config.write_text(
        config.read_text().replace(
            "headers=['include/test.h']",
            "headers=['include/test.h','include/second.h']",
        )
    )
    (root / "include/second.h").write_text("/* second private owner */\n")
    execution_inputs(root)
    if distinct:
        # Two real manifest/index owners, frozen before either application.
        second_config = root / "config/targets/exe/second"
        shutil.copytree(config.parent, second_config)
        shutil.copytree(root / "src/test", root / "src/second")
        for path in second_config.iterdir():
            path.write_text(
                path.read_text()
                .replace("exe/test", "exe/second")
                .replace("src/test", "src/second")
                .replace("test.bin", "second.bin")
                .replace("'include/test.h',", "")
            )
        binary = root / ("out/test.bin" if owner == "type" else "test.bin")
        binary.with_name("second.bin").write_bytes(binary.read_bytes())
        config.write_text(
            config.read_text()
            .replace(", 'include/second.h'", "")
            .replace(",'include/second.h'", "")
        )
        database = root / (
            "out/index/reverse.sqlite" if owner == "type" else "index.sqlite"
        )
        connection = sqlite3.connect(database)
        for table in (
            "targets",
            "functions",
            "type_candidates" if owner == "type" else "macro_input_fingerprints",
        ):
            for row in connection.execute(f"SELECT * FROM {table}").fetchall():
                row = tuple(
                    v.replace("exe/test", "exe/second")
                    .replace("src/test", "src/second")
                    .replace("test.bin", "second.bin")
                    if isinstance(v, str)
                    else v
                    for v in row
                )
                connection.execute(
                    f"INSERT INTO {table} VALUES ({','.join('?' for _ in row)})", row
                )
        connection.commit()
        connection.close()
        # Keep the changed fixture DB distinguishable from Git's initial stat cache,
        # including after the deliberate raw-index tamper/restoration below.
        stamp = database.stat().st_mtime_ns + 10_000_000_000
        os.utime(database, ns=(stamp, stamp))
    if shared:
        from test_shared_application_pre import prepare_inputs

        proposal = prepare_inputs(root, owner, proposal)
    scope = {"participating_targets": ["exe/second", "exe/test"]} if distinct else {}
    (root / ".gitignore").write_text("out/\nbuild/\n")
    (root / "unrelated.txt").write_text("adopted unrelated bytes\n")
    (root / "include/test.h").chmod(0o750)
    (root / "include/second.h").chmod(0o740)
    native = (types if owner == "type" else macros)._runner()
    calls = []

    def run(argv, **kwargs):
        calls.append(argv)
        if argv[0] == "bin/build":
            native_build(root)
        if (
            distinct
            and argv[0] in {"bin/asm-diff", "bin/byte-match"}
            and "exe/second" in " ".join(argv)
        ):
            result = native(argv, **kwargs)
            return subprocess.CompletedProcess(
                argv,
                result.returncode,
                result.stdout.replace("src/test", "src/second").replace(
                    "test.bin", "second.bin"
                ),
                result.stderr,
            )
        return native(argv, **kwargs)

    def parent(application, number):
        artifact = root / f"out/reviews/review-{number}.md"
        artifact.write_text(f"Synthetic independent review {number}\n")
        return {
            "schema": f"bof3.{owner}-parent-review/v1",
            "accepted": True,
            "parent_run_id": "parent",
            "implementation_run_id": f"implementation-{number}",
            "reviewer_run_id": f"reviewer-{number}",
            "review_artifact": {
                "path": str(artifact),
                "sha256": hashlib.sha256(artifact.read_bytes()).hexdigest(),
            },
            "binding": _binding(application),
            "preservation": dict(PRESERVATION),
        }

    proposal["adopted_baseline"] = transactions.workspace_baseline(root)["digest"]
    manifest = transactions.prepare_transaction(root, proposal)
    if distinct and gap is None:
        for invalid in (
            [],
            ["exe/second"],
            ["exe/test", "exe/test"],
            ["exe/test", "exe/second"],
            ["exe/missing", "exe/test"],
            ["exe/../test"],
            ["exe/a", "exe/second", "exe/test"],
        ):
            with pytest.raises(SystemExit if cli and not invalid else ValueError):
                transactions.run_transaction(
                    root,
                    manifest,
                    {"include/test.h": "bad"},
                    runner=run,
                    implementation_run_id="invalid",
                    participating_targets=invalid,
                )
            assert not calls
        with pytest.raises(ValueError):
            transactions.run_transaction(
                root,
                manifest,
                {"include/test.h": "bad"},
                participating_targets=scope["participating_targets"],
            )
    app = transactions.run_transaction(
        root,
        manifest,
        {"include/test.h": "/* first reviewed post */\n"},
        runner=run,
        implementation_run_id="implementation-1",
        **({} if gap == "missing_scope" else scope),
    )
    first = review.review_application(root, app, parent(app, 1), app["digest"])
    # Separate retained candidate: never overwrite the first transaction's evidence.
    proposal = copy.deepcopy(proposal)
    if owner == "type":
        old = root / proposal["candidate_artifacts"][0]
    else:
        old = root / proposal["candidate_artifact"]
    candidate = json.loads(old.read_text())
    header = "include/second.h"
    if owner == "type":
        candidate["candidate"].update(
            owners=[header],
            locations=[header],
            fingerprints={
                header: hashlib.sha256((root / header).read_bytes()).hexdigest()
            },
        )
        proposal["header"] = header
        proposal["candidate_artifacts"] = ["out/reviews/second-candidate.json"]
    else:
        candidate.update(
            owners=[header],
            owner_fingerprints={
                header: hashlib.sha256((root / header).read_bytes()).hexdigest()
            },
        )
        proposal["candidate_artifact"] = "out/reviews/second-candidate.json"
    if distinct:
        proposal["target"] = "exe/second"
        proposal["affected_functions"] = ["exe/second@0x80100000"]
        if owner == "type":
            proposal["index_candidate_ids"] = ["exe/second@80100000:prototype"]
            candidate["candidate"]["target"] = "exe/second"
            candidate["candidate"]["id"] = (
                "aggregate" if shared else "prototype"
            ) + ":exe/second@80100000"
            candidate["index_id"] = proposal["index_candidate_ids"][0]
            connection = types._connect(root)
            row = dict(
                connection.execute(
                    "SELECT * FROM type_candidates WHERE target_id='exe/second'"
                ).fetchone()
            )
            connection.close()
            row["evidence"] = json.loads(row["evidence"])
            candidate["index_row_digest"] = digest(row)
        else:
            report = macros.macro_accounting.candidate_account(
                root, target=None if shared else "exe/second"
            )
            if shared:
                report["rows"] = [
                    row for row in report["rows"] if row["kind"] == "exact_group"
                ]
            candidate["candidate_id"] = report["rows"][0]["id"]
            candidate["candidate_fingerprint"] = report["rows"][0][
                "candidate_fingerprint"
            ]
    candidate["digest"] = digest({k: v for k, v in candidate.items() if k != "digest"})
    (root / "out/reviews/second-candidate.json").write_text(json.dumps(candidate))
    if gap == "workspace":
        (root / "unrelated.txt").write_text("unreviewed intervening edit\n")
    elif gap == "input":
        (root / "config/compiler/object-flags.cmake").write_text("# unreviewed\n")
    elif gap == "build":
        with (root / "build/cmake/build.ninja").open("a") as stream:
            stream.write("\n# unreviewed build gap\n")
    elif gap == "mode":
        (root / "include/test.h").chmod(0o700)
    proposal["adopted_baseline"] = transactions.workspace_baseline(root)["digest"]
    second_manifest = transactions.prepare_transaction(root, proposal)
    second_app = transactions.run_transaction(
        root,
        second_manifest,
        {header: "/* second reviewed post */\n"},
        runner=run,
        implementation_run_id="implementation-2",
        **(
            {"participating_targets": ["exe/second"]}
            if gap == "mismatched_scope"
            else scope
        ),
    )
    second = review.review_application(
        root, second_app, parent(second_app, 2), second_app["digest"]
    )
    pins = [{"envelope": second, "expected_envelope_digest": second["digest"]}]
    frozen = copy.deepcopy((first, second))
    index = (root / ".git/index").read_bytes()
    options = {
        "execution_run_id": "common-check-1",
        "adopted_baseline": transactions.workspace_baseline(root)["digest"],
    }
    with pytest.raises(ValueError):
        review.verify_reviewed_application(root, first, first["digest"])
    with pytest.raises(ValueError):
        review.revalidate_application(
            root, first, first["digest"], runner=run, **options
        )

    def check(argv, **kwargs):
        calls.append(argv)
        if argv[0] == "bin/build":
            subprocess.run(
                ["ninja", "-C", str(root / "build/cmake")],
                check=True,
                capture_output=True,
                timeout=30,
            )
        if (
            distinct
            and argv[0] in {"bin/asm-diff", "bin/byte-match"}
            and "exe/second" in " ".join(argv)
        ):
            result = native(argv, **kwargs)
            return subprocess.CompletedProcess(
                argv,
                result.returncode,
                result.stdout.replace("src/test", "src/second").replace(
                    "test.bin", "second.bin"
                ),
                result.stderr,
            )
        return native(argv, **kwargs)

    if gap is not None:
        start = len(calls)
        with pytest.raises(
            ValueError, match="transition gap|build gap|post-state|membership gap"
        ):
            review.revalidate_application(
                root, first, first["digest"], runner=check, intervening=pins, **options
            )
        assert len(calls) == start
        return

    records = []
    accepted = []
    for number, envelope, intervening in ((1, first, pins), (2, second, [])):
        start = len(calls)
        record = review.revalidate_application(
            root,
            envelope,
            envelope["digest"],
            runner=check,
            intervening=intervening,
            **{**options, "execution_run_id": f"common-check-{number}"},
        )
        assert calls[start:] == [
            c["argv"] for c in record["manifest"]["required_checks"]
        ]
        fresh = parent(app, f"fresh-{number}")
        fresh.update(
            schema=f"bof3.{owner}-revalidation-parent-review/v1",
            implementation_run_id=f"common-check-{number}",
            binding=_review_binding(record),
        )
        accepted.append(
            review.review_revalidation(root, record, fresh, record["digest"])
        )
        records.append(record)
    assert (
        records[0]["review_context"]["initial_build"]
        == records[1]["review_context"]["initial_build"]
    )
    assert (
        records[0]["review_context"]["adopted_baseline"]
        == records[1]["review_context"]["adopted_baseline"]
    )
    if distinct:
        from harness.common.transition import _state_equal

        assert records[0]["manifest"]["target"] != records[1]["manifest"]["target"]
        _state_equal(
            records[0],
            records[0]["review_context"]["initial_state"],
            records[1],
            records[1]["review_context"]["initial_state"],
        )
    if distinct:
        altered = copy.deepcopy(records[0])
        for state in ("initial_state", "final_state"):
            altered["review_context"][state]["participating_targets"] = ["exe/test"]
        altered["digest"] = digest({k: v for k, v in altered.items() if k != "digest"})
        with pytest.raises(ValueError, match="participating targets drifted"):
            review.verify_revalidation(root, altered, altered["digest"])
    for envelope in accepted:
        assert review.verify_reviewed_revalidation(root, envelope, envelope["digest"])[
            "accepted"
        ]
    if shared:
        from test_shared_application_pre import shared_pre_checks

        shared_pre_checks(root, owner, transactions, review, accepted, check, calls)
        if cli:
            assert set(transactions.commands) == {
                "prepare",
                "run",
                "review",
                "final-verify",
                "revalidate",
                "verify-revalidation",
                "review-revalidation",
                "final-verify-revalidation",
            }
        assert (first, second) == frozen
        assert (root / ".git/index").read_bytes() == index
        return
    assert (first, second) == frozen
    assert (root / ".git/index").read_bytes() == index
    # Every closure class rejects despite explicitly adopting today's workspace.
    for name in (
        "include/test.h",
        "include/second.h",
        "unrelated.txt",
        "config/compiler/object-flags.cmake",
        "out/index/reverse.sqlite",
        "build/cmake/build.ninja",
        ".git/index",
        *(
            [
                "out/second.bin" if owner == "type" else "second.bin",
                "config/targets/exe/second/splat.yaml",
                "config/targets/exe/second/reviewed.rz",
                "config/targets/exe/second/symbols.txt",
            ]
            if distinct
            else []
        ),
    ):
        path = root / name
        before = path.read_bytes() if path.exists() else None
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes((before or b"") + b"\n")
        with pytest.raises((ValueError, subprocess.CalledProcessError)):
            review.verify_reviewed_revalidation(
                root, accepted[0], accepted[0]["digest"]
            )
        if before is None:
            path.unlink()
        else:
            path.write_bytes(before)
    for fault in ("pin", "missing", "duplicate", "order", "nested", "run"):
        bad = copy.deepcopy(pins)
        opts = dict(options)
        if fault == "pin":
            bad[0]["expected_envelope_digest"] = "0" * 64
        elif fault == "missing":
            bad = []
        elif fault == "duplicate":
            bad *= 2
        elif fault == "order":
            bad = [{"envelope": first, "expected_envelope_digest": first["digest"]}]
        elif fault == "nested":
            bad[0]["envelope"]["application"]["changed_paths"] = []
        else:
            opts["execution_run_id"] = "implementation-2"
        start = len(calls)
        with pytest.raises(ValueError):
            review.revalidate_application(
                root, first, first["digest"], runner=check, intervening=bad, **opts
            )
        assert len(calls) == start
    assert review.verify_reviewed_revalidation(
        root, accepted[0], accepted[0]["digest"]
    )["accepted"]
