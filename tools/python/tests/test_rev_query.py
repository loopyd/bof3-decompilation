from __future__ import annotations

import json
import sqlite3

from harness.commands._rev_query_graph import (
    _dominates,
    _enrich_graph,
    _function_metrics,
    _sccs,
)
from harness.commands._rev_query_priority import _candidate_exclusion, _priority_rows
from harness.commands.rev_query import _project_rows, build_parser, main
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


def test_rank_detail_projects_context_fields() -> None:
    row = {
        "id": "t@00000001",
        "instruction_count": 8,
        "cyclomatic_complexity": 1,
        "unique_callers": 2,
        "duplicate_leverage": 3,
        "leaf_status": "analyzer_no_edge",
        "lifted": False,
        "exact_sha256": "a" * 64,
    }

    assert _project_rows([row], command="quick-wins", detail="minimal") == [
        {
            "id": "t@00000001",
            "instruction_count": 8,
            "cyclomatic_complexity": 1,
            "unique_callers": 2,
            "duplicate_leverage": 3,
            "leaf_status": "analyzer_no_edge",
            "lifted": False,
        }
    ]
    assert _project_rows([row], command="quick-wins", detail="full") == [row]


def test_candidate_exclusions_are_reported_without_ranking(tmp_path) -> None:
    target = "exe/t"
    config = tmp_path / "config" / "targets" / "exe" / "t"
    config.mkdir(parents=True)
    (config / "target.toml").write_text(
        'schema = "harness.target/v2"\n'
        'id = "exe/t"\nkind = "executable"\n'
        'source_dir = "src/exe/t"\n'
        'binary = "out/binaries/exe/t.bin"\n'
        'splat = "config/targets/exe/t/splat.yaml"\n'
        "load_address = 0x80100000\n",
        encoding="utf-8",
    )
    (config / "splat.yaml").write_text(
        "segments:\n"
        "  - [0, c, func_80100000]\n"
        "  - [8, c, func_80100008]\n"
        "  - [16, c, func_80100010]\n"
        "  - [24, c, func_80100018]\n"
        "  - [32]\n",
        encoding="utf-8",
    )
    binary = tmp_path / "out/binaries/exe/t.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(
        b"\x08\x00\xe0\x03\x00\x00\x00\x00"  # SDK body
        + b"\x00\x00\x10\x80\x04\x00\x10\x80"  # in-image pointers
        + b"DATA\x00TXT"  # printable data mislabeled as code
        + b"\x08\x00\xe0\x03\x00\x00\x00\x00"  # canonical code
    )
    sdk = tmp_path / "config/sdk"
    sdk.mkdir(parents=True)
    (sdk / "psyq-slus.txt").write_text("SdkFn = 0x80100000;\n", encoding="utf-8")

    def row(address: int) -> dict[str, object]:
        return {
            "id": f"{target}@{address:08X}",
            "target": target,
            "address": address,
            "size": 8,
            "instruction_count": 2,
            "basic_blocks": 1,
            "cfg_edges": 0,
            "cyclomatic_complexity": 1,
            "loops": 0,
            "stack_frame": 0,
            "local_count": 0,
            "argument_count": 0,
            "trivial_kind": None,
            "caller_callsites": 0,
            "unique_callers": 0,
            "callee_callsites": 0,
            "unique_callees": 0,
            "unresolved_calls": 0,
            "reviewed": True,
            "lifted": False,
            "duplicate_members": 1,
            "unlifted_duplicate_members": 1,
            "duplicate_targets": 1,
            "exact_sha256": f"{address:064x}",
        }

    candidates = [row(0x80100000 + offset) for offset in (0, 8, 16, 24)]
    assert _candidate_exclusion(tmp_path, candidates[0]) == "shared_sdk_symbol"
    assert _candidate_exclusion(tmp_path, candidates[1]) == "in_image_pointer_table"
    assert _candidate_exclusion(tmp_path, candidates[2]) == "ascii_or_nul_data"
    assert _candidate_exclusion(tmp_path, candidates[3]) is None

    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    _schema(connection)
    args = build_parser().parse_args(["quick-wins", "--exclusions", "--limit", "0"])
    args.target = target
    args.function = None
    from unittest.mock import patch

    with patch(
        "harness.commands._rev_query_priority._function_metrics",
        return_value=candidates,
    ):
        exclusions = _priority_rows(connection, args, root=tmp_path)
    assert [entry["candidate_exclusion"] for entry in exclusions] == [
        "shared_sdk_symbol",
        "in_image_pointer_table",
        "ascii_or_nul_data",
    ]

    args.exclusions = False
    with patch(
        "harness.commands._rev_query_priority._function_metrics",
        return_value=candidates,
    ):
        ranked = _priority_rows(connection, args, root=tmp_path)
    assert [entry["id"] for entry in ranked] == [f"{target}@80100018"]


