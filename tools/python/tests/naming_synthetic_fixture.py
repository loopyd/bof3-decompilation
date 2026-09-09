"""Production-collected synthetic exact-capability test fixture."""

from __future__ import annotations

import hashlib
import json
import shutil
import sqlite3
from pathlib import Path
from types import MappingProxyType, SimpleNamespace

import pytest
from harness.analysis.index import index_path, rebuild
from harness.analysis.project import prepare_target
from harness.analysis.snapshot import (
    SNAPSHOT_SCHEMA,
    AnalysisSnapshot,
    SnapshotFunction,
    snapshot_path,
    write_snapshot,
)
from harness.naming.audit import initialize
from harness.naming.capabilities import ExactCapabilitySpec
from harness.naming.consumers import TableConsumerSpec
from harness.naming.execution import run_evidence
from harness.naming.namespace import journal_dir

SYNTHETIC_TARGET = "exe/test"
SYNTHETIC_ROW = "D_80100080"
SYNTHETIC_CONSUMER = 0x80100000
SYNTHETIC_WORDS = (
    0x27BDFFD8,
    0xAFBF0020,
    0x3C058010,
    0x24A50080,
    0x8CA20000,
    0x8CA30004,
    0x8CA40008,
    0xAFA20010,
    0xAFA30014,
    0xAFA40018,
    0x3C02800F,
    0x3C031F80,
    0x8C630044,
    0x34420800,
    0x3C018014,
    0xAC2259F0,
    0x90620001,
    0,
    0x00021080,
    0x03A21021,
    0x8C420010,
    0,
    0x0040F809,
    0,
    0x3C02800D,
    0x34423800,
    0x3C018014,
    0xAC2259F0,
    0x8FBF0020,
    0x27BD0028,
    0x03E00008,
    0,
)


def _synthetic_report_row() -> dict:
    gap = {
        "status": "open",
        "next_command": "bin/rev-query --json xrefs exe/test@0x80100080",
        "observations": [{"id": "gap", "text": "Evidence Gap: synthetic fixture"}],
        "authority": "target manifest, reviewed Splat, original image",
    }
    return {
        "kind": "data",
        "name": SYNTHETIC_ROW,
        "initializer_state": "bof3.naming-audit-initializer/v1",
        "rung_status": "blocked",
        "outside_payload": False,
        "partial_used": False,
        "rungs": {
            name: dict(gap)
            for name in (
                "selected_range",
                "selected_access",
                "storage_class",
                "one_level_beyond",
            )
        },
        "required_work": [
            {
                "id": "access:exe/test@80100000",
                "status": "open",
                "profile": "data_access",
                "description": "fixture access",
            }
        ],
        "optional_work": [],
        "interpretation": "No semantic name is accepted.",
        "authority": "target manifest, reviewed Splat, original image",
        "smallest_repair": (
            "bin/rev-query --json xrefs exe/test@0x80100080; "
            "bin/rz-project query exe/test -c 'axt @ 0x80100080'"
        ),
        "missing_fact": "consumer",
        "ceiling_next_command": "",
    }


