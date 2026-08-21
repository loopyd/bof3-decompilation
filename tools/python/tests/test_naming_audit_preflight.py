from __future__ import annotations

import json
import time
from pathlib import Path

import pytest

import harness.commands.naming_audit as naming_audit
from harness.commands.naming_audit import initialize, initialize_all, prepare, validate


def _repo(root: Path) -> Path:
    config = root / "config/targets/exe/test/target.toml"
    config.parent.mkdir(parents=True)
    config.write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src/test'\nbinary='out/test.bin'\nload_address=0x80100000\n"
        "splat='config/targets/exe/test/splat.yaml'\n"
        "sources=['src/test/func_80100000.c']\n",
        encoding="utf-8",
    )
    (config.parent / "splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [8]\n", encoding="utf-8"
    )
    (config.parent / "symbols.txt").write_text(
        "func_80100000 = 0x80100000;\n", encoding="utf-8"
    )
    binary = root / "out/test.bin"
    binary.parent.mkdir(parents=True)
    binary.write_bytes(b"\0" * 8)
    source = root / "src/test/func_80100000.c"
    source.parent.mkdir(parents=True)
    source.write_text(
        "/* @source 0x80100000\n * @behavior UNKNOWN: test\n"
        " * @status exact\n * @match 100.00\n */\nvoid func_80100000(void) {}\n",
        encoding="utf-8",
    )
    return source


