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
bin/psx-audio render BGMBAT04 --engine fast -o track.wav
bin/psx-audio play BGM000.EMI             # play directly from EMI (zero-copy)
bin/psx-audio play BGM000 -o out.wav      # render to WAV instead of speakers
bin/psx-audio play VOICE.STR -c 0         # play XA voice channel 0
bin/psx-audio render BGM000.EMI -o out.wav
bin/psx-audio vab2sf2 BGM000.EMI -o bank.sf2
bin/psx-audio emi-inspect BGM000.EMI
bin/psx-audio psf-pack out/extracted/SLUS_004.22 -o out/audio/bof3.psflib
bin/psx-audio psf-inspect out/audio/bof3.psflib
bin/psx-audio psf-run out/audio/bof3.psflib -n 100000
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

The `fast` renderer applies this bank volume and pan together with the selected
program and tone attributes. It does not apply a post-render bass boost or EQ.

### BOF3 quirks vs standard PsyQ

| Aspect | Standard PsyQ | BOF3 |
| --- | --- | --- |
| ProgAtr region | After VabHdr | Padded to 0x800 bytes |
| VagAtr start | After ProgAtr | Fixed at **0x820** |
| VagAtr count | `ts` (flat) | **`ps × 16`** (2D) |
| VAG size units | 8-byte | 8-byte |
| VAG pointer table | Per-sample sizes | Per-sample sizes; accumulate preceding entries for offsets |
| VAG first block | Format-dependent | 16 bytes of zero ADPCM data; transferred and played, not stripped |

### VagAtr (32 bytes each, at 0x820)

Key fields: vol (byte 2), pan (3), center note (4), unsigned pitch tune (5,
preserved as the full byte and added to playback pitch as `tune / 128`
semitones),
min/max note (6–7), adsr1 (bytes 16–17 u16 LE), adsr2 (18–19),
parent program (20–21 i16 LE), and vag index (22–23 i16 LE, 1-based). The
parent program selects the `ProgAtr` volume and pan used by the renderer.
Pitch bend range is tone-local: bytes 12–13 hold the downward/upward range in
semitones; the SEP bend value scales that range rather than a fixed MIDI ±2.
At playback, PsyQ `SsPitchFromNote` quantizes combined fine tune to 16 steps
per semitone and uses the linked integer lookup table before writing the SPU
pitch value (`0x1000` = 44100 Hz). The linked implementation is
`exe/slus_004_22@0x80171B20`; its 193-entry table is at runtime address
`0x8018445C` (raw payload offset `0xEDC5C`). The table's 386 bytes and the table
compiled into `spu.c` have the same SHA-256,
`293278b74970e97b814ab68b63edf21d4dcdc6630bd5394fce250aec6cd955b2`.
Addresses `0x800FDA84` and `0x800FDC5C`, previously attributed to this table,
contain zeros in the mapped raw payload and are not pitch-table evidence.

The linked routine masks the fine argument to 16 bits, adds the unsigned tone
shift, divides by 8, and carries one semitone when the result reaches 16. It
then forms a signed 16-bit semitone from `note + 60 - center + carry`, divides
that value by 12 with truncation toward zero, and indexes
`table[(remainder * 16) + fine_index]`. The quotient minus five shifts the
table value by octaves. It returns the low 16 bits without clamping; the
`0x4000` maximum belongs to the SPU pitch-counter step and is applied by each
renderer at playback.

### Note-on pitch and tone selection (libsnd voice manager)

BOF3 BGM is stock PsyQ `libsnd` SEP playback wrapped by a Capcom EMI bank
loader; there is no custom Capcom sequence language. The normal music note-on
path is `_SsNoteOn -> _SsVmKeyOn -> _SsVmSelectToneAndVag`, which is distinct
from the explicit-tone SFX dispatcher `SsUtKeyOnV` (the routine at `0x8016E400`
that calls `note2pitch` at `0x8016E73C` with a caller-supplied tone). The BGM
renderer must not be modeled on the SFX path.

Verified against the linked binary and corroborated by VGMTrans and stock
libsnd:

