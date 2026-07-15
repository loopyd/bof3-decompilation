---
type: Reference
title: Format conversion map
description: Lossless interchange, desktop derivatives, validation, and provenance for BOF3 assets.
tags: [formats, conversion, provenance]
---

# Format conversion map

Preserve original bytes and provenance first. Desktop derivatives are lossless
artifacts under `out/`; they never replace an EMI entry, media sector stream,
or target binary.

## Canonical map

| Stored bytes | Runtime interpretation | Logical asset | Lossless interchange | Lossless desktop derivative | Validator | Unresolved boundary |
| --- | --- | --- | --- | --- | --- | --- |
| EMI archive | Sector-aligned container and TOC | Ordered entry set | Original `.EMI` plus extracted entry bytes and manifest | Catalog only | `bin/harness emi unpack`; `bin/harness discover` | Archive is not one executable or asset. |
| EMI type `0` | Direct RAM payload | Code, data, palette, or other bytes | Raw `.bin` | Domain-specific only | Catalog hash/load argument; promoted-target diff for code | Type does not identify content. |
| EMI types `1`, `2` | Queued RAM transfer | Code or data bytes with loader bookkeeping | Raw `.bin` | Domain-specific only | Catalog hash/load argument and reviewed loader state | Exact semantic difference between types remains bounded by loader behavior. |
| EMI type `3` | Packed VRAM upload descriptor plus raw chunks | Texture-page words | Raw `.img`/`.bin` plus descriptor | PNG after texture mode and CLUT are proven | `0x800` chunk geometry; [graphics invariants](../pseudocode.md#type-3-vram-upload-and-separate-palette-mapping) | Payload has no TIM header and does not name its palette. |
| Type-`0` palette bytes | CPU-side PSX color words later selected by draw state | 4bpp or 8bpp CLUT row/bank | Little-endian `u16` word dump plus RAM destination | PNG palette strip; RGBA JSON if useful | 16 colors=`0x20`; 256 colors=`0x200`; round-trip raw words | Never pair texture and palette by archive adjacency alone. |
| PSX indexed texture + verified TPage/CLUT | GPU texture sampling | Resolved image region | Texture words, palette words, TPage, CLUT, UV rectangle | PNG | `bin/harness assets list`; compare metadata and raw-source hashes | Transparency/STP interpretation and draw blend state remain separate. |
| Sprite draw data | Runtime primitive plus table-selected geometry | Resolved sprite instance or atlas | Source texture/CLUT plus table bytes and draw parameters | PNG atlas and metadata JSON | Reviewed table xrefs; exact C diff where promoted | There is no universal BOF3 sprite-file format. |
| Extracted STR/XA sectors | 2336-byte inner sectors | MDEC video and/or XA channels | Original stream plus reversible 2352-byte wrapper | Lossless H.264 (`-qp 0`) + FLAC MKV | Sector round-trip, codec/pixel-format probe, and exact endpoint comparison; [timing evidence](str-xa.md#capcom30-timing-evidence) | Runtime scheduling and channel selection need code evidence. |
| EMI types `6`, `7` | VAB header/body pair | PsyQ sound bank | Original `.vh` and `.vb` pair | Compatible VAB player/exporter output | Header/body identity, hashes, selector, and pair ownership | Pairing and playback parameters require caller evidence. |
| EMI type `10` | Sequence-side payload | PsyQ sequence | Original `.seq` | Compatible SEQ playback/conversion | Payload hash, selector, and parser acceptance | MIDI conversion is not treated as lossless. Type `9` semantics remain less specific. |
| Header-validated PSX TIM, if found | Self-describing PSX image | TIM image and optional CLUT | Original `.tim` | PNG | Validate TIM magic/header, dimensions, block sizes, and round-trip bytes | No BOF3 TIM payload is currently verified; `.tim` extension guessing is not evidence. |

## Provenance contract

Retain enough information to reconstruct and audit every conversion:

- disc/archive path, EMI slot, entry type, archive offset, payload size, and
  payload hash;
- type-dependent load argument and verified runtime load address;
- for indexed graphics: texture mode, VRAM placement/TPage, CLUT word or source
  RAM range, UV rectangle, and the table/callsite that relates them;
- for STR/XA: original sector order, wrapper policy, file/channel selectors,
  decoder/version, codec options, pixel format/range, frame count, sample rate,
  and measured packet endpoints;
- converter command/version and output hash. Generated manifests and previews
  belong under `out/`.

## Repository commands

```sh
just extract
just unpack
bin/harness discover
bin/harness assets list
just check
```

The canonical Rust EMI extractor is documented in
[`third_party/emi-ex-v2/README.md`](../../../third_party/emi-ex-v2/README.md).
Use the [verified pseudocode index](../pseudocode.md) for executable invariants
and the owning [EMI](emi.md), [graphics](graphics.md), and [STR/XA](str-xa.md)
specs for byte-level meaning.

For STR/XA, archival preservation means retaining the original extracted bytes
and hash. The wrapper is acceptable only with a byte-for-byte inner-sector
round trip. The desktop derivative is Matroska using `libx264 -qp 0` with FLAC,
without scaling or pixel-range changes.

For every desktop mux, compute `video_seconds = frames / fps`,
`audio_seconds = samples / rate`, and
`pad_samples = max(0, round((video_seconds - audio_seconds) * rate))`; then
require final durations to agree within one sample period. For the measured
`CAPCOM30` example, `231 / 30 = 7.700` seconds, decoded stereo XA is `7.626667`
seconds, and the formula yields 2772 zero samples per 37800 Hz channel. Record
that as derived desktop mux padding, never as missing source sectors, and keep
the trailing mono stream separate.
