---
type: Spec reference
title: Game-data schema ledger
description: Evidence status for every record family currently documented by the BOF3 data specs.
tags: [tables, schemas, evidence]
---

# Game-data schema ledger

> A data-first checklist for recovering BOF3 records before lifting their code consumers.

## Scope and evidence

This ledger covers the fixed and pointer-backed record families listed in the
data specs. Record sizes, counts, locations, and byte boundaries are storage
verified against the US v1.1 corpus. Field semantics are tracked separately:
the pinned `vast-violence` layouts and existing specs identify candidates, but
runtime loads, stores, indexing, and behavior are required before a semantic
name becomes a C contract.

Status meanings:

- `storage` — size, count, coordinate, and byte range are verified.
- `access` — at least one runtime consumer proves field width or signedness.
- `semantic` — callers or behavior prove the field meaning.
- `unresolved` — bytes or semantics still require investigation.

## Source-catalog completeness

`third_party/references/vast-violence/tables/tables_list_1.1.txt` names 25
table records. The 23 populated struct families are listed in the ledger
below and have storage coordinates or pointer-map ownership. Two catalog
labels are explicit zero-byte placeholders (`empty.txt`, count `1`):
`EquipmentObject` and `MonsterAbilityObject`. They carry no serialized fields,
so there is no layout, ID namespace, or runtime C contract to recover yet.

## Fixed and pointer-backed records

| Domain | Record family | Owner/location | Count × size | Storage | Access | Semantic | Unresolved focus |
| --- | --- | --- | ---: | --- | --- | --- | --- |
| equipment | items | `GAME.EMI#0 @ 0x33964` | 92 × `0x12` | storage | access | partial | bytes after flags; flag bit meanings |
| equipment | key items | `GAME.EMI#0 @ 0x33fdc` | 16 × `0x10` | storage | pending | partial | 4 trailing bytes |
| equipment | weapons | `GAME.EMI#0 @ 0x340dc` | 83 × `0x18` | storage | access | partial | offsets `0x0d`, `0x11`, `0x13`–`0x15` |
| equipment | armor | `GAME.EMI#0 @ 0x348a4` | 68 × `0x16` | storage | access | partial | `0x0d`, `0x11`–`0x13` |
| equipment | accessories | `GAME.EMI#0 @ 0x34e7c` | 52 × `0x14` | storage | access | partial | `0x0d`, `0x10`–`0x11` |
| equipment | shops | `GAME.EMI#0 @ 0x3528c` | 40 × `0x17` | storage | pending | partial | slot sentinel and shop-specific behavior |
| equipment | abilities | `GAME.EMI#0 @ 0x3570c` | 228 × `0x14` | storage | access | partial | flag bits, control bytes, skill-type meanings; mode-gate names |
| equipment | level growth | `GAME.EMI#0 @ 0x368dc` | 693 × `0x08` | storage | access | partial | semantic names for offsets `0x02`–`0x07`; packed-stat consumers |
| characters | base stats | `START.EMI#8 @ 0x72914` | 8 × `0xa4` | storage | access | partial | unknown groups and mutable-state boundary |
| characters | `BaseStats2Object` (base-stats copy) | `STATUS.EMI#0 @ 0x1b114` | 8 × `0xa4` | storage | access | partial | copy ownership and lifecycle |
| characters | master skills | `SISYOU.EMI#0 @ 0x3c88` | 17 × `0x0c` | storage | pending | partial | empty-slot and level/ability interpretation |
| characters | master stats | `SISYOU.EMI#0 @ 0x3d54` | 17 × `0x06` | storage | pending | partial | signed application and clamping |
| characters | master names | `AFLDKWA.EMI @ 0x1be0` | 17 strings | storage | pending | partial | ownership versus duplicate in `FIRST.EMI` |
| characters | dragon pointer candidates | `STATUS.EMI#0 @ 0x1c018` | 10 × `u32` | storage | pending | unresolved | pointer ownership and consumer |
| characters | following STATUS region | `STATUS.EMI#0 @ 0x1c040` | unresolved | observed | pending | unresolved | boundaries and meaning |
| area | monsters | versioned pointer maps | 1,400 × `0x88` | storage | pending | partial | AI parameters, unknown ranges, ID/index split |
| area | formations | versioned pointer maps | 1,600 × `0x09` | storage | pending | partial | appearance-rate and inactive/boss semantics |
| area | chests | versioned pointer maps | 224 × `0x03` | storage | pending | partial | memory-address byte and save-state mapping |
| area | genes | `pointers_genes.txt` | 17 × `0x01` | storage | pending | partial | Infinity and patch-gated Flame behavior |
| area | chrysms | `pointers_chrysm.txt` | 13 × `0x01` | storage | pending | partial | gene/chrysm namespace separation |
| area | fairies | `pointers_fairies.txt` | 720 × `0x09` | storage | pending | unresolved | four stat bytes and identity indexing |
| fairy | gifts | `COMMU00.EMI @ 0x848` | 20 × `0x04` | storage | access | partial | runtime progression and reward dispatch |
| fairy | exploration items | `COMMU00.EMI @ 0x4218` | 48 × `0x02` | storage | access | partial | caller and row progression |
| fairy | prizes | `COMMU02.EMI @ 0x2d900` | 48 × `0x02` | storage | pending | partial | entry mapping and prize dispatch |
| area | Manillo items | `pointers_manillo_items_1.1.txt` | 165 × `0x08` | storage | pending | partial | trade IDs and fish namespace |
| area | Manillo stock | `AREA030.EMI @ 0x3e53e` | 16 × `0x0a` | storage | pending | partial | location IDs and v1.0 offset |

