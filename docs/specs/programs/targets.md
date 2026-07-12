---
type: Runtime
title: Module map
description: Confirmed BOF3 executable and EMI code targets.
tags: [runtime, targets]
---

# Module map

## Executables

| Shipped identity | Target ID | Source |
| --- | --- | --- |
| `SLUS_004.22` | `exe/slus_004_22` | `src/exe/slus_004_22/` |
| `LOGO/LOGO.EXE` | `exe/logo` | `src/exe/logo/` |

## Confirmed EMI code targets

| Shipped identity | Target ID | Source |
| --- | --- | --- |
| `BIN/ETC/GAME.EMI#0` | `emi/etc/game/00` | `src/emi/etc/game/00/` |
| `BIN/ETC/GAME.EMI#1` | `emi/etc/game/01` | `src/emi/etc/game/01/` |
| `BIN/ETC/COMMU00.EMI#0` | `emi/etc/commu00/00` | `src/emi/etc/commu00/00/` |
| `BIN/BATTLE/BATTLE.EMI#3` | `emi/battle/battle/03` | `src/emi/battle/battle/03/` |
| `BIN/BATTLE/BATTLE.EMI#15` | `emi/battle/battle/15` | `src/emi/battle/battle/15/` |
| `BIN/ETC/SCENA16.EMI#0` | `emi/etc/scena16/00` | `src/emi/etc/scena16/00/` |

This is a conservative documented subset, not a complete generated inventory.
Use `out/catalog/` for candidates and `bin/harness promote` to create a tracked
target only after review.

## Source ownership

- Standalone executables: `src/exe/<target>/`
- Confirmed EMI targets: `src/emi/<family>/<archive>/<slot>/`
- All reviewed source is owned by a canonical `src/exe/` or `src/emi/` target.

Do not duplicate a function across these trees.
