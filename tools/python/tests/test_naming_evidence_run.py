"""P0 native runner: one process, one shared index connection, bounded
resumable evidence collection with digest-bound receipts.

The runner gathers indexed facts for a report row's generated work; query
success never closes a semantic rung, so rows stay explicitly blocked until
typed observations are supplied and validated by the owning validator.
"""

from __future__ import annotations

import hashlib
import json
import os
import signal
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from types import SimpleNamespace
from typing import Any

import harness.naming.audit as naming_audit
import pytest
from harness.analysis.index import index_path, rebuild
from harness.analysis.project import prepare_target
from harness.analysis.snapshot import (
    SNAPSHOT_SCHEMA,
    AnalysisSnapshot,
    SnapshotCall,
    SnapshotFunction,
    snapshot_path,
    write_snapshot,
)
from harness.naming.debt import address_of
from harness.domain.receipts import command_records
from harness.naming.client import IndexWorker
from harness.naming.collection import terminalize_report
from harness.naming.state import checkpoint_path
from harness.naming.state import load_checkpoint
from harness.naming.state import manifest_path
from harness.naming.execution import run_evidence
from harness.naming.state import select_rows
from harness.naming.inventory import _report_set_digest, publish_reports
from harness.naming.journal import entry_sha256, load_journal
from harness.naming.plan import parse_semantic_command, semantic_operation_plan
from harness.naming.runner import build_parser
from harness.naming.worker import _query

TARGET = "exe/test"


def test_rows_option_rejects_repeated_use() -> None:
    parser = build_parser()
    with pytest.raises(SystemExit):
        parser.parse_args(
            [
                TARGET,
                "report.json",
                "--rows",
                "data:D_80100000",
                "--rows",
                "data:D_80100004",
            ]
        )


def _repo(root: Path) -> None:
    config = root / "config/targets/exe/test/target.toml"
    config.parent.mkdir(parents=True)
    config.write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src/test'\n"
        "binary='out/test.bin'\nload_address=0x80100000\n"
        "splat='config/targets/exe/test/splat.yaml'\n"
        "sources=[]\n",
        encoding="utf-8",
    )
    (root / "out").mkdir(parents=True, exist_ok=True)
    (root / "out/test.bin").write_bytes(b"\0" * 0x20)
    (config.parent / "splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [0x20]\n",
        encoding="utf-8",
    )
    (config.parent / "symbols.txt").write_text(
        "func_80100000 = 0x80100000;\n", encoding="utf-8"
    )
    sdk = root / "config/sdk/psyq-slus.txt"
    sdk.parent.mkdir(parents=True, exist_ok=True)
    sdk.write_text("", encoding="utf-8")


_INDEX_FIXTURE: tuple[bytes, bytes] | None = None


def _index(root: Path) -> None:
    """Install one reusable disposable reverse index for the fixture target."""

    global _INDEX_FIXTURE
    snapshot = snapshot_path(root, TARGET)
    index = index_path(root)
    base_types = root / "include/base/types.h"
    base_types.parent.mkdir(parents=True, exist_ok=True)
    base_types.write_text(
        "typedef unsigned char bool;\ntypedef unsigned int u32;\n",
        encoding="utf-8",
    )
    if _INDEX_FIXTURE is not None:
        for path, content in zip((snapshot, index), _INDEX_FIXTURE, strict=True):
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(content)
        return

    binary = root / "out/test.bin"
    functions = [
        SnapshotFunction(
            id=f"{TARGET}@80100000",
            address=0x80100000,
            analyzer_size=16,
            analyzer_name="func_80100000",
            exact_sha256="a" * 64,
        )
    ]
    calls = [
        SnapshotCall(f"{TARGET}@80100000", f"{TARGET}@80100010", 0x80100000),
        SnapshotCall(f"{TARGET}@80100000", f"{TARGET}@80100018", 0x80100004),
    ]
    for callee in (f"{TARGET}@80100010", f"{TARGET}@80100018"):
        functions.append(
            SnapshotFunction(
                id=callee,
                address=int(callee.rsplit("@", 1)[1], 16),
                analyzer_size=8,
                analyzer_name=callee.split("@", 1)[1],
                exact_sha256="b" * 64,
            )
        )
    write_snapshot(
        AnalysisSnapshot(
            schema=SNAPSHOT_SCHEMA,
            target=TARGET,
            engine={"name": "rizin", "version": "test"},
            inputs={
                "binary_sha256": hashlib.sha256(binary.read_bytes()).hexdigest(),
                "replay_sha256": prepare_target(root, TARGET).replay_sha256,
            },
            functions=tuple(functions),
            calls=tuple(calls),
            unresolved_calls=(),
        ),
        snapshot_path(root, TARGET),
    )
    rebuild(root)
    _INDEX_FIXTURE = (snapshot.read_bytes(), index.read_bytes())


def _blocked_row(name: str, work: list[dict[str, str]]) -> dict[str, Any]:
    """A blocked v3 row whose open generated work the runner must collect."""

    selector = f"{TARGET}@0x{address_of(name):08X}"

    def rung(missing: str) -> dict[str, Any]:
        return {
            "status": "open",
            "next_command": f"bin/rev-query --json xrefs {selector}",
            "observations": [
                {
                    "id": f"{name}.gap",
                    "text": f"Evidence Gap: {missing}",
                }
            ],
            "authority": "target manifest, reviewed Splat, original image",
        }

    return {
        "kind": "function",
        "name": name,
        "rung_status": "blocked",
        "outside_payload": False,
        "partial_used": False,
        "rungs": {
            "selected_range": rung("reviewed range and original bytes"),
            "selected_call": rung("callsite instructions"),
            "one_level_beyond": rung("independent consumer"),
        },
        "required_work": [dict(item, status="open") for item in work],
        "optional_work": [],
        "interpretation": "No semantic name is accepted until the gap is closed.",
        "authority": "target manifest, target-local map, reviewed Splat, original image",
        "smallest_repair": f"bin/rev-query --json xrefs {selector}",
        "missing_fact": "consumer",
        "ceiling_next_command": f"bin/rev-query --json xrefs {selector}",
    }


def _report(root: Path, rows: list[dict[str, Any]]) -> Path:
    report = root / "out" / "reviews" / f"{TARGET.replace('/', '__')}.json"
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(
        json.dumps(
            {"schema": "bof3.naming-audit/v3", "target": TARGET, "rows": rows},
            indent=2,
            sort_keys=True,
        )
        + "\n",
        encoding="utf-8",
    )
    return report


def _callee_work() -> list[dict[str, str]]:
    """Generated callee work items matching the fixture snapshot's two calls."""

    return [
        {
            "id": f"callee:{TARGET}@80100010",
            "profile": "callee_body",
            "description": "resolve callee",
        },
        {
            "id": f"callee:{TARGET}@80100018",
            "profile": "callee_body",
            "description": "resolve callee",
        },
    ]


