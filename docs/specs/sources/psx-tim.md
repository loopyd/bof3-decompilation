# PSX TIM Baseline

This page records standard PlayStation TIM behavior as a reference baseline for BOF3 image reverse engineering.

It does not claim that BOF3 stores all textures as normal TIM files. The point is to keep the standard PSX image model documented so BOF3-specific wrappers and raw payloads can be described against it.

## Sources

- Qhimm Modding Wiki: `PSX/TIM_file`
- Qhimm archive mirror: `PSX/TIM_format`
- corroborating local notes in `sources/psx-psyq-manuals.md`

## Why This Matters For BOF3

BOF3 does not appear to rely on one clean, universal TIM-on-disc format.

Current local evidence says:

- many EMI type-`3` payloads are raw image uploads, not normal TIM files
- palette-like data often travels separately as small type-`0` blobs
- the game still uses PSX-native VRAM, CLUT, and texture-page concepts

So TIM is still the right baseline reference for:

- CLUT layout
- indexed-vs-direct color modes
- VRAM-oriented image dimensions
- texture-page and palette assumptions inherited from PsyQ-era rendering

## Standard TIM Structure

The standard TIM model is:

1. file header
2. optional CLUT block
3. image block

Important baseline properties:

- TIM is a PSX-native little-endian image format
- the header encodes pixel format and whether a CLUT block exists
- indexed images depend on CLUT data
- image and CLUT blocks are shaped around PSX VRAM rectangles rather than modern abstract texture objects

## Useful Baseline Modes

TIM commonly appears in these PSX-native modes:

- 4bpp indexed
- 8bpp indexed
- 16bpp direct color
- 24bpp direct color

For BOF3, the most relevant baseline is indexed-image handling:

- raw pixel payload
- separate or preceding CLUT data
- VRAM upload semantics

That matches the current BOF3 working assumption that some EMI payload families are closer to raw `PXL` plus separate `CLT` than to one wrapped TIM file.

## BOF3-Specific Caution

Do not overfit TIM headers onto BOF3 image payloads.

Current BOF3 interpretation:

- standard TIM explains the hardware and PsyQ conventions
- BOF3 frequently stores the equivalent content without keeping the full TIM wrapper
- type-`3` EMI entries should currently be treated as raw image uploads whose destination is encoded in the EMI load argument

## How To Use This Reference

Use TIM as the baseline when:

- decoding title/frontend image uploads
- reasoning about raw `PXL` data and separate CLUT blobs
- deciding how to adapt PSX-native image uploads into a portable texture backend

Do not use TIM as proof that a BOF3 blob is a complete standalone TIM file unless the normal TIM header is actually present in the payload.
