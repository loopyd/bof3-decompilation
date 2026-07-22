---
type: Format
title: Audio formats
description: BOF3 audio subsystems — XA ADPCM streaming, VAB sample banks, SEP sequences, SPU emulation, and tooling.
tags: [formats, audio, xa, vab, sep, spu, tooling]
---

# Audio formats

BOF3 has two independent audio paths: **streaming XA ADPCM** (voice, SFX,
magic) delivered through the CD hardware, and **synthesized music** (VAB sample
banks + SEP sequences) driven by the PsyQ `libsnd`/`libspu` runtime.

## Quick start

```sh
bin/psx-audio list                        # browse all 81 BGM tracks
bin/psx-audio play BGM000                 # play by track name (auto-resolves)
bin/psx-audio play BGMBAT02 --gain 0.7    # adjust playback/render volume
bin/psx-audio render BGMBAT04 -o track.ogg # compressed Ogg Vorbis output
bin/psx-audio render BGMBAT04 -o track.flac # lossless compressed output
bin/psx-audio play BGM000.EMI             # play directly from EMI (zero-copy)
bin/psx-audio play BGM000 -o out.wav      # render to WAV instead of speakers
bin/psx-audio play VOICE.STR -c 0         # play XA voice channel 0
bin/psx-audio render BGM000.EMI -o out.wav
bin/psx-audio vab2sf2 BGM000.EMI -o bank.sf2
bin/psx-audio emi-inspect BGM000.EMI
bin/psx-audio --examples
```

Python wrapper (includes track browser with 81 BGM names):

```sh
bin/bof3-audio list                              # browse all audio
bin/bof3-audio play BGMBAT06 -g 0.5              # render + play boss battle
bin/bof3-audio export BGMOPN -f midi             # export to MIDI
```

## Audio content on disc

### BGM tracks (81 archives in `BIN/BGM/`)

Each `BGMxxx.EMI` contains a VAB+SEP bundle:

| EMI type | Role | Format |
| ---: | --- | --- |
| 6 | VAB header (VH) | PS1 sample bank header |
| 7 | VAB body (VB) | ADPCM-encoded samples |
| 10 | SEP sequence | Multi-track MIDI-like events |

Named tracks: `BGMOPN` (opening), `BGMEND` (ending), `BGMSPC` (special),
`BGMBAT00`–`BGMBAT06` (battle 1–6 / boss). Remaining are numbered
(`BGM000`–`BGM197`), some with `A`/`B` variants.

### XA streaming audio (STR files)

| File | Content | Channels |
| --- | --- | --- |
| `BIN/SCE_XA/S_XA00.STR` (79 MB) | Scenario music/SFX | 8 stereo |
| `BIN/SCE_XA/VOICE.STR` (7 MB) | Voice clips | 5 mono |
| `BIN/BMAG_XA/MAGIC00.STR` (30 MB) | Magic effects | 16 mono |
| `LOGO/CAPCOM30.STR` (2.3 MB) | Capcom logo (video+audio) | 1 stereo |

### Sound banks (VAB in non-BGM archives)

| Family | Banks | Content |
| --- | ---: | --- |
| BENEMY | 200 | Enemy voice/SFX |
| BPLCHAR | 207 | Player character battle audio |
| BOSS | 155 | Boss battle audio |
| BMAGIC | 128 | Magic effect SFX |
| WORLD00–04 | ~200 | Area-local ambient SFX |
| BATTLE | 10 | Battle SFX |
| ETC | 17 | System/frontend audio |
| PLCHAR | 19 | Player character audio |

## XA ADPCM format

### Sector layout (2336 bytes)

```text
Offset  Size  Field
0x000     4   Subheader: [file_number, channel, submode, coding]
0x004     4   Subheader copy
0x008  2304   18 sound units × 128 bytes
0x918    24   Reserved
```

### Sound unit (128 bytes)

4 parameter blocks (4 bytes each) at offset 0, then 112 bytes of sample data
(4 groups × 28 bytes) at offset 16.

Param byte 0: `shift` (bits 0–3), `filter` (bits 4–5).

**Mono**: all 4 param blocks identical; 224 samples per unit.
**Stereo**: blocks 0,2 = Left, blocks 1,3 = Right; 112 samples per channel.

### Coding byte

- bit 0: stereo (1) vs mono (0)
- bit 2: 18900 Hz (1) vs 37800 Hz (0)

BOF3 uses only `0x00` (mono 37800 Hz) and `0x01` (stereo 37800 Hz).

