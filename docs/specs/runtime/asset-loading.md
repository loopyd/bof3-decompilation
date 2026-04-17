# Asset Loading

This document summarizes how `SLUS_004.22` turns EMI entries into live runtime assets.

## High-Level Rule

The game does not load EMI entries in one uniform way.

The loader:

1. reads the entry metadata from the EMI TOC
2. stores `size`, `ram_ptr`, `first4`, and `type` in live state
3. dispatches through a type handler

That means asset behavior is defined jointly by:

- the EMI TOC entry
- the type handler in `SLUS_004.22`
- the subsystem-specific destination region or bank tables

## Entry-Type Dispatch Table

Current runtime handler map from `SLUS_004.22`:

| Type | Handler | Current meaning |
| ---: | --- | --- |
| `0` | `0x801625e4` | generic sector-to-RAM copy |
| `1` | `0x80162618` | queued RAM load with slot bookkeeping |
| `2` | `0x80162618` | same as type `1`, unresolved distinction |
| `3` | `0x80162698` | image or VRAM-oriented load path |
| `4` | `0x80162500` | special handler, meaning unresolved |
| `5` | `0x80162500` | same as type `4`, meaning unresolved |
| `6` | `0x80162790` | `VAB` header path |
| `7` | `0x80162898` | `VAB` body path |
| `8` | `0x801629f0` | auxiliary audio buffer path |
| `9` | `0x80162a6c` | auxiliary audio or sequence path |
| `10` | `0x80162a6c` | sequence path |

## Code And Generic Data

Type `0` is the broadest category.

What it includes:

- battle-side overlays at `0x801eec00`
- menu and common overlays at `0x801d0c00`
- world or area-side data and code at addresses like `0x80104000`
- compact tables and buffers at `0x80032000` through `0x800e4000`

Current conclusion:

- type `0` must not be treated as "plain data only"
- many code-bearing overlays are type `0`
- decompilation must classify type `0` entries by behavior, not by TOC type alone

## Graphics Loading

For the concrete graphics-side extraction algorithm, use
`docs/specs/runtime/emi-graphics-pipeline.md` alongside this document.

Type `3` is the strongest current graphics-specific path.

Observed behavior:

- the handler decodes fields from `ram_ptr`
- it does not treat `ram_ptr` as a normal CPU destination pointer
- it builds slot-local transfer words from those packed fields

Headless decomp of `0x80162698` now makes that more concrete:

```c
if (service_count == 0) {
  image_col = 0;
  image_base_x = (load_arg >> 24) & 0x3f;
  image_base_y = (load_arg >> 16) & 0x1f;
  image_span = (load_arg >> 8) & 0x3f;
}

slot_state[current_slot] = 3;
transfer_words[current_slot] =
    ((image_base_x + image_col) << 24) |
    (image_base_y << 16);
image_col++;
if (image_col >= image_span) {
  image_col = 0;
  image_base_y++;
}
service_count++;
```

Current implication:

- BOF3 is walking a packed descriptor and emitting chunk-local transfer words
- this is closer to a VRAM upload iterator than to a generic file decode path
- the exact low-word meaning of the descriptor is still open, but the high-byte stepping behavior is now locally proven

Current interpretation:

- type `3` is the raw image path used for PSX texture or VRAM uploads
- the `ram_ptr` value encodes image destination parameters
- the source-side payload format is often raw indexed pixels without a normal
  TIM header
- the corresponding palette or CLUT data often lives in separate small type-`0`
  blobs rather than inside the same wrapped image file

Current local confirmation from the title/menu slice:

- `FIRST/3.img` plus `FIRST/13.bin` palette row `0`
  - decodes into a readable title/menu glyph atlas once the raw 4bpp payload is
    rebuilt with the loader-style `0x800` chunk tiling rule
  - this matches the external texture-and-palette writeup materially well
- this is strong evidence that the current blocker on many title assets is
  palette-row or layout inference, not a hidden encryption layer

Important correction:

