from __future__ import annotations

from pathlib import Path

from harness.emi.operations import emi_pack, emi_unpack


def test_emi_unpack_extracts_all_archives_into_mirrored_dirs(
    monkeypatch, tmp_path: Path
) -> None:
    extracted_dir = tmp_path / "build" / "extracted"
    archive_path = extracted_dir / "BIN" / "ETC" / "GAME.EMI"
    archive_path.parent.mkdir(parents=True)
    archive_path.write_bytes(b"emi")
    calls: list[list[str]] = []

    def fake_run_command(command: list[str], *, cwd, env=None) -> None:
        calls.append(command)

    monkeypatch.setattr("harness.emi.operations.run_command", fake_run_command)

    archive_count = emi_unpack(
        tool_path=tmp_path / "build" / "tools" / "emi-ex",
        cwd=tmp_path,
        extracted_dir=extracted_dir,
        raw_emi_dir=tmp_path / "out" / "emi_raw",
    )

    assert archive_count == 1
    assert calls == [
        [
            str(tmp_path / "build" / "tools" / "emi-ex"),
            "extract",
            "--quiet",
            "-J",
            "-o",
            str(tmp_path / "out" / "emi_raw" / "BIN" / "ETC" / "GAME"),
            str(archive_path),
        ]
    ]


def test_emi_unpack_fails_when_no_archives_are_present(tmp_path: Path) -> None:
    extracted_dir = tmp_path / "build" / "extracted"
    extracted_dir.mkdir(parents=True)

    try:
        emi_unpack(
            tool_path=tmp_path / "build" / "tools" / "emi-ex",
            cwd=tmp_path,
            extracted_dir=extracted_dir,
            raw_emi_dir=tmp_path / "out" / "emi_raw",
        )
    except RuntimeError as error:
        assert f"no EMI archives found under {extracted_dir}" == str(error)
        return

    raise AssertionError("expected emi_unpack to fail without EMI archives")


def test_emi_pack_packs_all_manifests_back_into_extracted_tree(
    monkeypatch, tmp_path: Path
) -> None:
    raw_emi_dir = tmp_path / "out" / "emi_raw"
    archive_dir = raw_emi_dir / "BIN" / "ETC" / "GAME"
    archive_dir.mkdir(parents=True)
    (archive_dir / "emi.json").write_text("{}\n", encoding="utf-8")
    calls: list[list[str]] = []

    def fake_run_command(command: list[str], *, cwd, env=None) -> None:
        calls.append(command)

    monkeypatch.setattr("harness.emi.operations.run_command", fake_run_command)

    archive_count = emi_pack(
        tool_path=tmp_path / "build" / "tools" / "emi-ex",
        cwd=tmp_path,
        raw_emi_dir=raw_emi_dir,
        extracted_dir=tmp_path / "build" / "extracted",
    )

    assert archive_count == 1
    assert calls == [
        [
            str(tmp_path / "build" / "tools" / "emi-ex"),
            "pack",
            "--quiet",
            "-o",
            str(tmp_path / "build" / "extracted" / "BIN" / "ETC" / "GAME.EMI"),
            "-J",
            str(archive_dir / "emi.json"),
            str(archive_dir),
        ]
    ]
