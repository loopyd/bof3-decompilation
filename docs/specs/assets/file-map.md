---
type: Reference
title: File source-of-truth map
description: Which EMI file owns each data domain, how data flows through the engine, and why duplication exists.
tags: [assets, emi, data-flow]
---

# EMI file source-of-truth map

Every data domain has one authoritative source file. This document records
what that file is, how the engine reaches it, and why some data appears in
more than one place.

## Source-of-truth by data domain

### Equipment and abilities

| Data | Source file | Offset | Record count |
| --- | --- | --- | --- |
| Items | `ETC/GAME.EMI` | 0x00000 | 92 |
| Weapons | `ETC/GAME.EMI` | 0x0D800 | 83 |
| Armor | `ETC/GAME.EMI` | 0x1A800 | 68 |
| Accessories | `ETC/GAME.EMI` | 0x24000 | 52 |
| Abilities | `ETC/GAME.EMI` | 0x2C000 | 228 |
| Shops | `ETC/GAME.EMI` | (pointer table) | 40 |
| Level growth | `ETC/GAME.EMI` | (pointer table) | 693 |

No other file contains equipment or ability definitions. `GAME.EMI` is the
single source.

### Character data

| Data | Source file | Offset | Notes |
| --- | --- | --- | --- |
| Base stats | `ETC/START.EMI` | 0x72914 | 8 chars × 164 bytes, signed |
| Base stats copy | `ETC/STATUS.EMI` | 0x1B114 | Byte-identical to START.EMI |
| Master skills | `ETC/SISYOU.EMI` | 0x3C88 | 17 × 12 bytes |
| Master stats | `ETC/SISYOU.EMI` | 0x3D54 | 17 × 6 bytes, signed |
| Master names | `ETC/AFLDKWA.EMI` | 0x1BE0 | 17 null-terminated ASCII |
| Master names copy | `ETC/FIRST.EMI` | 0x3CBE0 | Same strings |

`START.EMI` is the authoritative base-stats source. `STATUS.EMI` holds a
verified byte-identical copy. The randomizer enforces that these always
match via its `cleanup()` method.

### Fairy system

| Data | Source file | Offset | Record count |
| --- | --- | --- | --- |
| Fairy gifts | `ETC/COMMU00.EMI` | 0x0848 | 20 × 4 bytes |
| Fairy explore items | `ETC/COMMU00.EMI` | 0x4218 | 48 × 2 bytes |
| Fairy data | `ETC/COMMU00.EMI` | (pointer table) | 720 records |
| Fairy prizes | `ETC/COMMU02.EMI` | 0x2D900 | 48 × 2 bytes |

Fairy data is split across two EMI files with no overlap.

### Dragon system

| Data | Source file | Offset | Size |
| --- | --- | --- | --- |
| Dragon data | `ETC/STATUS.EMI` | 0x1C018 | 20 bytes (10 × uint16 runtime pointers) |
| Dragon growth | `ETC/STATUS.EMI` | 0x1C02C | 600 bytes (100 × 6 growth table) |
| Dragon genes | `SCENARIO/SCENA03.EMI`, `SCENA08.EMI` | per-entry | 4 genes in SCENARIO |
| Dragon genes | `WORLD*/AREA*.EMI` | per-entry | 13 genes in WORLD |
| Chrysms | `WORLD*/AREA*.EMI` | per-entry | 13 records |

Dragon data (pointers, growth table) lives only in `STATUS.EMI`. Gene
locations are split between SCENARIO and WORLD archives.

### Randomizer seeds

| Data | Source file | Offset | Size |
| --- | --- | --- | --- |
| Seed (v1.1) | `ETC/STATUS.EMI` | 0x23998 | 6 bytes |
| Seed (v1.0) | `ETC/STATUS.EMI` | 0x285D4 | 13 bytes |

Only `STATUS.EMI` contains these. The v1.0 seed is in a different location
than v1.1.

### Per-area data

Each `WORLD*/AREA*.EMI` file is the sole source for its area's:

| Data | Record size | Total records | Archives involved |
| --- | --- | --- | --- |
| Monster records | 136 bytes | 1,400 | 200 |
| Formation records | 9 bytes | 1,600 | 200 |
| Chest records | 3 bytes | 224 | 77 |
| Gene records | 1 byte | 17 | 15 |
| Chrysm records | 1 byte | 13 | 13 |
| Fairy records | 9 bytes | 720 | 12 |
| Manillo item records | 8 bytes | 165 | 3 |

Monsters in a formation must come from the same archive as the formation.
There is no cross-area monster lookup.

### Battle engine

| Data | Source file | Notes |
| --- | --- | --- |
| Main battle code | `BATTLE/BATTLE.EMI` | Load address 0x801D0C00 |
| Battle code copy | `BATTLE/BATTLE2.EMI` | Identical content |
| Boss battle copies | `BOSS/BOSS001-055.EMI` | Each contains a full battle engine copy |

The battle engine is duplicated into every boss file. This is a deliberate
disc-layout optimization.

