---
type: Archive ownership map
title: Data ownership and duplication
description: Canonical and duplicate BOF3 data locations for US v1.1.
tags: [archives, ownership, duplication]
---

# Data ownership and duplication

This page identifies owning archives. Byte offsets and record layouts remain in
the linked [data specs](../data/index.md).

## Static data

| Domain | Owning archive | Duplicate or companion |
| --- | --- | --- |
| equipment, abilities, shops, levels | `BIN/ETC/GAME.EMI` | none confirmed |
| base character stats | `BIN/ETC/START.EMI` | byte-identical copy in `STATUS.EMI` |
| master skills and stat modifiers | `BIN/ETC/SISYOU.EMI` | none confirmed |
| master names | `BIN/ETC/AFLDKWA.EMI` | copy in `FIRST.EMI` |
| fairy gifts and exploration items | `BIN/ETC/COMMU00.EMI` | prizes in `COMMU02.EMI` |
| dragon growth data | `BIN/ETC/STATUS.EMI` | none confirmed |
| monsters and formations | each `WORLD*/AREA*.EMI` | area-local records |
| chests, genes, chrysms | referenced area/scenario archive | pointer-map locations |
| Manillo trade data | selected area archives | repeated trade tables |

## Programs

| Domain | Archive | Notes |
| --- | --- | --- |
| main battle program | `BIN/BATTLE/BATTLE.EMI` | shared battle implementation |
| battle copy | `BIN/BATTLE/BATTLE2.EMI` | duplicate payload group |
| boss battle programs | `BIN/BOSS/BOSS*.EMI` | battle implementation plus boss-local data |
| game frontend | `BIN/ETC/GAME.EMI` | confirmed code entries `0` and `1` |
| status menu | `BIN/ETC/STATUS.EMI` | confirmed code entry `0` |
| scenario controller | `BIN/SCENARIO/SCENA16.EMI` | confirmed code entry `0` |

Target identity remains archive path, entry slot, payload hash, and load
address. Duplicate bytes do not merge source ownership.

## Enemy audio

`BIN/BENEMY/ENEMY*.EMI` contains enemy audio banks rather than executable
overlays. The reference mapping uses monster ID `N` to select
`ENEMY{N-1}.EMI`; confirm the caller and bounds before promoting that relation
into code.

## Boot and media

| File | Role |
| --- | --- |
| `SYSTEM.CNF` | boot configuration |
| `SLUS_004.22` | main executable and loader |
| `LOGO/LOGO.EXE` | secondary logo executable |
| `LOGO/CAPCOM30.STR` | logo video and XA audio |
| `BIN/BMAG_XA/MAGIC00.STR` | battle-magic XA bank |
| `BIN/SCE_XA/S_XA00.STR` | scenario XA bank |
| `BIN/SCE_XA/VOICE.STR` | voice XA bank |

Generated corpus counts, duplicate groups, and per-entry manifests belong in
`out/catalog/` rather than this page.
