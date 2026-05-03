from __future__ import annotations

import json
from pathlib import Path

from rebof3.harness.binary import build_binary_diff, resolve_binary_pair
from rebof3.harness.catalog import emi_target_records, symbolic_emi_catalog
from rebof3.harness.classify import (
    EMI_AUDIO_SEQ,
    EMI_BINARY_RAM,
    EMI_IMAGE_VRAM,
    EMI_UNKNOWN,
    emi_kind,
)
from rebof3.harness.cli import binary_diff_exit_code
from rebof3.harness.config import load_harness_config
from rebof3.harness.context import build_context_header
from rebof3.harness.dashboard import SECTIONS, render_dashboard
from rebof3.harness.maps import build_binary_map
from rebof3.harness.report import build_report, choose_resume_action
from rebof3.harness.state import (
    acquire_lock,
    claim_target,
    finish_target,
    list_targets,
    lock_row,
    release_lock,
    state_db,
    upsert_targets,
    record_binary_map,
)
from rebof3.harness.tasks import source_function_payload, source_function_target_records
from rebof3.harness.tools import tool_health
from rebof3.harness.workspace import initialize_target_workspace


def test_harness_config_loads_repo_defaults() -> None:
    config = load_harness_config()

    assert config.schema == "rebof3-simple.harness/v1"
    assert config.database.name == "harness.sqlite3"
    assert config.commands["catalog"] == "bin/inventory-build"
    assert config.raw_ghidra_export.name == "raw_ghidra_export.json"
    assert [target.id for target in config.migration_targets] == [
        "slus_004_22",
        "logo_exe",
        "battle_03",
    ]


def test_emi_kind_mapping_uses_symbolic_names() -> None:
    assert emi_kind(0, ram_ptr=0x801D0C00, size=0x1000) == EMI_BINARY_RAM
    assert emi_kind(3, ram_ptr=0x1C080200, size=0x8000) == EMI_IMAGE_VRAM
    assert emi_kind(10, ram_ptr=1, size=0x100) == EMI_AUDIO_SEQ
    assert emi_kind(9, ram_ptr=0, size=0) == EMI_UNKNOWN


def test_symbolic_catalog_fixture_records_raw_type_and_explanation() -> None:
    catalog = symbolic_emi_catalog(
        {
            "archive_count": 1,
            "entries": [
                {
                    "archive_id": "BATTLE/BATTLE",
                    "entry_index": 3,
                    "ram_ptr": 0x801D0C00,
                    "ram_ptr_hex": "0x801d0c00",
                    "size": 0x1000,
                    "type": 0,
                }
            ],
        }
    )

    entry = catalog["entries"][0]
    records = emi_target_records(catalog)

    assert entry["raw_type"] == 0
    assert entry["emi_kind"] == EMI_BINARY_RAM
    assert "CPU RAM" in entry["classification"]["explanation"]
    assert records[0]["id"] == "emi:BATTLE/BATTLE#3"
    assert records[0]["status"] == "queued"


def test_state_claim_and_finish_lifecycle(tmp_path: Path) -> None:
    db_path = tmp_path / "harness.sqlite3"

    with state_db(db_path) as conn:
        upsert_targets(
            conn,
            [
                {
                    "id": "emi:BATTLE/BATTLE#3",
                    "type": "emi",
                    "status": "queued",
                    "priority": 10,
                    "summary": "battle entry",
                    "payload": {},
                }
            ],
        )
        claimed = claim_target(conn, owner="worker-a", lease_minutes=5)
        assert claimed is not None
        assert claimed["id"] == "emi:BATTLE/BATTLE#3"
        finish_target(
            conn,
            target_id="emi:BATTLE/BATTLE#3",
            status="done",
            message="matched",
        )

    with state_db(db_path) as conn:
        report = build_report(load_harness_config(), conn)
        assert report["counts_by_status"] == {"done": 1}
        assert report["active_claims"] == []