def _build_synthetic_exact_evidence(tmp_path: Path):
    config = tmp_path / "config/targets/exe/test"
    config.mkdir(parents=True)
    code = b"".join(word.to_bytes(4, "little") for word in SYNTHETIC_WORDS)
    table = b"".join(
        pointer.to_bytes(4, "little")
        for pointer in (0x8010008C, 0x80100098, 0x801000A0)
    )
    image = bytearray(0xA8)
    image[: len(code)] = code
    image[0x80:0x8C] = table
    binary = tmp_path / "out/test.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(image)
    (config / "target.toml").write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src/test'\nbinary='out/test.bin'\nload_address=0x80100000\n"
        "splat='config/targets/exe/test/splat.yaml'\nsources=[]\n"
    )
    (config / "symbols.txt").write_text(
        "func_80100000 = 0x80100000;\nD_80100080 = 0x80100080;\n"
        "func_8010008C = 0x8010008C;\nfunc_80100098 = 0x80100098;\nfunc_801000A0 = 0x801000A0;\n"
    )
    (config / "splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [0x80, data, D_80100080]\n"
        "  - [0x8c, c, func_8010008C]\n  - [0x98, c, func_80100098]\n"
        "  - [0xa0, c, func_801000A0]\n  - [0xa8]\n"
    )
    sdk = tmp_path / "config/sdk/psyq-slus.txt"
    sdk.parent.mkdir(parents=True)
    sdk.write_text("")
    base = tmp_path / "include/base/types.h"
    base.parent.mkdir(parents=True)
    base.write_text("typedef unsigned int u32;\n")
    digest = hashlib.sha256(image).hexdigest()
    functions = (
        SnapshotFunction(
            "exe/test@80100000", 0x80100000, 0x80, "func_80100000", "a" * 64
        ),
        SnapshotFunction(
            "exe/test@8010008C", 0x8010008C, 12, "func_8010008C", "b" * 64
        ),
        SnapshotFunction("exe/test@80100098", 0x80100098, 8, "func_80100098", "c" * 64),
        SnapshotFunction("exe/test@801000A0", 0x801000A0, 8, "func_801000A0", "d" * 64),
    )
    write_snapshot(
        AnalysisSnapshot(
            SNAPSHOT_SCHEMA,
            SYNTHETIC_TARGET,
            {"name": "rizin", "version": "test"},
            {
                "binary_sha256": digest,
                "replay_sha256": prepare_target(
                    tmp_path, SYNTHETIC_TARGET
                ).replay_sha256,
            },
            functions,
            (),
            (),
        ),
        snapshot_path(tmp_path, SYNTHETIC_TARGET),
    )
    rebuild(tmp_path)
    with sqlite3.connect(index_path(tmp_path)) as connection:
        connection.execute(
            "DELETE FROM data_references WHERE target_id = ?", (SYNTHETIC_TARGET,)
        )
        connection.execute(
            "INSERT INTO data_references VALUES (?, ?, ?, ?, ?, ?, ?)",
            (
                SYNTHETIC_TARGET,
                "exe/test@80100000",
                0x80100010,
                0x80100080,
                SYNTHETIC_ROW,
                "address",
                "addiu",
            ),
        )
    spec = TableConsumerSpec(
        SYNTHETIC_TARGET,
        f"data:{SYNTHETIC_ROW}",
        0x80100080,
        0x8010008C,
        SYNTHETIC_CONSUMER,
        0x80100080,
        digest,
        table,
        SYNTHETIC_WORDS,
        (0x8010008C, 0x80100098, 0x801000A0),
        (4, 5, 6),
    )
    registry = MappingProxyType(
        {
            (f"data:{SYNTHETIC_ROW}", "exe/test@80100000"): ExactCapabilitySpec(
                spec, "access:exe/test@80100000"
            )
        }
    )
    report = tmp_path / "out/reviews/synthetic.json"
    report.parent.mkdir(parents=True)
    initialized = initialize(tmp_path, SYNTHETIC_TARGET)
    initialized["rows"] = [
        _synthetic_report_row()
        if row.get("kind") == "data" and row.get("name") == SYNTHETIC_ROW
        else row
        for row in initialized["rows"]
    ]
    report.write_text(json.dumps(initialized, indent=2, sort_keys=True) + "\n")
    result = run_evidence(
        tmp_path,
        SYNTHETIC_TARGET,
        report,
        rows=[f"data:{SYNTHETIC_ROW}"],
        registry=registry,
    )
    assert result["telemetry_summary"]["rows"] == {
        "planned": 1,
        "completed": 1,
        "skipped": 0,
        "stale": 0,
    }
    namespace = journal_dir(tmp_path, report, SYNTHETIC_TARGET)
    derived = json.loads((namespace / SYNTHETIC_ROW / "derived.json").read_text())
    assert derived["conclusion_enabled"] is True
    assert derived["open"] == []
    assert derived["unavailable"] == []
    assert derived["conclusion_capability"]["row"] == f"data:{SYNTHETIC_ROW}"
    return SimpleNamespace(
        root=tmp_path, report=report, registry=registry, result=result
    )


@pytest.fixture(scope="session")
def synthetic_exact_evidence_baseline(tmp_path_factory: pytest.TempPathFactory):
    return _build_synthetic_exact_evidence(tmp_path_factory.mktemp("synthetic-exact"))


@pytest.fixture
def synthetic_exact_evidence(tmp_path: Path, synthetic_exact_evidence_baseline):
    """Clone one session-built exact fixture so mutation tests stay isolated."""

    baseline = synthetic_exact_evidence_baseline
    shutil.copytree(baseline.root, tmp_path, dirs_exist_ok=True)
    return SimpleNamespace(
        root=tmp_path,
        report=tmp_path / baseline.report.relative_to(baseline.root),
        registry=baseline.registry,
        result=baseline.result,
    )
