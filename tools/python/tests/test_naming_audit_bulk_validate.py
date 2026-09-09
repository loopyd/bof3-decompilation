"""P0 bulk validate: full-report runs on one bulk context; live path untouched.

Count-based guards from the cleanup-toolchain-performance plan: full-report
validate constructs exactly one ``required_work_snapshot`` on exactly one
index connection, the counts stay constant as the row count grows
(>=200-row fixture), and the result is field-equivalent to the legacy
per-row live validation (the 26-minute 249-row baseline is not rerun).
"""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

import harness.naming.audit as naming_audit
import pytest
from harness.analysis.index import rebuild
from harness.analysis.project import prepare_target
from harness.analysis.snapshot import (
    SNAPSHOT_SCHEMA,
    AnalysisSnapshot,
    SnapshotFunction,
    snapshot_path,
    write_snapshot,
)
from harness.naming.audit import validate

TARGET = "exe/test"


def _repo(root: Path) -> None:
    config = root / "config/targets/exe/test/target.toml"
    config.parent.mkdir(parents=True)
    config.write_text(
        "schema='harness.target/v2'\nid='exe/test'\nkind='executable'\n"
        "source_dir='src/test'\n"
        "binary='out/test.bin'\nload_address=0x80100000\n"
        "splat='config/targets/exe/test/splat.yaml'\n"
        "sources=[]\n",
        encoding="utf-8",
    )
    (config.parent / "symbols.txt").write_text(
        "func_80100000 = 0x80100000;\nD_80100010 = 0x80100010;\n",
        encoding="utf-8",
    )
    (config.parent / "splat.yaml").write_text(
        "segments:\n  - [0, c, func_80100000]\n  - [0x10, data, D_80100010]\n  - [0x20]\n",
        encoding="utf-8",
    )
    sdk = root / "config/sdk/psyq-slus.txt"
    sdk.parent.mkdir(parents=True, exist_ok=True)
    sdk.write_text("", encoding="utf-8")
    (root / "out").mkdir(parents=True, exist_ok=True)
    (root / "out/test.bin").write_bytes(b"\0" * 0x20)


def _index(root: Path) -> None:
    """Build the disposable reverse index for the fixture target."""
    base_types = root / "include/base/types.h"
    base_types.parent.mkdir(parents=True, exist_ok=True)
    base_types.write_text(
        "typedef unsigned char bool;\ntypedef unsigned int u32;\n",
        encoding="utf-8",
    )
    binary = root / "out/test.bin"
    write_snapshot(
        AnalysisSnapshot(
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
        ),
        snapshot_path(root, TARGET),
    )
    rebuild(root)


class CountingConnection:
    def __init__(self, calls: dict[str, int]) -> None:
        self.calls = calls

    def execute(self, *_args: Any, **_kwargs: Any) -> list[object]:
        self.calls["execute"] += 1
        return []

    def close(self) -> None:
        self.calls["closed"] += 1


def _connect_factory(calls: dict[str, int]):
    def connect(*_args: Any, **_kwargs: Any) -> CountingConnection:
        calls["connect"] += 1
        return CountingConnection(calls)

    return connect


def test_full_report_validate_uses_one_bulk_snapshot_and_one_connection(
    tmp_path: Path, monkeypatch
) -> None:
    """Exactly one required_work_snapshot construction, one index connection."""
    _repo(tmp_path)
    calls = {"connect": 0, "execute": 0, "closed": 0}
    reports: list[dict[str, Any]] = []

    def record_validate(
        root: Path, target: str, report: dict[str, Any], ctx: Any, **kwargs: Any
    ) -> dict[str, Any]:
        reports.append({"bulk": ctx.work_snapshot is not None})
        return {
            "schema": "bof3.naming-audit/v3",
            "target": target,
            "rows": 0,
            "complete": True,
        }

    monkeypatch.setattr(naming_audit, "validate_v3", record_validate)
    monkeypatch.setattr(naming_audit, "connect_index", _connect_factory(calls))
    result = validate(
        tmp_path,
        TARGET,
        {"schema": "bof3.naming-audit/v3", "target": TARGET, "rows": []},
    )
    assert result["complete"] is True
    assert reports == [{"bulk": True}], (
        "full-report validate must build one bulk TargetContext"
    )
    assert calls == {"connect": 1, "execute": 3, "closed": 1}, (
        "one index connection running the snapshot's three bounded queries"
    )