def test_state_filters_type_before_limit(tmp_path: Path) -> None:
    db_path = tmp_path / "harness.sqlite3"

    with state_db(db_path) as conn:
        upsert_targets(
            conn,
            [
                {
                    "id": "emi:one",
                    "type": "emi",
                    "status": "queued",
                    "priority": 1,
                    "summary": "emi one",
                    "payload": {},
                },
                {
                    "id": "emi:two",
                    "type": "emi",
                    "status": "queued",
                    "priority": 2,
                    "summary": "emi two",
                    "payload": {},
                },
                {
                    "id": "migration:battle_03",
                    "type": "migration",
                    "status": "queued",
                    "priority": 50,
                    "summary": "battle migration",
                    "payload": {},
                },
            ],
        )

        rows = list_targets(conn, limit=1, target_type="migration")

    assert [row["id"] for row in rows] == ["migration:battle_03"]


def test_claim_filters_type(tmp_path: Path) -> None:
    db_path = tmp_path / "harness.sqlite3"

    with state_db(db_path) as conn:
        upsert_targets(
            conn,
            [
                {
                    "id": "emi:one",
                    "type": "emi",
                    "status": "queued",
                    "priority": 1,
                    "summary": "emi one",
                    "payload": {},
                },
                {
                    "id": "migration:battle_03",
                    "type": "migration",
                    "status": "queued",
                    "priority": 50,
                    "summary": "battle migration",
                    "payload": {},
                },
            ],
        )

        claimed = claim_target(conn, owner="worker-a", target_type="migration")

    assert claimed is not None
    assert claimed["id"] == "migration:battle_03"


def test_lock_lifecycle(tmp_path: Path) -> None:
    db_path = tmp_path / "harness.sqlite3"

    with state_db(db_path) as conn:
        assert acquire_lock(conn, name="ghidra", owner="worker-a")
        assert not acquire_lock(conn, name="ghidra", owner="worker-b")
        assert lock_row(conn, "ghidra")["owner"] == "worker-a"
        release_lock(conn, name="ghidra", owner="worker-a")
        assert lock_row(conn, "ghidra") is None


def test_upsert_refreshes_open_status_but_preserves_closed_status(
    tmp_path: Path,
) -> None:
    db_path = tmp_path / "harness.sqlite3"

    with state_db(db_path) as conn:
        upsert_targets(
            conn,
            [
                {
                    "id": "artifact:battle",
                    "type": "artifact",
                    "status": "queued",
                    "priority": 80,
                    "summary": "battle placeholder",
                    "payload": {},
                },
                {
                    "id": "artifact:done",
                    "type": "artifact",
                    "status": "queued",
                    "priority": 80,
                    "summary": "done artifact",
                    "payload": {},
                },
            ],
        )
        finish_target(conn, target_id="artifact:done", status="done", message="closed")
        upsert_targets(
            conn,
            [
                {
                    "id": "artifact:battle",
                    "type": "artifact",
                    "status": "ready",
                    "priority": 80,
                    "summary": "battle archive",
                    "payload": {},
                },
                {
                    "id": "artifact:done",
                    "type": "artifact",
                    "status": "ready",
                    "priority": 80,
                    "summary": "done artifact refreshed",
                    "payload": {},
                },
            ],
        )
        rows = {row["id"]: row for row in list_targets(conn, limit=10)}

    assert rows["artifact:battle"]["status"] == "ready"
    assert rows["artifact:done"]["status"] == "done"


def test_resume_selects_catalog_then_claim(tmp_path: Path) -> None:
    db_path = tmp_path / "harness.sqlite3"
    config = load_harness_config()
    config = config.__class__(
        **{
            **config.__dict__,
            "database": db_path,
            "out_dir": tmp_path,
            "workspace_dir": tmp_path / "workspaces",
            "context_dir": tmp_path / "context",
            "dashboard_dir": tmp_path / "dashboard",
        }
    )

    with state_db(db_path) as conn:
        assert choose_resume_action(config, conn)["action"] == "catalog"
        upsert_targets(
            conn,
            [
                {
                    "id": "func:/boot/SLUS_004.22@0x80162d00",
                    "type": "function",
                    "status": "queued",
                    "priority": 20,
                    "summary": "emi_ready",
                    "payload": {},
                }
            ],
        )
        action = choose_resume_action(config, conn)

    assert action["action"] == "claim"
    assert action["target_id"] == "func:/boot/SLUS_004.22@0x80162d00"