def _large_fixture(root: Path, rows: int, calls_per_row: int) -> Path:
    """``rows`` deterministic function rows plus a matching snapshot/index."""

    base_types = root / "include/base/types.h"
    base_types.parent.mkdir(parents=True, exist_ok=True)
    base_types.write_text(
        "typedef unsigned char bool;\ntypedef unsigned int u32;\n",
        encoding="utf-8",
    )
    binary = root / "out/test.bin"
    binary.write_bytes(b"\0" * (rows * 0x10 + 0x20))
    symbols: list[str] = []
    functions: list[SnapshotFunction] = []
    calls: list[SnapshotCall] = []
    for index in range(rows):
        address = 0x80100000 + index * 0x10
        name = f"func_{address:08X}"
        symbols.append(f"{name} = 0x{address:08X};\n")
        functions.append(
            SnapshotFunction(
                id=f"{TARGET}@{address:08X}",
                address=address,
                analyzer_size=8,
                analyzer_name=name,
                exact_sha256=hashlib.sha256(str(address).encode()).hexdigest(),
            )
        )
        for call_index in range(calls_per_row):
            callee_address = address + 8 + call_index
            callee = f"{TARGET}@{callee_address:08X}"
            calls.append(SnapshotCall(f"{TARGET}@{address:08X}", callee, address))
            # Callees stay snapshot functions (call-graph valid) but never map
            # rows, so the debt inventory stays exactly the ``rows`` main rows.
            functions.append(
                SnapshotFunction(
                    id=callee,
                    address=callee_address,
                    analyzer_size=8,
                    analyzer_name=f"func_{callee_address:08X}",
                    exact_sha256=hashlib.sha256(
                        str(callee_address).encode()
                    ).hexdigest(),
                )
            )
    (root / "config/targets/exe/test/symbols.txt").write_text(
        "".join(symbols), encoding="utf-8"
    )
    (root / "config/targets/exe/test/splat.yaml").write_text(
        "segments:\n  - [0]\n", encoding="utf-8"
    )
    write_snapshot(
        AnalysisSnapshot(
            schema=SNAPSHOT_SCHEMA,
            target=TARGET,
            engine={"name": "rizin", "version": "test"},
            inputs={
                "binary_sha256": hashlib.sha256(binary.read_bytes()).hexdigest(),
                "replay_sha256": prepare_target(root, TARGET).replay_sha256,
            },
            functions=tuple(functions),
            calls=tuple(calls),
            unresolved_calls=(),
        ),
        snapshot_path(root, TARGET),
    )
    rebuild(root)
    report_rows = []
    for index in range(rows):
        address = 0x80100000 + index * 0x10
        work = [
            {
                "id": f"callee:{TARGET}@{address + 8 + call_index:08X}",
                "profile": "callee_body",
                "description": "resolve callee",
            }
            for call_index in range(calls_per_row)
        ]
        report_rows.append(_blocked_row(f"func_{address:08X}", work))
    return _report(root, report_rows)


def test_index_worker_isolates_non_word_dispatch_extent(tmp_path: Path) -> None:
    """A non-table data extent cannot abort the following worker request."""

    _repo(tmp_path)
    (tmp_path / "config/targets/exe/test/symbols.txt").write_text(
        "func_80100000 = 0x80100000;\nD_80100010 = 0x80100010;\n",
        encoding="utf-8",
    )
    (tmp_path / "config/targets/exe/test/splat.yaml").write_text(
        "segments:\n"
        "  - [0, c, func_80100000]\n"
        "  - [0x10, data, D_80100010]\n"
        "  - [0x18, c, func_80100018]\n"
        "  - [0x20]\n",
        encoding="utf-8",
    )
    global _INDEX_FIXTURE
    shared_fixture = _INDEX_FIXTURE
    _INDEX_FIXTURE = None
    _index(tmp_path)
    connection = __import__("sqlite3").connect(index_path(tmp_path))
    connection.execute(
        "INSERT INTO data_references "
        "(target_id, function_id, source, address, symbol, access_kind, opcode) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)",
        (
            TARGET,
            f"{TARGET}@80100000",
            0x80100004,
            0x80100010,
            "D_80100010",
            "write",
            "sb",
        ),
    )
    connection.commit()
    connection.close()
    data_row = _blocked_row(
        "D_80100010",
        [
            {
                "id": f"access:{TARGET}@80100000",
                "profile": "access_context",
                "description": "resolve consumer",
            }
        ],
    )
    data_row["kind"] = "data"
    data_row["rungs"] = {
        name: data_row["rungs"].get("selected_range")
        for name in (
            "selected_range",
            "selected_access",
            "storage_class",
            "one_level_beyond",
        )
    }
    later_row = _blocked_row("func_80100000", _callee_work())
    report = _report(tmp_path, [data_row, later_row])
    report_payload = json.loads(report.read_text(encoding="utf-8"))

    worker = IndexWorker(tmp_path, TARGET, report_payload)
    try:
        assert worker.receive(10) == {"ready": True}
        operations = worker.request(1, data_row, 10)
        assert worker.request(2, later_row, 10) is not None
    finally:
        worker.close()
        _INDEX_FIXTURE = shared_fixture

    dispatch = [
        operation
        for operation in operations
        if operation["operation"] == "data_dispatch_consumer"
    ][0]
    assert dispatch["selector"] == f"{TARGET}@80100000"
    assert dispatch["payload"]["selected"] == f"{TARGET}@80100010"
    assert dispatch["payload"]["selected_range"] == [0x80100010, 0x80100011]
    assert dispatch["payload"]["selected_bytes"] == "00"
    assert dispatch["payload"]["pointer_descriptions"] == []
    assert (
        dispatch["payload"]["image_sha256"]
        == hashlib.sha256((tmp_path / "out/test.bin").read_bytes()).hexdigest()
    )


def test_owner_work_uses_qualified_function_id(monkeypatch: pytest.MonkeyPatch) -> None:
    captured: list[Any] = []

    def owner_payload(connection: Any, function: Any) -> list[Any]:
        captured.append(function)
        return [{"target_id": function.target.value}]

    monkeypatch.setattr("harness.analysis.rev_queries.owner_payload", owner_payload)
    operation, selector, payload = _query(None, "owner:emi/etc/game/00@80123456")
    assert (operation, selector, payload) == (
        "owner",
        "emi/etc/game/00@80123456",
        [{"target_id": "emi/etc/game/00"}],
    )
    assert captured[0].target.value == "emi/etc/game/00"
    assert captured[0].address == 0x80123456


def test_owner_placeholder_is_qualified_or_omitted() -> None:
    row = _blocked_row("func_80100000", [])
    row["ceiling_next_command"] = (
        "bin/rev-query --json owners exe/test@0x80100000; "
        "bin/rz-project query OWNER -c 'pdf @ 0x80100000'"
    )
    assert semantic_operation_plan(row, TARGET) == []

    row["required_work"] = [
        {
            "id": "owner:emi/etc/game/00@80123456",
            "profile": "owner_resolution",
            "description": "resolve owner",
            "status": "open",
        }
    ]
    assert semantic_operation_plan(row, TARGET) == []


def test_evidence_run_collects_one_row_with_valid_receipts(tmp_path: Path) -> None:
    """One protocol worker collects a row and emits validator-valid receipts."""
    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", _callee_work())])
    result = run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert result["executed"] == 1
    assert result["errors"] == []
    entry = load_checkpoint(tmp_path, report)["function:func_80100000"]
    payload = json.loads((tmp_path / entry["evidence"]).read_text(encoding="utf-8"))
    assert len(payload["commands"]) == 2
    for record in payload["commands"]:
        # The owning validator accepts the command record and receipt file.
        normalized = command_records([record], "runner", tmp_path)
        receipt = json.loads((tmp_path / record["receipt"]).read_text(encoding="utf-8"))
        assert set(receipt) == {"command", "status", "target", "selector", "output"}
        assert (
            hashlib.sha256((tmp_path / record["receipt"]).read_bytes()).hexdigest()
            == record["sha256"]
        )
        assert normalized[0]["receipt"] == record["receipt"]
    manifest = json.loads((tmp_path / result["manifest"]).read_text(encoding="utf-8"))
    assert manifest["rows"][0]["row"] == "function:func_80100000"
    # The recorded sha256 hashes the manifest file itself, which never
    # carries its own hash field (no self-referential digest).
    assert (
        hashlib.sha256((tmp_path / result["manifest"]).read_bytes()).hexdigest()
        == result["manifest_sha256"]
    )


def test_explicit_evidence_root_telemetry_reconciles(tmp_path: Path) -> None:
    from harness.domain.receipts import reset_receipt_root, set_receipt_root
    from harness.naming.namespace import reset_evidence_root
    from harness.naming.namespace import set_evidence_root

    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", _callee_work())])
    explicit = (tmp_path.parent / f"{tmp_path.name}-explicit").resolve()
    evidence_token = set_evidence_root(explicit)
    receipt_token = set_receipt_root(explicit)
    try:
        result = run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    finally:
        reset_receipt_root(receipt_token)
        reset_evidence_root(evidence_token)
    assert result["telemetry_summary"]["receipt_bytes"] > 0
    assert result["telemetry_summary"]["stdout_bytes"] > 0


