from __future__ import annotations

import argparse
import json
import time
from pathlib import Path

import harness.naming.audit as naming_audit
import pytest
from harness.context.bof3_cleanup import parse_cleanup_request
from harness.naming.audit import (
    initialize,
    initialize_all,
    prepare,
    validate,
)
from harness.naming.campaign import (
    CAMPAIGN_ACCOUNT_SCHEMA,
    campaign_report_filename,
    resolve_campaign_report,
)
from harness.naming.preparation import RollbackError


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
        "harness.naming.audit.project_status", lambda *_: {"fresh": True}
    )
    monkeypatch.setattr(
        "harness.naming.audit.connect_index",
        lambda *_, **__: type(
            "Connection",
            (),
            {"execute": lambda self, *_: [], "close": lambda self: None},
        )(),
    )
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
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
        "harness.naming.audit.project_status", lambda *_: {"fresh": True}
    )
    monkeypatch.setattr(
        "harness.naming.audit.connect_index",
        lambda *_, **__: type(
            "Connection",
            (),
            {"execute": lambda self, *_: [], "close": lambda self: None},
        )(),
    )
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
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
        "harness.naming.audit.project_status", lambda *_: {"fresh": True}
    )

    class Connection:
        def execute(self, *_args) -> list[object]:
            return []

        def close(self) -> None:
            opened.append(tmp_path)

    monkeypatch.setattr(
        "harness.naming.audit.connect_index", lambda *_, **__: Connection()
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
        "harness.naming.audit.project_status", lambda *_: {"fresh": True}
    )
    monkeypatch.setattr("harness.naming.audit.connect_index", connect)
    initialize(tmp_path, "exe/test")
    assert calls == {"connect": 1, "execute": 3}


def test_campaign_report_resolver_is_manifest_owned(tmp_path: Path) -> None:
    output = tmp_path / "out/reviews/plan-audit-naming"
    output.mkdir(parents=True)
    report = output / "exe__test.json"
    report.write_text("{}\n")
    (output / "summary.json").write_text(
        json.dumps(
            {
                "schema": CAMPAIGN_ACCOUNT_SCHEMA,
                "targets": [{"target": "exe/test", "report": report.as_posix()}],
            }
        )
    )
    duplicate = tmp_path / "out/reviews/other/exe__test.json"
    duplicate.parent.mkdir(parents=True)
    duplicate.write_text("{}\n")
    assert (
        campaign_report_filename("emi/world00/area030/04")
        == "emi__world00__area030__04.json"
    )
    assert resolve_campaign_report(tmp_path, "exe/test") == report

    summary = json.loads((output / "summary.json").read_text())
    summary["targets"].append(dict(summary["targets"][0]))
    (output / "summary.json").write_text(json.dumps(summary))
    with pytest.raises(ValueError, match="ambiguous.*init-all"):
        resolve_campaign_report(tmp_path, "exe/test")


@pytest.mark.parametrize("link_name", ["plan-audit-naming", "exe__test.json"])
def test_campaign_report_resolver_rejects_symlinked_path_components(
    tmp_path: Path, link_name: str
) -> None:
    _repo(tmp_path)
    campaign = tmp_path / "out/reviews/plan-audit-naming"
    real = tmp_path / "out/reviews/real-campaign"
    real.mkdir(parents=True)
    report = real / "exe__test.json"
    report.write_text("{}\n")
    (real / "summary.json").write_text(
        json.dumps(
            {
                "schema": CAMPAIGN_ACCOUNT_SCHEMA,
                "targets": [{"target": "exe/test", "report": report.as_posix()}],
            }
        )
    )
    if link_name == "plan-audit-naming":
        campaign.parent.mkdir(parents=True, exist_ok=True)
        campaign.symlink_to(real, target_is_directory=True)
    else:
        campaign.mkdir(parents=True)
        (campaign / "summary.json").write_text(
            json.dumps(
                {
                    "schema": CAMPAIGN_ACCOUNT_SCHEMA,
                    "targets": [
                        {
                            "target": "exe/test",
                            "report": (campaign / link_name).as_posix(),
                        }
                    ],
                }
            )
        )
        (campaign / link_name).symlink_to(report)

    with pytest.raises(ValueError, match="symlink.*init-all"):
        resolve_campaign_report(tmp_path, "exe/test")
    with pytest.raises(ValueError, match="symlink.*init-all"):
        parse_cleanup_request(("audit-target", "exe/test"), root=tmp_path)


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
        "harness.naming.audit.project_status", lambda *_: {"fresh": True}
    )
    monkeypatch.setattr("harness.naming.audit.connect_index", connect)
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])
    summary = initialize_all(tmp_path, tmp_path / "out/audit")
    assert summary["target_count"] == 2
    assert summary["row_count"] == 1
    assert {item["target"]: item["rows"] for item in summary["targets"]} == {
        "exe/empty": 0,
        "exe/test": 1,
    }
    assert (tmp_path / "out/audit/summary.json").is_file()
    assert calls == {"connect": 1, "execute": 6}


