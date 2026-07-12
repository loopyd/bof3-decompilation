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
| 7 | `0x80` | target_enemy |
| 6 | `0x40` | target_ally |
| 5 | `0x20` | selectable_target |
| 4 | `0x10` | target_all |
| 3 | `0x08` | unknown_3 |
| 2 | `0x04` | unknown_2 |
| 1 | `0x02` | usable_battle |
| 0 | `0x01` | usable_menu |

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

### Plain ASCII

Used for master names (AFLDKWA.EMI, FIRST.EMI), fairy names, and area
dialogue text. Standard null-terminated ASCII with no special bytes.

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
| 15–8 | level required |
| 7–0 | ability index |

`0xFF` in high byte = empty slot.

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

### Base stats signed bytes

Values 0x80–0xFF are negative (signed byte): `value - 256`.
Observed range for master stats: -6 to +4.

### Zenny from chests

When `item_type = 0xFF`: zenny = `item_index × 40`.

## Traps to avoid

- **Two encoding systems**: Don't assume plain ASCII for game-data names.
  The custom encoding uses special bytes for punctuation and colors.
- **Name field padding**: Names shorter than 12 bytes are padded with
  `0x00`. Trailing non-zero bytes after visible text are struct fields,
  not name characters.
- **Signed vs unsigned**: Stat values (master stats, base stats) use
  signed bytes. Raw hex `0xFE` = -2, not 254.
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
