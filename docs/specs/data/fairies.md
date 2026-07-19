---
type: Table
title: Fairy reward data
description: Fixed fairy-gift, exploration, and prize records from COMMU00 and COMMU02.
tags: [tables, fairy, rewards]
---

# Fairy reward data

> Fairy reward records are separate from the 9-byte fairy-village roster records.

The three fixed tables below use item index/type pairs but have distinct table
ownership and index namespaces. Their archive offsets are raw EMI archive
coordinates; they are not offsets into the extracted fairy roster pointer map.

## Fixed tables

| Table | Archive | Archive offset | Records × size | Payload range | Runtime status |
| --- | --- | ---: | ---: | --- | --- |
| `FairyGiftObject` | `BIN/ETC/COMMU00.EMI` | `0x848` | 20 × `0x04` | `0x848`–`0x897` | loaded with COMMU00 entry 0 |
| `FairyExploreObject` | `BIN/ETC/COMMU00.EMI` | `0x4218` | 48 × `0x02` | `0x4218`–`0x4277` | loaded with COMMU00 entry 0 |
| `FairyPrizeObject` | `BIN/ETC/COMMU02.EMI` | `0x2d900` | 48 × `0x02` | `0x2d900`–`0x2d95f` | archive-owned prize data; runtime load mapping unresolved |

The ranges and row hashes are verified by the generated lift-report artifacts
(`bin/decomp-status` → `out/matching/`, `out/reports/`). The COMMU00 entry-0
payload is loaded at
`0x801eec00`, so its payload-relative coordinates are `0x48` and `0x3a18`,
with runtime layout addresses `0x801eec48` and `0x801f2618`. COMMU02 is a
multi-entry archive whose prize table is retained in archive coordinates until
its owning loader path is confirmed.

## Record layouts

### Fairy gifts

```text
FairyGiftObject = le16 num_battles; u8 item_index; u8 item_type;
```

`num_battles` is the serialized progression value associated with the gift
row. The item fields use the shared item-index and item-category value spaces;
the row index remains fairy-gift-local.

### Exploration items and prizes

```text
FairyExploreObject = u8 item_index; u8 item_type;
FairyPrizeObject   = u8 item_index; u8 item_type;
```

The exploration and prize tables share a record shape but not an ownership
coordinate or row namespace. Do not merge them with Manillo records solely
because both carry an item index and type.

## Runtime consumer evidence

`COMMU00.EMI#0 @ 0x801f18f8` copies 80 bytes from runtime address
`0x801eec48` into a local work area, covering all 20 `FairyGiftObject` rows.
The separate `COMMU00.EMI#0 @ 0x801f1bc8` consumer indexes runtime
`0x801f2618` with `row × 2` and loads the two bytes independently, confirming
the `FairyExploreObject` item-index and item-type widths. Target-local
candidates now exist at
`src/emi/etc/commu00/00/func_801F18F8.c` (23.26%) and
`src/emi/etc/commu00/00/func_801F1BC8.c` (22.22%) under canonical `-O2`;
the gift-row widths and exploration progression behavior are recorded, but
exact function promotion remains pending.

## Evidence boundary

- Storage: `third_party/references/vast-violence/tables/tables_list_1.1.txt`
  and `struct_fairy_{gift,item}.txt`.
- Byte ranges and hashes: generated lift-report artifacts (`out/reports/`,
  `out/matching/`).
- Runtime COMMU00 payload base: `out/catalog/emi.json` entry
  `ETC/COMMU00#0`.
- Runtime consumer semantics and COMMU02 entry mapping remain unresolved.
