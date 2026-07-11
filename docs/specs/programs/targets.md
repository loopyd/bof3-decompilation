---
type: Runtime
title: Module map
description: Confirmed BOF3 executable and EMI code targets.
tags: [runtime, targets]
---

# Module map

## Executables

| Target | Role |
| --- | --- |
| `SLUS_004.22` | boot, loader, shared runtime |
| `LOGO.EXE` | boot-logo and STR path |

## Confirmed EMI code targets

| Target | Role |
| --- | --- |
| `GAME.EMI#0` | title-selection support |
| `GAME.EMI#1` | frontend controller |
| `COMMU00.EMI#0` | shared menu/task runtime |
| `BATTLE.EMI#3` | battle core |
| `BATTLE.EMI#15` | battle selection path |
| `STATUS.EMI#0` | status-menu overlay |
| `SCENA16.EMI#0` | scenario controller |

This is a conservative documented subset, not a complete generated inventory.
Use `out/catalog/` for candidates and `bin/rebof3 promote` to create a tracked
target only after review.

## Source ownership

- Standalone executables: `src/exe/<target>/`
- Confirmed EMI targets: `src/emi/<family>/<archive>/<slot>/`
- Existing targets awaiting identity normalization: `src/modules/`

Do not duplicate a function across these trees.