def test_xrefs_are_filtered_by_target(capsys, monkeypatch) -> None:
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    _schema(connection)
    for target in ("exe/one", "exe/two"):
        connection.execute(
            "INSERT INTO targets VALUES (?, 'b', 'h', 0, 'rizin', 'v', 's', 'sh')",
            (target,),
        )
        connection.execute(
            "INSERT INTO xrefs VALUES (?, 0x80100010, 0x80100020, 'data')",
            (target,),
        )
    monkeypatch.setattr("harness.commands.rev_query.connect", lambda _root: connection)

    assert main(["xrefs", "exe/one@0x80100020"]) == 0

    output = capsys.readouterr().out
    assert "exe/one" in output
    assert "exe/two" not in output


def test_xrefs_require_target_qualified_address(capsys, monkeypatch) -> None:
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    _schema(connection)
    monkeypatch.setattr("harness.commands.rev_query.connect", lambda _root: connection)

    assert main(["xrefs", "0x80100020"]) == 2

    assert "function ID must be TARGET@8-digit-address" in capsys.readouterr().err


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


def test_mission_composes_sdk_callees_callers_and_risk(
    capsys, monkeypatch, tmp_path
) -> None:
    manifest = tmp_path / "config" / "targets" / "exe" / "t" / "target.toml"
    manifest.parent.mkdir(parents=True)
    manifest.write_text(
        'schema = "harness.target/v2"\nid = "exe/t"\nkind = "executable"\n'
        'source_dir = "src/exe/t"\nbinary = "out/binaries/exe/t.bin"\n'
        'splat = "config/targets/exe/t/splat.yaml"\nload_address = 0x80100000\n',
        encoding="utf-8",
    )
    sdk = tmp_path / "config" / "sdk"
    sdk.mkdir(parents=True)
    (sdk / "psyq-slus.txt").write_text("PadInit = 0x80174668;\n", encoding="utf-8")

    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    _schema(connection)
    connection.execute(
        "INSERT INTO targets VALUES ('exe/t', 'b', 'h', 0x80100000, 'rizin', 'v', 's', 'sh')"
    )
    columns = "?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?"
    connection.execute(
        f"INSERT INTO functions VALUES ({columns})",
        (
            "exe/t@80100000",
            "exe/t",
            0x80100000,
            16,
            "func_80100000",
            "a" * 64,
            1,
            0,
            None,
            6,
            2,
            1,
            2,
            1,
            0,
            1,
            1,
            None,
        ),
    )
    connection.execute(
        f"INSERT INTO functions VALUES ({columns})",
        (
            "exe/t@80174668",
            "exe/t",
            0x80174668,
            8,
            "PadInit",
            "b" * 64,
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
        ),
    )
    connection.execute(
        f"INSERT INTO functions VALUES ({columns})",
        (
            "exe/t@80100100",
            "exe/t",
            0x80100100,
            8,
            "func_80100100",
            "c" * 64,
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
        ),
    )
    connection.execute(
        "INSERT INTO calls VALUES ('exe/t@80100000', 'exe/t@80174668', 0x80100004)"
    )
    connection.execute(
        "INSERT INTO calls VALUES ('exe/t@80100100', 'exe/t@80100000', 0x80100104)"
    )
    connection.execute(
        "INSERT INTO unresolved_calls VALUES ('exe/t@80100000', 0x80174668, 0x80100008, 'unknown')"
    )
    monkeypatch.setattr(
        "harness.commands._rev_query_mission.connect", lambda _root: connection
    )

    assert main(["--root", str(tmp_path), "mission", "exe/t@0x80100000", "--json"]) == 0

    out = json.loads(capsys.readouterr().out)
    assert out["function"] == "exe/t@80100000"
    assert out["psyq_space"] == "slus"
    assert {"address": "0x80174668", "name": "PadInit"} in out["sdk_callees"]
    assert {"address": "0x80174668", "name": "PadInit"} in out["sdk_unresolved"]
    assert any(c["caller"] == "exe/t@80100100" for c in out["callers"])
    assert any(c["callee"] == "exe/t@80174668" for c in out["callees"])
