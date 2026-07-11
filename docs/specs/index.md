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
- [Generated artifacts](formats/artifacts.md) — output ownership.

## Runtime

- [Runtime model](runtime/runtime-layout.md) — executable and overlay boundaries.
- [EMI loader](runtime/emi-loader.md) — loader dispatch and payload handling.
- [Module map](runtime/module-map.md) — confirmed executable targets.
- [STR playback](runtime/str-playback.md) — BOF3 sector layout.
- [Recovered layouts](runtime/recovered-layouts.md) — evidenced structure offsets.
- [Data tables](tables/index.md) — verified archive offsets and record layouts.

## Assets

- [Archive families](assets/index.md) — stable family roles.
- [File source map](assets/file-map.md) — which EMI file owns each data domain.

## Authoring rule

Keep only facts supported by original bytes, tracked layouts, or confirmed
runtime behavior. Store hypotheses, tool output, corpus tables, and progress in
`out/work/`, `out/catalog/`, or `out/reports/`.
