# Asset Families

This document describes the major `BIN/` archive families and their current runtime roles.

The goal is not to fully decode every file yet. The goal is to keep a stable map of what each family appears to contain and how it is likely used by the runtime.

For generated archive counts and representative overlay maps, use `processed/inventory/inventory.sqlite` as the canonical source. Any JSON or Markdown exports from the inventory helpers are ad hoc views, not durable repo state.

## Family Roles

### `BATTLE`

Role:

- shared battle runtime archives
- battle overlays, battle images, and battle audio

Observed pattern:

- frequent type `0` code and data
- type `3` image payloads
- type `6` and `7` audio banks
- occasional type `10` sequence data

Representative files:

- `BATL_OVR.EMI`
- `BATL_END.EMI`
- `BATTLE.EMI`

Stable code-bearing cluster:

- large overlays at `0x801d0c00` and `0x80096800`
- recurring helper blocks at `0x800357e0` and `0x80036e00`

### `BENEMY`

Role:

- enemy-specific audio bundles

Observed pattern:

- almost entirely types `6`, `7`, and `8`
- `ram_ptr` values are small bank ids rather than CPU pointers
- no type-`3` image payloads or local palette-bank candidates

Graphics-side implication:

- this is an audio-only family and should be excluded from sprite/CLUT extraction
- the recurring type `8` blob is a small shared auxiliary audio-side payload, not render metadata

Representative file:

- `ENEMY000.EMI`

### `BGM`

Role:

- music bundles

Observed pattern:

- type `6` `VAB` header
- type `7` `VAB` body
- type `10` `SEQ`
- no code or graphics in the sampled files

Representative file:

- `BGM000.EMI`

### `BMAGIC`

Role:

- battle magic and effect bundles

Observed pattern:

- heavy mix of code-bearing type `0` payloads
- type `3` images
- audio triplets or quartets

Interpretation:

- this family likely combines effect code, effect textures, and effect sound assets in the same archive

Stable code-bearing cluster:

- common code blocks at `0x801eec00`, `0x80036c00`, `0x800c3800`, and `0x80033c00`

### `BOSS`

Role:

- boss encounter bundles

Observed pattern:

- code, images, audio, and some sequence content
- similar to battle bundles, but organized per boss or per boss group

Stable code-bearing cluster:

- shared battle-style core around `0x801d0c00`, `0x80096800`, `0x8001a000`, `0x800f0800`
- boss-local delta often appears at `0x800c1800`

### `BPLCHAR`

Role:

- battle player-character content families

Observed pattern:

- repeated audio bank groups
- large type `0` or type `1` payloads at `0x8003b800`
- no type-`3` image payloads or local palette-bank candidates

Confirmed runtime link:

- `FUN_8016728c` maps family-indexed requests into this folder

Interpretation:

- battle-side player-character archives likely package animation, effect, and audio content together
- the recurring `0x4000` blob at `0x8003b800` in the common 11-entry bucket is zero-filled workspace, not meaningful executable/render data
- final rendering for this family likely depends on battle-side model or table decoders, not the menu-style type-`3` sprite path

### `PLCHAR`

Role:

- field or non-battle player-character content families

Observed pattern:

- audio bank groups
- one large type `1` payload at `0x8003b800`

Interpretation:

- closely related to `BPLCHAR`, but for a different gameplay family

### `ETC`

Role:

- mixed common-system, menu, and utility archives

Observed pattern:

- menu-like overlays at `0x801d0c00`
- type `3` images
- small palette-like blobs in `0x80033xxx`
- audio bundles

Representative file:

- `BATE.EMI`

Interpretation:

- this folder holds many UI or system-mode archives
- it contains at least two distinct graphics families:
  - a banked `4bpp` menu family around `0x801d0c00` with twin `256x256` pages and `0x200` CLUT rows
  - the separate `FIRST`/`DEMO`/`GAME` title/frontend bundle
- do not treat `ETC` as one uniform render family

Stable code-bearing cluster:

- major menu/system overlays at `0x801d0c00`
- smaller companion blocks often appear at `0x80033a00` and `0x80033c00`

### `SCENARIO`

Role:

- scenario or script-driven system content

Observed pattern:

- almost entirely type `0`
- very little audio or image content

Interpretation:

- likely code, tables, or script-support data rather than multimedia bundles

Stable code-bearing cluster:

- large primary scenario overlays around `0x801f6c00`
- smaller helper blocks appear in the `0x80035200` to `0x80035400` range

### `WORLD00` through `WORLD04`

Current role:

- world and area bundles

Observed pattern:

- many type `0` payloads
- consistent type `3` image payloads
- consistent audio groups
- large content blobs in areas like `0x80104000` and `0x801f2c00`

Representative file:

- `WORLD01/AREA038.EMI`

Interpretation:

- these archives likely combine:
  - area-local code and data
  - map textures
  - encounter or environment audio
  - scripts, tables, or object definitions

Stable code-bearing cluster:

- recurring world/area banks include `0x80010000`, `0x80032000`, `0x80033e00`, `0x80035800`, `0x800d3800`, `0x800e3800`, `0x800e4000`, `0x80104000`, and `0x801f2c00`
- some larger representatives add `0x80033a00`, `0x80033c00`, and `0x800f5000`

## Practical Use

For decompilation and porting, treat these families as subsystem boundaries:

- `BATTLE`, `BMAGIC`, `BOSS`
  - battle runtime
- `PLCHAR`, `BPLCHAR`
  - player-character content
- `WORLD*`
  - area and map runtime
- `BGM`, `BENEMY`
  - audio-side families
- `ETC`, `SCENARIO`
  - system and script-side families

This family map should be used when naming recovered overlays, grouping decomp targets, and deciding which asset decoders to build first.
