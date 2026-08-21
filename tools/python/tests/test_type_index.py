from __future__ import annotations

import sqlite3
from pathlib import Path

import pytest

from harness.analysis.schema import create_schema
from harness.analysis import type_context as type_context_module
from harness.analysis.type_context import type_context, type_context_from_connection
from harness.analysis.type_index import (
    infer_type_candidates,
    insert_authored_types,
    insert_shared_scalar_types,
    type_candidates_payload,
    type_usages_payload,
    types_payload,
)
from harness.domain import load_target_manifests
from harness.domain.c_context import declaration_records, scalar_declaration_context


TARGET = "exe/test"


def _connection() -> sqlite3.Connection:
    connection = sqlite3.connect(":memory:")
    connection.row_factory = sqlite3.Row
    create_schema(connection)
    connection.execute(
        "INSERT INTO targets VALUES (?, 'b', 'h', 0, 'rizin', 'v', 's', 'sh')",
        (TARGET,),
    )
    return connection


def test_declaration_records_cover_aggregate_forms_fields_and_forward_declaration() -> (
    None
):
    records = declaration_records(
        """
        typedef struct Node Node;
        struct Forward;
        typedef union Value { volatile u32 word; u8 bytes[4]; } Value;
        typedef enum Kind { KIND_A, KIND_B = 3 } Kind;
        typedef void (*Handler)(Node *node);
        extern const Value values[2];
        s32 run(Node *node, Handler handler);
        """
    )
    by_name = {name: record for record in records for name in record.names}
    assert by_name["Node"].kind == "struct"
    assert by_name["Forward"].tag_name == "Forward"
    assert [(field.name, field.array_extent) for field in by_name["Value"].fields] == [
        ("word", None),
        ("bytes", "4"),
    ]
    assert [field.name for field in by_name["Kind"].fields] == ["KIND_A", "KIND_B"]
    assert by_name["Handler"].kind == "typedef"
    assert by_name["run"].kind == "prototype"


def test_scalar_context_tracks_every_base_alias() -> None:
    header = Path("include/base/types.h").read_text(encoding="utf-8")
    context = scalar_declaration_context(header)
    for name in (
        "bool",
        "s8",
        "s16",
        "s32",
        "s64",
        "u8",
        "u16",
        "u32",
        "u64",
        "f32",
        "f64",
    ):
        assert name in context


def test_reverse_index_owns_every_shared_scalar_alias() -> None:
    connection = _connection()
    insert_shared_scalar_types(connection, Path(".").resolve())
    rows = types_payload(
        connection, target="__shared__", pattern=None, untyped=False, limit=0
    )
    assert {row["name"] for row in rows} >= {
        "bool",
        "s8",
        "s16",
        "s32",
        "s64",
        "u8",
        "u16",
        "u32",
        "u64",
        "f32",
        "f64",
    }
    assert {row["provenance"] for row in rows} == {"shared_base"}


def test_index_populates_claimed_and_owned_header_types_constraints_and_kind(
    tmp_path: Path,
) -> None:
    private = tmp_path / "include/private.h"
    shared = tmp_path / "include/shared.h"
    private.parent.mkdir(parents=True)
    private.write_text(
        '#include "shared.h"\n'
        "typedef struct Local { Shared shared; u16 value; } Local;\n"
        "ASSERT_SIZE(Local, 8);\nASSERT_OFFSET(Local, value, 4);\n"
        "extern Local g_local; /* @kind data */\n",
        encoding="utf-8",
    )
    shared.write_text(
        "typedef struct Shared { u32 value; } Shared;\n", encoding="utf-8"
    )
    manifest = type(
        "Manifest",
        (),
        {
            "headers": (Path("include/private.h"),),
            "sources": (),
            "support_sources": (),
            "has_explicit_sources": True,
        },
    )()
    connection = _connection()
    insert_authored_types(connection, tmp_path, TARGET, manifest)
    types = types_payload(
        connection, target=TARGET, pattern=None, untyped=False, limit=0
    )
    assert {(row["name"], row["provenance"]) for row in types} >= {
        ("Local", "header_claim"),
    }
    assert all(row["name"] != "Shared" for row in types)
    assert [
        tuple(row)
        for row in connection.execute(
            "SELECT constraint_kind, field_name, value FROM type_constraints "
            "WHERE type_name='Local' ORDER BY constraint_kind"
        )
    ] == [("offset", "value", "4"), ("size", None, "8")]
    usage = type_usages_payload(connection, target=TARGET, pattern="g_local", limit=0)[
        0
    ]
    assert usage["type_name"] == "Local"
    assert usage["storage_kind"] == "data"


