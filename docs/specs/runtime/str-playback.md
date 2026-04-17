# STR Playback

BOF3 currently exposes four top-level `.STR` assets in `build/extracted/`.
They are not all the same kind of media:

- `LOGO/CAPCOM30.STR` is the only proved video-bearing STR.
- `BIN/BMAG_XA/MAGIC00.STR`, `BIN/SCE_XA/S_XA00.STR`, and
  `BIN/SCE_XA/VOICE.STR` are audio-only XA multiplexes stored in `.STR`
  containers.

## Corpus

| Path | Kind | Extracted sectors | Logical playback shape | Notes |
|------|------|-------------------|------------------------|-------|
| `LOGO/CAPCOM30.STR` | video + audio | `2336` bytes | `1` MDEC video stream + `1` stereo XA stream | extracted movie stream missing the outer raw-CD wrapper |
| `BIN/BMAG_XA/MAGIC00.STR` | audio only | `2336` bytes | `16` mono XA streams | channel-multiplexed bank |
| `BIN/SCE_XA/S_XA00.STR` | audio only | `2336` bytes | `8` stereo XA streams | heavy non-audio padding between streams |
| `BIN/SCE_XA/VOICE.STR` | audio only | `2336` bytes | `5` mono XA streams | small voice bank |

## Reverse-Engineered Extracted Format

The extracted BOF3 files are not stored as raw `2352`-byte CD sectors. They are
stored as `2336`-byte XA sectors with the outer sync/header layer removed.

Observed extracted layout:

```text
[XA sub-header 8][sector payload 2324][EDC 4]
```

Observed raw layout expected by generic STR readers:

```text
[CD sync 12][CD header 4][XA sub-header 8][sector payload 2324][EDC/ECC tail]
```

Implication:

- the in-sector video headers can still be standard STR/MDEC headers
- the extracted file is headerless relative to normal `2352`-byte STR tools
- inspection or playback through stock STR/XA tooling therefore needs a
  faithful `2336 -> 2352` rewrap step first

XA sub-header fields, repeated twice for redundancy:

- byte `0`: file number
- byte `1`: channel number
- byte `2`: submode
- byte `3`: coding info

Observed submode values in BOF3:

- `0x42`: video-bearing sector in `CAPCOM30.STR`
- `0x64`: audio-bearing XA sector
- `0xE4`: terminal/end-marked audio sector in the audio-only banks
- `0x00`: non-audio padding/filler sector in the audio-only banks

## CAPCOM30.STR

### Proven Findings

| Property | Value |
|----------|-------|
| Path | `LOGO/CAPCOM30.STR` |
| Size | `2,698,080` bytes |
| Sector size | `2336` |
| Sector count | `1155` |
| Video sectors | `1011` |
| Audio sectors | `144` |
| Video frames | `231` |
| Audio duration | about `7.68s` |
| Implied playback rate | about `30.08 fps` |

Observed sector patterns:

- dominant video sectors: file `1`, channel `1`, submode `0x42`, coding `0x80`
- dominant audio sectors: file `1`, channel `1`, submode `0x64`, coding `0x01`
- first video sector still carries a standard in-sector STR header:
  - STR id `0x0160`
  - STR type `0x8001`
  - width `320`
  - height `240`
  - frame `1`
  - chunk `0 / 5`

Interpretation:

- `CAPCOM30.STR` is custom in the extracted BOF3 sense because it is missing the
  outer raw-CD headers expected by stock `psxstr` readers
- its inner MDEC chunk headers still look like a normal STR/MDEC stream
- generic readers therefore need two BOF3-specific corrections:
  - rebuild the missing `2352`-byte sector wrapper
  - override the default `15 fps` assumption with the observed `~30 fps`

### Why Default FFmpeg Playback Is Wrong

FFmpeg's `psxstr` demuxer assumes `15 fps` video.

For `CAPCOM30.STR`:

- video at `15 fps`: `231 / 15 = 15.40s`
- audio duration: about `7.68s`
- correction factor: `7.68 / 15.40 = 0.498670`

Without that correction, the movie plays at roughly half speed and runs long
after the XA audio ends.

## Audio-Only STR Banks

These three files are not movies. They are XA audio multiplexes stored in `.STR`
containers. After faithful rewrap, generic XA-aware tooling should see each XA
channel as a separate logical audio stream.

### `MAGIC00.STR`

| Property | Value |
|----------|-------|
| Path | `BIN/BMAG_XA/MAGIC00.STR` |
| Sector size | `2336` |
| Audio streams after rewrap | `16` |
| Layout | mono XA, channels `0..15` |
| Approx per-stream duration | about `49s` to `51s` |

Observed structure:

- audio sectors: `14856`
- non-audio filler sectors: `552`
- terminal sectors: `16`
- every logical stream is short and roughly similar in duration

### `S_XA00.STR`

| Property | Value |
|----------|-------|
| Path | `BIN/SCE_XA/S_XA00.STR` |
| Sector size | `2336` |
| Audio streams after rewrap | `8` |
| Layout | stereo XA, channels `0..7` |
| Dominant streams | channels `1` and `3`, about `269s` each |

Observed structure:

- audio sectors: `11302`
- non-audio filler sectors: `29113`
- terminal sectors: `8`
- channels `1` and `3` hold the long-form programs; the other channels are short
  clips or side material

### `VOICE.STR`

| Property | Value |
|----------|-------|
| Path | `BIN/SCE_XA/VOICE.STR` |
| Sector size | `2336` |
| Audio streams after rewrap | `5` |
| Layout | mono XA, channels `0..4` |
| Approx per-stream duration | about `2.24s` to `3.79s` |

Observed structure:

- audio sectors: `297`
- non-audio filler sectors: `3238`
- terminal sectors: `5`

## Recovery Implication

For reverse-engineering purposes:

- keep the original `.STR` identity explicit in docs and loader traces
- treat `2336`-byte extracted sectors as the BOF3-local source format
- keep movie-bearing and audio-only `.STR` files separate in inventories and
  analysis
- do not describe the audio-only banks as one linear clip per file; their
  channel/stream structure is part of the runtime fact pattern

## References

- `docs/specs/runtime/logo-boot.md`
- `docs/specs/sources/psxspx-str-variants.md`