## GAME.EMI storage type maps

These are byte-exact storage maps, not ABI promises. Arrays are serialized
without compiler padding; multi-byte integers are little-endian. `unknown` and
`reserved` preserve bytes whose semantics are not yet established.

```text
ItemObject[92]      = u8 name[12]; u8 flags; u8 unknown[3]; u16 price;
KeyItemObject[16]   = u8 name[12]; u8 unknown[4];
WeaponObject[83]    = u8 name[12]; u8 equipability; u8 unknown[2];
                      u8 element; u8 weight; u8 unknown; u8 power;
                      u8 unknown[3]; u16 price;
ArmorObject[68]     = u8 name[12]; u8 equipability; u8 unknown;
                      u8 equip_type; u8 weight; u8 power; u8 unknown[3];
                      u16 price;
AccessoryObject[52] = u8 name[12]; u8 equipability; u8 unknown[2];
                      u8 weight; u8 unknown[2]; u16 price;
AbilityObject[228]  = u8 name[12]; u8 targeting_flags; u8 skill_type;
                      u8 cost; u8 power; u8 element; u8 ability_flags;
                      u8 reserved[2];
LevelObject[693]    = u16 exp; u8 hp; u8 ap; u8 power_defense;
                      u8 agility_intellect; u8 ability; u8 unknown;
ShopObject[40]      = u8 item_count; ShopItemRef slots[11];
ShopItemRef         = u8 item_type; u8 item_index;
FairyGiftObject[20] = le16 num_battles; u8 item_index; u8 item_type;
FairyExploreObject[48] = u8 item_index; u8 item_type;
```

The names above are storage-family names only. Runtime code may copy one of
these records into a differently shaped work object; that distinction is
resolved by the consumer-lift tasks.

## START/STATUS/SISYOU storage type maps

The base-stat record is byte-identical in its `START.EMI` and `STATUS.EMI`
locations. `le16` and `le32` describe serialized little-endian widths; they
do not settle signed arithmetic for every consumer.

```text
BaseStatsObject[8] =
  u8 name[5]; u8 character_index; u8 level; u8 unknown;
  le32 exp; le16 status;
  u8 weapon; u8 shield; u8 helmet; u8 armor; u8 accessories[2];
  le16 current_hp; le16 current_ap; u8 current_willpower;
  u8 innoculation; u8 fatigue; u8 master;
  le16 max_hp; le16 max_ap; le16 power; le16 defense;
  le16 agility; le16 intellect; le16 unknown;
  u8 willpower; u8 resistances[9];
  u8 surprise_chance; u8 reprisal_chance; u8 critical_chance;
  u8 evasion; u8 accuracy; u8 unknown[3];
  le16 base_hp; le16 base_ap; le16 base_power; le16 base_defense;
  le16 base_agility; le16 base_intellect; le16 unknown;
  u8 base_willpower; u8 base_resistances[9];
  u8 base_surprise_chance; u8 base_reprisal_chance;
  u8 base_critical_chance; u8 base_evasion; u8 base_accuracy;
  u8 unknown[3];
  u8 healing_abilities[10]; u8 assist_abilities[10];
  u8 attack_abilities[10]; u8 skill_abilities[10];
  u8 unknown_84; u8 level_up_modifiers[6]; u8 unknown[0x19];

MasterSkillsObject[17] = u8 skill_levels[6][2]; /* LE: level | ability << 8 */
MasterStatsObject[17]  = u8 stat_bonus[6];
DragonPointers         = le32 pointer[10];
```

The two `unknown` groups in `BaseStatsObject` at offsets `0x28` and `0x48`,
the three-byte groups at `0x39` and `0x59`, and the trailing `0x20` bytes are
preserved as separate ranges in [characters](characters.md). Dragon-growth
field meanings and pointer interpretation remain unresolved.

## Area-data storage type maps