def test_resume_never_replays_completed_rows(tmp_path: Path) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", _callee_work())])
    first = run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert first["executed"] == 1
    checkpoint = checkpoint_path(tmp_path, report)
    checkpoint_before = checkpoint.read_bytes()
    mtime_before = checkpoint.stat().st_mtime_ns
    second = run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert second["executed"] == 0
    assert second["resumed"] == 1
    assert second["manifest_sha256"] == first["manifest_sha256"], (
        "an unchanged rerun executes zero completed indexed operations"
    )
    assert checkpoint.read_bytes() == checkpoint_before
    assert checkpoint.stat().st_mtime_ns == mtime_before
    telemetry = second["telemetry_summary"]
    assert telemetry["rows"] == {
        "planned": 0,
        "completed": 0,
        "skipped": 1,
        "stale": 0,
    }
    assert telemetry["index_opens"] == 1
    entries = load_journal(tmp_path, report, TARGET)[0]
    receipt_bytes = stdout_bytes = 0
    for entry in entries.values():
        payload = json.loads((tmp_path / entry["evidence"]).read_text(encoding="utf-8"))
        for command in payload["commands"]:
            receipt_bytes += (tmp_path / command["receipt"]).stat().st_size
            stdout_bytes += len(command["output"].encode("utf-8"))
    assert telemetry["receipt_bytes"] == receipt_bytes > 0
    assert telemetry["stdout_bytes"] == stdout_bytes > 0
    assert first["telemetry_summary"]["receipt_bytes"] == receipt_bytes
    assert first["telemetry_summary"]["stdout_bytes"] == stdout_bytes
    assert telemetry["accounted_seconds"] <= telemetry["wall_seconds"] + 0.01
    assert (tmp_path / second["telemetry"]).stat().st_size <= 4096


def test_unchanged_ten_row_shard_is_zero_op_and_reconciled(tmp_path: Path) -> None:
    _repo(tmp_path)
    _large_fixture(tmp_path, 10, calls_per_row=1)
    report = tmp_path / "out/reviews/exe__test.json"
    first = run_evidence(tmp_path, TARGET, report, all_rows=True, rows_budget=10)
    second = run_evidence(tmp_path, TARGET, report)
    assert first["executed"] == 10
    assert second["executed"] == 0
    assert second["resumed"] == 10
    assert second["manifest_sha256"] == first["manifest_sha256"]
    telemetry = second["telemetry_summary"]
    assert telemetry["rows"] == {
        "planned": 0,
        "completed": 0,
        "skipped": 10,
        "stale": 0,
    }
    assert abs(telemetry["wall_seconds"] - telemetry["accounted_seconds"]) <= 0.01
    assert (tmp_path / second["telemetry"]).stat().st_size <= 4096


def test_successful_collection_keeps_semantic_rungs_blocked(tmp_path: Path) -> None:
    """Query success is a fact, not an interpretation: rungs stay open."""
    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", _callee_work())])
    run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    updated = json.loads(report.read_text(encoding="utf-8"))
    assert updated["rows"][0]["rung_status"] == "blocked", (
        "collection does not close rows unless the owning terminalization gate runs"
    )


def test_collection_receipts_cannot_terminalize_without_semantic_conclusions(
    tmp_path: Path,
) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", _callee_work())])
    before = report.read_bytes()
    result = run_evidence(
        tmp_path,
        TARGET,
        report,
        rows=["function:func_80100000"],
        terminalize=True,
    )
    assert result["terminalized"] is False
    assert report.read_bytes() == before


def test_terminalize_preserves_authored_rows_byte_for_byte(tmp_path: Path) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    row = _blocked_row("func_80100000", _callee_work())
    row["rung_status"] = "proposed"
    row["proposal"] = {"name": "authored_name"}
    report = _report(tmp_path, [row])
    before = report.read_bytes()
    assert terminalize_report(tmp_path, report, TARGET, [row], {}) is False
    assert report.read_bytes() == before


