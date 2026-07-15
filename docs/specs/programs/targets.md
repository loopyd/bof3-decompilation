---
type: Runtime
title: Module map
description: Active and quarantined BOF3 executable and EMI code targets.
tags: [runtime, targets]
---

# Module map

## Executables

| Shipped identity | Target ID | Source |
| --- | --- | --- |
| `SLUS_004.22` | `exe/slus_004_22` | `src/exe/slus_004_22/` |
| `LOGO/LOGO.EXE` | `exe/logo` | `src/exe/logo/` |

`LOGO.EXE` is tracked as an independent executable at its PS-X header load
address, `0x801ce000`. Its reviewed `0x801ce758` runtime stub and
`0x801cedfc` main function are split and exact-matching; the remaining ranges
are still binary-backed and must be reviewed before promotion.

## Active EMI code targets

| Shipped identity | Target ID | Source |
| --- | --- | --- |
| `BIN/ETC/GAME.EMI#0` | `emi/etc/game/00` | `src/emi/etc/game/00/` |
| `BIN/ETC/GAME.EMI#1` | `emi/etc/game/01` | `src/emi/etc/game/01/` |
| `BIN/WORLD00/AREA008.EMI#13` | `emi/world00/area008/13` | `src/emi/world00/area008/13/` |

These are the active tracked EMI targets. Quarantined targets retain their
durable payload layouts and source ownership but are excluded from normal
generated builds until their boundaries are reviewed.

Use `out/catalog/` for candidates and `bin/harness promote` to create a tracked
target only after review.

## Quarantined targets

The target manifests are authoritative for the complete quarantined list. They
use `status = "quarantined"` and a required `quarantine_reason`; inspect them
with `bin/harness targets` before promoting or lifting work.

## Source ownership

- Standalone executables: `src/exe/<target>/`
- Confirmed EMI targets: `src/emi/<family>/<archive>/<slot>/`
- All reviewed source is owned by a canonical `src/exe/` or `src/emi/` target.

Do not duplicate a function across these trees.
