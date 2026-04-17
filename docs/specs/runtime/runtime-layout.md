# Runtime Layout

This document is the top-level runtime model for the US v1.1 game build.

## Boot Chain

- `SYSTEM.CNF` boots `SLUS_004.22`
- `LOGO/LOGO.EXE` is a secondary PS-X EXE
- the main game content lives under `BIN/`
- most of the runtime content under `BIN/` is stored in EMI archives

## Runtime Roles

`SLUS_004.22` is the main runtime and loader. It is responsible for:

- CD access
- slot-to-file lookup
- EMI header parsing
- payload streaming
- type-specific asset handling
- handing work off to subsystem-local RAM regions

The important consequence is:

- the game is not just one EXE plus passive files
- the game is one EXE plus a large overlay and asset-loading system

The slot table is broader than EMI only:

- most slot entries resolve to EMI archives
- at least a few slot entries resolve directly to XA/STR assets
  - `BIN/BMAG_XA/MAGIC00.STR`
  - `BIN/SCE_XA/S_XA00.STR`
  - `BIN/SCE_XA/VOICE.STR`

Important reachability limit:

- presence in `DAT_80182444` proves that a shipped file has a top-level slot id and disc LBA
- presence in the slot table does not prove that a file is actually reached during normal gameplay
- some files may still be boot-only, debug-only, test-only, or otherwise conditionally loaded

Current examples:

- `LOGO/CAPCOM30.STR` is now resolved in the slot map as a top-level file-table entry and is externally reported as the animated Capcom boot-logo asset
- `BIN/WORLD04/AREA197.EMI`, `AREA198.EMI`, and `AREA199.EMI` are also resolved in the slot map and are contentful archives with world-area data indexed by `third_party/references/vast_violence`
- the unresolved part is not file presence anymore; it is the concrete runtime path that selects those slots

## Shared Working Regions

Recurring target regions observed in sampled archives:

| Region | Current interpretation |
| --- | --- |
| `0x801d0c00` | reused shared overlay load region; subsystem assignment must be proven per loaded program |
| `0x801eec00` | battle-side overlay region |
| `0x8003b800` | character, effect, or shared content work region |
| `0x80104000` | large world or area-side code/data region |
| `0x801f2c00` | compact world or area-side state/table region |
| `0x80033xxx` to `0x8003axxx` | small graphics-side or palette-side CPU buffers |

These are currently best understood as reused family-local working destinations
rather than globally unique assets or permanently fixed subsystem homes.

For example, `0x801d0c00` is proven in some programs as a frontend or game-mode
load region, but a representative `BIN/BATTLE/BATTLE` entry now also proves a
battle control implementation at the same base.
@source: 0x801d11d8 FUN_801d11d8

## Core Runtime Flow

1. A caller chooses a logical slot id, or computes one from a higher-level family/index request.
2. `SLUS_004.22` resolves that slot to a disc LBA from `DAT_80182444`.
3. The EXE reads the EMI header sector and validates `MATH_TBL`.
4. It computes aligned payload sector offsets from the TOC.
5. It selects a payload entry.
6. It streams the payload into RAM or into a type-specific subsystem path.
7. For code-bearing payloads, execution is later handed into shared overlay regions.

See:

- `emi-loader.md`
- `asset-loading.md`

## Why This Is Hard To Reverse

The project is not reversing one executable with passive assets. It is
reversing a layered runtime contract:

```text
[PSX disc image]
      |
      v
[SYSTEM.CNF]
      |
      v
[SLUS_004.22]
main EXE, but also loader and dispatcher
      |
      +-----------------------------------------------+
      |                                               |
      v                                               v
[slot/family selection]                         [callbacks / state]
index,family -> slot id                         EXE and overlay code
      |
      v
[DAT_80182444 slot -> LBA]
      |
      v
[disc file at that LBA]
      |
      +---------------------+---------------------+
      |                     |                     |
      v                     v                     v
 [EMI archive]         [STR/XA media]       [boot-side EXEs]
      |
      v
[EMI header + TOC]
MATH_TBL, sizes, ram_ptr, type
      |
      v
[dispatch by TOC type]
      |
      +-------------------+-------------------+-------------------+
      |                   |                   |                   |
      v                   v                   v                   v
[type 0/1]          [type 3]             [type 6/7/8/10]   [other types]
code or data        raw VRAM upload      PsyQ audio path    partial / open
mixed together      not final images     VH/VB/SEQ rules    semantics
      |                   |                   |
      v                   v                   v
[shared RAM regions] [PSX VRAM state]   [SPU / libsnd state]
0x801d0c00           tpage/clut/u/v      bank ids, staged loads
0x801eec00
0x80104000
...
      |
      v
[overlay-style payloads]
same addresses reused by many families
      |
      v
[runtime entry is not always obvious]
ram_ptr may be data, descriptor, or bank id
first callable code may be ram_ptr + 4 or after a local table
      |
      v
[running overlay requests more EMI]
FIRST -> GAME -> DEMO -> SCENAxx -> ...
```

