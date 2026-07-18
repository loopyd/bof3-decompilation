from __future__ import annotations

import sqlite3

from harness.commands.rev_query import (
    _dominates,
    _enrich_graph,
    _function_metrics,
    _sccs,
    build_parser,
)
from harness.reverse_index import _schema


def test_sccs_are_deterministic_and_collapse_recursion() -> None:
    edges = {"a": {"b"}, "b": {"a", "c"}, "c": set()}

    assert _sccs(["c", "b", "a"], edges) == [["a", "b"], ["c"]]


def test_pareto_dominance_uses_only_visible_complete_metrics() -> None:
    better = {
        "unique_callers": 3,
        "duplicate_leverage": 2,
        "instruction_count": 10,
        "cyclomatic_complexity": 2,
        "unresolved_calls": 0,
        "metric_missing": 0,
    }
    worse = {
        "unique_callers": 2,
        "duplicate_leverage": 2,
        "instruction_count": 12,
        "cyclomatic_complexity": 2,
        "unresolved_calls": 0,
        "metric_missing": 0,
    }

    assert _dominates(better, worse)
    assert not _dominates({**better, "metric_missing": 1}, worse)


def test_rank_options_work_after_the_subcommand() -> None:
    args = build_parser().parse_args(
        [
            "quick-wins",
            "--target",
            "emi/test/archive/00",
            "--unlifted",
            "--json",
            "--limit",
            "0",
        ]
    )

    assert args.target == "emi/test/archive/00"
    assert args.unlifted
    assert args.json
    assert args.limit == 0


def test_metrics_distinguish_recursive_leaf_with_unresolved_evidence() -> None:
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    _schema(connection)
    connection.execute(
        "INSERT INTO targets VALUES ('t', 'b', 'h', 0, 'rizin', 'v', 's', 'sh')"
    )
    function = (
        "t@00000001",
        "t",
        1,
        8,
        "a",
        "a" * 64,
        1,
        0,
        None,
        2,
        1,
        0,
        1,
        0,
        0,
        0,
        0,
        None,
    )
    connection.execute(
        "INSERT INTO functions VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        function,
    )
    second = ("t@00000002", "t", 2, 8, "b", "b" * 64, *function[6:])
    connection.execute(
        "INSERT INTO functions VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
        second,
    )
    connection.execute("INSERT INTO calls VALUES ('t@00000001', 't@00000002', 1)")
    connection.execute("INSERT INTO calls VALUES ('t@00000002', 't@00000001', 2)")
    connection.execute(
        "INSERT INTO unresolved_calls VALUES ('t@00000001', 99, 3, 'unknown')"
    )

    metrics = _function_metrics(connection, "t")
    _enrich_graph(connection, metrics)

    assert {row["scc_id"] for row in metrics} == {"t@00000001"}
    assert {row["leaf_status"] for row in metrics} == {"unresolved_edge"}