def test_publish_failure_and_restore_failure_preserve_backup(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    output = tmp_path / "reports"
    staging = tmp_path / "staging"
    output.mkdir()
    staging.mkdir()
    (output / "old.json").write_text("old", encoding="utf-8")
    real_replace = Path.replace

    def fail_replaces(path: Path, target: Path) -> Path:
        if path == staging or path.name == ".reports.backup":
            raise OSError("injected rename failure")
        return real_replace(path, target)

    monkeypatch.setattr(Path, "replace", fail_replaces)
    with pytest.raises(OSError, match="injected rename failure"):
        publish_reports(staging, output, expected_digest=_report_set_digest(output))
    backup = tmp_path / ".reports.backup"
    assert backup.is_dir()
    assert (backup / "old.json").read_text(encoding="utf-8") == "old"


def test_publish_retry_preserves_sole_backup_until_success(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    output = tmp_path / "reports"
    backup = tmp_path / ".reports.backup"
    staging = tmp_path / "staging"
    backup.mkdir()
    staging.mkdir()
    (backup / "old.json").write_text("old", encoding="utf-8")
    (staging / "new.json").write_text("new", encoding="utf-8")
    real_replace = Path.replace

    def fail_publish(path: Path, target: Path) -> Path:
        if path == staging:
            raise OSError("injected retry failure")
        return real_replace(path, target)

    monkeypatch.setattr(Path, "replace", fail_publish)
    with pytest.raises(OSError, match="injected retry failure"):
        publish_reports(staging, output, expected_digest=_report_set_digest(output))
    assert not output.exists()
    assert (backup / "old.json").read_text(encoding="utf-8") == "old"

    monkeypatch.setattr(Path, "replace", real_replace)
    publish_reports(staging, output, expected_digest=_report_set_digest(output))
    assert (output / "new.json").read_text(encoding="utf-8") == "new"
    assert not backup.exists()


def test_publish_rejects_concurrent_report_update_without_lost_write(
    tmp_path: Path,
) -> None:
    output = tmp_path / "reports"
    staging = tmp_path / "staging"
    output.mkdir()
    staging.mkdir()
    current = output / "target.json"
    current.write_text("old", encoding="utf-8")
    expected = _report_set_digest(output)
    current.write_text("concurrent", encoding="utf-8")
    (staging / "target.json").write_text("new", encoding="utf-8")

    with pytest.raises(ValueError, match="changed concurrently"):
        publish_reports(staging, output, expected_digest=expected)
    assert current.read_text(encoding="utf-8") == "concurrent"
    assert (staging / "target.json").read_text(encoding="utf-8") == "new"


def test_publish_rejects_creation_after_absent_state_was_captured(
    tmp_path: Path,
) -> None:
    output = tmp_path / "reports"
    staging = tmp_path / "staging"
    expected = _report_set_digest(output)
    output.mkdir()
    staging.mkdir()
    concurrent = output / "target.json"
    concurrent.write_text("concurrent", encoding="utf-8")
    (staging / "target.json").write_text("stale", encoding="utf-8")

    with pytest.raises(ValueError, match="changed concurrently"):
        publish_reports(staging, output, expected_digest=expected)
    assert concurrent.read_text(encoding="utf-8") == "concurrent"
    assert (staging / "target.json").read_text(encoding="utf-8") == "stale"


def test_index_worker_kill_tolerates_dead_child_and_broken_streams() -> None:
    """Cleanup remains idempotent after a child closes its pipes first."""
    from harness.naming.client import IndexWorker

    class BrokenStream:
        def __init__(self) -> None:
            self.close_count = 0

        def close(self) -> None:
            self.close_count += 1
            raise BrokenPipeError

    streams = [BrokenStream() for _ in range(3)]
    worker = IndexWorker.__new__(IndexWorker)
    worker.process = SimpleNamespace(  # type: ignore[assignment]
        poll=lambda: 1,
        stdin=streams[0],
        stdout=streams[1],
        stderr=streams[2],
    )

    worker.kill()
    worker.kill()

    assert [stream.close_count for stream in streams] == [2, 2, 2]


def test_index_worker_timeout_kills_reaps_and_never_commits(tmp_path: Path) -> None:
    """A killed child cannot perform its instrumented late durable write."""
    from harness.naming.client import IndexWorker

    late_write = tmp_path / "parent-owned-checkpoint"
    script = tmp_path / "hang_worker.py"
    script.write_text(
        "import pathlib,sys,time\n"
        "time.sleep(.5)\n"
        "pathlib.Path(sys.argv[1]).write_text('late')\n",
        encoding="utf-8",
    )
    worker = IndexWorker.__new__(IndexWorker)
    worker.process = subprocess.Popen(  # type: ignore[assignment]
        [sys.executable, str(script), str(late_write)],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    started = time.perf_counter()
    try:
        worker.receive(0.1)
    except TimeoutError:
        pass
    else:
        raise AssertionError("hung worker was accepted")
    assert time.perf_counter() - started < 3
    assert worker.process.poll() is not None
    time.sleep(0.6)
    assert not late_write.exists()


def test_real_kill_after_row_then_resume_matches_uninterrupted(tmp_path: Path) -> None:
    """SIGKILL after row N leaves committed evidence reusable and deterministic."""
    clean = tmp_path / "clean"
    interrupted = tmp_path / "interrupted"
    for root in (clean, interrupted):
        root.mkdir()
        _repo(root)
        _large_fixture(root, 3, calls_per_row=1)
    clean_report = clean / "out/reviews/exe__test.json"
    clean_result = run_evidence(
        clean, TARGET, clean_report, all_rows=True, rows_budget=3
    )

    script = interrupted / "kill_after_two.py"
    script.write_text(
        "import os,signal\n"
        "from pathlib import Path\n"
        "import harness.naming.execution as run\n"
        "real=run._commit_row\n"
        "count=0\n"
        "def commit(*args,**kwargs):\n"
        " global count\n"
        " result=real(*args,**kwargs); count+=1\n"
        " if count == 2: os.kill(os.getpid(), signal.SIGKILL)\n"
        " return result\n"
        "run._commit_row=commit\n"
        "root=Path(__file__).parent\n"
        "run.run_evidence(root,'exe/test',root/'out/reviews/exe__test.json',all_rows=True,rows_budget=3)\n",
        encoding="utf-8",
    )
    killed = subprocess.run(
        [sys.executable, str(script)],
        cwd=interrupted,
        env={**os.environ, "PYTHONPATH": str(Path(__file__).parents[1])},
        check=False,
    )
    assert killed.returncode == -signal.SIGKILL
    interrupted_report = interrupted / "out/reviews/exe__test.json"
    committed = load_checkpoint(interrupted, interrupted_report)
    assert len(committed) == 2
    mtimes = {
        key: (interrupted / value["evidence"]).stat().st_mtime_ns
        for key, value in committed.items()
    }
    resumed = run_evidence(
        interrupted, TARGET, interrupted_report, all_rows=True, rows_budget=3
    )
    clean_manifest = json.loads((clean / clean_result["manifest"]).read_text())
    resumed_manifest = json.loads((interrupted / resumed["manifest"]).read_text())
    clean_manifest["report"] = resumed_manifest["report"]
    expected_digest = hashlib.sha256(
        (json.dumps(clean_manifest, indent=2, sort_keys=True) + "\n").encode()
    ).hexdigest()
    assert resumed["manifest_sha256"] == expected_digest
    assert all(
        (interrupted / committed[key]["evidence"]).stat().st_mtime_ns == mtime
        for key, mtime in mtimes.items()
    )
    telemetry = resumed["telemetry_summary"]
    assert telemetry["rows"]["completed"] == 1
    assert telemetry["rows"]["skipped"] == 2
    assert telemetry["rows"]["completed"] + telemetry["rows"]["skipped"] == 3
    assert telemetry["checkpoint_id"] == resumed["manifest_sha256"]
    assert (interrupted / resumed["telemetry"]).stat().st_size <= 4096


def test_249_row_indexed_collection_stays_within_budget(tmp_path: Path) -> None:
    """249 rows traverse one whole-run protocol worker within budget."""
    _repo(tmp_path)
    report = _large_fixture(tmp_path, 249, calls_per_row=2)
    started = time.perf_counter()
    result = run_evidence(tmp_path, TARGET, report, all_rows=True, rows_budget=249)
    elapsed = time.perf_counter() - started
    assert result["executed"] == 249
    assert result["errors"] == []
    assert elapsed <= 60, f"indexed collection must stay within budget: {elapsed:.1f}s"


def test_full_report_validation_stays_within_budget(tmp_path: Path) -> None:
    """Three consecutive full validations of the 249-row report, each <= 30 s."""
    _repo(tmp_path)
    report = _large_fixture(tmp_path, 249, calls_per_row=2)
    for _ in range(3):
        started = time.perf_counter()
        result = naming_audit.validate(
            tmp_path, TARGET, json.loads(report.read_text(encoding="utf-8"))
        )
        elapsed = time.perf_counter() - started
        assert result["rows"] == 249
        assert result["complete"] is False
        assert elapsed <= 30, f"full validation must stay within budget: {elapsed:.1f}s"


def test_row_selection_is_deterministic_and_bounded(tmp_path: Path) -> None:
    rows = [_blocked_row(f"func_8010{i:04X}", []) for i in range(12)]
    report = {
        "schema": "bof3.naming-audit/v3",
        "target": TARGET,
        "rows": rows,
    }
    selected = select_rows(report, rows=None, all_rows=False, shard=10, checkpoint={})
    assert [row["name"] for row in selected] == [f"func_8010{i:04X}" for i in range(10)]
    checkpoint = {f"function:{selected[0]['name']}": {}}
    resumed = select_rows(
        report, rows=None, all_rows=False, shard=10, checkpoint=checkpoint
    )
    assert [row["name"] for row in resumed] == [
        f"func_8010{i:04X}" for i in range(1, 11)
    ]
    explicit = select_rows(
        report,
        rows=["function:func_80100004", "function:func_80100002"],
        all_rows=False,
        shard=10,
        checkpoint=checkpoint,
    )
    assert [row["name"] for row in explicit] == ["func_80100004", "func_80100002"]


def test_bounds_and_trust_fail_before_side_effects(tmp_path: Path) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", _callee_work())])
    evidence_root = tmp_path / "out/reviews/evidence"
    invalid = [
        {"shard": 0},
        {"shard": 11},
        {"concurrency": 0},
        {"concurrency": 5},
        {"deadline": 0},
        {"rows": ["function:func_80100001"]},
        {"rows": ["function:func_80100000"] * 2},
        {"rows": ["function:func_80100000"], "all_rows": True},
    ]
    for kwargs in invalid:
        before = sorted(evidence_root.rglob("*")) if evidence_root.exists() else []
        try:
            run_evidence(tmp_path, TARGET, report, **kwargs)
        except ValueError:
            pass
        else:
            raise AssertionError(f"accepted invalid arguments: {kwargs}")
        after = sorted(evidence_root.rglob("*")) if evidence_root.exists() else []
        assert after == before

    outside = tmp_path.parent / "outside-report.json"
    outside.write_text(report.read_text())
    try:
        run_evidence(tmp_path, TARGET, outside)
    except ValueError:
        pass
    else:
        raise AssertionError("accepted report outside repository")


def test_torn_and_tampered_journal_reselects_rows(tmp_path: Path) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", _callee_work())])
    first = run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert first["executed"] == 1
    checkpoint = checkpoint_path(tmp_path, report)
    checkpoint.write_text(checkpoint.read_text() + '{"row":', encoding="utf-8")
    resumed = run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert resumed["executed"] == 0
    assert checkpoint.with_name("checkpoint.jsonl.tail").is_file()

    entry = load_checkpoint(tmp_path, report)["function:func_80100000"]
    evidence = tmp_path / entry["evidence"]
    evidence.write_text("tampered", encoding="utf-8")
    stale = run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert stale["executed"] == 1


@pytest.mark.parametrize(
    "artifact,mutation",
    [
        ("receipt", "content"),
        ("receipt", "delete"),
        ("raw", "content"),
        ("raw", "delete"),
        ("stderr", "content"),
        ("stderr", "delete"),
        ("raw", "escape"),
        ("receipt", "malformed"),
    ],
)
def test_resume_recollects_tampered_referenced_artifacts(
    tmp_path: Path, artifact: str, mutation: str
) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", _callee_work())])
    assert (
        run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])[
            "executed"
        ]
        == 1
    )
    entry = load_checkpoint(tmp_path, report)["function:func_80100000"]
    payload_path = tmp_path / entry["evidence"]
    payload = json.loads(payload_path.read_text())
    if artifact == "receipt":
        path = tmp_path / payload["commands"][0]["receipt"]
        if mutation == "delete":
            path.unlink()
        elif mutation == "malformed":
            path.write_text("[]\n", encoding="utf-8")
        else:
            path.write_text('{"tampered":true}\n', encoding="utf-8")
    else:
        # Add a digest-bound semantic artifact to exercise resume validation
        # independently of external Rizin availability in this unit fixture.
        path = payload_path.parent / f"probe.{artifact}.raw"
        path.write_text("original", encoding="utf-8")
        item = {
            "raw_file": path.relative_to(tmp_path).as_posix(),
            "raw_sha256": hashlib.sha256(b"original").hexdigest(),
            "raw_size": 8,
            "stderr_file": path.relative_to(tmp_path).as_posix(),
            "stderr_sha256": hashlib.sha256(b"original").hexdigest(),
            "stderr_size": 8,
        }
        payload["semantic"] = [item]
        if mutation == "escape":
            item[f"{artifact}_file"] = "../outside.raw"
        elif mutation == "delete":
            path.unlink()
        else:
            path.write_text("tampered", encoding="utf-8")
        # Keep the payload/checkpoint binding valid so the referenced-artifact
        # contract, rather than the outer payload digest, rejects this row.
        text = json.dumps(payload, indent=2, sort_keys=True) + "\n"
        payload_path.write_text(text, encoding="utf-8")
        checkpoint = checkpoint_path(tmp_path, report)
        line = json.loads(checkpoint.read_text())
        line["evidence_sha256"] = hashlib.sha256(text.encode()).hexdigest()
        line["entry_sha256"] = entry_sha256(line)
        checkpoint.write_text(json.dumps(line, sort_keys=True) + "\n")
        manifest = manifest_path(tmp_path, report)
        recorded = json.loads(manifest.read_text())
        recorded["rows"][0]["evidence_sha256"] = line["evidence_sha256"]
        manifest.write_text(json.dumps(recorded, indent=2, sort_keys=True) + "\n")
    repaired = run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert repaired["executed"] == 1 and not repaired["errors"]
    assert (
        run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])[
            "executed"
        ]
        == 0
    )


