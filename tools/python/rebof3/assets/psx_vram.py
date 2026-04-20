from __future__ import annotations

from PIL import Image

from .emi_archive import EmiEntry


VRAM_WIDTH_WORDS = 1024
VRAM_HEIGHT = 512
VRAM_ROW_BYTES = VRAM_WIDTH_WORDS * 2


def psx_color_to_rgba(raw: bytes) -> tuple[int, int, int, int]:
    word = int.from_bytes(raw, "little")
    red = (word << 3) & 0xF8
    green = (word >> 2) & 0xF8
    blue = (word >> 7) & 0xF8
    stp = (word >> 15) & 1
    alpha = 0 if red == 0 and green == 0 and blue == 0 and stp == 0 else 255
    return red, green, blue, alpha


def tpage_from_type3_load_arg(load_arg: int, texture_mode: int) -> int:
    base_x_words = ((load_arg >> 24) & 0x3F) << 5
    base_y = ((load_arg >> 16) & 0x1F) << 5
    return (
        ((base_x_words >> 6) & 0x0F)
        | (((base_y >> 8) & 0x01) << 4)
        | (texture_mode << 7)
    )


class PsxVram:
    def __init__(self) -> None:
        self.bytes = bytearray(VRAM_ROW_BYTES * VRAM_HEIGHT)

    def upload_rect(
        self,
        x_words: int,
        y: int,
        width_words: int,
        height: int,
        payload: bytes,
    ) -> None:
        stride = width_words * 2
        expected_size = stride * height
        if len(payload) != expected_size:
            raise ValueError(
                f"rect payload size mismatch: got {len(payload)}, expected {expected_size}"
            )
        if (
            x_words < 0
            or y < 0
            or x_words + width_words > VRAM_WIDTH_WORDS
            or y + height > VRAM_HEIGHT
        ):
            raise ValueError("rect upload exceeds VRAM bounds")

        for row in range(height):
            src_offset = row * stride
            dst_offset = (y + row) * VRAM_ROW_BYTES + x_words * 2
            self.bytes[dst_offset : dst_offset + stride] = payload[
                src_offset : src_offset + stride
            ]

    def upload_type3(self, entry: EmiEntry, payload: bytes) -> None:
        if entry.type_id != 3 or len(payload) % 0x800 != 0:
            raise ValueError(f"not a chunked type-3 payload: entry {entry.index}")

        base_x = ((entry.load_arg >> 24) & 0x3F) << 5
        base_y = ((entry.load_arg >> 16) & 0x1F) << 5
        span = (entry.load_arg >> 8) & 0x3F
        if span <= 0:
            raise ValueError(f"invalid type-3 span for entry {entry.index}")

        for chunk_index in range(len(payload) // 0x800):
            chunk_x = (base_x + (chunk_index % span) * 32) & 0x03FF
            chunk_y = (base_y + (chunk_index // span) * 32) & 0x01FF
            chunk = payload[chunk_index * 0x800 : (chunk_index + 1) * 0x800]
            self.upload_rect(chunk_x, chunk_y, 32, 32, chunk)

    def palette_rgba(self, clut_word: int, bpp: int) -> list[tuple[int, int, int, int]]:
        color_count = 16 if bpp == 4 else 256
        x_words = (clut_word & 0x3F) << 4
        y = clut_word >> 6
        row_offset = y * VRAM_ROW_BYTES + x_words * 2
        raw = self.bytes[row_offset : row_offset + color_count * 2]
        return [
            psx_color_to_rgba(raw[offset : offset + 2])
            for offset in range(0, len(raw), 2)
        ]

    def render_textured_rect(
        self,
        *,
        u: int,
        v: int,
        width: int,
        height: int,
        tpage: int,
        clut_word: int | None = None,
    ) -> Image.Image:
        tpage &= 0x1FF
        texture_mode = (tpage >> 7) & 0x3
        base_x_words = (tpage & 0x0F) * 64
        base_y = ((tpage >> 4) & 0x01) * 256
        image = Image.new("RGBA", (width, height))
        pixels = image.load()

        if texture_mode in (0, 1):
            if clut_word is None:
                raise ValueError("indexed texture render requires a CLUT word")
            palette = self.palette_rgba(clut_word, 4 if texture_mode == 0 else 8)
            base_x_bytes = base_x_words * 2
            for y_offset in range(height):
                row_offset = (base_y + v + y_offset) * VRAM_ROW_BYTES + base_x_bytes
                for x_offset in range(width):
                    texel_x = u + x_offset
                    if texture_mode == 1:
                        index = self.bytes[row_offset + texel_x]
                    else:
                        packed = self.bytes[row_offset + (texel_x >> 1)]
                        index = packed & 0x0F if (texel_x & 1) == 0 else packed >> 4
                    pixels[x_offset, y_offset] = palette[index]
            return image

        if texture_mode == 2:
            for y_offset in range(height):
                row_offset = (base_y + v + y_offset) * VRAM_ROW_BYTES + (
                    base_x_words + u
                ) * 2
                for x_offset in range(width):
                    pixels[x_offset, y_offset] = psx_color_to_rgba(
                        self.bytes[
                            row_offset + x_offset * 2 : row_offset + x_offset * 2 + 2
                        ]
                    )
            return image

        raise ValueError(f"unsupported texture mode: {texture_mode}")


def iter_vram_safe_type3_entries(entries: list[EmiEntry]) -> list[EmiEntry]:
    safe_entries: list[EmiEntry] = []
    for entry in entries:
        if entry.type_id != 3 or entry.size == 0 or entry.size % 0x800 != 0:
            continue

        base_x = ((entry.load_arg >> 24) & 0x3F) << 5
        base_y = ((entry.load_arg >> 16) & 0x1F) << 5
        span = (entry.load_arg >> 8) & 0x3F
        if span <= 0:
            continue

        chunk_count = entry.size // 0x800
        row_count = (chunk_count + span - 1) // span
        width_words = span * 32
        height = row_count * 32
        if base_x + width_words > VRAM_WIDTH_WORDS or base_y + height > VRAM_HEIGHT:
            continue

        safe_entries.append(entry)
    return safe_entries
