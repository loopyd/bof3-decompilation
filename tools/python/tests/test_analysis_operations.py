from __future__ import annotations

from pathlib import Path

import pytest

from harness.analysis import operations
from harness.analysis.rizin import EngineIdentity
from harness.domain import TargetManifest
from harness.domain.ids import TargetId


def _manifest() -> TargetManifest:
    return TargetManifest(
        id=TargetId("emi/etc/game/01", "BIN/ETC/GAME.EMI#1"),
        disc_id="BIN/ETC/GAME.EMI#1",
        kind="emi",
        source_dir="src/emi/etc/game/01",
        binary="out/binaries/emi/etc/game/01.bin",
        splat="config/splat/emi/etc/game/01.yaml",
        load_address=0x801D0C00,
        profile="native/capcom97",
    )


def test_doctor_reports_rizin_when_available(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    def fake_find(name="rizin"):
        if name != "rizin":
            raise FileNotFoundError(f"{name} not found")
        return EngineIdentity(
            name="rizin",
            executable=Path("/usr/bin/rizin"),
            version="rizin 0.8.2",
            capabilities={
                "mips32_little_endian": True,
                "json": True,
                "projects": True,
                "decompiler": False,
            },
        )

    monkeypatch.setattr(operations, "find_engine", fake_find)

    result = operations.doctor()

    assert result["engine"] == "rizin"
    assert result["available"] is True
    assert result["version"] == "rizin 0.8.2"


def test_doctor_reports_r2_when_rizin_unavailable(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    calls = []

    def fake_find(name="rizin"):
        calls.append(name)
        if name == "rizin":
            raise FileNotFoundError("rizin not found")
        return EngineIdentity(
            name="r2",
            executable=Path("/usr/bin/r2"),
            version="radare2 6.1.4",
            capabilities={
                "mips32_little_endian": True,
                "json": True,
                "projects": True,
                "decompiler": True,
            },
        )

    monkeypatch.setattr(operations, "find_engine", fake_find)

    result = operations.doctor()

    assert result["engine"] == "r2"
    assert result["available"] is True
    assert calls == ["rizin", "r2"]


def test_analysis_paths_isolate_state(tmp_path: Path) -> None:
    manifest = _manifest()

    paths = operations._paths(tmp_path, manifest)

    assert paths[0] == (
        tmp_path / "out" / "analysis" / "projects" / "rizin" / "emi__etc__game__01"
    )
    assert paths[1] == (
        tmp_path / "out" / "analysis" / "exports" / "rizin" / "emi__etc__game__01"
    )


@pytest.mark.parametrize(
    ("value", "classification"),
    [
        ("Load Game", "text_candidate"),
        ("\x00\x02", "control_bytes"),
        ("\xff\xff", "repeated_fill"),
        ("\xae\xaf\xb0\xb1", "sequential_table"),
        ("\xae\xaf", "data_pattern"),
        ("+-", "data_pattern"),
        ("ok", "data_pattern"),
        (None, "data_pattern"),
    ],
)
def test_classifies_analyzer_string_guesses_conservatively(
    value: object, classification: str
) -> None:
    assert operations._classify_analyzer_string(value) == classification