def test_registry_context_falls_back_for_missing_or_placeholder_index(
    tmp_path: Path,
) -> None:
    header = tmp_path / "include/base/types.h"
    header.parent.mkdir(parents=True)
    header.write_text("typedef unsigned int u32;\n", encoding="utf-8")

    for placeholder in (False, True):
        index = tmp_path / "out/index/reverse.sqlite"
        index.unlink(missing_ok=True)
        if placeholder:
            index.parent.mkdir(parents=True, exist_ok=True)
            index.write_bytes(b"bootstrap placeholder")
        context = type_context(tmp_path, TARGET, "u32 value;")
        assert "WARNING: reverse type index unavailable" in context
        assert "typedef unsigned int u32;" in context


def test_registry_context_does_not_mask_stale_real_index(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    header = tmp_path / "include/base/types.h"
    header.parent.mkdir(parents=True)
    header.write_text("typedef unsigned int u32;\n", encoding="utf-8")
    (tmp_path / ".git").mkdir()
    index = tmp_path / "out/index/reverse.sqlite"
    index.parent.mkdir(parents=True)
    with sqlite3.connect(index) as connection:
        connection.execute("CREATE TABLE metadata (key TEXT PRIMARY KEY, value TEXT)")
        connection.execute("INSERT INTO metadata VALUES ('schema', 'real-schema')")

    monkeypatch.setattr(
        type_context_module,
        "connect",
        lambda _root: (_ for _ in ()).throw(ValueError("stale real index")),
    )

    with pytest.raises(ValueError, match="stale real index"):
        type_context(tmp_path, TARGET, "u32 value;")


def test_registry_context_closes_dependencies_across_owned_headers() -> None:
    connection = _connection()
    connection.executemany(
        "INSERT INTO type_declarations VALUES (?, ?, ?, 'typedef', NULL, ?, 'header_claim', ?, "
        "'reviewed', NULL, NULL, NULL)",
        (
            (
                f"{TARGET}:include/shared.h:typedef:Shared",
                TARGET,
                "Shared",
                "include/shared.h",
                "typedef struct Shared { u32 value;} Shared;",
            ),
            (
                f"{TARGET}:include/private.h:typedef:Local",
                TARGET,
                "Local",
                "include/private.h",
                "typedef struct Local { Shared member;} Local;",
            ),
        ),
    )
    connection.execute(
        "INSERT INTO type_declarations VALUES ('shared:u32', '__shared__', 'u32', "
        "'typedef', NULL, 'include/base/types.h', 'shared_base', "
        "'typedef unsigned int u32;', 'reviewed', NULL, NULL, NULL)"
    )

    context = type_context_from_connection(connection, TARGET, "Local value;")

    assert "typedef unsigned int u32;" in context
    assert "typedef struct Shared" in context
    assert "typedef struct Local" in context
    assert context.index("Shared") < context.rindex("Local")


def test_normal_type_projection_discloses_omitted_evidence() -> None:
    connection = _connection()
    connection.execute(
        "INSERT INTO type_declarations VALUES ('x', ?, 'X', 'struct', 'X', 'h', "
        "'header_claim', 'typedef struct X { u8 value;} X;', 'reviewed', 1, 1, NULL)",
        (TARGET,),
    )

    row = types_payload(
        connection,
        target=TARGET,
        pattern="X",
        untyped=False,
        limit=0,
        detail="normal",
    )[0]

    assert row["provenance"] == "header_claim"
    assert row["review_status"] == "reviewed"
    assert row["evidence_truncated"] is True
    assert row["full_evidence"] == "rerun with --detail full"
    assert "canonical" not in row


def test_target_private_same_name_layouts_remain_separate() -> None:
    connection = _connection()
    connection.execute(
        "INSERT INTO targets VALUES ('exe/other', 'b', 'h', 0, 'rizin', 'v', 's', 'sh')"
    )
    for target, canonical in (
        (TARGET, "typedef struct X { u8 a;} X;"),
        ("exe/other", "typedef struct X { u32 a;} X;"),
    ):
        connection.execute(
            "INSERT INTO type_declarations VALUES (?, ?, 'X', 'struct', 'X', 'h', 'header_claim', ?, 'reviewed', NULL, NULL, NULL)",
            (f"{target}:h:struct:X", target, canonical),
        )
    assert (
        connection.execute(
            "SELECT COUNT(DISTINCT canonical) FROM type_declarations WHERE name='X'"
        ).fetchone()[0]
        == 2
    )


def test_inference_proposes_only_multi_consumer_consistent_width_and_blocks_conflict() -> (
    None
):
    connection = _connection()
    for function in ("a", "b", "c", "d"):
        connection.execute(
            "INSERT INTO functions VALUES (?, ?, 0, 4, ?, NULL, 'h', NULL, NULL, 0, 0, NULL, 'unlifted', 1, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0)",
            (function, TARGET, function),
        )
    for function in ("a", "b"):
        connection.execute(
            "INSERT INTO data_references VALUES (?, ?, ?, 0x80100010, NULL, 'load', 'lbu')",
            (TARGET, function, 1 if function == "a" else 2),
        )
    connection.execute(
        "INSERT INTO data_references VALUES (?, 'c', 3, 0x80100020, NULL, 'load', 'lbu')",
        (TARGET,),
    )
    connection.execute(
        "INSERT INTO data_references VALUES (?, 'd', 4, 0x80100020, NULL, 'load', 'lw')",
        (TARGET,),
    )
    infer_type_candidates(connection, TARGET)
    rows = type_candidates_payload(connection, target=TARGET, status=None, limit=0)
    by_address = {row["address"]: row for row in rows}
    assert by_address["0x80100010"]["status"] == "blocked"
    assert by_address["0x80100010"]["kind"] == "storage"
    assert "aggregate base" in by_address["0x80100010"]["blocker"]
    assert by_address["0x80100010"]["width"] == 1
    assert by_address["0x80100010"]["signedness"] == "unsigned"
    assert by_address["0x80100020"]["status"] == "blocked"
    assert by_address["0x80100020"]["blocker"] == "conflicting access widths"
    assert connection.execute("SELECT COUNT(*) FROM type_conflicts").fetchone()[0] == 1
    assert {row["representation_status"] for row in rows} == {"lead"}
    assert {row["semantic_status"] for row in rows} == {"unresolved"}


def test_complex_inference_emits_blocked_regions_fields_arrays_and_prototypes() -> None:
    connection = _connection()
    connection.execute(
        "INSERT INTO symbols VALUES (?, 0x80100010, 'D_80100010', 'data')", (TARGET,)
    )
    for ordinal, address in enumerate((0x80100010, 0x80100014, 0x80100018)):
        function = f"f{ordinal}"
        connection.execute(
            "INSERT INTO functions VALUES (?, ?, ?, 4, ?, NULL, 'h', NULL, NULL, 0, 0, NULL, "
            "'unlifted', 1, NULL, NULL, NULL, NULL, NULL, NULL, 1, NULL, 0)",
            (function, TARGET, 0x80110000 + ordinal * 4, function),
        )
        connection.execute(
            "INSERT INTO data_references VALUES (?, ?, ?, ?, NULL, 'load', 'lw')",
            (TARGET, function, 0x80110000 + ordinal * 4, address),
        )
    infer_type_candidates(connection, TARGET)
    rows = type_candidates_payload(connection, target=TARGET, status="blocked", limit=0)
    kinds = {row["kind"] for row in rows}
    assert {
        "aggregate_region",
        "field_offset_0",
        "field_offset_4",
        "field_offset_8",
        "array_stride",
        "prototype",
    } <= kinds
    assert all(row["semantic_status"] == "unresolved" for row in rows)
    assert all(row["status"] == "blocked" for row in rows)


def test_declaration_parser_handles_nested_braces_and_anonymous_typedef() -> None:
    record = declaration_records(
        "typedef struct { union { u8 a; u16 b; } nested; u8 tail[2]; } Outer;"
    )[0]
    assert record.names == ("Outer",)
    assert record.kind == "struct"
    assert [field.name for field in record.fields] == ["nested", "tail"]


def test_assertions_resolve_declaration_size_field_offset_and_width(
    tmp_path: Path,
) -> None:
    header = tmp_path / "include/private.h"
    header.parent.mkdir(parents=True)
    header.write_text(
        "typedef struct X { u8 a; u16 values[2]; } X;\n"
        "ASSERT_SIZE(X, 8);\nASSERT_OFFSET(X, values, 2);\n",
        encoding="utf-8",
    )
    manifest = type(
        "M",
        (),
        {
            "headers": (Path("include/private.h"),),
            "sources": (),
            "support_sources": (),
            "has_explicit_sources": True,
        },
    )()
    connection = _connection()
    insert_authored_types(connection, tmp_path, TARGET, manifest)
    row = types_payload(connection, target=TARGET, pattern="X", untyped=False, limit=0)[
        0
    ]
    assert row["byte_size"] == 8
    assert row["canonical"].startswith("typedef struct X")
    assert row["constraints"]
    fields = {field["name"]: field for field in row["fields"]}
    assert fields["values"]["byte_offset"] == 2
    assert fields["values"]["byte_width"] == 4


def test_anonymous_enumerators_and_diagnostics_are_indexed(tmp_path: Path) -> None:
    header = tmp_path / "include/private.h"
    header.parent.mkdir(parents=True)
    header.write_text("enum { VALUE_A = 1, VALUE_B = 2 };\n", encoding="utf-8")
    manifest = type(
        "M",
        (),
        {
            "headers": (Path("include/private.h"),),
            "sources": (),
            "support_sources": (),
            "has_explicit_sources": True,
        },
    )()
    connection = _connection()
    (tmp_path / "include/base/types.h").parent.mkdir(parents=True, exist_ok=True)
    (tmp_path / "include/base/types.h").write_text(
        "typedef unsigned int u32;\n", encoding="utf-8"
    )
    insert_authored_types(connection, tmp_path, TARGET, manifest)
    rows = types_payload(
        connection, target=TARGET, pattern="VALUE_", untyped=False, limit=0
    )
    assert {row["name"] for row in rows} == {"VALUE_A", "VALUE_B"}
    assert all(row["diagnostic"] == "unsupported declaration name" for row in rows)


def test_all_manifest_claimed_headers_populate_without_kind_cross_talk() -> None:
    root = Path(".").resolve()
    manifests = load_target_manifests(root)
    for target, manifest in manifests.items():
        connection = sqlite3.connect(":memory:")
        connection.row_factory = sqlite3.Row
        create_schema(connection)
        connection.execute(
            "INSERT INTO targets VALUES (?, 'b', 'h', 0, 'rizin', 'v', 's', 'sh')",
            (target,),
        )
        insert_authored_types(connection, root, target, manifest)
        connection.close()


def test_conflicting_same_name_declarations_are_diagnosed(tmp_path: Path) -> None:
    header = tmp_path / "include/private.h"
    header.parent.mkdir(parents=True)
    header.write_text(
        "typedef struct X { u8 a; } X;\ntypedef struct X { u16 a; } X;\n",
        encoding="utf-8",
    )
    manifest = type(
        "M",
        (),
        {
            "headers": (Path("include/private.h"),),
            "sources": (),
            "support_sources": (),
            "has_explicit_sources": True,
        },
    )()
    connection = _connection()
    with pytest.raises(sqlite3.IntegrityError):
        insert_authored_types(connection, tmp_path, TARGET, manifest)
