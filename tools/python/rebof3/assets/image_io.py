from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw


def rgba_to_image(width: int, height: int, rgba: bytes) -> Image.Image:
    return Image.frombytes("RGBA", (width, height), rgba)


def save_rgba_image(width: int, height: int, rgba: bytes, output_path: Path) -> Path:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    rgba_to_image(width, height, rgba).save(output_path)
    return output_path


def build_contact_sheet(
    images: list[Image.Image],
    *,
    columns: int,
    labels: list[str] | None = None,
) -> Image.Image:
    if not images:
        raise ValueError("no images to place in contact sheet")

    columns = max(1, columns)
    width, height = images[0].size
    rows = (len(images) + columns - 1) // columns
    label_height = 12 if labels else 0
    sheet = Image.new("RGBA", (width * columns, (height + label_height) * rows))
    draw = ImageDraw.Draw(sheet) if labels else None

    for index, image in enumerate(images):
        x = (index % columns) * width
        y = (index // columns) * (height + label_height)
        if labels and draw is not None:
            draw.text((x + 2, y), labels[index], fill=(255, 255, 255, 255))
        sheet.paste(image, (x, y + label_height))

    return sheet


def palette_preview_image(rows: list[list[tuple[int, int, int, int]]]) -> Image.Image:
    if not rows:
        raise ValueError("palette preview requires at least one row")

    colors_per_row = len(rows[0])
    swatch_size = 8 if colors_per_row <= 16 else 2
    image = Image.new(
        "RGBA",
        (colors_per_row * swatch_size, len(rows) * swatch_size),
    )
    for row_index, row in enumerate(rows):
        for color_index, rgba in enumerate(row):
            tile = Image.new("RGBA", (swatch_size, swatch_size), rgba)
            image.paste(tile, (color_index * swatch_size, row_index * swatch_size))
    return image
