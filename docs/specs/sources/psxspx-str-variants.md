---
type: External source summary
title: PSXSPX STR variants
description: STR header variants from the PSXSPX streaming-format reference.
tags: [source, psx, str, external]
---

# PSXSPX STR Variants

Source: https://problemkaputt.de/psxspx-cdrom-file-video-streaming-str-variants.htm
Retrieved: 2026-03-27

## Standard STR Header

| Offset | Size | Description |
|--------|------|-------------|
| 0x00   | 2    | STR ID (0x0160 = standard) |
| 0x02   | 2    | STR Type (0x8001 = standard MDEC) |
| 0x04   | 2    | Sector number within current frame |
| 0x06   | 2    | Number of sectors in this frame |
| 0x08   | 4    | Frame number (1 = first) |
| 0x0C   | 4    | Frame size in bytes |
| 0x10   | 2    | Bitmap width |
| 0x12   | 2    | Bitmap height |
| 0x14   | 2    | MDEC code count (div 64, times 2) |
| 0x16   | 2    | BS ID (0x3800 = standard) |
| 0x18   | 2    | BS quantization scale |
| 0x1A   | 2    | BS version (2 or 3) |
| 0x1C   | 4    | Usually zero (or custom) |
| 0x20   | ...  | Frame data (BS format) |

## STR ID Values

| ID | Description |
|----|-------------|
| 0x0160 | Standard STR header |
| 0x01 | Ace Combat 3 Electrosphere |
| "SMJ",0x01 | Final Fantasy 8, Video |
| "SMN",0x01 | Final Fantasy 8, Audio/left |
| "SMR",0x01 | Final Fantasy 8, Audio/right |
| 0x0000000x | Judge Dredd |
| 0xDDCCBBAA | Crusader: No Remorse, older EA |
| 0x08895574 | Chunk header in 1st sector only, Best Sports (demo) |
| "VLC0" | Chunk header in 1st sector only, newer EA |
| "VMNK" | Chunk header in 1st sector only, Policenauts |
| 0x01,"XSP" | Sentient header in 1st sector only |

## STR Type Values

| Type | Description |
|------|-------------|
| 0x0000-0x7FFF | User Defined |
| 0x8000-0xFFFF | System |
| 0x8001 | Standard MDEC (most common) |
| 0x0000 | Polygon Video / MDEC (Alice in Cyberland) |
| 0x0001 | MDEC (Ridge Racer Type 4 PAL) / Subtitles (MGS) |
| 0x0002 | Software-rendered video / MDEC with IntroTableSet |
| 0x0003 | MDEC with EndingTableSet |
| 0x0004 | MDEC (Final Fantasy 9, MODE2/FORM2) |
| 0x5349 | MDEC (Gran Turismo 1/2, "IS") |
| 0x8101 | MDEC + bit8=FlagDisc2 (Chrono Cross Disc 2) |

## Capcom STR Format

Used by:
- Resident Evil 2 (ZMOVIE\*.STR, PL0\ZMOVIE\*.STR)
- Super Puzzle Fighter II Turbo (STR/CAPCOM15.STR)

Custom field at offset 0x1C:
```
0x1C  4    Sector number of 1st sector of current frame  ;<-- instead of zero
```

## Breath of Fire III STR Analysis

CAPCOM30.STR header (first sector):
- File size: 2,698,080 bytes (1155 sectors at 2336 bytes/sector)
- STR ID: 0x0160 (standard)
- STR Type: 0x8001 (standard MDEC)
- Dimensions: 320x240
- Sectors per frame: 5
- Total frames: ~231
- Frame size: 7648 bytes
- BS ID: 0x3800
- BS version: 2

XA Sub-header (at sector offset 0-7):
- File #: 1
- Channel: 1
- Submode: 0x42 (video, Form1)

Important BOF3-specific nuance:

- the in-sector MDEC header in `CAPCOM30.STR` looks standard
- the extracted BOF3 file is still not a drop-in raw `2352`-byte STR for generic
  tools because the outer CD sync/header layer is missing
- repo-local tooling therefore rewraps the extracted `2336`-byte XA sectors into
  a normal raw-sector STR before FFmpeg playback
- `CAPCOM30.STR` also needs a BOF3-specific playback-rate correction; generic
  `15 fps` defaults play it at about half speed

Current repo interpretation:

- not proven to use Capcom's offset-`0x1c` first-sector-of-frame variant
- proven to use a BOF3-local headerless extracted wrapper plus a custom playback
  profile on top of otherwise standard-looking in-sector STR/MDEC headers
