"""Own the serial naming index subprocess client."""

from __future__ import annotations

import json
import os
import re
import select
import subprocess
import sys
import time
from collections.abc import Sequence
from pathlib import Path
from typing import Any

from harness.common.process import OwnedProcess, owned_popen
from harness.common.deadlines import resolve_deadline
from harness.naming.namespace import selected_evidence_root

OUTPUT_BUDGET = 65536
REPORT_SCHEMA = "bof3.naming-audit/v3"
FUNCTION_RE = re.compile(r"func_[0-9A-F]{8}")
DATA_RE = re.compile(r"D_[0-9A-F]{8}(?:_[A-Za-z0-9_]+)?")
MAX_SHARDS = 10
MAX_DEADLINE = 600


class IndexWorker:
    """Whole-run JSON-lines child owning the sole SQLite connection/snapshot."""

    def __init__(
        self,
        root: Path,
        target: str,
        report: dict[str, Any],
        timeout: float = MAX_DEADLINE,
        *,
        work_deadline: float | None = None,
    ) -> None:
        self.work_deadline = resolve_deadline(work_deadline)
        environment = dict(os.environ)
        environment.pop("BOF3_NAMING_EVIDENCE_ROOT", None)
        environment["PYTHONPATH"] = str(Path(__file__).resolve().parents[2])
        evidence_root = selected_evidence_root()
        if evidence_root is not None:
            environment["BOF3_NAMING_EVIDENCE_ROOT"] = str(evidence_root)
        self.process: OwnedProcess = owned_popen(
            [sys.executable, "-m", "harness.naming.worker"],
            cwd=root,
            env=environment,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1,
            **(
                {"deadline": self.work_deadline}
                if self.work_deadline is not None
                else {}
            ),
        )
        self._send({"root": str(root), "target": target, "report": report}, timeout)

    def resolve_cutoff(self, timeout: float, *, cleanup: bool = False) -> float:
        cutoff = time.monotonic() + timeout
        inherited = getattr(self, "work_deadline", None)
        return (
            min(cutoff, inherited) if inherited is not None and not cleanup else cutoff
        )

    def _send(
        self,
        value: dict[str, Any],
        timeout: float = MAX_DEADLINE,
        *,
        cleanup: bool = False,
    ) -> None:
        if self.process.stdin is None:
            raise RuntimeError("index worker stdin closed")
        deadline = self.resolve_cutoff(timeout, cleanup=cleanup)
        try:
            pending = memoryview((json.dumps(value) + "\n").encode())
            descriptor = self.process.stdin.fileno()
            os.set_blocking(descriptor, False)
            while pending:
                remaining = deadline - time.monotonic()
                if (
                    remaining <= 0
                    or not select.select([], [descriptor], [], remaining)[1]
                ):
                    raise TimeoutError("index worker deadline exceeded")
                try:
                    written = os.write(descriptor, pending)
                except BlockingIOError:
                    continue
                pending = pending[written:]
        except BaseException:
            self.kill()
            raise

    def receive(self, timeout: float) -> dict[str, Any]:
        if self.process.stdout is None:
            raise RuntimeError("index worker stdout closed")
        deadline = self.resolve_cutoff(timeout)
        stdout = self.process.stdout.fileno()
        stderr = (
            self.process.stderr.fileno() if self.process.stderr is not None else None
        )
        descriptors = [stdout] if stderr is None else [stdout, stderr]
        pending = getattr(self, "_stdout_buffer", b"")
        detail = getattr(self, "_stderr_buffer", b"")
        try:
            while b"\n" not in pending:
                remaining = deadline - time.monotonic()
                ready = select.select(descriptors, [], [], max(0, remaining))[0]
                if remaining <= 0 or not ready:
                    raise TimeoutError("index worker deadline exceeded")
                for descriptor in sorted(ready, key=lambda item: item == stdout):
                    chunk = os.read(descriptor, 65536)
                    if descriptor == stderr:
                        detail = (detail + chunk)[-OUTPUT_BUDGET:]
                        if not chunk:
                            descriptors.remove(descriptor)
                    elif chunk:
                        pending += chunk
                    else:
                        raise RuntimeError(
                            f"index worker failed: {detail.decode(errors='replace')}"
                        )
            if time.monotonic() >= deadline:
                raise TimeoutError("index worker deadline exceeded")
            line, self._stdout_buffer = pending.split(b"\n", 1)
            self._stderr_buffer = detail
            return json.loads(line)
        except BaseException:
            self.kill()
            raise

    def request(
        self, request_id: int, row: dict[str, Any], timeout: float
    ) -> list[dict[str, Any]]:
        deadline = self.resolve_cutoff(timeout)
        self._send({"id": request_id, "row": row}, timeout)
        response = self.receive(max(0, deadline - time.monotonic()))
        if response.get("id") != request_id or not isinstance(
            response.get("operations"), list
        ):
            raise RuntimeError("malformed index worker response")
        return response["operations"]

    def kill(self) -> None:
        if isinstance(self.process, OwnedProcess):
            self.process.terminate_tree(timeout=2)
        elif self.process.poll() is None:
            self.process.kill()
            self.process.wait(timeout=2)
        for stream in (self.process.stdin, self.process.stdout, self.process.stderr):
            if stream is not None:
                try:
                    stream.close()
                except OSError:
                    pass

    def close(self) -> None:
        try:
            if self.process.poll() is None:
                self._send({"op": "close"}, 2, cleanup=True)
                self.process.wait(timeout=2)
        except (BrokenPipeError, subprocess.TimeoutExpired, TimeoutError):
            pass
        finally:
            self.kill()


