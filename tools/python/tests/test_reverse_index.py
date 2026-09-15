from __future__ import annotations

import hashlib
import json
import sqlite3
from argparse import Namespace
from pathlib import Path

import pytest

from harness.domain.manifests import load_target_manifests
from harness.domain.layout import parse_splat_layout
from harness.domain.mips import data_references

from harness.analysis.engine import EngineIdentity
from harness.analysis.index import (
    SCHEMA_VERSION,
    connect,
    connect_status,
    index_path,
    rebuild,
)
from harness.analysis.schema import create_schema, required_schema
from harness.analysis import index_build
from harness.commands import index as index_command
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
    sdk = root / "config/sdk/psyq-slus.txt"
    sdk.parent.mkdir(parents=True, exist_ok=True)
    sdk.write_text("", encoding="utf-8")
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


_INDEX_FIXTURES: dict[str, bytes] = {}


def _seed_index(root: Path) -> Path:
    """Reuse immutable fixture index bytes; symlink cases still build live."""

    digest = hashlib.sha256()
    for path in sorted(root.rglob("*")):
        relative = path.relative_to(root)
        if relative.parts[:2] == ("out", "index"):
            continue
        if path.is_symlink():
            return rebuild(root)
        if path.is_file():
            digest.update(relative.as_posix().encode())
            digest.update(path.read_bytes())
    key = digest.hexdigest()
    output = index_path(root)
    if cached := _INDEX_FIXTURES.get(key):
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_bytes(cached)
        return output
    output = rebuild(root)
    _INDEX_FIXTURES[key] = output.read_bytes()
    return output


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


def test_selected_map_fingerprints_foreign_key_contract() -> None:
    connection = sqlite3.connect(":memory:")
    try:
        create_schema(connection)
        assert connection.execute("PRAGMA foreign_keys").fetchone() == (1,)
        assert connection.execute(
            "PRAGMA foreign_key_list(selected_map_fingerprints)"
        ).fetchall() == [
            (0, 0, "targets", "target_id", "id", "NO ACTION", "CASCADE", "NONE")
        ]
        assert "selected_map_fingerprints" in required_schema()
    finally:
        connection.close()


