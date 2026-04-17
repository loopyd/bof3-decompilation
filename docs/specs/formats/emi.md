# EMI Format

This document describes the BOF3 EMI archive container itself. The game-specific loader behavior lives in `../runtime/emi-loader.md`.

## Status

- Confidence: medium
- Basis:
  - local header parsing from extracted `BIN/**/*.EMI`
  - `SLUS_004.22` loader disassembly
  - external corroboration from `third_party/references/BoF3-Data-Doc`

## Container Layout

EMI is a sector-aligned archive format used throughout `build/extracted/BIN/`.

Header layout:

| Offset | Size | Type | Meaning |
| --- | ---: | --- | --- |
| `0x00` | 4 | `u32` | entry count |
| `0x04` | 4 | `u32` | version or unknown |
| `0x08` | 8 | `char[8]` | magic `MATH_TBL` |

TOC entry layout:

| Offset | Size | Type | Meaning |
| --- | ---: | --- | --- |
| `0x00` | 4 | `u32` | payload size |
| `0x04` | 4 | `u32` | `ram_ptr` field |
| `0x08` | 4 | `u32` | first four payload bytes, cached in the TOC |
| `0x0c` | 2 | `u16` | type id |
| `0x0e` | 2 | `u16` | unknown trailing TOC field (`toc_unk`) |

Payload layout:

- the first payload begins at `0x800`
- each payload is aligned to `0x800`
- next payload offset is:

```c
next_offset = current_offset + (((size + 0x7ff) >> 11) * 0x800);
```

This alignment rule is confirmed by both the extracted archives and the code in `SLUS_004.22`.

## Meaning Of `ram_ptr`

The `ram_ptr` field is not one universal pointer type. Its meaning depends on the TOC `type`.

Observed uses:

- real CPU RAM destination
  - examples: `0x801d0c00`, `0x801eec00`, `0x8003b800`, `0x80104000`
- packed image or VRAM descriptor
  - examples: `0x1c080200`, `0x1a080200`, `0x0e001000`
- logical audio bank id
  - examples: `0x00000001` through `0x00000006`

For porting and tooling, EMI entries should therefore be modeled as:

```c
struct EmiEntry {
  u32 size;
  u32 ram_ptr;
  u32 first4;
  u16 type;
  u16 unk;
};
```

plus a runtime-side interpretation layer keyed by `type`.

## Known Type Usage

Known or strongly supported type meanings:

| Type | Current meaning | Confidence |
| ---: | --- | --- |
| `0` | generic binary payload, often code or CPU-RAM data | medium |
| `1` | large CPU-RAM content blob with extra post-load handling | low |
| `3` | raw image payload | high |
| `6` | PSX `VAB` header (`VH`) | high |
| `7` | PSX `VAB` body (`VB`) | high |
| `8` | small audio-side metadata or auxiliary buffer payload | low |
| `10` | PSX sequence (`SEQ`) | high |

Notes:

- type `0` includes both executable MIPS overlays and non-code data blobs
- type `3` payloads are raw image data without standard TIM headers
- palette-like data often appears as type `0` with small sizes such as `0x200` or `0x400`
- handlers also exist in `SLUS_004.22` for types `4`, `5`, and `9`, but those meanings are not yet proven
- current local EMI manifests contain many type `6`, `7`, `8`, and `10`
  payloads, but no concrete shipped type-`9` sample is currently confirmed
- current local counts across `processed/emi_raw/` are:
  - type `6`: `1020`
  - type `7`: `1020`
  - type `8`: `904`
  - type `10`: `119`

## Extraction Correctness

Current extraction is consistent with the shipped game:

- EMI headers parse cleanly and validate against `MATH_TBL`
- payload boundaries computed from the TOC match the extracted files
- `SLUS_004.22` computes payload sector offsets using the same `0x800` alignment rule
- the EXE's slot table maps to disc LBAs that resolve to the extracted EMI archives

Current conclusion:

- the EMI splitting/extraction is correct enough to use as the base for reverse engineering
- the remaining uncertainty is not archive extraction
- the remaining uncertainty is semantic classification of each payload

## Important Open Points

- full meaning of type `1`, `8`, and `9`
- full distinction between raw textures, CLUTs, and other graphics-side blobs
- any relocation or init convention for code-bearing payloads
- exact 3D model format stored in character, world, or battle archives
