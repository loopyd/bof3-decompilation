from __future__ import annotations

import json
import sqlite3

from harness.analysis.graph import dominates, enrich_graph, function_metrics, sccs
from harness.analysis.priority import candidate_exclusion, priority_rows
from harness.commands.rev_query import _project_rows, build_parser, main
from harness.analysis.schema import create_schema


def test_sccs_are_deterministic_and_collapse_recursion() -> None:
    edges = {"a": {"b"}, "b": {"a", "c"}, "c": set()}

    assert sccs(["c", "b", "a"], edges) == [["a", "b"], ["c"]]


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

    assert dominates(better, worse)
    assert not dominates({**better, "metric_missing": 1}, worse)


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
        "load_address = 0x80100000\n"
        'sources = ["src/exe/t/func_80100000.c"]\n',
        encoding="utf-8",
    )
    claimed = tmp_path / "src/exe/t/func_80100000.c"
    claimed.parent.mkdir(parents=True, exist_ok=True)
    claimed.write_text("void placeholder(void) {}\n", encoding="utf-8")
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
    assert candidate_exclusion(tmp_path, candidates[0]) == "shared_sdk_symbol"
    assert candidate_exclusion(tmp_path, candidates[1]) == "in_image_pointer_table"
    assert candidate_exclusion(tmp_path, candidates[2]) == "ascii_or_nul_data"
    assert candidate_exclusion(tmp_path, candidates[3]) is None

    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    create_schema(connection)
    args = build_parser().parse_args(["quick-wins", "--exclusions", "--limit", "0"])
    args.target = target
    args.function = None
    from unittest.mock import patch

    with patch(
        "harness.analysis.priority.function_metrics",
        return_value=candidates,
    ):
        exclusions = priority_rows(
            connection,
            target=args.target,
            command=args.command,
            limit=args.limit,
            exclusions=getattr(args, "exclusions", False),
            include_trivial=getattr(args, "include_trivial", False),
            unlifted=getattr(args, "unlifted", False),
            function=getattr(args, "function", None),
            root=tmp_path,
        )
    assert [entry["candidate_exclusion"] for entry in exclusions] == [
        "shared_sdk_symbol",
        "in_image_pointer_table",
        "ascii_or_nul_data",
    ]

    args.exclusions = False
    with patch(
        "harness.analysis.priority.function_metrics",
        return_value=candidates,
    ):
        ranked = priority_rows(
            connection,
            target=args.target,
            command=args.command,
            limit=args.limit,
            exclusions=getattr(args, "exclusions", False),
            include_trivial=getattr(args, "include_trivial", False),
            unlifted=getattr(args, "unlifted", False),
            function=getattr(args, "function", None),
            root=tmp_path,
        )
    assert [entry["id"] for entry in ranked] == [f"{target}@80100018"]


def test_type_query_cli_details_filters_and_json(capsys, monkeypatch) -> None:
    def typed_connection():
        connection = sqlite3.connect(":memory:")
        connection.row_factory = sqlite3.Row
        create_schema(connection)
        connection.execute(
            "INSERT INTO targets VALUES ('exe/test', 'b', 'h', 0, 'rizin', 'v', 's', 'sh')"
        )
        connection.execute(
            "INSERT INTO type_declarations VALUES ('d', 'exe/test', 'Thing', 'struct', 'Thing', "
            "'h', 'header_claim', 'struct Thing;', 'reviewed', 4, 4, NULL)"
        )
        connection.execute(
            "INSERT INTO type_usages VALUES ('exe/test', 'h', 'g_thing', NULL, 'Thing', "
            "'global', 'data', 'header_claim', 'declaration')"
        )
        connection.execute(
            "INSERT INTO type_candidates VALUES ('c', 'exe/test', 1, 5, 'aggregate_region', "
            "'representation', 4, 'unknown', 'blocked', 'lead', 'unresolved', '[]', 'gap')"
        )
        return connection

    monkeypatch.setattr(
        "harness.commands.rev_query.connect", lambda _root: typed_connection()
    )

    assert (
        main(["--json", "types", "Thing", "--target", "exe/test", "--detail", "full"])
        == 0
    )
    declaration = json.loads(capsys.readouterr().out)[0]
    assert declaration["canonical"] == "struct Thing;"
    assert declaration["fields"] == []

    assert main(["--json", "type-uses", "g_thing", "--target", "exe/test"]) == 0
    assert json.loads(capsys.readouterr().out)[0]["type_name"] == "Thing"

    assert (
        main(
            [
                "--json",
                "type-candidates",
                "--target",
                "exe/test",
                "--status",
                "blocked",
                "--kind",
                "aggregate_region",
            ]
        )
        == 0
    )
    candidate = json.loads(capsys.readouterr().out)[0]
    assert candidate["representation_status"] == "lead"
    assert candidate["semantic_status"] == "unresolved"


