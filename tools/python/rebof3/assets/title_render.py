from __future__ import annotations

import json
import shutil
import struct
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

from PIL import Image

from .emi_archive import EmiArchive
from .psx_vram import PsxVram, tpage_from_type3_load_arg
from .title_bundle_metadata import title_bundle_metadata


TITLE_LAYOUT_COUNT = 20
TITLE_LAYOUT_TABLE = 0x801D1C6C
BANNER_LAYOUT_START = 11
BANNER_LAYOUT_COUNT = 4
BANNER_BASE_X_WORDS = 0x140
BANNER_PAGE_STRIDE_WORDS = 0x80
BANNER_TEXTURE_MODE = 1


@dataclass(frozen=True)
class TitleLayout:
    index: int
    u: int
    v: int
    width: int
    height: int
    palette_y: int

    @property
    def clut_word(self) -> int:
        return self.palette_y << 6


def load_title_layouts(game_emi: EmiArchive) -> list[TitleLayout]:
    entry = game_emi.entry(1)
    if entry.type_id != 0:
        raise ValueError("GAME.EMI entry 1 is not the expected binary blob")

    payload = game_emi.payload(1)
    table_offset = TITLE_LAYOUT_TABLE - entry.load_arg
    layouts: list[TitleLayout] = []
    for index in range(TITLE_LAYOUT_COUNT):
        u, v, width, height, palette_y = struct.unpack_from(
            "<HHHHH", payload, table_offset + index * 10
        )
        layouts.append(
            TitleLayout(
                index=index,
                u=u,
                v=v,
                width=width,
                height=height,
                palette_y=palette_y,
            )
        )
    return layouts


def banner_tpage(page_index: int) -> int:
    page_x_words = BANNER_BASE_X_WORDS + page_index * BANNER_PAGE_STRIDE_WORDS
    return ((page_x_words & 0x03FF) >> 6) | (BANNER_TEXTURE_MODE << 7)


def build_title_vram(first_emi: EmiArchive, demo_emi: EmiArchive) -> PsxVram:
    vram = PsxVram()
    vram.upload_rect(
        0, 0x01E0, 16, len(first_emi.payload(13)) // 32, first_emi.payload(13)
    )
    vram.upload_rect(
        0, 0x01E4, 256, len(demo_emi.payload(7)) // 512, demo_emi.payload(7)
    )

    for archive in (first_emi, demo_emi):
        for entry in archive.entries:
            if entry.type_id != 3 or entry.size == 0 or entry.size % 0x800 != 0:
                continue
            vram.upload_type3(entry, archive.payload(entry.index))

    return vram


def render_first_common_sheet(
    vram: PsxVram, first_emi: EmiArchive, output_dir: Path
) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    entry = first_emi.entry(3)
    image = vram.render_textured_rect(
        u=0,
        v=0,
        width=256,
        height=256,
        tpage=tpage_from_type3_load_arg(entry.load_arg, texture_mode=0),
        clut_word=0x01E0 << 6,
    )
    output_path = output_dir / "first_03_sheet.png"
    image.save(output_path)
    return output_path


def render_known_title_layouts(
    vram: PsxVram,
    layouts: list[TitleLayout],
    output_dir: Path,
) -> dict[str, Path]:
    output_dir.mkdir(parents=True, exist_ok=True)
    manifest: list[dict[str, object]] = []
    render_paths: list[Path] = []

    for page_index in range(BANNER_LAYOUT_COUNT):
        layout_index = BANNER_LAYOUT_START + page_index
        layout = layouts[layout_index]
        image = vram.render_textured_rect(
            u=layout.u,
            v=layout.v,
            width=layout.width,
            height=layout.height,
            tpage=banner_tpage(page_index),
            clut_word=layout.clut_word,
        )
        output_path = output_dir / f"layout{layout_index:02d}_fix.png"
        image.save(output_path)
        render_paths.append(output_path)
        manifest.append(
            {
                "label": f"banner_page_{page_index}",
                "filename": output_path.name,
                "layout_index": layout_index,
                "layout": asdict(layout),
                "clut_word": layout.clut_word,
            }
        )

    banner_sheet = Image.new("RGBA", (256 * 4, 192))
    for page_index, render_path in enumerate(render_paths):
        banner_sheet.paste(Image.open(render_path), (page_index * 256, 0))
    banner_path = output_dir / "banner_sheet_fix.png"
    banner_sheet.save(banner_path)

    manifest_path = output_dir / "manifest.json"
    manifest_path.write_text(
        json.dumps({"renders": manifest}, indent=2) + "\n", encoding="utf-8"
    )
    return {"manifest": manifest_path, "banner": banner_path}


def alpha_place(canvas: Image.Image, sprite: Image.Image, x: int, y: int) -> None:
    src_x = max(0, -x)
    src_y = max(0, -y)
    dst_x = max(0, x)
    dst_y = max(0, y)
    width = min(sprite.width - src_x, canvas.width - dst_x)
    height = min(sprite.height - src_y, canvas.height - dst_y)
    if width <= 0 or height <= 0:
        return
    clipped = sprite.crop((src_x, src_y, src_x + width, src_y + height))
    canvas.alpha_composite(clipped, (dst_x, dst_y))


