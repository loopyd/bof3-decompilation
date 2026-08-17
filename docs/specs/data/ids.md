---
type: Spec reference
title: Game-data IDs and encodings
description: Namespaces, masks, packed values, and sentinels used by the documented BOF3 data records.
tags: [ids, enums, flags, encoding]
---

# Game-data IDs and encodings

> Namespace boundaries are explicit; equal byte values do not imply equal IDs.

## Namespace matrix

| Namespace | Width | Proven range/values | Owner | Status |
| --- | ---: | --- | --- | --- |
| item category | `u8` | `0` item, `1` weapon, `2` armor, `3` accessory, `4` key item | `GAME.EMI` dispatch | verified by `SLUS_004.22 @ 0x80165d48` (`getEquipRecordBase`, exact) |
| empty item category | `u8` | `0xff` | shop/chest/drop contexts | context-dependent |
| item index | `u8` | `0x00`–`0x5b` | `ItemObject` | storage-verified |
| fish item index | `u8` | `0x38`–`0x4c` | item table / Manillo records | storage-verified |
| ability index | `u8` | `0x00`–`0xe3` (228 records) | `AbilityObject` | storage-verified |
| level-growth record index | array index | `0`–`0x2b4` (693 records) | `LevelObject` | storage-verified |
| character record slot | array index | `0`–`7` | `BaseStatsObject` | storage-verified |
| character ID field | `u8` | Whelp stores `0x0a`; not equal to slot `7` | base stats | verified outlier; conversions pending |
| master ID | array index | `0`–`0x10` | master names/skills/stats | storage-verified |
| enemy ID | `le16` | `0x0001`–`0x04a8` observed | `EnemyObject @ 0x08` | storage-verified |
| area-local monster index | pointer-map index | archive-local | formation slots | distinct from monster ID |
| formation index | pointer-map index | archive-local | formation table | storage-verified |
| gene byte | `u8` | `0x00`–`0x11`; `0x21` patch-gated Flame | gene records | storage-verified |
| chrysm byte | `u8` | same byte layout as gene records | chrysm records | separate namespace |
| shop index | array index | `0`–`0x27` (40 records) | `ShopObject` | storage-verified |
| fairy record index | pointer-map index | archive-local | fairy records | storage-verified |
| fairy gift index | array index | `0`–`0x13` (20 records) | `FairyGiftObject` | storage-verified |
| fairy exploration index | array index | `0`–`0x2f` (48 records) | `FairyExploreObject` | storage-verified |
| fairy prize index | array index | `0`–`0x2f` (48 records) | `FairyPrizeObject` | storage-verified |
| Manillo trade index | pointer-map index | archive-local | Manillo records | storage-verified |

Status vocabulary: `verified` means a live runtime selector (named in the
row) reproduces the range; `storage-verified` means the range is recorded
structural provenance from the tracked vast-violence table extraction with no
tracked byte-verifier re-check (see [Evidence boundary](#evidence-boundary)).

## Character equipability mask

The `u8` equipability mask layout is defined in
[encoding.md](encoding.md#equipability-1-byte); it is not duplicated here.

## Element masks and resistance slots

Equipment and ability element mask bit positions are defined in
[encoding.md](encoding.md#element-1-byte); they are not duplicated here.
Monster and character resistance arrays have nine byte slots. Their labels
use `Frost` and `Thunder` for the second and third slots; whether those are
the display names or distinct runtime elements remains a conversion question.
The ninth slot is `Death` and has no element-mask bit.

## Ability skill class

The low two bits of the ability skill-type byte are documented as:

| Value | Candidate class |
| ---: | --- |
| `0` | healing |
| `1` | assist |
| `2` | attack |
| `3` | examinable / Blue Magic |

The class names are corroborated by the pinned randomizer and existing table
notes. `BATTLE.EMI#3 @ 0x801daae4` reads the byte at ability offset `0x0d`
and branches on `0`, `1`, and `3`; the class labels remain candidates until
that larger consumer is an exact lift.

`GAME.EMI#0 @ 0x801af5b0` independently tests ability `+0x0c` bits `0` and
`1` as mode gates and ability `+0x10` bit `0x400` as a halfword selection gate.
These masks are runtime evidence, not finalized public enum names.

## Other packed values and sentinels

| Encoding | Meaning |
| --- | --- |
| master skill pair high byte | required level; `0xff`/`0x63` are empty/unused markers in different contexts |
| level-growth byte `0x04` | power high nibble, defense low nibble |
| level-growth byte `0x05` | agility high nibble, intellect low nibble |
| chest `item_type = 0xff` | zenny; amount is `item_index × 40` |
| formation monster index `0xff` | empty slot |
| pointer-map offset `0x00000`/`0xfffff` | empty pointer slot |
| monster condition `0x63` | unused AI/skill block |
| master ID `0xff` in base stats | no statically assigned master |

## Evidence boundary

The tables and masks above are storage/spec facts. A shared C enum or named
constant additionally requires runtime comparisons or indexing in a consumer;
until then, use namespace-qualified integer constants and preserve raw bytes.

Sources: [schema ledger](schema-ledger.md), [equipment](equipment.md),
[fairy rewards](fairies.md), [characters](characters.md), [area data](areas.md), [encoding](encoding.md),
and `third_party/references/vast-violence/randomizer.py` for explicitly marked
candidates.
