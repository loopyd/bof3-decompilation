---
type: Table
title: Equipment and shops
description: Items, weapons, armor, accessories, abilities, shops, and level growth — all from GAME.EMI.
tags: [tables, equipment, emi]
---

# Equipment and shop data

All tables below live in entry `0` of `BIN/ETC/GAME.EMI`. Locations are raw
archive offsets; subtract the entry payload start (`0x800`) for payload-relative
offsets. Verified against the US v1.1 disc with zero boundary failures.

## Fixed table locations

| Archive offset | Records | Size | Content |
| ---: | ---: | ---: | --- |
| `0x33964` | 92 | `0x12` | items |
| `0x33fdc` | 16 | `0x10` | key items |
| `0x340dc` | 83 | `0x18` | weapons |
| `0x348a4` | 68 | `0x16` | armor |
| `0x34e7c` | 52 | `0x14` | accessories |
| `0x3528c` | 40 | `0x17` | shops |
| `0x3570c` | 228 | `0x14` | abilities |
| `0x368dc` | 693 | `0x08` | level growth |

## Record layouts

### Items (18 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | `0x0c` | name (12-byte inline string) |
| `0x0c` | 1 | flags — bit: `target_both` `target_selectable` `target_enemy_default` `target_all` `story_item` `show_name` `show_animation` `usable_menu` |
| `0x0d` | 3 | unknown |
| `0x10` | 2 | price (little-endian) |

92 records. Index `0x00` is "Nothing". Indices `0x01`–`0x37` are consumables
and key items; `0x38`–`0x4C` are fish (used in manillo trades); `0x4D`–`0x5B`
are special items.

### Key items (16 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | `0x0c` | name (12-byte inline string) |
| `0x0c` | 4 | unknown |

16 records. Key items are story-critical and cannot be sold.

### Weapons (24 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | `0x0c` | name (12-byte inline string) |
| `0x0c` | 1 | equipability (see bitmask above) |
| `0x0d` | 2 | unknown |
| `0x0f` | 1 | element (see bitmask above) |
| `0x10` | 1 | weight |
| `0x11` | 1 | unknown |
| `0x12` | 1 | power |
| `0x13` | 3 | unknown |
| `0x16` | 2 | price (little-endian) |

83 records. Verified: `Dagger` (01) equip=0x19 (Ryu+Teepo+Rei),
`PointedStick` (1D) equip=0x02 (Nina), `Claws` (37) equip=0x40 (Peco).

### Armor (22 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | `0x0c` | name (12-byte inline string) |
| `0x0c` | 1 | equipability (see bitmask above) |
| `0x0d` | 1 | unresolved byte |
| `0x0e` | 1 | equip type: shield `2`, helmet `3`, body `4` |
| `0x0f` | 1 | weight |
| `0x10` | 1 | power (defense) |
| `0x11` | 3 | unknown |
| `0x14` | 2 | price (little-endian) |

68 records. Equip type values: `2` = shield, `3` = helmet, `4` = body armor.
Verified: `Bandana` (21) etype=3, `Bracers` (32) etype=2, `Clothing` (01) etype=4.

### Accessories (20 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | `0x0c` | name (12-byte inline string) |
| `0x0c` | 1 | equipability — bit: `ryu` `nina` `garr` `teepo` `rei` `momo` `peco` `whelp` |
| `0x0d` | 2 | unknown |
| `0x0f` | 1 | weight |
| `0x10` | 2 | unknown |
| `0x12` | 2 | price (little-endian) |

52 records. Indices `0x1C`–`0x33` are fishing tackle (equip=0x00).

### Abilities (20 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | `0x0c` | name (12-byte inline string) |
| `0x0c` | 1 | targeting/display flags — bit: `u0` `u1` `u2` `u3` `u4` `default_target_enemy` `u6` `u7` |
| `0x0d` | 1 | skill type (lower 2 bits); `3` = examinable |
| `0x0e` | 1 | cost (MP) |
| `0x0f` | 1 | power |
| `0x10` | 1 | element — bit: `fire` `ice` `lightning` `earth` `wind` `holy` `psionic` `status` |
| `0x11` | 1 | ability flags — bit: `affects_stats` `examinable` `target_ally_default` |
| `0x12` | 2 | reserved/control bytes |

228 records. Skill type `0x03` marks examinable abilities (Blue Magic).
Verified: `Nue Stomp` (01) stype=0x13 (attack+examinable), `Gambit` (02) stype=0x02 (assist).

Indices 0xAE–0xD6 are copies of 0x46–0x70 (master skill variants).

### Level growth (8 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 2 | exp |
| `0x02` | 1 | hp |
| `0x03` | 1 | ap |
| `0x04` | 1 | power + defense (packed: pwr<<4|dfn) |
| `0x05` | 1 | agility + intellect (packed: agi<<4|int) |
| `0x06` | 1 | ability (slot unlock indicator) |
| `0x07` | 1 | unknown |

693 records. 99 levels × 7 characters (Ryu, Nina, Garr, Teepo, Rei, Momo,
Peco). Character index = `record_index // 99`. Level = `(record_index % 99) + 1`.
Ability `0x00` = no skill unlocked; non-zero = ability index learned at that level.

### Shops (23 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 1 | num_items |
| `0x01` | `0x16` | item references — 11 × 2 bytes (item type + item index per slot) |

40 records. Shop names verified from `names_shops.txt`: indices 0x11–0x16 are
fairy village shops (not sorted during cleanup).

### Shop item reference (2 bytes per slot)

| Bits | Field |
| ---: | --- |
| 7–0 | item_type |
| 15–8 | item_index |

Item type codes: `0`=ItemObject, `1`=WeaponObject, `2`=ArmorObject,
`3`=AccessoryObject, `4`=KeyItemObject, `0xFF`=empty/zenny.

## Evidence

- Source file: `out/extracted/BIN/ETC/GAME.EMI`
- Validation: `out/index/vast-violence-1.1.json`
- Struct definitions: `third_party/references/vast-violence/tables/struct_*.txt`
- Ability names: `third_party/references/vast-violence/ability_names.txt` (228 abilities)
- Shop names: `third_party/references/vast-violence/tables/names_shops.txt` (40 shops)
