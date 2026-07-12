from __future__ import annotations

from pathlib import Path

from .image_io import build_contact_sheet, rgba_to_image
from .indexed_image import (
    decode_indexed_image,
    decode_palette_rows,
    indices_to_rgba_bytes,
)


def preview_indexed_image(
    *,
    image_path: Path,
    palette_path: Path,
    output_path: Path,
    bpp: int,
    stripped_width: int,
    palette_row: int,
    contact_sheet: bool,
    columns: int,
    unstrip: bool,
) -> Path:
    decoded = decode_indexed_image(
        image_path.read_bytes(),
        bpp=bpp,
        stripped_width=stripped_width,
        unstrip=unstrip,
    )
    palette_rows = decode_palette_rows(palette_path.read_bytes(), bpp)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    if contact_sheet:
        images = [
            rgba_to_image(
                decoded.width,
                decoded.height,
                indices_to_rgba_bytes(
                    decoded.indices,
                    decoded.width,
                    decoded.height,
                    row,
                ),
            )
            for row in palette_rows
        ]
        build_contact_sheet(images, columns=columns).save(output_path)
        return output_path

    if palette_row < 0 or palette_row >= len(palette_rows):
        raise ValueError(f"palette row out of range: {palette_row}")

    rgba_to_image(
        decoded.width,
        decoded.height,
        indices_to_rgba_bytes(
            decoded.indices,
            decoded.width,
            decoded.height,
            palette_rows[palette_row],
        ),
    ).save(output_path)
    return output_path
