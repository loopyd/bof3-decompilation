---
type: Table
title: Characters and masters
description: Base character stats, master skills, master stats, and master names.
tags: [tables, characters, masters]
---

# Character and master data

## Base stats (164 bytes)

The table-list name `BaseStats2Object` refers to the byte-identical STATUS.EMI
copy of this record (`STATUS.EMI#0 @ 0x1b114`); it is not a second layout.

Raw archive location: `BIN/ETC/START.EMI` @ `0x72914` (primary).
`BIN/ETC/STATUS.EMI` @ `0x1b114` holds a byte-identical copy.

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
| `0x84` | 1 | unknown |
| `0x85` | 6 | level-up stat modifiers (runtime use) |
| `0x8b` | `0x19` | unknown |

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

### Runtime level-up contract

`GAME.EMI#0 @ 0x801addd4` operates on the mutable character array at runtime
`0x80144968`, with the same `0xa4` stride as `BaseStatsObject`. For the input
character index it reads level at record offset `0x06` and experience at
`0x08`, then indexes `GAME#0` level-growth rows as `character × 99 + level`.

For each newly reached level it updates and clamps to `999` the six base-stat
halfwords at offsets `0x3c`, `0x3e`, `0x40`, `0x42`, `0x44`, and `0x46`
(base HP, AP, power, defense, agility, and intellect). The six signed modifier
bytes at offsets `0x85`–`0x8a` are added to the corresponding level-growth
values; these bytes are part of the previously unresolved trailing `0x20`
bytes, not padding. The function writes the new level at `0x06`, invokes the
shared character recalculation routine, and passes both level-record bytes at
`0x06` and `0x07` to the ability-registration call.

This contract is disassembly-backed; an exact C lift remains pending, so the
remaining trailing bytes and recalculation semantics stay target-local. A
readable target-local candidate exists at
`src/bof3/ui/func_801ADDD4.c`, but it is not promoted as an exact replacement.

## Master IDs

Masters are identified by array index (0-16). No explicit ID field exists
in the binary; the index IS the ID.

| ID | Name | HP | AP | PWR | DFN | AGI | INT | Skills (`ability_id:level`) |
| --: | --- | --: | --: | --: | --: | --: | --: | --- |
| 0 | Bunyan | +2 | -2 | +2 | +1 | +0 | -3 | `0xaa:2 0x27:5 0x03:8 0x14:10` |
| 1 | Mygas | +0 | +1 | -1 | -1 | +0 | +2 | `0xc4:1 0xa3:4 0xac:6 0xcb:8` |
| 2 | Yggdrasil | -1 | +1 | -2 | +1 | +0 | +2 | `0x3a:2 0x24:5 0xb9:8` |
| 3 | D'lonzo | -1 | -2 | +1 | +0 | +1 | +0 | `0xa6:2 0xad:3 0xd8:4` |
| 4 | Fahl | +4 | +0 | +1 | +3 | -3 | -3 | `0x0f:2 0x1a:4 0x99:6` |
| 5 | Durandal | +0 | +0 | +0 | +0 | +0 | +0 | `0x0a:1 0xa4:2 0xa5:3` |
| 6 | Giotto | +4 | +3 | -1 | -1 | -1 | -1 | `0x8c:2 0x29:5 0x3f:8` |
| 7 | Hondara | +0 | +1 | -2 | +0 | +0 | +1 | `0xb3:2 0xcf:5 0xa9:8` |
| 8 | Emitai | +0 | +4 | -2 | -2 | +0 | +4 | `0xbc:2 0x06:4 0x28:6` |
| 9 | Deis | -3 | +3 | +1 | -3 | +1 | +3 | `0xc3:2 0xc6:5 0xc9:8 0xce:11 0x3e:15` |
| 10 | Hachio | +2 | -2 | +2 | +1 | -1 | -1 | `0x9a:2 0x9d:4` |
| 11 | Bais | +0 | +0 | +1 | +0 | +0 | +0 | `0xaa:5 0x27:8 0x03:12` |
| 12 | Lang | +0 | +0 | +0 | +1 | +0 | +0 | `0xaa:5 0x27:8 0x03:12` |
| 13 | Lee | +0 | +0 | +0 | +0 | +0 | +1 | `0xaa:5 0x27:8 0x03:12` |
| 14 | Wynn | +1 | +0 | +0 | +0 | +0 | +0 | `0xaa:5 0x27:8 0x03:12` |
| 15 | Ladon | -6 | -6 | +2 | +2 | +1 | +2 | `0x2a:3 0x9c:5 0x2b:7 0x13:9` |
| 16 | Meryleep | -1 | +0 | -1 | -1 | +2 | +0 | `0x9e:2 0xab:5 0x26:8` |

Indices 11–14 (Bais, Lang, Lee, Wynn) are restricted and cannot have
skills randomized.

## Master skills (12 bytes)

Source: `BIN/ETC/SISYOU.EMI` @ `0x3C88`.

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | `0x0c` | skill levels — 6 × 2-byte little-endian pairs (`level | ability << 8`) |

17 records, one per master. Record index is the master id. `0xFF` in high byte
= empty slot.

### Encoding

12 bytes per master, 6 skill slots, 2 bytes each. The low byte is the required
level and the high byte is the ability index. A high byte of `0xff` is empty;
the editor uses low-byte `0x63` for an unused slot.

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

- `AFLDKWA.EMI` @ 0x1BE0: 17 null-terminated custom-encoded strings (primary)
- `FIRST.EMI` @ 0x3CBE0: same list (copy)
- `SISYOU.EMI` @ 0x3C88: master skills (no names)
- `SISYOU.EMI` @ 0x3D54: master stats (no names)

All characters have `master=0xFF` in base stats. Master assignment is
determined dynamically at runtime, not stored in the base stats record.

## Dragon data in STATUS.EMI

| Offset | Size | Content |
| ---: | ---: | --- |
| 0x1C018 | 40 bytes | 10 x uint32 pointer candidates (runtime addresses) |
| 0x1C040 | unresolved | mixed values and additional pointer candidates |

The ten values at `0x1C018` are little-endian 32-bit pointer candidates, not
table indices. The following region is unresolved; the old `0x1C02C`/100×6
growth interpretation overlapped these pointers and is not promoted.

## Randomizer seed slots

The randomizer writes two encoded ASCII seed values into both `AFLDKWA.EMI`
and `FIRST.EMI`. These are archive-local offsets, not STATUS.EMI offsets.

| Version | Seed 1 offset / size | Seed 2 offset / size |
| --- | ---: | ---: |
| v1.1 | `0x1571` / `0x14` | `0x1598` / `0x0c` |
| v1.0 | `0x156b` / `0x14` | `0x1592` / `0x0c` |

The first value is the `Seed: ` display slot; spaces are encoded as `0xff`
and the colon as `0x8f`. The second value is the decimal seed slot. The
corresponding `AFLDKWA`/`FIRST` mirror offsets are `0x3c571`/`0x3c598` in
v1.1 and `0x3c56b`/`0x3c592` in v1.0.

## Evidence

- Source files: `out/extracted/BIN/ETC/START.EMI`, `STATUS.EMI`, `SISYOU.EMI`, `AFLDKWA.EMI`, `FIRST.EMI`
- Struct definitions: `third_party/references/vast-violence/tables/struct_*.txt`
