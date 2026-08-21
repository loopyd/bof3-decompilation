"""Tests for current exactly-once macro opportunity accounting."""

from __future__ import annotations

import copy
import hashlib
import sqlite3
from pathlib import Path

import pytest

from harness.analysis import macro_accounting
from harness.analysis.schema import create_schema
from harness.commands.macro_audit import main

TARGET = "exe/test"
BINARY_DATA = b""
SOURCE_TEXT = "int f(void) { int a=17,b=17,c=17; return a+b+c; }\n"


def _database(root: Path) -> Path:
    database = root / "index.sqlite"
    connection = sqlite3.connect(database)
    create_schema(connection)
    binary = root / "test.bin"
    binary.write_bytes(BINARY_DATA)
    connection.execute(
        "INSERT INTO targets VALUES (?, ?, ?, 0x80100000, 'rizin', 'v', 's', 'sh')",
        (TARGET, "test.bin", hashlib.sha256(BINARY_DATA).hexdigest()),
    )
    source = root / "src/test.c"
    source.parent.mkdir(parents=True)
    source.write_text(SOURCE_TEXT, encoding="utf-8")
    connection.execute(
        "INSERT INTO macro_input_fingerprints VALUES (?, ?, ?, 'source_claim', ?)",
        (
            TARGET,
            "src/test.c",
            hashlib.sha256(SOURCE_TEXT.encode()).hexdigest(),
            TARGET,
        ),
    )
    connection.commit()
    connection.close()
    return database


def _connect(database: Path) -> sqlite3.Connection:
    connection = sqlite3.connect(database)
    connection.row_factory = sqlite3.Row
    return connection


def test_account_is_exactly_once_fresh_blocked_and_zero_safe(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    database = _database(tmp_path)
    monkeypatch.setattr(macro_accounting, "connect", lambda _root: _connect(database))

    report = macro_accounting.candidate_account(tmp_path)

    assert report["complete"] is True
    assert report["fresh"] is True
    assert report["candidate_count"] == 1
    assert report["safe_application_count"] == 0
    assert report["counts"] == {"blocked": 1}
    assert len({row["id"] for row in report["rows"]}) == len(report["rows"])
    assert report["rows"][0]["status"] == "blocked"
    assert report["rows"][0]["blocked_reason"]
    assert report["rows"][0]["candidate_fingerprint"].startswith("v1:")
    assert report["source_input_fingerprint"].startswith("v1:")
    assert all(item["fresh"] is True for item in report["inputs"])
    assert macro_accounting.validate_account(tmp_path, report) == report


def test_validate_rejects_missing_duplicate_and_extra_rows(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    database = _database(tmp_path)
    monkeypatch.setattr(macro_accounting, "connect", lambda _root: _connect(database))
    report = macro_accounting.candidate_account(tmp_path)
    extra = {**report["rows"][0], "id": "constant:extra"}

    for rows in ([], report["rows"] * 2, [*report["rows"], extra]):
        changed = copy.deepcopy(report)
        changed["rows"] = rows
        with pytest.raises(ValueError, match="stale, incomplete, or duplicated"):
            macro_accounting.validate_account(tmp_path, changed)


def test_account_rejects_stale_source_binary_and_report_fingerprints(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    database = _database(tmp_path)
    monkeypatch.setattr(macro_accounting, "connect", lambda _root: _connect(database))
    report = macro_accounting.candidate_account(tmp_path)

    changed = copy.deepcopy(report)
    changed["source_input_fingerprint"] = "v1:stale"
    with pytest.raises(ValueError, match="stale, incomplete, or duplicated"):
        macro_accounting.validate_account(tmp_path, changed)

    changed = copy.deepcopy(report)
    changed["inputs"][0]["sha256"] = "stale"
    with pytest.raises(ValueError, match="stale, incomplete, or duplicated"):
        macro_accounting.validate_account(tmp_path, changed)

    (tmp_path / "src/test.c").write_text("int changed;\n", encoding="utf-8")
    with pytest.raises(ValueError, match="stale macro opportunity source"):
        macro_accounting.candidate_account(tmp_path)

    (tmp_path / "src/test.c").write_text(SOURCE_TEXT, encoding="utf-8")
    (tmp_path / "test.bin").write_bytes(b"changed")
    with pytest.raises(ValueError, match="stale macro account input: binary:exe/test"):
        macro_accounting.candidate_account(tmp_path)


def test_account_rejects_duplicate_ids_or_blocked_without_reason(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    database = _database(tmp_path)
    monkeypatch.setattr(macro_accounting, "connect", lambda _root: _connect(database))
    candidate = {
        "id": "same",
        "kind": "constant",
        "status": "blocked",
        "blockers": ["review_required"],
    }
    monkeypatch.setattr(
        macro_accounting,
        "macro_opportunities_payload",
        lambda *_args, **_kwargs: [candidate, candidate],
    )
    monkeypatch.setattr(
        macro_accounting,
        "near_duplicates_payload",
        lambda *_args, **_kwargs: [],
    )
    with pytest.raises(ValueError, match="not exactly once"):
        macro_accounting.candidate_account(tmp_path)

    candidate["id"] = "one"
    candidate["blockers"] = []
    monkeypatch.setattr(
        macro_accounting,
        "macro_opportunities_payload",
        lambda *_args, **_kwargs: [candidate],
    )
    with pytest.raises(ValueError, match="lacks explicit reason"):
        macro_accounting.candidate_account(tmp_path)


def test_safe_application_count_counts_only_accepted_candidates(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    database = _database(tmp_path)
    monkeypatch.setattr(macro_accounting, "connect", lambda _root: _connect(database))
    monkeypatch.setattr(
        macro_accounting,
        "macro_opportunities_payload",
        lambda *_args, **_kwargs: [
            {
                "id": "blocked",
                "kind": "constant",
                "status": "blocked",
                "blockers": ["review_required"],
            },
            {
                "id": "accepted",
                "kind": "constant",
                "status": "accepted",
                "blockers": [],
            },
        ],
    )
    monkeypatch.setattr(
        macro_accounting,
        "near_duplicates_payload",
        lambda *_args, **_kwargs: [],
    )

    report = macro_accounting.candidate_account(tmp_path)

    assert report["counts"] == {"accepted": 1, "blocked": 1}
    assert report["safe_application_count"] == 1
    assert {row["id"]: row["blocked_reason"] for row in report["rows"]} == {
        "accepted": None,
        "blocked": "review_required",
    }


def test_macro_audit_cli_writes_and_validates_account(
    tmp_path: Path,
    monkeypatch: pytest.MonkeyPatch,
    capsys: pytest.CaptureFixture[str],
) -> None:
    database = _database(tmp_path)
    monkeypatch.setattr(macro_accounting, "connect", lambda _root: _connect(database))
    output = tmp_path / "account.json"

    assert main(["--root", str(tmp_path), "account", str(output)]) == 0
    assert '"safe_application_count": 0' in capsys.readouterr().out
    assert main(["--root", str(tmp_path), "validate-account", str(output)]) == 0
    assert '"fresh": true' in capsys.readouterr().out
