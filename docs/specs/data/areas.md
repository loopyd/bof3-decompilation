---
type: Table
title: Per-area data
description: Monsters, formations, chests, genes, chrysms, fairies, and manillo items — all from WORLD*/AREA*.EMI.
tags: [tables, area-data, monsters, formations]
---

# Per-area data structures

Each `WORLD*/AREA*.EMI` file is the sole source for its area's data.
Monsters in a formation must come from the same archive as the formation.
There is no cross-area monster lookup.

## Pointer tables

| Source map | Records | Archives | Record size |
| --- | ---: | ---: | ---: |
| `pointers_monsters_1.1.txt` | 1,400 (`1.0`: 1,386) | 200 (`1.0`: 198) | `0x88` |
| `pointers_formations_1.1.txt` | 1,600 (`1.0`: 1,584) | 200 (`1.0`: 198) | `0x09` |
| `pointers_chests_1.1.txt` | 224 (`1.0`: 216) | 77 (`1.0`: 76) | `0x03` |
| `pointers_genes.txt` | 17 | 15 | `0x01` |
| `pointers_chrysm.txt` | 13 | 13 | `0x01` |
| `pointers_fairies.txt` | 720 | 12 | `0x09` |
| `pointers_manillo_items_1.1.txt` | 165 | 3 | `0x08` |

Pointer-map locations are raw archive offsets. v1.1 adds AREA063 and AREA154
records for monsters and formations, and the Central Wyndia battlefield chest
records; layouts and record sizes remain unchanged. `0x00000` and `0xFFFFF`
indicate empty slots.

## Monsters (136 bytes)

The largest record type. Each monster stores name, AI, stats, drop data,
four skill-condition blocks, and resistances.

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 8 | monster name |
| `0x08` | 2 | monster_id |
| `0x0a` | 1 | choice AI |
| `0x0b` | 3 | unknown |
| `0x0e` | 1 | target preference |
| `0x0f` | 1 | unknown |
| `0x10` | 2 | zenny (gold drop) |
| `0x12` | 2 | exp |
| `0x14` | 1 | level |
| `0x15` | 3 | unknown |
| `0x18` | 8 | initial skills |
| `0x20` | 2 | hp |
| `0x22` | 2 | ap |
| `0x24` | 2 | power |
| `0x26` | 2 | defense |
| `0x28` | 2 | agility |
| `0x2a` | 2 | intellect |
| `0x2c` | 1 | steal item index |
| `0x2d` | 1 | steal item type |
| `0x2e` | 2 | steal rate |
| `0x30` | 1 | drop item index |
| `0x31` | 1 | drop item type |
| `0x32` | 2 | drop rate |
| `0x34` | 1 | condition 1 |
| `0x35` | 7 | AI params 1 |
| `0x3c` | 8 | skills block 1 |
| `0x44` | 1 | condition 2 |
| `0x45` | 7 | AI params 2 |
| `0x4c` | 8 | skills block 2 |
| `0x54` | 1 | condition 3 |
| `0x55` | 7 | AI params 3 |
| `0x5c` | 8 | skills block 3 |
| `0x64` | 1 | condition 4 |
| `0x65` | 7 | AI params 4 |
| `0x6c` | 8 | skills block 4 |
| `0x74` | 4 | unknown |
| `0x78` | 9 | resistances |
| `0x81` | 7 | unknown |

Each skills block stores 8 ability indexes. Condition byte `0x63` (99)
= unused block. Boss detection: monsters not appearing in any formation
with nonzero appearance rate.

### Monster IDs

Bytes 8-9 contain a unique monster ID. ID range: 0x0001–0x04A8 (175
unique IDs for 168 unique names).

Five monsters have multiple IDs (different variants/forms):

| Monster | IDs | Notes |
| --- | --- | --- |
| Ammonite | 0x0095, 0x0096 | same area, likely color variants |
| Beyd | 0x0058, 0x0059 | boss (10000 HP) vs regular (100 HP) |
| Garr | 0x0454, 0x0465 | party member vs NPC variant |
| Golem | 0x004C, 0x004D | same area, likely color variants |
| Nue | 0x000D, 0x000E | different areas, slightly different stats |
| ToxicMan | 0x007F, 0x00AE, 0x00AF | three variants in same area |

The pointer list index is NOT the monster ID. Multiple pointers can
reference the same monster (same ID) in different areas.

### Monster resistance array (9 bytes)

| Byte | Element |
| ---: | --- |
| 0 | Fire |
| 1 | Frost |
| 2 | Thunder |
| 3 | Earth |
| 4 | Wind |
| 5 | Holy |
| 6 | Psionic |
| 7 | Status |
| 8 | Death |

Values 0–7 (resistance level). Condition byte `0x63` (99) = unused block.

