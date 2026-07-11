---
type: Format
title: Graphics
description: Type-3 VRAM upload and indexed-palette layout.
tags: [graphics, vram]
---

# EMI graphics

Type-`3` entries contain raw upload bytes without a TIM header. The EMI load
argument encodes the VRAM destination and row span.

## Load argument

| Bits | Meaning | Unit |
| --- | --- | --- |
| `31:24` | base X | 32 VRAM words |
| `23:16` | base Y | 32 rows |
| `13:8` | chunks per row | `0x800` byte chunks |

## Chunk layout

Each `0x800` byte chunk uploads a `32x32` rectangle of 16-bit VRAM words.

| Texture mode | Pixel dimensions per chunk |
| --- | --- |
| 4bpp | `128x32` |
| 8bpp | `64x32` |
| 16bpp | `32x32` |

Chunks advance horizontally by 32 VRAM words. After the encoded span, the next
chunk row starts 32 pixels lower.

## Palette

Palette entries are raw little-endian PSX color words:

```text
15      14..10 9..5 4..0
STP     blue   green red
```

| Texture mode | Colors | Row size |
| --- | ---: | ---: |
| 4bpp | 16 | `0x20` |
| 8bpp | 256 | `0x200` |

Palette data commonly uses small type-`0` entries loaded into the
`0x80033xxx`–`0x8003axxx` region. The texture-to-palette association is supplied
by runtime drawing data, not the type-`3` payload.

The raw-texture and palette model agrees with the pinned
[`BoF3-Data-Doc`](../../../third_party/references/bof3-data-doc/src/DataStructures/3_TextureAndPalette.md);
the chunk geometry and descriptor fields are confirmed by the local loader and
VRAM reconstruction.
