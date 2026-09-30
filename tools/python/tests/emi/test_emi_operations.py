"""Characterize direct EMI pack operations."""

from pathlib import Path

import pytest

import harness.emi.operations as operations
from harness.emi.operations import emi_pack


def test_emi_pack_maps_sorted_manifests_to_archives(
    monkeypatch: pytest.MonkeyPatch, tmp_path: Path
) -> None:
    raw = tmp_path / "raw"
    extracted = tmp_path / "extracted"
    for relative in (Path("z/02"), Path("a/01")):
        directory = raw / relative
        directory.mkdir(parents=True)
        (directory / "emi.json").write_text("{}", encoding="utf-8")
    calls: list[tuple[list[str], Path]] = []
    monkeypatch.setattr(
        operations,
        "run_command",
        lambda argv, *, cwd: calls.append((argv, cwd)),
    )
    tool = tmp_path / "emi-ex"
    cwd = tmp_path / "cwd"

    assert (
        emi_pack(tool_path=tool, cwd=cwd, raw_emi_dir=raw, extracted_dir=extracted) == 2
    )
    assert calls == [
        (
            [
                str(tool),
                "pack",
                "--quiet",
                "-o",
                str(extracted / "a/01.EMI"),
                "-J",
                str(raw / "a/01/emi.json"),
                str(raw / "a/01"),
            ],
            cwd,
        ),
        (
            [
                str(tool),
                "pack",
                "--quiet",
                "-o",
                str(extracted / "z/02.EMI"),
                "-J",
                str(raw / "z/02/emi.json"),
                str(raw / "z/02"),
            ],
            cwd,
        ),
    ]
    assert (extracted / "a").is_dir()
    assert (extracted / "z").is_dir()


def test_emi_pack_rejects_missing_manifests(tmp_path: Path) -> None:
    raw = tmp_path / "raw"
    raw.mkdir()
    with pytest.raises(
        RuntimeError, match=f"^no unpacked EMI manifests found under {raw}$"
    ):
        emi_pack(
            tool_path=tmp_path / "emi-ex",
            cwd=tmp_path,
            raw_emi_dir=raw,
            extracted_dir=tmp_path / "out",
        )
