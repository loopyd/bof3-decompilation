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

The analysis catalog in `config/analysis/shared/bof3_objects.h` records
packed storage layouts and canonical names. Compiled declarations remain
target-owned until their ABI is confirmed by consumers; one target-local
battle overlay records the proven alternate ability-table interpretation.

## Verification

- 16 fixed tables located and byte-matched
- 7 pointer sets resolved across 200+ archives
- 23 populated record layouts validated (the catalog's two zero-byte
  placeholders are tracked separately)
- 0 mismatches

Evidence: `out/index/vast-violence-1.1.json`

## Location catalog

Offsets below are raw US v1.1 archive offsets. Versioned pointer maps are the
location source for area-local records.

For fixed tables, the archive-relative location is converted to the extracted
payload coordinate by subtracting the first-payload offset (`0x800`). All
`GAME.EMI` tables below are in `ETC/GAME#0`, whose loader destination is
`0x80195800`; the corresponding runtime address is therefore
`0x80195800 + (archive_offset - 0x800)`. The runtime addresses are layout
coordinates, not shared C declarations.

| Domain | EMI entry | Payload-relative | Runtime address |
| --- | --- | ---: | ---: |
| items | `ETC/GAME#0` | `0x33164` | `0x801c8964` |
| key items | `ETC/GAME#0` | `0x337dc` | `0x801c8fdc` |
| weapons | `ETC/GAME#0` | `0x338dc` | `0x801c90dc` |
| armor | `ETC/GAME#0` | `0x340a4` | `0x801c98a4` |
| accessories | `ETC/GAME#0` | `0x3467c` | `0x801c9e7c` |
| shops | `ETC/GAME#0` | `0x34a8c` | `0x801ca28c` |
| abilities | `ETC/GAME#0` | `0x34f0c` | `0x801ca70c` |
| level growth | `ETC/GAME#0` | `0x360dc` | `0x801cb8dc` |

Additional fixed reward tables use archive coordinates because their runtime
entry mapping is not shared with the GAME tables:

| Domain | EMI archive | Archive offset | Records × size | Layout |
| --- | --- | ---: | ---: | --- |
| fairy gifts | `ETC/COMMU00.EMI` | `0x848` | 20 × `0x04` | [fairy reward data](fairies.md#fairy-gifts) |
| fairy exploration items | `ETC/COMMU00.EMI` | `0x4218` | 48 × `0x02` | [fairy reward data](fairies.md#exploration-items-and-prizes) |
| fairy prizes | `ETC/COMMU02.EMI` | `0x2d900` | 48 × `0x02` | [fairy reward data](fairies.md#exploration-items-and-prizes) |

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
| dragon pointer candidates | `BIN/ETC/STATUS.EMI` | `0x1c018` | 10 × `u32` | [characters](characters.md#dragon-data-in-statusemi) |
| following STATUS region | `BIN/ETC/STATUS.EMI` | `0x1c040` | unresolved | [characters](characters.md#dragon-data-in-statusemi) |
| monsters | `pointers_monsters_1.1.txt` | pointer map | 1,400 × `0x88` (`1.0`: 1,386) | [areas](areas.md#monsters-136-bytes) |
| formations | `pointers_formations_1.1.txt` | pointer map | 1,600 × `0x09` (`1.0`: 1,584) | [areas](areas.md#formations-9-bytes) |
| chests | `pointers_chests_1.1.txt` | pointer map | 224 × `0x03` (`1.0`: 216) | [areas](areas.md#chests-3-bytes) |
| dragon genes | `pointers_genes.txt` | pointer map | 17 × `0x01` | [areas](areas.md#genes-1-byte) |
| chrysms | `pointers_chrysm.txt` | pointer map | 13 × `0x01` | [areas](areas.md#chrysms-1-byte) |
| fairies | `pointers_fairies.txt` | pointer map | 720 × `0x09` | [areas](areas.md#fairies-9-bytes) |
| Manillo trades | `pointers_manillo_items_1.1.txt` | pointer map | 165 × `0x08` | [areas](areas.md#manillo-items-8-bytes) |

## Detailed references

- [Schema ledger](schema-ledger.md) — data-first recovery status for every record family
- [IDs and encodings](ids.md) — namespaces, masks, packed values, and sentinels
- [Equipment](equipment.md) — items, weapons, armor, accessories, abilities, shops, level growth (`GAME.EMI`)
- [Fairy reward data](fairies.md) — fairy gifts, exploration items, and prizes (`COMMU00.EMI`, `COMMU02.EMI`)
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
