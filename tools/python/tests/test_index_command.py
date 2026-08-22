from argparse import Namespace
from contextlib import redirect_stdout
import io
import json
from pathlib import Path
import sqlite3
import subprocess
from typing import Any, cast

import pytest

from harness.commands import analysis_readiness, index


def test_analysis_readiness_reports_unavailable_work_graphs_without_index(
    tmp_path: Path, monkeypatch
) -> None:
    monkeypatch.setattr(
        analysis_readiness,
        "load_target_manifests",
        lambda _root: {"emi/test/archive/00": object()},
    )
    monkeypatch.setattr(
        analysis_readiness,
        "project_status",
        lambda _root, target: {"target": target, "fresh": False},
    )
    monkeypatch.setattr(
        analysis_readiness,
        "_summaries",
        lambda *_args: (_ for _ in ()).throw(FileNotFoundError("missing index")),
    )

    payload = analysis_readiness.readiness(tmp_path)

    assert payload["schema"] == "bof3.analysis-readiness/v2"
    assert payload["ready"] is False
    assert payload["stale_facts"] == {
        "count": 2,
        "targets": ["emi/test/archive/00"],
        "index": True,
    }
    summaries = payload["summaries"]
    assert isinstance(summaries, dict)
    assert summaries["naming"] == {
        "available": False,
        "error": "missing index",
    }
    assert payload["recovery"] == "bin/index --recover"


def test_analysis_readiness_includes_naming_type_macro_work_graphs(
    tmp_path: Path, monkeypatch
) -> None:
    monkeypatch.setattr(
        analysis_readiness,
        "load_target_manifests",
        lambda _root: {"emi/test/archive/00": object()},
    )
    monkeypatch.setattr(
        analysis_readiness,
        "project_status",
        lambda _root, target: {"target": target, "fresh": True},
    )
    summaries = {
        concern: {"work_graph": []} for concern in ("naming", "types", "macros")
    }
    monkeypatch.setattr(analysis_readiness, "_summaries", lambda *_args: summaries)

    payload = analysis_readiness.readiness(tmp_path, "emi/test/archive/00")

    assert payload["ready"] is True
    assert payload["stale_facts"] == {"count": 0, "targets": [], "index": False}
    assert payload["summaries"] == summaries
    assert payload["recovery"] is None


