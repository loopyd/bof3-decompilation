"""Tests for bof3-lift-loop/scripts/loop-status.py — fail-closed by default.

Mock subprocess.run at the wire, inject _targets_override / _argv to main().
"""

from __future__ import annotations

import importlib.util
import json
import subprocess
import sys
from pathlib import Path
from typing import Any, Callable
from unittest.mock import patch


SCRIPT = (
    Path(__file__).resolve().parents[3]
    / ".pi"
    / "skills"
    / "bof3-lift-loop"
    / "scripts"
    / "loop-status.py"
)


def _load_module() -> Any:
    sys.modules.pop("loop_status", None)
    spec = importlib.util.spec_from_file_location("loop_status", SCRIPT)
    assert spec is not None, f"cannot find spec for {SCRIPT}"
    assert spec.loader is not None, f"no loader for {SCRIPT}"
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def _completed(
    args: tuple[str, ...], rc: int = 0, stdout: str = "", stderr: str = ""
) -> subprocess.CompletedProcess:
    return subprocess.CompletedProcess(
        args=list(args), returncode=rc, stdout=stdout, stderr=stderr
    )


def _default_command(args: tuple[str, ...]) -> subprocess.CompletedProcess:
    if args[:2] == ("git", "status"):
        return _completed(args, stdout="")
    if args[:3] == ("git", "diff", "--cached"):
        return _completed(args, stdout="")
    if args[:2] == ("bin/rz-project", "status"):
        return _completed(args, stdout=json.dumps({"fresh": True}))
    if args[0] == "bin/rev-query":
        return _completed(args, stdout=json.dumps({"items": []}))
    if args[:2] == ("bin/rz-project", "analyze"):
        return _completed(args)
    if args[0] == "bin/index":
        return _completed(args)
    return _completed(args)


def _run_report(
    command_fn: Callable | None = None,
    targets_list: list[str] | None = None,
    argv: list[str] | None = None,
) -> dict[str, Any]:
    """Run main() with mocks and return parsed JSON report."""
    mod = _load_module()
    targets_list = targets_list or ["SLUS_004.22"]
    fn = command_fn or _default_command

    def _wire(args, **kw):
        return fn(tuple(args))

    import io
    from contextlib import redirect_stdout

    buf = io.StringIO()

    with patch("subprocess.run") as mock_run:
        mock_run.side_effect = _wire
        try:
            with redirect_stdout(buf):
                mod.main(
                    _targets_override=targets_list,
                    _argv=argv if argv is not None else [],
                )
        except SystemExit:
            pass

    raw = buf.getvalue()
    return json.loads(raw) if raw.strip() else {}


# -- tests ----------------------------------------------------------------


def test_journal_reports_malformed_row(tmp_path: Path) -> None:
    path = tmp_path / "results.tsv"
    path.write_text(
        "function\tstatus\tcommit\tnotes\n"
        "selection\\tblocked\\t\\tstale evidence\n"
        "SLUS_004.22@0x80010000\texact\tabc123\tok\n"
    )

    records = _load_module().journal(path)

    assert records == [
        {"error": f"invalid journal row 2: {path}"},
        {
            "function": "SLUS_004.22@0x80010000",
            "status": "exact",
            "commit": "abc123",
            "notes": "ok",
        },
    ]


def test_default_stale_fail_closed() -> None:
    """Default (no --recover) with stale snapshots → fail-closed, no rev-query."""
    rev_query_called = False

    def _fn(args):
        nonlocal rev_query_called
        if args[0] == "bin/rev-query":
            rev_query_called = True
            raise AssertionError("rev-query must not run when snapshots stale")
        if args[:2] == ("bin/rz-project", "status") and "SLUS_004.22" in args:
            return _completed(args, stdout=json.dumps({"fresh": False}))
        return _default_command(args)

    report = _run_report(command_fn=_fn)
    assert report["suppressed_candidates"] is not None
    assert report["suppressed_candidates"]["reason"] == "stale_snapshot"
    assert report["suppressed_candidates"]["stale_targets"] == ["SLUS_004.22"]
    assert report["candidates"]["command"] == ["(skipped)"]
    assert "inspect stale snapshot" in report["next_action"]
    assert not rev_query_called


def test_default_fresh_passes() -> None:
    """Default with all fresh snapshots → rev-query runs, candidates available."""
    calls: list[tuple[str, ...]] = []

    def _fn(args):
        calls.append(args)
        return _default_command(args)

    report = _run_report(
        command_fn=_fn, argv=["--selection", "hotspots", "--limit", "1"]
    )
    assert report["suppressed_candidates"] is None
    assert report["candidates"]["command"] != ["(skipped)"]
    assert "select one candidate" in report["next_action"]
    assert (
        "bin/rev-query",
        "hotspots",
        "--unlifted",
        "--detail",
        "minimal",
        "--limit",
        "1",
        "--json",
    ) in calls