def test_source_function_records_support_fast_iteration_without_ghidra(
    tmp_path: Path,
) -> None:
    source = tmp_path / "bof3/src/modules/battle/03/func_801d0c20.c"
    source.parent.mkdir(parents=True)
    source.write_text("/* @source: 0x801d0c20 FUN_801d0c20 */\n", encoding="utf-8")
    config = load_harness_config()
    config = config.__class__(**{**config.__dict__, "root": tmp_path})

    records = source_function_target_records(config)

    assert records == [
        {
            "id": "func-src:src/modules/battle/03/func_801d0c20.c",
            "type": "function",
            "status": "queued",
            "priority": 25,
            "summary": "src/modules/battle/03/func_801d0c20.c 0x801d0c20",
            "source_hint": "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
            "program_path": "/bins/BIN/BATTLE/BATTLE/03.bin",
            "entry_hex": "0x801d0c20",
            "payload": {
                "source_path": "bof3/src/modules/battle/03/func_801d0c20.c",
                "binary_path": str(tmp_path / "out/emi_raw/BIN/BATTLE/BATTLE/3.bin"),
                "load_address": 0x801D0C00,
            },
        }
    ]


def test_source_function_payload_infers_battle_overlay_binary() -> None:
    config = load_harness_config()
    payload = source_function_payload(
        config, config.root / "bof3/src/modules/battle/03/func_801ddf28.c"
    )

    assert payload["binary_path"].endswith("out/emi_raw/BIN/BATTLE/BATTLE/3.bin")
    assert payload["load_address"] == 0x801D0C00


def test_source_function_payload_infers_battle_15_binary_and_size(
    tmp_path: Path,
) -> None:
    source_path = tmp_path / "bof3/src/modules/battle/15/func_8009c868.c"
    source_path.parent.mkdir(parents=True)
    source_path.write_text(
        "/* @source: 0x8009c868 FUN_8009c868 */\n"
        "u8 func_8009c868(volatile u8* entry, s32 bit_index) { return 0; }\n",
        encoding="utf-8",
    )
    raw_export = tmp_path / "out/inventory/raw_ghidra_export.json"
    raw_export.parent.mkdir(parents=True)
    raw_export.write_text(
        json.dumps(
            {
                "rows": [
                    {
                        "kind": "function",
                        "program_path": "/bins/BATTLE/BATTLE/15.bin",
                        "address": "8009c868",
                        "body_min": "8009c868",
                        "body_max": "8009c87b",
                    }
                ]
            }
        ),
        encoding="utf-8",
    )
    config = load_harness_config()
    config = config.__class__(
        **{
            **config.__dict__,
            "root": tmp_path,
            "raw_ghidra_export": raw_export,
        }
    )

    payload = source_function_payload(config, source_path)

    assert payload["binary_path"] == str(
        tmp_path / "out/emi_raw/BIN/BATTLE/BATTLE/15.bin"
    )
    assert payload["load_address"] == 0x80096800
    assert payload["size"] == 0x14


def test_context_build_writes_module_local_stubs(tmp_path: Path) -> None:
    config = load_harness_config()
    config = config.__class__(
        **{
            **config.__dict__,
            "root": tmp_path,
            "context_dir": tmp_path / "out/harness/context",
            "workspace_dir": tmp_path / "out/harness/workspaces",
        }
    )
    target = {
        "id": "func-src:src/core/emi/func_80162178.c",
        "entry_hex": "0x80162178",
        "program_path": "/boot/SLUS_004.22",
        "source_hint": "build/extracted/SLUS_004.22",
        "summary": "source function",
        "type": "function",
    }

    workspace = initialize_target_workspace(config, target)
    context = build_context_header(config, target)
    text = context.read_text(encoding="utf-8")

    assert workspace.is_file()
    assert '#include "symbols.h"' in text
    assert (context.parent / "structs.h").is_file()
    assert (context.parent / "globals.h").is_file()
    assert (context.parent / "prototypes.h").is_file()