def test_initialize_accounts_for_every_row_as_explicit_evidence_gap(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(
        "harness.commands.naming_audit.project_status", lambda *_: {"fresh": True}
    )
    monkeypatch.setattr(
        "harness.commands.naming_audit.connect_index",
        lambda *_, **__: type(
            "Connection",
            (),
            {"execute": lambda self, *_: [], "close": lambda self: None},
        )(),
    )
    monkeypatch.setattr("harness.analysis.naming.required_work_items", lambda *_: [])
    report = initialize(tmp_path, "exe/test")
    assert report["complete"] is False
    assert {(row["kind"], row["name"]) for row in report["rows"]} == {
        ("function", "func_80100000")
    }
    assert report["rows"][0]["rung_status"] == "blocked"
    assert all(rung["status"] == "open" for rung in report["rows"][0]["rungs"].values())
    assert validate(tmp_path, "exe/test", report)["complete"] is False


def test_initialize_keeps_malformed_progress_as_blocked_not_crashing(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(
        "harness.commands.naming_audit.project_status", lambda *_: {"fresh": True}
    )
    monkeypatch.setattr(
        "harness.commands.naming_audit.connect_index",
        lambda *_, **__: type(
            "Connection",
            (),
            {"execute": lambda self, *_: [], "close": lambda self: None},
        )(),
    )
    monkeypatch.setattr("harness.analysis.naming.required_work_items", lambda *_: [])
    report = initialize(tmp_path, "exe/test")
    assert report["rows"][0]["rung_status"] == "blocked"
    assert validate(tmp_path, "exe/test", report)["complete"] is False


def test_initialize_zero_rows_is_complete_and_validates_index(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    (tmp_path / "config/targets/exe/test/symbols.txt").write_text("", encoding="utf-8")
    opened: list[Path] = []
    monkeypatch.setattr(
        "harness.commands.naming_audit.project_status", lambda *_: {"fresh": True}
    )

    class Connection:
        def execute(self, *_args) -> list[object]:
            return []

        def close(self) -> None:
            opened.append(tmp_path)

    monkeypatch.setattr(
        "harness.commands.naming_audit.connect_index", lambda *_, **__: Connection()
    )
    report = initialize(tmp_path, "exe/test")
    assert report["rows"] == []
    assert report["complete"] is True
    assert opened == [tmp_path]
    assert validate(tmp_path, "exe/test", report)["complete"] is True


def test_initialize_uses_one_connection_and_three_snapshot_queries(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    calls = {"connect": 0, "execute": 0}

    class Connection:
        def execute(self, *_args) -> list[object]:
            calls["execute"] += 1
            return []

        def close(self) -> None:
            pass

    def connect(*_args, **_kwargs) -> Connection:
        calls["connect"] += 1
        return Connection()

    monkeypatch.setattr(
        "harness.commands.naming_audit.project_status", lambda *_: {"fresh": True}
    )
    monkeypatch.setattr("harness.commands.naming_audit.connect_index", connect)
    initialize(tmp_path, "exe/test")
    assert calls == {"connect": 1, "execute": 3}


def test_initialize_all_accounts_for_zero_and_nonzero_targets(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    other = tmp_path / "config/targets/exe/empty"
    other.mkdir(parents=True)
    (other / "target.toml").write_text(
        "schema='harness.target/v2'\nid='exe/empty'\nkind='executable'\n"
        "source_dir='src/empty'\nbinary='out/empty.bin'\nload_address=0x80200000\n"
        "splat='config/targets/exe/empty/splat.yaml'\nsources=[]\n",
        encoding="utf-8",
    )
    (other / "splat.yaml").write_text("segments:\n  - [0]\n", encoding="utf-8")
    (other / "symbols.txt").write_text("", encoding="utf-8")
    (tmp_path / "out/empty.bin").write_bytes(b"")

    calls = {"connect": 0, "execute": 0}

    class Connection:
        def execute(self, *_args) -> list[object]:
            calls["execute"] += 1
            return []

        def close(self) -> None:
            pass

    def connect(*_args, **_kwargs) -> Connection:
        calls["connect"] += 1
        return Connection()

    monkeypatch.setattr(
        "harness.commands.naming_audit.project_status", lambda *_: {"fresh": True}
    )
    monkeypatch.setattr("harness.commands.naming_audit.connect_index", connect)
    monkeypatch.setattr("harness.analysis.naming.required_work_items", lambda *_: [])
    summary = initialize_all(tmp_path, tmp_path / "out/audit")
    assert summary["target_count"] == 2
    assert summary["row_count"] == 1
    assert {item["target"]: item["rows"] for item in summary["targets"]} == {
        "exe/empty": 0,
        "exe/test": 1,
    }
    assert (tmp_path / "out/audit/summary.json").is_file()
    assert calls == {"connect": 1, "execute": 6}


def test_initialize_all_loads_manifests_and_inventory_once(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    calls = {"connect": 0, "execute": 0, "manifests": 0, "inventory": 0}

    class Connection:
        def execute(self, *_args) -> list[object]:
            calls["execute"] += 1
            return []

        def close(self) -> None:
            pass

    original_manifests = naming_audit.load_target_manifests

    def manifests(root: Path):
        calls["manifests"] += 1
        return original_manifests(root)

    def connect(*_args, **_kwargs) -> Connection:
        calls["connect"] += 1
        return Connection()

    import harness.commands.naming_audit_bulk as bulk

    original_inventory = bulk.collect_naming_debt

    def inventory(root: Path, loaded):
        calls["inventory"] += 1
        return original_inventory(root, loaded)

    monkeypatch.setattr(naming_audit, "load_target_manifests", manifests)
    monkeypatch.setattr(naming_audit, "connect_index", connect)
    monkeypatch.setattr(bulk, "collect_naming_debt", inventory)
    initialize_all(tmp_path, tmp_path / "out/audit")
    assert calls == {"connect": 1, "execute": 3, "manifests": 1, "inventory": 1}


def test_initialize_all_synthetic_targets_has_bounded_shared_work(
    tmp_path: Path, monkeypatch
) -> None:
    target_count = 40
    for index in range(target_count):
        target = f"exe/test{index:02d}"
        config = tmp_path / f"config/targets/{target}"
        source = tmp_path / f"src/test{index:02d}/func_80{index:06X}.c"
        binary = tmp_path / f"out/test{index:02d}.bin"
        config.mkdir(parents=True)
        source.parent.mkdir(parents=True)
        binary.parent.mkdir(parents=True, exist_ok=True)
        (config / "target.toml").write_text(
            f"schema='harness.target/v2'\nid='{target}'\nkind='executable'\n"
            f"source_dir='src/test{index:02d}'\nbinary='out/test{index:02d}.bin'\n"
            f"load_address=0x80{index:06X}\n"
            f"splat='config/targets/{target}/splat.yaml'\n"
            f"sources=['src/test{index:02d}/{source.name}']\n",
            encoding="utf-8",
        )
        (config / "splat.yaml").write_text(
            f"segments:\n  - [0, c, func_80{index:06X}]\n  - [8]\n",
            encoding="utf-8",
        )
        (config / "symbols.txt").write_text(
            f"func_80{index:06X} = 0x80{index:06X};\n", encoding="utf-8"
        )
        source.write_text(
            f"/* @source 0x80{index:06X}\n * @behavior UNKNOWN: test\n"
            " * @status exact\n * @match 100.00\n * @residual none\n */\n"
            f"void func_80{index:06X}(void) {{}}\n",
            encoding="utf-8",
        )
        binary.write_bytes(b"\0" * 8)

    calls = {"connect": 0, "execute": 0}

    class Connection:
        def execute(self, *_args) -> list[object]:
            calls["execute"] += 1
            return []

        def close(self) -> None:
            pass

    def connect(*_args, **_kwargs) -> Connection:
        calls["connect"] += 1
        return Connection()

    monkeypatch.setattr("harness.commands.naming_audit.connect_index", connect)
    start = time.perf_counter()
    summary = initialize_all(tmp_path, tmp_path / "out/audit")
    assert time.perf_counter() - start < 2.0
    assert summary["target_count"] == target_count
    assert summary["row_count"] == target_count
    assert calls == {"connect": 1, "execute": 3 * target_count}


def test_initialize_all_failure_preserves_previous_report_set(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    output = tmp_path / "out/audit"
    output.mkdir(parents=True)
    prior = {"schema": "prior", "target_count": 1}
    (output / "summary.json").write_text(json.dumps(prior), encoding="utf-8")
    (output / "prior.json").write_text("prior\n", encoding="utf-8")

    class Connection:
        def execute(self, *_args) -> list[object]:
            return []

        def close(self) -> None:
            pass

    monkeypatch.setattr(
        "harness.commands.naming_audit.connect_index", lambda *_, **__: Connection()
    )
    monkeypatch.setattr(
        "harness.commands.naming_audit.validate_v3",
        lambda *_args, **_kwargs: (_ for _ in ()).throw(
            ValueError("synthetic failure")
        ),
    )
    with pytest.raises(ValueError, match="synthetic failure"):
        initialize_all(tmp_path, output)
    assert json.loads((output / "summary.json").read_text()) == prior
    assert (output / "prior.json").read_text() == "prior\n"
    assert sorted(path.name for path in output.iterdir()) == [
        "prior.json",
        "summary.json",
    ]


def test_initialize_all_success_replaces_previous_report_set(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    output = tmp_path / "out/audit"
    output.mkdir(parents=True)
    (output / "stale.json").write_text("stale\n", encoding="utf-8")

    class Connection:
        def execute(self, *_args) -> list[object]:
            return []

        def close(self) -> None:
            pass

    monkeypatch.setattr(
        "harness.commands.naming_audit.connect_index", lambda *_, **__: Connection()
    )
    initialize_all(tmp_path, output)
    assert not (output / "stale.json").exists()
    assert (output / "exe__test.json").is_file()
    assert json.loads((output / "summary.json").read_text())["target_count"] == 1


def test_prepare_classifies_safe_exact_metadata_repair(tmp_path: Path) -> None:
    _repo(tmp_path)
    result = prepare(tmp_path, "exe/test")
    assert result["ready"] is False
    assert result["findings"][0]["class"] == "safe_metadata_repair"


def test_prepare_repairs_only_after_live_exact_proof(
    tmp_path: Path, monkeypatch
) -> None:
    source = _repo(tmp_path)
    monkeypatch.setattr(
        "harness.commands.naming_audit._live_exact",
        lambda *_: (True, ["asm-diff: exit 0", "byte-match: exit 0"]),
    )
    result = prepare(tmp_path, "exe/test", repair=True)
    assert result["ready"] is True
    assert "@residual none" in source.read_text(encoding="utf-8")


def test_prepare_does_not_repair_failed_live_proof(tmp_path: Path, monkeypatch) -> None:
    source = _repo(tmp_path)
    before = source.read_text(encoding="utf-8")
    monkeypatch.setattr(
        "harness.commands.naming_audit._live_exact",
        lambda *_: (False, ["byte-match: exit 1"]),
    )
    result = prepare(tmp_path, "exe/test", repair=True)
    assert result["ready"] is False
    assert source.read_text(encoding="utf-8") == before
