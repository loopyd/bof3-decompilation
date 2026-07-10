---
type: External source summary
title: vast_violence
description: Scope and useful BOF3 offset and structure evidence from the randomizer project.
tags: [source, bof3, offsets, structures, external]
---

# Source Summary: vast_violence

Source repo:

- `third_party/references/vast_violence`
- upstream: `https://github.com/abyssonym/vast_violence`

## What It Is

`vast_violence` is a BOF3 randomizer, not a decomp project. Its value here is as a secondary knowledge base for:

- file-relative offsets inside EMI archives
- data structure layouts
- content pointer lists
- version-specific offset differences between BOF3 `1.0` and `1.1`

## Useful Artifacts

### Table inventory

Observed categories under `third_party/references/vast_violence/tables`:

- `20` structure tables: `struct_*.txt`
- `11` pointer tables: `pointers_*.txt`
- `4` patch tables: `patch_*.txt`
- `2` name tables: `names_*.txt`
- `4` misc tables: `empty.txt`, `master.txt`, `tables_list_*.txt`

### `tables_list_1.1.txt`

This is the highest-value index file. It ties structure definitions to file-relative offsets and/or pointer tables.

Examples:

- item, weapon, armor, accessory, ability, and level data in `BIN/ETC/GAME.EMI`
- master skill/stat data in `BIN/ETC/SISYOU.EMI`
- base stats in `BIN/ETC/START.EMI` and `BIN/ETC/STATUS.EMI`
- stock/item/shop/chest/gene/fairy/formation/monster data via pointer tables

### Structure tables

Representative examples:

- `struct_monster.txt`
  - monster name
  - stats
  - steal/drop data
  - conditions and skill lists
  - resistances
- `struct_formation.txt`
  - 8 monster indexes
  - 1 appearance-rate field
- other structure files cover:
  - items
  - equipment
  - genes
  - shops
  - fairies
  - masters
  - manillo data

### Pointer tables

These files are strong leads for world/area content placement inside EMI archives.

Examples:

- `pointers_monsters_1.1.txt`
  - repeated file-relative monster record offsets inside `BIN/WORLD*/AREA*.EMI`
- `pointers_formations_1.1.txt`
  - repeated file-relative formation offsets inside `BIN/WORLD*/AREA*.EMI`
- `pointers_chests_1.1.txt`
  - chest offsets plus human-readable item annotations
- `pointers_genes.txt`, `pointers_fairies.txt`, `pointers_manillo_items_1.1.txt`
  - content-specific pointer lists across multiple area files

## Concrete Takeaways

- `vast_violence` gives us reusable file-relative offsets for a large amount of world/area content without needing to rediscover every table from scratch.
- The `struct_*.txt` files are already close to what we want in a reverse-spec/type-layout format.
- The pointer tables are especially valuable for validating our EMI extraction and for locating repeated record layouts across area overlays.
- The project also gives an external checksum cross-check for the US v1.1 target disc image:
  - MD5 `9dd9a7c934b8b59d0ce76b0f25d18176`

## Limits

- It does not explain the `SLUS_004.22` loader/runtime.
- It does not provide a global overlay execution model.
- It is strongest on data structures and content records, not on code flow.
- Notes in `ambition.txt` about copied battle engine data or boss-file battle copies are useful leads, but they are not local proof.

## How To Use It In This Repo

- Cross-check world/area EMI file-relative offsets against our extracted EMI entry boundaries.
- Promote stable `struct_*.txt` knowledge into repo-native reverse specs or type-layout docs.
- Use versioned pointer files (`1.0` vs `1.1`) when validating US v1.1 data offsets.
- Treat `ambition.txt` as lead material, not proof.