def render_asset_bucket(
    vram: PsxVram,
    layouts: list[TitleLayout],
    output_dir: Path,
    asset_specs: list[dict[str, Any]],
    *,
    graph_type: int,
) -> tuple[dict[str, Image.Image], list[dict[str, Any]]]:
    output_dir.mkdir(parents=True, exist_ok=True)
    rendered: dict[str, Image.Image] = {}
    manifest_assets: list[dict[str, Any]] = []

    for asset_meta in asset_specs:
        layout = layouts[asset_meta["layout_index"]]
        tpage = int(asset_meta["tpage_by_graph_type"][str(graph_type)], 16)
        image = vram.render_textured_rect(
            u=layout.u,
            v=layout.v,
            width=layout.width,
            height=layout.height,
            tpage=tpage,
            clut_word=layout.clut_word,
        )
        output_path = output_dir / f"{asset_meta['name']}.png"
        image.save(output_path)
        rendered[asset_meta["name"]] = image
        manifest_assets.append(
            {
                **asset_meta,
                "filename": output_path.name,
                "layout": asdict(layout),
                "tpage_effective": f"0x{(tpage & 0x1FF):03x}",
                "clut_word": f"0x{layout.clut_word:04x}",
            }
        )

    return rendered, manifest_assets


def render_composites(
    rendered: dict[str, Image.Image],
    composite_specs: dict[str, Any],
    output_dir: Path,
) -> dict[str, Path]:
    output_dir.mkdir(parents=True, exist_ok=True)
    outputs: dict[str, Path] = {}
    for name, composite in composite_specs.items():
        canvas = Image.new(
            "RGBA",
            (composite["size"]["width"], composite["size"]["height"]),
            (0, 0, 0, 0),
        )
        for piece in composite["pieces"]:
            sprite = rendered.get(piece["name"])
            if sprite is not None:
                alpha_place(canvas, sprite, piece["x"], piece["y"])
        output_path = output_dir / f"{name}.png"
        canvas.save(output_path)
        outputs[name] = output_path
    return outputs


def render_draw_sequences(
    rendered: dict[str, Image.Image],
    draw_sequences: dict[str, Any],
    output_dir: Path,
) -> dict[str, Path]:
    outputs: dict[str, Path] = {}
    for sequence_name, sequence in draw_sequences.items():
        output = sequence.get("output")
        if output is None:
            continue
        canvas = Image.new(
            "RGBA",
            (output["size"]["width"], output["size"]["height"]),
            (0, 0, 0, 0),
        )
        for call in sequence.get("calls", []):
            for draw in call.get("draws", []):
                sprite = rendered.get(draw["asset"])
                if sprite is not None:
                    alpha_place(canvas, sprite, draw["x"], draw["y"])
        output_path = output_dir / output["filename"]
        output_path.parent.mkdir(parents=True, exist_ok=True)
        canvas.save(output_path)
        outputs[sequence_name] = output_path
    return outputs


def render_title_extracts(
    vram: PsxVram,
    layouts: list[TitleLayout],
    output_dir: Path,
    bundle_metadata: dict[str, Any],
) -> dict[str, Path]:
    asset_dir = output_dir / "title_assets"
    validated_dir = asset_dir / "validated"
    candidate_dir = asset_dir / "candidates"
    graph_type = bundle_metadata["validated_graph_type"]

    validated_rendered, validated_manifest = render_asset_bucket(
        vram,
        layouts,
        validated_dir,
        bundle_metadata["assets"]["validated"],
        graph_type=graph_type,
    )
    candidate_rendered, candidate_manifest = render_asset_bucket(
        vram,
        layouts,
        candidate_dir,
        bundle_metadata["assets"]["candidates"],
        graph_type=graph_type,
    )
    all_rendered = {**validated_rendered, **candidate_rendered}
    validated_composites = render_composites(
        validated_rendered, bundle_metadata["composites"]["validated"], output_dir
    )
    candidate_composites = render_composites(
        candidate_rendered, bundle_metadata["composites"]["candidates"], output_dir
    )
    sequence_outputs = render_draw_sequences(
        all_rendered, bundle_metadata.get("draw_sequences", {}), output_dir
    )

    manifest_path = output_dir / "title_assets_manifest.json"
    manifest_path.write_text(
        json.dumps(
            {
                "graph_type_branch": graph_type,
                "validated_assets": validated_manifest,
                "candidate_assets": candidate_manifest,
                "validated_composites": {
                    name: path.name for name, path in validated_composites.items()
                },
                "candidate_composites": {
                    name: path.name for name, path in candidate_composites.items()
                },
                "draw_sequence_outputs": {
                    name: path.name for name, path in sequence_outputs.items()
                },
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    return {
        "assets_manifest": manifest_path,
        **validated_composites,
        **candidate_composites,
        **sequence_outputs,
    }


def render_title_bundle(
    first_path: Path,
    demo_path: Path,
    game_path: Path,
    output_dir: Path,
    *,
    clean: bool = False,
) -> dict[str, Path]:
    if clean and output_dir.exists():
        shutil.rmtree(output_dir)

    first_emi = EmiArchive(first_path)
    demo_emi = EmiArchive(demo_path)
    game_emi = EmiArchive(game_path)
    layouts = load_title_layouts(game_emi)
    vram = build_title_vram(first_emi, demo_emi)
    outputs = render_known_title_layouts(vram, layouts, output_dir)
    outputs["first_sheet"] = render_first_common_sheet(vram, first_emi, output_dir)
    outputs.update(
        render_title_extracts(vram, layouts, output_dir, title_bundle_metadata())
    )
    return outputs