def test_lagging_manifest_promotes_valid_journal_prefix(tmp_path: Path) -> None:
    _repo(tmp_path)
    report = _large_fixture(tmp_path, 2, 1)
    assert run_evidence(tmp_path, TARGET, report, all_rows=True)["executed"] == 2
    manifest = manifest_path(tmp_path, report)
    recorded = json.loads(manifest.read_text())
    recorded["rows"] = recorded["rows"][:1]
    manifest.write_text(json.dumps(recorded, indent=2, sort_keys=True) + "\n")

    resumed = run_evidence(tmp_path, TARGET, report, all_rows=True)
    assert resumed["executed"] == 0 and not resumed["errors"]
    assert len(json.loads(manifest.read_text())["rows"]) == 2


def test_lagging_manifest_recollects_tampered_extra_once(tmp_path: Path) -> None:
    _repo(tmp_path)
    report = _large_fixture(tmp_path, 2, 1)
    assert run_evidence(tmp_path, TARGET, report, all_rows=True)["executed"] == 2
    manifest = manifest_path(tmp_path, report)
    recorded = json.loads(manifest.read_text())
    recorded["rows"] = recorded["rows"][:1]
    manifest.write_text(json.dumps(recorded, indent=2, sort_keys=True) + "\n")
    omitted = load_checkpoint(tmp_path, report)["function:func_80100010"]
    payload = json.loads((tmp_path / omitted["evidence"]).read_text())
    (tmp_path / payload["commands"][0]["receipt"]).write_text(
        '{"tampered":true}\n', encoding="utf-8"
    )

    repaired = run_evidence(tmp_path, TARGET, report, all_rows=True)
    assert repaired["executed"] == 2 and not repaired["errors"]


def _rewrite_checkpoint_entry(report: Path, row: str, update: Any) -> None:
    checkpoint = checkpoint_path(report.parent.parent.parent, report)
    lines = [json.loads(line) for line in checkpoint.read_text().splitlines()]
    entry = next(item for item in lines if item["row"] == row)
    update(entry)
    entry["entry_sha256"] = entry_sha256(entry)
    checkpoint.write_text(
        "\n".join(json.dumps(item, sort_keys=True) for item in lines) + "\n"
    )


def test_lagging_manifest_rejects_report_inconsistent_extra(tmp_path: Path) -> None:
    _repo(tmp_path)
    report = _large_fixture(tmp_path, 2, 1)
    assert run_evidence(tmp_path, TARGET, report, all_rows=True)["executed"] == 2
    manifest = manifest_path(tmp_path, report)
    recorded = json.loads(manifest.read_text())
    recorded["rows"] = recorded["rows"][:1]
    manifest.write_text(json.dumps(recorded, indent=2, sort_keys=True) + "\n")
    row = "function:func_80100010"
    omitted = load_checkpoint(tmp_path, report)[row]
    payload_path = tmp_path / omitted["evidence"]
    payload = json.loads(payload_path.read_text())
    payload["items"] = []
    text = json.dumps(payload, indent=2, sort_keys=True) + "\n"
    payload_path.write_text(text)
    _rewrite_checkpoint_entry(
        report,
        row,
        lambda entry: entry.update(
            operations=[], evidence_sha256=hashlib.sha256(text.encode()).hexdigest()
        ),
    )

    repaired = run_evidence(tmp_path, TARGET, report, all_rows=True)
    assert repaired["executed"] == 2 and not repaired["errors"]


def test_lagging_manifest_malformed_extra_recollects_without_crash(
    tmp_path: Path,
) -> None:
    _repo(tmp_path)
    report = _large_fixture(tmp_path, 2, 1)
    assert run_evidence(tmp_path, TARGET, report, all_rows=True)["executed"] == 2
    manifest = manifest_path(tmp_path, report)
    recorded = json.loads(manifest.read_text())
    recorded["rows"] = recorded["rows"][:1]
    manifest.write_text(json.dumps(recorded, indent=2, sort_keys=True) + "\n")
    row = "function:func_80100010"
    omitted = load_checkpoint(tmp_path, report)[row]
    payload_path = tmp_path / omitted["evidence"]
    payload = json.loads(payload_path.read_text())
    payload["items"] = [None]
    text = json.dumps(payload, indent=2, sort_keys=True) + "\n"
    payload_path.write_text(text)
    _rewrite_checkpoint_entry(
        report,
        row,
        lambda entry: entry.update(
            evidence_sha256=hashlib.sha256(text.encode()).hexdigest()
        ),
    )

    repaired = run_evidence(tmp_path, TARGET, report, all_rows=True)
    assert repaired["executed"] == 2 and not repaired["errors"]


@pytest.mark.parametrize("artifact", ["raw", "stderr"])
def test_lagging_manifest_recollects_tampered_extra_semantic_artifact(
    tmp_path: Path, artifact: str
) -> None:
    _repo(tmp_path)
    report = _large_fixture(tmp_path, 2, 1)
    assert run_evidence(tmp_path, TARGET, report, all_rows=True)["executed"] == 2
    manifest = manifest_path(tmp_path, report)
    recorded = json.loads(manifest.read_text())
    recorded["rows"] = recorded["rows"][:1]
    manifest.write_text(json.dumps(recorded, indent=2, sort_keys=True) + "\n")
    row = "function:func_80100010"
    omitted = load_checkpoint(tmp_path, report)[row]
    payload_path = tmp_path / omitted["evidence"]
    payload = json.loads(payload_path.read_text())
    artifact_path = payload_path.parent / f"omitted.{artifact}.raw"
    artifact_path.write_text("original")
    semantic = {
        "command": "bin/rz-project query exe/test -c px",
        "raw_file": artifact_path.relative_to(tmp_path).as_posix(),
        "raw_sha256": hashlib.sha256(b"original").hexdigest(),
        "raw_size": 8,
        "stderr_file": artifact_path.relative_to(tmp_path).as_posix(),
        "stderr_sha256": hashlib.sha256(b"original").hexdigest(),
        "stderr_size": 8,
    }
    payload["semantic"] = [semantic]
    payload["items"] = payload["items"]
    artifact_path.write_text("tampered")
    text = json.dumps(payload, indent=2, sort_keys=True) + "\n"
    payload_path.write_text(text)
    _rewrite_checkpoint_entry(
        report,
        row,
        lambda entry: entry.update(
            operations=[
                *entry["operations"],
                "semantic:rz-project:bin/rz-project query exe/test -c px",
            ],
            evidence_sha256=hashlib.sha256(text.encode()).hexdigest(),
        ),
    )

    repaired = run_evidence(tmp_path, TARGET, report, all_rows=True)
    assert repaired["executed"] == 2 and not repaired["errors"]