### Enemy audio

| Data | Source file | Notes |
| --- | --- | --- |
| Enemy audio banks | `BENEMY/ENEMY000-199.EMI` | Audio samples only, no code |
| Unique banks | 67 of 200 files | 155 files are duplicates |
| Mapping | Monster ID N → `ENEMY{N-1}.EMI` | |

### Other system files

| Data | Source file | Notes |
| --- | --- | --- |
| Shop system code | `ETC/SHOP.EMI` | 237 KB |
| Battle events | `ETC/BATE.EMI` | Beyd, Whelp references |
| Demo/cutscenes | `ETC/DEMO.EMI` | 968 KB |
| End credits | `ETC/ENDKANJI.EMI` | Kanji text |
| Save template | `ETC/SHISU.EMI` | BASLUS-00422 region code |
| Debug tools | `ETC/MTEST.EMI`, `RTEST.EMI` | Debug only |

## Boot and system files (non-EMI)

| File | Role |
| --- | --- |
| `SYSTEM.CNF` | Boot config, points to `SLUS_004.22` |
| `SLUS_004.22` | Main executable: disc access, slot resolution, EMI parsing |
| `LOGO.EXE` | Capcom logo splash |
| `license_data.dat` | PS-X license/region data |
| `LOGO/CAPCOM30.STR` | Capcom logo XA audio |
| `SCE_XA/VOICE.STR` | Voice/cutscene XA audio |
| `SCE_XA/S_XA00.STR` | Voice/cutscene XA audio |
| `BMAG_XA/MAGIC00.STR` | Battle magic XA audio |

## Data flow: how the engine loads EMI data

### Boot chain

```
SYSTEM.CNF → SLUS_004.22 → LOGO.EXE / EMI entries / STR-XA media
```

`SLUS_004.22` owns disc access, slot resolution, EMI parsing, and payload
streaming. It does not load all EMI files at once.

### Slot table

The engine uses a slot table (`src/core/disc/slot_table_data.c`) that maps
logical slots to disc LBAs. Known slots include DEMO, FIRST, GAME,
SCENA16, CAPCOM30.STR, LOGO.EXE.

### Entry dispatch by type

| Type | Behavior |
| --- | --- |
| 0 | Direct RAM copy (code/data, no decompression) |
| 1–2 | Queued RAM load |
| 3 | Graphics upload (CLUT + texture data) |
| 6–7 | VAB audio (sound banks) |
| 10 | Sequence (music) |

### Runtime load regions

Memory is reused across gameplay states:

| Address | Purpose |
| --- | --- |
| 0x801D0C00 | Shared frontend/game-mode/battle overlay base |
| 0x801EEC00 | Battle/effect overlay region |
| 0x8003B800 | Character/effect work region |
| 0x80104000 | World/area code-data region |

### On-demand streaming

The engine streams EMI files from disc as needed. When a player enters a
new area, the engine loads that area's `AREA*.EMI` and extracts monster
formations, chests, and dialogue. When a battle starts, the engine loads
the relevant `ENEMY*.EMI` audio bank and `BPLCHAR` sprite data.

## Why duplication exists

### PSX disc seek time

The PSX CD-ROM drive has a seek time of roughly 200–800ms depending on
track distance. Loading the same data from a nearby disc location is faster
than seeking to a single authoritative location far away on the disc.

### Design rationale

The original developers deliberately duplicated data to minimize seek
times during gameplay:

- **Base stats** appear in both `START.EMI` and `STATUS.EMI` so the
  status menu can read them without seeking back to `START.EMI`.
- **Master names** appear in both `AFLDKWA.EMI` and `FIRST.EMI` for the
  same reason.
- **Battle engine code** is copied into every `BOSS*.EMI` file so boss
  encounters can load from a single nearby disc location.
- **Per-area data** is self-contained in each `AREA*.EMI` so entering an
  area requires only one disc read.

### Implications for decompilation

When promoting data tables, always identify which copy is the canonical
source. The randomizer's `cleanup()` method enforces consistency between
duplicated copies (e.g., `STATUS.EMI` base stats must match `START.EMI`).
Use the primary source for all promoted specs.

## File counts by family

| Family | Files | Unique contents | Notes |
| --- | --- | --- | --- |
| `ETC` | 22 | 22 | All unique |
| `WORLD00`–`WORLD04` | 200 | 200 | All unique (per-area) |
| `SCENARIO` | 25 | 25 | All unique |
| `BATTLE` | 9 | 9 | BATTLE.EMI ≡ BATTLE2.EMI |
| `BOSS` | 40 | 40 | Each has battle engine copy |
| `BENEMY` | 200 | 67 | 155 duplicates |
| `BMAGIC` | 144 | varies | Battle effects/magic |
| `BPLCHAR` | 121 | varies | Player battle sprites |
| `PLCHAR` | 38 | varies | Player overworld sprites |
| `BGM` | 81 | varies | Music tracks |
| **Total EMI** | **880** | | |
| System (non-EMI) | 8 | 8 | Boot, XA, EXE |
