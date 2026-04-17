# Source Summary: jPSXdec

Source repo:

- `third_party/references/jpsxdec`
- upstream: `https://github.com/m35/jpsxdec`

Primary ingested documents:

- `third_party/references/jpsxdec/jpsxdec/PlayStation1_STR_format.txt`
- `third_party/references/jpsxdec/jpsxdec/jPSXdec-design.md`
- `third_party/references/jpsxdec/README.md`

## What It Is

`jPSXdec` is a PlayStation 1 media extractor/converter with strong coverage for:

- STR/MDEC video
- XA and SPU ADPCM audio
- TIM images
- sector-based disc indexing and extraction

For this repo, it is best treated as an external reference and offline validator.

## Useful Confirmed Claims

- `PlayStation1_STR_format.txt` is a mature STR-format reference and is MIT
  licensed as a document.
- The program itself is separate from that document and the upstream README says
  `jPSXdec` is free for non-commercial use.
- PlayStation movie data is fundamentally sector-driven:
  - sectors are `2352` bytes physically
  - video commonly lives in Mode 2 Form 1 sectors
  - XA audio commonly lives in Mode 2 Form 2 sectors
  - video and audio sectors may be interleaved
- STR decoding is not only "read one file as a blob":
  - sector headers matter
  - demultiplexing frame data matters
  - MDEC bitstream interpretation matters
  - XA/SPU audio handling is a separate concern
- The design doc confirms a strong sector-indexing architecture:
  - sector readers
  - sector identification/claiming
  - disc-item indexing
  - media-specific decoders layered on top
- The design doc also confirms `jPSXdec` is extraction/indexing oriented, not an
  emulator.

## Why It Matters Here

- It is a good technical source for the `LOGO/CAPCOM30.STR` branch.
- It is a good reference for building a repo-local movie adapter for the
  preserved `LOGO.EXE -> CAPCOM30.STR` seam.
- Its sector-first design is a useful model for native tooling that needs to
  reason about PlayStation media from original disc-layout facts instead of only
  from host filesystem copies.
- It is a strong offline validator when comparing:
  - `FFmpeg`-based native playback
  - repo-local extraction
  - expected STR/XA framing behavior

## Current Local Use

- external reference for `CAPCOM30.STR` and future STR/XA work
- source material for documenting a BOF3 movie adapter seam
- external validator for color, frame, and multiplexing expectations

## Limits

- It is not BOF3-specific.
- It does not explain EMI containers, BOF3 slot tables, or BOF3 overlay code.
- Its runtime/codebase is not a good direct dependency for the native port path
  because the upstream program is non-commercial.
- Prefer repo-local adapter code plus a conventional backend such as `FFmpeg`
  for native runtime playback.