def test_dashboard_includes_required_sections() -> None:
    html = render_dashboard(
        {
            "database": "out/harness/harness.sqlite3",
            "counts_by_status": {"queued": 2},
            "counts_by_type": {"emi": 1, "function": 1},
            "next_targets": [],
            "active_claims": [],
            "blockers": [],
            "tool_health": [],
        }
    )

    for section in SECTIONS:
        assert section.title() in html


def test_tool_health_reports_decomp_and_binary_tools() -> None:
    names = {item.name for item in tool_health(load_harness_config())}

    assert {"ghidra", "m2c", "maspsx", "objdiff-cli", "psn00b-objdump"} <= names


def test_binary_diff_reports_missing_compiled_bin(tmp_path: Path) -> None:
    original = tmp_path / "out/emi_raw/BIN/BATTLE/BATTLE/3.bin"
    original.parent.mkdir(parents=True)
    original.write_bytes(b"\x00\x00\x00\x00")
    artifact_manifest = tmp_path / "build/default/artifacts/metadata/artifacts.json"
    artifact_manifest.parent.mkdir(parents=True)
    artifact_manifest.write_text(
        json.dumps(
            {
                "artifacts": [
                    {
                        "target": "bof3_battle_03_raw",
                        "kind": "module",
                        "program_path": "/bins/BIN/BATTLE/BATTLE/03.bin",
                        "build_stage": "raw",
                        "source_hint": "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                        "placeholder": False,
                    }
                ]
            }
        ),
        encoding="utf-8",
    )
    config = load_harness_config()
    config = config.__class__(
        **{
            **config.__dict__,
            "root": tmp_path,
            "artifact_manifest": artifact_manifest,
        }
    )
    target = {
        "id": "emi:BATTLE/BATTLE#3",
        "source_hint": "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
        "payload": {"payload_path": str(original), "ram_ptr": 0x801D0C00},
    }

    pair = resolve_binary_pair(config, target)
    payload, report_path = build_binary_diff(
        config, target, output_root=tmp_path / "diff"
    )

    assert pair.original == original
    assert (
        pair.compiled
        == tmp_path / "build/default/artifacts/raw/BIN/BATTLE/BATTLE/03.bin"
    )
    assert payload["status"] == "missing_compiled_bin"
    assert payload["compiled_bin"] == str(
        tmp_path / "build/default/artifacts/raw/BIN/BATTLE/BATTLE/03.bin"
    )
    assert report_path.is_file()


def test_binary_diff_allow_different_keeps_mismatches_non_blocking() -> None:
    assert binary_diff_exit_code(["different"], allow_different=True) == 0
    assert (
        binary_diff_exit_code(["exact_match", "different"], allow_different=True) == 0
    )
    assert binary_diff_exit_code(["missing_compiled_bin"], allow_different=True) == 1
    assert binary_diff_exit_code(["different"], allow_different=False) == 1


