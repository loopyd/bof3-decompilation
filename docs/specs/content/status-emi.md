---
type: Archive case study
title: STATUS.EMI
description: A mixed-content EMI archive retained as a menu-mode reference point.
tags: [content, emi, status, menu]
---

# STATUS.EMI

This document keeps one concrete mixed-content archive documented as a stable
reference point for menu-mode reverse engineering.

Archive:

- `out/extracted/BIN/ETC/STATUS`

## Local Manifest

Local extracted manifest:

| Entry | Type | Load arg | Size | Current meaning |
| ---: | ---: | --- | ---: | --- |
| `0` | `0` | `0x801d0c00` | `117986` | status-menu game-mode overlay |
| `1` | `3` | `0x1c080200` | `32768` | menu/image payload |
| `2` | `3` | `0x1a080200` | `32768` | menu/background payload |
| `3` | `0` | `0x80033a00` | `512` | small palette-like blob |
| `4` | `0` | `0x80033c00` | `512` | small palette-like blob |
| `5` | `6` | `0x1` | `3616` | `VH` / VAB header |
| `6` | `8` | `0x1` | `44` | small auxiliary audio/control blob |
| `7` | `7` | `0x1` | `64560` | `VB` / VAB body |

## Why It Matters

`STATUS.EMI` is the cleanest current local example of the BOF3 mixed-content
pattern that the external docs describe:

- one menu/game-mode overlay at `0x801d0c00`
- type-`3` image payloads for menu art
- small type-`0` palette-like blobs in the `0x80033xxx` range
- one `VH/VB` audio bank pair plus the tiny auxiliary blob

That makes it a good sanity-check archive for:

- the recurring `0x801d0c00` menu/game-mode overlay convention
- the type-`3` image path
- the split between shared overlay/control bins and archive-local palette bins
- menu-side audio-bank loading

## Source Corroboration

External source:

- `third_party/references/BoF3-Data-Doc/src/DataContent/1_STATUS.EMI.md`

The external document and the local manifest agree on the broad layout:

- status-menu executable first
- two image payloads
- two palette-like payloads
- one small unknown/control payload
- one `VH` plus one `VB`

## Local Proof

The local repo now has one source-backed rendering path for `STATUS` portraits
from the original `EMI` bytes.

Proven local facts:

- the portrait/menu image page is `entry 1` at `0x1c080200`
- the portrait crop and CLUT selector table is `DAT_801ec96c` in the
  `0x801d0c00` overlay
- each portrait record is `4` bytes:
  - `+0x00 u`
  - `+0x01 v`
  - `+0x02 clut_x_byte`
  - `+0x03 clut_y_selector`
- `FUN_801dc4b8` fixes portrait size to `40x48` and packs the final CLUT word
  as:
  - `(clut_x_byte >> 4) | ((clut_y_selector + 0x1e0) << 6)`
- `FUN_8014e22c` uploads a full `256x32` CLUT bank from CPU memory to VRAM at
  `x = 0, y = 0x01e0`
- `FUN_8014e284` copies the CPU-side palette bank from `0x80033800` to
  `0x80037800` before that VRAM upload
- the two local `512`-byte palette blobs therefore occupy rows inside that
  `256x32` bank, not one standalone `16x16` CLUT block:
  - `0x80033a00 - 0x80033800 = 0x0200`, so `entry 3` is row `1` -> `y = 0x01e1`
  - `0x80033c00 - 0x80033800 = 0x0400`, so `entry 4` is row `2` -> `y = 0x01e2`
- within each `512`-byte row, the sixteen `16`-color 4bpp CLUT slots are
  selected by `clut_x_byte = 0x00 .. 0xf0`
- local `STATUS` mapping is now strong enough to treat as proven:
  - `entry 3` / load `0x80033a00` provides CLUT row `y = 0x01e1`
  - `entry 4` / load `0x80033c00` provides CLUT row `y = 0x01e2`
- the shared overlay still references row `0` at `y = 0x01e0` for text/common
  UI CLUTs, so that row is not archive-local to `STATUS`
- the broader menu metadata is not owned only by `STATUS/0.bin`
  - several local menu helpers in the `0x801d0c00` overlay call shared draw
    helpers in `GAME.EMI` entry `0`
  - proven local callers include:
    - `FUN_801df410`
    - `FUN_801df6ec`
    - `FUN_801dfbf4`
    - `FUN_801e0354`
    - `FUN_801e0a70`
    - `FUN_801e11a8`
- the shared helper `FUN_801af2a0` in `GAME.EMI` entry `0` takes a symbolic
  sprite id plus flags, not a local `u/v/w/h` record
  - `FUN_801af270` chooses between two shared `4`-byte rect tables:
    - `DAT_801cce84`
    - `DAT_801ccf7c`
  - `flags & 2` chooses the CLUT source:
    - `GetClut(0xb0, 0x01e1)`
    - `GetClut(0x80, 0x01e2)`
  - the local `STATUS` caller still chooses the `tpage` separately through the
    preceding draw-mode path, so the final metadata is split across callers and
    the shared helper
- this means complete `STATUS` extraction requires merging:
  - archive-local VRAM uploads from `STATUS.EMI`
  - archive-local palette rows from `STATUS.EMI`
  - local `STATUS` caller metadata such as screen position and `tpage`
  - shared rect/CLUT helper metadata from `GAME.EMI` entry `0`
- `FUN_801df21c` proves one local repeated tile-strip path, but it is not the
  whole status-page composition and should not be treated as the final
  bottom-right menu mapping by itself

Local validation artifact:

- `tmp/status_validate/portraits_sheet.png` now renders a coherent portrait
  sheet from the original `STATUS.EMI`
- `tmp/status_validate/portraits_atlas.png` now renders the validated portrait
  atlas from the original `STATUS.EMI`
- `tmp/status_validate/manifest.json` now records:
  - local portrait metadata
  - local layout tables from `STATUS/0.bin`
  - shared rect-table metadata from `GAME/0.bin`
  - the currently proven STATUS helper inventory

Explicit current status:

- `entry 1` at `0x1c080200` is locally validated for portraits
- `entry 2` at `0x1a080200` is still unresolved
- the unresolved part is now narrowed down:
  - the remaining menu/background metadata is split across multiple code paths
    and is not recoverable from `STATUS/0.bin` alone
  - the current frontier is recovering the exact local caller -> shared helper
    -> shared rect table linkage for each status subpanel
- tooling should emit:
  - raw indices for both image pages
  - the local palette rows from `entry 3` and `entry 4`
  - validated final-colored output only for `entry 1`, kept close to the original page/atlas layout
  - unresolved candidate rows for `entry 2` so visual verification can continue until the overlay-side sprite/tile records are recovered

## Current Limits

- the local repo has not yet promoted the `STATUS` overlay into recovered C
- the portrait path is locally proven, but broader menu-tile composition still
  needs the shared helper path to be recovered completely
- `entry 2` background/menu tiles do not yet have one fully merged metadata set
  covering:
  - local caller `tpage`
  - local screen placement
  - shared rect-table ids
  - final CLUT branch per helper path
- `START` shares the same overlay family and image pages, but its full local
  CLUT state is still incomplete because it only carries one local `512`-byte
  palette blob
- the external note about palette helper `FUN_8014E3D4` remains an upstream
  lead, not a local conclusion
