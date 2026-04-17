from __future__ import annotations

import zipfile
from pathlib import Path

from rebof3.toolchain import setup_disc as disc_lib
from rebof3.toolchain.setup_disc import import_bof3_disc


def make_fake_bof3_archive(path: Path) -> None:
    with zipfile.ZipFile(path, "w") as archive:
        archive.writestr(
            "Breath Of Fire III/game.cue",
            'FILE "game.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n',
        )
        archive.writestr("Breath Of Fire III/game.bin", b"fake-disc")


def test_import_bof3_disc_stages_active_disc_set(monkeypatch, tmp_path: Path) -> None:
    monkeypatch.setattr(disc_lib, "REPO_ROOT", tmp_path)
    archive_path = tmp_path / "BreathOfFireIII.zip"
    private_root = tmp_path / "private-assets"
    disc_dir = tmp_path / "inputs" / "disc"
    make_fake_bof3_archive(archive_path)

    result = import_bof3_disc(
        archive=archive_path,
        dest=disc_dir,
        private_assets_root=private_root,
    )

    assert (
        result.archive_path
        == private_root / "bof3" / "source-media" / archive_path.name
    )
    assert (disc_dir / "game.cue").exists()
    assert (disc_dir / "game.bin").read_bytes() == b"fake-disc"


def test_discover_disc_archive_rejects_outside_repo(tmp_path: Path) -> None:
    archive_path = tmp_path / "BreathOfFireIII.zip"
    archive_path.write_bytes(b"zip")

    try:
        disc_lib.discover_disc_archive(archive_path)
    except ValueError as exc:
        assert "must stay inside the repo workspace" in str(exc)
    else:
        raise AssertionError("expected outside-repo archive to be rejected")