def test_lagging_manifest_rejects_duplicate_extra(tmp_path: Path) -> None:
    _repo(tmp_path)
    report = _large_fixture(tmp_path, 2, 1)
    assert run_evidence(tmp_path, TARGET, report, all_rows=True)["executed"] == 2
    manifest = manifest_path(tmp_path, report)
    recorded = json.loads(manifest.read_text())
    recorded["rows"] = recorded["rows"][:1]
    manifest.write_text(json.dumps(recorded, indent=2, sort_keys=True) + "\n")
    checkpoint = checkpoint_path(tmp_path, report)
    lines = checkpoint.read_text().splitlines()
    checkpoint.write_text("\n".join([*lines, lines[-1]]) + "\n")

    repaired = run_evidence(tmp_path, TARGET, report, all_rows=True)
    assert repaired["executed"] == 2 and not repaired["errors"]
    assert checkpoint.with_name("checkpoint.jsonl.corrupt").is_file()


def test_journal_lines_are_self_hashed(tmp_path: Path) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", _callee_work())])
    run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    entry = json.loads(checkpoint_path(tmp_path, report).read_text().splitlines()[0])
    assert entry["entry_sha256"] == entry_sha256(entry)


def test_evidence_run_cli_reports_success_and_resume_noop(tmp_path: Path) -> None:
    from harness.naming.runner import main as run_main

    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", _callee_work())])
    argv = [
        TARGET,
        str(report),
        "--rows",
        "function:func_80100000",
        "--root",
        str(tmp_path),
    ]
    assert run_main(argv) == 0
    # A second run of the same rows is a resume no-op: still successful,
    # with zero newly executed operations.
    assert run_main(argv) == 0


def test_report_semantic_injection_fails_before_artifacts(tmp_path: Path) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    row = _blocked_row("func_80100000", _callee_work())
    row["rungs"]["selected_range"]["next_command"] = (
        f"bin/rz-project query {TARGET} -c 'axt @ 0x80100000\\nwtf owned'"
    )
    report = _report(tmp_path, [row])
    with pytest.raises(ValueError, match="control character|not code-owned"):
        run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert not (tmp_path / "out/reviews/evidence").exists()

    row["rungs"]["selected_range"]["next_command"] = (
        f"bin/asm-diff {TARGET}@0x80100000 --json -o owned"
    )
    report.write_text(
        json.dumps({"schema": "bof3.naming-audit/v3", "target": TARGET, "rows": [row]})
    )
    with pytest.raises(ValueError, match="not code-owned"):
        run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert not (tmp_path / "out/reviews/evidence").exists()


def test_semantic_parser_rizin_and_native_argv(tmp_path: Path) -> None:
    from harness.naming.native import NativeByteOps

    rz = parse_semantic_command("bin/rz-project query exe/test -c 'px 4 @ 0x80100000'")
    assert rz == {
        "kind": "rz-project",
        "target": "exe/test",
        "rizin": "px 4 @ 0x80100000",
    }
    log = tmp_path / "argv.json"
    (tmp_path / "bin").mkdir()
    for name in ("asm-diff", "byte-match"):
        wrapper = tmp_path / "bin" / name
        wrapper.write_text(
            "#!/usr/bin/env python3\nimport json,sys\n"
            f"open({str(log)!r},'w').write(json.dumps(sys.argv[1:]))\n",
            encoding="utf-8",
        )
        wrapper.chmod(0o755)
        args = "--json --detail full" if name == "asm-diff" else "--json"
        operation = parse_semantic_command(f"bin/{name} exe/test@0x80100000 {args}")
        assert operation is not None
        result = NativeByteOps(tmp_path, 2).run(operation)
        assert result.exit == 0
        expected = ["exe/test@0x80100000", "--json"]
        if name == "asm-diff":
            expected.extend(["--detail", "full"])
        assert json.loads(log.read_text()) == expected


def test_partial_baseline_requires_strict_identity_bound_json(
    tmp_path: Path, monkeypatch: Any
) -> None:
    import harness.common.checks as checks
    import harness.domain.registry as registry
    import harness.naming.execution as evidence_run
    from harness.naming.native import SemanticResult

    source = tmp_path / "src.c"
    source.write_text("/* source */\n")
    resolved = SimpleNamespace(
        source=source,
        compiled_symbol="func_80100000",
        target=SimpleNamespace(id=SimpleNamespace(value=TARGET)),
    )
    monkeypatch.setattr(registry, "resolve_function", lambda *_: resolved)
    observed = []
    monkeypatch.setattr(
        checks,
        "validate_partial_evidence",
        lambda root, **kwargs: observed.append((root, kwargs)) or {"valid": True},
    )
    operation = {"kind": "asm-diff", "target": f"{TARGET}@0x80100000"}
    mismatch = SemanticResult("asm-diff", operation["target"], 1, "json", raw="{}")
    assert evidence_run._semantic_succeeded(
        tmp_path, {"partial_used": True}, operation, mismatch
    )
    assert observed[0][1]["selector"] == operation["target"]
    assert observed[0][1]["function"] == "func_80100000"
    assert observed[0][1]["source"] == "src.c"

    def reject(*_args: Any, **_kwargs: Any) -> None:
        raise ValueError("malformed or wrong selector")

    monkeypatch.setattr(checks, "validate_partial_evidence", reject)
    assert not evidence_run._semantic_succeeded(
        tmp_path, {"partial_used": True}, operation, mismatch
    )
    assert not evidence_run._semantic_succeeded(
        tmp_path, {"partial_used": False}, operation, mismatch
    )


def test_rizin_protocol_exact_output_single_session(tmp_path: Path) -> None:
    from harness.naming.semantics import RizinSession

    engine = tmp_path / "fake-rizin"
    engine.write_text(
        "#!/usr/bin/env python3\nimport sys\n"
        "for line in sys.stdin:\n"
        " line=line.strip()\n"
        " if line.startswith('echo '): print(line[5:], flush=True)\n"
        " elif line.startswith('?e BOF3_'):\n"
        "  print('ERROR: core: Error while parsing command: `'+line+'`', file=sys.stderr, flush=True)\n"
        " elif line: print('EXACT:'+line, flush=True)\n",
        encoding="utf-8",
    )
    engine.chmod(0o755)
    _repo(tmp_path)
    session = RizinSession(tmp_path, TARGET, executable=engine)
    session.queue("px 4 @ 0x80100000", TARGET)
    result = session.run_queued(2)[0]
    assert result.output == "EXACT:px 4 @ 0x80100000"
    assert session.process_count == 1
    session.close()
    assert session.process_count == 0


def test_rizin_protocol_reports_command_failure(tmp_path: Path) -> None:
    from harness.naming.semantics import RizinSession

    engine = tmp_path / "fake-rizin"
    engine.write_text(
        "#!/usr/bin/env python3\nimport sys\n"
        "for line in sys.stdin:\n"
        " line=line.strip()\n"
        " if line.startswith('echo '): print(line[5:], flush=True)\n"
        " elif line.startswith('?e BOF3_'):\n"
        "  print('ERROR: core: Error while parsing command: `'+line+'`', file=sys.stderr, flush=True)\n"
        " elif line: print('ERROR: '+line, file=sys.stderr, flush=True)\n",
        encoding="utf-8",
    )
    engine.chmod(0o755)
    _repo(tmp_path)
    session = RizinSession(tmp_path, TARGET, executable=engine)
    session.queue("invalid-command", TARGET)
    result = session.run_queued(2)[0]
    assert result.exit == 1 and "ERROR: invalid-command" in result.output
    session.close()


