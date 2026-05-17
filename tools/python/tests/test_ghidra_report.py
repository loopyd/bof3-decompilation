from __future__ import annotations

import json
from pathlib import Path

from rebof3.ghidra_report import (
    build_duplicate_groups,
    context_gaps,
    function_report,
    queue_report,
    render_markdown,
)
from rebof3.paths import repo_layout


def write_json(path: Path, payload: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload), encoding="utf-8")


def write_fixture(layout_root: Path) -> None:
    layout = repo_layout(layout_root)
    binary = layout.emi_root / "TEST/MODULE/0.bin"
    binary.parent.mkdir(parents=True, exist_ok=True)
    binary.write_bytes(b"\x01\x02\x03\x04" + b"\x10\x20\x30\x40")
    write_json(
        binary.parent / "emi.json",
        {"entries": [{"index": 0, "name": "0.bin", "ram_ptr": 0x80100000}]},
    )
    rows = [
        {
            "body_min": "80100000",
            "body_max": "80100003",
            "entry": "80100000",
            "entry_hex": "0x80100000",
            "is_thunk": False,
            "name": "FUN_80100000",
            "name_source": "DEFAULT",
            "parameters": [{"name": "arg0", "data_type": "int", "storage": "r4"}],
            "locals": [{"name": "local_0", "data_type": "int", "storage": "Stack[0x0]"}],
            "program_path": "/bins/TEST/MODULE/0.bin",
            "signature": "undefined FUN_80100000(int arg0)",
            "source_hint": "output/extracted/TEST/MODULE.EMI#0",
        },
        {
            "body_min": "80100004",
            "body_max": "80100007",
            "entry": "80100004",
            "entry_hex": "0x80100004",
            "is_thunk": False,
            "name": "FUN_80100004",
            "name_source": "DEFAULT",
            "program_path": "/bins/TEST/MODULE/0.bin",
            "signature": "undefined FUN_80100004(void)",
            "source_hint": "output/extracted/TEST/MODULE.EMI#0",
        },
        {
            "body_min": "80100000",
            "body_max": "80100003",
            "entry": "80100000",
            "entry_hex": "0x80100000",
            "is_thunk": False,
            "name": "FUN_80100000",
            "name_source": "DEFAULT",
            "program_path": "/bins/TEST/MODULE_COPY/0.bin",
            "signature": "undefined FUN_80100000(void)",
            "source_hint": "output/extracted/TEST/MODULE_COPY.EMI#0",
        },
    ]
    copy_binary = layout.emi_root / "TEST/MODULE_COPY/0.bin"
    copy_binary.parent.mkdir(parents=True, exist_ok=True)
    copy_binary.write_bytes(binary.read_bytes())
    write_json(
        copy_binary.parent / "emi.json",
        {"entries": [{"index": 0, "name": "0.bin", "ram_ptr": 0x80100000}]},
    )
    write_json(
        layout.inventory_ghidra_function_index_path,
        {"schema": "rebof3-simple.inventory-ghidra-function-index/v1", "rows": rows},
    )
    write_json(
        layout.inventory_artifacts_dir / "raw_ghidra_export.json",
        {
            "rows": [
                {
                    "address": "80102000",
                    "kind": "symbol",
                    "name": "DAT_80102000",
                    "program_path": "/bins/TEST/MODULE/0.bin",
                },
                {
                    "from_address": "80100000",
                    "kind": "xref",
                    "program_path": "/bins/TEST/MODULE/0.bin",
                    "reference_type": "DATA",
                    "to_address": "80102000",
                },
                {
                    "from_address": "80100004",
                    "kind": "xref",
                    "program_path": "/bins/TEST/MODULE/0.bin",
                    "reference_type": "CALL",
                    "to_address": "80100000",
                },
            ]
        },
    )
    write_json(
        layout.out_dir / "source-status-full.json",
        {
            "modules": [
                {
                    "merged_function_statuses": [
                        {
                            "address": "0x80100000",
                            "source_hint": "output/extracted/TEST/MODULE.EMI#0",
                            "source": "src/modules/test/func_80100000.c",
                            "status": "lifted",
                        }
                    ]
                }
            ]
        },
    )


def test_function_report_uses_original_binary_and_ghidra_evidence(tmp_path: Path) -> None:
    write_fixture(tmp_path)
    layout = repo_layout(tmp_path)

    report = function_report(
        layout, "0x80100000", "output/extracted/TEST/MODULE.EMI#0"
    )

    assert report["size"] == 4
    assert report["file_offset"] == "0x00000000"
    assert report["source_status"] == "lifted"
    assert report["parameters"][0]["name"] == "arg0"
    assert report["locals"][0]["name"] == "local_0"
    assert report["refs"]["data_refs"][0]["symbol"] == "DAT_80102000"
    assert "FUN_80100000" in render_markdown(report)


def test_duplicate_groups_are_raw_original_body_hashes(tmp_path: Path) -> None:
    write_fixture(tmp_path)

    duplicates = build_duplicate_groups(repo_layout(tmp_path))

    assert duplicates["duplicate_group_count"] == 1
    group = duplicates["groups"][0]
    assert group["kind"] == "raw_body_hash"
    assert group["function_count"] == 2
    assert group["recommended_action"].startswith("prioritize matching")


def test_queue_report_prefers_existing_unmatched_sources(tmp_path: Path) -> None:
    write_fixture(tmp_path)

    queue = queue_report(repo_layout(tmp_path), limit=10)

    assert queue["candidate_count"] == 1
    assert queue["tasks"][0]["verify"] == (
        "bin/harness verify function bof3/src/modules/test/func_80100000.c"
    )


def test_context_gaps_classifies_internal_definitions(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    context = layout.bof3_dir / "include/bof3/context.h"
    context.parent.mkdir(parents=True)
    context.write_text("#define SHARED_VALUE 1\n", encoding="utf-8")
    internal = layout.bof3_dir / "src/modules/test/internal.h"
    internal.parent.mkdir(parents=True)
    internal.write_text("#define SHARED_VALUE 1\n#define LOCAL_VALUE 2\n", encoding="utf-8")

    report = context_gaps(layout)

    assert report["modules"][0]["status"] == "mixed"
    assert report["modules"][0]["missing_samples"] == ["LOCAL_VALUE"]