What makes this difficult in practice:

- the game often selects a family or logical slot first, not a direct file
- EMI is a typed runtime container, not one asset format with one meaning
- `ram_ptr` is overloaded across CPU RAM, VRAM-oriented descriptors, and audio bank ids
- type `0` can be code or non-code data, so payload classification is not trivial
- many code-bearing payloads behave like overlays and reuse the same load regions
- the first callable instruction of a loaded payload may not be at `ram_ptr`
- graphics correctness depends on reconstructing PSX VRAM state, not just extracting bytes
- audio correctness depends on reproducing PsyQ `VH` / `VB` / `SEQ` load behavior
- loader phase tables and overlay dispatch tables may point at internal labels, not clean function starts

The practical payoff works in the opposite direction: each proven transition
shrinks the unknown part of the system and makes the project easier to recover
and maintain.

```text
few proven transitions
      |
      v
[large hidden runtime contract]
- guessed module assignment
- guessed entrypoints
- guessed asset meaning
- fragile tooling and docs
      |
      v
prove one more handoff:
SLUS -> FIRST
SLUS -> GAME
GAME -> DEMO
GAME -> SCENA16
...
      |
      v
[more explicit flow graph]
- known caller
- known callee
- known load slot
- known entry rule
- known asset dependencies
      |
      v
[clearer module assignment]
EXE owns loading
overlay owns local state
asset pack owns content payloads
      |
      v
[easier maintenance and porting]
- less guesswork
- better inventories
- safer naming
- narrower interfaces
- easier replacement with native code
```

In other words: the work gets easier as the runtime graph becomes explicit.
Each recovered transition converts "some bytes loaded somewhere" into a stable
contract between modules.

## EMI Entry Ordering

EMI entry numbering is not the same thing as runtime execution order.

- entry indices like `0.bin`, `1.bin`, or `7.bin` identify TOC payload slots
- actual load and execution order is chosen by caller code, callback installation,
  and overlay-local state or phase tables
- for code-bearing entries, the first address that runs may be a dispatcher or
  callback target inside a larger subsystem, not "entry 0 then entry 1"

Current proven front/title example:

- `SLUS` loads `FIRST.EMI`
- `SLUS` callback `0x8014ec64` then loads `GAME.EMI`
- `SLUS` enters `GAME.EMI` entry `1` at `0x801d0c04`
- `GAME.EMI` entry `1` requests `DEMO.EMI` and owns the title transition
- `GAME.EMI` entry `1` later installs callback `0x80197068`
- `0x80197068` lives in `GAME.EMI` entry `0`, which then takes over the
  front/menu backing state machine

This is proven for the current `SLUS -> FIRST -> GAME -> DEMO` slice only.
Do not generalize it to every EMI family until the same handoff pattern is
proven there too.

## Higher-Level File Selection

At least one higher-level selector is already proven:

- `FUN_8016728c` maps `(index, family)` into one of four slot-id ranges and then calls the EMI loader.

This matters because gameplay systems are already choosing family-local content ids, not raw filenames. The slot formulas and concrete examples live in `emi-loader.md`.

## Recovery Implication

The reverse-engineering target is not "decompile one EXE." It is "recover the
EXE plus the overlay loader contract plus the asset-loading contract."

That means:

- keep loader, graphics, audio, and overlay behavior separated by their
  original runtime boundaries
- prove one handoff at a time instead of flattening the system into a guessed
  engine layout
- document the exact asset and region assignment that each proven transition
  depends on

## Open Points

- exact code-entry dispatch after overlays finish loading
- relocation rules, if any
- exact model and animation formats
- full text LUT and control-code decoding