### Multiplexing

Streams separated by `channel` field. Round-robin with fixed period.
EOF sectors (submode & 0x80) are valid audio.

## VAB format (BOF3 variant)

### VabHdr (32 bytes at 0x00)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 4 | Magic `"VABp"` (bytes `70 42 41 56`) |
| `0x04` | 4 | Version (7) |
| `0x0C` | 4 | fsize (VH + VB total) |
| `0x10` | 2 | reserved0 (`0xEEEE`) |
| `0x12` | 2 | ps (programs) |
| `0x14` | 2 | ts (total tones) |
| `0x16` | 2 | vs (VAGs) |
| `0x18` | 1 | mvol |
| `0x19` | 1 | pan |

### BOF3 quirks vs standard PsyQ

| Aspect | Standard PsyQ | BOF3 |
| --- | --- | --- |
| ProgAtr region | After VabHdr | Padded to 0x800 bytes |
| VagAtr start | After ProgAtr | Fixed at **0x820** |
| VagAtr count | `ts` (flat) | **`ps × 16`** (2D) |
| VAG size units | 8-byte | 8-byte |
| VAG pointer table | Per-sample sizes | Per-sample sizes; accumulate preceding entries for offsets |
| VAG data prefix | None | 16 bytes of zeros per size-delimited sample |

### VagAtr (32 bytes each, at 0x820)

Key fields: vol (byte 2), pan (3), center note (4), unsigned pitch tune (5,
clamped to 127 and added to playback pitch as `tune / 128` semitones),
min/max note (6–7), adsr1 (bytes 16–17 u16 LE), adsr2 (18–19),
vag index (22–23 i16 LE, 1-based).
Pitch bend range is tone-local: bytes 12–13 hold the downward/upward range in
semitones; the SEP bend value scales that range rather than a fixed MIDI ±2.
At playback, PsyQ `SsPitchFromNote` quantizes combined fine tune to 16 steps
per semitone and writes an integer SPU pitch value (`0x1000` = 44100 Hz).

## SEP format (Sequence Package)

The EMI catalog labels type 10 as "SEQ", but the container is **SEP**
(multi-track). All 119 BOF3 files have exactly 4 sequences.

### File header (6 bytes)

Magic `70 51 45 53` ("SEQp" LE), version u16 BE = 0.

### Per-sequence header (13 bytes)

seq_id (u16 BE), resolution (u16 BE, always 48), tempo (3 bytes BE,
µs/quarter), time signature (2 bytes), data_size (u32 BE).

### Event encoding

MIDI-like: VLQ delta times, running status, note on/off,
program change, control change, pitch bend. Only meta events are tempo and EOT.

Pitch bend occupies two MIDI-style data bytes, but PsyQ `libsnd` ignores the
first and uses the second as a 7-bit value centered at 64. Meta events omit
SMF lengths: tempo is `FF 51 tt tt tt` and EOT is `FF 2F`.

NRPN extensions: loop start (20), loop end (30), VAB attribute control.

## PS1 SPU emulation

### ADPCM decode

Hardware-verified formula (from DuckStation, matching nocash PSX specs):

```c
sample = (int16_t)(nibble << 12) >> shift;   // sign-extend + shift
sample += (prev1 * filter_pos) >> 6;          // no +32 rounding
sample += (prev2 * filter_neg) >> 6;
clamp(sample, -32768, 32767);
```

Filter coefficients:

| Filter | pos | neg |
| ---: | ---: | ---: |
| 0 | 0 | 0 |
| 1 | 60 | 0 |
| 2 | 115 | −52 |
| 3 | 98 | −55 |
| 4 | 122 | −60 |

Reserved shifts 13–15 decode as shift 9; reserved filters 5–15 use zero
coefficients. ADPCM predictor and Gaussian sample history continue across loop
jumps rather than resetting at the loop-start block.

### Gaussian interpolation

4-point interpolation using the hardware's 512-entry gaussian table
(extracted from DuckStation, verified against nocash specs):

```c
out  = (gauss[0x0FF - i] * oldest) >> 15;
out += (gauss[0x1FF - i] * older)  >> 15;
out += (gauss[0x100 + i] * old)    >> 15;
out += (gauss[0x000 + i] * new)    >> 15;
```

### ADSR envelope

From nocash PSX specs, cross-referenced with DuckStation:

