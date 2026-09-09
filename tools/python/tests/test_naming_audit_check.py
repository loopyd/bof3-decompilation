from __future__ import annotations

import hashlib
import json
import threading
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Any

import pytest
from harness.naming.audit import (
    prepare_transaction,
    validate,
)
from harness.naming.audit import (
    verify as _verify,
)
from harness.naming.context import DIGEST_VERSION, pre_apply


def verify(
    root: Path, target: str, report: dict[str, Any], transaction: str
) -> dict[str, Any]:
    """Invoke production verification with the fixture's canonical report path."""
    return _verify(
        root,
        target,
        report,
        transaction,
        report_path=root / "docs/reviews/test.json",
    )


TARGET = "exe/test"


class _EmptyConnection:
    """Bulk-work snapshot connection stand-in: three empty bounded queries."""

    def execute(self, *_args, **_kwargs):
        return []

    def close(self):
        pass


class _DataReferenceConnection:
    """Bulk snapshot that reports one data access for ``D_80100010``.

    Only the second bounded query (``data_references``) yields a row so the
    snapshot generates an ``access:`` work item for the data row; the other
    two queries stay empty. This keeps the bulk snapshot count at three
    queries while letting the data row carry open required work.
    """

    def execute(self, query: str, *_args: object, **_kwargs: object) -> list[object]:
        if "data_references" in query:
            return [(0x80100010, "x")]
        return []

    def close(self) -> None:
        pass


def _repo(root: Path) -> None:
    config = root / "config/targets/exe/test/target.toml"
    config.parent.mkdir(parents=True)
    config.write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src/test'\n"
        "binary='out/test.bin'\nload_address=0x80100000\n"
        "splat='config/targets/exe/test/splat.yaml'\n"
        "sources=['src/test/func_80100000.c']\n",
        encoding="utf-8",
    )
    (config.parent / "symbols.txt").write_text(
        "func_80100000 = 0x80100000;\nD_80100010 = 0x80100010;\n",
        encoding="utf-8",
    )
    (config.parent / "splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [8, data, D_80100008]\n  - [0x20]\n",
        encoding="utf-8",
    )
    source = root / "src/test/func_80100000.c"
    source.parent.mkdir(parents=True)
    source.write_text(
        "/* @source 0x80100000\n * @behavior UNKNOWN: test\n"
        " * @status exact\n * @match 100.00\n * @residual none\n */\n"
        "void func_80100000(void) {}\n",
        encoding="utf-8",
    )
    binary = root / "out/test.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"\0" * 0x20)


def _command(root: Path, name: str, command: str | None = None) -> dict[str, Any]:
    receipt = root / f"out/reviews/evidence/{name}.json"
    receipt.parent.mkdir(parents=True, exist_ok=True)
    text = command or f"inspect {name}"
    selector = (
        "exe/test@0x80100000"
        if text.startswith(
            ("bin/asm-diff", "bin/byte-match", "partial baseline", "independent review")
        )
        else None
    )
    record: dict[str, Any] = {
        "command": text,
        "status": "passed",
        "target": "exe/test",
        "selector": selector,
        "output": "observed bytes",
        "receipt": receipt.relative_to(root).as_posix(),
    }
    payload = {
        key: record[key]
        for key in ("command", "status", "target", "selector", "output")
    }
    receipt.write_text(json.dumps(payload, sort_keys=True), encoding="utf-8")
    record["sha256"] = hashlib.sha256(receipt.read_bytes()).hexdigest()
    return record


def _post_apply_receipts(root: Path) -> list[dict[str, str]]:
    selector = "exe/test@0x80100000"
    commands = [
        "bin/symbols normalize exe/test --write",
        "bin/symbols check exe/test",
        "bin/splat exe/test",
        "bin/build exe/test",
        f"bin/asm-diff {selector} --detail normal",
        f"bin/byte-match {selector}",
        "independent review exe/test@0x80100000",
    ]
    return [
        _command(root, f"post-{index}", command)
        for index, command in enumerate(commands)
    ]


def _observation(name: str) -> dict[str, str]:
    return {"id": name, "text": f"exact observation {name}"}