def test_selected_map_fingerprints_enforce_orphan_and_target_delete() -> None:
    connection = sqlite3.connect(":memory:")
    try:
        create_schema(connection)
        with pytest.raises(sqlite3.IntegrityError):
            connection.execute(
                "INSERT INTO selected_map_fingerprints VALUES (?, ?, ?)",
                (TARGET, "config/custom.txt", "a" * 64),
            )
        connection.execute(
            "INSERT INTO targets VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
            (
                TARGET,
                "binary",
                "a" * 64,
                0x80100000,
                "rizin",
                "test",
                "snapshot",
                "b" * 64,
            ),
        )
        connection.execute(
            "INSERT INTO selected_map_fingerprints VALUES (?, ?, ?)",
            (TARGET, "config/custom.txt", "c" * 64),
        )
        assert connection.execute("PRAGMA foreign_key_check").fetchall() == []
        connection.execute("DELETE FROM targets WHERE id = ?", (TARGET,))
        assert (
            connection.execute("SELECT * FROM selected_map_fingerprints").fetchall()
            == []
        )
    finally:
        connection.close()


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
    output = _seed_index(tmp_path)
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
    output = _seed_index(tmp_path)

    monkeypatch.setattr(
        index_build,
        "_validate_candidate",
        lambda _path, _targets, **_kwargs: (_ for _ in ()).throw(
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
    _seed_index(tmp_path)

    connection = connect(tmp_path)
    try:
        assert connection.row_factory is sqlite3.Row
    finally:
        connection.close()


@pytest.mark.parametrize("status", [False, True])
def test_connect_rejects_orphan_selected_map_fingerprint(
    tmp_path: Path, status: bool
) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    output = _seed_index(tmp_path)
    with sqlite3.connect(output) as connection:
        connection.execute("PRAGMA foreign_keys = OFF")
        connection.execute(
            "INSERT INTO selected_map_fingerprints VALUES (?, ?, ?)",
            ("missing/target", "config/custom.txt", "a" * 64),
        )
    opener = connect_status if status else connect
    with pytest.raises(ValueError, match="foreign key check failed"):
        opener(tmp_path)


def test_recover_atomically_repairs_orphan_selected_map_fingerprint(
    tmp_path: Path,
) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    output = _seed_index(tmp_path)
    with sqlite3.connect(output) as connection:
        connection.execute("PRAGMA foreign_keys = OFF")
        connection.execute(
            "INSERT INTO selected_map_fingerprints VALUES (?, ?, ?)",
            ("missing/target", "config/custom.txt", "a" * 64),
        )
    before = output.stat().st_ino

    assert index_command.run(Namespace(root=tmp_path, recover=True, timeout=5)) == 0

    assert output.stat().st_ino != before
    connection = connect(tmp_path)
    try:
        assert connection.execute("PRAGMA foreign_key_check").fetchall() == []
        assert (
            connection.execute(
                "SELECT COUNT(*) FROM selected_map_fingerprints WHERE target_id = ?",
                ("missing/target",),
            ).fetchone()[0]
            == 0
        )
    finally:
        connection.close()


def _replace_selected_map_table(path: Path, foreign_key: str) -> None:
    with sqlite3.connect(path) as connection:
        connection.execute("PRAGMA foreign_keys = OFF")
        connection.execute("ALTER TABLE selected_map_fingerprints RENAME TO old_maps")
        connection.execute(
            "CREATE TABLE selected_map_fingerprints ("
            f"target_id TEXT NOT NULL {foreign_key}, "
            "source_path TEXT NOT NULL, sha256 TEXT NOT NULL, "
            "PRIMARY KEY (target_id, source_path))"
        )
        connection.execute(
            "INSERT INTO selected_map_fingerprints SELECT * FROM old_maps"
        )
        connection.execute("DROP TABLE old_maps")


@pytest.mark.parametrize(
    "foreign_key",
    ["", "REFERENCES targets(id) ON DELETE NO ACTION"],
    ids=["missing", "wrong-delete-action"],
)
def test_connectors_reject_and_recover_repairs_foreign_key_schema(
    tmp_path: Path, foreign_key: str
) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    output = _seed_index(tmp_path)
    _replace_selected_map_table(output, foreign_key)
    before = output.stat().st_ino

    for opener in (connect, connect_status):
        with pytest.raises(ValueError, match="table schema mismatch"):
            opener(tmp_path)

    assert index_command.run(Namespace(root=tmp_path, recover=True, timeout=5)) == 0
    assert output.stat().st_ino != before
    with sqlite3.connect(output) as connection:
        assert connection.execute(
            "PRAGMA foreign_key_list(selected_map_fingerprints)"
        ).fetchall() == [
            (0, 0, "targets", "target_id", "id", "NO ACTION", "CASCADE", "NONE")
        ]


@pytest.mark.parametrize("table", ["targets", "functions", "type_input_fingerprints"])
def test_connect_rejects_missing_required_tables(tmp_path: Path, table: str) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    output = _seed_index(tmp_path)
    with sqlite3.connect(output) as connection:
        connection.execute(f'DROP TABLE "{table}"')
    with pytest.raises(ValueError, match="reverse index"):
        connect(tmp_path)


def test_connect_rejects_required_table_column_drift(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    output = _seed_index(tmp_path)
    with sqlite3.connect(output) as connection:
        connection.execute("ALTER TABLE functions RENAME TO old_functions")
        connection.execute("CREATE TABLE functions (id TEXT PRIMARY KEY)")
    with pytest.raises(ValueError, match="table schema mismatch: functions"):
        connect(tmp_path)


def test_connect_rejects_required_column_default_drift(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    output = _seed_index(tmp_path)
    with sqlite3.connect(output) as connection:
        connection.execute("PRAGMA writable_schema = ON")
        sql = connection.execute(
            "SELECT sql FROM sqlite_master WHERE type='table' AND name='functions'"
        ).fetchone()[0]
        connection.execute(
            "UPDATE sqlite_master SET sql = ? WHERE type='table' AND name='functions'",
            (sql.replace("DEFAULT 'unlifted'", "DEFAULT 'partial'", 1),),
        )
        connection.execute("PRAGMA writable_schema = OFF")
    with pytest.raises(ValueError, match="table schema mismatch: functions"):
        connect(tmp_path)


@pytest.mark.parametrize("status", [False, True])
def test_connect_modes_reject_schema_default_drift(
    tmp_path: Path, status: bool
) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    output = _seed_index(tmp_path)
    with sqlite3.connect(output) as connection:
        connection.execute("PRAGMA writable_schema = ON")
        sql = connection.execute(
            "SELECT sql FROM sqlite_master WHERE type='table' AND name='functions'"
        ).fetchone()[0]
        connection.execute(
            "UPDATE sqlite_master SET sql = ? WHERE type='table' AND name='functions'",
            (sql.replace("DEFAULT 'unlifted'", "DEFAULT 'partial'", 1),),
        )
        connection.execute("PRAGMA writable_schema = OFF")
    opener = connect_status if status else connect
    with pytest.raises(ValueError, match="table schema mismatch: functions"):
        opener(tmp_path)


def test_status_connect_rejects_input_drift(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    _seed_index(tmp_path)
    binary.write_bytes(binary.read_bytes() + b"drift")
    with pytest.raises(ValueError, match="stale reverse index binary"):
        connect_status(tmp_path)


@pytest.mark.parametrize("status", [False, True])
def test_connect_modes_reject_escaping_snapshot_symlink(
    tmp_path: Path, status: bool
) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    snapshot = snapshot_path(tmp_path, TARGET)
    _seed_index(tmp_path)
    outside = tmp_path.parent / f"{tmp_path.name}-snapshot.json"
    outside.write_bytes(snapshot.read_bytes())
    snapshot.unlink()
    snapshot.symlink_to(outside)
    opener = connect_status if status else connect
    with pytest.raises(
        ValueError, match="(?:unowned Rizin snapshot|repository input escapes root)"
    ):
        opener(tmp_path)


def test_connect_modes_allow_in_repo_snapshot_symlink(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    snapshot = snapshot_path(tmp_path, TARGET)
    alternate = snapshot.with_name("owned-snapshot.json")
    alternate.write_bytes(snapshot.read_bytes())
    snapshot.unlink()
    snapshot.symlink_to(alternate.name)
    _seed_index(tmp_path)
    connect(tmp_path).close()
    connect_status(tmp_path).close()


@pytest.mark.parametrize("kind", ["manifest", "binary", "splat"])
@pytest.mark.parametrize("status", [False, True])
def test_connect_modes_reject_escaping_registered_input_symlink(
    tmp_path: Path, kind: str, status: bool
) -> None:
    binary, manifest = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    _seed_index(tmp_path)
    paths = {
        "manifest": manifest,
        "binary": binary,
        "splat": tmp_path / "config/targets/emi/test/archive/00/splat.yaml",
    }
    path = paths[kind]
    outside = tmp_path.parent / f"{tmp_path.name}-{kind}{path.suffix}"
    outside.write_bytes(path.read_bytes())
    path.unlink()
    path.symlink_to(outside)
    opener = connect_status if status else connect
    with pytest.raises(ValueError, match="repository input escapes root"):
        opener(tmp_path)


@pytest.mark.parametrize("kind", ["manifest", "binary", "splat"])
def test_connect_modes_allow_in_repo_registered_input_symlink(
    tmp_path: Path, kind: str
) -> None:
    binary, manifest = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    paths = {
        "manifest": manifest,
        "binary": binary,
        "splat": tmp_path / "config/targets/emi/test/archive/00/splat.yaml",
    }
    path = paths[kind]
    alternate = (
        tmp_path / "owned-inputs" / kind / path.name
        if kind == "manifest"
        else path.with_name(f"owned-{path.name}")
    )
    alternate.parent.mkdir(parents=True, exist_ok=True)
    alternate.write_bytes(path.read_bytes())
    path.unlink()
    path.symlink_to(alternate if kind == "manifest" else alternate.name)
    _seed_index(tmp_path)
    connect(tmp_path).close()
    connect_status(tmp_path).close()


@pytest.mark.parametrize("kind", ["target-map", "sdk-map"])
@pytest.mark.parametrize("status", [False, True])
def test_connect_modes_reject_missing_required_map(
    tmp_path: Path, kind: str, status: bool
) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    _seed_index(tmp_path)
    paths = {
        "target-map": tmp_path / "config/targets/emi/test/archive/00/symbols.txt",
        "sdk-map": tmp_path / "config/sdk/psyq-slus.txt",
    }
    paths[kind].unlink()
    opener = connect_status if status else connect
    with pytest.raises((FileNotFoundError, ValueError)):
        opener(tmp_path)


@pytest.mark.parametrize("kind", ["target-map", "sdk-map"])
@pytest.mark.parametrize("status", [False, True])
def test_connect_modes_reject_escaping_required_map(
    tmp_path: Path, kind: str, status: bool
) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    _seed_index(tmp_path)
    paths = {
        "target-map": tmp_path / "config/targets/emi/test/archive/00/symbols.txt",
        "sdk-map": tmp_path / "config/sdk/psyq-slus.txt",
    }
    path = paths[kind]
    outside = tmp_path.parent / f"{tmp_path.name}-{kind}.txt"
    outside.write_bytes(path.read_bytes())
    path.unlink()
    path.symlink_to(outside)
    opener = connect_status if status else connect
    with pytest.raises(ValueError, match="repository input escapes root"):
        opener(tmp_path)


@pytest.mark.parametrize("kind", ["target-map", "sdk-map"])
def test_connect_modes_allow_in_repo_required_map_symlink(
    tmp_path: Path, kind: str
) -> None:
    binary, _ = _manifest(tmp_path)
    paths = {
        "target-map": tmp_path / "config/targets/emi/test/archive/00/symbols.txt",
        "sdk-map": tmp_path / "config/sdk/psyq-slus.txt",
    }
    path = paths[kind]
    alternate = path.with_name(f"owned-{path.name}")
    alternate.write_bytes(path.read_bytes())
    path.unlink()
    path.symlink_to(alternate.name)
    _snapshot(tmp_path, binary)
    _seed_index(tmp_path)
    connect(tmp_path).close()
    connect_status(tmp_path).close()


@pytest.mark.parametrize("status", [False, True])
def test_selected_shared_map_is_required(tmp_path: Path, status: bool) -> None:
    binary, _ = _manifest(tmp_path)
    splat = tmp_path / "config/targets/emi/test/archive/00/splat.yaml"
    splat.write_text(
        "options:\n  symbol_addrs_path:\n"
        "  - config/targets/shared/symbols.txt\n"
        "segments:\n  - [0, c, func_80100000]\n",
        encoding="utf-8",
    )
    shared = tmp_path / "config/targets/shared/symbols.txt"
    shared.parent.mkdir(parents=True)
    shared.write_text("", encoding="utf-8")
    _snapshot(tmp_path, binary)
    _seed_index(tmp_path)
    shared.unlink()
    opener = connect_status if status else connect
    with pytest.raises((FileNotFoundError, ValueError)):
        opener(tmp_path)


@pytest.mark.parametrize("mutation", ["delete", "escape", "drift"])
@pytest.mark.parametrize("status", [False, True])
def test_connect_modes_reject_selected_shared_map_drift(
    tmp_path: Path, mutation: str, status: bool
) -> None:
    binary, _ = _manifest(tmp_path)
    splat = tmp_path / "config/targets/emi/test/archive/00/splat.yaml"
    splat.write_text(
        "options:\n  symbol_addrs_path:\n"
        "  - config/targets/shared/symbols.txt\n"
        "segments:\n  - [0, c, func_80100000]\n",
        encoding="utf-8",
    )
    shared = tmp_path / "config/targets/shared/symbols.txt"
    shared.parent.mkdir(parents=True)
    shared.write_text("D_80100010 = 0x80100010;\n", encoding="utf-8")
    _snapshot(tmp_path, binary)
    _seed_index(tmp_path)
    if mutation == "delete":
        shared.unlink()
    elif mutation == "escape":
        outside = tmp_path.parent / f"{tmp_path.name}-shared.txt"
        outside.write_bytes(shared.read_bytes())
        shared.unlink()
        shared.symlink_to(outside)
    else:
        shared.write_text("D_80100014 = 0x80100014;\n", encoding="utf-8")
    opener = connect_status if status else connect
    with pytest.raises((FileNotFoundError, ValueError)):
        opener(tmp_path)


def test_connect_modes_allow_in_repo_selected_shared_map_symlink(
    tmp_path: Path,
) -> None:
    binary, _ = _manifest(tmp_path)
    splat = tmp_path / "config/targets/emi/test/archive/00/splat.yaml"
    splat.write_text(
        "options:\n  symbol_addrs_path:\n"
        "  - config/targets/shared/symbols.txt\n"
        "segments:\n  - [0, c, func_80100000]\n",
        encoding="utf-8",
    )
    shared = tmp_path / "config/targets/shared/symbols.txt"
    shared.parent.mkdir(parents=True)
    alternate = shared.with_name("owned-symbols.txt")
    alternate.write_text("", encoding="utf-8")
    shared.symlink_to(alternate.name)
    _snapshot(tmp_path, binary)
    _seed_index(tmp_path)
    connect(tmp_path).close()
    connect_status(tmp_path).close()


def test_unselected_shared_map_remains_optional(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    _seed_index(tmp_path)
    assert not (tmp_path / "config/targets/shared/symbols.txt").exists()
    connect(tmp_path).close()
    connect_status(tmp_path).close()


def test_real_battle15_splat_selects_owned_symbol_maps() -> None:
    root = Path(__file__).resolve().parents[3]
    layout = parse_splat_layout(
        root / "config/targets/emi/battle/battle/15/splat.yaml", 0x80096800
    )
    assert layout.symbol_map_paths == (
        "config/targets/shared/symbols.txt",
        "config/sdk/psyq-slus.txt",
        "config/targets/emi/battle/battle/15/symbols.txt",
    )


@pytest.mark.parametrize("status", [False, True])
@pytest.mark.parametrize("mutation", ["drift", "remove", "add", "path-change"])
def test_arbitrary_selected_map_set_and_content_are_fingerprinted(
    tmp_path: Path, status: bool, mutation: str
) -> None:
    binary, _ = _manifest(tmp_path)
    splat = tmp_path / "config/targets/emi/test/archive/00/splat.yaml"
    selected = tmp_path / "config/maps/arbitrary.txt"
    selected.parent.mkdir(parents=True)
    selected.write_text("D_80100010 = 0x80100010;\n", encoding="utf-8")

    def write_splat(paths: list[str]) -> None:
        import yaml

        splat.write_text(
            yaml.safe_dump(
                {
                    "options": {"symbol_addrs_path": paths},
                    "segments": [[0, "c", "func_80100000"]],
                }
            ),
            encoding="utf-8",
        )

    write_splat(["config/maps/arbitrary.txt"])
    _snapshot(tmp_path, binary)
    _seed_index(tmp_path)
    if mutation == "drift":
        selected.write_text("D_80100014 = 0x80100014;\n", encoding="utf-8")
    elif mutation == "remove":
        write_splat([])
    elif mutation == "add":
        extra = tmp_path / "config/maps/extra.txt"
        extra.write_text("", encoding="utf-8")
        write_splat(["config/maps/arbitrary.txt", "config/maps/extra.txt"])
    else:
        replacement = tmp_path / "config/maps/replacement.txt"
        replacement.write_bytes(selected.read_bytes())
        write_splat(["config/maps/replacement.txt"])
    opener = connect_status if status else connect
    with pytest.raises(ValueError):
        opener(tmp_path)


@pytest.mark.parametrize(
    "value",
    ["/tmp/symbols.txt", "../symbols.txt", ["config/symbols.txt", 3]],
)
def test_splat_rejects_invalid_symbol_map_paths(tmp_path: Path, value: object) -> None:
    splat = tmp_path / "splat.yaml"
    import yaml

    splat.write_text(
        yaml.safe_dump(
            {
                "options": {"symbol_addrs_path": value},
                "segments": [[0, "c", "func_80100000"]],
            }
        ),
        encoding="utf-8",
    )
    with pytest.raises(ValueError, match="invalid Splat symbol_addrs_path"):
        parse_splat_layout(splat, 0x80100000)


def test_connect_rejects_truncated_database(tmp_path: Path) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    output = _seed_index(tmp_path)
    content = output.read_bytes()
    output.write_bytes(content[: len(content) // 2])
    with pytest.raises(ValueError, match="invalid reverse index"):
        connect(tmp_path)


def test_connect_reuses_preloaded_manifests_without_weakening_freshness(
    tmp_path: Path, monkeypatch
) -> None:
    binary, _ = _manifest(tmp_path)
    _snapshot(tmp_path, binary)
    _seed_index(tmp_path)
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

    output = _seed_index(tmp_path)
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
        _seed_index(tmp_path)


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

    with sqlite3.connect(_seed_index(tmp_path)) as connection:
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
    with sqlite3.connect(_seed_index(tmp_path)) as connection:
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


@pytest.mark.parametrize("collision", [None, "prototype", "reviewed_range"])
def test_declared_globals_classify_symbols_without_cross_target_leakage(
    tmp_path: Path, collision: str | None
) -> None:
    from harness.analysis.rev_queries import variables_payload
    from harness.analysis.symbols import insert_symbols

    binary, _ = _manifest(tmp_path)
    symbols = tmp_path / f"config/targets/{TARGET}/symbols.txt"
    with symbols.open("a") as stream:
        stream.write("g_battle_work = 0x1F800044;\nunclassified = 0x1F800048;\n")
    _snapshot(tmp_path, binary)
    connection = sqlite3.connect(rebuild(tmp_path))
    connection.row_factory = sqlite3.Row
    connection.execute("DELETE FROM symbols")
    for name, kind, provenance in (
        ("g_battle_work", "global", "header_claim"),
        ("unclassified", "global", "shared_base"),
    ):
        connection.execute(
            "INSERT INTO type_usages VALUES (?, 'header.h', ?, NULL, 'u32', ?, NULL, ?, 'declaration')",
            (TARGET, name, kind, provenance),
        )
    if collision == "prototype":
        connection.execute(
            "INSERT INTO type_usages VALUES (?, 'other.h', 'g_battle_work', NULL, 'void()', 'prototype', NULL, 'header_claim', 'declaration')",
            (TARGET,),
        )
    elif collision:
        connection.execute(
            "INSERT INTO function_candidates VALUES (?, 0x1F800044, 0x1F800048, 'entry', ?, 'high', 0)",
            (TARGET, collision),
        )
    manifest = load_target_manifests(tmp_path)[TARGET]
    if collision:
        with pytest.raises(ValueError, match="conflicting function/global"):
            insert_symbols(connection, tmp_path, TARGET, manifest)
        return
    insert_symbols(connection, tmp_path, TARGET, manifest)
    assert variables_payload(connection, "g_battle_work", limit=0) == [
        {"target_id": TARGET, "address": "0x1F800044", "name": "g_battle_work"}
    ]
    assert variables_payload(connection, "D_80100010", limit=0)
    assert not variables_payload(connection, "unclassified", limit=0)
    connection.execute("DELETE FROM symbols")
    connection.execute("UPDATE type_usages SET target_id = '__shared__'")
    insert_symbols(connection, tmp_path, TARGET, manifest)
    assert not variables_payload(connection, "g_battle_work", limit=0)


def test_rebuild_indexes_declared_fixed_ram_global_and_rejects_stale_header(
    tmp_path: Path,
) -> None:
    from harness.analysis.rev_queries import variables_payload

    binary, config = _manifest(tmp_path)
    header = tmp_path / "include/private.h"
    header.parent.mkdir(parents=True)
    header.write_text(
        "extern unsigned char *volatile g_battle_work; /* @kind data */\n"
    )
    with config.open("a") as stream:
        stream.write("headers = ['include/private.h']\n")
    with (config.parent / "symbols.txt").open("a") as stream:
        stream.write("g_battle_work = 0x1F800044;\n")
    _snapshot(tmp_path, binary)
    inventories = []
    for _ in range(2):
        rebuild(tmp_path)
        with connect(tmp_path) as connection:
            inventories.append(variables_payload(connection, None, limit=0))
    assert (
        inventories[0]
        == inventories[1]
        == [
            {"target_id": TARGET, "address": "0x1F800044", "name": "g_battle_work"},
            {"target_id": TARGET, "address": "0x80100010", "name": "D_80100010"},
        ]
    )
    header.write_text("extern unsigned short g_battle_work;\n")
    with pytest.raises(ValueError, match="stale"):
        connect(tmp_path)


def test_data_references_indexed_base_and_materialized_pointer_ceiling() -> None:
    from harness.analysis.rev_queries import xrefs_payload
    from harness.domain.ids import parse_function_id

    # Original pop-block shape at 800A540C; append uses the same dynamic base.
    words = [
        0x3C038014,  # lui v1,0x8014
        0x246363C7,  # addiu v1,v1,0x63c7
        0x90620000,  # lbu v0,0(v1): pointer chaining is unsupported
        0x00000000,
        0x1040001D,
        0x2442FFFF,
        0xA0620000,  # sb v0,0(v1): not a separate indexed count store
        0x304200FF,
        0x3C018014,
        0x00220821,  # addu at,at,v0: unknown dynamic index
        0x902563C4,  # lbu a1,0x63c4(at): not definite 801463C4
        0xA02263C4,  # sb v0,0x63c4(at): same ceiling
    ]
    refs = data_references(b"".join(word.to_bytes(4, "little") for word in words))
    assert refs == [(4, 0x801463C7, "address", "addiu")]
    with sqlite3.connect(":memory:") as connection:
        connection.row_factory = sqlite3.Row
        create_schema(connection)
        connection.execute(
            "INSERT INTO targets VALUES (?, '', '', 0, '', '', '', '')", (TARGET,)
        )
        connection.execute(
            "INSERT INTO functions (id, target_id, address, size, name, "
            "analyzer_sha256, reviewed, lifted, instruction_count) "
            "VALUES (?, ?, ?, ?, '', '', 0, 0, ?)",
            (f"{TARGET}@800a5330", TARGET, 0x800A5330, 1260, 315),
        )
        connection.executemany(
            "INSERT INTO data_references VALUES (?, ?, ?, ?, ?, ?, ?)",
            [
                (TARGET, f"{TARGET}@800a5330", 0x800A540C + site, addr, None, kind, op)
                for site, addr, kind, op in refs
            ],
        )
        assert (
            xrefs_payload(connection, parse_function_id(f"{TARGET}@801463C4"), limit=0)
            == []
        )
        count = xrefs_payload(
            connection, parse_function_id(f"{TARGET}@801463C7"), limit=0
        )
        assert [(row["source"], row["kind"]) for row in count] == [
            ("0x800A5410", "address")
        ]
        assert (
            xrefs_payload(
                connection, parse_function_id("emi/test/archive/01@801463C7"), limit=0
            )
            == []
        )