def test_analysis_readiness_summary_filters_all_counts_and_work_to_target(
    tmp_path: Path, monkeypatch
) -> None:
    database = sqlite3.connect(":memory:")
    database.row_factory = sqlite3.Row
    for table in ("type_fields", "type_usages", "type_conflicts", "macro_uses"):
        database.execute(f"CREATE TABLE {table} (target_id TEXT)")
    database.execute("CREATE TABLE type_declarations (target_id TEXT, diagnostic TEXT)")
    database.execute("CREATE TABLE macro_definitions (owner_target TEXT)")
    database.execute("CREATE TABLE macro_templates (owner_target TEXT)")
    for target in ("a", "a", "b"):
        database.execute("INSERT INTO type_declarations VALUES (?, NULL)", (target,))
        database.execute("INSERT INTO type_fields VALUES (?)", (target,))
        database.execute("INSERT INTO type_usages VALUES (?)", (target,))
        database.execute("INSERT INTO type_conflicts VALUES (?)", (target,))
        database.execute("INSERT INTO macro_definitions VALUES (?)", (target,))
        database.execute("INSERT INTO macro_uses VALUES (?)", (target,))
        database.execute("INSERT INTO macro_templates VALUES (?)", (target,))
    database.execute("INSERT INTO macro_definitions VALUES ('__shared__')")
    database.execute("INSERT INTO macro_templates VALUES ('__shared__')")
    database.execute("UPDATE type_declarations SET diagnostic='x' WHERE rowid=1")
    monkeypatch.setattr(analysis_readiness, "connect", lambda _root: database)
    monkeypatch.setattr(
        analysis_readiness,
        "inventory_expected",
        lambda _root, target, _manifests: (
            {("function", "func_00000001")}
            if target == "a"
            else {("data", "D_00000002")}
        ),
    )

    class Work:
        def items(self, _address, _kind):
            return ["one", "two"]

    monkeypatch.setattr(
        analysis_readiness, "required_work_snapshot", lambda *_args: Work()
    )

    class Debt:
        def to_rows(self):
            return {"raw_functions": ["a:x", "b:y"], "raw_data": []}

    monkeypatch.setattr(analysis_readiness, "collect_naming_debt", lambda *_: Debt())
    monkeypatch.setattr(
        analysis_readiness,
        "type_account",
        lambda _root: {
            "rows": [
                {"id": "ta", "target": "a", "kind": "field", "status": "blocked"},
                {"id": "tb", "target": "b", "kind": "prototype", "status": "proposed"},
            ]
        },
    )
    monkeypatch.setattr(
        analysis_readiness,
        "macro_account",
        lambda _root: {
            "rows": [
                {
                    "id": "ma",
                    "kind": "constant",
                    "status": "blocked",
                    "targets": ["a"],
                    "shared": False,
                },
                {
                    "id": "mb",
                    "kind": "accessor",
                    "status": "accepted",
                    "targets": ["a", "b"],
                    "shared": False,
                },
                {
                    "id": "mc",
                    "kind": "constant",
                    "status": "accepted",
                    "targets": ["b"],
                    "shared": False,
                },
            ]
        },
    )

    summary = cast(
        dict[str, Any],
        analysis_readiness._summaries(
            tmp_path,
            cast(
                Any,
                {
                    "a": Namespace(sources=()),
                    "b": Namespace(sources=()),
                },
            ),
            ["a"],
            False,
        ),
    )

    assert summary["naming"]["inventory_count"] == 1
    assert summary["naming"]["required_work_count"] == 2
    assert summary["naming"]["debt"] == {"raw_functions": 1, "raw_data": 0}
    assert summary["types"] == {
        "inventory_count": 2,
        "field_count": 2,
        "usage_count": 2,
        "conflict_count": 2,
        "diagnostic_count": 1,
        "candidate_count": 1,
        "proposed_transactions": 0,
        "unresolved_evidence_ceiling_count": 1,
        "work_graph": [{"status": "blocked", "count": 1}],
    }
    assert summary["macros"] == {
        "inventory_count": 3,
        "use_count": 2,
        "template_count": 3,
        "candidate_count": 2,
        "proposed_transactions": 1,
        "unresolved_evidence_ceiling_count": 1,
        "work_graph": [
            {"status": "accepted", "count": 1},
            {"status": "blocked", "count": 1},
        ],
    }


def test_target_macro_rows_are_canonical_global_subsets() -> None:
    rows = [
        {"id": "local-a", "targets": ["a"], "shared": False},
        {"id": "cross", "targets": ["a", "b"], "shared": False},
        {"id": "shared", "targets": [], "shared": True},
        {"id": "local-b", "targets": ["b"], "shared": False},
    ]

    a_rows = analysis_readiness._target_macro_rows(rows, "a")
    b_rows = analysis_readiness._target_macro_rows(rows, "b")

    assert [row["id"] for row in a_rows] == ["local-a", "cross", "shared"]
    assert [row["id"] for row in b_rows] == ["cross", "shared", "local-b"]
    assert {row["id"] for row in a_rows + b_rows} == {row["id"] for row in rows}
    assert all(row in rows for row in a_rows + b_rows)


def test_analysis_readiness_default_output_has_strict_bounds(monkeypatch) -> None:
    payload = {
        "schema": "bof3.analysis-readiness/v2",
        "ready": True,
        "summaries": {
            concern: {"inventory_count": 0, "work_graph": []}
            for concern in ("naming", "types", "macros")
        },
    }
    monkeypatch.setattr(analysis_readiness, "readiness", lambda *_args: payload)
    output = io.StringIO()
    with redirect_stdout(output):
        assert (
            analysis_readiness.run(
                Namespace(root=Path("."), target=None, detail="summary")
            )
            == 0
        )
    encoded = output.getvalue().encode()
    assert json.loads(encoded)["summaries"]["types"]["work_graph"] == []
    assert len(encoded) <= 1024
    assert encoded.count(b"\n") <= 40