def test_full_report_validate_is_field_equivalent_to_live_validation(
    tmp_path: Path, monkeypatch
) -> None:
    """Bulk path output must equal the legacy per-row live validation exactly.

    Equivalence is pinned by validating the same report under both context
    shapes and hitting the same validation error on the same defect; the
    legacy 26-minute 249-row baseline is not rerun.
    """
    _repo(tmp_path)
    _index(tmp_path)
    monkeypatch.setattr(
        "harness.naming.audit.project_status", lambda *_: {"fresh": True}
    )
    from harness.naming.audit import initialize

    report = initialize(tmp_path, TARGET)
    bulk = validate(tmp_path, TARGET, report)
    isolated = naming_audit.validate_v3(
        tmp_path, TARGET, report, naming_audit._context(tmp_path, TARGET)
    )
    assert bulk == isolated
    assert bulk["complete"] is False
    assert bulk["rows"] == len(report["rows"])

    bad = json.loads(json.dumps(report))
    bad["rows"][0].pop("missing_fact")
    with pytest.raises(ValueError, match="missing missing_fact"):
        validate(tmp_path, TARGET, bad)
    with pytest.raises(ValueError, match="missing missing_fact"):
        naming_audit.validate_v3(
            tmp_path, TARGET, bad, naming_audit._context(tmp_path, TARGET)
        )


