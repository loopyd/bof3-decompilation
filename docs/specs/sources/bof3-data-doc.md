# Source Summary: BoF3-Data-Doc

Source repo:

- `third_party/references/BoF3-Data-Doc`
- upstream: `https://github.com/GlitchedDragon-dev/BoF3-Data-Doc`

## What It Is

An mdBook-style BOF3 data documentation project. At the time of ingestion, the published chapters with actual content are:

- `Introduction`
- `The "EMI" files format`
- `GameMode executable`
- `Texture and Palette`
- `STATUS.EMI`

The other chapters listed in the summary are placeholders.

## Useful Confirmed Claims

- EMI header:
  - count
  - unknown/version
  - `MATH_TBL`
  - 16-byte TOC entries
- EMI payloads are 0x800-aligned.
- EMI TOC fields are documented as:
  - size
  - RAM pointer/load argument
  - first 4 bytes
  - type
  - 2 trailing unknown bytes
- documented BOF3 type assignments:
  - `3` = image
  - `6` = `VH`
  - `7` = `VB`
  - `10` = `SEQ`
- BOF3 may duplicate identical content across multiple EMI archives for load-time locality.
- Many textures are raw type-`3` image data with no standard header.
- palettes are also raw/headerless and should be treated separately from textures.
- Game-mode executables are PSX overlays found near the start of certain EMI files.
- the recurring game-mode overlay load address described by the source is `0x801D0C00`.
- the source's `STATUS.EMI` example matches the local extracted shape closely:
  - entry `0` code at `0x801d0c00`
  - entries `1` and `2` as type-`3` menu images
  - entries `3` and `4` as small palette-like type-`0` blobs
  - entries `5` through `7` as the menu audio bank set
- the source's texture notes match one concrete local title/frontend result:
  - raw type-`3` image plus separate palette blob
  - 4bpp indexed payloads
  - strip/unstrip transform required before the atlas looks readable
 - the source also materially supports the current title-side failure mode:
   - raw type-`3` previews look split or duplicated when viewed naïvely
   - textures can be tile-split and code-composed
   - palette-row selection can be tile-specific
   - this means the final title/menu image is not always one blob -> one PNG
- `STATUS.EMI` is documented as a concrete mixed-content archive with:
  - one executable entry
  - texture entries
  - palette entries
  - audio entries
  - one small unknown entry
- `STATUS.EMI` is a concrete worked example of:
  - one overlay/code payload
  - texture payloads
  - palette payloads
  - audio payloads

## Current Local Use

- corroborating the EMI format in `formats/emi.md`
- corroborating the overlay concept in `runtime/emi-loader.md`
- supplying heuristics like the recurring `0x801D0C00` game-mode overlay load address
- supplying raw texture/palette handling hints for future asset reconstruction
- supplying one concrete mixed-content menu archive example through `STATUS.EMI`
- supplying a durable explanation for why title/menu image recovery must combine:
  - raw archive payloads
  - strip-aware texture handling
  - code-driven tile/layout composition
  - palette-row selection

## Limits

- many claims are explicitly marked incomplete by the upstream author
- file coverage is still narrow
- loader/runtime linkage in `SLUS_004.22` still needed local verification
- the recurring `0x801D0C00` game-mode address is still a source claim until each family is checked locally
- portrait palette-row assignments and menu-tile usage in `STATUS.EMI` should stay source-only until reproduced locally
- the source does not explain `FIRST.EMI`, `DEMO.EMI`, or `CAPCOM30.STR`
  directly, so the current title/logo slice still requires local game-code RE
- the source does not explain where title/menu tile-layout tables live; it only
  suggests that they are likely hardcoded by code use sites
