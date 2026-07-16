---
type: Runtime
title: Module map
description: Tracked BOF3 executable and EMI code targets.
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

## EMI code targets

| Shipped identity | Target ID | Source |
| --- | --- | --- |
| `BIN/ETC/GAME.EMI#0` | `emi/etc/game/00` | `src/emi/etc/game/00/` |
| `BIN/ETC/GAME.EMI#1` | `emi/etc/game/01` | `src/emi/etc/game/01/` |
| `BIN/WORLD00/AREA008.EMI#13` | `emi/world00/area008/13` | `src/emi/world00/area008/13/` |

Every manifest denotes a known target. Splat segments own its reviewed
lifecycle. Use extracted evidence and the
target map before adding source; `bin/promote TARGET@0xADDRESS candidate.c`
validates a candidate but does not create source, layouts, or maps.

## Source ownership

- Standalone executables: `src/exe/<target>/`
- Confirmed EMI targets: `src/emi/<family>/<archive>/<slot>/`
- All reviewed source is owned by a canonical `src/exe/` or `src/emi/` target.

Do not duplicate a function across these trees.
