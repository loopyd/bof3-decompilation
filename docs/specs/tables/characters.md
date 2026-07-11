---
type: Table
title: Characters and masters
description: Base character stats, master skills, master stats, and master names.
tags: [tables, characters, masters]
---

# Character and master data

## Base stats (164 bytes)

Source: `BIN/ETC/START.EMI` @ `0x72914` (primary). `BIN/ETC/STATUS.EMI` @
`0x1b114` holds a byte-identical copy.

Full character save-state record. 8 records (one per playable character +
whelp).

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 5 | character name |
| `0x05` | 1 | character index |
| `0x06` | 1 | level |
| `0x07` | 1 | unknown |
| `0x08` | 4 | exp |
| `0x0c` | 2 | status |
| `0x0e` | 1 | weapon index |
| `0x0f` | 1 | shield index |
| `0x10` | 1 | helmet index |
| `0x11` | 1 | armor index |
| `0x12` | 2 | accessory indexes |
| `0x14` | 2 | current hp |
| `0x16` | 2 | current ap |
| `0x18` | 1 | current willpower |
| `0x19` | 1 | innoculation |
| `0x1a` | 1 | fatigue |
| `0x1b` | 1 | master id |
| `0x1c` | 2 | max hp |
| `0x1e` | 2 | max ap |
| `0x20` | 2 | power |
| `0x22` | 2 | defense |
| `0x24` | 2 | agility |
| `0x26` | 2 | intellect |
| `0x28` | 2 | unknown |
| `0x2a` | 1 | willpower |
| `0x2b` | 9 | resistances |
| `0x34` | 1 | surprise chance |
| `0x35` | 1 | reprisal chance |
| `0x36` | 1 | critical chance |
| `0x37` | 1 | evasion |
| `0x38` | 1 | accuracy |
| `0x39` | 3 | unknown |
| `0x3c` | 2 | base hp |
| `0x3e` | 2 | base ap |
| `0x40` | 2 | base power |
| `0x42` | 2 | base defense |
| `0x44` | 2 | base agility |
| `0x46` | 2 | base intellect |
| `0x48` | 2 | unknown |
| `0x4a` | 1 | base willpower |
| `0x4b` | 9 | base resistances |
| `0x54` | 1 | base surprise chance |
| `0x55` | 1 | base reprisal chance |
| `0x56` | 1 | base critical chance |
| `0x57` | 1 | base evasion |
| `0x58` | 1 | base accuracy |
| `0x59` | 3 | unknown |
| `0x5c` | 10 | healing abilities |
| `0x66` | 10 | assist abilities |
| `0x70` | 10 | attack abilities |
| `0x7a` | 10 | skill abilities |
| `0x84` | `0x20` | unknown |

### Verified character data

| Index | Name | Level | HP | AP | Pwr | Dfn | Agi | Int |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 0 | Ryu | 1 | 20 | 11 | 12 | 10 | 8 | 10 |
| 1 | Nina | 5 | 27 | 33 | 13 | 13 | 14 | 24 |
| 2 | Garr | 13 | 99 | 7 | 58 | 44 | 17 | 21 |
| 3 | Teepo | 1 | 22 | 13 | 13 | 10 | 10 | 12 |
| 4 | Rei | 5 | 42 | 12 | 23 | 15 | 19 | 22 |
| 5 | Momo | 10 | 52 | 40 | 30 | 25 | 15 | 50 |
| 6 | Peco | 1 | 40 | 8 | 18 | 14 | 3 | 4 |
| 7 | Whelp | 0 | 15 | 15 | 10 | 8 | 4 | 8 |

Note: Whelp has character_index=10, not 7.

## Master IDs

Masters are identified by array index (0-16). No explicit ID field exists
in the binary; the index IS the ID.