def _rung(root: Path, row: str, rung: str) -> dict[str, object]:
    return {
        "status": "passed",
        "commands": [_command(root, row + rung)],
        "observations": [_observation(row + "." + rung)],
        "authority": "original bytes",
    }


def _row(root: Path, kind: str, name: str, state: str) -> dict[str, Any]:
    rungs = (
        ("selected_range", "selected_call", "one_level_beyond")
        if kind == "function"
        else ("selected_range", "selected_access", "storage_class", "one_level_beyond")
    )
    row = {
        "kind": kind,
        "name": name,
        "rung_status": state,
        "outside_payload": False,
        "partial_used": False,
        "rungs": {rung: _rung(root, name, rung) for rung in rungs},
        "required_work": [],
        "optional_work": [{"id": "runtime-trace"}],
        "interpretation": "mechanical role",
        "authority": "manifest, map, reviewed Splat, original bytes",
        "missing_fact": "semantic role",
        "ceiling_next_command": "runtime-trace: optional experiment",
    }
    return row


def _report(root: Path) -> dict[str, Any]:
    function = _row(root, "function", "func_80100000", "proposed")
    function.update(
        {
            "new_name": "returnImmediately",
            "semantic_status": "accepted",
            "transaction_status": "ready",
            "readiness_blockers": [],
            "corroborators": {
                "A": {
                    "observation_ids": ["func_80100000.selected_call"],
                    "mechanism": "selected_original_instructions",
                },
                "B": {
                    "observation_ids": ["func_80100000.one_level_beyond"],
                    "mechanism": "independent_caller",
                },
            },
            "name_terms": {"return": ["A"], "immediately": ["B"]},
            "identity": {
                "selector": "exe/test@0x80100000",
                "old": "func_80100000",
                "new": "returnImmediately",
                "unchanged_range": "0x80100000..0x80100008",
                "binding_locations": ["config/targets/exe/test/symbols.txt"],
                "source_locations": [
                    "config/targets/exe/test/splat.yaml",
                    "config/targets/exe/test/target.toml",
                    "src/test/func_80100000.c",
                ],
            },
        }
    )
    return {
        "schema": "bof3.naming-audit/v3",
        "target": TARGET,
        "complete": True,
        "rows": [function, _row(root, "data", "D_80100010", "blocked")],
    }


def _prepared_report(
    root: Path, report: dict[str, Any] | None = None, monkeypatch=None
) -> tuple[dict[str, Any], Path]:
    """Prepare a proposal through the production atomic producer."""
    payload = report or _report(root)
    if monkeypatch is not None:
        monkeypatch.setattr(
            "harness.naming.audit.connect_index",
            lambda *_, **__: _EmptyConnection(),
        )
    payload["complete"] = False
    data = payload["rows"][1]
    data["smallest_repair"] = "bin/rev-query --json xrefs exe/test@0x80100010"
    data["rungs"]["selected_access"] = {
        "status": "open",
        "next_command": "bin/rev-query --json xrefs exe/test@0x80100010",
        "observations": [_observation("D_80100010.selected_access.open")],
        "authority": "original bytes",
    }
    path = root / "docs/reviews/test.json"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    prepare_transaction(root, TARGET, path, "function:func_80100000")
    return json.loads(path.read_text(encoding="utf-8")), path


def _prepared_post_apply(root: Path, monkeypatch) -> dict[str, Any]:
    """Prepare through production, then add only post-apply receipts."""
    report, _ = _prepared_report(root, monkeypatch=monkeypatch)
    report["rows"][0]["post_apply_receipts"] = _post_apply_receipts(root)
    return report


