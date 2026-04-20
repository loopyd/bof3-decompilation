from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class DecodedIndexedImage:
    width: int
    height: int
    indices: bytes


def unpack_pixels(raw: bytes, bpp: int) -> bytearray:
    if bpp == 8:
        return bytearray(raw)
    if bpp != 4:
        raise ValueError(f"unsupported bpp: {bpp}")

    out = bytearray(len(raw) * 2)
    for index, packed in enumerate(raw):
        out[index * 2] = packed & 0x0F
        out[index * 2 + 1] = packed >> 4
    return out


def unstrip_encoded(raw: bytes, stripped_width: int) -> bytearray:
    if (
        stripped_width <= 0
        or (stripped_width % 64) != 0
        or len(raw) == 0
        or (len(raw) % 0x800) != 0
    ):
        raise ValueError("image size is not compatible with stripped width")

    span = stripped_width // 64
    chunk_count = len(raw) // 0x800
    row_blocks = (chunk_count + span - 1) // span
    out = bytearray(stripped_width * row_blocks * 32)

    for chunk_index in range(chunk_count):
        chunk = raw[chunk_index * 0x800 : (chunk_index + 1) * 0x800]
        chunk_x = (chunk_index % span) * 64
        chunk_y = (chunk_index // span) * 32
        for row in range(32):
            src_offset = row * 64
            dst_offset = (chunk_y + row) * stripped_width + chunk_x
            out[dst_offset : dst_offset + 64] = chunk[src_offset : src_offset + 64]
    return out


def decode_indexed_image(
    raw: bytes,
    *,
    bpp: int,
    stripped_width: int,
    unstrip: bool = True,
) -> DecodedIndexedImage:
    encoded = bytes(unstrip_encoded(raw, stripped_width)) if unstrip else raw
    indices = unpack_pixels(encoded, bpp)
    width = stripped_width * (2 if bpp == 4 else 1)
    if width <= 0 or len(indices) % width != 0:
        raise ValueError("decoded image size does not match width")
    return DecodedIndexedImage(
        width=width,
        height=len(indices) // width,
        indices=bytes(indices),
    )


def decode_palette_rows(raw: bytes, bpp: int) -> list[list[tuple[int, int, int, int]]]:
    if bpp == 4:
        colors_per_row = 16
    elif bpp == 8:
        colors_per_row = 256
    else:
        raise ValueError(f"unsupported bpp: {bpp}")

    row_bytes = colors_per_row * 2
    if len(raw) % row_bytes != 0:
        raise ValueError("palette size is not aligned to row width")

    rows: list[list[tuple[int, int, int, int]]] = []
    for row_offset in range(0, len(raw), row_bytes):
        row: list[tuple[int, int, int, int]] = []
        for color_offset in range(row_offset, row_offset + row_bytes, 2):
            pixel = raw[color_offset] | (raw[color_offset + 1] << 8)
            red = (pixel << 3) & 0xF8
            green = (pixel >> 2) & 0xF8
            blue = (pixel >> 7) & 0xF8
            stp = (pixel >> 15) & 1
            alpha = 0 if red == 0 and green == 0 and blue == 0 and stp == 0 else 255
            row.append((red, green, blue, alpha))
        rows.append(row)
    return rows


def indices_to_rgba_bytes(
    indices: bytes,
    width: int,
    height: int,
    palette_row: list[tuple[int, int, int, int]],
) -> bytes:
    rgba = bytearray(width * height * 4)
    for index, palette_index in enumerate(indices):
        red, green, blue, alpha = palette_row[palette_index]
        base = index * 4
        rgba[base] = red
        rgba[base + 1] = green
        rgba[base + 2] = blue
        rgba[base + 3] = alpha
    return bytes(rgba)


def infer_stripped_width(load_arg: int) -> int:
    span = (load_arg >> 8) & 0x3F
    if span <= 0:
        raise ValueError(
            f"load arg does not encode a valid image span: 0x{load_arg:08x}"
        )
    return span * 64


def infer_bpp(image_size: int, palette_size: int | None = None) -> int:
    if palette_size is not None and palette_size > 512:
        return 8
    if image_size <= 0x8000:
        return 4
    return 8
