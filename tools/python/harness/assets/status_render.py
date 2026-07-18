from __future__ import annotations

import json
import shutil
import struct
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

from .emi_archive import EmiArchive, EmiEntry
from .family_render_outputs import write_candidate_sheet, write_index_preview
from .indexed_image import decode_palette_rows
from .psx_vram import PsxVram, iter_vram_safe_type3_entries, tpage_from_type3_load_arg
from .resolved_sprite_sheet import ResolvedSpriteSpec, render_resolved_atlas


STATUS_OVERLAY_LOAD = 0x801D0C00
STATUS_PORTRAIT_TABLE = 0x801EC96C
GAME_OVERLAY_LOAD = 0x80195800
GAME_SHARED_STATUS_TABLE_A = 0x801CCE84
GAME_SHARED_STATUS_TABLE_B = 0x801CCF7C
GAME_SHARED_STATUS_TABLE_RECORD_COUNT = 0x40
STATUS_PALETTE_BANK_BASE = 0x80033800
STATUS_PALETTE_BANK_STRIDE = 0x200
STATUS_PALETTE_BANK_ROWS = 0x20
PORTRAIT_WIDTH = 0x28
PORTRAIT_HEIGHT = 0x30
TEXTURE_PAGE_WIDTH = 0x100
TEXTURE_PAGE_HEIGHT = 0x100
PORTRAIT_TEXTURE_LOAD_ARG = 0x1C080200
STATUS_BACKGROUND_TEXTURE_LOAD_ARG = 0x1A080200

STATUS_LOCAL_LAYOUT_TABLE_ADDRESSES = (
    0x801EC598,
    0x801EC628,
    0x801EC654,
    0x801EC660,
    0x801EC678,
    0x801EC684,
    0x801EC690,
    0x801EC69C,
    0x801EC6B0,
    0x801EC6D4,
    0x801EC6E0,
    0x801EC6F8,
    0x801EC728,
    0x801EC754,
    0x801EC760,
    0x801EC76C,
    0x801EC778,
    0x801EC784,
    0x801EC790,
    0x801EC79C,
    0x801EC7F8,
    0x801EC854,
    0x801ECFB8,
    0x801ED05C,
    0x801ED468,
    0x801ED48C,
    0x801ED498,
    0x801ED5F0,
    0x801ED5FC,
)

STATUS_SHARED_HELPER_SPECS = (
    {
        "function": "0x801dbbcc",
        "layout_tables": [(0x801EC628, 0), (0x801EC654, 0), (0x801EC660, 0)],
    },
    {"function": "0x801de5d4", "layout_tables": [(0x801EC728, 1)]},
    {"function": "0x801dfbf4", "layout_tables": [(0x801EC76C, 1), (0x801EC778, 1)]},
    {
        "function": "0x801e0354",
        "layout_tables": [
            (0x801EC754, 1),
            (0x801EC76C, 1),
            (0x801EC784, 1),
            (0x801EC790, 1),
        ],
    },
    {"function": "0x801e0a70", "layout_tables": [(0x801EC754, 1), (0x801EC760, 1)]},
    {"function": "0x801e11a8", "layout_tables": [(0x801EC854, 0), (0x801EC854, 2)]},
)


@dataclass(frozen=True)
class StatusPortraitRecord:
    index: int
    u: int
    v: int
    clut_x_byte: int
    clut_y_selector: int

    @property
    def clut_row(self) -> int:
        return self.clut_x_byte >> 4

    @property
    def clut_word(self) -> int:
        return self.clut_row | ((self.clut_y_selector + 0x01E0) << 6)


def find_overlay_entry(archive: EmiArchive) -> EmiEntry:
    return next(
        entry
        for entry in archive.entries
        if entry.type_id == 0 and entry.load_arg == STATUS_OVERLAY_LOAD
    )


def find_portrait_texture_entry(archive: EmiArchive) -> EmiEntry:
    return next(
        entry
        for entry in archive.entries
        if entry.type_id == 3 and entry.load_arg == PORTRAIT_TEXTURE_LOAD_ARG
    )


def find_background_texture_entry(archive: EmiArchive) -> EmiEntry:
    return next(
        entry
        for entry in archive.entries
        if entry.type_id == 3 and entry.load_arg == STATUS_BACKGROUND_TEXTURE_LOAD_ARG
    )