@pytest.mark.parametrize("escape", ["outside", "symlink"])
def test_initialize_all_rejects_output_escape_before_creation(
    tmp_path: Path, escape: str
) -> None:
    root = tmp_path / "repo"
    root.mkdir()
    outside = tmp_path / "outside"
    if escape == "outside":
        output = outside / "audit"
    else:
        outside.mkdir()
        (root / "reports").symlink_to(outside, target_is_directory=True)
        output = root / "reports/audit"

    with pytest.raises(ValueError, match="escapes repository root|symlink escape"):
        initialize_all(root, output)
    assert not (outside / "audit").exists()


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

    import harness.naming.inventory as bulk

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

    monkeypatch.setattr("harness.naming.audit.connect_index", connect)
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
        "harness.naming.audit.connect_index", lambda *_, **__: Connection()
    )
    monkeypatch.setattr(
        "harness.naming.audit.validate_v3",
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
        "harness.naming.audit.connect_index", lambda *_, **__: Connection()
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


def test_live_exact_runs_and_records_both_acceptance_commands(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    calls: list[list[str]] = []

    def run(command, **_kwargs):
        calls.append(command)
        return type(
            "Result", (), {"returncode": 1 if "asm-diff" in command[0] else 0}
        )()

    monkeypatch.setattr(naming_audit.subprocess, "run", run)
    exact, validation = naming_audit._live_exact(tmp_path, "exe/test", 0x80100000)
    assert exact is False
    assert [Path(command[0]).name for command in calls] == ["asm-diff", "byte-match"]
    assert validation == [
        f"{tmp_path}/bin/asm-diff exe/test@0x80100000 --detail normal: exit 1",
        f"{tmp_path}/bin/byte-match exe/test@0x80100000: exit 0",
    ]


def test_prepare_repairs_only_after_live_exact_proof(
    tmp_path: Path, monkeypatch
) -> None:
    source = _repo(tmp_path)
    monkeypatch.setattr(
        "harness.naming.audit._live_exact",
        lambda _root, _target, _address: (
            True,
            [
                "bin/asm-diff selector --detail normal: exit 0",
                "bin/byte-match selector: exit 0",
            ],
        ),
    )
    result = prepare(tmp_path, "exe/test", repair=True)
    assert result["ready"] is True
    assert "@residual none" in source.read_text(encoding="utf-8")


def test_prepare_does_not_repair_failed_live_proof(tmp_path: Path, monkeypatch) -> None:
    source = _repo(tmp_path)
    before = source.read_text(encoding="utf-8")
    monkeypatch.setattr(
        "harness.naming.audit._live_exact",
        lambda _root, _target, _address: (
            False,
            [
                "bin/asm-diff selector --detail normal: exit 1",
                "bin/byte-match selector: exit 1",
            ],
        ),
    )
    result = prepare(tmp_path, "exe/test", repair=True)
    assert result["ready"] is False
    assert source.read_text(encoding="utf-8") == before


def _repo_two_sources(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> tuple[Path, Path]:
    """``exe/test`` claims two repairable lift sources at distinct addresses."""

    def _write_source(address: str, name: str) -> Path:
        source = tmp_path / f"src/test/{name}.c"
        source.parent.mkdir(parents=True, exist_ok=True)
        source.write_text(
            f"/* @source {address}\n * @behavior UNKNOWN: test\n"
            " * @status exact\n * @match 100.00\n */\n"
            f"void {name}(void) {{}}\n",
            encoding="utf-8",
        )
        return source

    _repo(tmp_path)
    manifest = naming_audit.load_target_manifests(tmp_path)["exe/test"]
    loaded = {
        **naming_audit.load_target_manifests(tmp_path),
        "exe/test": manifest,
    }
    loaded["exe/test"] = type(
        "Manifest",
        (),
        {
            name: getattr(manifest, name) if name not in {"sources"} else None
            for name in dir(manifest)
            if not name.startswith("_")
        },
    )
    loaded["exe/test"].sources = [
        "src/test/func_80100000.c",
        "src/test/func_80100010.c",
    ]
    monkeypatch.setattr(naming_audit, "load_target_manifests", lambda *_: loaded)
    return _write_source("0x80100000", "func_80100000"), _write_source(
        "0x80100010", "func_80100010"
    )


def test_prepare_repairs_only_requested_rows(tmp_path: Path, monkeypatch) -> None:
    selected, unselected = _repo_two_sources(tmp_path, monkeypatch)
    proved: list[int] = []

    def live_exact(_root, _target, address):
        proved.append(address)
        return True, [
            "bin/asm-diff selector --detail normal: exit 0",
            "bin/byte-match selector: exit 0",
        ]

    monkeypatch.setattr("harness.naming.audit._live_exact", live_exact)
    result = prepare(tmp_path, "exe/test", repair=True, rows=["function:func_80100000"])
    assert proved == [0x80100000]
    assert [record["row"] for record in result["repaired"]] == [
        "function:func_80100000"
    ]
    assert "@residual none" in selected.read_text(encoding="utf-8")
    assert "@residual none" not in unselected.read_text(encoding="utf-8")
    assert "function:func_80100010" in [
        str(finding["row"]) for finding in result["findings"]
    ]


def test_prepare_rejects_unsorted_duplicate_rows(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(
        "harness.naming.audit._live_exact",
        lambda _root, _target, _address: (
            True,
            [
                "bin/asm-diff selector --detail normal: exit 0",
                "bin/byte-match selector: exit 0",
            ],
        ),
    )
    monkeypatch.setattr(naming_audit, "select_rows_findings", lambda *_: None)
    with pytest.raises(ValueError, match="sorted"):
        prepare(
            tmp_path,
            "exe/test",
            repair=True,
            rows=["function:func_80100010", "function:func_80100000"],
        )
    with pytest.raises(ValueError, match="duplicate"):
        prepare(
            tmp_path,
            "exe/test",
            repair=True,
            rows=["function:func_80100000", "function:func_80100000"],
        )


def test_prepare_rejects_unknown_or_non_repair_row(tmp_path: Path, monkeypatch) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(
        "harness.naming.audit._live_exact",
        lambda _root, _target, _address: (
            True,
            [
                "bin/asm-diff selector --detail normal: exit 0",
                "bin/byte-match selector: exit 0",
            ],
        ),
    )
    with pytest.raises(ValueError, match="unknown target row"):
        prepare(tmp_path, "exe/test", repair=True, rows=["function:func_DEADBEEF"])


def test_prepare_rejects_rows_without_repair(tmp_path: Path) -> None:
    _repo(tmp_path)
    with pytest.raises(ValueError, match="requires --repair"):
        prepare(tmp_path, "exe/test", rows=["function:func_80100000"])


def test_prepare_selected_failed_proof_rolls_back(tmp_path: Path, monkeypatch) -> None:
    selected, unselected = _repo_two_sources(tmp_path, monkeypatch)
    monkeypatch.setattr(
        "harness.naming.audit._live_exact",
        lambda _root, _target, _address: (
            True,
            [
                "bin/asm-diff selector --detail normal: exit 0",
                "bin/byte-match selector: exit 0",
            ],
        ),
    )
    monkeypatch.setattr(
        "harness.naming.preparation.canonical_exact_progress",
        lambda _text: (_ for _ in ()).throw(ValueError("synthetic")),
    )
    before = selected.read_text(encoding="utf-8")
    result = prepare(
        tmp_path,
        "exe/test",
        repair=True,
        rows=["function:func_80100000", "function:func_80100010"],
    )
    assert result["repaired"] == []
    assert selected.read_text(encoding="utf-8") == before
    assert "@residual none" not in unselected.read_text(encoding="utf-8")


def test_prepare_cli_rows_repair_requires_flag_and_valid_selectors(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    monkeypatch.setattr(
        "harness.naming.audit._live_exact",
        lambda _root, _target, _address: (
            True,
            [
                "bin/asm-diff selector --detail normal: exit 0",
                "bin/byte-match selector: exit 0",
            ],
        ),
    )
    import harness.naming.cli as cli

    with pytest.raises(ValueError, match="requires --repair"):
        cli._run_prepare(
            argparse.Namespace(
                root=tmp_path,
                target="exe/test",
                repair=False,
                rows="function:func_80100000",
            )
        )
    with pytest.raises(ValueError, match="unknown target row"):
        cli._run_prepare(
            argparse.Namespace(
                root=tmp_path,
                target="exe/test",
                repair=True,
                rows="function:func_CAFE0001",
            )
        )
    with pytest.raises(ValueError, match="invalid row selector"):
        cli._run_prepare(
            argparse.Namespace(
                root=tmp_path, target="exe/test", repair=True, rows="bogus"
            )
        )


def test_prepare_cli_omitted_rows_is_legacy_full_repair(tmp_path: Path, monkeypatch):
    """Omitted ``--rows`` (None) repairs every safe row, the legacy contract."""

    selected, unselected = _repo_two_sources(tmp_path, monkeypatch)
    monkeypatch.setattr(
        "harness.naming.audit._live_exact",
        lambda _root, _target, _address: (
            True,
            [
                "bin/asm-diff selector --detail normal: exit 0",
                "bin/byte-match selector: exit 0",
            ],
        ),
    )
    import harness.naming.cli as cli

    exit_code = cli._run_prepare(
        argparse.Namespace(root=tmp_path, target="exe/test", repair=True, rows=None)
    )
    assert exit_code == 0
    assert "@residual none" in selected.read_text(encoding="utf-8")
    assert "@residual none" in unselected.read_text(encoding="utf-8")


def test_prepare_cli_empty_rows_repairs_nothing(tmp_path: Path, monkeypatch):
    """A supplied empty ``--rows`` (or commas-only) is explicit and writes nothing."""

    import harness.naming.cli as cli

    for index, rows in enumerate(("", ",,")):
        case_root = tmp_path / str(index)
        source, other = _repo_two_sources(case_root, monkeypatch)
        before = [
            source.read_text(encoding="utf-8"),
            other.read_text(encoding="utf-8"),
        ]
        calls = {"n": 0}
        original = naming_audit._live_exact

        def live_exact(*_args):
            calls["n"] += 1
            return original(*_args)

        monkeypatch.setattr(naming_audit, "_live_exact", live_exact)
        exit_code = cli._run_prepare(
            argparse.Namespace(
                root=case_root, target="exe/test", repair=True, rows=rows
            )
        )
        assert exit_code == 1
        # No live proof was run and no file was rewritten.
        assert calls["n"] == 0
        assert source.read_text(encoding="utf-8") == before[0]
        assert other.read_text(encoding="utf-8") == before[1]


def test_prepare_supplied_empty_rows_repairs_nothing(tmp_path: Path, monkeypatch):
    """API level: ``rows=[]`` is explicit and safe; no live proof, no writes."""

    source, other = _repo_two_sources(tmp_path, monkeypatch)
    before = [source.read_text(encoding="utf-8"), other.read_text(encoding="utf-8")]
    calls = {"n": 0}
    original = naming_audit._live_exact

    def live_exact(*_args):
        calls["n"] += 1
        return original(*_args)

    monkeypatch.setattr(naming_audit, "_live_exact", live_exact)
    monkeypatch.setattr(
        naming_audit,
        "source_addresses",
        lambda *_args: (_ for _ in ()).throw(AssertionError("scope scan ran")),
    )
    result = prepare(tmp_path, "exe/test", repair=True, rows=[])
    assert result["repaired"] == []
    assert calls["n"] == 0
    assert source.read_text(encoding="utf-8") == before[0]
    assert other.read_text(encoding="utf-8") == before[1]


def test_prepare_first_write_failure_reports_first_selector(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    source, other = _repo_two_sources(tmp_path, monkeypatch)
    monkeypatch.setattr(
        "harness.naming.audit._live_exact",
        lambda _root, _target, _address: (
            True,
            [
                "bin/asm-diff selector --detail normal: exit 0",
                "bin/byte-match selector: exit 0",
            ],
        ),
    )
    import harness.naming.preparation as prep

    before = [source.read_text(encoding="utf-8"), other.read_text(encoding="utf-8")]
    monkeypatch.setattr(
        prep,
        "_atomic_write",
        lambda *_args: (_ for _ in ()).throw(OSError("first write failed")),
    )
    result = prepare(
        tmp_path,
        "exe/test",
        repair=True,
        rows=["function:func_80100000", "function:func_80100010"],
    )
    assert result["repaired"] == []
    assert [
        source.read_text(encoding="utf-8"),
        other.read_text(encoding="utf-8"),
    ] == before
    failed = [
        finding
        for finding in result["findings"]
        if finding.get("class") == "review_required"
    ]
    assert [finding["row"] for finding in failed] == ["function:func_80100000"]
    assert failed[0]["validation"] == [
        "bin/asm-diff selector --detail normal: exit 0",
        "bin/byte-match selector: exit 0",
    ]


def test_prepare_write_failure_rolls_back(tmp_path: Path, monkeypatch):
    """A failing repair write restores the snapshot; no silent success, no loss."""

    source, other = _repo_two_sources(tmp_path, monkeypatch)
    monkeypatch.setattr(
        "harness.naming.audit._live_exact",
        lambda _root, _target, _address: (
            True,
            [
                "bin/asm-diff selector --detail normal: exit 0",
                "bin/byte-match selector: exit 0",
            ],
        ),
    )
    import harness.naming.preparation as prep

    before = [source.read_text(encoding="utf-8"), other.read_text(encoding="utf-8")]
    calls = {"n": 0}
    atomic_write = prep._atomic_write

    def fail_second_write(root, relative, text):
        calls["n"] += 1
        if calls["n"] == 2:  # second repair write fails, rollback succeeds
            raise OSError(f"simulated write failure on {relative}")
        atomic_write(root, relative, text)

    monkeypatch.setattr(prep, "_atomic_write", fail_second_write)
    result = prepare(
        tmp_path,
        "exe/test",
        repair=True,
        rows=["function:func_80100000", "function:func_80100010"],
    )
    assert result["repaired"] == []
    # Every touched file is back to its pre-image; nothing was left half-written.
    assert source.read_text(encoding="utf-8") == before[0]
    assert other.read_text(encoding="utf-8") == before[1]
    assert "@residual none" not in source.read_text(encoding="utf-8")
    rollback_note = [
        f
        for f in result["findings"]
        if f.get("class") == "review_required"
        and "rolled back" in str(f.get("reason", ""))
    ]
    assert rollback_note, "a rolled-back row must be reported, not silently dropped"


def test_prepare_restore_failure_reports_rollback_error(tmp_path: Path, monkeypatch):
    """A failing rollback is raised (not masked) and leaves a recoverable snapshot."""

    source, other = _repo_two_sources(tmp_path, monkeypatch)
    monkeypatch.setattr(
        "harness.naming.audit._live_exact",
        lambda _root, _target, _address: (
            True,
            [
                "bin/asm-diff selector --detail normal: exit 0",
                "bin/byte-match selector: exit 0",
            ],
        ),
    )
    import harness.naming.preparation as prep

    calls = {"n": 0}

    atomic_write = prep._atomic_write

    def fail_write_and_restore(root, relative, text):
        calls["n"] += 1
        if calls["n"] == 1:
            atomic_write(root, relative, text)
            return
        raise OSError(f"simulated failure on {relative}")

    monkeypatch.setattr(prep, "_atomic_write", fail_write_and_restore)
    with pytest.raises(RollbackError) as excinfo:
        prepare(
            tmp_path,
            "exe/test",
            repair=True,
            rows=["function:func_80100000", "function:func_80100010"],
        )
    error = excinfo.value
    assert "rollback failed" in str(error)
    # The recoverable snapshot holds the pre-images of every file that was written.
    assert set(error.snapshot) == {
        "src/test/func_80100000.c",
        "src/test/func_80100010.c",
    }
    assert "@residual none" not in error.snapshot["src/test/func_80100000.c"]
    assert "@residual none" in source.read_text(encoding="utf-8")
    # No temp files are leaked.
    assert list((tmp_path / "src/test").glob("*.rollback-tmp")) == []