def test_binary_map_collects_functions_symbols_and_xrefs(tmp_path: Path) -> None:
    function_index = tmp_path / "out/inventory/ghidra_function_index.json"
    function_index.parent.mkdir(parents=True)
    function_index.write_text(
        json.dumps(
            {
                "rows": [
                    {
                        "entry": "801d0c00",
                        "entry_hex": "0x801d0c00",
                        "name": "entry",
                        "program_path": "/bins/BIN/BATTLE/BATTLE/03.bin",
                        "signature": "void entry(void)",
                        "source_hint": "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                    }
                ]
            }
        ),
        encoding="utf-8",
    )
    raw_export = tmp_path / "out/inventory/raw_ghidra_export.json"
    raw_export.write_text(
        json.dumps(
            {
                "rows": [
                    {
                        "address": "0x801d1000",
                        "kind": "data",
                        "name": "global_1000",
                        "program_path": "/bins/BATTLE/BATTLE/3.bin",
                    },
                    {
                        "from_address": "0x801d0c10",
                        "kind": "xref",
                        "name": "call",
                        "program_path": "/bins/BATTLE/BATTLE/3.bin",
                        "to_address": "0x801d1000",
                    },
                ]
            }
        ),
        encoding="utf-8",
    )
    raw_metadata = (
        tmp_path / "build/default/artifacts/raw/BIN/BATTLE/BATTLE/03.bin.json"
    )
    raw_metadata.parent.mkdir(parents=True)
    raw_metadata.write_text(
        json.dumps(
            {
                "placements": [
                    {
                        "address": "0x801d0c00",
                        "object": "func_801d0c00.c.obj",
                        "offset": 0,
                        "original_size": 16,
                        "size": 16,
                        "truncated": False,
                    },
                    {
                        "address": "0x801d0c20",
                        "object": "func_801d0c20.c.obj",
                        "offset": 0x20,
                        "original_size": 12,
                        "size": 12,
                        "truncated": False,
                    },
                ]
            }
        ),
        encoding="utf-8",
    )
    artifact_manifest = tmp_path / "build/default/artifacts/metadata/artifacts.json"
    artifact_manifest.parent.mkdir(parents=True)
    artifact_manifest.write_text(
        json.dumps(
            {
                "artifacts": [
                    {
                        "target": "bof3_battle_03_raw",
                        "kind": "module",
                        "program_path": "/bins/BIN/BATTLE/BATTLE/03.bin",
                        "build_stage": "raw",
                        "source_hint": "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
                        "placeholder": False,
                    }
                ]
            }
        ),
        encoding="utf-8",
    )
    config = load_harness_config()
    config = config.__class__(
        **{
            **config.__dict__,
            "root": tmp_path,
            "function_index": function_index,
            "emi_catalog": tmp_path / "out/inventory/emi_catalog.json",
            "raw_ghidra_export": raw_export,
            "artifact_manifest": artifact_manifest,
        }
    )
    target = {
        "id": "emi:BATTLE/BATTLE#3",
        "source_hint": "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
        "payload": {"archive_id": "BATTLE/BATTLE", "entry_name": "3.bin"},
    }

    payload = build_binary_map(config, target)

    assert payload["function_count"] == 2
    assert payload["functions"][1]["source"] == "raw-module-metadata"
    assert payload["symbol_count"] == 1
    assert payload["xref_count"] == 1


def test_record_binary_map_populates_symbol_and_xref_tables(tmp_path: Path) -> None:
    db_path = tmp_path / "harness.sqlite3"
    payload = {
        "target_id": "emi:BATTLE/BATTLE#3",
        "original_bin": "out/emi_raw/BIN/BATTLE/BATTLE/3.bin",
        "compiled_bin": "build/default/artifacts/raw/BIN/BATTLE/BATTLE/03.bin",
        "source_hint": "build/extracted/BIN/BATTLE/BATTLE.EMI#3",
        "functions": [{"kind": "function", "name": "entry", "entry_hex": "0x1"}],
        "symbols": [{"kind": "data", "name": "global", "address": "0x2"}],
        "xrefs": [
            {
                "kind": "xref",
                "from_address": "0x1",
                "to_address": "0x2",
                "name": "load",
            }
        ],
    }

    with state_db(db_path) as conn:
        upsert_targets(
            conn,
            [
                {
                    "id": "emi:BATTLE/BATTLE#3",
                    "type": "emi",
                    "status": "queued",
                    "priority": 10,
                    "summary": "battle entry",
                    "payload": {},
                }
            ],
        )
        record_binary_map(conn, payload)
        symbol_count = conn.execute("SELECT COUNT(*) FROM symbols").fetchone()[0]
        xref_count = conn.execute("SELECT COUNT(*) FROM xrefs").fetchone()[0]

    assert symbol_count == 2
    assert xref_count == 1
