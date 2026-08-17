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
| EMI archive | Sector-aligned container and TOC | Ordered entry set | Original `.EMI` plus extracted entry bytes and manifest | Catalog only | `bin/emi-ex`; extracted-entry hashes | Archive is not one executable or asset. |
| EMI type `0` | Direct RAM payload | Code, data, palette, or other bytes | Raw `.bin` | Domain-specific only | Catalog hash/load argument; promoted-target diff for code | Type does not identify content. |
| EMI types `1`, `2` | Queued RAM transfer | Code or data bytes with loader bookkeeping | Raw `.bin` | Domain-specific only | Catalog hash/load argument and reviewed loader state | Exact semantic difference between types remains bounded by loader behavior. |
| EMI type `3` | Packed VRAM upload descriptor plus raw chunks | Texture-page words | Raw `.img`/`.bin` plus descriptor | PNG after texture mode and CLUT are proven | `0x800` chunk geometry; [graphics invariants](../pseudocode.md#type-3-vram-upload-and-separate-palette-mapping) | Payload has no TIM header and does not name its palette. |
| Type-`0` palette bytes | CPU-side PSX color words later selected by draw state | 4bpp or 8bpp CLUT row/bank | Little-endian `u16` word dump plus RAM destination | PNG palette strip; RGBA JSON if useful | 16 colors=`0x20`; 256 colors=`0x200`; round-trip raw words | Never pair texture and palette by archive adjacency alone. |
| PSX indexed texture + verified TPage/CLUT | GPU texture sampling | Resolved image region | Texture words, palette words, TPage, CLUT, UV rectangle | PNG | Compare reviewed metadata and raw-source hashes | Transparency/STP interpretation and draw blend state remain separate. |
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
  decoder/options, pixel format/range, frame count, sample rate, and measured
  packet endpoints;
- the generated `conversion.json` records the source path, the full
  converter command line (`command`), fps, audio-padding arithmetic, ffprobe
  inspection of source and output, output timing, and `status`; the source
  hash is recorded as the nested `validation.source_sha256` field. It does
  not record the converter executable version or an output hash; compute those
  reproducibly with `ffmpeg -version | head -n 1` and
  `sha256sum <output>.mkv` at conversion time when an audit needs them.
  Generated manifests and previews belong under `out/`.

## Repository commands

```sh
bin/emi-ex --help
bin/str-media inspect INPUT
just check
```

The canonical Rust EMI extractor is documented in
[`tools/rust/emi-ex/README.md`](../../../tools/rust/emi-ex/README.md).
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
require final durations to agree within one sample period. Two distinct
timing attestations exist: `bin/str-media validate` checks source timing
against a coarse tolerance (two video frames or two XA sectors) and never
attests mux endpoint equality, while `bin/str-media convert` pads the primary
stream to the video endpoint and then requires the muxed output to agree
within one audio sample (`tools/python/harness/media/str_media.py`,
`validate_str` vs `convert_str`). A passing `validate` therefore does not
mean a desktop mux needs no padding. For the pinned `CAPCOM30.STR`
extraction (SHA-256
`0f9145e980e401ded21f4c315375bcb989f49b8b83582f46f4a2946dd33ff06d`),
`bin/str-media validate out/extracted/LOGO/CAPCOM30.STR --expected-fps 30`
passes because 203/30 = 6.7667 s
video against 254016/37800 = 6.72 s stereo XA audio (delta 0.0467 s) is
within its 0.1067 s tolerance, but the conversion formula requires
`round((203/30 - 254016/37800) * 37800) = 1764` padding samples per channel
for the primary stream; the converter computes exactly that
(`padding_samples_per_channel` in `conversion.json`). Treat any padding as
derived desktop output, never as missing source sectors; the pinned
extraction contains exactly one stereo XA stream, so there is no trailing
mono stream to keep separate. `bin/str-media convert
out/extracted/LOGO/CAPCOM30.STR --fps 30 -o out/str-media/CAPCOM30/CAPCOM30.mkv`
exits 0 even when its result
status is `fail`; require `status: pass` in `out/str-media/<stem>/conversion.json`
before treating a conversion as valid.

The `out/str-media/<stem>/conversion.json` receipts are disposable per-run
artifacts rather than durable facts. The reproducible contract is: run
`bin/str-media convert out/extracted/LOGO/CAPCOM30.STR --fps 30 -o <out>.mkv`,
then require `status: pass` in the generated `conversion.json` and record the
output SHA-256 with `sha256sum` at conversion time. `bin/str-media convert`
exits 0 even when its result status is `fail`, so CLI exit code alone is never
conversion acceptance; a passing source validation is likewise not conversion
acceptance.