def test_macro_query_cli_filters_and_decodes_payload(capsys, monkeypatch) -> None:
    def indexed_connection():
        connection = sqlite3.connect(":memory:")
        connection.row_factory = sqlite3.Row
        create_schema(connection)
        connection.execute(
            "INSERT INTO targets VALUES ('exe/test', 'b', 'h', 0, 'rizin', 'v', 's', 'sh')"
        )
        connection.execute(
            "INSERT INTO macro_definitions VALUES "
            "('d', '__shared__', 'EMIT', 'src/shared/x.inc', 1, '[\"name\"]', "
            "'void name(void) {}', '[]', 'body_emitting_template', 'shared_template', "
            "'[\"target_local_wrapper_required\"]', 0, 'existing', 'abc', NULL)"
        )
        connection.execute(
            "INSERT INTO macro_uses VALUES "
            "('exe/test', 'd', 'EMIT', 'src/test/x.c', 4, 1, 'run', '[]', "
            "'expansion', NULL, 0, 'existing', '[\"target_local_wrapper_required\"]')"
        )
        return connection

    monkeypatch.setattr(
        "harness.commands.rev_query.connect", lambda _root: indexed_connection()
    )

    assert main(["--json", "macros", "EMIT", "--target", "exe/test"]) == 0
    definition = json.loads(capsys.readouterr().out)[0]
    assert definition["parameters"] == ["name"]
    assert definition["restrictions"] == ["target_local_wrapper_required"]

    assert main(["--json", "macro-uses", "EMIT", "--target", "exe/test"]) == 0
    use = json.loads(capsys.readouterr().out)[0]
    assert use["definition_owner"] == "__shared__"
    assert use["source_line"] == 4


def test_xrefs_are_filtered_by_target(capsys, monkeypatch) -> None:
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    create_schema(connection)
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
    create_schema(connection)
    monkeypatch.setattr("harness.commands.rev_query.connect", lambda _root: connection)

    assert main(["xrefs", "0x80100020"]) == 2

    assert "function ID must be TARGET@8-digit-address" in capsys.readouterr().err


def test_inventory_lists_target_raw_functions_and_data(
    capsys, monkeypatch, tmp_path
) -> None:
    monkeypatch.setattr(
        "harness.commands.rev_query.load_target_manifests",
        lambda _root: {"exe/t": object()},
    )
    monkeypatch.setattr(
        "harness.commands.rev_query.collect_naming_debt",
        lambda _root, _manifests: type(
            "Debt",
            (),
            {
                "raw_functions": frozenset(
                    {"exe/t:func_80100000", "exe/u:func_80200000"}
                ),
                "raw_data": frozenset({"exe/t:D_80100010"}),
            },
        )(),
    )
    monkeypatch.setattr(
        "harness.commands.rev_query.connect",
        lambda _root: (_ for _ in ()).throw(
            AssertionError("inventory must not open index")
        ),
    )

    assert (
        main(["--root", str(tmp_path), "--json", "--limit", "1", "inventory", "exe/t"])
        == 0
    )

    assert json.loads(capsys.readouterr().out) == [
        {"kind": "function", "name": "func_80100000"},
        {"kind": "data", "name": "D_80100010"},
    ]


