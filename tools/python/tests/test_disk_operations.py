from __future__ import annotations

from pathlib import Path

from rebof3.disk.operations import disk_extract


def test_disk_extract_imports_archive_when_disc_inputs_are_missing(
    monkeypatch, tmp_path: Path
) -> None:
    disc_dir = tmp_path / "inputs" / "disc"
    private_assets_root = tmp_path / "external" / "private-assets"
    extracted_dir = tmp_path / "build" / "extracted"
    calls: list[list[str]] = []

    def fake_import_bof3_disc(**kwargs) -> None:
        disc_dir.mkdir(parents=True, exist_ok=True)
        (disc_dir / "game.cue").write_text('FILE "game.bin" BINARY\n', encoding="utf-8")
        (disc_dir / "game.bin").write_bytes(b"fake")

    def fake_run_command(command: list[str], *, cwd, env=None) -> None:
        calls.append(command)

    monkeypatch.setattr(
        "rebof3.disk.operations.import_bof3_disc", fake_import_bof3_disc
    )
    monkeypatch.setattr("rebof3.disk.operations.run_command", fake_run_command)

    input_path = disk_extract(
        tool_path=tmp_path / "build" / "third_party" / "bof3-disk",
        cwd=tmp_path,
        output_dir=extracted_dir,
        disc_dir=disc_dir,
        private_assets_root=private_assets_root,
        archive_path=tmp_path / "downloads" / "bof3.7z",
        force=True,
    )

    assert input_path == disc_dir / "game.cue"
    assert calls == [
        [
            str(tmp_path / "build" / "third_party" / "bof3-disk"),
            "extract",
            "-i",
            str(disc_dir / "game.cue"),
            "-o",
            str(extracted_dir),
        ]
    ]