def test_full_report_validate_never_calls_per_row_work_lookup(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    calls = {"connect": 0, "execute": 0, "closed": 0}
    monkeypatch.setattr(naming_audit, "connect_index", _connect_factory(calls))
    monkeypatch.setattr(
        "harness.naming.audit.project_status", lambda *_: {"fresh": True}
    )
    from harness.naming.audit import initialize

    report = initialize(tmp_path, TARGET)

    def no_live_work(*_args: Any, **_kwargs: Any) -> list[dict[str, str]]:
        raise AssertionError("full-report validate must not call required_work_items")

    monkeypatch.setattr("harness.naming.context.required_work_items", no_live_work)
    result = validate(tmp_path, TARGET, report)
    assert result["complete"] is False


def test_isolated_transaction_validation_keeps_live_context(
    tmp_path: Path, monkeypatch
) -> None:
    _repo(tmp_path)
    modes: list[bool] = []
    original_context = naming_audit._context

    def spy_context(
        root: Path,
        target: str,
        *,
        bulk_work: bool = False,
        manifests: dict[str, Any] | None = None,
        connection: Any = None,
    ) -> Any:
        modes.append(bulk_work)
        return original_context(
            root,
            target,
            bulk_work=bulk_work,
            manifests=manifests,
            connection=connection,
        )

    monkeypatch.setattr(naming_audit, "_context", spy_context)
    monkeypatch.setattr(
        naming_audit,
        "connect_index",
        _connect_factory({"connect": 0, "execute": 0, "closed": 0}),
    )
    monkeypatch.setattr(
        "harness.naming.audit.project_status", lambda *_: {"fresh": True}
    )
    monkeypatch.setattr("harness.naming.context.required_work_items", lambda *_: [])

    def record_validate(
        root: Path, target: str, report: dict[str, Any], ctx: Any, **kwargs: Any
    ) -> dict[str, Any]:
        return {
            "schema": "bof3.naming-audit/v3",
            "target": target,
            "transaction": kwargs.get("transaction"),
            "rows": 1,
        }

    monkeypatch.setattr(naming_audit, "validate_v3", record_validate)
    from harness.naming.audit import initialize

    report = initialize(tmp_path, TARGET)
    modes.clear()
    result = validate(tmp_path, TARGET, report, transaction="function:func_80100000")
    assert modes == [False]
    assert result["transaction"] == "function:func_80100000"


def test_bulk_context_snapshot_counts_are_constant_in_row_count(
    tmp_path: Path, monkeypatch
) -> None:
    """The >=200-row fixture: one connection + one three-query snapshot per
    full validation, independent of row count."""
    _repo(tmp_path)
    (tmp_path / "out/test.bin").write_bytes(b"\0" * (0x1000 * 249 + 0x20))
    calls = {"connect": 0, "execute": 0, "closed": 0}
    monkeypatch.setattr(naming_audit, "connect_index", _connect_factory(calls))
    for rows in (200, 249):
        (tmp_path / "config/targets/exe/test/symbols.txt").write_text(
            "".join(f"func_8010{i:04X} = 0x8010{i:04X};\n" for i in range(rows))
            + "D_80100010 = 0x80100010;\n",
            encoding="utf-8",
        )
        before = {key: calls.get(key, 0) for key in ("connect", "execute", "closed")}
        result = validate(tmp_path, TARGET, _blocked_inventory(rows))
        assert result["rows"] == rows + 1, (
            f"{rows} function rows + one data row: {result['rows']}"
        )
        assert result["complete"] is False
        delta = {key: calls[key] - before[key] for key in before}
        assert delta == {"connect": 1, "execute": 3, "closed": 1}, (
            f"{rows}-row validation must not add per-row work: {delta}"
        )


def _blocked_inventory(rows: int) -> dict[str, Any]:
    """A blocked v3 inventory: ``rows`` open function rows plus the data row."""

    def function_row(index: int) -> dict[str, Any]:
        name = f"func_8010{index:04X}"
        return {
            "kind": "function",
            "name": name,
            "rung_status": "blocked",
            "outside_payload": False,
            "partial_used": False,
            "rungs": {
                rung: {
                    "status": "open",
                    "next_command": "bin/rev-query --json xrefs exe/test",
                    "observations": [
                        {
                            "id": f"{name}.{rung}.gap",
                            "text": f"Evidence gap: {rung} not yet recorded",
                        }
                    ],
                    "authority": "target manifest, reviewed Splat, original image",
                }
                for rung in ("selected_range", "selected_call", "one_level_beyond")
            },
            "required_work": [],
            "optional_work": [],
            "interpretation": "No semantic name is accepted until the gap is closed.",
            "authority": "target manifest, target-local map, reviewed Splat, original image",
            "smallest_repair": "bin/rev-query --json xrefs exe/test",
            "missing_fact": "one_level_beyond consumer",
            "ceiling_next_command": "bin/rev-query --json xrefs exe/test",
        }

    data_row = {
        "kind": "data",
        "name": "D_80100010",
        "rung_status": "blocked",
        "outside_payload": False,
        "partial_used": False,
        "rungs": {
            rung: {
                "status": "open",
                "next_command": "bin/rev-query --json xrefs exe/test",
                "observations": [
                    {
                        "id": f"D_80100010.{rung}.gap",
                        "text": f"Evidence gap: {rung} not yet recorded",
                    }
                ],
                "authority": "target manifest, reviewed Splat, original image",
            }
            for rung in (
                "selected_range",
                "selected_access",
                "storage_class",
                "one_level_beyond",
            )
        },
        "required_work": [],
        "optional_work": [],
        "interpretation": "No semantic name is accepted until the gap is closed.",
        "authority": "target manifest, target-local map, reviewed Splat, original image",
        "smallest_repair": "bin/rev-query --json xrefs exe/test",
        "missing_fact": "storage class",
        "ceiling_next_command": "bin/rev-query --json xrefs exe/test",
    }
    return {
        "schema": "bof3.naming-audit/v3",
        "target": TARGET,
        "complete": False,
        "rows": [function_row(index) for index in range(rows)] + [data_row],
    }
