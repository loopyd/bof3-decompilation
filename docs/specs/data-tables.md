---
type: Data table map
title: Data tables
description: US v1.1 archive offsets and record layouts corroborated locally.
tags: [tables, offsets, structures]
---

# Data tables

Offsets below are raw EMI archive offsets for the US v1.1 input. Local bytes
were checked against the tables published by
[`vast-violence`](../../third_party/references/vast-violence/tables/).

## Fixed tables

| Archive | Offset | Records | Size | Content |
| --- | ---: | ---: | ---: | --- |
| `BIN/ETC/COMMU00.EMI` | `0x00848` | 20 | `0x04` | fairy gifts |
| `BIN/ETC/COMMU00.EMI` | `0x04218` | 48 | `0x02` | fairy exploration items |
| `BIN/ETC/COMMU02.EMI` | `0x2d900` | 48 | `0x02` | fairy prizes |
| `BIN/ETC/GAME.EMI` | `0x33964` | 92 | `0x12` | items |
| `BIN/ETC/GAME.EMI` | `0x33fdc` | 16 | `0x10` | key items |
| `BIN/ETC/GAME.EMI` | `0x340dc` | 83 | `0x18` | weapons |
| `BIN/ETC/GAME.EMI` | `0x348a4` | 68 | `0x16` | armor |
| `BIN/ETC/GAME.EMI` | `0x34e7c` | 52 | `0x14` | accessories |
| `BIN/ETC/GAME.EMI` | `0x3528c` | 40 | `0x17` | shops |
| `BIN/ETC/GAME.EMI` | `0x3570c` | 228 | `0x14` | abilities |
| `BIN/ETC/GAME.EMI` | `0x368dc` | 693 | `0x08` | level growth |
| `BIN/ETC/SISYOU.EMI` | `0x03c88` | 17 | `0x0c` | master skills |
| `BIN/ETC/SISYOU.EMI` | `0x03d54` | 17 | `0x06` | master stats |
| `BIN/ETC/START.EMI` | `0x72914` | 8 | `0xa4` | base stats |
| `BIN/ETC/STATUS.EMI` | `0x1b114` | 8 | `0xa4` | base stats copy |
| `BIN/WORLD00/AREA030.EMI` | `0x3e53e` | 16 | `0x0a` | Manillo stock |

The `START.EMI` and `STATUS.EMI` base-stat ranges are byte-identical for all
eight records in the current US v1.1 input.

## Pointer tables

Every US v1.1 pointer entry below resolves to an existing archive and fits
within that archive. The complete locations remain in the pinned source and in
`out/reports/vast-violence-1.1.json`.

| Source map | Records | Archives | Size |
| --- | ---: | ---: | ---: |
| `pointers_chests_1.1.txt` | 224 | 77 | `0x03` |
| `pointers_chrysm.txt` | 13 | 13 | `0x01` |
| `pointers_fairies.txt` | 720 | 12 | `0x09` |
| `pointers_formations_1.1.txt` | 1,600 | 200 | `0x09` |
| `pointers_genes.txt` | 17 | 15 | `0x01` |
| `pointers_manillo_items_1.1.txt` | 165 | 3 | `0x08` |
| `pointers_monsters_1.1.txt` | 1,400 | 200 | `0x88` |

## Equipment records

All equipment records begin with a 12-byte name.

| Record | Key offsets |
| --- | --- |
| item, `0x12` | flags `0x0c`; price `0x10` |
| weapon, `0x18` | equip mask `0x0c`; element `0x0f`; weight `0x10`; power `0x12`; price `0x16` |
| armor, `0x16` | equip mask `0x0c`; type `0x0e`; weight `0x0f`; power `0x10`; price `0x14` |
| accessory, `0x14` | equip mask `0x0c`; weight `0x0f`; price `0x12` |

The eight equip-mask bits correspond to the eight playable-character slots.
Element bits are ordered fire, ice, lightning, earth, wind, holy, psionic, and
status.

## Ability record

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | `0x0c` | name |
| `0x0c` | 1 | targeting/display flags |
| `0x0d` | 1 | skill type |
| `0x0e` | 1 | cost |
| `0x0f` | 1 | power |
| `0x10` | 1 | element mask |
| `0x11` | 1 | ability flags |
| `0x12` | 2 | reserved/control bytes |

## Boundary

These locations are game-data facts, not C source ownership. Generated table
dumps and decoded rows belong under `out/`; promote names into shared headers
only when runtime code confirms their use.
