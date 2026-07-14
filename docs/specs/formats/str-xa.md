---
type: Runtime
title: STR playback
description: BOF3 extracted STR and XA sector representation.
tags: [runtime, str, xa]
---

# STR playback

BOF3 stores extracted STR/XA data as `2336` byte sectors without the outer raw
CD sync and header:

```text
[XA subheader 8][payload 2324][EDC 4]
```

Generic PSX media tools commonly expect `2352` byte raw sectors. Rewrap the
sector before decoding; do not alter the inner payload.

## Preservation and desktop outputs

The extracted 2336-byte-sector file is the canonical archival source. Keep its
hash and provenance. A 2352-byte-sector wrapper is a reversible compatibility
representation: removing each generated 16-byte outer header must reproduce
every original 2336-byte sector byte-for-byte.

Decoded media is a derivative, even when encoded losslessly. The desktop output
is Matroska with H.264 lossless mode (`libx264 -qp 0`) and FLAC, preserving the
decoded pixel format and range without scaling. The conversion manifest records
codec options, pixel format/range, timing formula, and channel selection.

## Known files

| Path | Content |
| --- | --- |
| `LOGO/CAPCOM30.STR` | MDEC video plus stereo XA |
| `BIN/BMAG_XA/MAGIC00.STR` | multiplexed mono XA |
| `BIN/SCE_XA/S_XA00.STR` | multiplexed stereo XA |
| `BIN/SCE_XA/VOICE.STR` | multiplexed mono XA |

`CAPCOM30.STR` contains standard inner STR/MDEC chunk headers; the three `BIN/`
files are audio banks, not movies.

## CAPCOM30 timing evidence

For a desktop mux, derive timing from decoded measurements rather than fixed
asset constants:

```text
video_seconds = video_frame_count / selected_frames_per_second
audio_seconds = decoded_samples_per_channel / sample_rate
pad_samples_per_channel =
    max(0, round((video_seconds - audio_seconds) * sample_rate))
```

After padding, require audio and video duration equality within one audio-sample
period (or a stricter muxer-supported tolerance).

Desktop conversion uses lossless H.264 video (`qp = 0`) and FLAC audio in
Matroska. Conversion keeps the decoder's dimensions and chroma sampling when
the lossless encoder supports them and never scales the image. Assign video
timestamps from the frame index,
`pts = frame_index / selected_frames_per_second`; do not combine timestamp
rescaling with an output-rate option because that can duplicate a boundary
frame. Pad the selected XA stream by the formula above, then trim it to exactly
`round(video_seconds * sample_rate)` samples.

The extracted file is exactly `1155 * 2336` bytes. A reversible 2352-byte
sector wrapper preserves all 1155 inner sectors and yields these measured
streams when decoded:

| Measurement | Result |
| --- | ---: |
| Video frames | 231 |
| Main stereo XA packets | 143 |
| XA sample rate | 37800 Hz |
| Decoded stereo XA duration | 7.626667 s |
| Video duration at 30 fps (`231 / 30`) | 7.700 s |
| Padded desktop audio duration | 7.700 s |
| Desktop mux padding delta | 0.073333 s |
| Desktop silence per stereo channel at 37800 Hz | 2772 samples |
| 1155 sectors at 2x CD rate | 7.700 s |

The naïve ffmpeg conversion reproduces the reported video-at-half-audio-speed
symptom because its default time base is wrong for this stream. That default is
not a canonical asset duration. The file is an exact number of extracted
sectors, and lossless wrapping recovers both streams, so missing end padding is
not supported as the cause.

`INFERRED:` 30 fps video at 2x CD sector delivery is the strongly supported
desktop-conversion intent: both give 7.700 seconds and differ from decoded XA
by only 0.073333 seconds. Applying the formula above to this measured example
yields 2772 zero samples per 37800 Hz stereo channel. This is derived output
padding, not evidence of missing PSX source sectors. Keep the trailing mono
stream separate rather than folding it into the main stereo track. Runtime
intent remains unproven until the LOGO scheduler paths at `0x801ce760` and
`0x801cea98` are reviewed.