def test_rizin_stderr_boundary_waits_for_delayed_reader(
    tmp_path: Path, monkeypatch: Any
) -> None:
    from harness.naming.semantics import RizinSession

    engine = tmp_path / "fake-rizin"
    engine.write_text(
        "#!/usr/bin/env python3\nimport sys\n"
        "for line in sys.stdin:\n"
        " line=line.strip()\n"
        " if line.startswith('echo '): print(line[5:], flush=True)\n"
        " elif line.startswith('?e BOF3_'):\n"
        "  print('ERROR: core: Error while parsing command: `'+line+'`', file=sys.stderr, flush=True)\n"
        " elif line:\n"
        "  print('ERROR: invalid-command', file=sys.stderr, flush=True)\n",
        encoding="utf-8",
    )
    engine.chmod(0o755)
    _repo(tmp_path)
    original = RizinSession._drain_stderr

    def delayed(self: Any, stream: Any) -> None:
        time.sleep(0.2)
        original(self, stream)

    monkeypatch.setattr(RizinSession, "_drain_stderr", delayed)
    session = RizinSession(tmp_path, TARGET, executable=engine)
    session.queue("invalid-command", TARGET)
    result = session.run_queued(2)[0]
    session.close()
    assert result.exit == 1
    assert "ERROR: invalid-command" in result.stderr
    assert "STDERR_END" not in result.stderr


def test_rizin_random_exact_framing_preserves_colliding_fragmented_payload(
    tmp_path: Path, monkeypatch: Any
) -> None:
    import harness.naming.semantics as semantic

    token = "a" * 64
    monkeypatch.setattr(semantic.secrets, "token_hex", lambda _size: token)
    engine = tmp_path / "fake-rizin"
    engine.write_text(
        "#!/usr/bin/env python3\nimport sys,time\n"
        "for line in sys.stdin:\n"
        " line=line.strip()\n"
        " if line.startswith('echo BOF3_FRAME_'):\n"
        "  for char in line[5:]+'\\n': print(char,end='',flush=True); time.sleep(.001)\n"
        " elif line.startswith('?e BOF3_FRAME_'):\n"
        "  marker='ERROR: token-substring '+line[3:]+' remains\\n'\n"
        "  sys.stderr.write(marker)\n"
        "  sys.stderr.write('ERROR: core: Error while parsing command: `'+line+'`\\n'); sys.stderr.flush()\n"
        " elif line:\n"
        "  print('BEFORE BOF3_FRAME_'+('a'*64)+'_END AFTER',flush=True)\n",
        encoding="utf-8",
    )
    engine.chmod(0o755)
    _repo(tmp_path)
    session = semantic.RizinSession(tmp_path, TARGET, executable=engine)
    session.queue("payload", TARGET)
    result = session.run_queued(3)[0]
    session.close()
    assert "BEFORE BOF3_FRAME_" in result.raw and "_END AFTER" in result.raw
    assert "ERROR: token-substring" in result.stderr
    assert "Error while parsing command" not in result.stderr


def _wait_for_path(path: Path) -> None:
    deadline = time.monotonic() + 3
    while not path.exists():
        assert time.monotonic() < deadline, f"timed out waiting for {path.name}"
        time.sleep(0.001)


def test_rizin_boundaries_require_complete_lines_without_cross_command_leak(
    tmp_path: Path, monkeypatch: Any
) -> None:
    import harness.naming.semantics as semantic

    monkeypatch.setattr(semantic.secrets, "token_hex", lambda _size: "b" * 64)
    engine = tmp_path / "fake-rizin"
    engine.write_text(
        "#!/usr/bin/env python3\nimport pathlib,sys,threading,time\n"
        "root=pathlib.Path.cwd()\n"
        "def fragment(stream,text,kind):\n"
        " stream.write(text); stream.flush(); (root/(kind+'.ready')).touch()\n"
        " while not (root/(kind+'.release')).exists(): time.sleep(.001)\n"
        " stream.write(' '+kind+'-same-line-tail\\r\\n'+text+'\\n'); stream.flush()\n"
        "ends=boundaries=0\n"
        "for line in sys.stdin:\n"
        " line=line.strip()\n"
        " if line.startswith('echo BOF3_FRAME_'):\n"
        "  if line.endswith('_END'):\n"
        "   ends += 1\n"
        "   if ends == 1:\n"
        "    threading.Thread(target=fragment,args=(sys.stdout,line[5:],'stdout')).start()\n"
        "   else: print(line[5:], flush=True)\n"
        "  else: print(line[5:], flush=True)\n"
        " elif line.startswith('?e BOF3_FRAME_'):\n"
        "  boundaries += 1\n"
        "  boundary='ERROR: core: Error while parsing command: `'+line+'`'\n"
        "  if boundaries == 1:\n"
        "   threading.Thread(target=fragment,args=(sys.stderr,boundary,'stderr')).start()\n"
        "  else: sys.stderr.write(boundary+'\\n'); sys.stderr.flush()\n"
        " elif line: print('PAYLOAD:'+line, flush=True)\n",
        encoding="utf-8",
    )
    engine.chmod(0o755)
    _repo(tmp_path)
    session = semantic.RizinSession(tmp_path, TARGET, executable=engine)
    session.queue("first", TARGET)
    session.queue("second", TARGET)
    with ThreadPoolExecutor(max_workers=1) as pool:
        result = pool.submit(session.run_queued, 5)
        _wait_for_path(tmp_path / "stdout.ready")
        _wait_for_path(tmp_path / "stderr.ready")
        assert not result.done()
        (tmp_path / "stdout.release").touch()
        (tmp_path / "stderr.release").touch()
        first, second = result.result(timeout=6)
    session.close()
    assert "stdout-same-line-tail" in first.raw
    assert "stderr-same-line-tail" in first.stderr
    assert first.exit == 1
    assert "TAIL" not in second.raw and "tail" not in second.stderr
    assert second.exit == 0 and "PAYLOAD:second" in second.raw


def test_pinned_rizin_protocol_distinguishes_success_error_and_old_markers() -> None:
    from harness.naming.semantics import RizinSession

    root = Path(__file__).resolve().parents[3]
    engine = root / "toolchains/rizin/bin/rizin"
    binary = root / "out/binaries/exe/logo.bin"
    if not engine.is_file() or not binary.is_file():
        pytest.skip("pinned Rizin or prepared logo binary unavailable")
    session = RizinSession(root, "exe/logo", executable=engine)
    session.queue("echo BEFORE; echo BOF3_0_END; echo AFTER", "exe/logo")
    session.queue("?e BOF3_0_STDERR_END", "exe/logo")
    success, failure = session.run_queued(5)
    session.close()
    assert success.exit == 0 and all(
        text in success.raw for text in ("BEFORE", "BOF3_0_END", "AFTER")
    )
    assert failure.exit == 1 and "BOF3_0_STDERR_END" in failure.stderr


def test_one_shard_deadline_rejects_report_owned_semantic_arguments(
    tmp_path: Path, monkeypatch: Any
) -> None:
    """Arbitrary report-owned native arguments fail before execution."""
    import harness.naming.execution as evidence_run

    _repo(tmp_path)
    _index(tmp_path)
    row = _blocked_row("func_80100000", _callee_work())
    for index, rung in enumerate(row["rungs"].values()):
        rung["next_command"] = f"bin/asm-diff {TARGET}@0x80100000 --example={index}"
    row["ceiling_next_command"] = f"bin/asm-diff {TARGET}@0x80100000 --example=root"
    report = _report(tmp_path, [row])
    (tmp_path / "bin").mkdir(exist_ok=True)
    wrapper = tmp_path / "bin/asm-diff"
    wrapper.write_text("#!/bin/sh\nsleep .3\necho ok\n", encoding="utf-8")
    wrapper.chmod(0o755)
    monkeypatch.setattr(evidence_run, "SHARD_WALL_CLOCK", 0.5)
    with pytest.raises(ValueError, match="not code-owned"):
        run_evidence(
            tmp_path, TARGET, report, rows=["function:func_80100000"], deadline=2
        )
    assert not (tmp_path / "out/reviews/evidence").exists()


def test_report_native_command_without_partial_profile_fails_before_artifacts(
    tmp_path: Path,
) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    row = _blocked_row("func_80100000", _callee_work())
    row["rungs"]["selected_range"]["next_command"] = (
        f"bin/asm-diff {TARGET}@0x80100000 --json --detail full"
    )
    report = _report(tmp_path, [row])
    with pytest.raises(ValueError, match="not code-owned"):
        run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert not (tmp_path / "out/reviews/evidence").exists()