- **Initial pitch uses `fine = 0`.** The normal note-on call is
  `note2pitch(note, 0, center, shift)`. The channel's current pitch bend is
  **not** folded into a new note. Bend is applied only when a pitch-bend event
  arrives, via `_SsVmPitchBend` (`0x801728E0`) iterating active voices through
  `_SsVmPBVoice` (`0x801726E0`, which has a single caller). Folding the stored
  channel bend into note-on pitch makes notes sound sharp until the next bend
  event.
- **Tone selection layers every matching tone.** For a logical program and
  note, `_SsVmSelectToneAndVag` walks the program's up-to-16 tones and selects
  **all** with `VagAtr.min <= note <= VagAtr.max`, allocating one SPU voice per
  layer. It does not pick only the first match, the nearest center note, or a
  fallback tone.
- **Logical program -> physical tone block is packed.** Tone blocks are stored
  only for non-empty programs; the physical block is the count of non-empty
  programs preceding the logical program (stock libsnd caches it in
  `ProgAtr.reserved1`). The embedded `VagAtr.prog` (bytes 20-21) names the
  logical program and is the renderer's lookup key.

#### Fast-path pitch faithfulness audit (BGMBAT04 off-key investigation)

| Component | Status | Evidence |
| --- | --- | --- |
| EMI -> VH/VB/SEP loading | byte-identical to direct bins | SHA-256 (VH@0x800, SEP@0x3000, VB@0x8800) |
| `spu_pitch_from_note` | faithful | disasm `0x80171B20` |
| `pitch_table` | byte-identical | dump @`0x8018445C` |
| `voice_pitch` bend arithmetic | faithful | disasm `_SsVmPBVoice@0x801726E0` |
| SPU pitch counter / Gaussian / key-on | matches | psx-spx + DuckStation |
| VagAtr center/shift parse | correct (`[4]`/`[5]`) | disasm + VGMTrans |
| note-on bend folding | **bug** | libsnd uses base pitch; bend only via events |

Differential: `BGMBAT02.EMI` sounds correct while `BGMBAT04.EMI` is sharp; both
share the same code and structurally similar VABs (single-note piano tones with
`center > note` and `shift` 54/59/62), so the defect is data-triggered rather
than a per-tone formula error. Piano (prog 0) has `pbmin = pbmax = 0` and
disjoint tone ranges, so the bend-fold fix does not change its pitch; if piano
remains sharp after the fix, a renderer pitch trace must pinpoint the cause.

## SEP format (Sequence Package)

The EMI catalog labels type 10 as "SEQ", but the container is **SEP**
(multi-track). All 81 BOF3 music files have exactly 4 sequences.

### File header (6 bytes)

Magic `70 51 45 53` ("SEQp" LE), version u16 BE = 0.

### Per-sequence header (13 bytes)

seq_id (u16 BE), resolution (u16 BE, always 48), tempo (3 bytes BE,
µs/quarter), time signature (2 bytes), data_size (u32 BE).

### Event encoding

MIDI-like: VLQ delta times, running status, note on/off,
program change, control change, pitch bend. Only meta events are tempo and EOT.

Pitch bend occupies two MIDI-style data bytes. The direct renderer uses the
second as the 7-bit coarse value centered at 64; the first is the fine byte and
is not consumed by the linked coarse bend calculation. Meta events omit SMF lengths:
tempo is `FF 51 tt tt tt` and EOT is `FF 2F`.

NRPN extensions: loop start (20), loop end (30), VAB attribute control.

## PS1 SPU emulation

### Evidence boundary

Use each source only for the layer it owns:

