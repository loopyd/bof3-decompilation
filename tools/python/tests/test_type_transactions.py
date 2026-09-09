from __future__ import annotations

import argparse
import hashlib
import json
import sqlite3
import subprocess
from pathlib import Path
from typing import Any

import pytest
from harness.analysis.index import SCHEMA_VERSION
from harness.analysis.schema import create_schema
from harness.common import files as transaction_files
from harness.common import git as transaction_git
from harness.common import quarantine as transaction_quarantine
from harness.common import runtime
from harness.common.digests import digest
from harness.domain.receipts import CANDIDATE_SCHEMA, sha256_file
from harness.types import cli as type_audit
from harness.types import proofs as type_shared_proofs
from harness.types import transactions
from harness.types.review import SCHEMA as REVIEW_SCHEMA
from transaction_native_fixture import exact_payload

TARGET = "exe/test"


def _repo(root: Path, *, status: str = "blocked", git: bool = False) -> None:
    config = root / "config/targets/exe/test/target.toml"
    config.parent.mkdir(parents=True)
    config.write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src/test'\nbinary='out/test.bin'\nload_address=0x80100000\n"
        "splat='config/targets/exe/test/splat.yaml'\n"
        "sources=['src/test/func_80100000.c']\nheaders=['include/test.h']\n",
        encoding="utf-8",
    )
    (config.parent / "splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [8]\n"
    )
    (config.parent / "symbols.txt").write_text("func_80100000 = 0x80100000;\n")
    binary = root / "out/test.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"\0" * 8)
    header = root / "include/test.h"
    header.parent.mkdir(parents=True)
    header.write_text("typedef unsigned int Test;\n")
    source = root / "src/test/func_80100000.c"
    source.parent.mkdir(parents=True)
    source.write_text(
        "/** @source 0x80100000\n * @behavior test lift\n */\n"
        "void func_80100000(void) {}\n"
    )
    index = root / "out/index/reverse.sqlite"
    index.parent.mkdir(parents=True)
    connection = sqlite3.connect(index)
    create_schema(connection)
    connection.execute("INSERT INTO metadata VALUES ('schema', ?)", (SCHEMA_VERSION,))
    binary_digest = hashlib.sha256(binary.read_bytes()).hexdigest()
    connection.execute(
        "INSERT INTO targets VALUES (?, ?, ?, ?, 'test', '1', ?, ?)",
        (
            TARGET,
            "out/test.bin",
            binary_digest,
            0x80100000,
            "out/snapshot.json",
            "0" * 64,
        ),
    )
    connection.execute(
        "INSERT INTO functions (id,target_id,address,size,name,compiled_symbol,analyzer_sha256,"
        "reviewed_sha256,reviewed_size,reviewed,lifted,source,lift_status,instruction_count,contains_data) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
        (
            f"{TARGET}@80100000",
            TARGET,
            0x80100000,
            8,
            "func_80100000",
            "func_80100000",
            "a" * 64,
            "b" * 64,
            8,
            1,
            1,
            "src/test/func_80100000.c",
            "exact",
            2,
            0,
        ),
    )
    connection.execute(
        "INSERT INTO type_candidates VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)",
        (
            f"{TARGET}@80100000:prototype",
            TARGET,
            0x80100000,
            None,
            "prototype",
            "abi",
            None,
            "unknown",
            status,
            "lead",
            "unresolved",
            "[]",
            "review needed",
        ),
    )
    connection.commit()
    connection.close()
    if git:
        subprocess.run(["git", "init", "-q"], cwd=root, check=True)
        subprocess.run(["git", "add", "."], cwd=root, check=True)
        subprocess.run(
            [
                "git",
                "-c",
                "user.name=T",
                "-c",
                "user.email=t@x",
                "commit",
                "-qm",
                "base",
            ],
            cwd=root,
            check=True,
        )


def _connect(root: Path):
    connection = sqlite3.connect(root / "out/index/reverse.sqlite")
    connection.row_factory = sqlite3.Row
    return connection


def _candidate(root: Path, status: str = "accepted") -> dict[str, object]:
    owner = "include/test.h"
    return {
        "schema": CANDIDATE_SCHEMA,
        "id": "prototype:exe/test@80100000",
        "kind": "prototype",
        "status": status,
        "target": TARGET,
        "address": 0x80100000,
        "end": None,
        "owners": [owner],
        "locations": [owner],
        "fingerprints": {owner: sha256_file(root / owner)},
        "provenance": ["caller", "callee"],
        "authority": "reviewed",
        "observations": [
            {"id": "caller", "text": "call ABI"},
            {"id": "callee", "text": "callee ABI"},
        ],
        "missing_facts": [],
        "receipts": [],
    }


def _artifact(root: Path, *, status: str = "accepted") -> str:
    connection = _connect(root)
    row = dict(connection.execute("SELECT * FROM type_candidates").fetchone())
    row["evidence"] = json.loads(row["evidence"])
    connection.close()
    facts = {
        "schema": REVIEW_SCHEMA,
        "index_id": row["id"],
        "candidate": _candidate(root, status),
        "representation": {"status": "resolved", "contract": {"abi": "void(void)"}},
        "semantics": {"status": "resolved", "contract": {"role": "test"}},
        "review": {"verdict": "accepted", "reviewer": "independent"},
        "index_row_digest": digest(row),
    }
    path = root / "out/reviews/type-candidate.json"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps({**facts, "digest": digest(facts)}))
    return path.relative_to(root).as_posix()


def _request(root: Path, **extra) -> dict[str, Any]:
    return {
        "schema": transactions.REQUEST_SCHEMA,
        "target": TARGET,
        "concern": "prototype",
        "header": "include/test.h",
        "index_candidate_ids": [f"{TARGET}@80100000:prototype"],
        "candidate_artifacts": [_artifact(root)],
        "affected_functions": [f"{TARGET}@0x80100000"],
        **extra,
    }


def _runner(fail: int | None = None):
    count = 0

    def run(argv, **kwargs):
        nonlocal count
        count += 1
        code = 1 if fail == count else 0
        output = (
            json.dumps(exact_payload(kwargs["cwd"], argv[0]))
            if argv[0] in {"bin/asm-diff", "bin/byte-match"}
            else "ran " + " ".join(argv)
        )
        return subprocess.CompletedProcess(argv, code, output, "")

    return run