```text
AdsrCycles = 1 << max(0, shift - 11)
AdsrStep   = step_value << max(0, 11 - shift)

if exponential AND increasing AND level > 0x6000:
    AdsrCycles *= 4    (or step /= 4 for rate < 40)

if exponential AND decreasing:
    AdsrStep = AdsrStep * level / 0x8000
```

| Phase | Mode | Direction | Step |
| --- | --- | --- | --- |
| Attack | Linear/Exp | Increase | +7,+6,+5,+4 |
| Decay | Exp (fixed) | Decrease | −8 |
| Sustain | Linear/Exp | Prog | +7..+4 / −8..−5 |
| Release | Linear/Exp | Decrease | −8 |

ADSR1: bit15=attack_mode, bits14-10=attack_shift, bits9-8=attack_step,
bits7-4=decay_shift, bits3-0=sustain_level (`(N+1)*0x800`).

ADSR2: bit15=sustain_mode, bit14=sustain_dir, bits12-8=sustain_shift,
bits7-6=sustain_step, bit5=release_mode, bits4-0=release_shift.

## Runtime loading

| EMI type | Handler | Action |
| ---: | --- | --- |
| 6 | `func_80162790` | Release prior owner, copy VH to RAM |
| 7 | `func_80162898` | `SpuSetTransferMode(0)`, `SsVabClose`, `SsVabOpenHeadSticky` |
| 8 | `func_801629F0` | Copy auxiliary audio (ADSR override table, `INFERRED`) |
| 9, 10 | `func_80162A6C` | Copy sequence to RAM |

## Tooling

### C tool (`tools/c/psx-audio/`)

Self-contained C11 library + CLI. Uses miniaudio for playback.
Gaussian table and ADSR from DuckStation (hardware-verified).

```sh
bin/psx-audio <command>           # auto-builds on first run
```

| Command | Description |
| --- | --- |
| `play <vh> <vb> <sep>` | Render BGM + play through speakers |
| `play-xa <str> [-c CH]` | Decode XA + play |
| `play-vag <vh> <vb> [-v N]` | Play VAG sample(s) |
| `render <vh> <vb> <sep> -o out.wav` | Render BGM to WAV |
| `xa-decode <str> -o out.wav [-c CH]` | Decode XA to WAV |
| `xa-inspect <str>` | List XA streams |
| `vab-extract <vh> <vb> -o DIR` | Extract VAGs to WAV |
| `vab-inspect <vh>` | Show VAB info |
| `sep-inspect <sep>` | Show SEP info |
| `sep2mid <sep> -o out.mid` | Export to Standard MIDI |

### Python wrapper (`tools/python/harness/`)

Track browser with 81 BGM names, batch export, SF2 SoundFont generation.

```sh
bin/bof3-audio list [bgm|xa|banks]
bin/bof3-audio info <track>
bin/bof3-audio play <track> [-g GAIN] [-s SEQ]
bin/bof3-audio export <track> [-f midi|sf2|wav|all]
bin/bof3-audio export-all
```

### ffmpeg validation

ffmpeg has `adpcm_xa`, `adpcm_psx` decoders and `psxstr` demuxer for
cross-validating our output:

```sh
ffmpeg -f psxstr -i wrapped_2352.str -vn output.wav
```

## File locations

| Path | Content |
| --- | --- |
| `tools/c/psx-audio/` | C library + CLI source |
| `tools/c/psx-audio/psx_util.h` | Shared helpers + gaussian table |
| `tools/c/psx-audio/psx_adpcm.c` | ADPCM decode core |
| `tools/c/psx-audio/psx_spu.c` | ADSR envelope |
| `tools/c/psx-audio/psx_render.c` | BGM renderer |
| `tools/c/psx-audio/third_party/miniaudio.h` | Audio playback (v0.11.25) |
| `tools/python/harness/assets/audio/` | Python decoders |
| `tools/python/harness/commands/audio.py` | Python CLI |
| `bin/psx-audio` | C tool wrapper |
| `bin/bof3-audio` | Python tool wrapper |
| `out/extracted/BIN/BGM/` | Extracted BGM archives |
| `out/extracted/BIN/SCE_XA/` | Extracted XA streams |
| `out/catalog/emi.json` | Full EMI entry catalog |

## Open questions

- Sequence playback pipeline: `SsSeqOpen` absent from SDK maps; BOF3 may
  use a custom player. Runtime track selection unproven.
- Type-8 semantics: ADSR override table structure plausible but unconfirmed.
- SPU RAM layout: VAB base addresses not documented.
- Area→BGM mapping: lives in scenario controller code, not yet lifted.
