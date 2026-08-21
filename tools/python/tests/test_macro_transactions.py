"""Adversarial tests for reviewed macro application transactions."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import sqlite3
import subprocess
from pathlib import Path
from typing import Any

import pytest

from harness.analysis import macro_accounting, macro_transactions, transaction_files
from harness.analysis import type_transaction_runtime as runtime
from harness.analysis.macro_transaction_review import GUARDS, OBSERVATIONS
from harness.analysis.schema import create_schema
from harness.analysis.type_candidate_review import digest
from harness.commands import macro_audit
from harness.domain.receipts import sha256_file

TARGET = "exe/test"
SELECTOR = f"{TARGET}@0x80100000"
SOURCE_TEXT = (
    "/** @source 0x80100000\n * @behavior test lift\n */\n"
    "void func_80100000(void) { int a=17,b=17,c=17; }\n"
)


def _repo(root: Path, *, git: bool = False) -> Path:
    config = root / "config/targets/exe/test/target.toml"
    config.parent.mkdir(parents=True)
    config.write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src/test'\nbinary='test.bin'\nload_address=0x80100000\n"
        "splat='config/targets/exe/test/splat.yaml'\n"
        "sources=['src/test/func_80100000.c']\nheaders=['include/test.h']\n"
    )
    (config.parent / "splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [8]\n"
    )
    (config.parent / "symbols.txt").write_text("func_80100000 = 0x80100000;\n")
    (root / "test.bin").write_bytes(b"\0" * 8)
    source = root / "src/test/func_80100000.c"
    source.parent.mkdir(parents=True)
    source.write_text(SOURCE_TEXT)
    header = root / "include/test.h"
    header.parent.mkdir(parents=True)
    header.write_text("#define OLD_VALUE 17\n")
    database = root / "index.sqlite"
    connection = sqlite3.connect(database)
    create_schema(connection)
    connection.execute(
        "INSERT INTO targets VALUES (?, 'test.bin', ?, 0x80100000, 'r', 'v', 's', 'sh')",
        (TARGET, hashlib.sha256(b"\0" * 8).hexdigest()),
    )
    connection.execute(
        "INSERT INTO macro_input_fingerprints VALUES (?, ?, ?, 'source_claim', ?)",
        (TARGET, source.relative_to(root).as_posix(), sha256_file(source), TARGET),
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
            source.relative_to(root).as_posix(),
            "exact",
            2,
            0,
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
    return database


def _connect(database: Path):
    connection = sqlite3.connect(database)
    connection.row_factory = sqlite3.Row
    return connection


def _artifact(
    root: Path,
    report: dict[str, Any],
    concern: str = "constant",
    owners: list[str] | None = None,
) -> str:
    owners = owners or ["include/test.h"]
    facts = {
        "schema": macro_transactions.REVIEW_SCHEMA,
        "candidate_id": report["rows"][0]["id"],
        "candidate_fingerprint": report["rows"][0]["candidate_fingerprint"],
        "concern": concern,
        "owners": owners,
        "owner_fingerprints": {owner: sha256_file(root / owner) for owner in owners},
        "semantic_guards": {
            name: {"status": "resolved", "evidence": f"reviewed {name}"}
            for name in GUARDS
        },
        "observations": {name: f"reviewed {name}" for name in OBSERVATIONS},
        "review": {"verdict": "accepted", "reviewer": "independent"},
    }
    path = root / "out/reviews/macro-opportunity.json"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps({**facts, "digest": digest(facts)}))
    return path.relative_to(root).as_posix()


def _request(root: Path, report: dict[str, Any], **extra) -> dict[str, Any]:
    return {
        "schema": macro_transactions.REQUEST_SCHEMA,
        "target": TARGET,
        "concern": "constant",
        "candidate_artifact": _artifact(root, report),
        "affected_functions": [SELECTOR],
        **extra,
    }


def _runner(fail: int | None = None):
    count = 0

    def run(argv, **_kwargs):
        nonlocal count
        count += 1
        return subprocess.CompletedProcess(argv, int(count == fail), "ran", "")

    return run


def _setup(tmp_path: Path, monkeypatch: pytest.MonkeyPatch, *, git: bool = False):
    database = _repo(tmp_path, git=git)

    def connection(_root):
        return _connect(database)

    monkeypatch.setattr(macro_accounting, "connect", connection)
    monkeypatch.setattr(macro_transactions, "connect", connection)
    return macro_accounting.candidate_account(tmp_path)


def _manifest(tmp_path: Path, monkeypatch: pytest.MonkeyPatch, *, git: bool = False):
    report = _setup(tmp_path, monkeypatch, git=git)
    request = _request(tmp_path, report)
    if git:
        request["adopted_baseline"] = macro_transactions.workspace_baseline(tmp_path)[
            "digest"
        ]
    return macro_transactions.prepare_transaction(tmp_path, request)


def test_forged_index_source_cannot_override_manifest_metadata(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    report = _setup(tmp_path, monkeypatch)
    forged = tmp_path / "AGENTS.md"
    forged.write_text("/** @source 0x80100000\n * @behavior forged\n */\n")
    connection = _connect(tmp_path / "index.sqlite")
    connection.execute("UPDATE functions SET source='AGENTS.md'")
    connection.commit()
    connection.close()
    manifest = macro_transactions.prepare_transaction(
        tmp_path, _request(tmp_path, report)
    )
    assert manifest["affected_functions"][0]["source"] == "src/test/func_80100000.c"


@pytest.mark.parametrize("command", ["prepare", "run"])
def test_macro_cli_outputs_are_confined_and_symlink_safe(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, command: str
) -> None:
    victim = tmp_path / "victim.json"
    victim.write_text("keep\n")
    monkeypatch.setattr(
        macro_audit, "prepare_transaction", lambda *_args: {"proof": True}
    )
    monkeypatch.setattr(macro_audit, "run_transaction", lambda *_args: {"proof": True})
    monkeypatch.setattr(macro_audit, "_read", lambda _path: {})
    handler = macro_audit._prepare if command == "prepare" else macro_audit._run
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
    (evidence / "proof.json").symlink_to(victim)
    with pytest.raises(ValueError, match="regular file|unsafe"):
        handler(args)
    assert victim.read_text() == "keep\n"


def test_all_four_concern_classes_are_declared() -> None:
    assert macro_transactions.CONCERNS == {
        "constant",
        "expression",
        "local_template",
        "shared_template",
    }


def _manifest_owner():
    class Manifest:
        has_explicit_sources = True
        sources = ("src/test/func_80100000.c",)
        support_sources = ()
        headers = ("include/test.h",)
        splat = "config/targets/exe/test/splat.yaml"

    return Manifest()


@pytest.mark.parametrize(
    ("concern", "kind"),
    [
        ("constant", "constant"),
        ("expression", "expression_accessor"),
        ("local_template", "statement_window"),
        ("shared_template", "parameterized_near_duplicate"),
    ],
)
def test_reviewed_artifact_accepts_each_matching_concern_class(
    tmp_path: Path, concern: str, kind: str
) -> None:
    _repo(tmp_path)
    candidate = {
        "id": "lead",
        "kind": kind,
        "candidate_fingerprint": digest({"lead": kind}),
    }
    report = {"rows": [candidate]}
    if concern == "shared_template":
        shared = tmp_path / "src/shared/test.inc"
        shared.parent.mkdir(parents=True)
        shared.write_text("shared\n")
        artifact = _artifact(tmp_path, report, concern, ["src/shared/test.inc"])
    else:
        artifact = _artifact(tmp_path, report, concern)
    reviewed = macro_transactions.reviewed_artifact(
        tmp_path, artifact, concern, report, {TARGET: _manifest_owner()}
    )
    assert reviewed["candidate_id"] == "lead"


def test_private_review_rejects_mixed_claimed_and_unowned_paths(
    tmp_path: Path,
) -> None:
    _repo(tmp_path)
    agents = tmp_path / "AGENTS.md"
    agents.write_text("not target-owned\n")
    candidate = {
        "id": "lead",
        "kind": "constant",
        "candidate_fingerprint": digest({"lead": "constant"}),
    }
    report = {"rows": [candidate]}
    artifact = _artifact(tmp_path, report, owners=["include/test.h", "AGENTS.md"])
    with pytest.raises(ValueError, match="not all owned by one target"):
        macro_transactions.reviewed_artifact(
            tmp_path, artifact, "constant", report, {TARGET: _manifest_owner()}
        )


def test_shared_review_rejects_private_or_policy_owner(tmp_path: Path) -> None:
    _repo(tmp_path)
    candidate = {
        "id": "lead",
        "kind": "parameterized_near_duplicate",
        "candidate_fingerprint": digest({"lead": "shared"}),
    }
    report = {"rows": [candidate]}
    agents = tmp_path / "AGENTS.md"
    agents.write_text("not shared\n")
    artifact = _artifact(
        tmp_path, report, "shared_template", ["include/test.h", "AGENTS.md"]
    )
    with pytest.raises(ValueError, match="not a sanctioned shared path"):
        macro_transactions.reviewed_artifact(
            tmp_path,
            artifact,
            "shared_template",
            report,
            {TARGET: _manifest_owner()},
        )


def test_shared_review_rejects_target_private_internal_header_even_with_two_unrelated_exact_proofs(
    tmp_path: Path,
) -> None:
    _repo(tmp_path)
    private_header = "include/bof3/scenario/scena16_internal.h"
    header = tmp_path / private_header
    header.parent.mkdir(parents=True)
    header.write_text("#define PRIVATE_VALUE 17\n")

    class Manifest:
        def __init__(self, source: str, headers: tuple[str, ...] = ()) -> None:
            self.sources = (source,)
            self.support_sources = ()
            self.headers = headers

    manifests = {}
    proofs = []
    for index, target in enumerate(("exe/a", "exe/b")):
        source = f"src/test/wrapper_{index}.c"
        (tmp_path / source).write_text(f"void wrapper_{index}(void) {{}}\n")
        manifests[target] = Manifest(source, (private_header,) if index == 0 else ())
        selector = f"{target}@0x8010000{index}"
        proofs.append(
            {
                "path": f"out/reviews/proof-{index}.json",
                "sha256": f"{index}" * 64,
                "target": target,
                "selector": selector,
                "expected_application_digest": digest({"proof": index}),
                "application": {
                    "manifest": {
                        "affected_functions": [
                            {
                                "selector": selector,
                                "target": target,
                                "status": "exact",
                                "source": source,
                            }
                        ]
                    }
                },
            }
        )

    candidate = {
        "id": "lead",
        "kind": "parameterized_near_duplicate",
        "candidate_fingerprint": digest({"lead": "shared"}),
    }
    report = {"rows": [candidate]}
    artifact = _artifact(tmp_path, report, "shared_template", [private_header])
    with pytest.raises(ValueError, match="not a sanctioned shared path"):
        macro_transactions.reviewed_artifact(
            tmp_path,
            artifact,
            "shared_template",
            report,
            manifests,
            proofs=proofs,
        )


def test_shared_review_rejects_public_owner_unrelated_to_both_exact_wrappers(
    tmp_path: Path,
) -> None:
    _repo(tmp_path)
    shared = tmp_path / "include/shared/unrelated.h"
    shared.parent.mkdir(parents=True)
    shared.write_text("#define UNRELATED 17\n")

    class Manifest:
        def __init__(self, source: str) -> None:
            self.sources = (source,)
            self.support_sources = ()
            self.headers = ()

    manifests = {}
    proofs = []
    for index, target in enumerate(("exe/a", "exe/b")):
        source = f"src/test/wrapper_{index}.c"
        (tmp_path / source).write_text(f"void wrapper_{index}(void) {{}}\n")
        manifests[target] = Manifest(source)
        selector = f"{target}@0x8010000{index}"
        proofs.append(
            {
                "target": target,
                "selector": selector,
                "application": {
                    "manifest": {
                        "affected_functions": [
                            {
                                "selector": selector,
                                "target": target,
                                "status": "exact",
                                "source": source,
                            }
                        ]
                    }
                },
            }
        )
    candidate = {
        "id": "lead",
        "kind": "parameterized_near_duplicate",
        "candidate_fingerprint": digest({"lead": "shared"}),
    }
    report = {"rows": [candidate]}
    artifact = _artifact(
        tmp_path, report, "shared_template", ["include/shared/unrelated.h"]
    )
    with pytest.raises(ValueError, match="proven dependency of both exact wrappers"):
        macro_transactions.reviewed_artifact(
            tmp_path,
            artifact,
            "shared_template",
            report,
            manifests,
            proofs=proofs,
        )


def test_shared_prepare_passes_pinned_proofs_to_review(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    report = _setup(tmp_path, monkeypatch)
    captured = {}
    proofs = [{"target": TARGET}, {"target": "exe/other"}]
    monkeypatch.setattr(
        macro_transactions,
        "load_target_manifests",
        lambda _root: {TARGET: object(), "exe/other": object()},
    )
    monkeypatch.setattr(
        macro_transactions, "exact_proofs", lambda *args, **kwargs: proofs
    )

    def review(*args, **kwargs):
        captured["proofs"] = kwargs["proofs"]
        return {"owners": ["include/test.h"], "declared_targets": [TARGET, "exe/other"]}

    monkeypatch.setattr(macro_transactions, "reviewed_artifact", review)
    monkeypatch.setattr(macro_transactions, "_functions", lambda *args: [])
    request = {
        "schema": macro_transactions.REQUEST_SCHEMA,
        "target": TARGET,
        "concern": "shared_template",
        "candidate_artifact": _artifact(tmp_path, report, "shared_template"),
        "shared_targets": [TARGET, "exe/other"],
        "exact_function_proofs": [{}, {}],
        "affected_functions": [SELECTOR],
    }
    macro_transactions.prepare_transaction(tmp_path, request)
    assert captured["proofs"] is proofs


def test_shared_prepare_rejects_unrelated_owner_end_to_end(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    report = _setup(tmp_path, monkeypatch)
    shared = tmp_path / "include/shared/unrelated.h"
    shared.parent.mkdir(parents=True)
    shared.write_text("#define UNRELATED 17\n")

    class Manifest:
        def __init__(self, source: str) -> None:
            self.sources = (source,)
            self.support_sources = ()
            self.headers = ()

    manifests = {}
    proofs = []
    for index, target in enumerate((TARGET, "exe/other")):
        source = f"src/test/wrapper_{index}.c"
        (tmp_path / source).write_text(f"void wrapper_{index}(void) {{}}\n")
        manifests[target] = Manifest(source)
        selector = f"{target}@0x8010000{index}"
        proofs.append(
            {
                "target": target,
                "selector": selector,
                "application": {
                    "manifest": {
                        "affected_functions": [
                            {
                                "selector": selector,
                                "target": target,
                                "status": "exact",
                                "source": source,
                            }
                        ]
                    }
                },
            }
        )
    report["rows"][0]["kind"] = "parameterized_near_duplicate"
    report["rows"][0]["candidate_fingerprint"] = digest(
        {"candidate": "shared-unrelated"}
    )
    artifact = _artifact(
        tmp_path,
        report,
        "shared_template",
        ["include/shared/unrelated.h"],
    )
    monkeypatch.setattr(macro_accounting, "candidate_account", lambda _: report)
    monkeypatch.setattr(
        macro_transactions, "load_target_manifests", lambda _: manifests
    )
    monkeypatch.setattr(
        macro_transactions,
        "_target",
        lambda value, known: (
            value if value in known else (_ for _ in ()).throw(ValueError())
        ),
    )
    monkeypatch.setattr(
        macro_transactions, "exact_proofs", lambda *args, **kwargs: proofs
    )
    request = {
        "schema": macro_transactions.REQUEST_SCHEMA,
        "target": TARGET,
        "concern": "shared_template",
        "candidate_artifact": artifact,
        "shared_targets": [TARGET, "exe/other"],
        "exact_function_proofs": [{}, {}],
        "affected_functions": [SELECTOR],
    }
    with pytest.raises(ValueError, match="proven dependency of both exact wrappers"):
        macro_transactions.prepare_transaction(tmp_path, request)


def test_shared_review_artifact_rejects_address_leak(tmp_path: Path) -> None:
    _repo(tmp_path)
    candidate = {
        "id": "lead",
        "kind": "parameterized_near_duplicate",
        "candidate_fingerprint": digest({"lead": "shared"}),
    }
    report = {"rows": [candidate]}
    shared = tmp_path / "src/shared/test.inc"
    shared.parent.mkdir(parents=True)
    shared.write_text("shared\n")
    artifact_name = _artifact(
        tmp_path, report, "shared_template", ["src/shared/test.inc"]
    )
    path = tmp_path / artifact_name
    artifact = json.loads(path.read_text())
    artifact["observations"]["all_use_sites"] = "target 0x80101234"
    facts = {key: item for key, item in artifact.items() if key != "digest"}
    path.write_text(json.dumps({**facts, "digest": digest(facts)}))
    with pytest.raises(ValueError, match="address leak"):
        macro_transactions.reviewed_artifact(
            tmp_path,
            artifact_name,
            "shared_template",
            report,
            {TARGET: _manifest_owner()},
        )


def test_macro_prepare_rejects_leaf_and_parent_symlinks(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    report = _setup(tmp_path, monkeypatch)
    request = _request(tmp_path, report)
    header = tmp_path / "include/test.h"
    outside = tmp_path / "outside.h"
    outside.write_text("outside\n")
    header.unlink()
    header.symlink_to(outside)
    with pytest.raises(ValueError, match="path is invalid|fingerprint"):
        macro_transactions.prepare_transaction(tmp_path, request)

    header.unlink()
    (tmp_path / "include").rmdir()
    outside_dir = tmp_path / "outside"
    outside_dir.mkdir()
    (outside_dir / "test.h").write_text("outside\n")
    (tmp_path / "include").symlink_to(outside_dir, target_is_directory=True)
    with pytest.raises(ValueError, match="path is invalid|fingerprint"):
        macro_transactions.prepare_transaction(tmp_path, request)


def test_macro_forged_escape_swap_and_rollback_symlink_fail_closed(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    manifest = _manifest(tmp_path, monkeypatch)
    forged = copy.deepcopy(manifest)
    forged["allowed_paths"].append("../escape.h")
    facts = {key: item for key, item in forged.items() if key != "digest"}
    forged["digest"] = digest(facts)
    with pytest.raises(ValueError, match="allowed_paths|repo-relative|not canonical"):
        macro_transactions.run_transaction(
            tmp_path, forged, {"include/test.h": "changed\n"}
        )

    outside = tmp_path / "outside.h"
    outside.write_text("outside\n")
    original = runtime._atomic_write
    swapped = False

    def swap(root, name, content, **kwargs):
        nonlocal swapped
        if name == "include/test.h" and not swapped:
            swapped = True
            (root / name).symlink_to(outside)
        return original(root, name, content, **kwargs)

    monkeypatch.setattr(runtime, "_atomic_write", swap)
    with pytest.raises((ValueError, RuntimeError), match="unsafe|rollback"):
        macro_transactions.run_transaction(
            tmp_path,
            manifest,
            {"include/test.h": "changed\n"},
            runner=_runner(),
        )
    assert outside.read_text() == "outside\n"

    monkeypatch.setattr(runtime, "_atomic_write", original)
    header = tmp_path / "include/test.h"
    if header.is_symlink():
        header.unlink()
        header.write_text("#define OLD_VALUE 17\n")

    def attack(argv, **kwargs):
        header.unlink()
        header.symlink_to(outside)
        return subprocess.CompletedProcess(argv, 1, "", "failed")

    with pytest.raises(RuntimeError, match="rollback failed"):
        macro_transactions.run_transaction(
            tmp_path,
            manifest,
            {"include/test.h": "changed\n"},
            runner=attack,
        )
    assert outside.read_text() == "outside\n"


def test_current_zero_account_does_not_authorize_without_review(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    report = _setup(tmp_path, monkeypatch)
    assert report["safe_application_count"] == 0
    request = _request(tmp_path, report)
    request["candidate_artifact"] = "missing.json"
    with pytest.raises(ValueError, match="path is invalid"):
        macro_transactions.prepare_transaction(tmp_path, request)


def test_review_requires_all_eight_guards_two_observations_and_fresh_owner(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    report = _setup(tmp_path, monkeypatch)
    request = _request(tmp_path, report)
    path = tmp_path / request["candidate_artifact"]
    artifact = json.loads(path.read_text())
    artifact["semantic_guards"].pop("aliasing")
    facts = {key: item for key, item in artifact.items() if key != "digest"}
    path.write_text(json.dumps({**facts, "digest": digest(facts)}))
    with pytest.raises(ValueError, match="unresolved semantic guards"):
        macro_transactions.prepare_transaction(tmp_path, request)

    request["candidate_artifact"] = _artifact(tmp_path, report)
    (tmp_path / "include/test.h").write_text("drift\n")
    with pytest.raises(ValueError, match="owner fingerprint drifted"):
        macro_transactions.prepare_transaction(tmp_path, request)


def test_atomic_apply_receipts_attestation_expected_digest_and_rollback(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    manifest = _manifest(tmp_path, monkeypatch)
    application = macro_transactions.run_transaction(
        tmp_path, manifest, {"include/test.h": "#define VALUE 17\n"}, runner=_runner()
    )
    expected = application["digest"]
    assert macro_transactions.verify_application(tmp_path, application, expected)[
        "applied"
    ]
    receipt = json.loads((tmp_path / application["receipts"][0]["path"]).read_text())
    assert receipt["schema"] == macro_transactions.RECEIPT_SCHEMA
    with pytest.raises(ValueError, match="attestation is not trusted"):
        macro_transactions.verify_application(
            tmp_path, application, digest({"wrong": 1})
        )

    (tmp_path / "include/test.h").write_text("#define OLD_VALUE 17\n")
    manifest["pre_state"] = macro_transactions.file_state(
        tmp_path, set(manifest["allowed_paths"])
    )
    facts = {key: item for key, item in manifest.items() if key != "digest"}
    manifest = {**facts, "digest": digest(facts)}
    before = (tmp_path / "include/test.h").read_bytes()
    with pytest.raises(RuntimeError, match="validation failed"):
        macro_transactions.run_transaction(
            tmp_path, manifest, {"include/test.h": "bad\n"}, runner=_runner(1)
        )
    assert (tmp_path / "include/test.h").read_bytes() == before


def test_macro_validation_substitution_retains_leaf_and_original_quarantine(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    manifest = _manifest(tmp_path, monkeypatch)
    leaf = tmp_path / "include/test.h"
    before = leaf.read_bytes()

    def substitute(argv, **kwargs):
        leaf.write_bytes(b"unexpected\n")
        return subprocess.CompletedProcess(argv, 0, "", "")

    with pytest.raises(RuntimeError, match="rollback failed"):
        macro_transactions.run_transaction(
            tmp_path,
            manifest,
            {"include/test.h": "#define VALUE 17\n"},
            runner=substitute,
        )

    assert leaf.read_bytes() == b"unexpected\n"
    quarantines = list((tmp_path / transaction_files.QUARANTINE_DIRECTORY).iterdir())
    assert any(path.read_bytes() == before for path in quarantines)


def test_macro_run_rejects_redigested_manifest_empty_checks_and_arbitrary_argv(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    manifest = _manifest(tmp_path, monkeypatch)
    for mutate in (
        lambda value: value.update(concern="expression"),
        lambda value: value.update(required_checks=[]),
        lambda value: value["required_checks"][0].update(argv=["rm", "-rf", "."]),
    ):
        forged = copy.deepcopy(manifest)
        mutate(forged)
        facts = {key: item for key, item in forged.items() if key != "digest"}
        forged["digest"] = digest(facts)
        with pytest.raises(ValueError, match="not canonical"):
            macro_transactions.run_transaction(
                tmp_path, forged, {"include/test.h": "changed\n"}
            )


def test_manifest_and_receipt_tampering_fail_closed(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    manifest = _manifest(tmp_path, monkeypatch)
    changed = copy.deepcopy(manifest)
    changed["concern"] = "expression"
    with pytest.raises(ValueError, match="manifest drifted"):
        macro_transactions.run_transaction(tmp_path, changed, {"include/test.h": "x"})
    application = macro_transactions.run_transaction(
        tmp_path, manifest, {"include/test.h": "#define VALUE 17\n"}, runner=_runner()
    )
    receipt = tmp_path / application["receipts"][0]["path"]
    receipt.write_text("{}")
    application["receipts"][0]["sha256"] = sha256_file(receipt)
    with pytest.raises(ValueError, match="proof drifted|receipt replaced"):
        macro_transactions.verify_application(
            tmp_path, application, application["digest"]
        )


def test_validation_cannot_unstage_git_index(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    report = _setup(tmp_path, monkeypatch, git=True)
    marker = tmp_path / "marker.txt"
    marker.write_text("staged\n")
    subprocess.run(["git", "add", marker.name], cwd=tmp_path, check=True)
    request = _request(tmp_path, report)
    request["adopted_baseline"] = macro_transactions.workspace_baseline(tmp_path)[
        "digest"
    ]
    manifest = macro_transactions.prepare_transaction(tmp_path, request)
    before = subprocess.run(
        ["git", "ls-files", "--stage", "-z"],
        cwd=tmp_path,
        capture_output=True,
        check=True,
    ).stdout

    def unstage(argv, **_kwargs):
        subprocess.run(
            ["git", "reset", "-q", "HEAD", "--", marker.name], cwd=tmp_path, check=True
        )
        return subprocess.CompletedProcess(argv, 0, "", "")

    with pytest.raises(ValueError, match="changed the Git index"):
        macro_transactions.run_transaction(
            tmp_path, manifest, {"include/test.h": "changed\n"}, runner=unstage
        )
    assert (
        subprocess.run(
            ["git", "ls-files", "--stage", "-z"],
            cwd=tmp_path,
            capture_output=True,
            check=True,
        ).stdout
        == before
    )


def test_dirty_baseline_and_runner_side_effect_fail_closed(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    report = _setup(tmp_path, monkeypatch, git=True)
    (tmp_path / "unrelated.txt").write_text("dirty")
    request = _request(tmp_path, report)
    with pytest.raises(ValueError, match="adopted_baseline"):
        macro_transactions.prepare_transaction(tmp_path, request)
    request["adopted_baseline"] = macro_transactions.workspace_baseline(tmp_path)[
        "digest"
    ]
    manifest = macro_transactions.prepare_transaction(tmp_path, request)
    source = tmp_path / "src/test/func_80100000.c"
    before = source.read_text()

    def mutate(argv, **_kwargs):
        source.write_text("mutated\n")
        return subprocess.CompletedProcess(argv, 0, "", "")

    with pytest.raises(ValueError, match="validation mutated"):
        macro_transactions.run_transaction(
            tmp_path, manifest, {"include/test.h": "changed\n"}, runner=mutate
        )
    assert source.read_text() == before


def test_shared_proofs_require_external_unique_exact_pins_and_no_address_leaks(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    manifests = {"exe/a": object(), "exe/b": object()}
    proofs = []
    for index, target in enumerate(manifests):
        application = {
            "concern": "local_template",
            "exact_function_proofs": [f"{target}@0x80100000"],
            "semantic_guards": {
                name: {"status": "resolved", "evidence": "same"} for name in GUARDS
            },
            "observations": {name: "same" for name in OBSERVATIONS},
        }
        path = tmp_path / f"out/reviews/proof-{index}.json"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(application))
        expected = digest({"proof": index})
        proofs.append(
            {
                "path": path.relative_to(tmp_path).as_posix(),
                "target": target,
                "selector": application["exact_function_proofs"][0],
                "expected_application_digest": expected,
            }
        )
    monkeypatch.setattr(
        "harness.analysis.macro_transaction_review.repo_path",
        lambda _root, value: value,
    )

    def verify(_root, value, expected):
        assert expected in {item["expected_application_digest"] for item in proofs}
        return {
            "target": value["exact_function_proofs"][0].split("@", 1)[0],
            "concern": value["concern"],
        }

    def normalized(value, _manifests):
        return value

    result = macro_transactions.exact_proofs(
        tmp_path,
        proofs,
        manifests,
        normalize_target=normalized,
        verify_application=verify,
    )
    assert len(result) == 2
    proofs[1]["expected_application_digest"] = proofs[0]["expected_application_digest"]
    with pytest.raises(ValueError, match="independently pinned"):
        macro_transactions.exact_proofs(
            tmp_path,
            proofs,
            manifests,
            normalize_target=normalized,
            verify_application=verify,
        )
    proofs[1]["expected_application_digest"] = digest({"proof": 1})
    second = json.loads((tmp_path / proofs[1]["path"]).read_text())
    second["observations"]["all_use_sites"] = "leak 0x80101234"
    (tmp_path / proofs[1]["path"]).write_text(json.dumps(second))
    with pytest.raises(ValueError, match="differ or contain address leaks"):
        macro_transactions.exact_proofs(
            tmp_path,
            proofs,
            manifests,
            normalize_target=normalized,
            verify_application=verify,
        )
