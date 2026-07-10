---
type: Runtime pipeline
title: EMI graphics pipeline
description: Proven BOF3 extraction and runtime model for graphics-side EMI content.
tags: [runtime, emi, graphics]
---

# EMI Graphics Pipeline

This document records the current repo-local extraction model for BOF3
graphics-side `EMI` content.

It is the durable description of the graphics-side extraction model currently
reflected by:

- `scripts/rebof3/assets/emi_archive.py`
- `out/inventory/`
- the related runtime specs in `docs/specs/runtime/` and `docs/specs/formats/`

## Scope

This pipeline only covers the graphics-side path that is currently proven.

What it does cover:

- direct parsing of original `.EMI` archives under `out/extracted/`
- loader-faithful reconstruction of type-`3` image uploads
- indexed-image decode for `4bpp` and `8bpp` pages
- CLUT decode from palette-side type-`0` blobs
- VRAM sampling of clean sprite or page rects once `tpage`, `clut`, and
  source rects are known

What it does not cover by itself:

- proving final palette-row choice for every archive
- proving final sprite composition for every family
- proving animation or transition timing for every screen

Those last steps remain overlay-driven and only become `validated` once the
archive bytes and the corresponding code-side mapping agree.

## 1. Parse The Archive

Source of truth:

- original `.EMI` archives under `out/extracted/BIN/**/*.EMI`

The parser reads the TOC and exposes, per entry:

- `index`
- `type`
- `size`
- `load_arg`
- `first_word`
- payload bytes

Current broad graphics-side classification:

- `type 3`
  - raw image / VRAM upload path
- `type 0` in `0x80033000 .. 0x8003afff` with known small sizes
  - palette / CLUT candidate
- `type 0` or `type 1` at code-like RAM windows
  - code or table candidate

## 2. Rebuild Type-3 Uploads

The primary reconstruction rule is the recovered `SLUS` type-`3` loader, not
the older generic “unstrip” hypothesis.

Packed fields from `load_arg`:

```c
base_x_words = ((load_arg >> 24) & 0x3f) << 5;
base_y       = ((load_arg >> 16) & 0x1f) << 5;
span_chunks  = (load_arg >> 8) & 0x3f;
```

Proven constants:

- chunk size: `0x800` bytes
- chunk destination rect: `32x32` VRAM words/pixels in the encoded page space

Upload iteration:

```c
for (chunk_index = 0; chunk_index < chunk_count; ++chunk_index) {
    chunk_x = (base_x_words + (chunk_index % span_chunks) * 32) & 0x03ff;
    chunk_y = (base_y + (chunk_index / span_chunks) * 32) & 0x01ff;
    upload_rect(chunk_x, chunk_y, 32, 32, payload_chunk);
}
```

Current implications:

- the texture page is reconstructed in VRAM order
- the old “striped” look is a symptom of reading the raw bytes linearly instead
  of replaying the loader chunk placement
- any offline extraction that does not match this chunk upload rule is not
  runtime-faithful

## 3. Decode Indexed Pixels

Once the encoded page is reconstructed, the repo decodes the indexed pixels:

- `8bpp`
  - one byte per pixel
- `4bpp`
  - low nibble first, then high nibble

`4bpp` unpack rule:

```c
out[i * 2 + 0] = packed & 0x0f;
out[i * 2 + 1] = packed >> 4;
```

Width formulas after decode:

- `4bpp`: `pixel_width = encoded_width * 2`
- `8bpp`: `pixel_width = encoded_width`

This produces the `raw_indices` layer used for geometry validation:

- if `raw_indices` look coherent, the loader/chunk reconstruction is likely
  correct
- if `raw_indices` look coherent but colors do not, the remaining problem is
  CLUT selection or composition

## 4. Decode CLUT Rows

Palette-side entries are decoded as PS1 `16-bit` colors using the standard
layout:

```text
SBBBBBGGGGGRRRRR
```

Current RGBA decode:

```c
red   = (pixel << 3) & 0xf8;
green = (pixel >> 2) & 0xf8;
blue  = (pixel >> 7) & 0xf8;
alpha = (rgb == 0 && stp == 0) ? 0 : 255;
```

Current row interpretation:

- `4bpp`
  - `16` colors per row
  - `0x20` bytes per row
- `8bpp`
  - `256` colors per row
  - `0x200` bytes per row

This is enough to produce:

- palette-preview sheets
- direct-color outputs for archives where one page + one row is already
  unambiguous

## 5. Sample Clean Quads From VRAM

The clean output path is not “save the reconstructed page and stop.”

Once `tpage`, `clut`, and source rects are proven, the repo samples the
corresponding texture quad from reconstructed VRAM.

Current `tpage` reconstruction from a type-`3` base:

```c
tpage =
    ((base_x_words >> 6) & 0x0f) |
    (((base_y >> 8) & 0x01) << 4) |
    (texture_mode << 7);
```

Sampling rules:

- `4bpp`
  - sample packed nibbles
  - resolve through a `16`-color CLUT row
- `8bpp`
  - sample bytes
  - resolve through a `256`-color CLUT row
- `16bpp`
  - sample raw `15-bit` colors directly

This is the first point where a “clean texture quad” becomes a trustworthy
asset output.

## 6. Candidate Versus Validated Outputs

The metadata and extractors now keep the render states explicit:

- `raw_indices_only`
  - page geometry is known
  - no validated CLUT/layout mapping yet
- `direct_color_candidate`
  - archive bytes alone suggest one image + one palette path
  - still not locally validated
- `validated_direct`
  - archive bytes alone are sufficient and the result has been validated
- `validated_family`
  - overlay-side tables or code prove the final page/rect/CLUT mapping

This prevents the extractor from claiming correctness where it still only has a
guess.

## 7. Proven Family Patterns

### STATUS / START Menu Bank

Currently proven:

- `0x801d0c00` overlay family
- portrait crop table at `DAT_801ec96c`
- fixed portrait size `40x48`
- CLUT bank upload to VRAM `x=0, y=0x01e0, w=256, h=32`
- local `0x200` palette rows mapping into that bank by
  `bank_row = (load_arg - 0x80033800) / 0x200`
- complete menu extraction is not fully archive-local
  - local `STATUS` callers choose position and `tpage`
  - shared `GAME.EMI` helper `FUN_801af2a0` resolves several menu sprite ids
    through shared rect tables

That is why the `STATUS` portrait path is currently a `validated_family`
renderer, while the broader menu composition remains partially recovered.

### FIRST / DEMO / GAME Title Bundle

Currently proven:

- `FIRST` and `DEMO` provide the type-`3` pages and CLUT payloads
- `GAME` provides the layout records and draw order
- the title outputs must therefore be sampled from merged VRAM state, not
  treated as one image blob per file

That is why title outputs split into:

- validated original-ish pieces
- code-backed candidate pieces
- derived composites

## 8. Metadata Contract

Machine source of truth:

- `out/inventory/`

That metadata is expected to hold:

- global decode/upload formulas
- per-archive graphics summaries
- per-entry dimension and CLUT candidate facts
- family-level render rules
- bundle-level rules for multi-archive paths like `FIRST + DEMO + GAME`

When a new extraction rule becomes defensible, the rule should be promoted into:

1. this document
2. the smallest repo-owned implementation that consumes the metadata

Do not leave a validated rule only in one ad hoc script.