| Evidence | Owns | Does not establish |
| --- | --- | --- |
| Original `SLUS_004.22` bytes and linked routines | BOF3/PsyQ SEP parsing, VAB interpretation, note-to-pitch conversion, voice allocation, and scheduler behavior | Undocumented electrical/timing behavior inside the SPU |
| Sony PsyQ 4.7 headers in `toolchains/psyq/4.7/include/` | Public structure layouts, field types, and API contracts | Linked implementation details or BOF3 call policy |
| [psx-spx SPU specification](https://psx-spx.consoledev.net/soundprocessingunitspu/) | Hardware registers, ADPCM, pitch counter, interpolation, ADSR, volume, transfer, noise, modulation, and reverb behavior | BOF3's sequence semantics or game scheduler |
| [DuckStation `src/core/spu.cpp`](https://github.com/stenzek/duckstation/blob/master/src/core/spu.cpp) | Tested implementation cross-check for the hardware specification | BOF3/PsyQ-specific parsing and allocation |

External implementations are corroboration, not code to import blindly. Preserve
their license boundaries and verify constants or algorithms against psx-spx and,
where possible, original bytes or hardware traces.

### Renderer architecture

`audio_render()` is the stable rendering seam. `AUDIO_ENGINE_FAST` delegates to
the direct SEP/VAB renderer. `AUDIO_ENGINE_GAME` is reserved for execution of
the linked BOF3 sound code and currently returns an explicit unsupported-engine
status; it must not silently fall back to the approximate renderer.

The exact path is split below that seam:

| Module | Contract | Current state |
| --- | --- | --- |
| `psf.c` | PSF1/MiniPSF load, overlay, CRC, PC/SP, and package | Implemented and tested |
| `psx_machine.c` | Bounded R3000 execution and PSF hardware boundary | Partial vertical slice |
| `spu_device.c` | SPU registers, 24 voices, live ADPCM, pitch, Gaussian interpolation, ADSR, sound RAM, FIFO/DMA | Implemented except reverb, noise, modulation, and volume sweeps |
| `render.c` | Direct SEP/VAB scheduling into `spu_device.c` | Implemented `fast` engine with 24-voice stealing and register-driven output |

The machine intentionally faults on unsupported instructions, BIOS calls, and
hardware addresses. The complete game PSF currently reaches its first CD-ROM
register access at `PC=0x80176E80` after 33,470 interpreted instructions. PSF1
does not provide CD-ROM hardware, so the exact player requires a bootstrap that
installs audio assets before entering the game sound runtime; emulating the full
game boot is not the target architecture.

### Hardware coverage

The SPU advances at 44.1 kHz, or once per `0x300` CPU clocks. Voice and main
register changes are therefore sample-clocked on hardware; the current device
applies most writes immediately. This timing difference matters to exact game
execution but not to an offline `fast` event scheduled on output frames.

| Hardware contract | Register-driven `spu_device.c` | Direct `fast` renderer |
| --- | --- | --- |
| 24 equivalent voices | Implemented | Implemented; oldest voice is stolen when full |
| 512 KiB SPU RAM and 8-byte address units | Implemented with wrapping | Uses the same register-driven SPU RAM |
| 16-byte ADPCM blocks, 28 samples | Live decode | Uses the same live decoder |
| Loop start/end/repeat flags and ENDX | Implemented | Uses the same flag and ENDX path |
| Predictor and interpolation history across loops | Preserved by live decode | Preserved by live decode |
| Zero interpolation history on key-on | Implemented | Implemented by zero-padding before sample index zero |
| `VxPitch`: `0x1000` = 44.1 kHz, clamp above `0x4000` | Implemented | Equivalent ratio after PsyQ note conversion |
| 4-point, 512-entry Gaussian interpolation | Implemented | Uses the same implementation |
| ADSR attack/decay/sustain/release | Implemented | Uses the same implementation |
| Signed fixed voice/main volume | Implemented | Writes fixed SPU voice/main volume registers |
| Volume sweep mode | Not implemented; sweep values currently mute | Not implemented |
| KON, KOFF, ENDX | Implemented | Modeled as note events, not registers |
| Pitch modulation (PMON) | Not implemented | Not implemented |
| Noise source (NON) | Not implemented | Not implemented |
| Per-voice and master reverb | Not implemented | Not implemented |
| SPUCNT enable/mute and delayed status | Registers stored; behavior/timing incomplete | Not applicable |
| Manual/DMA transfer FIFO timing and IRQ | Data transfer implemented; FIFO timing and IRQ incomplete | Not applicable |
| CD/XA and external-input mixing/capture | Not implemented | Separate XA decoder; not mixed through SPU |

DuckStation explicitly zeroes the previous-block interpolation samples at key-on
to avoid clicks in *Breath of Fire III*. Keep this as a BOF3 regression invariant.
The fast path now decodes loop starts again with predictor and Gaussian history
retained from the loop end instead of replaying a predecoded PCM loop.

### ADPCM decode

Hardware-verified formula (from DuckStation, matching nocash PSX specs):

```c
sample = (int16_t)(nibble << 12) >> shift;   // sign-extend + shift
sample += (prev1 * filter_pos) >> 6;
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
coefficients. On hardware, ADPCM predictor and Gaussian sample history continue
across loop jumps rather than resetting at the loop-start block. This is true in
the register-driven device but not yet exact in the predecoded `fast` cache.

### Gaussian interpolation

4-point interpolation using the hardware's 512-entry gaussian table
(extracted from DuckStation, verified against nocash specs):

```c
out  = (gauss[0x0FF - i] * oldest) >> 15;
out += (gauss[0x1FF - i] * older)  >> 15;
out += (gauss[0x100 + i] * old)    >> 15;
out += (gauss[0x000 + i] * new)    >> 15;
```

The interpolation index is pitch-counter bits 4–11. Bits 12 and above select
the decoded source sample. Steps above `0x4000` clamp to `0x4000` after optional
pitch modulation, so high pitches may skip decoded samples while still using
the same four-point interpolation window.

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

### Voice and transfer invariants

- KON clears the corresponding ENDX bit, resets ADSR level to zero, resets
  interpolation/predictor history, and starts attack from the configured start
  address. Writing a start address does not redirect an already playing voice.
- KOFF enters release from any ADSR phase. It does not immediately silence the
  voice.
- ADPCM flag bit 0 sets ENDX after the current block. With repeat bit 1 set it
  jumps to the repeat address; otherwise playback ends and release begins.
  Flag bit 2 copies the current block address into the repeat address.
- Fixed volume is signed and negative values invert phase. Bit 15 selects a
  separate sweep envelope, not a fixed magnitude.
- SPU RAM is not CPU-mapped. Transfer and voice addresses are in 8-byte units;
  the transfer FIFO port moves 16-bit values and the hardware FIFO holds 32
  halfwords. DMA uses channel 4.
- SPUCNT controls enable/mute, transfer mode, reverb, noise clock, and CD or
  external input. SPUSTAT reflects several changes after hardware delay rather
  than immediately.

PMON modulates voice `n` from voice `n-1`; voice 0 cannot be modulated. NON
replaces ADPCM with a shared hardware noise source, so `VxPitch` does not set
noise frequency. Reverb is a half-rate SPU-RAM feedback pipeline with per-voice
send bits and master enable/output controls. These features should be ported as
coherent units from the hardware contract, not approximated by post-render DSP.

## Runtime loading

| EMI type | Handler | Action |
| ---: | --- | --- |
| 6 | `func_80162790` | Release prior owner, copy VH to RAM |
| 7 | `func_80162898` | `SpuSetTransferMode(0)`, `SsVabClose`, `SsVabOpenHeadSticky` |
| 8 | `func_801629F0` | Copy auxiliary audio (ADSR override table, `INFERRED`) |
| 9, 10 | `func_80162A6C` | Copy sequence to RAM |

### Executable sound control

The following behavior is derived from `SLUS_004.22` instructions and game
callers, not from PsyQ object implementations:

| Address | Binary-supported role |
| ---: | --- |
| `0x8015CEBC` | Audio shutdown/reset: all-key-off, closes active VAB IDs, releases seven active slots through a linked routine, disables reverb, and ends the sound runtime |
| `0x8015D044` | Polls the key state of all 24 SPU voices into the game voice-state table |
| `0x8015DF18` | Queued cue dispatcher used by overlays; its cases issue `SsUtKeyOnV` and detailed voice-volume operations |
| `0x80161BBC` | Ensures one logical audio bank is active, starting the EMI stream when the selected bank differs |
| `0x80161C20` | Starts and records a selected cue through game wrappers at `0x8015D300` and `0x8015D49C` |
| `0x80161CD0` | Updates a selected cue through the game wrapper at `0x8015D554` |

`0x8015CEBC` is therefore not the music scheduler or bootstrap entry point. A
standalone PSF bootstrap must reproduce the tables populated by the EMI audio
lifecycle before calling the cue-start path.

The linked calls observed below the game wrappers include addresses
`0x8016AE7C`, `0x8016B9CC`, `0x8016D6C4`, and `0x8016DBA0`. Their exact roles
and signatures remain address-based until all game-binary callers agree.

### PSF1 image contract

The PSF1 module enforces:

- PSF version `0x01`, compressed-program CRC, and bounded zlib expansion.
- A valid PS-X EXE program no larger than the PSF1 limit.
- Text overlays fully contained in 2 MiB PlayStation RAM.
- `_lib` recursion limited to 10 levels.
- `_lib`, current image, then contiguous `_lib2` and later overlay order.
- Initial PC/SP inherited from the first/deepest base image.
- First applicable `_refresh` tag, otherwise the outer EXE region marker.

For the local BOF3 executable, `psf-pack` followed by `psf-inspect` reports:

```text
PC:      0x8014AA0C
SP:      0x801FFFF0
RAM:     0x96800-0x1F7000
refresh: 60Hz
```

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
| `sep-inspect <sep> [--programs] [--notes]` | Show SEP info and optional program/note histograms |
| `sep2mid <sep> -o out.mid` | Export to Standard MIDI |
| `psf-pack <PS-X EXE> -o out.psflib` | Package a PSF1 executable |
| `psf-inspect <file.psf>` | Validate and compose a PSF1/MiniPSF image |
| `psf-run <file.psf> [-n N]` | Run a bounded machine diagnostic |

`--engine fast|game` selects the BGM engine. `fast` is the current default.
`game` now executes the linked initialization, VAB upload, SEP open/play, and
manual scheduler path, but refuses to emit a file while that scheduler produces
no audible voice-register state. It never falls back to `fast`. The remaining
bootstrap work is to reproduce the game-owned scheduler/table state that turns
parsed SEP events into nonzero voice volume/key writes.

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
| `tools/c/psx-audio/util.h` | Shared helpers and Gaussian table |
| `tools/c/psx-audio/adpcm.c` | ADPCM decode core |
| `tools/c/psx-audio/spu.c` | ADSR envelope |
| `tools/c/psx-audio/spu_device.c` | Register-driven SPU device under construction |
| `tools/c/psx-audio/psx_machine.c` | Bounded PSF1 R3000 runtime under construction |
| `tools/c/psx-audio/psf.c` | PSF1/MiniPSF image loader and writer |
| `tools/c/psx-audio/render.c` | Approximate `fast` BGM renderer |
| `tools/c/psx-audio/third_party/miniaudio.h` | Audio playback (v0.11.25) |
| `tools/python/harness/assets/audio/` | Python decoders |
| `tools/python/harness/commands/audio.py` | Python CLI |
| `bin/psx-audio` | C tool wrapper |
| `bin/bof3-audio` | Python tool wrapper |
| `out/extracted/BIN/BGM/` | Extracted BGM archives |
| `out/extracted/BIN/SCE_XA/` | Extracted XA streams |
| `out/catalog/emi.json` | Full EMI entry catalog |

## Open questions

- Standalone bootstrap: recover the EMI-populated bank/sequence tables needed
  by `0x80161C20` and the callback cadence that services active sequences.
- Linked sequence calls: prove names and signatures for `0x8016AE7C`,
  `0x8016B9CC`, `0x8016D6C4`, and `0x8016DBA0` from game-binary callers.
- Type-8 semantics: ADSR override table structure plausible but unconfirmed.
- SPU RAM layout: VAB base addresses not documented.
- Area→BGM mapping: lives in scenario controller code, not yet lifted.