def test_describe_reports_payload_splat_symbol_and_references(
    capsys, monkeypatch, tmp_path
) -> None:
    target = "exe/test"
    config = tmp_path / "config/targets/exe/test/target.toml"
    config.parent.mkdir(parents=True)
    config.write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src/test'\nbinary='out/test.bin'\nload_address=0x80100000\n"
        "splat='config/targets/exe/test/splat.yaml'\n",
        encoding="utf-8",
    )
    (config.parent / "splat.yaml").write_text(
        "segments:\n  - [0x0, data, D_80100000]\n  - [0x20]\n",
        encoding="utf-8",
    )
    binary = tmp_path / "out/test.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"\0" * 0x20)
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    create_schema(connection)
    connection.execute(
        "INSERT INTO targets VALUES (?, 'b', 'h', 0x80100000, 'rizin', 'v', 's', 'sh')",
        (target,),
    )
    connection.execute(
        "INSERT INTO symbols VALUES (?, 0x80100010, 'D_80100010', 'data')", (target,)
    )
    connection.execute(
        "INSERT INTO data_references VALUES (?, NULL, 0x80100004, 0x80100010, 'D_80100010', 'store', 'sh')",
        (target,),
    )
    monkeypatch.setattr("harness.commands.rev_query.connect", lambda _root: connection)

    assert (
        main(["--root", str(tmp_path), "--json", "describe", f"{target}@0x80100010"])
        == 0
    )
    row = json.loads(capsys.readouterr().out)[0]
    assert row["payload"] == {
        "contained": True,
        "file_offset": "0x10",
        "payload_offset": "0x10",
        "remaining_bytes": 16,
    }
    assert row["splat"]["kind"] == "data"
    assert row["symbol"] == {"kind": "data", "name": "D_80100010"}
    assert row["references"][0]["opcode"] == "sh"