Pointer-map indexes are archive-local references. They are not interchangeable
with the embedded monster IDs or the global item/ability namespaces.

```text
EnemyObject[1400] =
  u8 name[8]; le16 enemy_id; u8 choice_ai; u8 unknown[3];
  u8 target_preference; u8 unknown; le16 zenny; le16 exp; u8 level;
  u8 unknown[3]; u8 initial_skills[8];
  le16 hp; le16 ap; le16 power; le16 defense; le16 agility; le16 intellect;
  u8 steal_item_index; u8 steal_item_type; le16 steal_rate;
  u8 drop_item_index; u8 drop_item_type; le16 drop_rate;
  EnemyAiBlock ai[4]; u8 unknown[4]; u8 resistances[9]; u8 unknown[7];
EnemyAiBlock         = u8 condition; u8 parameters[7]; u8 skills[8];
FormationObject[1600] = u8 enemy_indexes[8]; u8 appearance_rate;
ChestObject[224]      = u8 memory; u8 item_index; u8 item_type;
GeneObject            = u8 gene_index;
ChrysmObject          = u8 gene_index;
FairyObject[720]      = u8 name[5]; u8 stats[4];
ManilloItemObject[165] = u8 item_index; u8 item_type;
                         u8 fish_indexes[3]; u8 fish_quantities[3];
ManilloStockObject[16] = u8 trade_indexes[10];
```

The upstream table catalog labels `EnemyObject` as `MonsterObject`; this
project uses `EnemyObject` consistently. `EnemyAiBlock` is a storage grouping
only; its seven parameter bytes are not
named until an AI consumer proves their individual meanings. Formation slots,
fish indexes, and empty sentinels remain namespace-specific even where they
use the same byte value (`0xff`).

## Runtime contracts and match status

These target-local contracts are promoted only after the serialized layout and
the consuming instruction stream agree. They are not shared engine headers.

