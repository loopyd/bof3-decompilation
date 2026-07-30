from __future__ import annotations

import hashlib
import json
import sqlite3
from pathlib import Path

import pytest

from harness.reverse_index import index_path, rebuild
from harness.rizin_project import prepare_target, status
from harness.snapshot import (
    SNAPSHOT_SCHEMA,
    SnapshotFunction,
    TargetSnapshot,
    write_snapshot,
)


TARGET = "emi/test/archive/00"


def _manifest(root: Path) -> tuple[Path, Path]:
    binary = root / "out/binaries/emi/test/archive/00.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"\0" * 32)
    config = root / "config/targets/emi/test/archive/00/target.toml"
    config.parent.mkdir(parents=True)
    config.write_text(
        "schema = 'harness.target/v2'\n"
        f"id = '{TARGET}'\n"
        "kind = 'emi'\nsource_dir = 'src/emi/test/archive/00'\n"
        "binary = 'out/binaries/emi/test/archive/00.bin'\n"
        "splat = 'config/targets/emi/test/archive/00/splat.yaml'\n",
        encoding="utf-8",
    )
    splat = root / "config/targets/emi/test/archive/00/splat.yaml"
    splat.parent.mkdir(parents=True, exist_ok=True)
    splat.write_text("segments:\n  - [0, c, func_80100000]\n", encoding="utf-8")
    symbols = root / "config/targets/emi/test/archive/00/symbols.txt"
    symbols.parent.mkdir(parents=True, exist_ok=True)
    symbols.write_text(
        "func_80100000 = 0x80100000;\nD_80100010 = 0x80100010;\n", encoding="utf-8"
    )
    return binary, config


def _snapshot(root: Path, binary: Path) -> None:
    snapshot = TargetSnapshot(
        schema=SNAPSHOT_SCHEMA,
        target=TARGET,
        engine={"name": "rizin", "version": "test"},
        inputs={
            "binary_sha256": hashlib.sha256(binary.read_bytes()).hexdigest(),
            "replay_sha256": prepare_target(root, TARGET).replay_sha256,
        },
        functions=(
            SnapshotFunction(
                id=f"{TARGET}@80100000",
                address=0x80100000,
                analyzer_size=16,
                analyzer_name="func_80100000",
                exact_sha256="a" * 64,
            ),
        ),
        calls=(),
        unresolved_calls=(),
    )
    write_snapshot(snapshot, root / "out/reverse/emi/test/archive/00/snapshot.json")


def test_project_recipe_is_target_qualified_and_read_only(tmp_path: Path) -> None:
    _manifest(tmp_path)
    project = prepare_target(tmp_path, TARGET)
    assert "afn func_80100000 0x80100000" in project.replay
    assert "f D_80100010 4 @ 0x80100010" in project.replay
    assert project.replay_sha256 == hashlib.sha256(project.replay.encode()).hexdigest()
    assert status(tmp_path, TARGET)["fresh"] is False
    assert not (tmp_path / "out/rizin").exists()


def test_status_rejects_pre_jal_snapshot_schema(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    snapshot = tmp_path / "out/reverse/emi/test/archive/00/snapshot.json"
    assert status(tmp_path, TARGET)["fresh"] is True

    payload = json.loads(snapshot.read_text(encoding="utf-8"))
    payload["schema"] = "bof3.analysis-snapshot/v2"
    snapshot.write_text(json.dumps(payload), encoding="utf-8")

    assert status(tmp_path, TARGET)["fresh"] is False


def test_rebuild_is_atomic_when_a_snapshot_is_stale(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    output = rebuild(tmp_path)
    assert output == index_path(tmp_path)
    with sqlite3.connect(output) as connection:
        assert connection.execute("SELECT COUNT(*) FROM functions").fetchone()[0] == 1

    binary.write_bytes(b"\1" * 32)
    with pytest.raises(ValueError, match="stale Rizin snapshot bytes"):
        rebuild(tmp_path)
    with sqlite3.connect(output) as connection:
        assert connection.execute("SELECT COUNT(*) FROM functions").fetchone()[0] == 1
