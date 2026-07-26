"""Tests for the target-scoped analysis sequence command."""

from __future__ import annotations

from pathlib import Path
from unittest.mock import patch

from harness.commands.analysis_sequence import build_parser, main


TARGET = "emi/test/archive/00"


def test_stale_snapshot_stops_before_indexing(capsys, tmp_path: Path) -> None:
    with (
        patch(
            "harness.commands.analysis_sequence.status",
            return_value={"fresh": False},
        ),
        patch("harness.commands.analysis_sequence.rebuild") as rebuild,
        patch("harness.commands.rev_query.run_query") as run_query,
    ):
        assert main(["--root", str(tmp_path), TARGET, "--ranking", "quick-wins"]) == 1

    assert not rebuild.called
    assert not run_query.called
    assert f"stale snapshot for {TARGET}: stage=snapshot" in capsys.readouterr().err


def test_fresh_snapshot_rebuilds_index_then_runs_query(tmp_path: Path) -> None:
    index = tmp_path / "out/index/reverse.sqlite"
    with (
        patch(
            "harness.commands.analysis_sequence.status",
            return_value={"fresh": True},
        ),
        patch("harness.commands.analysis_sequence.rebuild", return_value=index) as rebuild,
        patch("harness.commands.rev_query.run_query", return_value=0) as run_query,
    ):
        assert main(["--root", str(tmp_path), TARGET, "--ranking", "quick-wins"]) == 0

    rebuild.assert_called_once_with(tmp_path)
    assert run_query.call_count == 1


def test_ranking_is_required() -> None:
    try:
        build_parser().parse_args([TARGET])
    except SystemExit as error:
        assert error.code == 2
    else:
        raise AssertionError("--ranking must be required")