def find_game_overlay_entry(archive: EmiArchive) -> EmiEntry:
    return next(
        entry
        for entry in archive.entries
        if entry.type_id == 0 and entry.load_arg == GAME_OVERLAY_LOAD
    )


def extract_status_layout_table(
    overlay_payload: bytes, table_address: int, *, max_entries: int = 0x40
) -> dict[str, object]:
    offset = table_address - STATUS_OVERLAY_LOAD
    entries: list[dict[str, int]] = []
    for _ in range(max_entries):
        x_tiles, y_tiles, sprite_id = struct.unpack_from(
            "<BBB", overlay_payload, offset
        )
        offset += 3
        if sprite_id == 0xFF:
            break
        entries.append({"x_tiles": x_tiles, "y_tiles": y_tiles, "sprite_id": sprite_id})
    return {"address": f"0x{table_address:08x}", "entries": entries}


def extract_game_shared_sprite_table(
    game_payload: bytes, table_address: int
) -> dict[str, object]:
    offset = table_address - GAME_OVERLAY_LOAD
    records: list[dict[str, int]] = []
    for sprite_id in range(GAME_SHARED_STATUS_TABLE_RECORD_COUNT):
        u, v, width, height = struct.unpack_from(
            "<BBBB", game_payload, offset + sprite_id * 4
        )
        records.append(
            {"sprite_id": sprite_id, "u": u, "v": v, "width": width, "height": height}
        )
    return {"address": f"0x{table_address:08x}", "records": records}


def build_status_shared_metadata(
    status_overlay_payload: bytes, game_archive_path: Path | None
) -> dict[str, object]:
    metadata: dict[str, object] = {
        "local_layout_tables": {
            f"0x{address:08x}": extract_status_layout_table(
                status_overlay_payload, address
            )
            for address in STATUS_LOCAL_LAYOUT_TABLE_ADDRESSES
        }
    }
    if game_archive_path is None or not game_archive_path.exists():
        metadata["shared_tables_available"] = False
        return metadata

    game_archive = EmiArchive(game_archive_path)
    game_payload = game_archive.payload(find_game_overlay_entry(game_archive).index)
    metadata["shared_tables_available"] = True
    metadata["shared_game_archive"] = str(game_archive_path)
    metadata["shared_sprite_tables"] = {
        "D_801CCE84": extract_game_shared_sprite_table(
            game_payload, GAME_SHARED_STATUS_TABLE_A
        ),
        "D_801CCF7C": extract_game_shared_sprite_table(
            game_payload, GAME_SHARED_STATUS_TABLE_B
        ),
    }
    metadata["status_helper_inventory"] = list(STATUS_SHARED_HELPER_SPECS)
    return metadata


def extract_portrait_records(overlay_payload: bytes) -> list[StatusPortraitRecord]:
    table_offset = STATUS_PORTRAIT_TABLE - STATUS_OVERLAY_LOAD
    records: list[StatusPortraitRecord] = []
    for index in range(16):
        u, v, clut_x, clut_y = struct.unpack_from(
            "<BBBB", overlay_payload, table_offset + index * 4
        )
        if u >= 0x100 or v >= 0x100 or (clut_x & 0x0F) != 0 or clut_y not in {1, 2}:
            break
        records.append(
            StatusPortraitRecord(
                index=index, u=u, v=v, clut_x_byte=clut_x, clut_y_selector=clut_y
            )
        )
    if not records:
        raise ValueError("failed to recover portrait records from STATUS overlay")
    return records


def build_status_vram(archive: EmiArchive) -> tuple[PsxVram, list[dict[str, int]]]:
    vram = PsxVram()
    palette_uploads: list[dict[str, int]] = []
    palette_bank = bytearray(256 * STATUS_PALETTE_BANK_ROWS * 2)

    for entry in iter_vram_safe_type3_entries(archive.entries):
        vram.upload_type3(entry, archive.payload(entry.index))

    for entry in archive.entries:
        if entry.type_id != 0 or entry.size != STATUS_PALETTE_BANK_STRIDE:
            continue
        if not (
            STATUS_PALETTE_BANK_BASE
            <= entry.load_arg
            < STATUS_PALETTE_BANK_BASE + len(palette_bank)
        ):
            continue
        bank_offset = entry.load_arg - STATUS_PALETTE_BANK_BASE
        payload = archive.payload(entry.index)
        palette_bank[bank_offset : bank_offset + len(payload)] = payload
        palette_uploads.append(
            {
                "entry_index": entry.index,
                "load_arg": entry.load_arg,
                "bank_offset": bank_offset,
                "target_clut_y": 0x01E0 + bank_offset // STATUS_PALETTE_BANK_STRIDE,
            }
        )

    vram.upload_rect(0, 0x01E0, 256, STATUS_PALETTE_BANK_ROWS, bytes(palette_bank))
    return vram, palette_uploads