def test_forged_index_source_cannot_override_manifest_metadata(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    forged = tmp_path / "AGENTS.md"
    forged.write_text("/** @source 0x80100000\n * @behavior forged\n */\n")
    connection = _connect(tmp_path)
    connection.execute("UPDATE functions SET source='AGENTS.md'")
    connection.commit()
    connection.close()
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    assert manifest["affected_functions"][0]["source"] == "src/test/func_80100000.c"


@pytest.mark.parametrize("command", ["prepare", "run"])
def test_type_cli_outputs_are_confined_and_symlink_safe(
    tmp_path: Path, monkeypatch, command: str
) -> None:
    victim = tmp_path / "victim.json"
    victim.write_text("keep\n")
    monkeypatch.setattr(
        type_audit, "prepare_transaction", lambda *_args: {"proof": True}
    )
    monkeypatch.setattr(
        type_audit,
        "run_transaction",
        lambda root, *_args, **kwargs: type_audit.write_evidence_output(
            root, kwargs["output"], {"proof": True}
        ),
    )
    monkeypatch.setattr(type_audit, "_read", lambda _path: {})
    handler = type_audit._prepare if command == "prepare" else type_audit._run
    args = argparse.Namespace(
        root=tmp_path,
        request=victim,
        manifest=victim,
        changes=victim,
        output=Path("victim.json"),
    )
    for output in (
        victim,
        Path("../victim.json"),
        Path("out/reviews/evidence/../../victim.json"),
    ):
        args.output = output
        with pytest.raises(ValueError, match="repo-relative|out/reviews/evidence"):
            handler(args)
    assert victim.read_text() == "keep\n"

    outside = tmp_path / "outside"
    outside.mkdir()
    evidence = tmp_path / "out/reviews/evidence"
    evidence.parent.mkdir(parents=True)
    evidence.symlink_to(outside, target_is_directory=True)
    args.output = Path("out/reviews/evidence/proof.json")
    with pytest.raises(ValueError, match="unsafe"):
        handler(args)
    assert list(outside.iterdir()) == []

    evidence.unlink()
    evidence.mkdir()
    leaf = evidence / "proof.json"
    leaf.symlink_to(victim)
    with pytest.raises(ValueError, match="regular file|unsafe"):
        handler(args)
    assert victim.read_text() == "keep\n"


def test_db_status_never_authorizes_without_reviewed_artifact(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path, status="proposed")
    monkeypatch.setattr(transactions, "connect", _connect)
    request = _request(tmp_path)
    request["candidate_artifacts"] = ["missing.json"]
    with pytest.raises(ValueError, match="artifact missing"):
        transactions.prepare_transaction(tmp_path, request)


def test_reviewed_candidate_requires_resolved_two_corroborators(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    request = _request(tmp_path)
    value = json.loads((tmp_path / request["candidate_artifacts"][0]).read_text())
    value["candidate"]["observations"] = value["candidate"]["observations"][:1]
    facts = {key: item for key, item in value.items() if key != "digest"}
    (tmp_path / request["candidate_artifacts"][0]).write_text(
        json.dumps({**facts, "digest": digest(facts)})
    )
    with pytest.raises(ValueError, match="unresolved evidence"):
        transactions.prepare_transaction(tmp_path, request)


def test_dirty_worktree_requires_exact_adopted_baseline(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path, git=True)
    monkeypatch.setattr(transactions, "connect", _connect)
    (tmp_path / "unrelated.txt").write_text("dirty")
    request = _request(tmp_path)
    with pytest.raises(ValueError, match="adopted_baseline"):
        transactions.prepare_transaction(tmp_path, request)
    current = transactions._workspace_state(tmp_path)
    request["adopted_baseline"] = digest(current)
    assert transactions.prepare_transaction(tmp_path, request)["workspace_baseline"][
        "adopted"
    ]


@pytest.mark.parametrize("implementation_run_id", [None, "implementation-1"])
def test_validation_cannot_change_git_index_even_for_allowed_path(
    tmp_path: Path, monkeypatch, implementation_run_id
) -> None:
    from transaction_native_fixture import execution_inputs

    _repo(tmp_path, git=True)
    if implementation_run_id:
        execution_inputs(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    request = _request(tmp_path)
    request["adopted_baseline"] = digest(transactions._workspace_state(tmp_path))
    manifest = transactions.prepare_transaction(tmp_path, request)
    before = subprocess.run(
        ["git", "ls-files", "--stage", "-z"],
        cwd=tmp_path,
        capture_output=True,
        check=True,
    ).stdout

    def stage(argv, **_kwargs):
        subprocess.run(["git", "add", "include/test.h"], cwd=tmp_path, check=True)
        staged["index"] = (tmp_path / ".git/index").read_bytes()
        return subprocess.CompletedProcess(argv, 0, "", "")

    staged = {}
    original = (tmp_path / "include/test.h").read_bytes()
    with pytest.raises(RuntimeError, match="external state preserved"):
        transactions.run_transaction(
            tmp_path,
            manifest,
            {"include/test.h": "changed\n"},
            runner=stage,
            implementation_run_id=implementation_run_id,
        )
    after = subprocess.run(
        ["git", "ls-files", "--stage", "-z"],
        cwd=tmp_path,
        capture_output=True,
        check=True,
    ).stdout
    assert after != before
    assert (tmp_path / ".git/index").read_bytes() == staged["index"]
    assert (tmp_path / "include/test.h").read_bytes() == original
    assert (
        subprocess.run(["git", "diff", "--cached", "--quiet"], cwd=tmp_path).returncode
        == 1
    )


def test_atomic_run_and_immutable_runner_receipts(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    application = transactions.run_transaction(
        tmp_path,
        manifest,
        {"include/test.h": "typedef unsigned long Test;\n"},
        runner=_runner(),
    )
    expected_digest = application["digest"]
    assert transactions.verify_application(tmp_path, application, expected_digest)[
        "applied"
    ]
    receipt = tmp_path / application["receipts"][0]["path"]
    record = json.loads(receipt.read_text())
    record["output"] = "fabricated"
    receipt.write_text(json.dumps(record))
    application["receipts"][0]["sha256"] = sha256_file(receipt)
    with pytest.raises(ValueError, match="proof drifted|receipt replaced"):
        transactions.verify_application(tmp_path, application, expected_digest)


def test_full_chain_recompute_rejects_wrong_expected_digest(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    application = transactions.run_transaction(
        tmp_path,
        manifest,
        {"include/test.h": "typedef unsigned short Test;\n"},
        runner=_runner(),
    )
    trusted_digest = application["digest"]
    application["semantics"] = {"status": "resolved", "contract": {"role": "fake"}}
    facts = {
        key: item
        for key, item in application.items()
        if key not in {"attestation", "digest"}
    }
    application["digest"] = digest(facts)
    with pytest.raises(ValueError, match="attestation is not trusted"):
        transactions.verify_application(tmp_path, application, trusted_digest)


def test_runner_mutating_allowed_path_fails_without_deleting_substitute(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    header = tmp_path / "include/test.h"
    before = header.read_bytes()
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))

    def mutate(argv, **kwargs):
        header.write_text("runner changed this\n")
        return subprocess.CompletedProcess(argv, 0, "", "")

    with pytest.raises(RuntimeError, match="rollback failed"):
        transactions.run_transaction(
            tmp_path,
            manifest,
            {"include/test.h": "typedef unsigned short Test;\n"},
            runner=mutate,
        )
    assert header.read_bytes() == b"runner changed this\n"
    quarantines = list(
        (tmp_path / transaction_quarantine.QUARANTINE_DIRECTORY).iterdir()
    )
    assert any(path.read_bytes() == before for path in quarantines)


def test_runner_receipt_binds_argv_exit_selector_and_post_state(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    application = transactions.run_transaction(
        tmp_path,
        manifest,
        {"include/test.h": "typedef unsigned short Test;\n"},
        runner=_runner(),
    )
    record = json.loads(
        (tmp_path / application["receipts"][0]["path"]).read_text(encoding="utf-8")
    )
    assert record["argv"] == manifest["required_checks"][0]["argv"]
    assert record["exit_code"] == 0
    assert record["target"] == TARGET
    assert record["selector"] is None
    assert record["post_state_digest"] == application["post_state_digest"]


def test_invalid_second_change_does_not_write_first(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    header = tmp_path / "include/test.h"
    source = tmp_path / "src/test/func_80100000.c"
    before = (header.read_bytes(), source.read_bytes())
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    original_write_text = Path.write_text
    writes = []

    def record_write(path, content, *args, **kwargs):
        if path in {header, source}:
            writes.append(path)
        return original_write_text(path, content, *args, **kwargs)

    monkeypatch.setattr(Path, "write_text", record_write)
    with pytest.raises(ValueError, match="content must be text"):
        transactions.run_transaction(
            tmp_path,
            manifest,
            {"include/test.h": "changed\n", "src/test/func_80100000.c": None},
        )

    assert not writes
    assert (header.read_bytes(), source.read_bytes()) == before


def test_second_write_failure_restores_only_changed_paths(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    header = tmp_path / "include/test.h"
    source = tmp_path / "src/test/func_80100000.c"
    before = (header.read_bytes(), source.read_bytes())
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    original = runtime.install_image
    original_unlink = runtime._safe_unlink
    writes = 0
    unlink_calls: list[str] = []

    def record_unlink(root, name, **kwargs):
        unlink_calls.append(name)
        return original_unlink(root, name, **kwargs)

    def fail_second_write(root, name, content, **kwargs):
        nonlocal writes
        if name in manifest["allowed_paths"]:
            writes += 1
            if writes == 2:
                raise OSError("write failed")
        return original(root, name, content, **kwargs)

    monkeypatch.setattr(runtime, "install_image", fail_second_write)
    monkeypatch.setattr(runtime, "_safe_unlink", record_unlink)
    with pytest.raises(OSError, match="write failed"):
        transactions.run_transaction(
            tmp_path,
            manifest,
            {
                "include/test.h": "changed\n",
                "src/test/func_80100000.c": "changed\n",
            },
        )

    assert (header.read_bytes(), source.read_bytes()) == before
    assert unlink_calls.count("src/test/func_80100000.c") == 1


def test_failed_check_rolls_back(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    before = (tmp_path / "include/test.h").read_bytes()
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    with pytest.raises(RuntimeError, match="validation failed"):
        transactions.run_transaction(
            tmp_path, manifest, {"include/test.h": "bad\n"}, runner=_runner(1)
        )
    assert (tmp_path / "include/test.h").read_bytes() == before


def test_failed_check_preserves_unowned_runner_side_effect(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path, git=True)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))

    def side_effect(argv, **kwargs):
        (tmp_path / "src/test/func_80100000.c").write_text("runner changed this\n")
        return subprocess.CompletedProcess(argv, 1, "", "failed")

    original = (tmp_path / "include/test.h").read_bytes()
    with pytest.raises(RuntimeError, match="external state preserved"):
        transactions.run_transaction(
            tmp_path, manifest, {"include/test.h": "bad\n"}, runner=side_effect
        )
    assert (
        tmp_path / "src/test/func_80100000.c"
    ).read_text() == "runner changed this\n"
    assert (tmp_path / "include/test.h").read_bytes() == original


def test_rollback_failure_is_reported(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    monkeypatch.setattr(
        transactions,
        "rollback",
        lambda *_: (_ for _ in ()).throw(
            RuntimeError("type transaction rollback failed")
        ),
    )
    with pytest.raises(RuntimeError, match="rollback failed"):
        transactions.run_transaction(
            tmp_path,
            manifest,
            {"include/test.h": "bad\n"},
            runner=_runner(1),
        )
    (tmp_path / "include/test.h").write_text("drift\n")
    with pytest.raises(ValueError, match="base drifted|cannot be re-derived"):
        transactions.run_transaction(
            tmp_path, manifest, {"include/test.h": "x"}, runner=_runner()
        )


def test_write_rollback_failure_is_reported(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    original = runtime.install_image
    writes = 0

    def fail_second_write(root, name, content, **kwargs):
        nonlocal writes
        if name in manifest["allowed_paths"]:
            writes += 1
            if writes == 2:
                raise OSError("write failed")
        return original(root, name, content, **kwargs)

    monkeypatch.setattr(runtime, "install_image", fail_second_write)
    monkeypatch.setattr(
        runtime,
        "restore_quarantined",
        lambda *_args, **_kwargs: (_ for _ in ()).throw(OSError("restore failed")),
    )
    with pytest.raises(RuntimeError, match="rollback failed"):
        transactions.run_transaction(
            tmp_path,
            manifest,
            {
                "include/test.h": "changed\n",
                "src/test/func_80100000.c": "changed\n",
            },
        )


def _private_proof(
    root: Path, name: str, target: str, role: str = "same"
) -> dict[str, str]:
    expected_digest = digest({"private": name})
    value = {
        "target": target,
        "concern": "layout",
        "representation": {"size": 8, "fields": [[0, 4], [4, 4]]},
        "semantics": {"role": role},
    }
    path = root / f"out/reviews/{name}.json"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps({"application": value}))
    return {
        "path": path.relative_to(root).as_posix(),
        "target": target,
        "expected_envelope_digest": expected_digest,
    }


def _verify_private(
    _root: Path, proof: dict[str, Any], expected_digest: str
) -> dict[str, str]:
    assert expected_digest != proof.get("digest")
    return {"target": proof["application"]["target"], "digest": expected_digest}


def _private_manifests() -> dict[str, object]:
    return {"exe/a": object(), "exe/b": object()}


def _verified_private_proof(
    root: Path, name: str, target: str, receipt_index: int
) -> dict[str, str]:
    header = f"include/{name}.h"
    (root / header).parent.mkdir(parents=True, exist_ok=True)
    (root / header).write_text("typedef unsigned int SharedTest;\n")
    state = transactions.file_state(root, {header})
    check = {
        "command": f"private proof check {name}",
        "argv": ["python3", "-c", "import sys; sys.exit(0)"],
        "target": target,
        "selector": None,
    }
    receipts, passed = transactions.run_checks(
        root, [check], digest(state), runner=_runner(), start_index=receipt_index
    )
    assert passed
    manifest_facts = {
        "schema": transactions.MANIFEST_SCHEMA,
        "header": header,
        "affected_functions": [],
        "allowed_paths": [header],
        "pre_state": state,
        "required_checks": [check],
    }
    manifest = {**manifest_facts, "digest": digest(manifest_facts)}
    application = transactions.application_record(
        manifest["digest"], state, state, [], receipts
    )
    application.update(
        target=target,
        concern="layout",
        representation={"size": 8, "fields": [[0, 4], [4, 4]]},
        semantics={"role": "same"},
        manifest=manifest,
        receipts=receipts,
        applied=True,
    )
    application_facts = {
        key: item for key, item in application.items() if key != "digest"
    }
    application["digest"] = digest(application_facts)
    application["attestation"] = transactions.write_attestation(root, application)
    path = root / f"out/reviews/{name}.json"
    path.write_text(json.dumps(application))
    return {
        "path": path.relative_to(root).as_posix(),
        "target": target,
        "expected_application_digest": application["digest"],
    }


def test_shared_header_rejects_unrelated_include_and_internal_path(
    tmp_path: Path,
) -> None:
    class Manifest:
        headers = ()

    for name in ("unrelated.h", "include/test_internal.h"):
        path = tmp_path / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("typedef int Test;\n")
        with pytest.raises(ValueError, match="sanctioned public shared path"):
            transactions._header(tmp_path, Manifest(), "shared", name)


def test_shared_header_requires_every_candidate_location_and_proof_dependency(
    tmp_path: Path, monkeypatch
) -> None:
    header = "include/shared/test.h"
    (tmp_path / header).parent.mkdir(parents=True)
    (tmp_path / header).write_text("typedef int Test;\n")
    reviewed = [
        {"candidate": {"locations": [header]}},
        {"candidate": {"locations": [header]}},
    ]
    proofs = [{"target": "exe/a"}, {"target": "exe/b"}]
    monkeypatch.setattr(
        type_shared_proofs,
        "proof_dependencies",
        lambda _root, proof: {header} if proof["target"] == "exe/a" else set(),
    )
    with pytest.raises(ValueError, match="private proof dependency for every owner"):
        type_shared_proofs.validate_shared_header(tmp_path, header, reviewed, proofs)
    reviewed[1]["candidate"]["locations"] = []
    with pytest.raises(ValueError, match="reviewed candidate location for every owner"):
        type_shared_proofs.validate_shared_header(tmp_path, header, reviewed, proofs)


def test_legacy_private_integrity_pins_reject_shared_promotion(tmp_path: Path) -> None:
    pins = [
        _verified_private_proof(tmp_path, "a", "exe/a", 10),
        _verified_private_proof(tmp_path, "b", "exe/b", 20),
    ]
    for pin in pins:
        app = json.loads((tmp_path / pin["path"]).read_text())
        assert transactions.verify_application(
            tmp_path, app, pin["expected_application_digest"]
        )["applied"]
    with pytest.raises(ValueError, match="proof pin is invalid"):
        transactions._private_proofs(tmp_path, pins, _private_manifests())


def test_replaced_full_private_proof_chain_is_rejected(tmp_path: Path) -> None:
    pins = [
        _verified_private_proof(tmp_path, "a", "exe/a", 30),
        _verified_private_proof(tmp_path, "b", "exe/b", 40),
    ]
    trusted_digest = pins[1]["expected_application_digest"]
    replacement = _verified_private_proof(tmp_path, "b", "exe/b", 50)
    assert replacement["expected_application_digest"] != trusted_digest
    with pytest.raises(ValueError, match="proof pin is invalid"):
        transactions._private_proofs(tmp_path, pins, _private_manifests())


def test_private_proof_pins_require_unique_target_and_digest(
    tmp_path: Path, monkeypatch
) -> None:
    monkeypatch.setattr(
        "harness.types.application.verify_reviewed_application",
        _verify_private,
    )
    pins = [
        _private_proof(tmp_path, "a", "exe/a"),
        _private_proof(tmp_path, "b", "exe/a"),
    ]
    with pytest.raises(ValueError, match="targets must be unique"):
        transactions._private_proofs(tmp_path, pins, _private_manifests())
    pins[1] = _private_proof(tmp_path, "b", "exe/b")
    pins[1]["expected_envelope_digest"] = pins[0]["expected_envelope_digest"]
    with pytest.raises(ValueError, match="digests must be unique"):
        transactions._private_proofs(tmp_path, pins, _private_manifests())


def test_private_proof_path_is_confined_to_reviews(tmp_path: Path, monkeypatch) -> None:
    monkeypatch.setattr(
        "harness.types.application.verify_reviewed_application",
        _verify_private,
    )
    outside = _private_proof(tmp_path, "a", "exe/a")
    outside_path = tmp_path / "outside.json"
    outside_path.write_bytes((tmp_path / outside["path"]).read_bytes())
    outside["path"] = "outside.json"
    pins = [outside, _private_proof(tmp_path, "b", "exe/b")]
    with pytest.raises(ValueError, match="path is invalid"):
        transactions._private_proofs(tmp_path, pins, _private_manifests())


def test_shared_proofs_require_two_equal_private_contracts(
    tmp_path: Path, monkeypatch
) -> None:
    monkeypatch.setattr(
        "harness.types.application.verify_reviewed_application",
        _verify_private,
    )
    pins = [
        _private_proof(tmp_path, "a", "exe/a"),
        _private_proof(tmp_path, "b", "exe/b"),
    ]
    proofs = transactions._private_proofs(tmp_path, pins, _private_manifests())
    assert [proof["expected_envelope_digest"] for proof in proofs] == [
        pin["expected_envelope_digest"] for pin in pins
    ]
    second = tmp_path / pins[1]["path"]
    second.write_text(
        json.dumps(
            {
                "application": {
                    "target": "exe/b",
                    "concern": "layout",
                    "representation": {"size": 12},
                    "semantics": {"role": "same"},
                }
            }
        )
    )
    with pytest.raises(ValueError, match="contracts differ"):
        transactions._private_proofs(tmp_path, pins, _private_manifests())


def test_shared_proof_rejects_target_address(tmp_path: Path, monkeypatch) -> None:
    monkeypatch.setattr(
        "harness.types.application.verify_reviewed_application",
        _verify_private,
    )
    pins = [
        _private_proof(tmp_path, "a", "exe/a", "table at 0x80100000"),
        _private_proof(tmp_path, "b", "exe/b", "table at 0x80100000"),
    ]
    with pytest.raises(ValueError, match="target-local address"):
        transactions._private_proofs(tmp_path, pins, _private_manifests())


def test_candidate_account_zero_safe_remains_valid(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    report = transactions.candidate_account(tmp_path)
    assert report["safe_application_count"] == 0
    assert report["counts"] == {"blocked": 1}
    assert transactions.validate_account(tmp_path, report) == report


def test_prepare_rejects_escape_leaf_and_parent_symlinks(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    request = _request(tmp_path)
    request["header"] = "../escape.h"
    with pytest.raises(ValueError, match="repo-relative|existing repo-relative"):
        transactions.prepare_transaction(tmp_path, request)

    header = tmp_path / "include/test.h"
    outside = tmp_path / "outside.h"
    outside.write_text("outside\n")
    header.unlink()
    header.symlink_to(outside)
    request["header"] = "include/test.h"
    with pytest.raises(ValueError, match="existing repo-relative"):
        transactions.prepare_transaction(tmp_path, request)

    header.unlink()
    (tmp_path / "include").rmdir()
    outside_dir = tmp_path / "outside"
    outside_dir.mkdir()
    (outside_dir / "test.h").write_text("outside\n")
    (tmp_path / "include").symlink_to(outside_dir, target_is_directory=True)
    with pytest.raises(ValueError, match="existing repo-relative"):
        transactions.prepare_transaction(tmp_path, request)


def test_run_rejects_redigested_manifest_empty_checks_and_arbitrary_argv(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    for mutate in (
        lambda value: value.update(concern="field"),
        lambda value: value.update(required_checks=[]),
        lambda value: value["required_checks"][0].update(argv=["rm", "-rf", "."]),
    ):
        forged = json.loads(json.dumps(manifest))
        mutate(forged)
        facts = {key: item for key, item in forged.items() if key != "digest"}
        forged["digest"] = digest(facts)
        with pytest.raises(ValueError, match="not canonical"):
            transactions.run_transaction(
                tmp_path, forged, {"include/test.h": "changed\n"}
            )


def test_workspace_state_ignores_tool_owned_quarantine(tmp_path: Path) -> None:
    _repo(tmp_path, git=True)
    quarantine = tmp_path / "out/reviews/evidence/quarantine/recovery"
    quarantine.parent.mkdir(parents=True)
    quarantine.write_bytes(b"recoverable user content\n")

    assert transactions._workspace_state(tmp_path) == {}
    assert all(
        not name.startswith(transaction_quarantine.QUARANTINE_DIRECTORY)
        for name in transaction_git.workspace_backup(tmp_path)
    )


def test_workspace_state_parses_nul_paths_renames_and_content(tmp_path: Path) -> None:
    _repo(tmp_path, git=True)
    spaced = tmp_path / 'name with "quotes".txt'
    spaced.write_text("one\n")
    first = transactions._workspace_state(tmp_path)
    assert first[spaced.name]["sha256"] == sha256_file(spaced)
    spaced.write_text("two\n")
    second = transactions._workspace_state(tmp_path)
    assert second[spaced.name]["sha256"] != first[spaced.name]["sha256"]

    tracked = tmp_path / "tracked name.txt"
    tracked.write_text("tracked\n")
    subprocess.run(["git", "add", tracked.name], cwd=tmp_path, check=True)
    subprocess.run(
        ["git", "-c", "user.name=T", "-c", "user.email=t@x", "commit", "-qm", "path"],
        cwd=tmp_path,
        check=True,
    )
    renamed = 'renamed "path".txt'
    subprocess.run(["git", "mv", tracked.name, renamed], cwd=tmp_path, check=True)
    state = transactions._workspace_state(tmp_path)
    assert renamed in state
    assert state[renamed]["sha256"] == sha256_file(tmp_path / renamed)


def _partial_common() -> dict[str, object]:
    return {
        "status": "different",
        "exact_match": False,
        "byte_match": False,
        "source": "src/test/func_80100000.c",
        "function": "func_80100000",
        "address": "0x80100000",
        "original_size": 8,
        "current_size": 8,
        "size_delta": 0,
        "original_binary": "out/test.bin",
        "current_object": "build/test.o",
        "outputs": {},
    }


def test_exact_checks_require_success_and_partial_checks_preserve_baseline(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    exact = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    with pytest.raises(RuntimeError, match="validation failed"):
        transactions.run_transaction(
            tmp_path,
            exact,
            {"include/test.h": "exact failure\n"},
            runner=_runner(3),
        )

    connection = _connect(tmp_path)
    connection.execute("UPDATE functions SET lift_status='partial'")
    connection.commit()
    connection.close()
    common = _partial_common()
    outputs = {
        "bin/asm-diff": json.dumps(
            {
                "schema": "harness.asm-diff-one/v2",
                **common,
                "outputs": {
                    "directory": "out/diff",
                    "summary": "out/diff/summary.json",
                    "diff": "out/diff/diff.patch",
                    "original": "out/diff/original.s",
                    "current": "out/diff/current.s",
                    "compiler": "out/diff/compiler.s",
                    "original_bytes": "out/diff/original.bin",
                    "build_log": "out/diff/build.log",
                    "linked": "out/diff/linked.s",
                },
                "instruction_count": {
                    "original": 2,
                    "current": 2,
                    "matching": 1,
                    "match_percent": 50.0,
                },
                "first_mismatch": {
                    "original_index": 1,
                    "current_index": 1,
                    "original_offset": 4,
                    "current_offset": 4,
                    "original": "old",
                    "current": "new",
                },
            }
        ),
        "bin/byte-match": json.dumps({"schema": "harness.byte-match-one/v1", **common}),
    }

    from harness.common.checks import _baseline_facts, _check

    baselines = []
    for tool in ("asm-diff", "byte-match"):
        check = _check(
            tool,
            TARGET,
            f"{TARGET}@0x80100000",
            "func_80100000",
            "src/test/func_80100000.c",
        )
        facts = _baseline_facts(check, 1, outputs[check["argv"][0]], tmp_path)
        baselines.append({**facts, "digest": digest(facts)})

    def partial_runner(argv, **_kwargs):
        if argv[0] in {"bin/splat", "bin/build"}:
            return subprocess.CompletedProcess(argv, 0, "", "")
        return subprocess.CompletedProcess(argv, 1, outputs[argv[0]], "")

    monkeypatch.setattr(
        transactions,
        "capture_partial_baselines",
        lambda *_args, **_kwargs: {f"{TARGET}@0x80100000": baselines},
    )
    partial = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    transactions.run_transaction(
        tmp_path,
        partial,
        {"include/test.h": "partial unchanged\n"},
        runner=partial_runner,
    )


def test_partial_baseline_rejects_empty_missing_swapped_and_unselected_changes(
    tmp_path: Path,
) -> None:
    from harness.common.checks import _baseline_facts, _check, check_evidence

    selector = f"{TARGET}@0x80100000"
    check = _check(
        "byte-match",
        TARGET,
        selector,
        "func_80100000",
        "src/test/func_80100000.c",
    )
    payload = {"schema": "harness.byte-match-one/v1", **_partial_common()}
    raw = json.dumps(payload, sort_keys=True)
    facts = _baseline_facts(check, 1, raw, tmp_path)
    check["partial_baseline"] = {**facts, "digest": digest(facts)}
    assert check_evidence(check, 1, raw, tmp_path)["passed"]
    for changed in (
        {},
        {**payload, "function": "other"},
        {**payload, "outputs": {"unselected": "two"}},
    ):
        assert not check_evidence(
            check, 1, json.dumps(changed, sort_keys=True), tmp_path
        )["passed"]


@pytest.mark.parametrize("tool", ["asm-diff", "byte-match"])
def test_partial_evidence_rejects_every_malformed_field(
    tool: str, tmp_path: Path
) -> None:
    import copy

    from harness.common.checks import _baseline_facts, _check

    selector = f"{TARGET}@0x80100000"
    check = _check(
        tool,
        TARGET,
        selector,
        "func_80100000",
        "src/test/func_80100000.c",
    )
    payload = {"schema": "harness.byte-match-one/v1", **_partial_common()}
    if tool == "asm-diff":
        payload.update(
            schema="harness.asm-diff-one/v2",
            outputs={
                "directory": "out/diff",
                "summary": "out/diff/summary.json",
                "diff": "out/diff/diff.patch",
                "original": "out/diff/original.s",
                "current": "out/diff/current.s",
                "compiler": "out/diff/compiler.s",
                "original_bytes": "out/diff/original.bin",
                "build_log": "out/diff/build.log",
            },
            instruction_count={
                "original": 2,
                "current": 2,
                "matching": 1,
                "match_percent": 50.0,
            },
            first_mismatch={
                "original_index": 1,
                "current_index": 1,
                "original_offset": 4,
                "current_offset": 4,
                "original": "old",
                "current": "new",
            },
        )
    raw = json.dumps(payload, sort_keys=True)
    assert _baseline_facts(check, 1, raw, tmp_path)["function"] == "func_80100000"

    malformed = []
    for key in payload:
        missing = copy.deepcopy(payload)
        missing.pop(key)
        malformed.append(missing)
        wrong = copy.deepcopy(payload)
        wrong[key] = None
        malformed.append(wrong)
    malformed += [
        {**payload, "unexpected": True},
        {**payload, "status": "exact_match"},
        {**payload, "exact_match": True},
        {**payload, "byte_match": True},
        {**payload, "function": "other"},
        {**payload, "source": "src/test/other.c"},
        {**payload, "address": "0x80100004"},
        {**payload, "original_size": True},
        {**payload, "current_size": -1},
        {**payload, "size_delta": 1},
    ]
    if tool == "asm-diff":
        malformed += [
            {
                **payload,
                "instruction_count": {**payload["instruction_count"], "matching": 3},
            },
            {
                **payload,
                "instruction_count": {
                    **payload["instruction_count"],
                    "match_percent": 49.0,
                },
            },
            {
                **payload,
                "first_mismatch": {**payload["first_mismatch"], "original_index": 2},
            },
            {
                **payload,
                "first_mismatch": {**payload["first_mismatch"], "current_offset": 8},
            },
            {
                **payload,
                "first_mismatch": {**payload["first_mismatch"], "current": None},
            },
        ]
    for value in malformed:
        with pytest.raises(ValueError, match="partial"):
            _baseline_facts(check, 1, json.dumps(value, sort_keys=True), tmp_path)
    for exit_code in (0, 2, True, "1"):
        with pytest.raises(ValueError, match="exit 1"):
            _baseline_facts(check, exit_code, raw, tmp_path)


def test_partial_receipt_preserves_full_raw_json(tmp_path: Path) -> None:
    from harness.common.checks import _baseline_facts, _check

    check = _check(
        "byte-match",
        TARGET,
        f"{TARGET}@0x80100000",
        "func_80100000",
        "src/test/func_80100000.c",
    )
    raw = json.dumps({"schema": "harness.byte-match-one/v1", **_partial_common()})
    facts = _baseline_facts(check, 1, raw, tmp_path)
    check["partial_baseline"] = {**facts, "digest": digest(facts)}

    def runner(argv, **_kwargs):
        return subprocess.CompletedProcess(argv, 1, raw, "stderr must not alter JSON")

    receipts, passed = transactions.run_checks(
        tmp_path, [check], digest({}), runner=runner
    )
    assert passed
    assert receipts[0]["output"] == raw
    assert receipts[0]["evidence_digest"] == digest(facts)


# Real asm-diff/byte-match JSON captured live from
#   bin/asm-diff --json --detail full
#   bin/byte-match --json
# for the partial lifts clampPaletteChannels (BIN/SCENARIO/SCENA16.EMI#0@0x801F83B0)
# and drawTexturedFrame (BIN/WORLD00/AREA008.EMI#13@0x801F3D88): the tools emit
# an absolute source path and a lowercase hex address.  The {root} prefix is
# bound to the test repository root at runtime.
_REAL_ASM_SCENA16 = {
    "schema": "harness.asm-diff-one/v2",
    "status": "different",
    "exact_match": False,
    "byte_match": False,
    "source": "{root}/src/bof3/scenario/clampPaletteChannels.c",
    "function": "clampPaletteChannels",
    "address": "0x801f83b0",
    "original_size": 172,
    "current_size": 172,
    "size_delta": 0,
    "original_binary": "{root}/out/binaries/emi/scenario/scena16/00.bin",
    "current_object": "{root}/build/src/bof3/scenario/clampPaletteChannels.o",
    "instruction_count": {
        "original": 43,
        "current": 43,
        "matching": 21,
        "match_percent": 48.84,
    },
    "first_mismatch": {
        "original_index": None,
        "current_index": 5,
        "original_offset": None,
        "current_offset": 20,
        "original": None,
        "current": "lui t6,0x8003",
    },
    "outputs": {
        "directory": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels",
        "summary": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/summary.json",
        "diff": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/diff.patch",
        "original": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/original.s",
        "current": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/current.s",
        "compiler": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/compiler.s",
        "original_bytes": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/original.bin",
        "build_log": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/build.log",
        "linked": "{root}/out/asm-diff/emi_scenario_scena16_00/clampPaletteChannels/linked.s",
    },
}
_REAL_BYTE_SCENA16 = {
    "schema": "harness.byte-match-one/v1",
    "status": "different",
    "exact_match": False,
    "byte_match": False,
    "source": "{root}/src/bof3/scenario/clampPaletteChannels.c",
    "function": "clampPaletteChannels",
    "address": "0x801f83b0",
    "original_size": 172,
    "current_size": 172,
    "size_delta": 0,
    "original_binary": "{root}/out/binaries/emi/scenario/scena16/00.bin",
    "current_object": "{root}/build/src/bof3/scenario/clampPaletteChannels.o",
    "outputs": {},
}
_REAL_ASM_AREA008 = {
    "schema": "harness.asm-diff-one/v2",
    "status": "different",
    "exact_match": False,
    "byte_match": False,
    "source": "{root}/src/bof3/world/drawTexturedFrame.c",
    "function": "drawTexturedFrame",
    "address": "0x801f3d88",
    "original_size": 1356,
    "current_size": 1356,
    "size_delta": 0,
    "original_binary": "{root}/out/binaries/emi/world00/area008/13.bin",
    "current_object": "{root}/build/src/bof3/world/drawTexturedFrame.o",
    "instruction_count": {
        "original": 339,
        "current": 339,
        "matching": 326,
        "match_percent": 96.17,
    },
    "first_mismatch": {
        "original_index": None,
        "current_index": 85,
        "original_offset": None,
        "current_offset": 340,
        "original": None,
        "current": "sb s2,37(s0)",
    },
    "outputs": {
        "directory": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame",
        "summary": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/summary.json",
        "diff": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/diff.patch",
        "original": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/original.s",
        "current": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/current.s",
        "compiler": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/compiler.s",
        "original_bytes": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/original.bin",
        "build_log": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/build.log",
        "linked": "{root}/out/asm-diff/emi_world00_area008_13/drawTexturedFrame/linked.s",
    },
}
_REAL_BYTE_AREA008 = {
    "schema": "harness.byte-match-one/v1",
    "status": "different",
    "exact_match": False,
    "byte_match": False,
    "source": "{root}/src/bof3/world/drawTexturedFrame.c",
    "function": "drawTexturedFrame",
    "address": "0x801f3d88",
    "original_size": 1356,
    "current_size": 1356,
    "size_delta": 0,
    "original_binary": "{root}/out/binaries/emi/world00/area008/13.bin",
    "current_object": "{root}/build/src/bof3/world/drawTexturedFrame.o",
    "outputs": {},
}


def _real_payload(payload: dict[str, object], root: Path) -> dict[str, object]:
    return {
        key: (
            str(value).replace("{root}", str(root)) if isinstance(value, str) else value
        )
        for key, value in payload.items()
    }


def test_partial_baseline_captures_and_verifies_real_tool_identity(
    tmp_path: Path,
) -> None:
    """Real asm-diff/byte-match identity shapes capture and verify; forgeries
    (outside source, wrong source, symlinked source, wrong address) fail.
    """

    from harness.common.checks import (
        _baseline_facts,
        _check,
        capture_partial_baselines,
        required_checks,
    )

    for relative in (
        "src/bof3/scenario/clampPaletteChannels.c",
        "src/bof3/world/drawTexturedFrame.c",
    ):
        source = tmp_path / relative
        source.parent.mkdir(parents=True, exist_ok=True)
        source.write_text("/* @source 0x80100000 */\n", encoding="utf-8")

    functions = [
        {
            "selector": "scena16@0x801F83B0",
            "target": "scena16",
            "function": "clampPaletteChannels",
            "source": "src/bof3/scenario/clampPaletteChannels.c",
            "status": "partial",
        },
        {
            "selector": "area008/13@0x801F3D88",
            "target": "area008/13",
            "function": "drawTexturedFrame",
            "source": "src/bof3/world/drawTexturedFrame.c",
            "status": "partial",
        },
    ]
    responses = {
        "scena16@0x801F83B0": {
            "bin/asm-diff": _real_payload(_REAL_ASM_SCENA16, tmp_path),
            "bin/byte-match": _real_payload(_REAL_BYTE_SCENA16, tmp_path),
        },
        "area008/13@0x801F3D88": {
            "bin/asm-diff": _real_payload(_REAL_ASM_AREA008, tmp_path),
            "bin/byte-match": _real_payload(_REAL_BYTE_AREA008, tmp_path),
        },
    }

    def real_runner(argv, **_kwargs):
        if argv[0] in {"bin/splat", "bin/build"}:
            return subprocess.CompletedProcess(argv, 0, "", "")
        return subprocess.CompletedProcess(
            argv, 1, json.dumps(responses[argv[1]][argv[0]]), ""
        )

    baselines = capture_partial_baselines(tmp_path, functions, runner=real_runner)
    assert set(baselines) == {"scena16@0x801F83B0", "area008/13@0x801F3D88"}
    checks = required_checks(["scena16", "area008/13"], functions, baselines)
    receipts, passed = runtime.run_checks(
        tmp_path, checks, digest({}), runner=real_runner
    )
    assert passed
    assert len(receipts) == len(checks)
    assert all(receipt["status"] == "passed" for receipt in receipts)

    check = _check(
        "byte-match",
        "scena16",
        "scena16@0x801F83B0",
        "clampPaletteChannels",
        "src/bof3/scenario/clampPaletteChannels.c",
    )
    byte = _real_payload(_REAL_BYTE_SCENA16, tmp_path)
    assert _baseline_facts(check, 1, json.dumps(byte), tmp_path)["address"] == (
        "0x801f83b0"
    )
    # Hex-case-insensitive address identity is accepted, not loosened.
    assert (
        _baseline_facts(
            check, 1, json.dumps({**byte, "address": "0x801F83B0"}), tmp_path
        )["address"]
        == "0x801F83B0"
    )

    alias = tmp_path / "src/bof3/scenario/alias.c"
    alias.symlink_to(tmp_path / "src/bof3/scenario/clampPaletteChannels.c")
    outside = tmp_path.parent / f"{tmp_path.name}-alias.c"
    outside.symlink_to(tmp_path / "src/bof3/scenario/clampPaletteChannels.c")
    linked = tmp_path / "src/bof3/scenario/linked"
    linked.symlink_to(tmp_path / "src/bof3/scenario", target_is_directory=True)
    check_linked = _check(
        "byte-match",
        "scena16",
        "scena16@0x801F83B0",
        "clampPaletteChannels",
        "src/bof3/scenario/linked/clampPaletteChannels.c",
    )
    for forged in (
        {**byte, "source": str(tmp_path.parent / "outside.c")},
        {
            **byte,
            "source": str(tmp_path / "src/bof3/world/drawTexturedFrame.c"),
        },
        {**byte, "source": str(alias)},
        {**byte, "source": str(outside)},
        {**byte, "address": "0x801f83b4"},
    ):
        with pytest.raises(ValueError, match="partial"):
            _baseline_facts(check, 1, json.dumps(forged), tmp_path)
    # External symlink alias reached by a relative .. escape is also rejected.
    with pytest.raises(ValueError, match="partial"):
        _baseline_facts(
            check,
            1,
            json.dumps(
                {
                    **byte,
                    "source": f"../{tmp_path.name}-alias.c",
                }
            ),
            tmp_path,
        )
    # In-repo symlinked directory with an exactly matching lexical path fails.
    with pytest.raises(ValueError, match="partial"):
        _baseline_facts(
            check_linked,
            1,
            json.dumps({**byte, "source": str(linked / "clampPaletteChannels.c")}),
            tmp_path,
        )
    outside.unlink()


def test_forged_allowed_paths_and_swap_before_write_fail_closed(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    forged = dict(manifest)
    forged["allowed_paths"] = [*manifest["allowed_paths"], "../escape.h"]
    facts = {key: item for key, item in forged.items() if key != "digest"}
    forged["digest"] = digest(facts)
    with pytest.raises(ValueError, match="allowed_paths|repo-relative|not canonical"):
        transactions.run_transaction(tmp_path, forged, {"include/test.h": "x\n"})

    outside = tmp_path / "outside.h"
    outside.write_text("outside\n")
    original = runtime.install_image
    swapped = False

    def swap(root, name, content, **kwargs):
        nonlocal swapped
        if name == "include/test.h" and not swapped:
            swapped = True
            (root / name).symlink_to(outside)
        return original(root, name, content, **kwargs)

    monkeypatch.setattr(runtime, "install_image", swap)
    with pytest.raises((ValueError, RuntimeError), match="unsafe|rollback"):
        transactions.run_transaction(
            tmp_path, manifest, {"include/test.h": "changed\n"}, runner=_runner()
        )
    assert outside.read_text() == "outside\n"


def test_rollback_rejects_leaf_symlink(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    header = tmp_path / "include/test.h"
    outside = tmp_path / "outside.h"
    outside.write_text("outside\n")

    def attack(argv, **kwargs):
        header.unlink()
        header.symlink_to(outside)
        return subprocess.CompletedProcess(argv, 1, "", "failed")

    with pytest.raises(RuntimeError, match="rollback failed"):
        transactions.run_transaction(
            tmp_path, manifest, {"include/test.h": "changed\n"}, runner=attack
        )
    assert outside.read_text() == "outside\n"


@pytest.mark.parametrize("key", ["schema", "verdict", "reviewer"])
def test_prepare_rejects_duplicate_review_keys_before_writes(
    tmp_path: Path, monkeypatch, key: str
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    request = _request(tmp_path)
    path = tmp_path / request["candidate_artifacts"][0]
    # Last-value parsing would retain the valid digest and accepted review.
    path.write_text(
        path.read_text().replace(f'"{key}":', f'"{key}": "rejected", "{key}":', 1)
    )
    before = {p: p.read_bytes() for p in tmp_path.rglob("*") if p.is_file()}
    with pytest.raises(ValueError, match="duplicate JSON key"):
        transactions.prepare_transaction(tmp_path, request)
    assert {p: p.read_bytes() for p in tmp_path.rglob("*") if p.is_file()} == before


def test_shared_type_rejects_duplicate_private_proof_keys(tmp_path: Path) -> None:
    pins = [
        _verified_private_proof(tmp_path, "a", "exe/a", 10),
        _verified_private_proof(tmp_path, "b", "exe/b", 20),
    ]
    for pin in pins:
        pin["expected_envelope_digest"] = pin.pop("expected_application_digest")
    path = tmp_path / pins[0]["path"]
    path.write_text(
        path.read_text().replace('"concern":', '"concern": "shared", "concern":', 1)
    )
    with pytest.raises(ValueError, match="duplicate JSON key"):
        transactions._private_proofs(tmp_path, pins, _private_manifests())


@pytest.mark.parametrize("tool", ["bin/asm-diff", "bin/byte-match"])
@pytest.mark.parametrize(
    "fault",
    [
        "warning",
        "missing",
        "unknown",
        "duplicate",
        "malformed",
        "source",
        "binary",
        "size",
        "boolean",
        "receipt",
    ],
)
def test_native_exact_gate_rejects_exit_zero_without_owner_evidence(
    tmp_path: Path, monkeypatch, tool: str, fault: str
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    before = {
        name: (tmp_path / name).read_bytes() for name in manifest["allowed_paths"]
    }
    normal = _runner()

    def run(argv, **kwargs):
        result = normal(argv, **kwargs)
        if argv[0] != tool:
            return result
        payload = exact_payload(tmp_path, tool)
        if fault == "warning":
            payload["status"] = "warning"
        elif fault == "missing":
            del payload["status"]
        elif fault == "unknown":
            payload["unexpected"] = True
        elif fault == "source":
            payload["source"] = "src/other.c"
        elif fault == "binary":
            payload["original_binary"] = "out/other.bin"
        elif fault == "size":
            payload["current_size"] = 12
        elif fault == "boolean":
            payload["exact_match"] = 1
        output = json.dumps(payload)
        if fault == "duplicate":
            output = output.replace('"status":', '"status": "warning", "status":', 1)
        elif fault == "malformed":
            output = "not JSON"
        elif fault == "receipt":
            # Exact stdout must survive stderr and the old 16K tail truncation.
            result.stderr = "diagnostic " * 2000
            result.stdout = output
            return result
        return subprocess.CompletedProcess(argv, 0, output, "")

    if fault == "receipt":
        application = transactions.run_transaction(
            tmp_path, manifest, {"include/test.h": "changed header\n"}, runner=run
        )
        assert transactions.verify_application(
            tmp_path, application, application["digest"]
        )["applied"]
        receipt = next(
            item for item in application["receipts"] if item["argv"][0] == tool
        )
        assert json.loads(receipt["output"])["status"] == "exact_match"
        assert receipt["stderr"] == "diagnostic " * 2000
        path = tmp_path / receipt["path"]
        record = json.loads(path.read_text())
        payload = json.loads(record["output"])
        payload["status"] = "warning"
        record["output"] = json.dumps(payload)
        record["digest"] = digest({k: v for k, v in record.items() if k != "digest"})
        path.write_text(json.dumps(record))
        receipt.update({k: v for k, v in record.items() if k != "schema"})
        receipt["sha256"] = sha256_file(path)
        with pytest.raises(ValueError, match="does not match transaction"):
            runtime.validate_receipts(
                tmp_path,
                application["receipts"],
                manifest["required_checks"],
                application["post_state_digest"],
                receipt_schema=record["schema"],
            )
    else:
        with pytest.raises(RuntimeError, match="validation failed"):
            transactions.run_transaction(
                tmp_path, manifest, {"include/test.h": "changed header\n"}, runner=run
            )
        assert {name: (tmp_path / name).read_bytes() for name in before} == before
        receipts = list((tmp_path / "out/reviews/evidence").glob("*-run-*.json"))
        assert any(
            json.loads(path.read_text())["status"] == "failed" for path in receipts
        )


@pytest.mark.parametrize(
    "drift",
    [
        None,
        "catalog",
        "mode",
        "source",
        "index",
        "tooling",
        "build",
        "publication",
        "launcher",
        "python-config",
        "python-import",
        "ninja-script",
    ],
)
def test_type_execution_context_binds_actual_gates(tmp_path, monkeypatch, drift):
    from transaction_native_fixture import execution_inputs, native_build

    _repo(tmp_path)
    execution_inputs(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    header = tmp_path / "include/test.h"
    header.chmod(0o750)
    original = header.read_bytes()
    native = _runner()
    calls = []

    def mutate():
        paths = {
            "launcher": ".venv/bin/splat",
            "python-config": ".venv/pyvenv.cfg",
            "python-import": ".venv/lib/site-packages/runtime.pth",
            "ninja-script": "build/cmake/CMakeFiles/VerifyGlobs.cmake",
            "catalog": "out/catalog/emi.json",
            "source": "src/new.c",
            "index": "out/index/reverse.sqlite",
            "tooling": "bin/new",
            "build": "build/cmake/build.ninja",
            "publication": "out/catalog/emi.json",
        }
        if drift == "mode":
            (tmp_path / "CMakeLists.txt").chmod(0o600)
        else:
            path = tmp_path / paths[drift]
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"drift")

    def run(argv, **kwargs):
        calls.append(argv)
        if not drift and argv[0] == "bin/build":
            native_build(tmp_path)
        result = native(argv, **kwargs)
        if drift not in {None, "publication"}:
            mutate()
        return result

    if drift == "publication":
        original_attest = transactions.write_attestation

        def attest(*args, **kwargs):
            result = original_attest(*args, **kwargs)
            mutate()
            return result

        monkeypatch.setattr(transactions, "write_attestation", attest)
    if drift:
        with pytest.raises(ValueError, match="execution context"):
            transactions.run_transaction(
                tmp_path,
                manifest,
                {"include/test.h": "typedef unsigned long Test;\n"},
                runner=run,
                implementation_run_id="implementation-1",
            )
        assert header.read_bytes() == original
        assert header.stat().st_mode & 0o7777 == 0o750
        assert len(calls) == (
            len(manifest["required_checks"]) if drift == "publication" else 1
        )
        return
    app = transactions.run_transaction(
        tmp_path,
        manifest,
        {"include/test.h": "typedef unsigned long Test;\n"},
        runner=run,
        implementation_run_id="implementation-1",
    )
    assert (
        app["review_context"]["final_build"]["build/cmake/CMakeFiles/VerifyGlobs.cmake"]
        is not None
    )
    assert header.stat().st_mode & 0o7777 == 0o750
    context = app["review_context"]
    assert context["initial_state"]["inputs"]["out/catalog/emi.json"] is None
    assert (
        context["initial_state"]["inputs"]["include/test.h"]
        != context["final_state"]["inputs"]["include/test.h"]
    )
    assert [g["receipt_digest"] for g in context["gates"]] == app["receipt_digests"]
    assert transactions.verify_application(tmp_path, app, app["digest"])["applied"]
    (tmp_path / "CMakeLists.txt").write_text("changed")
    with pytest.raises(ValueError, match="execution context"):
        transactions.verify_application(tmp_path, app, app["digest"])


@pytest.mark.parametrize("mode", [True, -1, 0o10000, "644"])
def test_atomic_write_rejects_invalid_explicit_mode(tmp_path, mode):
    with pytest.raises(ValueError, match="file mode"):
        transaction_files.atomic_write(tmp_path, "new/file", b"x", mode=mode)
    assert not (tmp_path / "new").exists()


@pytest.mark.parametrize("invalid", ["", " ", False])
def test_type_execution_context_rejects_invalid_run_before_apply(
    tmp_path, monkeypatch, invalid
):
    _repo(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    original = (tmp_path / "include/test.h").read_bytes()
    with pytest.raises(ValueError, match="run ID"):
        transactions.run_transaction(
            tmp_path,
            manifest,
            {"include/test.h": "new"},
            runner=_runner(),
            implementation_run_id=invalid,
        )
    assert (tmp_path / "include/test.h").read_bytes() == original


@pytest.mark.parametrize(
    "field", ["unknown", "gate", "initial", "null", "catalog", "environment", "build"]
)
def test_type_execution_context_rejects_rebound_shape_and_transition(
    tmp_path, monkeypatch, field
):
    from harness.common.execution import validate_context
    from transaction_native_fixture import execution_inputs

    _repo(tmp_path)
    execution_inputs(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    app = transactions.run_transaction(
        tmp_path,
        manifest,
        {"include/test.h": "typedef unsigned long Test;\n"},
        runner=_runner(),
        implementation_run_id="implementation-1",
    )
    ctx = app["review_context"]
    if field == "unknown":
        ctx["unknown"] = True
    elif field == "gate":
        ctx["gates"][0]["unknown"] = True
    elif field == "initial":
        ctx["initial_state"]["inputs"]["include/test.h"]["sha256"] = "0" * 64
    elif field == "null":
        app["review_context"] = None
    elif field == "catalog":
        ctx["initial_state"]["inputs"]["out/catalog/emi.json"] = {
            "sha256": "0" * 64,
            "mode": 0o644,
        }
    elif field == "environment":
        ctx["initial_state"]["environment_digest"] = "forged"
    else:
        ctx["gates"][0]["build_before"]["build/cmake/build.ninja"] = {
            "sha256": "0" * 64,
            "mode": 0o644,
        }
    with pytest.raises(ValueError, match="execution"):
        validate_context(tmp_path, app)


def test_type_cli_execution_id_and_duplicate_input(tmp_path):
    args = type_audit.build_parser().parse_args(
        ["run", "manifest", "changes", "output", "--implementation-run-id", "worker"]
    )
    assert args.implementation_run_id == "worker"
    path = tmp_path / "input.json"
    path.write_text('{"review_context":{"x":1,"x":2}}')
    with pytest.raises(ValueError, match="duplicate"):
        type_audit._read(path)


def test_type_context_concurrent_index_preserves_owned_source_rollback(
    tmp_path, monkeypatch
):
    from transaction_native_fixture import execution_inputs

    _repo(tmp_path, git=True)
    monkeypatch.setattr(transactions, "connect", _connect)
    request = _request(tmp_path)
    execution_inputs(tmp_path)
    (tmp_path / "include/test.h").chmod(0o750)
    request["adopted_baseline"] = transactions.workspace_baseline(tmp_path)["digest"]
    manifest = transactions.prepare_transaction(tmp_path, request)
    header = tmp_path / "include/test.h"
    original = header.read_bytes()
    index = tmp_path / ".git/index"
    concurrent = index.read_bytes() + b"concurrent"

    def reject(*_args):
        index.write_bytes(concurrent)
        raise ValueError("execution context concurrent drift")

    monkeypatch.setattr(transactions.execution_context, "checked", reject)
    with pytest.raises(RuntimeError, match="external state preserved") as error:
        transactions.run_transaction(
            tmp_path,
            manifest,
            {"include/test.h": "changed\n"},
            runner=_runner(),
            implementation_run_id="implementation-1",
        )
    assert "concurrently" in str(error.value.__cause__)
    assert index.read_bytes() == concurrent
    assert header.read_bytes() == original
    assert header.stat().st_mode & 0o7777 == 0o750


@pytest.mark.parametrize(
    "invalid", ["missing-launcher", "empty-launcher", "wrong-python", "system-site"]
)
def test_type_context_rejects_unsupported_splat_runtime(tmp_path, monkeypatch, invalid):
    from transaction_native_fixture import execution_inputs

    _repo(tmp_path)
    execution_inputs(tmp_path)
    monkeypatch.setattr(transactions, "connect", _connect)
    manifest = transactions.prepare_transaction(tmp_path, _request(tmp_path))
    launcher = tmp_path / ".venv/bin/splat"
    if invalid == "missing-launcher":
        launcher.unlink()
    elif invalid == "empty-launcher":
        launcher.write_text("")
    elif invalid == "wrong-python":
        launcher.write_text("#!/usr/bin/env python\n")
    else:
        (tmp_path / ".venv/pyvenv.cfg").write_text(
            "include-system-site-packages = true\n"
        )
    header = tmp_path / "include/test.h"
    original = header.read_bytes()
    with pytest.raises(ValueError, match="execution context"):
        transactions.run_transaction(
            tmp_path,
            manifest,
            {"include/test.h": "changed\n"},
            runner=_runner(),
            implementation_run_id="implementation-1",
        )
    assert header.read_bytes() == original


@pytest.mark.parametrize("failure", [None, "before", "after", "write", "index"])
def test_type_cli_publication_rolls_back_owned_edits(tmp_path, monkeypatch, failure):
    from transaction_native_fixture import execution_inputs

    monkeypatch.setenv("GIT_OPTIONAL_LOCKS", "0")

    _repo(tmp_path, git=True)
    monkeypatch.setattr(transactions, "connect", _connect)
    request = _request(tmp_path)
    execution_inputs(tmp_path)
    header = tmp_path / "include/test.h"
    header.chmod(0o750)
    original = header.read_bytes()
    unrelated = tmp_path / "unrelated.txt"
    unrelated.write_text("adopted dirty work\n")
    request["adopted_baseline"] = transactions.workspace_baseline(tmp_path)["digest"]
    manifest = transactions.prepare_transaction(tmp_path, request)
    inputs = tmp_path / "out/reviews/evidence"
    inputs.mkdir(parents=True, exist_ok=True)
    manifest_path = inputs / "manifest.json"
    manifest_path.write_text(json.dumps(manifest))
    changes_path = inputs / "changes.json"
    changes_path.write_text(json.dumps({"include/test.h": "changed\n"}))
    output = Path("out/reviews/evidence/application.json")
    index = tmp_path / ".git/index"
    original_index = index.read_bytes()
    catalog = tmp_path / "out/catalog/emi.json"
    assert not catalog.exists()
    context = transactions.execution_context
    validate = context.validate_context
    write = context.write_evidence_output
    validations = []

    def validate_publication(root, application):
        validations.append(True)
        if failure == "before" and len(validations) == 1:
            catalog.parent.mkdir(parents=True, exist_ok=True)
            catalog.write_text("concurrent catalog\n")
        validate(root, application)

    def write_publication(root, name, application):
        if failure == "write":
            raise OSError("publication failed")
        write(root, name, application)
        if failure == "after":
            catalog.parent.mkdir(parents=True, exist_ok=True)
            catalog.write_text("concurrent catalog\n")
        if failure == "index":
            index.write_bytes(original_index + b"concurrent")

    monkeypatch.setattr(context, "validate_context", validate_publication)
    monkeypatch.setattr(context, "write_evidence_output", write_publication)
    run = transactions.run_transaction
    monkeypatch.setattr(
        type_audit,
        "run_transaction",
        lambda *args, **kwargs: run(*args, runner=_runner(), **kwargs),
    )
    args = type_audit.build_parser().parse_args(
        [
            "--root",
            str(tmp_path),
            "run",
            str(manifest_path),
            str(changes_path),
            str(output),
            "--implementation-run-id",
            "implementation-1",
        ]
    )
    if failure is None:
        assert type_audit._run(args) == 0
        app = json.loads((tmp_path / output).read_text())
        assert transactions.verify_application(tmp_path, app, app["digest"])["applied"]
        assert header.read_text() == "changed\n"
    else:
        with pytest.raises((ValueError, RuntimeError, OSError)) as error:
            type_audit._run(args)
        assert header.read_bytes() == original
        if failure == "index":
            assert "concurrently" in str(error.value.__cause__)
        elif failure == "write":
            assert "publication failed" in str(error.value)
        else:
            assert "execution context" in str(error.value)
        assert (tmp_path / output).exists() == (failure in {"after", "index"})
        if failure in {"after", "index"}:
            app = json.loads((tmp_path / output).read_text())
            with pytest.raises(ValueError, match="post-state drifted"):
                transactions.verify_application(tmp_path, app, app["digest"])
    assert header.stat().st_mode & 0o7777 == 0o750
    assert unrelated.read_text() == "adopted dirty work\n"
    assert index.read_bytes() == original_index + (
        b"concurrent" if failure == "index" else b""
    )
    if failure in {"before", "after"}:
        assert catalog.read_text() == "concurrent catalog\n"
