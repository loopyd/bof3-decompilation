---
type: Table
title: Equipment and shops
description: Items, weapons, armor, accessories, abilities, shops, and level growth — all from GAME.EMI.
tags: [tables, equipment, emi]
---

# Equipment and shop data

All tables below live in entry `0` of `BIN/ETC/GAME.EMI`. Locations are raw
archive offsets; subtract the entry payload start (`0x800`) for payload-relative
offsets. Verified against the US `BOF3_1.1` corpus with zero boundary failures.
Here, `1.1` is the pinned vast-violence corpus label for the input's exact Track
1 MD5; it is not a claim about a separately catalogued US retail revision.

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

### Runtime item-reference dispatch

`SLUS_004.22` function `0x800df548` masks the category and dispatches record
address calculation. The five handled categories use the fixed runtime bases
and strides below; the function then enters a shared continuation at
`0x80165dfc`.

| Item type | Record family | Runtime base | Stride | Index calculation |
| ---: | --- | ---: | ---: | --- |
| `0` | item | `0x801c8964` | `0x12` | `index × 18` |
| `1` | weapon | `0x801c90dc` | `0x18` | `index × 24` |
| `2` | armor | `0x801c98a4` | `0x16` | `index × 22` |
| `3` | accessory | `0x801c9e7c` | `0x14` | `index × 20` |
| `4` | key item | `0x801c8fdc` | `0x10` | `index × 16` |

This verifies the five non-empty item-type codes and their record families,
consistent with the shop and inventory reference encoding. The `0xff`
empty/zenny value is handled by callers outside this dispatcher and remains
context-dependent.

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

#### Battle selection-kind view

Battle selection code indexes the same `0x14`-byte stride beginning at
`0x801ca70c + 0x0c`:

| Runtime address | Ability-record offset | Selection use | Evidence |
| ---: | ---: | --- | --- |
| `0x801ca718 + kind × 0x14` | `0x0c` | selection flags; tested with `0x40`, `0x10`, `0x80`, `0x20` | `BATTLE.EMI#15 @ 0x8009761c` |
| `0x801ca71c + kind × 0x14` | `0x10` | selection mask, loaded as `u16` | `BATTLE.EMI#15` internal contract |
| `0x801ca71e + kind × 0x14` | `0x12` | selection name/resource ID, loaded as `u16` | `BATTLE.EMI#15` internal contract |

This proves an alternate runtime view of the ability-stride table. It does
not replace the ability-table field names: a shared C declaration must preserve
the byte offsets and support both consumers.

#### Code-level confirmations

| Target and address | Access | Confirmed interpretation |
| --- | --- | --- |
| `BATTLE.EMI#3 @ 0x801d3844` | `ability[kind] + 0x0c` | selection flags drive the returned target/mode code |
| `BATTLE.EMI#3 @ 0x801daae4` | `ability[kind] + 0x0d` | skill type separates attack, assist, and healing rank bonuses |
| `BATTLE.EMI#15 @ 0x8009761c` | `+0x0c`, `+0x10`, `+0x12` | selection flags, mask, and 16-bit selector resource value |
| `SLUS_004.22 @ 0x800df5ec` | `item_index & 0xff`, then `× 0x10` | key-item record accessor at `0x801c8fdc` |

`SLUS_004.22 @ 0x800df604` is a second category-indexed accessor. For the
handled equipment categories it reads item `+0x0d`, weapon `+0x0e`, armor
`+0x0e`, and accessory `+0x0e`; the accessory result is masked to its low
nibble. Categories outside those branches fall through to a shared runtime
continuation and are not assigned a shared field name here.

`GAME.EMI#0 @ 0x801af5b0` indexes the ability table as `kind × 0x14`. It tests
`+0x0c` bit `0` for one selection mode and bit `1` for another, then loads
`+0x10` as a halfword and tests bit `0x400` in the second mode. These are
runtime flag uses; their public names remain candidates. A readable target-local
candidate now exists at `src/emi/etc/game/00/func_801af5b0.c`; it measures
51.35% under canonical `-O2`, so the ability-gate names and exact function
replacement remain unpromoted.

These are consumer facts, not a claim that every byte has one global semantic
name. The target-local `Battle03AbilityRecordView` keeps the proven offsets
typed while preserving the `+0x10` byte/halfword overlay.

The `BATTLE.EMI#3 @ 0x801d3844` lift now matches all 376 bytes. The exact
match required real external array symbols for the random tables and ability
records, plus a retained pointer to the halfword state global; these bindings
are target-local and do not promote a shared runtime ABI.

### Level growth (8 bytes)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 2 | exp |
| `0x02` | 1 | hp |
| `0x03` | 1 | ap |
| `0x04` | 1 | power + defense (packed: pwr<<4|dfn) |
| `0x05` | 1 | agility + intellect (packed: agi<<4|int) |
| `0x06` | 1 | ability (slot unlock indicator) |
| `0x07` | 1 | second ability-related byte (candidate) |

693 records. 99 levels × 7 characters (Ryu, Nina, Garr, Teepo, Rei, Momo,
Peco). Character index = `record_index // 99`. Level = `(record_index % 99) + 1`.
Ability `0x00` = no skill unlocked; non-zero = ability index learned at that level.

#### Runtime consumer

`GAME.EMI#0 @ 0x801addd4` confirms the serialized widths and packed-byte
accesses. The function computes a character block as `character_index × 99`,
then reads `u16` at record offset `0x00` while walking level counters from `1`
through `< 99`. The same record base is subsequently read as `u8` at offsets
`0x02` and `0x03`, as packed nibbles at offsets `0x04` and `0x05`, and as
individual bytes at offsets `0x06` and `0x07`. The runtime base used by those
loads is `0x801cb8dc`, matching the payload coordinate in the index.

This proves the access widths and that both trailing bytes participate in level
processing. Both trailing values are passed independently to the same ability
registration call, so `0x07` is retained as an ability-related candidate rather
than an unused byte; final semantic names remain unresolved until the full
consumer is lifted.

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
- Runtime dispatch: `SLUS_004.22` @ `0x800df548`
- Validation: `out/index/vast-violence-1.1.json`
- Struct definitions: `third_party/references/vast-violence/tables/struct_*.txt`
- Ability names: `third_party/references/vast-violence/ability_names.txt` (228 abilities)
- Shop names: `third_party/references/vast-violence/tables/names_shops.txt` (40 shops)
