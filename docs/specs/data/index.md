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

## Location catalog

Offsets below are raw US v1.1 archive offsets. Versioned pointer maps are the
location source for area-local records.

| Domain | Archive or pointer map | Location | Records | Layout |
| --- | --- | ---: | ---: | --- |
| items | `BIN/ETC/GAME.EMI` | `0x33964` | 92 × `0x12` | [equipment](equipment.md#items-18-bytes) |
| key items | `BIN/ETC/GAME.EMI` | `0x33fdc` | 16 × `0x10` | [equipment](equipment.md#key-items-16-bytes) |
| weapons | `BIN/ETC/GAME.EMI` | `0x340dc` | 83 × `0x18` | [equipment](equipment.md#weapons-24-bytes) |
| armor | `BIN/ETC/GAME.EMI` | `0x348a4` | 68 × `0x16` | [equipment](equipment.md#armor-22-bytes) |
| accessories | `BIN/ETC/GAME.EMI` | `0x34e7c` | 52 × `0x14` | [equipment](equipment.md#accessories-20-bytes) |
| shops | `BIN/ETC/GAME.EMI` | `0x3528c` | 40 × `0x17` | [equipment](equipment.md#shops-23-bytes) |
| abilities | `BIN/ETC/GAME.EMI` | `0x3570c` | 228 × `0x14` | [equipment](equipment.md#abilities-20-bytes) |
| level growth | `BIN/ETC/GAME.EMI` | `0x368dc` | 693 × `0x08` | [equipment](equipment.md#level-growth-8-bytes) |
| base character stats | `BIN/ETC/START.EMI` | `0x72914` | 8 × `0xa4` | [characters](characters.md#base-stats-164-bytes) |
| base-stats copy | `BIN/ETC/STATUS.EMI` | `0x1b114` | 8 × `0xa4` | [characters](characters.md#base-stats-164-bytes) |
| master skills | `BIN/ETC/SISYOU.EMI` | `0x3c88` | 17 × `0x0c` | [characters](characters.md#master-skills-12-bytes) |
| master stats | `BIN/ETC/SISYOU.EMI` | `0x3d54` | 17 × `0x06` | [characters](characters.md#master-stats-6-bytes) |
| master names | `BIN/ETC/AFLDKWA.EMI` | `0x1be0` | 17 strings | [characters](characters.md#master-name-locations) |
| dragon pointers | `BIN/ETC/STATUS.EMI` | `0x1c018` | 10 × `u16` | [characters](characters.md#dragon-data-in-statusemi) |
| dragon growth | `BIN/ETC/STATUS.EMI` | `0x1c02c` | 100 × `0x06` | [characters](characters.md#dragon-data-in-statusemi) |
| monsters | `pointers_monsters_1.1.txt` | pointer map | 1,400 × `0x88` | [areas](areas.md#monsters-136-bytes) |
| formations | `pointers_formations_1.1.txt` | pointer map | 1,600 × `0x09` | [areas](areas.md#formations-9-bytes) |
| chests | `pointers_chests_1.1.txt` | pointer map | 224 × `0x03` | [areas](areas.md#chests-3-bytes) |
| dragon genes | `pointers_genes.txt` | pointer map | 17 × `0x01` | [areas](areas.md#genes-1-byte) |
| chrysms | `pointers_chrysm.txt` | pointer map | 13 × `0x01` | [areas](areas.md#chrysms-1-byte) |
| fairies | `pointers_fairies.txt` | pointer map | 720 × `0x09` | [areas](areas.md#fairies-9-bytes) |
| Manillo trades | `pointers_manillo_items_1.1.txt` | pointer map | 165 × `0x08` | [areas](areas.md#manillo-items-8-bytes) |

## Detailed references

- [Equipment](equipment.md) — items, weapons, armor, accessories, abilities, shops, level growth (`GAME.EMI`)
- [Characters](characters.md) — base stats, master skills/stats/names, dragon data, randomizer seeds
- [Area data](areas.md) — monsters, formations, chests, genes, chrysms, fairies, manillo items
- [Encoding](encoding.md) — bitmasks, name encoding, stat packing, and formulas

## Version differences

US v1.0 (`BOF3_1.0`, md5 `226771993e29ca5c8e6e2c094c40e8d2`) differs only in
the manillo stock offset (`0x3e53a` vs `0x3e53e`) and pointer-table archive
offsets. Record counts, sizes, and field layouts are identical.

## Boundary

Generated table dumps and decoded rows belong under `out/`. Promote names
into shared headers only when runtime code confirms their use.
