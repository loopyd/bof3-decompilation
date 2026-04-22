# Module Map

Use shipped module names and archive slots.

For code-bearing EMI archives, the stable identity is:

- archive name
- entry slot
- load address

## Core Executables

| Module | Current responsibility |
| --- | --- |
| `SLUS_004.22` | boot/root executable; slot map, EMI streaming, callback/thread seam, shared runtime services |
| `LOGO.EXE` | logo/movie branch and STR playback entry path |

## Documented Code-Bearing EMI Modules

| Module | Current responsibility | Content shape |
| --- | --- | --- |
| `GAME.EMI#0` | title-selection authoring, layout, hit-test, and selection-side support | code-bearing slot inside `GAME.EMI` |
| `GAME.EMI#1` | title/front controller and pre-demo branch | code-bearing slot inside `GAME.EMI` |
| `COMMU00.EMI#0` | shared menu/task substrate for frontend record/task flows | code-bearing slot inside `COMMU00.EMI` |
| `BATTLE.EMI#3` | representative battle core loaded at `0x801d0c00` | code-bearing slot inside `BATTLE.EMI` |
| `BATTLE.EMI#15` | battle selection corridor | code-bearing slot inside `BATTLE.EMI` |
| `STATUS.EMI#0` | status-menu game-mode overlay | code-bearing slot inside `STATUS.EMI` |
| `SCENA16.EMI#0` | first proven subordinate scenario module after the title/front path | code-bearing slot inside `SCENA16.EMI` |

These rows are the conservative locally documented subset: archives or slots with
runtime writeups that already defend executable responsibility, not just TOC
metadata patterns.

## Required Resource Packs On The Boot Path

| Module | Current responsibility | Content shape |
| --- | --- | --- |
| `FIRST.EMI` | common title/menu resource pack loaded before `GAME.EMI` | mixed archive; audio, images, and CPU-RAM tables/text, but not the title-state controller |
| `DEMO.EMI` | title/demo presentation pack requested by `GAME.EMI` state `0` | mixed archive; audio, images, and presentation-side control data, but not a trustworthy code module |
| `AFLDKWA.EMI` | duplicate resource payload also carried by `FIRST.EMI#11` | CPU-RAM text/table pack; do not treat it as an overlay root by default |

For full extraction/reverse/decompile, these packs should stay in scope as
archive/data inputs even though they are not part of the conservative
executable-overlay subset.

## Ghidra Import Catalog

For the broader practical Ghidra import set built from local EMI TOC metadata,
see [emi-ghidra-import-catalog.md](runtime/emi-ghidra-import-catalog.md).

## Mixed-Content Archive Pattern

Recurring EMI shape:

- one or more code-bearing slots (`type 0`)
- image payloads, often `type 3`
- small palette-like rows in the `0x80033xxx` range
- optional audio bank pairs such as `VH` / `VB`

Example stable reference:

- `STATUS.EMI`
  - one menu/game-mode overlay at `0x801d0c00`
  - image payloads
  - small palette-like blobs
  - one `VH` plus one `VB`

## Naming Rule For Recovered Source

Recovered source should mirror shipped module identity:

- `bof3/src/modules/game/00/` for `GAME.EMI#0`
- `bof3/src/modules/game/01/` for `GAME.EMI#1`
- `bof3/src/modules/commu00/00/` for `COMMU00.EMI#0`
- `bof3/src/modules/battle/15/` for `BATTLE.EMI#15`

Do not invent synthetic names like `GAME00` or `COMMON00`.
Use the shipped archive name plus slot.
