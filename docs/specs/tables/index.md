---
type: Spec index
title: Data tables
description: Verified archive offsets, record layouts, and pointer maps for BOF3 game data.
tags: [index]
---

# Data tables

Game data tables embedded in EMI archives. All offsets, record sizes, and
field layouts were verified against the US v1.1 disc (`BOF3_1.1`,
md5 `9dd9a7c934b8b59d0ce76b0f25d18176`) with zero failures.

No C struct definitions exist for these tables yet. These are game-data
facts, not source-code ownership.

## Verification

- 16 fixed tables located and byte-matched
- 7 pointer sets resolved across 200+ archives
- 20 record layouts validated
- 0 mismatches

Evidence: `out/reports/vast-violence-1.1.json`

## Table files

- [Equipment](equipment.md) — items, weapons, armor, accessories, abilities, shops, level growth (`GAME.EMI`)
- [Characters](characters.md) — base stats, master skills/stats/names, dragon data, randomizer seeds
- [Area data](area-data.md) — monsters, formations, chests, genes, chrysms, fairies, manillo items
- [Encoding](encoding.md) — bitmasks, name encoding, stat packing, formulas, EMI header format

## Version differences

US v1.0 (`BOF3_1.0`, md5 `226771993e29ca5c8e6e2c094c40e8d2`) differs only in
the manillo stock offset (`0x3e53a` vs `0x3e53e`) and pointer-table archive
offsets. Record counts, sizes, and field layouts are identical.

## Boundary

Generated table dumps and decoded rows belong under `out/`. Promote names
into shared headers only when runtime code confirms their use.
