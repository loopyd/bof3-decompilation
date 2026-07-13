---
type: Reference
title: Encoding and formulas
description: Shared reference for bitmask values, name encoding, stat packing, and game formulas.
tags: [encoding, bitmasks, formulas]
---

# Bitmasks, encoding schemes, and formulas

Shared reference for data encoding used across multiple table types.

## Bitmask values

Verified from binary bytes in `GAME.EMI`.

### Equipability (1 byte)

Used in weapon, armor, and accessory records to indicate which characters
can equip the item.

| Bit | Value | Character |
| ---: | ---: | --- |
| 0 | `0x01` | Ryu |
| 1 | `0x02` | Nina |
| 2 | `0x04` | Garr |
| 3 | `0x08` | Teepo |
| 4 | `0x10` | Rei |
| 5 | `0x20` | Momo |
| 6 | `0x40` | Peco |
| 7 | `0x80` | Whelp |

### Element (1 byte)

Used in weapon and ability records.

| Bit | Value | Element |
| ---: | ---: | --- |
| 0 | `0x01` | Fire |
| 1 | `0x02` | Ice |
| 2 | `0x04` | Lightning |
| 3 | `0x08` | Earth |
| 4 | `0x10` | Wind |
| 5 | `0x20` | Holy |
| 6 | `0x40` | Psionic |
| 7 | `0x80` | Status |

### Item flags (1 byte)

| Bit | Value | Meaning |
| ---: | ---: | --- |
| 7 | `0x80` | usable_menu |
| 6 | `0x40` | show_animation |
| 5 | `0x20` | show_name |
| 4 | `0x10` | story_item |
| 3 | `0x08` | target_all |
| 2 | `0x04` | target_enemy_default |
| 1 | `0x02` | target_selectable |
| 0 | `0x01` | target_both |

These are the storage labels from the table definition. The narrower runtime
meanings of the target bits remain consumer-dependent until a call site proves
them.

### Ability flags (1 byte, offset 0x11 in AbilityObject)

| Bit | Value | Meaning |
| ---: | ---: | --- |
| 4 | `0x10` | affects_stats |
| 3 | `0x08` | unknown_3 |
| 2 | `0x04` | target_ally_default |
| 1 | `0x02` | examinable |
| 0 | `0x01` | unknown_0 |

Verified: 19 unique values across 228 abilities.

### Armor equip type (1 byte, offset 0x0e)

| Value | Type | Count |
| ---: | --- | ---: |
| 0 | Nothing | 1 |
| 2 | Shield | 17 |
| 3 | Helmet | 17 |
| 4 | Body | 33 |

## Encoding schemes

### Custom encoding (GAME.EMI tables)

Used for item, weapon, armor, accessory, and ability names. Each name field
is 12 bytes, null-terminated with padding zeros.

| Byte | Meaning |
| ---: | --- |
| `0x00` | end of string |
| `0xFF` | space |
| `0x8E` | apostrophe (`'`) |
| `0x3D` | hyphen (`-`) |
| `0x3E` | period (`.`) |
| `0x8B` | plus (`+`) |
| `0x05 0x02` | RED color tag |
| `0x05 0x03` | BLUE color tag |
| `0x06` | NOCOLOR (reset) |
| `0x01` | newline (weapon name display) |

### Plain and custom-encoded names

Fairy names and area dialogue use null-terminated ASCII-compatible bytes.
Master names in `AFLDKWA.EMI` and `FIRST.EMI` use the same table encoding for
punctuation (for example `0x8e` is an apostrophe in `D'lonzo`).

### Trailing bytes after names

The name field is exactly 12 bytes. Bytes that appear after the visible
ASCII text but within the name area are either padding zeros or the start
of the next struct field (equipability, element, etc.), not part of the
name itself.

### Level growth stat packing (1 byte each)

| Byte | Upper nibble | Lower nibble |
| ---: | --- | --- |
| `pwr_dfn` | power | defense |
| `agi_int` | agility | intellect |

Values are 0–15 per nibble. Full stat = base_stat + nibble_value.

### Master skill encoding (2 bytes per slot)

| Bits | Field |
| ---: | --- |
| 15–8 | ability index |
| 7–0 | level required |

`0xFF` in the high byte = empty slot; `0x63` in the low byte is the
unused-level marker used by the table editor.

### Shop item reference (2 bytes per slot)

| Bits | Field |
| ---: | --- |
| 7–0 | item_type |
| 15–8 | item_index |

Item type codes: `0`=ItemObject, `1`=WeaponObject, `2`=ArmorObject,
`3`=AccessoryObject, `4`=KeyItemObject, `0xFF`=empty/zenny.

### Chest record (3 bytes)

| Byte | Field |
| ---: | --- |
| 0 | memory address byte (`0xFF` = empty) |
| 1 | item_index |
| 2 | item_type |

When `item_type = 0xFF`: zenny = `item_index × 40`.

## Formulas

### Steal/drop rate

```
rate = 2^value / 128    (as fraction, e.g. value=3 → 8/128 = 6.25%)
```

Value range 0–7. Value 0 = 0.8% (2^0/128).

### Monster resistance scale

Values 0–7 per element (see the monster resistance array in [areas](areas.md)).
Condition byte `0x63` (99) = unused block.

### Signed byte bonuses and modifiers

Only `MasterStatsObject` bonuses and the `BaseStatsObject` level-up modifier
bytes at `0x85`–`0x8a` are established signed-byte fields. Values `0x80`–`0xff`
decode as `value - 256`; observed master-stat range is -6 to +4. Base stat
fields themselves are little-endian 16-bit values.

### Zenny from chests

When `item_type = 0xFF`: zenny = `item_index × 40`.

## Traps to avoid

- **Two encoding systems**: Don't assume plain ASCII for game-data names.
  The custom encoding uses special bytes for punctuation and colors.
- **Name field padding**: Original names use `0x00` terminators/padding;
  rewritten randomizer names may use `0xff` fill. Trailing non-zero bytes
  after visible text are struct fields, not name characters.
- **Signed vs unsigned**: Do not reinterpret BaseStatsObject halfwords as
  signed bytes. Raw `0xfe` is -2 only in the proven signed-byte fields.
- **Pointer validity**: Not all pointer slots are filled. `0x00000` and
  `0xFFFFF` indicate empty/unused entries.
- **Cross-version differences**: v1.0 and v1.1 have different offsets
  for some tables (e.g., Manillo stock at 0x3E53A vs 0x3E53E).
- **Monster ID ≠ pointer index**: The pointer list index is not the
  monster ID. Multiple pointers can reference the same monster.
- **Master assignment**: Characters have `master=0xFF` in base stats.
  Master assignment is determined at runtime, not stored in the record.

## Evidence

- Binary verification: `out/index/vast-violence-1.1.json`
- Struct definitions: `third_party/references/vast-violence/tables/struct_*.txt`