| Target/function | Contract proven | Evidence |
| --- | --- | --- |
| `BATTLE.EMI#3 @ 0x801d3844` | ability records are `0x14` bytes; `+0x0c` is the selector flag byte; state at `0x801463c0` is a `u16` | 376/376 byte exact match |
| `BATTLE.EMI#3 @ 0x801e4368` | `0x80146374` and `0x80146375` are byte globals; `0x801463c0` is a halfword; random helper result is signed 32-bit for `% 100` | 296/296 byte exact match |
| `GAME.EMI#0 @ 0x80196ffc` | payload base is `0x80195800` (the target's first function is `0x8019611c`); entry state at `+0x3b90` is `u16`, palette serial at `+0x5988` is `u8` | 85.71% candidate; width contract confirmed, scheduler residue remains |
| `SLUS_004.22 @ 0x800df548` | category/index dispatch selects item, weapon, armor, accessory, or key-item bases with strides `0x12`, `0x18`, `0x16`, `0x14`, and `0x10`; category values `>=5` fall back to items | reviewed disassembly; target-local C candidate is 70.21% with equal size |
| `SLUS_004.22 @ 0x800df5ec` | key-item index is masked to `u8`, scaled by `0x10`, and added to `0x801c8fdc` | disassembly contract confirmed; C candidate is 16.67% without inline assembly |

## Validated consumers outside exact-match promotions

These observations come directly from reviewed disassembly and are kept
separate from the exact-match contracts above until their C lifts match.

| Target/function | Observation | Evidence boundary |
| --- | --- | --- |
| `GAME.EMI#0 @ 0x801addd4` | runtime character records use base `0x80144968`, stride `0xa4`; level is `+0x06`, exp `+0x08`; six base-stat halfwords at `+0x3c`–`+0x46` are updated from level rows, with signed modifiers at `+0x85`–`+0x8a` | reviewed raw instructions; target-local C candidate at 39.71%, exact promotion pending |
| `GAME.EMI#0 @ 0x801af5b0` | ability records use `kind × 0x14`; offset `0x0c` is a flag byte and offset `0x10` is loaded as a halfword for selection checks | reviewed raw instructions; target-local C candidate at 51.35%, exact promotion pending |
| `COMMU00.EMI#0 @ 0x801f18f8` | gift table is copied as 20 × `0x04` from runtime `0x801eec48` | reviewed raw instructions; target-local C candidate at 23.26%, exact promotion pending |
| `COMMU00.EMI#0 @ 0x801f1bc8` | exploration table at runtime `0x801f2618` is indexed as `row × 2`; item index/type bytes are passed to reward and name consumers | reviewed raw instructions; target-local C candidate at 22.22%, exact promotion pending |

The exact-match source uses external array symbols and consumer-local pointer
casts where those declarations are necessary to reproduce the original ABI.
The symbols remain in the owning target's `symbols.c`; they do not establish
cross-overlay identity.

## Unknown-byte corpus classifications

The following classifications come from a v1.1 row-variance scan. They are
storage evidence, not runtime semantic names.

| Record | Range | Corpus observation | Classification |
| --- | --- | --- | --- |
| item | `0x0d` | constant zero across 92 rows | reserved/unused candidate |
| item | `0x0e`–`0x0f` | varies; `0x0f` has two corpus values | unresolved semantic bytes |
| key item | `0x0e`–`0x0f` | constant zero across 16 rows | reserved/unused candidate |
| key item | `0x0c`–`0x0d` | varies across rows | unresolved semantic bytes |
| weapon | `0x13` | constant zero across 83 rows | reserved/unused candidate |
| weapon | `0x0d`–`0x0e`, `0x11`, `0x14` | varies across rows | unresolved semantic bytes |
| weapon | `0x15` | constant `0x40` across 83 rows | constant storage byte; meaning unresolved |
| armor | `0x11` | constant zero across 68 rows | reserved/unused candidate |
| armor | `0x0d`, `0x12` | varies across rows | unresolved semantic bytes |
| armor | `0x13` | two corpus values (`0x40`/`0x41`) | constant-class byte; meaning unresolved |
| accessory | `0x0d`–`0x0e`, `0x10` | varies across rows | unresolved semantic bytes |
| accessory | `0x11` | two corpus values (`0x40`/`0x41`) | constant-class byte; meaning unresolved |
| ability | `0x12`–`0x13` | `0x12` varies; `0x13` has two corpus values | unresolved semantic bytes |
| level | `0x07` | constant zero across 693 rows | reserved/unused candidate |
| base stats | `0x29`–`0x2b`, `0x39`–`0x3b`, `0x49`, `0x59`–`0x5b`, `0x8b`–`0xa3` | constant zero in all eight rows | reserved/unused candidates |
| base stats | `0x85`–`0x8a` | runtime level-up modifiers, signed byte loads | access-proven; individual stat labels follow the six base-stat order |
| base stats | `0x07`, `0x28`, `0x48` | one nonzero outlier each | unresolved semantic bytes |
| monster | `0x0b`–`0x0d`, `0x0f`, `0x15` | constant zero across 1,400 rows | reserved/unused candidates |
| monster | `0x16`–`0x17`, `0x74`–`0x77`, `0x81`–`0x87` | varies in a subset of rows | unresolved AI/record bytes |

Constant values can still be deliberate sentinels or type markers. They are
not renamed or removed until a consumer confirms their role.

## Shared namespaces to recover

These are intentionally separate until a caller proves a conversion:

- item category and item index;
- key-item index;
- weapon, armor, and accessory indexes;
- ability ID;
- playable-character ID and party slot;
- area-local monster index and global monster ID;
- formation index;
- master ID;
- gene ID and chrysm ID;
- shop, fairy, Manillo trade, and location IDs;
- battle selection kind and runtime status flags.

## Recovery order

1. Decode every storage layout and coordinate from the owning data spec.
2. Extract all IDs, masks, packed fields, and sentinels into namespace tables.
3. Compare duplicate records and versioned locations byte-for-byte.
4. Trace one consumer per family to prove access width and signedness.
5. Promote target-local C89 declarations only after steps 1–4 agree.

## Next lift queue

The remaining work is ordered by evidence already available in this checkout:

1. `GAME.EMI#0 @ 0x801af5b0` — continue the existing ability-selection
   candidate toward an exact match. The `0x14`-byte stride and `+0x0c`/`+0x10`
   accesses are already disassembly-backed, so this is the next low-risk
   replacement loop.
2. `GAME.EMI#0 @ 0x801addd4` — finish the level-up consumer. Its mutable
   character stride, packed growth bytes, and signed modifier range are known;
   the remaining work is compiler/control-flow matching and recalculation
   ownership.
3. Master, monster, formation, chest, and Manillo consumers — promote or lift
   only after their owning EMI entry has code evidence and a target-local
   source directory. Their storage layouts are complete, but no consumer
   contract is recorded yet.

Each queue item remains a candidate until `bin/harness diff` reports an exact
byte match; an address-only symbol or target-local view is preferred while
field semantics remain unresolved.

## Acceptance criteria

A record family is complete when every byte range is classified, its IDs and
sentinels have an owning namespace, version differences are recorded, and at
least one runtime consumer confirms any field whose C type depends on width or
signedness. Remaining behavior-only questions stay explicitly `unresolved`.

## Sources

- [Data-table index](index.md)
- [Equipment](equipment.md)
- [Fairy reward data](fairies.md)
- [Characters](characters.md)
- [Area data](areas.md)
- [Encoding](encoding.md)
- `out/reports/vast-violence-1.1.json`
- `third_party/references/vast-violence/tables/struct_*.txt`