def test_prepare_transaction_rejects_concurrent_write_atomically(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(
        "harness.naming.audit.connect_index",
        lambda *_, **__: _EmptyConnection(),
    )
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _report(tmp_path)
    report["complete"] = False
    data = report["rows"][1]
    data["smallest_repair"] = "bin/rev-query --json xrefs exe/test@0x80100010"
    data["rungs"]["selected_access"] = {
        "status": "open",
        "next_command": data["smallest_repair"],
        "observations": [_observation("D_80100010.selected_access.open")],
        "authority": "original bytes",
    }
    path = tmp_path / "docs/reviews/test.json"
    path.parent.mkdir(parents=True)
    path.write_text(json.dumps(report, sort_keys=True) + "\n", encoding="utf-8")
    from harness.naming import audit

    real_write = audit.atomic_write_if_unchanged
    concurrent = b'{"concurrent":true}\n'

    def race(target, expected, payload):
        target.write_bytes(concurrent)
        real_write(target, expected, payload)

    monkeypatch.setattr(audit, "atomic_write_if_unchanged", race)
    with pytest.raises(ValueError, match="changed concurrently"):
        prepare_transaction(tmp_path, TARGET, path, "function:func_80100000")
    assert path.read_bytes() == concurrent
    assert not list(path.parent.glob(f".{path.name}.*"))


def test_prepare_transaction_serializes_competing_writers(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(
        "harness.naming.audit.connect_index",
        lambda *_, **__: _EmptyConnection(),
    )
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _report(tmp_path)
    report["complete"] = False
    data = report["rows"][1]
    data["smallest_repair"] = "bin/rev-query --json xrefs exe/test@0x80100010"
    data["rungs"]["selected_access"] = {
        "status": "open",
        "next_command": data["smallest_repair"],
        "observations": [_observation("D_80100010.selected_access.open")],
        "authority": "original bytes",
    }
    path = tmp_path / "docs/reviews/test.json"
    path.parent.mkdir(parents=True)
    path.write_text(json.dumps(report, sort_keys=True) + "\n", encoding="utf-8")

    # The lock owns read/validate/CAS/replace. A competing writer cannot enter
    # until the first leaves, so its stale expected bytes lose the CAS.
    from harness.naming.editing import report_mutation
    from harness.naming.proposal import atomic_write_if_unchanged

    expected = path.read_bytes()
    barrier = threading.Barrier(2)

    def compete(writer: int) -> str:
        barrier.wait(timeout=2)
        try:
            with report_mutation(path, report=True):
                atomic_write_if_unchanged(path, expected, {"writer": writer})
            return "accepted"
        except ValueError:
            return "rejected"

    with ThreadPoolExecutor(max_workers=2) as executor:
        results = list(executor.map(compete, (1, 2)))
    assert sorted(results) == ["accepted", "rejected"]


def test_validate_v3_accepts_complete_explicit_blocked_gap_inventory(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(
        "harness.naming.audit.connect_index",
        lambda *_, **__: _EmptyConnection(),
    )
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _report(tmp_path)
    report["complete"] = False
    for row in report["rows"]:
        row["rung_status"] = "blocked"
        row.pop("conclusion_provenance", None)
        row["smallest_repair"] = "bin/rev-query --json xrefs exe/test@0x80100000"
        row["required_work"] = []
        for rung in row["rungs"].values():
            rung["status"] = "open"
            rung["next_command"] = "bin/rev-query --json xrefs exe/test@0x80100000"
            rung.pop("commands", None)
    assert validate(tmp_path, TARGET, report)["complete"] is False


@pytest.mark.parametrize("mutation", ["missing", "malformed", "extra"])
def test_validate_v3_rejects_noncanonical_terminal_provenance(
    tmp_path: Path, monkeypatch, mutation: str
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(
        "harness.naming.audit.connect_index",
        lambda *_, **__: _EmptyConnection(),
    )
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report, report_path = _prepared_report(tmp_path, monkeypatch=monkeypatch)
    provenance = report["rows"][0]["conclusion_provenance"]
    if mutation == "missing":
        report["rows"][0].pop("conclusion_provenance")
    elif mutation == "malformed":
        provenance["kind"] = "unknown"
    else:
        provenance["extra"] = True
    with pytest.raises(ValueError, match="provenance|non-canonical"):
        validate(tmp_path, TARGET, report, report_path=report_path)


def test_validate_v3_full_and_isolated_transaction(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(
        "harness.naming.audit.connect_index",
        lambda *_, **__: _EmptyConnection(),
    )
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report, report_path = _prepared_report(tmp_path, monkeypatch=monkeypatch)
    assert (
        validate(tmp_path, TARGET, report, report_path=report_path)["complete"] is False
    )
    assert (
        validate(
            tmp_path,
            TARGET,
            report,
            transaction="function:func_80100000",
            report_path=report_path,
        )["ready"]
        is True
    )


def test_post_apply_validates_new_scope_and_old_absence(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _prepared_post_apply(tmp_path, monkeypatch)
    for path in (
        tmp_path / "config/targets/exe/test/symbols.txt",
        tmp_path / "config/targets/exe/test/splat.yaml",
        tmp_path / "config/targets/exe/test/target.toml",
        tmp_path / "src/test/func_80100000.c",
    ):
        path.write_text(
            path.read_text().replace("func_80100000", "returnImmediately"),
            encoding="utf-8",
        )
    old_source = tmp_path / "src/test/func_80100000.c"
    if old_source.exists():
        old_source.rename(tmp_path / "src/test/returnImmediately.c")
    result = verify(tmp_path, TARGET, report, "function:func_80100000")
    assert result["applied"] is True


@pytest.mark.parametrize(
    "case",
    ["interpretation", "authored-row", "report", "row-digest", "report-digest"],
)
def test_post_apply_rejects_provenance_digest_drift(
    tmp_path: Path, monkeypatch, case: str
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _prepared_post_apply(tmp_path, monkeypatch)
    for path in (
        tmp_path / "config/targets/exe/test/symbols.txt",
        tmp_path / "config/targets/exe/test/splat.yaml",
        tmp_path / "config/targets/exe/test/target.toml",
        tmp_path / "src/test/func_80100000.c",
    ):
        path.write_text(
            path.read_text().replace("func_80100000", "returnImmediately"),
            encoding="utf-8",
        )
    (tmp_path / "src/test/func_80100000.c").rename(
        tmp_path / "src/test/returnImmediately.c"
    )
    row = report["rows"][0]
    if case == "interpretation":
        row["interpretation"] = "tampered after apply"
    elif case == "authored-row":
        row["confidence"] = "low"
    elif case == "report":
        report["campaign"] = "tampered"
    elif case == "row-digest":
        row["conclusion_provenance"]["row_sha256"] = "0" * 64
    else:
        row["conclusion_provenance"]["report_sha256"] = "0" * 64
    with pytest.raises(ValueError, match="proposal (row|report) digest drifted"):
        verify(tmp_path, TARGET, report, "function:func_80100000")


@pytest.mark.parametrize("case", ["missing", "failed", "stale", "wrong-target"])
def test_post_apply_rejects_invalid_required_receipts(
    tmp_path: Path, monkeypatch, case: str
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _prepared_post_apply(tmp_path, monkeypatch)
    receipts = report["rows"][0]["post_apply_receipts"]
    if case == "missing":
        receipts = receipts[:-1]
    elif case == "failed":
        receipts[0]["status"] = "failed"
    elif case == "stale":
        (tmp_path / receipts[0]["receipt"]).write_text("changed\n", encoding="utf-8")
    else:
        receipts[0]["command"] = "bin/symbols normalize exe/other --write"
    report["rows"][0]["post_apply_receipts"] = receipts
    for path in (
        tmp_path / "config/targets/exe/test/symbols.txt",
        tmp_path / "config/targets/exe/test/splat.yaml",
        tmp_path / "config/targets/exe/test/target.toml",
        tmp_path / "src/test/func_80100000.c",
    ):
        path.write_text(
            path.read_text().replace("func_80100000", "returnImmediately"),
            encoding="utf-8",
        )
    (tmp_path / "src/test/func_80100000.c").rename(
        tmp_path / "src/test/returnImmediately.c"
    )
    with pytest.raises(ValueError):
        verify(tmp_path, TARGET, report, "function:func_80100000")


def test_post_apply_rejects_wrong_review_selector(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _prepared_post_apply(tmp_path, monkeypatch)
    receipts = report["rows"][0]["post_apply_receipts"]
    review = receipts[-1]
    review["selector"] = "exe/WRONG@0xDEADBEEF"
    path = tmp_path / review["receipt"]
    payload = {
        key: review[key]
        for key in ("command", "status", "target", "selector", "output")
    }
    path.write_text(json.dumps(payload, sort_keys=True), encoding="utf-8")
    review["sha256"] = hashlib.sha256(path.read_bytes()).hexdigest()
    report["rows"][0]["post_apply_receipts"] = receipts
    for changed in (
        tmp_path / "config/targets/exe/test/symbols.txt",
        tmp_path / "config/targets/exe/test/splat.yaml",
        tmp_path / "config/targets/exe/test/target.toml",
        tmp_path / "src/test/func_80100000.c",
    ):
        changed.write_text(
            changed.read_text().replace("func_80100000", "returnImmediately"),
            encoding="utf-8",
        )
    (tmp_path / "src/test/func_80100000.c").rename(
        tmp_path / "src/test/returnImmediately.c"
    )
    with pytest.raises(ValueError, match="selector mismatch"):
        verify(tmp_path, TARGET, report, "function:func_80100000")


@pytest.mark.parametrize(
    "field",
    [
        "schema",
        "transaction",
        "inventory",
        "scope",
        "reviewed",
        "reviewed_digest",
        "storage",
        "range",
        "baseline",
        "work",
        "collision",
        "required_checks",
    ],
)
def test_post_apply_rejects_each_tampered_manifest_field(
    tmp_path: Path, monkeypatch, field: str
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _prepared_post_apply(tmp_path, monkeypatch)
    manifest = report["rows"][0]["manifest"]
    manifest[field] = (
        "tampered"
        if not isinstance(manifest[field], list)
        else (["tampered"] if not manifest[field] else [])
    )
    for changed in (
        tmp_path / "config/targets/exe/test/symbols.txt",
        tmp_path / "config/targets/exe/test/splat.yaml",
        tmp_path / "config/targets/exe/test/target.toml",
        tmp_path / "src/test/func_80100000.c",
    ):
        changed.write_text(
            changed.read_text().replace("func_80100000", "returnImmediately"),
            encoding="utf-8",
        )
    (tmp_path / "src/test/func_80100000.c").rename(
        tmp_path / "src/test/returnImmediately.c"
    )
    with pytest.raises(ValueError, match="transaction records differ from provenance"):
        verify(tmp_path, TARGET, report, "function:func_80100000")


def test_reviewed_rz_scope_digest_covers_reviewed_annotations(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    from harness.domain.manifests import load_target_manifests
    from harness.naming.context import TargetContext, reviewed_scope_digest

    manifest = load_target_manifests(tmp_path)[TARGET]
    ctx = TargetContext(tmp_path, TARGET, manifest)
    assert reviewed_scope_digest(tmp_path, TARGET) is None
    (tmp_path / "config/targets/exe/test/reviewed.rz").write_text(
        "f data.handlerIndex 1 @ 0x80100010\n", encoding="utf-8"
    )
    first = reviewed_scope_digest(tmp_path, TARGET)
    assert first is not None
    facts = pre_apply(ctx, "function", "func_80100000", _report(tmp_path)["rows"][0])
    assert facts["facts"]["reviewed_digest"] == first
    # The digest is re-derived from current file contents: a reviewed .rz that
    # changes (or first appears) mid-transaction must no longer hash to the
    # captured record, so a fresh digest drifts from the recorded one.
    (tmp_path / "config/targets/exe/test/reviewed.rz").write_text(
        "f data.handlerIndex 2 @ 0x80100010\n", encoding="utf-8"
    )
    assert reviewed_scope_digest(tmp_path, TARGET) != first


def test_transaction_check_captures_versioned_pre_apply_facts(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report, report_path = _prepared_report(tmp_path, monkeypatch=monkeypatch)
    result = validate(
        tmp_path,
        TARGET,
        report,
        transaction="function:func_80100000",
        report_path=report_path,
    )
    assert result["ready"] is True
    binding = result["pre_apply"]
    assert binding["version"] == DIGEST_VERSION
    assert binding["digest"].startswith(f"v{DIGEST_VERSION}:")
    facts = binding["facts"]
    assert facts["selector"] == "exe/test@0x80100000"
    assert facts["status"] == {
        "rung_status": "proposed",
        "semantic_status": "accepted",
        "transaction_status": "ready",
    }
    assert facts["scope"]["binding_locations"] == [
        "config/targets/exe/test/symbols.txt"
    ]
    assert facts["reviewed"] == []
    assert facts["reviewed_digest"] is None


def test_post_apply_rejects_new_name_change_after_capture(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _prepared_post_apply(tmp_path, monkeypatch)
    for path in (
        tmp_path / "config/targets/exe/test/symbols.txt",
        tmp_path / "config/targets/exe/test/splat.yaml",
        tmp_path / "config/targets/exe/test/target.toml",
        tmp_path / "src/test/func_80100000.c",
    ):
        path.write_text(
            path.read_text().replace("func_80100000", "returnImmediately"),
            encoding="utf-8",
        )
    old_source = tmp_path / "src/test/func_80100000.c"
    if old_source.exists():
        old_source.rename(tmp_path / "src/test/returnImmediately.c")
    # The applied spelling must equal the captured proposal; a second rename
    # (or an invented new name) after capture is a fresh transaction.
    report["rows"][0]["new_name"] = "laterName"
    report["rows"][0]["identity"]["new"] = "laterName"
    with pytest.raises(ValueError, match="proposal row digest drifted"):
        verify(tmp_path, TARGET, report, "function:func_80100000")


def test_post_apply_rejects_pre_apply_digest_drift(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report, _ = _prepared_report(tmp_path, monkeypatch=monkeypatch)
    binding = report["rows"][0]["pre_apply"]
    binding["facts"]["scope"]["binding_locations"] = [
        "config/targets/exe/test/invented.txt"
    ]
    report["rows"][0]["pre_apply"] = binding
    for path in (
        tmp_path / "config/targets/exe/test/symbols.txt",
        tmp_path / "config/targets/exe/test/splat.yaml",
        tmp_path / "config/targets/exe/test/target.toml",
        tmp_path / "src/test/func_80100000.c",
    ):
        path.write_text(
            path.read_text().replace("func_80100000", "returnImmediately"),
            encoding="utf-8",
        )
    old_source = tmp_path / "src/test/func_80100000.c"
    if old_source.exists():
        old_source.rename(tmp_path / "src/test/returnImmediately.c")
    with pytest.raises(ValueError, match="transaction records differ from provenance"):
        verify(tmp_path, TARGET, report, "function:func_80100000")


def test_post_apply_rejects_older_digest_version(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report, _ = _prepared_report(tmp_path, monkeypatch=monkeypatch)
    binding = report["rows"][0]["pre_apply"]
    binding["version"] = 0
    report["rows"][0]["pre_apply"] = binding
    for path in (
        tmp_path / "config/targets/exe/test/symbols.txt",
        tmp_path / "config/targets/exe/test/splat.yaml",
        tmp_path / "config/targets/exe/test/target.toml",
        tmp_path / "src/test/func_80100000.c",
    ):
        path.write_text(
            path.read_text().replace("func_80100000", "returnImmediately"),
            encoding="utf-8",
        )
    old_source = tmp_path / "src/test/func_80100000.c"
    if old_source.exists():
        old_source.rename(tmp_path / "src/test/returnImmediately.c")
    with pytest.raises(ValueError, match="transaction records differ from provenance"):
        verify(tmp_path, TARGET, report, "function:func_80100000")


def test_post_apply_requires_captured_pre_apply_binding(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _report(tmp_path)
    for path in (
        tmp_path / "config/targets/exe/test/symbols.txt",
        tmp_path / "config/targets/exe/test/splat.yaml",
        tmp_path / "config/targets/exe/test/target.toml",
        tmp_path / "src/test/func_80100000.c",
    ):
        path.write_text(
            path.read_text().replace("func_80100000", "returnImmediately"),
            encoding="utf-8",
        )
    old_source = tmp_path / "src/test/func_80100000.c"
    if old_source.exists():
        old_source.rename(tmp_path / "src/test/returnImmediately.c")
    with pytest.raises(
        ValueError, match="proposal provenance has a non-canonical shape"
    ):
        verify(tmp_path, TARGET, report, "function:func_80100000")


def test_post_apply_rejects_changed_map_address(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _prepared_post_apply(tmp_path, monkeypatch)
    for path in (
        tmp_path / "config/targets/exe/test/symbols.txt",
        tmp_path / "config/targets/exe/test/splat.yaml",
        tmp_path / "config/targets/exe/test/target.toml",
        tmp_path / "src/test/func_80100000.c",
    ):
        text = path.read_text().replace("func_80100000", "returnImmediately")
        if path.name == "symbols.txt":
            text = text.replace("0x80100000", "0x80100008")
        path.write_text(text, encoding="utf-8")
    (tmp_path / "src/test/func_80100000.c").rename(
        tmp_path / "src/test/returnImmediately.c"
    )
    with pytest.raises(ValueError, match="map address changed"):
        verify(tmp_path, TARGET, report, "function:func_80100000")


def test_post_apply_rejects_old_spelling_in_local_include(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    source = tmp_path / "src/test/func_80100000.c"
    source.write_text('#include "local.h"\n' + source.read_text(), encoding="utf-8")
    local = source.parent / "local.h"
    local.write_text("void func_80100000(void);\n", encoding="utf-8")
    report = _report(tmp_path)
    report["rows"][0]["identity"]["source_locations"].append("src/test/local.h")
    report, _ = _prepared_report(tmp_path, report, monkeypatch)
    report["rows"][0]["post_apply_receipts"] = _post_apply_receipts(tmp_path)
    for path in (
        tmp_path / "config/targets/exe/test/symbols.txt",
        tmp_path / "config/targets/exe/test/splat.yaml",
        source,
    ):
        path.write_text(
            path.read_text().replace("func_80100000", "returnImmediately"),
            encoding="utf-8",
        )
    with pytest.raises(ValueError, match="old spelling remains"):
        verify(tmp_path, TARGET, report, "function:func_80100000")


def test_post_apply_rejects_remaining_old_spelling(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report, _ = _prepared_report(tmp_path, monkeypatch=monkeypatch)
    with pytest.raises(ValueError, match="binding scope|old spelling|pre-apply"):
        verify(tmp_path, TARGET, report, "function:func_80100000")


def test_post_apply_requires_transaction(tmp_path: Path) -> None:
    _repo(tmp_path)
    with pytest.raises(ValueError, match="requires --transaction"):
        verify(tmp_path, TARGET, _report(tmp_path), None)


def test_v2_is_retired(tmp_path: Path) -> None:
    _repo(tmp_path)
    with pytest.raises(ValueError, match="v2 is retired"):
        validate(tmp_path, TARGET, {"schema": "bof3.naming-audit/v2"})


def test_transaction_scope_rejects_omitted_or_invented_locations(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report, report_path = _prepared_report(tmp_path, monkeypatch=monkeypatch)
    report["rows"][0]["identity"]["source_locations"] = ["invented.c"]
    with pytest.raises(ValueError, match="must equal derived scope"):
        validate(
            tmp_path,
            TARGET,
            report,
            transaction="function:func_80100000",
            report_path=report_path,
        )


def test_blocked_rejects_unsubstantiated_generated_work(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(
        "harness.naming.audit.connect_index",
        lambda *_, **__: _DataReferenceConnection(),
    )
    report = _report(tmp_path)
    report["rows"][0]["rung_status"] = "blocked"
    report["rows"][0].pop("conclusion_provenance", None)
    report["rows"][0]["smallest_repair"] = (
        "bin/rev-query --json callers exe/test@0x80100000"
    )
    report["rows"][0]["rungs"]["selected_call"]["status"] = "open"
    report["rows"][0]["rungs"]["selected_call"]["next_command"] = report["rows"][0][
        "smallest_repair"
    ]
    row = report["rows"][1]
    row["required_work"] = [{"id": "access:x", "status": "open"}]
    with pytest.raises(ValueError, match="blocked row requires failed rung"):
        validate(tmp_path, TARGET, report)


def test_corroborators_must_link_independent_observations(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report, report_path = _prepared_report(tmp_path, monkeypatch=monkeypatch)
    report["rows"][0]["corroborators"]["B"] = {
        "observation_ids": ["func_80100000.selected_call"],
        "mechanism": "selected_original_instructions",
    }
    with pytest.raises(ValueError, match="share evidence"):
        validate(
            tmp_path,
            TARGET,
            report,
            transaction="function:func_80100000",
            report_path=report_path,
        )


def _data_proposal(root: Path) -> dict[str, Any]:
    """Build the historical DATA proposal used by storage validator tests."""
    report = _report(root)
    function = report["rows"][0]
    function["rung_status"] = "blocked"
    function["smallest_repair"] = "bin/rev-query --json callers exe/test@0x80100000"
    function["rungs"]["selected_call"]["status"] = "open"
    function["rungs"]["selected_call"]["next_command"] = function["smallest_repair"]
    data = report["rows"][1]
    data.update(
        {
            "rung_status": "proposed",
            "new_name": "handlerIndex",
            "semantic_status": "accepted",
            "transaction_status": "ready",
            "readiness_blockers": [],
            "corroborators": {
                "A": {
                    "observation_ids": ["D_80100010.selected_access"],
                    "mechanism": "selected_original_instructions",
                },
                "B": {
                    "observation_ids": ["D_80100010.one_level_beyond"],
                    "mechanism": "independent_consumer",
                },
            },
            "name_terms": {"handler": ["A"], "index": ["B"]},
            "identity": {
                "selector": "exe/test@0x80100010",
                "old": "D_80100010",
                "new": "handlerIndex",
                "unchanged_range": "0x80100010..0x80100011",
                "binding_locations": ["config/targets/exe/test/symbols.txt"],
                "source_locations": ["config/targets/exe/test/target.toml"],
            },
            "storage": {
                "kind": "data",
                "start": "0x80100010",
                "end": "0x80100020",
                "file_offset": "0x10",
                "present_in_binary": True,
                "authority": ["original_binary", "reviewed_splat"],
            },
        }
    )
    return report


def test_storage_authority_order_is_canonicalized(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _data_proposal(tmp_path)
    from harness.naming import audit

    assert (
        audit.validate_v3(
            tmp_path,
            TARGET,
            report,
            audit._context(tmp_path, TARGET),
            transaction="data:D_80100010",
        )["ready"]
        is True
    )


def test_duplicate_work_profile_comes_from_generated_item(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report, report_path = _prepared_report(tmp_path, monkeypatch=monkeypatch)
    generated = [
        {"id": "caller:a", "profile": "caller_context", "description": "a"},
        {"id": "callee:b", "profile": "callee_body", "description": "b"},
    ]
    monkeypatch.setattr(
        "harness.naming.context.required_work_items", lambda *_: generated
    )
    report["rows"][0]["required_work"] = [
        {
            "id": "caller:a",
            "profile": "callee_body",
            "status": "completed",
            "commands": [_command(tmp_path, "caller-a")],
            "observations": [_observation("caller-a")],
        },
        {"id": "callee:b", "status": "duplicate", "duplicate_of": "caller:a"},
    ]
    with pytest.raises(ValueError, match="same-profile"):
        validate(
            tmp_path,
            TARGET,
            report,
            transaction="function:func_80100000",
            report_path=report_path,
        )


def test_new_name_must_equal_identity_new(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report, report_path = _prepared_report(tmp_path, monkeypatch=monkeypatch)
    report["rows"][0]["identity"]["new"] = "differentName"
    with pytest.raises(ValueError, match="new_name must equal"):
        validate(
            tmp_path,
            TARGET,
            report,
            transaction="function:func_80100000",
            report_path=report_path,
        )


def test_partial_used_is_repository_derived(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report, report_path = _prepared_report(tmp_path, monkeypatch=monkeypatch)
    source = tmp_path / "src/test/func_80100000.c"
    source.write_text(
        source.read_text()
        .replace("@status exact", "@status partial")
        .replace("@match 100.00", "@match 50.00")
        .replace("@residual none", "@residual instruction order differs"),
        encoding="utf-8",
    )
    with pytest.raises(ValueError, match="partial_used must match"):
        validate(
            tmp_path,
            TARGET,
            report,
            transaction="function:func_80100000",
            report_path=report_path,
        )


def test_data_storage_must_equal_canonical_storage(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    report = _data_proposal(tmp_path)
    report["rows"][1]["storage"] = {
        "kind": "bss",
        "start": "0x80100010",
        "end": "0x80100011",
        "present_in_binary": False,
        "authority": [],
    }
    from harness.naming import audit

    with pytest.raises(ValueError, match="canonical storage"):
        audit.validate_v3(
            tmp_path,
            TARGET,
            report,
            audit._context(tmp_path, TARGET),
            transaction="data:D_80100010",
        )