def _row_key(row: dict[str, Any]) -> str:
    return f"{row.get('kind')}:{row.get('name')}"


def _valid_selector(value: str, report_rows: Sequence[dict[str, Any]]) -> bool:
    return value in {_row_key(row) for row in report_rows}


def _validate_report_shape(
    payload: dict[str, Any], target: str
) -> list[dict[str, Any]]:
    """Pure trust-boundary checks; canonical derived validation runs in worker."""
    if not isinstance(payload, dict) or payload.get("schema") != REPORT_SCHEMA:
        raise ValueError(f"report schema must be {REPORT_SCHEMA}")
    if payload.get("target") != target:
        raise ValueError(f"report target does not match {target}")
    rows = payload.get("rows")
    if not isinstance(rows, list):
        raise ValueError("report rows must be an array")
    seen: set[str] = set()
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError("report row must be an object")
        kind, name = row.get("kind"), row.get("name")
        pattern = (
            FUNCTION_RE if kind == "function" else DATA_RE if kind == "data" else None
        )
        if (
            pattern is None
            or not isinstance(name, str)
            or pattern.fullmatch(name) is None
        ):
            raise ValueError(f"invalid raw report row: {kind}:{name}")
        if not isinstance(row.get("rungs"), dict) or not isinstance(
            row.get("required_work"), list
        ):
            raise ValueError(f"malformed report row: {kind}:{name}")
        key = _row_key(row)
        if key in seen:
            raise ValueError(f"duplicate report row: {key}")
        seen.add(key)
    return rows


def _validate_run_arguments(
    rows: Sequence[str] | None,
    all_rows: bool,
    shard: int,
    concurrency: int,
    deadline: int,
    rows_budget: int,
    report_rows: Sequence[dict[str, Any]],
) -> None:
    if all_rows and rows:
        raise ValueError("--rows cannot be combined with --all-rows")
    if concurrency != 1:
        raise ValueError("indexed concurrency must be 1 (one serial index worker)")
    if not 1 <= deadline <= MAX_DEADLINE:
        raise ValueError(f"deadline must be within 1-{MAX_DEADLINE} seconds")
    if shard < 1 or (not all_rows and shard > MAX_SHARDS):
        raise ValueError(f"shard must be within 1-{MAX_SHARDS}")
    if rows is not None:
        if len(rows) > MAX_SHARDS or len(set(rows)) != len(rows):
            raise ValueError("explicit selection must be unique and at most 10 rows")
        for selector in rows:
            if not _valid_selector(selector, report_rows):
                raise ValueError(f"invalid or unknown row selector: {selector}")
    if rows_budget < 1 or (all_rows and len(report_rows) > rows_budget):
        raise ValueError("--rows-budget must explicitly cover --all-rows")