- the old generic "unstrip" permutation is not the current source of truth
- the stronger local rule is the recovered `SLUS` type-`3` loader itself:
  - each sector chunk is `0x800` bytes
  - each chunk is uploaded as one `32x32` block
  - chunks advance across the span encoded in `load_arg`
- offline extraction should follow that loader rule first and only keep older
  texture hypotheses as secondary checks

This matches local archive samples:

- `BATE.EMI`
  - image entries at `0x1c080200` and `0x1a080200`
- `AREA038.EMI`
  - image entries at `0x0e001000` and `0x0a081000`

Palette-like data appears separately:

- often as small type `0` entries
- commonly in the `0x80033xxx` to `0x8003axxx` range
- common sizes are `0x200` and `0x400`

Current interpretation:

- textures and palettes are not represented by one single EMI type
- raw image payloads use type `3`
- palette or CLUT payloads often travel as small generic RAM blobs

## Audio Loading

The audio path is the clearest example of type-specific behavior.

### Type `6`

Observed behavior:

- `ram_ptr` is treated as a logical audio bank id
- the handler remaps that id into a real runtime buffer
- it copies the payload there

Headless decomp of `0x80162790` now proves more of the setup logic:

- the bank id is looked up through a runtime table, not used directly as a raw pointer
- prior sequence state for that bank can be torn down before the new bank is staged
- the handler ultimately falls through into the generic sector-copy routine at `0x80162c14`

Current interpretation:

- type `6` loads a `VAB` header (`VH`)

### Type `7`

Observed behavior:

- references `SpuSetTransferMode`
- references `SsVabClose`
- references `SsVabOpenHeadSticky`
- follows the logical-bank path established by type `6`

Headless decomp of `0x80162898` now proves the order:

1. `SpuSetTransferMode(0)`
2. `SsVabClose(bank_handle)`
3. `SsVabOpenHeadSticky(...)`
4. queue the body-copy phase if the open succeeded

The handler does not behave like a plain memory copy. It is a BOF3-shaped wrapper around PsyQ VAB lifecycle plus the sector streaming path.

Current interpretation:

- type `7` loads or streams the `VAB` body (`VB`) for the bank previously prepared by type `6`

### Type `8`

Observed behavior:

- remaps the bank id into a second runtime buffer
- copies a small or medium payload there

Headless decomp of `0x801629f0` confirms that this path just remaps the current bank through a different runtime table before reusing `0x80162c14`.

Current interpretation:

- auxiliary audio payload
- exact format still unresolved
- current extracted samples are usually very small bank-local blobs rather than
  music-sized payloads

### Type `9`

Observed behavior:

- handler shares code with type `10`
- current local `SLUS` decomp places it on the same sequence-side copy path as
  type `10`

Current interpretation:

- audio-side auxiliary or sequence-adjacent payload
- exact meaning still unresolved
- no concrete shipped type-`9` sample is currently confirmed in the local EMI
  extraction mirror

### Type `10`

Observed behavior:

- samples begin with `SEQp`
- handler shares code with type `9`
- payload is copied into a bank-selected runtime buffer

Headless decomp of `0x80162a6c` confirms the shared buffer path:

- the current bank is remapped through a sequence-side table
- a per-bank flag word has bit `1` set before the stream copy
- the payload then reuses `0x80162c14`

Current interpretation:

- sequence data for the PsyQ music runtime

## Recovery Implication

Keep BOF3 routing, metadata, and timing explicit in the recovered model.

Current implication:

- recover what asset is requested, when, and with what parameters
- keep the type-specific handler split visible instead of flattening the loader
  into one generic asset narrative
- preserve original shipped asset identity when documenting loader behavior

## Unload Behavior

Current proven unload or replacement behavior:

- audio banks are actively replaced
  - old voices can be keyed off
  - old `SEP` handles can be closed
  - old `VAB` handles can be closed

Current strong inference for non-audio assets:

- overlays and many large content blobs are replaced by writing new content into the same family-local RAM region
- this is likely how game-mode overlays, battle overlays, and area modules are "unloaded"