def test_recovery_stale_succeeds() -> None:
    """--recover with stale targets → recovery runs, rechecks, then rev-query."""
    status_call_count = [0]
    rev_query_called = False

    def _fn(args):
        nonlocal rev_query_called
        if args[0] == "bin/rev-query":
            rev_query_called = True
            return _completed(args, stdout=json.dumps({"items": []}))
        if args[:2] == ("bin/rz-project", "status") and "SLUS_004.22" in args:
            status_call_count[0] += 1
            if status_call_count[0] >= 2:
                return _completed(args, stdout=json.dumps({"fresh": True}))
            return _completed(args, stdout=json.dumps({"fresh": False}))
        return _default_command(args)

    report = _run_report(command_fn=_fn, argv=["--recover"])
    assert report["recovery"] is not None
    assert len(report["recovery"]["analyses"]) == 1
    assert rev_query_called, "rev-query should run after successful recovery"
    assert "select one candidate" in report["next_action"]


def test_recovery_still_stale_skips_index_and_ranking() -> None:
    """A zero-exit analysis still needs a fresh snapshot before indexing."""
    commands: list[tuple[str, ...]] = []
    status_calls = 0

    def _fn(args):
        nonlocal status_calls
        commands.append(args)
        if args[:2] == ("bin/rz-project", "status"):
            status_calls += 1
            return _completed(args, stdout=json.dumps({"fresh": False}))
        if args[:2] == ("bin/rz-project", "analyze"):
            return _completed(args)
        if args[0] in {"bin/index", "bin/rev-query"}:
            raise AssertionError(f"must not run before fresh recheck: {args}")
        return _default_command(args)

    report = _run_report(command_fn=_fn, argv=["--recover"])

    assert status_calls == 2
    assert report["suppressed_candidates"]["reason"] == "recovery_incomplete"
    assert report["recovery"]["index_rebuild"] is None
    assert not any(args[0] == "bin/index" for args in commands)


def test_recovery_analysis_failure_skips_index_and_ranking() -> None:
    """Failed analysis fails closed before index rebuild or candidate query."""
    commands: list[tuple[str, ...]] = []

    def _fn(args):
        commands.append(args)
        if args[:2] == ("bin/rz-project", "status"):
            return _completed(args, stdout=json.dumps({"fresh": False}))
        if args[:2] == ("bin/rz-project", "analyze"):
            return _completed(args, rc=1, stderr="analysis error")
        if args[0] in {"bin/index", "bin/rev-query"}:
            raise AssertionError(f"must not run after analysis failure: {args}")
        return _default_command(args)

    report = _run_report(command_fn=_fn, argv=["--recover"])

    assert report["suppressed_candidates"]["reason"] == "recovery_incomplete"
    assert report["recovery"]["index_rebuild"] is None
    assert not any(args[0] == "bin/index" for args in commands)


def test_fresh_index_failure_suppresses_ranking() -> None:
    """A failed index readiness check must never be followed by ranking."""

    def _fn(args):
        if args[0] == "bin/rev-query" and args[-1] == "status":
            return _completed(args, rc=2, stderr="reverse index stale")
        if args[0] == "bin/rev-query":
            raise AssertionError("ranking must not run after index failure")
        return _default_command(args)

    report = _run_report(command_fn=_fn)

    assert report["suppressed_candidates"]["reason"] == "stale_or_invalid_index"
    assert report["candidates"]["command"] == ["(skipped)"]


def test_recover_repairs_stale_index_without_stale_snapshots() -> None:
    """--recover can rebuild a stale index even when snapshots are fresh."""
    index_checks = 0
    calls: list[tuple[str, ...]] = []

    def _fn(args):
        nonlocal index_checks
        calls.append(args)
        if args[0] == "bin/rev-query" and args[-1] == "status":
            index_checks += 1
            return _completed(
                args,
                rc=2 if index_checks == 1 else 0,
                stdout="" if index_checks == 1 else json.dumps({"items": []}),
            )
        if args[0] == "bin/rev-query":
            return _completed(args, stdout=json.dumps({"items": []}))
        return _default_command(args)

    report = _run_report(command_fn=_fn, argv=["--recover"])

    assert report["recovery"] is not None
    assert report["recovery"]["analyses"] == []
    assert any(args[0] == "bin/index" for args in calls)
    assert report["suppressed_candidates"] is None
    assert report["candidates"]["command"] != ["(skipped)"]


def test_recovery_fails_after_repair() -> None:
    """--recover but target still stale after repair → suppressed recovery_incomplete."""
    rev_query_called = False

    def _fn(args):
        nonlocal rev_query_called
        if args[0] == "bin/rev-query":
            rev_query_called = True
            raise AssertionError(
                "rev-query must not run when still stale after recovery"
            )
        if args[:2] == ("bin/rz-project", "status") and "SLUS_004.22" in args:
            return _completed(args, stdout=json.dumps({"fresh": False}))
        if args[:2] == ("bin/rz-project", "analyze"):
            return _completed(args, rc=1, stderr="analysis error")
        return _default_command(args)

    report = _run_report(command_fn=_fn, argv=["--recover"])
    assert report["suppressed_candidates"] is not None
    assert report["suppressed_candidates"]["reason"] == "recovery_incomplete"
    assert (
        "analysis or index recovery failed" in report["suppressed_candidates"]["hint"]
    )
    assert not rev_query_called


def test_fresh_snapshots_with_dirty_worktree() -> None:
    """Fresh snapshots but dirty worktree → still runs rev-query, next_action says clean."""

    def _fn(args):
        if args[:2] == ("git", "status"):
            return _completed(args, stdout=" M src/foo.c\n")
        return _default_command(args)

    report = _run_report(command_fn=_fn)
    assert report["suppressed_candidates"] is None
    assert "clean or explicitly scope the worktree" in report["next_action"]