def test_semantic_receipts_are_unique_and_validated(tmp_path: Path) -> None:
    from harness.domain.receipts import command_records
    from harness.naming.collection import _receipts_for

    _repo(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", [])])
    row = json.loads(report.read_text())["rows"][0]
    semantic = [
        {
            "command": f"bin/{name} {TARGET}@0x80100000",
            "selector": f"{TARGET}@0x80100000",
            "exit": 1,
            "semantic_success": True,
            "output": "validated non-exact evidence",
        }
        for name in ("asm-diff", "byte-match")
    ]
    from harness.naming.collection import evidence_namespace

    records = _receipts_for(
        evidence_namespace(tmp_path, report, TARGET), TARGET, row, [], semantic
    )
    assert len({record["receipt"] for record in records}) == 2
    assert command_records(records, "commands", tmp_path) == records
    assert all(
        json.loads((tmp_path / record["receipt"]).read_text())["status"] == "passed"
        for record in records
    )


def test_partial_runner_commits_strict_exit_one_evidence(
    tmp_path: Path, monkeypatch: Any
) -> None:
    import harness.domain.registry as registry

    _repo(tmp_path)
    _index(tmp_path)
    source = tmp_path / "src/test/func_80100000.c"
    source.parent.mkdir(parents=True, exist_ok=True)
    source.write_text(
        "/* @source 0x80100000 */\n"
        "/* @status partial */\n"
        "/* @match 50.0 */\n"
        "/* @residual test mismatch */\n"
    )
    resolved = SimpleNamespace(
        source=source,
        compiled_symbol="func_80100000",
        target=SimpleNamespace(id=SimpleNamespace(value=TARGET)),
    )
    monkeypatch.setattr(registry, "resolve_function", lambda *_: resolved)

    class Worker:
        def __init__(self, *_args: Any) -> None:
            pass

        def receive(self, _timeout: float) -> dict[str, Any]:
            return {"status": "ready"}

        def request(self, *_args: Any) -> list[Any]:
            return []

        def kill(self) -> None:
            pass

        def close(self) -> None:
            pass

    import harness.naming.session as sessions

    monkeypatch.setattr(sessions, "IndexWorker", Worker)
    common = {
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
    }
    asm = {
        "schema": "harness.asm-diff-one/v2",
        **common,
        "outputs": {
            key: f"out/diff/{key}"
            for key in (
                "directory",
                "summary",
                "diff",
                "original",
                "current",
                "compiler",
                "original_bytes",
                "build_log",
            )
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
    byte = {"schema": "harness.byte-match-one/v1", **common, "outputs": {}}
    (tmp_path / "bin").mkdir(exist_ok=True)
    for name, payload in (("asm-diff", asm), ("byte-match", byte)):
        wrapper = tmp_path / "bin" / name
        wrapper.write_text(
            "#!/usr/bin/env python3\nimport json,sys\n"
            f"print(json.dumps({payload!r}))\nsys.exit(1)\n"
        )
        wrapper.chmod(0o755)
    row = _blocked_row("func_80100000", [])
    row["partial_used"] = True
    commands = [
        f"bin/asm-diff {TARGET}@0x80100000 --json --detail full",
        f"bin/byte-match {TARGET}@0x80100000 --json",
    ]
    for rung, command in zip(row["rungs"].values(), commands + commands[:1]):
        rung["next_command"] = command
    row["ceiling_next_command"] = commands[1]
    report = _report(tmp_path, [row])
    result = run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert result["executed"] == 1 and not result["errors"]
    entry = load_checkpoint(tmp_path, report)["function:func_80100000"]
    payload = json.loads((tmp_path / entry["evidence"]).read_text())
    assert [item["exit"] for item in payload["semantic"]] == [1, 1]
    assert all(item["semantic_success"] for item in payload["semantic"])
    assert command_records(payload["commands"], "commands", tmp_path)
    for item in payload["semantic"]:
        for kind in ("raw", "stderr"):
            text = (tmp_path / item[f"{kind}_file"]).read_text()
            assert hashlib.sha256(text.encode()).hexdigest() == item[f"{kind}_sha256"]
    assert all(
        json.loads((tmp_path / record["receipt"]).read_text())["status"] == "passed"
        for record in payload["commands"]
    )

    asm["function"] = "wrong"
    (tmp_path / "bin/asm-diff").write_text(
        "#!/usr/bin/env python3\nimport json,sys\n"
        f"print(json.dumps({asm!r}))\nsys.exit(1)\n"
    )
    (tmp_path / "bin/asm-diff").chmod(0o755)
    checkpoint_path(tmp_path, report).unlink()
    manifest_path(tmp_path, report).unlink()
    bad = run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert bad["executed"] == 0 and bad["errors"]
    assert load_checkpoint(tmp_path, report) == {}


def test_native_timeout_kills_grandchild_before_late_write(tmp_path: Path) -> None:
    from harness.naming.native import NativeByteOps

    late = tmp_path / "late"
    (tmp_path / "bin").mkdir(exist_ok=True)
    wrapper = tmp_path / "bin/asm-diff"
    wrapper.write_text(
        '#!/bin/sh\n(sleep .4; echo late > "$1") &\nsleep 5\n',
        encoding="utf-8",
    )
    wrapper.chmod(0o755)
    result = NativeByteOps(tmp_path, 1).run(
        {"kind": "asm-diff", "target": str(late), "args": []}, timeout=0.1
    )
    assert result.killed and result.exit == 124
    time.sleep(0.6)
    assert not late.exists()


def test_large_raw_output_is_complete_and_summary_bounded(tmp_path: Path) -> None:
    from harness.naming.native import OUTPUT_BUDGET, _bounded_text

    raw = "x" * (OUTPUT_BUDGET + 4096)
    summary = _bounded_text(raw)
    path = tmp_path / "raw"
    path.write_text(raw)
    assert path.read_text() == raw
    assert len(summary) <= OUTPUT_BUDGET
    assert "truncated" in summary


def test_valid_corrupt_valid_journal_reuses_nothing(tmp_path: Path) -> None:
    from harness.naming.journal import commit_entry, load_journal

    _repo(tmp_path)
    report = _report(tmp_path, [])
    commit_entry(tmp_path, report, TARGET, {"row": "function:func_80100000"})
    checkpoint = checkpoint_path(tmp_path, report)
    first = checkpoint.read_text()
    checkpoint.write_text(first + "corrupt\n" + first.replace("80100000", "80100010"))
    entries, corrupt = load_journal(tmp_path, report, TARGET)
    assert corrupt and entries == {}
    assert checkpoint.with_name("checkpoint.jsonl.corrupt").read_text()


def test_missing_inventory_row_fails_before_evidence(tmp_path: Path) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [])
    try:
        run_evidence(tmp_path, TARGET, report, all_rows=True)
    except RuntimeError as error:
        assert "inventory mismatch" in str(error)
    else:
        raise AssertionError("incomplete canonical inventory accepted")
    assert not list((tmp_path / "out/reviews/evidence").rglob("*.json"))


def test_malformed_schema_and_raw_names_fail_before_evidence(tmp_path: Path) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    for schema, name in (
        ("wrong", "func_80100000"),
        ("bof3.naming-audit/v3", "func_BAD_80100000"),
    ):
        report = _report(tmp_path, [_blocked_row("func_80100000", [])])
        payload = json.loads(report.read_text())
        payload["schema"] = schema
        payload["rows"][0]["name"] = name
        report.write_text(json.dumps(payload))
        try:
            run_evidence(tmp_path, TARGET, report)
        except ValueError:
            pass
        else:
            raise AssertionError("malformed report accepted")


def test_checkpoint_and_manifest_are_digest_bound(tmp_path: Path) -> None:
    _repo(tmp_path)
    _index(tmp_path)
    report = _report(tmp_path, [_blocked_row("func_80100000", _callee_work())])
    run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    entry = load_checkpoint(tmp_path, report)["function:func_80100000"]
    evidence_text = (tmp_path / entry["evidence"]).read_bytes()
    assert hashlib.sha256(evidence_text).hexdigest() == entry["evidence_sha256"]
    assert manifest_path(tmp_path, report).is_file()
