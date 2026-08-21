from __future__ import annotations

import hashlib
import json
import sqlite3
from pathlib import Path

import pytest

from harness.domain import load_target_manifests
from harness.domain.mips import data_references

from harness.analysis.engine import EngineIdentity
from harness.analysis.index import SCHEMA_VERSION, connect, index_path, rebuild
from harness.analysis.schema import create_schema
from harness.analysis import index_build
from harness.analysis.project import prepare_target, replay_commands, rizin_argv, status
from harness.analysis.snapshot import (
    SNAPSHOT_SCHEMA,
    SnapshotFunction,
    AnalysisSnapshot,
    snapshot_path,
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
        "kind = 'emi'\nload_address = 0x80100000\nsource_dir = 'src/emi/test/archive/00'\n"
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


def _base_types(root: Path) -> None:
    path = root / "include/base/types.h"
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists():
        path.write_text(
            "typedef unsigned char bool;\ntypedef signed char s8;\ntypedef signed short s16;\n"
            "typedef signed int s32;\ntypedef signed long long s64;\ntypedef unsigned char u8;\n"
            "typedef unsigned short u16;\ntypedef unsigned int u32;\ntypedef unsigned long long u64;\n"
            "typedef float f32;\ntypedef double f64;\n",
            encoding="utf-8",
        )


def _snapshot(root: Path, binary: Path) -> None:
    _base_types(root)
    snapshot = AnalysisSnapshot(
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
    write_snapshot(snapshot, snapshot_path(root, TARGET))


def test_rizin_replay_fingerprint_includes_claim_identity(
    tmp_path: Path,
) -> None:
    """Explicit claims participate in the replay fingerprint without altering
    the commands Rizin actually executes."""

    _binary, config = _manifest(tmp_path)
    config.write_text(
        config.read_text()
        + 'sources = ["src/bof3/io/load.c"]\n'
        + 'support_sources = ["src/bof3/io/symbols.c"]\n'
        + 'headers = ["src/bof3/io/private.h"]\n',
        encoding="utf-8",
    )
    for relative in (
        "src/bof3/io/load.c",
        "src/bof3/io/symbols.c",
        "src/bof3/io/private.h",
    ):
        path = tmp_path / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("/* placeholder */\n", encoding="utf-8")

    before = prepare_target(tmp_path, TARGET)
    assert "# claim src/bof3/io/load.c" in before.replay
    assert "# claim src/bof3/io/symbols.c" in before.replay
    assert "# claim src/bof3/io/private.h" in before.replay
    commands = replay_commands(before.replay)
    assert not any("claim" in line for line in commands)

    config.write_text(
        config.read_text().replace('sources = ["src/bof3/io/load.c"]\n', ""),
        encoding="utf-8",
    )
    after = prepare_target(tmp_path, TARGET)
    assert after.replay_sha256 != before.replay_sha256


def test_project_recipe_is_target_qualified_and_read_only(tmp_path: Path) -> None:
    _manifest(tmp_path)
    project = prepare_target(tmp_path, TARGET)
    assert "afn func_80100000 0x80100000" in project.replay
    assert "f D_80100010 4 @ 0x80100010" in project.replay
    assert project.binary_offset == 0
    assert project.replay_sha256 == hashlib.sha256(project.replay.encode()).hexdigest()
    assert status(tmp_path, TARGET)["fresh"] is False
    assert not (tmp_path / "out/rizin").exists()


def test_rizin_argv_sets_endianness_and_appends_bounded_commands(
    tmp_path: Path,
) -> None:
    from harness.analysis.engine import EngineIdentity
    from harness.analysis.project import rizin_argv

    _manifest(tmp_path)
    project = prepare_target(tmp_path, TARGET)
    engine = EngineIdentity("rizin", tmp_path / "rizin", "1.0", {})

    argv = rizin_argv(project, engine, commands=("pd 4 @ 0x80100000",), quiet=True)

    assert argv[argv.index("-E") + 1] == "little"
    assert not any("cfg.bigendian" in arg for arg in argv)
    assert "-q" in argv
    assert argv[-3:] == ["-c", "pd 4 @ 0x80100000", str(project.binary)]


def test_psx_exe_project_maps_payload_after_header(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    data = bytearray(0x820)
    data[:8] = b"PS-X EXE"
    data[0x18:0x20] = (0x80100000).to_bytes(4, "little") + (0x20).to_bytes(4, "little")
    data[0x800:0x808] = b"\x08\x00\xe0\x03\x00\x00\x00\x00"
    binary.write_bytes(data)

    project = prepare_target(tmp_path, TARGET)
    argv = rizin_argv(project, EngineIdentity("rizin", tmp_path / "rizin", "1.0", {}))

    assert project.binary_offset == 0x800
    assert project.load_address == 0x80100000
    assert "# binary_offset 0x800" in project.replay
    assert argv[argv.index("-m") + 1] == "0x800FF800"


def test_psx_exe_project_rejects_header_manifest_mismatch(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    data = bytearray(0x820)
    data[:8] = b"PS-X EXE"
    data[0x18:0x20] = (0x80101000).to_bytes(4, "little") + (0x20).to_bytes(4, "little")
    binary.write_bytes(data)

    with pytest.raises(ValueError, match="t_addr"):
        prepare_target(tmp_path, TARGET)


def test_status_rejects_pre_jal_snapshot_schema(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    snapshot = snapshot_path(tmp_path, TARGET)
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


def test_rebuild_preserves_old_index_when_candidate_validation_fails(
    tmp_path: Path, monkeypatch
) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    output = rebuild(tmp_path)

    monkeypatch.setattr(
        index_build,
        "_validate_candidate",
        lambda _path, _targets: (_ for _ in ()).throw(
            ValueError("integrity check failed")
        ),
    )
    with pytest.raises(ValueError, match="integrity check failed"):
        rebuild(tmp_path)

    with sqlite3.connect(output) as connection:
        assert connection.execute("SELECT COUNT(*) FROM functions").fetchone()[0] == 1


def test_candidate_validation_rejects_missing_target_coverage(tmp_path: Path) -> None:
    candidate = tmp_path / "candidate.sqlite"
    with sqlite3.connect(candidate) as connection:
        create_schema(connection)
        connection.execute(
            "INSERT INTO metadata VALUES (?, ?)", ("schema", SCHEMA_VERSION)
        )
    with pytest.raises(ValueError, match="target coverage"):
        index_build._validate_candidate(candidate, {TARGET})


def test_connect_accepts_matching_type_input_digest_with_row_factory(
    tmp_path: Path,
) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    rebuild(tmp_path)

    connection = connect(tmp_path)
    try:
        assert connection.row_factory is sqlite3.Row
    finally:
        connection.close()


def test_connect_reuses_preloaded_manifests_without_weakening_freshness(
    tmp_path: Path, monkeypatch
) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    rebuild(tmp_path)
    manifests = load_target_manifests(tmp_path)
    monkeypatch.setattr(
        "harness.analysis.index.load_target_manifests",
        lambda *_: (_ for _ in ()).throw(AssertionError("manifests reloaded")),
    )
    connection = connect(tmp_path, manifests=manifests)
    connection.close()
    binary.write_bytes(b"changed")
    with pytest.raises(ValueError, match="stale reverse index binary"):
        connect(tmp_path, manifests=manifests)


def test_rebuild_type_rows_are_deterministic_across_two_builds(tmp_path: Path) -> None:
    binary, config = _manifest(tmp_path)
    header = tmp_path / "include/private.h"
    header.parent.mkdir(parents=True, exist_ok=True)
    header.write_text(
        "typedef struct X { u32 value; } X;\nASSERT_SIZE(X, 4);\n", encoding="utf-8"
    )
    config.write_text(
        config.read_text() + 'headers = ["include/private.h"]\n', encoding="utf-8"
    )
    _snapshot(tmp_path, binary)

    output = rebuild(tmp_path)
    with sqlite3.connect(output) as connection:
        first = connection.execute(
            "SELECT target_id, name, kind, canonical, byte_size, diagnostic FROM type_declarations ORDER BY 1, 2, 3, 4"
        ).fetchall()
    rebuild(tmp_path)
    with sqlite3.connect(output) as connection:
        second = connection.execute(
            "SELECT target_id, name, kind, canonical, byte_size, diagnostic FROM type_declarations ORDER BY 1, 2, 3, 4"
        ).fetchall()
    assert first == second
    assert (
        TARGET,
        "X",
        "struct",
        "typedef struct X { u32 value;} X;",
        4,
        None,
    ) in first
    assert any(row[0] == "__shared__" and row[1] == "u32" for row in first)


def test_rebuild_fails_on_missing_shared_type_input(tmp_path: Path) -> None:
    binary, _config = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    (tmp_path / "include/base/types.h").unlink()
    with pytest.raises(ValueError, match="missing claimed type input"):
        rebuild(tmp_path)


def test_rebuild_reads_psx_exe_function_bytes_from_payload(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    words = [
        (0x0F << 26) | (8 << 16) | 0x8010,
        (0x23 << 26) | (8 << 21) | (2 << 16) | 0x0010,
        0,
        0,
    ]
    payload = b"".join(word.to_bytes(4, "little") for word in words) + b"\0" * 16
    data = bytearray(0x800) + bytearray(payload)
    data[:8] = b"PS-X EXE"
    data[0x18:0x20] = (0x80100000).to_bytes(4, "little") + len(payload).to_bytes(
        4, "little"
    )
    binary.write_bytes(data)
    _snapshot(tmp_path, binary)

    with sqlite3.connect(rebuild(tmp_path)) as connection:
        assert connection.execute(
            "SELECT source, address, access_kind, opcode FROM data_references"
        ).fetchall() == [(0x80100004, 0x80100010, "load", "lw")]


def test_analyzer_candidates_exclude_reviewed_duplicate_groups(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    _base_types(tmp_path)
    (tmp_path / "config/targets/emi/test/archive/00/splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [16, c, func_80100010]\n  - [32]\n",
        encoding="utf-8",
    )
    (tmp_path / "config/targets/emi/test/archive/00/symbols.txt").write_text(
        "func_80100000 = 0x80100000;\nfunc_80100010 = 0x80100010;\n",
        encoding="utf-8",
    )
    snapshot = AnalysisSnapshot(
        schema=SNAPSHOT_SCHEMA,
        target=TARGET,
        engine={"name": "rizin", "version": "test"},
        inputs={
            "binary_sha256": hashlib.sha256(binary.read_bytes()).hexdigest(),
            "replay_sha256": prepare_target(tmp_path, TARGET).replay_sha256,
        },
        functions=tuple(
            SnapshotFunction(
                id=f"{TARGET}@{address:08x}",
                address=address,
                analyzer_size=16,
                analyzer_name=f"func_{address:08X}",
                exact_sha256="a" * 64,
            )
            for address in (0x80100000, 0x80100010)
        ),
        calls=(),
        unresolved_calls=(),
    )
    write_snapshot(snapshot, snapshot_path(tmp_path, TARGET))
    with sqlite3.connect(rebuild(tmp_path)) as connection:
        assert (
            connection.execute("SELECT COUNT(*) FROM duplicate_groups").fetchone()[0]
            == 1
        )
        assert (
            connection.execute(
                "SELECT COUNT(*) FROM unconfirmed_candidates"
            ).fetchone()[0]
            == 0
        )


def test_data_references_decodes_lui_lo_pairs() -> None:
    # lui t0, 0x8014 ; lw v0, -4(t0) ; addiu t0, t0, 8 ; ori v1, t0, 0x1234
    words = [
        (0x0F << 26) | (8 << 16) | 0x8014,
        (0x23 << 26) | (8 << 21) | (2 << 16) | 0xFFFC,
        (0x09 << 26) | (8 << 21) | (8 << 16) | 0x0008,
        (0x0D << 26) | (8 << 21) | (3 << 16) | 0x1234,
    ]
    data = b"".join(w.to_bytes(4, "little") for w in words)
    refs = data_references(data)
    assert refs == [
        (4, 0x8013FFFC, "load", "lw"),
        (8, 0x80140008, "address", "addiu"),
    ]
