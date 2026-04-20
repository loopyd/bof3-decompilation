from __future__ import annotations

import json
from pathlib import Path

from rebof3.commands import assets as assets_command
from rebof3.jsonio import read_json


def make_palette_row() -> bytes:
    row = bytearray()
    for index in range(16):
        value = (index & 0x1F) | ((index & 0x1F) << 5) | ((index & 0x1F) << 10)
        row.extend(value.to_bytes(2, "little"))
    return bytes(row)


def make_indexed_image_payload() -> bytes:
    return bytes([0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE] * 256)


def write_manifest_archive(root: Path, archive_id: str) -> Path:
    archive_dir = root / archive_id
    archive_dir.mkdir(parents=True, exist_ok=True)
    image_payload = make_indexed_image_payload()
    palette_payload = make_palette_row()
    (archive_dir / "0.img").write_bytes(image_payload)
    (archive_dir / "1.bin").write_bytes(palette_payload)
    (archive_dir / "emi.json").write_text(
        json.dumps(
            {
                "entries": [
                    {
                        "index": 0,
                        "name": "0.img",
                        "type": 3,
                        "size": len(image_payload),
                        "ram_ptr": 0x100,
                    },
                    {
                        "index": 1,
                        "name": "1.bin",
                        "type": 0,
                        "size": len(palette_payload),
                        "ram_ptr": 0x80033800,
                    },
                ]
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    return archive_dir


def write_catalog(catalog_path: Path, archive_dir: Path) -> None:
    catalog_path.write_text(
        json.dumps(
            {
                "entries": [
                    {
                        "archive_id": "ETC/FIRST",
                        "archive_name": "FIRST",
                        "family": "ETC",
                        "manifest_path": str(archive_dir / "emi.json"),
                        "image_candidate": True,
                        "palette_candidate": False,
                        "code_candidate": False,
                    },
                    {
                        "archive_id": "ETC/FIRST",
                        "archive_name": "FIRST",
                        "family": "ETC",
                        "manifest_path": str(archive_dir / "emi.json"),
                        "image_candidate": False,
                        "palette_candidate": True,
                        "code_candidate": False,
                    },
                ]
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )


def test_emi_extract_builds_catalog_and_render_metadata(
    tmp_path: Path, monkeypatch
) -> None:
    extracted_dir = tmp_path / "build" / "extracted"
    raw_emi_dir = tmp_path / "out" / "emi_raw"
    emi_catalog_json = tmp_path / "out" / "inventory" / "emi_catalog.json"
    emi_catalog_md = tmp_path / "out" / "inventory" / "emi_catalog.md"
    render_metadata_json = tmp_path / "out" / "inventory" / "render_metadata.json"
    render_metadata_md = tmp_path / "out" / "inventory" / "render_metadata.md"

    def fake_emi_unpack(**_: object) -> int:
        write_manifest_archive(raw_emi_dir / "BIN", "ETC/FIRST")
        return 1

    monkeypatch.setattr(assets_command, "emi_unpack", fake_emi_unpack)

    result = assets_command.main(
        [
            "emi-extract",
            "--input-dir",
            str(extracted_dir),
            "--output-dir",
            str(raw_emi_dir),
            "--emi-catalog-json-out",
            str(emi_catalog_json),
            "--emi-catalog-md-out",
            str(emi_catalog_md),
            "--render-metadata-json-out",
            str(render_metadata_json),
            "--render-metadata-md-out",
            str(render_metadata_md),
        ]
    )

    assert result == 0
    assert read_json(emi_catalog_json)["archive_count"] == 1
    assert read_json(render_metadata_json)["archive_count"] == 1
    assert emi_catalog_md.is_file()
    assert render_metadata_md.is_file()


def test_emi_extract_archive_renders_pngs_from_manifest_dir(tmp_path: Path) -> None:
    archive_dir = write_manifest_archive(tmp_path / "emi", "ETC/FIRST")
    output_dir = tmp_path / "pngs"

    result = assets_command.main(
        [
            "emi-extract-archive",
            str(archive_dir),
            str(output_dir),
            "--palette-row",
            "0",
        ]
    )

    assert result == 0
    assert (output_dir / "0__1__row00.png").is_file()


def test_emi_review_builds_review_packet(tmp_path: Path) -> None:
    archive_dir = write_manifest_archive(tmp_path / "emi", "ETC/FIRST")
    catalog_path = tmp_path / "emi_catalog.json"
    output_root = tmp_path / "review"
    write_catalog(catalog_path, archive_dir)

    result = assets_command.main(
        [
            "emi-review",
            "--catalog",
            str(catalog_path),
            "--output-root",
            str(output_root),
            "--family",
            "ETC",
        ]
    )

    assert result == 0
    payload = read_json(output_root / "review_manifest.json")
    assert payload["selected_archive_count"] == 1
    assert (output_root / "assets" / "ETC/FIRST").is_dir()


def test_emi_preview_writes_one_output_image(tmp_path: Path) -> None:
    image_path = tmp_path / "0.img"
    palette_path = tmp_path / "1.bin"
    output_path = tmp_path / "preview.png"
    image_path.write_bytes(make_indexed_image_payload())
    palette_path.write_bytes(make_palette_row())

    result = assets_command.main(
        [
            "emi-preview",
            str(image_path),
            str(palette_path),
            str(output_path),
        ]
    )

    assert result == 0
    assert output_path.is_file()


def test_emi_render_title_command_calls_specialized_renderer(
    tmp_path: Path,
    monkeypatch,
) -> None:
    calls: list[dict[str, Path | bool]] = []

    def fake_render_title_bundle(**kwargs):
        calls.append(kwargs)
        return {"banner": tmp_path / "banner.png"}

    monkeypatch.setattr(assets_command, "render_title_bundle", fake_render_title_bundle)

    result = assets_command.main(
        [
            "emi-render-title",
            "--etc-root",
            str(tmp_path / "ETC"),
            "--output-dir",
            str(tmp_path / "title"),
            "--clean",
        ]
    )

    assert result == 0
    assert calls[0]["first_path"] == tmp_path / "ETC" / "FIRST.EMI"
    assert calls[0]["output_dir"] == tmp_path / "title"
    assert calls[0]["clean"] is True


def test_emi_render_status_command_calls_specialized_renderer(
    tmp_path: Path,
    monkeypatch,
) -> None:
    calls: list[dict[str, Path | bool | None]] = []

    def fake_render_status_archive(**kwargs):
        calls.append(kwargs)
        return {"manifest": tmp_path / "status" / "manifest.json"}

    monkeypatch.setattr(
        assets_command, "render_status_archive", fake_render_status_archive
    )

    result = assets_command.main(
        [
            "emi-render-status",
            "--archive",
            str(tmp_path / "STATUS.EMI"),
            "--game-archive",
            str(tmp_path / "GAME.EMI"),
            "--output-dir",
            str(tmp_path / "status"),
        ]
    )

    assert result == 0
    assert calls[0]["archive_path"] == tmp_path / "STATUS.EMI"
    assert calls[0]["game_archive_path"] == tmp_path / "GAME.EMI"
