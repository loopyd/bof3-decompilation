---
type: Spec index
title: Reverse-engineering specs
description: Verified BOF3 binary facts. Generated evidence and active investigations belong under out/.
tags: [index]
---

# Reverse-engineering specs

Verified BOF3 binary facts only. Generated evidence and active investigations
belong under `out/`.

## Formats

- [EMI](formats/emi.md) — container and entry layout.
- [Graphics](formats/graphics.md) — type-3 uploads and palettes.
- [STR/XA](formats/str-xa.md) — media-sector representation.

## Programs

- [Targets](programs/targets.md) — standalone executables and confirmed overlays.

## Runtime

- [Runtime model](runtime/runtime-layout.md) — executable and overlay boundaries.
- [EMI loader](runtime/emi-loader.md) — loader dispatch and payload handling.
- [Frontend flow](runtime/frontend.md) — reviewed title, menu, and attract-path transitions.
- [Memory layouts](runtime/memory-layouts.md) — evidenced structure offsets.

## Data

- [Game data](data/index.md) — verified IDs, values, offsets, and record layouts.

## Archives

- [Families](archives/families.md) — stable archive roles.
- [Ownership](archives/ownership.md) — canonical and duplicate data locations.

## Methods

- [Discovery](methods/discovery.md) — locate and infer structures.
- [Verification](methods/verification.md) — accept or reject findings.
- [Source retention audit](migration.md) — audited C retention/removal record.

## Authoring rule

Keep only facts supported by original bytes, tracked layouts, or confirmed
runtime behavior. Store hypotheses, tool output, corpus tables, and progress in
`out/index/`, `out/catalog/`, or the relevant retained workflow directory under
`out/`.
