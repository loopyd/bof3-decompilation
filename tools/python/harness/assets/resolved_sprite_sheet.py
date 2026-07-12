from __future__ import annotations

from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

from PIL import Image, ImageDraw, ImageFont

from .psx_vram import PsxVram


@dataclass(frozen=True)
class ResolvedSpriteSpec:
    name: str
    u: int
    v: int
    width: int
    height: int
    tpage: int
    clut_word: int
    source_entry: int | None = None
    notes: tuple[str, ...] = ()


def render_resolved_atlas(
    vram: PsxVram,
    sprites: list[ResolvedSpriteSpec],
    output_dir: Path,
    *,
    basename: str,
    canvas_size: tuple[int, int] | None = None,
    emit_labeled: bool = False,
) -> dict[str, Any]:
    if not sprites:
        raise ValueError("resolved atlas requires at least one sprite")

    output_dir.mkdir(parents=True, exist_ok=True)
    font = ImageFont.load_default()
    if canvas_size is None:
        atlas_width = max(sprite.u + sprite.width for sprite in sprites)
        atlas_height = max(sprite.v + sprite.height for sprite in sprites)
    else:
        atlas_width, atlas_height = canvas_size

    atlas = Image.new("RGBA", (atlas_width, atlas_height), (0, 0, 0, 0))
    labeled = (
        Image.new("RGBA", (atlas_width, atlas_height), (0, 0, 0, 0))
        if emit_labeled
        else None
    )
    metadata: list[dict[str, Any]] = []

    for sprite in sprites:
        image = vram.render_textured_rect(
            u=sprite.u,
            v=sprite.v,
            width=sprite.width,
            height=sprite.height,
            tpage=sprite.tpage,
            clut_word=sprite.clut_word,
        )
        atlas.alpha_composite(image, (sprite.u, sprite.v))
        if labeled is not None:
            labeled.alpha_composite(image, (sprite.u, sprite.v))
            draw = ImageDraw.Draw(labeled)
            draw.rectangle(
                (
                    sprite.u,
                    sprite.v,
                    sprite.u + sprite.width - 1,
                    sprite.v + sprite.height - 1,
                ),
                outline=(255, 255, 255, 255),
            )
            draw.text(
                (sprite.u + 1, sprite.v + 1),
                sprite.name,
                fill=(255, 255, 255, 255),
                font=font,
            )

        metadata.append(
            {
                **asdict(sprite),
                "tpage_effective": f"0x{(sprite.tpage & 0x1FF):03x}",
                "clut_word_hex": f"0x{sprite.clut_word:04x}",
            }
        )

    atlas_path = output_dir / f"{basename}.png"
    atlas.save(atlas_path)
    labeled_path = None
    if labeled is not None:
        labeled_path = output_dir / f"{basename}_labeled.png"
        labeled.save(labeled_path)
    return {"atlas": atlas_path, "atlas_labeled": labeled_path, "sprites": metadata}
