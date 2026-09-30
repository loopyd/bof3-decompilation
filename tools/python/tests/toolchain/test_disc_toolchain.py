"""Disc toolchain path ownership contracts."""

from __future__ import annotations

from pathlib import Path

from harness.io import repo_layout
from harness.toolchain import disc


def test_disc_fallback_passes_layout_owned_roots(tmp_path: Path, monkeypatch) -> None:
    layout = repo_layout(tmp_path)
    calls = []
    cue = layout.external_inputs_dir / "disc.cue"
    track = layout.external_inputs_dir / "disc.bin"

    def fake_find(root: Path):
        if not calls:
            raise FileNotFoundError
        return cue, [track]

    def fake_import(**kwargs):
        calls.append(kwargs)
        return disc.DiscImportResult(
            archive_path=layout.private_assets_dir / "bof3/source-media/disc.zip",
            extracted_root=layout.private_assets_dir / "bof3/source-tree/disc",
            cue_path=cue,
            staged_paths=(cue, track),
        )

    monkeypatch.setattr(disc, "find_disc_set", fake_find)
    monkeypatch.setattr(disc, "import_bof3_disc", fake_import)

    assert disc.DiscToolchain(layout).install() == "disc.cue, 1 tracks"
    assert calls == [
        {
            "inputs_root": layout.inputs_dir,
            "private_assets_root": layout.private_assets_dir,
            "downloads_root": layout.downloads_dir,
            "force": False,
        }
    ]


def test_disc_archive_discovery_isolated_to_temp_layout(tmp_path: Path) -> None:
    layout = repo_layout(tmp_path)
    archive = layout.external_inputs_dir / disc.ARCHIVE_NAME
    archive.parent.mkdir(parents=True)
    archive.write_bytes(b"archive")

    assert (
        disc.discover_disc_archive(
            inputs_root=layout.inputs_dir,
            archive_cache_root=layout.private_assets_dir / "bof3/source-media",
        )
        == archive
    )
