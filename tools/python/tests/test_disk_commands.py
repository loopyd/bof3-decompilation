from __future__ import annotations

from pathlib import Path

from rebof3.commands import disk as disk_command


def test_disk_extract_command_runs_shared_operation(
    monkeypatch, capsys, tmp_path: Path
) -> None:
    calls: list[dict[str, Path | bool | None]] = []

    def fake_disk_extract(**kwargs):
        calls.append(kwargs)
        return tmp_path / "inputs" / "disc" / "game.cue"

    monkeypatch.setattr(disk_command, "disk_extract", fake_disk_extract)

    result = disk_command.main(
        [
            "disk-extract",
            "--input",
            str(tmp_path / "game.cue"),
            "--output",
            str(tmp_path / "build" / "extracted"),
        ]
    )

    assert result == 0
    assert calls[0]["input_path"] == tmp_path / "game.cue"
    assert calls[0]["output_dir"] == tmp_path / "build" / "extracted"
    assert "extracted" in capsys.readouterr().out


def test_disk_rebuild_command_uses_detected_project_xml(
    monkeypatch, capsys, tmp_path: Path
) -> None:
    extracted_dir = tmp_path / "build" / "extracted"
    extracted_dir.mkdir(parents=True)
    project_xml = extracted_dir / "project.xml"
    project_xml.write_text("<iso_project />\n", encoding="utf-8")
    calls: list[dict[str, Path]] = []

    def fake_disk_rebuild(**kwargs) -> None:
        calls.append(kwargs)

    monkeypatch.setattr(disk_command, "disk_rebuild", fake_disk_rebuild)

    result = disk_command.main(
        [
            "disk-rebuild",
            "--extracted-dir",
            str(extracted_dir),
            "--rebuilt-dir",
            str(tmp_path / "build" / "rebuilt"),
        ]
    )

    assert result == 0
    assert calls[0]["project_xml_path"] == project_xml
    assert (
        calls[0]["output_path"]
        == tmp_path / "build" / "rebuilt" / "project_track01.bin"
    )
    assert (
        calls[0]["cue_path"] == tmp_path / "build" / "rebuilt" / "project_track01.cue"
    )
    assert "rebuilt disc image" in capsys.readouterr().out


def test_disk_rebuild_command_fails_without_project_xml(tmp_path: Path) -> None:
    extracted_dir = tmp_path / "build" / "extracted"
    extracted_dir.mkdir(parents=True)

    try:
        disk_command.main(["disk-rebuild", "--extracted-dir", str(extracted_dir)])
    except RuntimeError as error:
        assert f"no project XML found under {extracted_dir}" == str(error)
        return

    raise AssertionError("expected disk-rebuild to fail without a project XML")


def test_disk_verify_command_runs_shared_operation(
    monkeypatch, capsys, tmp_path: Path
) -> None:
    calls: list[dict[str, Path]] = []

    def fake_disk_verify(**kwargs) -> None:
        calls.append(kwargs)

    monkeypatch.setattr(disk_command, "disk_verify", fake_disk_verify)

    result = disk_command.main(
        [
            "disk-verify",
            "--input-dir",
            str(tmp_path / "inputs" / "disc"),
            "--checksums",
            str(tmp_path / "out" / "disk_checksums.json"),
        ]
    )

    assert result == 0
    assert calls[0]["input_dir"] == tmp_path / "inputs" / "disc"
    assert calls[0]["checksums_path"] == tmp_path / "out" / "disk_checksums.json"
    assert "verified disk images" in capsys.readouterr().out


def test_disk_checksums_command_runs_shared_operation(
    monkeypatch, capsys, tmp_path: Path
) -> None:
    calls: list[dict[str, Path]] = []

    def fake_disk_checksums(**kwargs) -> None:
        calls.append(kwargs)

    monkeypatch.setattr(disk_command, "disk_checksums", fake_disk_checksums)

    result = disk_command.main(
        [
            "disk-checksums",
            "--input-dir",
            str(tmp_path / "inputs" / "disc"),
            "--output",
            str(tmp_path / "out" / "disk_checksums.json"),
        ]
    )

    assert result == 0
    assert calls[0]["output_path"] == tmp_path / "out" / "disk_checksums.json"
    assert "wrote disk checksums" in capsys.readouterr().out