## Formations (9 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 8 | monster indexes (8 slots, `0xFF` = empty) |
| `0x08` | 1 | appearance rate |

1,600 records across 200 area archives. Appearance rate 0 = boss
formation / inactive.

To identify monsters absent from normal encounters, collect the monster indexes
from formations whose appearance rate is nonzero. A monster referenced only by
zero-rate formations is a boss or otherwise inactive; confirm the caller before
assigning a narrower semantic name.

## Genes (1 byte)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 1 | gene index (`0x00`–`0x11`) |

17 records across 15 archives. Gene `0x21` is reserved for Flame gene
(gated behind v1.1 patch flag).

### Dragon gene locations

| Gene | Archive | Offset |
| --- | --- | ---: |
| Flame | SCENARIO/SCENA03.EMI | 0x031A4 |
| Frost | WORLD01/AREA067.EMI | 0xD1200 |
| Thunder | WORLD02/AREA100.EMI | 0xBC808 |
| Shadow | SCENARIO/SCENA08.EMI | 0x0543C |
| Radiance | WORLD03/AREA146.EMI | 0xA116C |
| Force | WORLD03/AREA117.EMI | 0xAAB8C |
| Defender | SCENARIO/SCENA03.EMI | 0x05D94 |
| Eldritch | WORLD02/AREA077.EMI | 0xCADB8 |
| Miracle | WORLD02/AREA103.EMI | 0xCCD10 |
| Gross | WORLD02/AREA105.EMI | 0xC055C |
| Thorn | WORLD01/AREA054.EMI | 0xC6020 |
| Reverse | WORLD02/AREA093.EMI | 0xBA820 |
| Mutant | WORLD01/AREA075.EMI | 0xC2C10 |
| ??? | WORLD03/AREA114.EMI | 0xBE084 |
| Trance | WORLD03/AREA123.EMI | 0xB9820 |
| Failure | WORLD03/AREA145.EMI | 0xCB7B8 |
| Fusion | SCENARIO/SCENA08.EMI | 0x05448 |
| Infinity | (no pointer) | — |

## Chrysms (1 byte)

Same struct as genes. 13 records across 13 archives. Each chrysm archive
has exactly one gene record. No Flame/Defender/Shadow/Fusion/Infinity
chrysms exist — these are gene-only.

| Chrysm | Archive | Offset |
| --- | --- | ---: |
| Frost | WORLD01/AREA067.EMI | 0xD1790 |
| Thunder | WORLD02/AREA100.EMI | 0xBCE07 |
| Radiance | WORLD03/AREA146.EMI | 0xA15BF |
| Force | WORLD03/AREA117.EMI | 0xAB3B2 |
| Eldritch | WORLD02/AREA077.EMI | 0xCB8A0 |
| Miracle | WORLD02/AREA103.EMI | 0xCCECC |
| Gross | WORLD02/AREA105.EMI | 0xC079D |
| Thorn | WORLD01/AREA054.EMI | 0xC60C4 |
| Reverse | WORLD02/AREA093.EMI | 0xBA852 |
| Mutant | WORLD01/AREA075.EMI | 0xC3DB0 |
| ??? | WORLD03/AREA114.EMI | 0xBE44E |
| Trance | WORLD03/AREA123.EMI | 0xB9860 |
| Failure | WORLD03/AREA145.EMI | 0xCC9EA |

## Fairies (9 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 5 | fairy name |
| `0x05` | 4 | stats |

720 records across 12 area archives (fairy village).

## Chests (3 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 1 | memory address byte (`0xFF` = empty) |
| `0x01` | 1 | item index |
| `0x02` | 1 | item type |

224 records across 77 world archives. When `item_type = 0xFF`, the chest
contains zenny: `item_index × 40`.

## Manillo items (8 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 1 | item index |
| `0x01` | 1 | item type |
| `0x02` | 3 | fish indexes (`0xFF` = empty) |
| `0x05` | 3 | fish quantities |

165 records across 3 manillo shop archives. Fish items start at index
`0x38` in the item table.

### Manillo stock (10 bytes)

Source: `BIN/WORLD00/AREA030.EMI` @ `0x3e53e` (v1.1) / `0x3e53a` (v1.0).

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | `0x0a` | trade indexes — 10 × 1-byte item references |

16 records. Points into the manillo item table via these indexes.
Stock locations (from `ManilloStockObject`): Farm (0), Tower (2),
Urkan Tapa (7), Dauna Mine (9), Cliff (0xB), Steel Beach (0xD),
Kombinat (0xF).

## Evidence

- Pointer tables: `third_party/references/vast-violence/tables/pointers_*.txt`
- Struct definitions: `third_party/references/vast-violence/tables/struct_*.txt`
