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

The extracted file is an exact multiple of `2336` bytes. For the pinned
input `out/extracted/LOGO/CAPCOM30.STR` (SHA-256
`0f9145e980e401ded21f4c315375bcb989f49b8b83582f46f4a2946dd33ff06d`),
`bin/str-media inspect out/extracted/LOGO/CAPCOM30.STR` reports 1013
sectors, 203 frame records (frames 1-203, no gaps, frame 203 incomplete), and
one stereo XA stream (file 1,
channel 1, 37800 Hz, 126 sectors, 254016 samples, 6.72 s). A reversible
2352-byte sector wrapper preserves all inner sectors.
`bin/str-media validate out/extracted/LOGO/CAPCOM30.STR --expected-fps 30` writes
`out/str-media/CAPCOM30/validation.json` (schema `harness.str-validation/v1`)
with status `pass`: 203/30 = 6.7667 s video against 6.72 s audio, delta
0.0467 s within the 0.1067 s tolerance (two video frames or two primary XA
audio sectors). These numbers are reproducible by running the tracked
commands on the pinned input; the input itself is ignored disposable
extraction state, so the numbers are generated evidence for that extraction,
not durable corpus facts.

Earlier desktop-mux figures (231 frames, 1155 sectors, 2772 pad samples)
have no preserved source hash, date, or artifact identity, so they are not
reproducible and are not asserted. An unproven
30 fps at 2x CD sector-delivery inference drawn from that older external
sample is not attributed to any current asset and is not a game-runtime
contract until the LOGO scheduler path at `0x801cea98` is reviewed;
`0x801ce760` is already an exact lift
(`initWorkAreaAndStartSubsystems`, 37/37, 148 bytes, `@status exact`), and
its work-area-init/subsystem-start metadata does not by itself prove
conversion timing.

The naïve ffmpeg conversion reproduces the reported video-at-half-audio-speed
symptom because its default time base is wrong for this stream. That default is
not a canonical asset duration. The file is an exact number of extracted
sectors, and lossless wrapping recovers both streams, so missing end padding is
not supported as the cause. Padding is derived output only; the pinned
extraction contains exactly one stereo XA stream, so there is no separate
mono track to fold. Note
that `bin/str-media convert` exits 0 regardless of result status: read
`out/str-media/<stem>/conversion.json` and require `status: pass` before
treating a conversion as valid. `out/str-media/<stem>/conversion.json`
receipts are disposable per-run artifacts, so the reproducible contract is to
run `bin/str-media convert` and require `status: pass` in the generated
manifest (computing the output SHA-256 at conversion time); the passing
source validation is not conversion acceptance.
