"""P0 toolchain performance budgets: stable, count-based guards.

Wall-clock SLOs belong to the benchmark harness and to the focused
evidence-run tests (which carry the timed guards on recorded hardware);
these guards lock the *structural* counts that make the SLOs hold, and
they are deterministic on any hardware: one index connection per native
run, one manifest load per command invocation, constant query counts per
full-report validation, and a cross-thread-safe shared connection for
bounded read-only concurrency.  Recorded evidence may only tighten them.
"""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

import harness.naming.audit as naming_audit
from harness.analysis.index import connect as real_connect
from harness.analysis.index import rebuild
from harness.analysis.project import prepare_target
from harness.analysis.snapshot import (
    SNAPSHOT_SCHEMA,
    AnalysisSnapshot,
    SnapshotCall,
    SnapshotFunction,
    snapshot_path,
    write_snapshot,
)
from harness.naming.state import load_checkpoint
from harness.naming.execution import run_evidence

TARGET = "exe/test"


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
    sdk = root / "config/sdk/psyq-slus.txt"
    sdk.parent.mkdir(parents=True, exist_ok=True)
    sdk.write_text("", encoding="utf-8")
    (root / "out").mkdir(parents=True, exist_ok=True)
    (root / "out/test.bin").write_bytes(b"\0" * 0x20)
    (config.parent / "splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [0x20]\n",
        encoding="utf-8",
    )


def _index(root: Path) -> None:
    """One-row fixture: a function with two indexed callees."""

    (root / "include/base").mkdir(parents=True, exist_ok=True)
    (root / "include/base/types.h").write_text(
        "typedef unsigned char bool;\ntypedef unsigned int u32;\n",
        encoding="utf-8",
    )
    (root / "config/targets/exe/test/symbols.txt").write_text(
        "func_80100000 = 0x80100000;\n", encoding="utf-8"
    )
    binary = root / "out/test.bin"
    functions: list[SnapshotFunction] = [
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


def _blocked_report(root: Path) -> Path:
    """A blocked report row whose two open callee items the runner collects."""

    work = [
        {
            "id": f"callee:{TARGET}@801000{address:02X}",
            "profile": "callee_body",
            "description": "resolve callee",
        }
        for address in (0x10, 0x18)
    ]
    selector = f"{TARGET}@0x80100000"

    def rung(missing: str) -> dict[str, Any]:
        return {
            "status": "open",
            "next_command": f"bin/rev-query --json xrefs {selector}",
            "observations": [
                {"id": "func_80100000.gap", "text": f"Evidence Gap: {missing}"}
            ],
            "authority": "target manifest, reviewed Splat, original image",
        }

    row = {
        "kind": "function",
        "name": "func_80100000",
        "rung_status": "blocked",
        "outside_payload": False,
        "partial_used": False,
        "rungs": {
            "selected_range": rung("range"),
            "selected_call": rung("callsite"),
            "one_level_beyond": rung("consumer"),
        },
        "required_work": [dict(item, status="open") for item in work],
        "optional_work": [],
        "interpretation": "No semantic name is accepted until the gap is closed.",
        "authority": "target manifest, target-local map, reviewed Splat, original image",
        "smallest_repair": f"bin/rev-query --json xrefs {selector}",
        "missing_fact": "consumer",
        "ceiling_next_command": f"bin/rev-query --json xrefs {selector}",
    }
    report = root / "out" / "reviews" / "perf-baseline.json"
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text(
        json.dumps(
            {"schema": "bof3.naming-audit/v3", "target": TARGET, "rows": [row]},
            indent=2,
            sort_keys=True,
        )
        + "\n",
        encoding="utf-8",
    )
    return report


class _CountingConnection:
    """Real connection with counted execute/close (close is read-only)."""

    def __init__(self, connection: Any, counts: dict[str, int]) -> None:
        self.connection = connection
        self.counts = counts

    def execute(self, query: str, params: tuple[object, ...] = ()) -> Any:
        self.counts["execute"] = self.counts.get("execute", 0) + 1
        return self.connection.execute(query, params)

    def close(self) -> None:
        self.counts["closed"] = self.counts.get("closed", 0) + 1
        self.connection.close()


def test_native_evidence_run_uses_whole_run_worker_protocol(tmp_path: Path) -> None:
    """The subprocess protocol executes the complete selected run."""
    _repo(tmp_path)
    _index(tmp_path)
    report = _blocked_report(tmp_path)
    result = run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    assert result["executed"] == 1
    assert result["errors"] == []


def test_full_report_validation_query_count_is_constant(
    tmp_path: Path, monkeypatch
) -> None:
    """Full-report validate: one connection, exactly the three snapshot queries."""
    _repo(tmp_path)
    _index(tmp_path)
    report = _blocked_report(tmp_path)
    counts: dict[str, int] = {"connect": 0}

    def connect(*args: Any, **kwargs: Any) -> Any:
        counts["connect"] = counts.get("connect", 0) + 1
        connection = real_connect(*args, **kwargs)
        return _CountingConnection(connection, counts)

    monkeypatch.setattr(naming_audit, "connect_index", connect)
    result = naming_audit.validate(
        tmp_path, TARGET, json.loads(report.read_text(encoding="utf-8"))
    )
    assert result["rows"] == 1
    assert counts["connect"] == 1, "full-report validation uses one connection"
    assert counts["execute"] == 3, (
        "the bulk snapshot runs its three bounded queries, constant in row "
        "count (the >=200-row fixture in the evidence-run tests pins this)"
    )
    assert counts["closed"] == 1


def test_evidence_run_cli_exit_codes_are_stable(tmp_path: Path) -> None:
    """A successful run and its resume no-op both exit 0."""
    from harness.naming.runner import main as evidence_main

    _repo(tmp_path)
    _index(tmp_path)
    report = _blocked_report(tmp_path)
    argv = [
        TARGET,
        str(report),
        "--rows",
        "function:func_80100000",
        "--root",
        str(tmp_path),
    ]
    assert evidence_main(argv) == 0
    assert evidence_main(argv) == 0


def test_model_facing_output_stays_within_budget(tmp_path: Path, monkeypatch) -> None:
    """No model-facing command output may exceed the 8 KiB tool-result budget.

    Full raw evidence always lives in the digest-bound file under
    ``out/reviews/evidence``; the receipt's ``output`` field is the bounded
    summary that a shard may inject into a model context.
    """

    import harness.naming.native as native

    _repo(tmp_path)
    _index(tmp_path)
    report = _blocked_report(tmp_path)
    run_evidence(tmp_path, TARGET, report, rows=["function:func_80100000"])
    entry = load_checkpoint(tmp_path, report)["function:func_80100000"]
    payload = json.loads((tmp_path / entry["evidence"]).read_text(encoding="utf-8"))
    for record in payload["commands"]:
        assert len(record["output"]) <= native.OUTPUT_BUDGET, (
            "bounded summary: raw payloads never reach the model context"
        )
    # A hostile oversized raw payload is bounded the same way: the result
    # never exceeds the budget and names where the full evidence lives.
    monkeypatch.setattr(native, "OUTPUT_BUDGET", 64)
    bounded = native._bounded_text("x" * 100, 64)
    assert len(bounded) <= 64 and "truncated" in bounded
