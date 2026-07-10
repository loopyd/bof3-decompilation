---
type: Payload format reference
title: EMI graphics payload semantics
description: Evidence and confidence boundaries for EMI graphics-side payload types.
tags: [format, emi, graphics]
---

# EMI Graphics Payload Semantics

This document tracks graphics-side payload semantics carried inside EMI archives.

Use `emi.md` for container structure and generic type mapping. Use runtime docs for draw-order and helper behavior.

## Status

- Confidence: medium
- Scope:
  - Type `3` payload behavior and how it maps to PSX VRAM upload conventions
  - Palette/CLUT-side payload conventions commonly seen as type `0` companions

## Current Proven Model

At current confidence, type `3` entries should be treated as raw VRAM upload payloads, not complete standalone TIM files.

Corroborating references:

- `docs/specs/formats/emi.md`
- `docs/specs/runtime/emi-graphics-pipeline.md`
- `docs/specs/sources/psx-tim.md`

## Type-3 Payloads

Current behavior model:

- payload bytes are staged into VRAM-oriented destinations using loader/runtime-side semantics
- metadata needed for final composition (page selection, CLUT choice, sprite rects, draw order) often lives in code-bearing overlays or shared helper tables
- a type-`3` payload alone is usually insufficient to claim final in-game composition

Implication:

- keep extraction outputs split into:
  - raw upload surfaces
  - code-ordered composition metadata
  - final validated composite only when both agree

## Palette/CLUT Companion Payloads

Current local pattern:

- palette-like blobs frequently appear as small type-`0` entries (for example `0x200` or `0x400` bytes)
- these are often rows or subregions within a larger runtime CLUT bank, not necessarily one standalone CLUT object

See:

- `docs/specs/content/status-emi.md`
- `docs/specs/runtime/emi-graphics-pipeline.md`

## TIM Baseline vs BOF3 Storage

TIM remains a hardware baseline (CLUT indexing, VRAM geometry), but BOF3 frequently stores equivalent image/CLUT content without full TIM wrappers.

Do not claim a payload is a complete TIM unless a valid TIM header is present in the bytes.

## Open Points

- precise per-family mapping from load args to final `tpage`/CLUT usage
- robust distinction between sprite atlases, tile pages, and effect pages across all `ETC`, `WORLD*`, and battle families
- standard criteria for promoting a graphics path from tentative to proven