def test_owners_find_other_images_covering_runtime_address(capsys, monkeypatch) -> None:
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    create_schema(connection)
    for target in ("emi/etc/game/01", "exe/slus_004_22", "emi/other/00"):
        connection.execute(
            "INSERT INTO targets VALUES (?, 'b', 'h', 0, 'rizin', 'v', 's', 'sh')",
            (target,),
        )
    columns = "?,?,?,?,?,NULL,?,NULL,NULL,?,?,?,'unlifted',?,?,?,?,?,?,?,?,?,0"
    for function_id, target, address, size, name in (
        ("emi/etc/game/01@801D0000", "emi/etc/game/01", 0x801D0000, 0x100, "overlay"),
        ("exe/slus_004_22@8014B854", "exe/slus_004_22", 0x8014B854, 0x28, "resident"),
        ("emi/other/00@8014B850", "emi/other/00", 0x8014B850, 0x10, "overlap"),
    ):
        connection.execute(
            f"INSERT INTO functions VALUES ({columns})",
            (
                function_id,
                target,
                address,
                size,
                name,
                "a" * 64,
                0,
                0,
                None,
                1,
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
    connection.executemany(
        "INSERT INTO function_candidates VALUES (?, ?, ?, ?, 'analyzer_range', 'medium', 1)",
        (
            ("exe/slus_004_22", 0x8014B854, 0x8014B87C, "resident"),
            ("emi/other/00", 0x8014B850, 0x8014B860, "overlap"),
        ),
    )
    monkeypatch.setattr("harness.commands.rev_query.connect", lambda _root: connection)

    assert main(["--json", "owners", "emi/etc/game/01@0x8014B854"]) == 0

    assert json.loads(capsys.readouterr().out) == [
        {
            "function_address": "0x8014B854",
            "function_end": "0x8014B87C",
            "confidence": "medium",
            "match": "entry",
            "name": "resident",
            "payload_contained": 1,
            "provenance": "analyzer_range",
            "size": 40,
            "target_id": "exe/slus_004_22",
        },
        {
            "function_address": "0x8014B850",
            "function_end": "0x8014B860",
            "confidence": "medium",
            "match": "contains",
            "name": "overlap",
            "payload_contained": 1,
            "provenance": "analyzer_range",
            "size": 16,
            "target_id": "emi/other/00",
        },
    ]


def test_xrefs_include_rich_data_reference_metadata(capsys, monkeypatch) -> None:
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    create_schema(connection)
    connection.execute(
        "INSERT INTO targets VALUES ('exe/test', 'b', 'h', 0, 'rizin', 'v', 's', 'sh')"
    )
    connection.execute(
        "INSERT INTO data_references VALUES ('exe/test', NULL, 0x80100004, 0x80100010, 'D_80100010', 'store', 'sh')"
    )
    monkeypatch.setattr("harness.commands.rev_query.connect", lambda _root: connection)

    assert main(["--json", "xrefs", "exe/test@0x80100010"]) == 0
    assert json.loads(capsys.readouterr().out) == [
        {
            "destination": "0x80100010",
            "function_id": None,
            "kind": "store",
            "opcode": "sh",
            "source": "0x80100004",
            "target_id": "exe/test",
        }
    ]


def test_xrefs_limit_applies_per_reference_kind(capsys, monkeypatch) -> None:
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    create_schema(connection)
    connection.execute(
        "INSERT INTO targets VALUES ('exe/test', 'b', 'h', 0, 'rizin', 'v', 's', 'sh')"
    )
    connection.execute(
        "INSERT INTO xrefs VALUES ('exe/test', 0x80100008, 0x80100010, 'call')"
    )
    connection.execute(
        "INSERT INTO data_references VALUES ('exe/test', NULL, 0x80100004, 0x80100010, NULL, 'load', 'lw')"
    )
    monkeypatch.setattr("harness.commands.rev_query.connect", lambda _root: connection)

    assert main(["--json", "--limit", "1", "xrefs", "exe/test@0x80100010"]) == 0
    assert [row["kind"] for row in json.loads(capsys.readouterr().out)] == [
        "call",
        "load",
    ]


def test_metrics_distinguish_recursive_leaf_with_unresolved_evidence() -> None:
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    create_schema(connection)
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
        "INSERT INTO functions VALUES (?, ?, ?, ?, ?, NULL, ?, NULL, NULL, ?, ?, ?, 'unlifted', ?, ?, ?, ?, ?, ?, ?, ?, ?, 0)",
        function,
    )
    second = ("t@00000002", "t", 2, 8, "b", "b" * 64, *function[6:])
    connection.execute(
        "INSERT INTO functions VALUES (?, ?, ?, ?, ?, NULL, ?, NULL, NULL, ?, ?, ?, 'unlifted', ?, ?, ?, ?, ?, ?, ?, ?, ?, 0)",
        second,
    )
    connection.execute("INSERT INTO calls VALUES ('t@00000001', 't@00000002', 1)")
    connection.execute("INSERT INTO calls VALUES ('t@00000002', 't@00000001', 2)")
    connection.execute(
        "INSERT INTO unresolved_calls VALUES ('t@00000001', 99, 3, 'unknown')"
    )

    metrics = function_metrics(connection, "t")
    enrich_graph(connection, metrics)

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
        'splat = "config/targets/exe/t/splat.yaml"\nload_address = 0x80100000\n'
        'sources = ["src/exe/t/func_80100000.c"]\n',
        encoding="utf-8",
    )
    claimed = tmp_path / "src/exe/t/func_80100000.c"
    claimed.parent.mkdir(parents=True, exist_ok=True)
    claimed.write_text("void placeholder(void) {}\n", encoding="utf-8")
    (manifest.parent / "splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [16]\n",
        encoding="utf-8",
    )
    sdk = tmp_path / "config" / "sdk"
    sdk.mkdir(parents=True)
    (sdk / "psyq-slus.txt").write_text("PadInit = 0x80174668;\n", encoding="utf-8")

    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    create_schema(connection)
    connection.execute(
        "INSERT INTO targets VALUES ('exe/t', 'b', 'h', 0x80100000, 'rizin', 'v', 's', 'sh')"
    )
    columns = "?,?,?,?,?,NULL,?,NULL,NULL,?,?,?,'unlifted',?,?,?,?,?,?,?,?,?,0"
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
    monkeypatch.setattr("harness.analysis.mission.connect", lambda _root: connection)

    assert main(["--root", str(tmp_path), "mission", "exe/t@0x80100000", "--json"]) == 0

    out = json.loads(capsys.readouterr().out)
    assert out["function"] == "exe/t@80100000"
    assert out["psyq_space"] == "slus"
    assert {"address": "0x80174668", "name": "PadInit"} in out["sdk_callees"]
    assert {"address": "0x80174668", "name": "PadInit"} in out["sdk_unresolved"]
    assert any(c["caller"] == "exe/t@80100100" for c in out["callers"])
    assert any(c["callee"] == "exe/t@80174668" for c in out["callees"])