def test_all_live_target_macro_summaries_partition_canonical_global_account(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    root = Path(__file__).parents[3]
    manifests = analysis_readiness.load_target_manifests(root)
    account = analysis_readiness.macro_account(root)
    global_rows = account["rows"]
    monkeypatch.setattr(analysis_readiness, "macro_account", lambda _root: account)
    observed_union: set[str] = set()

    for target in sorted(manifests):
        summary = cast(
            dict[str, Any],
            analysis_readiness._summaries(root, manifests, [target], detail=True),
        )["macros"]
        expected = [
            row
            for row in global_rows
            if row["shared"] is True or target in row["targets"]
        ]
        actual_rows = summary["work_graph"]
        assert [row["id"] for row in actual_rows] == [row["id"] for row in expected]
        assert summary["candidate_count"] == len(expected)
        assert summary["proposed_transactions"] == sum(
            row["status"] == "accepted" for row in expected
        )
        assert summary["unresolved_evidence_ceiling_count"] == sum(
            row["status"] == "blocked" for row in expected
        )
        observed_union.update(row["id"] for row in actual_rows)

    expected_union = {
        row["id"] for row in global_rows if row["shared"] or row["targets"]
    }
    assert observed_union == expected_union


def test_analysis_readiness_live_output_ceiling() -> None:
    root = Path(__file__).parents[3]
    commands = (
        ([root / "bin/analysis-readiness"], 16_000, 450),
        (
            [root / "bin/analysis-readiness", "emi/battle/batl_re2/01"],
            2_500,
            100,
        ),
    )
    for command, byte_ceiling, line_ceiling in commands:
        result = subprocess.run(command, cwd=root, capture_output=True, check=False)
        assert result.returncode == 0, result.stderr.decode()
        assert json.loads(result.stdout)["schema"] == "bof3.analysis-readiness/v2"
        assert len(result.stdout) <= byte_ceiling
        assert result.stdout.count(b"\n") <= line_ceiling


def test_recover_reanalyzes_every_stale_target_before_rebuild(
    tmp_path: Path, monkeypatch
) -> None:
    calls: list[tuple[str, str]] = []
    monkeypatch.setattr(
        index,
        "load_target_manifests",
        lambda _root: {"fresh": object(), "stale/a": object(), "stale/b": object()},
    )
    recovered: set[str] = set()
    monkeypatch.setattr(
        index,
        "status",
        lambda _root, target: {"fresh": target == "fresh" or target in recovered},
    )

    def analyze(_root: Path, target: str, timeout: int) -> None:
        calls.append(("analyze", target))
        recovered.add(target)

    monkeypatch.setattr(index, "analyze_project", analyze)

    def rebuild(_root: Path) -> Path:
        calls.append(("rebuild", "index"))
        output = tmp_path / "out/index/reverse.sqlite"
        output.parent.mkdir(parents=True)
        output.touch()
        return output

    monkeypatch.setattr(index, "rebuild", rebuild)

    assert index.run(Namespace(root=tmp_path, recover=True, timeout=5)) == 0
    assert calls == [
        ("analyze", "stale/a"),
        ("analyze", "stale/b"),
        ("rebuild", "index"),
    ]


def test_recover_rechecks_targets_that_were_initially_fresh(
    tmp_path: Path, monkeypatch
) -> None:
    calls = {"fresh": 0}
    rebuilt = False
    monkeypatch.setattr(
        index,
        "load_target_manifests",
        lambda _root: {"fresh": object(), "stale": object()},
    )

    def status(_root: Path, target: str) -> dict[str, bool]:
        if target == "fresh":
            calls["fresh"] += 1
            return {"fresh": calls["fresh"] == 1}
        return {"fresh": False}

    monkeypatch.setattr(index, "status", status)
    monkeypatch.setattr(index, "analyze_project", lambda *_args, **_kwargs: None)

    def rebuild(_root: Path) -> Path:
        nonlocal rebuilt
        rebuilt = True
        return tmp_path / "out/index/reverse.sqlite"

    monkeypatch.setattr(index, "rebuild", rebuild)
    with pytest.raises(ValueError, match="fresh, stale"):
        index.run(Namespace(root=tmp_path, recover=True, timeout=5))
    assert calls["fresh"] == 2
    assert rebuilt is False


def test_recover_refuses_rebuild_when_target_remains_stale(
    tmp_path: Path, monkeypatch
) -> None:
    rebuilt = False
    monkeypatch.setattr(
        index, "load_target_manifests", lambda _root: {"stale": object()}
    )
    monkeypatch.setattr(index, "status", lambda *_: {"fresh": False})
    monkeypatch.setattr(index, "analyze_project", lambda *_args, **_kwargs: None)

    def rebuild(_root: Path) -> Path:
        nonlocal rebuilt
        rebuilt = True
        return tmp_path / "out/index/reverse.sqlite"

    monkeypatch.setattr(index, "rebuild", rebuild)

    with pytest.raises(ValueError, match="stale targets: stale"):
        index.run(Namespace(root=tmp_path, recover=True, timeout=5))
    assert rebuilt is False