def render_palette_preview(raw: bytes, output_path: Path) -> None:
    from PIL import Image, ImageDraw

    rows = decode_palette_rows(raw, 4)
    swatch = 12
    image = Image.new("RGBA", (16 * swatch, len(rows) * swatch))
    draw = ImageDraw.Draw(image)
    for row_index, row in enumerate(rows):
        for col_index, color in enumerate(row):
            x0 = col_index * swatch
            y0 = row_index * swatch
            draw.rectangle((x0, y0, x0 + swatch - 1, y0 + swatch - 1), fill=color)
    image.save(output_path)


def render_portrait_sheet(
    vram: PsxVram,
    portrait_entry: EmiEntry,
    records: list[StatusPortraitRecord],
    output_dir: Path,
) -> list[dict[str, object]]:
    from PIL import Image

    output_dir.mkdir(parents=True, exist_ok=True)
    columns = 4
    sheet = Image.new(
        "RGBA",
        (
            columns * PORTRAIT_WIDTH,
            ((len(records) + columns - 1) // columns) * PORTRAIT_HEIGHT,
        ),
        (0, 0, 0, 0),
    )
    tpage = tpage_from_type3_load_arg(portrait_entry.load_arg, texture_mode=0)
    metadata: list[dict[str, object]] = []

    for record in records:
        portrait = vram.render_textured_rect(
            u=record.u,
            v=record.v,
            width=PORTRAIT_WIDTH,
            height=PORTRAIT_HEIGHT,
            tpage=tpage,
            clut_word=record.clut_word,
        )
        sheet.paste(
            portrait,
            (
                (record.index % columns) * PORTRAIT_WIDTH,
                (record.index // columns) * PORTRAIT_HEIGHT,
            ),
        )
        metadata.append(
            {
                **asdict(record),
                "clut_row": record.clut_row,
                "clut_word": record.clut_word,
            }
        )

    sheet.save(output_dir / "portraits_sheet.png")
    return metadata


def render_portrait_atlas(
    vram: PsxVram,
    portrait_entry: EmiEntry,
    records: list[StatusPortraitRecord],
    output_dir: Path,
) -> tuple[dict[str, Any], dict[str, Any]]:
    tpage = tpage_from_type3_load_arg(portrait_entry.load_arg, texture_mode=0)
    sprites = [
        ResolvedSpriteSpec(
            name=f"portrait_{record.index:02d}",
            u=record.u,
            v=record.v,
            width=PORTRAIT_WIDTH,
            height=PORTRAIT_HEIGHT,
            tpage=tpage,
            clut_word=record.clut_word,
            source_entry=portrait_entry.index,
            notes=("STATUS portrait table",),
        )
        for record in records
    ]
    return (
        render_resolved_atlas(
            vram,
            sprites,
            output_dir,
            basename=f"{portrait_entry.index}__validated_atlas",
        ),
        render_resolved_atlas(
            vram,
            sprites,
            output_dir,
            basename=f"{portrait_entry.index}__validated_page",
            canvas_size=(TEXTURE_PAGE_WIDTH, TEXTURE_PAGE_HEIGHT),
        ),
    )


def render_unresolved_background_candidates(
    vram: PsxVram, background_entry: EmiEntry, output_dir: Path
) -> dict[str, object]:
    rows = [0, 1, 2, 3]
    row_images = [
        vram.render_textured_rect(
            u=0,
            v=0,
            width=256,
            height=256,
            tpage=tpage_from_type3_load_arg(background_entry.load_arg, texture_mode=0),
            clut_word=((0x01E0 + row) << 6),
        )
        for row in rows
    ]
    output_path = output_dir / f"{background_entry.index}__candidate_rows.png"
    write_candidate_sheet(row_images, output_path, columns=2)
    return {
        "entry_index": background_entry.index,
        "output": output_path.name,
        "candidate_rows": rows,
    }


def build_status_manifest(
    archive_path: Path,
    game_archive_path: Path | None,
    overlay_entry: EmiEntry,
    portrait_entry: EmiEntry,
    background_entry: EmiEntry,
    palette_uploads: list[dict[str, int]],
    shared_status_metadata: dict[str, object],
    portrait_metadata: list[dict[str, object]],
    portrait_atlas: dict[str, Any],
    portrait_resolved_page: dict[str, Any],
    unresolved_background: dict[str, object],
) -> dict[str, object]:
    return {
        "archive": str(archive_path),
        "shared_game_archive": None
        if game_archive_path is None
        else str(game_archive_path),
        "overlay_entry": overlay_entry.index,
        "portrait_texture_entry": portrait_entry.index,
        "background_texture_entry": background_entry.index,
        "palette_uploads": palette_uploads,
        "shared_status_metadata": shared_status_metadata,
        "portraits": portrait_metadata,
        "resolved_atlas": {
            "atlas": Path(str(portrait_atlas["atlas"])).name,
            "sprites": portrait_atlas["sprites"],
        },
        "resolved_pages": {
            f"entry_{portrait_entry.index}": {
                "atlas": Path(str(portrait_resolved_page["atlas"])).name,
                "sprites": portrait_resolved_page["sprites"],
            }
        },
        "unresolved_background": unresolved_background,
    }


def render_status_archive(
    archive_path: Path,
    output_dir: Path,
    *,
    game_archive_path: Path | None = None,
    clean: bool = False,
) -> dict[str, Path]:
    if clean and output_dir.exists():
        shutil.rmtree(output_dir)

    archive = EmiArchive(archive_path)
    overlay_entry = find_overlay_entry(archive)
    portrait_entry = find_portrait_texture_entry(archive)
    background_entry = find_background_texture_entry(archive)
    overlay_payload = archive.payload(overlay_entry.index)
    records = extract_portrait_records(overlay_payload)
    vram, palette_uploads = build_status_vram(archive)
    shared_status_metadata = build_status_shared_metadata(
        overlay_payload, game_archive_path
    )

    output_dir.mkdir(parents=True, exist_ok=True)
    write_index_preview(
        archive.payload(portrait_entry.index),
        portrait_entry.size,
        portrait_entry.load_arg,
        output_dir / f"{portrait_entry.index}__indices.png",
    )
    write_index_preview(
        archive.payload(background_entry.index),
        background_entry.size,
        background_entry.load_arg,
        output_dir / f"{background_entry.index}__indices.png",
    )
    portrait_metadata = render_portrait_sheet(vram, portrait_entry, records, output_dir)
    portrait_atlas, portrait_resolved_page = render_portrait_atlas(
        vram, portrait_entry, records, output_dir
    )
    unresolved_background = render_unresolved_background_candidates(
        vram, background_entry, output_dir
    )

    for palette_upload in palette_uploads:
        render_palette_preview(
            archive.payload(palette_upload["entry_index"]),
            output_dir / f"palette_entry_{palette_upload['entry_index']:02d}.png",
        )

    manifest_path = output_dir / "manifest.json"
    manifest_path.write_text(
        json.dumps(
            build_status_manifest(
                archive_path,
                game_archive_path,
                overlay_entry,
                portrait_entry,
                background_entry,
                palette_uploads,
                shared_status_metadata,
                portrait_metadata,
                portrait_atlas,
                portrait_resolved_page,
                unresolved_background,
            ),
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    return {
        "manifest": manifest_path,
        "sheet": output_dir / "portraits_sheet.png",
        "atlas": output_dir / f"{portrait_entry.index}__validated_atlas.png",
        "resolved_page": output_dir / f"{portrait_entry.index}__validated_page.png",
        "indices_1": output_dir / f"{portrait_entry.index}__indices.png",
        "indices_2": output_dir / f"{background_entry.index}__indices.png",
        "background_candidates": output_dir
        / f"{background_entry.index}__candidate_rows.png",
    }