| ID | Name | HP | AP | PWR | DFN | AGI | INT | Skills |
| --: | --- | --: | --: | --: | --: | --: | --: | --- |
| 0 | Bunyan | +2 | -2 | +2 | +1 | +0 | -3 | L17:02 L39:05 L03:08 L20:0A |
| 1 | Mygas | +0 | +1 | -1 | -1 | +0 | +2 | L19:01 L16:04 L17:06 L20:08 |
| 2 | Yggdrasil | -1 | +1 | -2 | +1 | +0 | +2 | L05:02 L03:05 L18:08 |
| 3 | D'lonzo | -1 | -2 | +1 | +0 | +1 | +0 | L16:02 L17:03 L21:04 |
| 4 | Fahl | +4 | +0 | +1 | +3 | -3 | -3 | L01:02 L02:04 L15:06 |
| 5 | Durandal | +0 | +0 | +0 | +0 | +0 | +0 | L01:01 L16:02 L16:03 |
| 6 | Giotto | +4 | +3 | -1 | -1 | -1 | -1 | L14:02 L04:05 L06:08 |
| 7 | Hondara | +0 | +1 | -2 | +0 | +0 | +1 | L17:02 L20:05 L16:08 |
| 8 | Emitai | +0 | +4 | -2 | -2 | +0 | +4 | L18:02 L00:04 L04:06 |
| 9 | Deis | -3 | +3 | +1 | -3 | +1 | +3 | L19:02 L19:05 L20:08 L20:0B L06:0F |
| 10 | Hachio | +2 | -2 | +2 | +1 | -1 | -1 | L15:02 L15:04 |
| 11 | Bais | +0 | +0 | +1 | +0 | +0 | +0 | L17:05 L03:08 L00:0C |
| 12 | Lang | +0 | +0 | +0 | +1 | +0 | +0 | L17:05 L03:08 L00:0C |
| 13 | Lee | +0 | +0 | +0 | +0 | +0 | +1 | L17:05 L03:08 L00:0C |
| 14 | Wynn | +1 | +0 | +0 | +0 | +0 | +0 | L17:05 L03:08 L00:0C |
| 15 | Ladon | -6 | -6 | +2 | +2 | +1 | +2 | L04:03 L15:05 L04:07 L01:09 |
| 16 | Meryleep | -1 | +0 | -1 | -1 | +2 | +0 | L15:02 L17:05 L03:08 |

Indices 11–14 (Bais, Lang, Lee, Wynn) are restricted and cannot have
skills randomized.

## Master skills (12 bytes)

Source: `BIN/ETC/SISYOU.EMI` @ `0x3C88`.

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | `0x0c` | skill levels — 6 × 2-byte (level<<8|skill_id) pairs |

17 records, one per master. Record index is the master id. `0xFF` in high byte
= empty slot.

### Encoding

12 bytes per master, 6 skill slots, 2 bytes each:
- High byte = level required (0x63 = unused slot)
- Low byte = ability ID (index into abilities table)

## Master stats (6 bytes)

Source: `BIN/ETC/SISYOU.EMI` @ `0x3D54`.

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 1 | hp bonus (signed) |
| `0x01` | 1 | ap bonus (signed) |
| `0x02` | 1 | power bonus (signed) |
| `0x03` | 1 | defense bonus (signed) |
| `0x04` | 1 | agility bonus (signed) |
| `0x05` | 1 | intellect bonus (signed) |

17 records, one per master. Values are signed bytes (`0x80`–`0xFF` = negative,
`value - 256`). Range observed: -6 to +4.

## Master name locations

- `AFLDKWA.EMI` @ 0x1BE0: 17 null-terminated ASCII strings (primary)
- `FIRST.EMI` @ 0x3CBE0: same list (copy)
- `SISYOU.EMI` @ 0x3C88: master skills (no names)
- `SISYOU.EMI` @ 0x3D54: master stats (no names)

All characters have `master=0xFF` in base stats. Master assignment is
determined dynamically at runtime, not stored in the base stats record.

## Dragon data in STATUS.EMI

| Offset | Size | Content |
| ---: | ---: | --- |
| 0x1C018 | 20 bytes | 10 x uint16 — dragon pointers (runtime addresses) |
| 0x1C02C | 600 bytes | 100 x 6 bytes — dragon growth table |

Dragon pointers (0x1C018) are game-memory addresses, not table indices.
Dragon growth (0x1C02C) format is not yet fully decoded.

## Randomizer seeds

| Offset | Size | Version |
| ---: | ---: | --- |
| 0x23998 | 6 bytes | v1.1 seed |
| 0x285D4 | 13 bytes | v1.0 seed |

Seed format: raw bytes used by the randomizer for reproducible
randomization. v1.0 and v1.1 use different seed sizes.

## Evidence

- Source files: `out/extracted/BIN/ETC/START.EMI`, `STATUS.EMI`, `SISYOU.EMI`, `AFLDKWA.EMI`, `FIRST.EMI`
- Validation: `out/reports/vast-violence-1.1.json`
