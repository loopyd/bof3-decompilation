---
type: Format
title: Graphics
description: Qualified US graphics storage, palette selection, and reconstruction limits.
tags: [graphics, vram]
---

# EMI graphics

Type-`3` entries contain raw upload bytes without a TIM header. The EMI load
argument encodes the VRAM destination and row span.

## Load argument

| Bits | Meaning | Unit |
| --- | --- | --- |
| `29:24` | base X | 32 VRAM words |
| `20:16` | base Y | 32 rows |
| `13:8` | chunks per row | `0x800` byte chunks |

The producer at `SLUS_004.22@0x80162698` masks these fields with `0x3f`,
`0x1f` and `0x3f` respectively. The descriptor does not select GPU bit depth.

## Chunk layout

Each `0x800` byte chunk uploads a `32x32` rectangle of 16-bit VRAM words.

| Texture mode | Pixel dimensions per chunk |
| --- | --- |
| 4bpp | `128x32` |
| 8bpp | `64x32` |
| 16bpp | `32x32` |

Chunks advance horizontally by 32 VRAM words. After the encoded span, the next
chunk row starts 32 pixels lower.

The queue stores X in one byte and the upload consumer masks Y with `0x3f`.
`SLUS_004.22@0x80163378` submits a chunk only when its complete rectangle fits
within 1024 words by 512 rows. Skipped source chunks remain part of the archive
and its editable package. They do not wrap into visible VRAM.

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

Index zero does not imply transparency. The selected palette word and GPU draw
state determine the result. Preserve all 16 bits, including duplicate colors,
word zero and STP black. A word-preserving RGBA export is an editing view; it
does not establish that an original consumer uses direct-color rendering.

## Qualified title panels

`BIN/ETC/DEMO.EMI#6` uploads 512 words by 256 rows at VRAM `(320, 0)`.
The four-panel loop at `BIN/ETC/GAME.EMI#1@0x801D18F8` selects 8-bit pages
`0x85`, `0x87`, `0x89` and `0x8b`. Records 11 through 14 in its 10-byte table
at `0x801D1C6C` each sample UV `(0, 0)` with width 256 and height 192.

| Panel | VRAM X in words | CLUT row | Palette bytes within DEMO entry 7 |
| ---: | ---: | ---: | --- |
| 0 | 320 | 487 | `1536..2048` |
| 1 | 448 | 488 | `2048..2560` |
| 2 | 576 | 489 | `2560..3072` |
| 3 | 704 | 490 | `3072..3584` |

These are conditional static associations. The consumer samples only the first
192 rows of each 256-row panel. Whole-atlas palette previews do not reproduce
the assembled screen, scrolling or fades. Upload execution, intervening RAM and
VRAM writes, and rendered appearance require runtime observations.

## Sprite storage and records

The [texture tool](../../../tools/rust/bof3-textures/README.md) owns checked
decoding and inverse encoding for the qualified US layouts:

| Storage | Original US consumer | Contract |
| --- | --- | --- |
| Outer compressed bank | `SLUS_004.22@0x80164064` | Preserve original streams; edited output must fit its consumed allocation. |
| Raw-word bank image | `SLUS_004.22@0x8014E6D0` | Uncompressed words follow an eight-byte image header. Dimensions and consumed allocation stay fixed. |
| Mode-1 image | `SLUS_004.22@0x8014E7F8` | Six packed five-bit values per word, with unused bits retained. |
| Mode-2 image | `SLUS_004.22@0x8014E8FC` | Bounded LZSS with a 512-byte dictionary and preserved prefix and final-token overshoot. |
| Three-root bank | `BIN/ETC/GAME.EMI#0@0x801C1254` | Separate image, palette, program and selection sections; selection tails remain opaque. |
| Standalone sprite root | `SLUS_004.22@0x8014DC08` | The root starts at payload offset zero with entry-relative section offsets. Active root installation remains unresolved. |
| Sprite palette record | `SLUS_004.22@0x80166C48` | Each record supplies 32 exact color words. It does not establish a complete draw CLUT or its neighboring colors. |
| Counted sprite definitions | `SLUS_004.22@0x8014DD3C` | Eight-byte records follow a four-byte header. Draw-state edits retain IDs, groups and order; texel and palette sources remain unresolved. |
| Resident programs | `SLUS_004.22@0x8014D878` | Relative group and program tables; source-bound cell offsets and bounded step pairs. |
| Cell parts | `SLUS_004.22@0x8014C6F8` | Five-byte parts; size table at `0x8017F53C`; geometry depends on entity state. |
| Selected area surface words | `SLUS_004.22@0x80154294`, `0x80155560` | Directly selected packed words are editable. Coordinate-read spans stay fixed; complete geometry and source bindings remain unresolved. |

An image's compression mode does not establish its GPU texture mode. Parsed
programs and cells do not establish character identities, direction mappings,
resident loading or rendered frames. See the tool's
[catalog record contract](../../../tools/rust/bof3-textures/docs/catalog.md),
[composition contract](../../../tools/rust/bof3-textures/docs/composition.md) and
[selection contract](../../../tools/rust/bof3-textures/docs/selection.md) for
the required source, draw and selector context. Unresolved formats and consumers
remain full-support blockers.

## Embedded save icons

`BIN/ETC/SHOP.EMI#8` and `BIN/ETC/START.EMI#9` contain equal producer payloads
loaded at `0x801EEC00`. They retain separate source identities. The producer at
`0x801EF27C` writes an `SC` header at `0x800C1800`. The original SHOP entry-0
caller at `0x801D6A08` supplies one block, icon mode `0x13`, and a title pointer
at `0x80145AD4`.

| Data | Producer payload offset | Save-header offset | Bytes |
| --- | --- | --- | ---: |
| 16-word palette | `0x1268` | `0x60` | 32 |
| Frame 0 | `0x1294` | `0x80` | 128 |
| Frame 1 | `0x1354` | `0x100` | 128 |
| Frame 2 | `0x1414` | `0x180` | 128 |

Each frame contains 16 by 16 four-bit indices, with the low nibble first.
All frames use the same palette. Source gaps between frames are outside the
copied index planes and remain unchanged during packing. The producer writes
the mode and block count at header offsets `2` and `3`, copies the title to
offset `4`, and clears offsets `0x44..0x60`.

The texture tool edits these source image ranges. Memory-card operations belong
to the emulator's memory-card owner. Producer execution alone does not establish
natural save-path execution, persistence, icon display, or animation timing.

The raw-texture and palette model agrees with the pinned
[`BoF3-Data-Doc`](../../../third_party/references/bof3-data-doc/src/DataStructures/3_TextureAndPalette.md);
the chunk geometry and descriptor fields are confirmed by the local loader and
VRAM reconstruction.
