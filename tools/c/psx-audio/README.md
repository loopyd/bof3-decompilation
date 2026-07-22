# BOF3 PSX audio tool

This directory contains the C11 audio library and CLI behind `bin/psx-audio`.
It decodes BOF3 XA, VAB, and SEP data, provides an approximate direct music
renderer, and hosts the executable-backed PSF/R3000/SPU path under development.

The durable format and hardware specification is
[`docs/specs/formats/audio.md`](../../../docs/specs/formats/audio.md). This page
is the implementation handoff: current state, findings, priorities, and checks.

## Current state

| Area | State |
| --- | --- |
| XA decoding | Implemented |
| EMI/VAB/SEP inspection | Implemented |
| WAV/Ogg/FLAC rendering | Implemented; Ogg/FLAC depend on system libraries |
| `fast` BGM engine | Usable and very close on current listening fixtures |
| Register-driven SPU | ADPCM, loops, pitch, Gaussian interpolation, ADSR, fixed volume, KON/KOFF/ENDX, RAM upload |
| PSF1/MiniPSF loader | Implemented and tested |
| Bounded R3000 machine | Partial, faults explicitly on unsupported behavior |
| `game` BGM engine | Initializes and uploads assets, but does not yet schedule audible voices |

`fast` is the default. `game` must fail explicitly while incomplete and must
never fall back to `fast`, because that would make reverse-engineering results
ambiguous.

## Known-good decisions

- Preserve raw `VagAtr.prog` from bytes 20-21. Reconstructed program ownership
  sounded worse on `BGM043`.
- Use all 24 hardware voices. When full, `fast` deterministically steals the
  oldest voice.
- Saturate the final mix instead of globally peak-normalizing it. Runtime gain
  remains a user playback/output option.
- Use the exact 193-entry linked note-to-pitch table found twice in
  `SLUS_004.22`, not floating-point `pow()` pitch conversion.
- Apply bank, program, tone, sequence-channel, velocity, and pan attributes.
- Keep the tone fine shift as the unsigned byte declared by PsyQ `VagAtr`.
- Keep pitch bend on `SepEvent.data2` for now. Swapping to `data1` caused an
  audible regression. `_SsSetPitchBend@0x8016A4A4` must be traced from the
  linked game parser/caller before changing the parsed field again.
- Zero ADPCM predictor and Gaussian history on key-on. DuckStation identifies
  this specifically as necessary to avoid clicks in *Breath of Fire III*.

## Listening findings

These are human observations, not byte-level proof:

| Fixture | Current observation |
| --- | --- |
| `BGM040A` | Horn/French-horn timbre or articulation is not fully correct |
| `BGM043` | Raw tone program ownership is materially better than reconstruction |
| `BGM054` | High/piano passages are close but retain mapping or articulation differences |
| `BGM067` | Piano and bass are very close; occasional pitch/articulation difference remains |
| `BGMBAT04` | Bass is close; sustained strings retain pitch/loop/effect differences |
| `BGMBAT05` | Current baseline is very close; `data1` pitch bend was worse |
| `BGM099`, `BGM127`, `BGM131` | Strings remain useful loop/effect regression fixtures |

Default-sequence audits for `BGM054`, `BGM067`, and `BGMBAT04` found that used
notes are covered by declared VAB tone ranges. A simple missing-range fallback
is therefore not the main remaining defect. `BGM054` and `BGM067` sequence 0
are identical but use different VAB/sample banks. Relevant tones inspected in
those tracks have zero vibrato and portamento fields.

## Likely remaining fast-path differences

Prioritize evidence and audible impact in this order:

1. Reverse the linked `SLUS_004.22` note-to-pitch routine completely. Prove
   signed fine-tune handling, center/shift combination, octave folding, table
   indexing, and output clamping. The table bytes themselves are already exact.
2. Add a note-to-tone audit reporting exact range matches, layers, fallback,
   center/shift, sample, bend range, and resulting SPU pitch for every used
   `(program, note)` pair.
3. Make `fast` loop samples through live ADPCM state. Its cached PCM loop reuses
   samples decoded with first-pass predictor history; hardware carries ADPCM
   predictor and Gaussian history from loop end into loop start. Sustained
   strings are the highest-risk fixture for this difference.
4. Trace pitch bend end to end through the linked SEP parser, event dispatch,
   `_SsSetPitchBend@0x8016A4A4`, `_SsVmPitchBend@0x801728E0`, and
   `_SsVmPBVoice@0x801726E0`. Do not infer parser field ownership from one
   callee's argument alone.
5. Determine from linked writes and VAB mode fields whether affected voices use
   reverb, pitch modulation, noise, or volume sweeps before implementing them.
6. Implement hardware reverb as the documented SPU-RAM half-rate feedback
   pipeline, not generic post-render reverb.
7. Match libsnd priority and voice allocation only if voice-pressure traces show
   current oldest-voice stealing changes the affected passages.

The hardware contract and coverage matrix are maintained in the main audio
spec. Primary external references are
[psx-spx](https://psx-spx.consoledev.net/soundprocessingunitspu/) and
[DuckStation's SPU](https://github.com/stenzek/duckstation/blob/master/src/core/spu.cpp).
They own hardware behavior, not BOF3 SEP/VAB semantics.

## Game-engine blocker

The executable-backed path currently reaches linked initialization, VAB head
open/body transfer, SEP open, SEP play, and a manual scheduler path. It then
reports:

```text
game scheduler produced no audible voice-register state
```

Initialization key masks are visible, but the game-owned tables/callback cadence
needed to turn parsed SEP events into nonzero voice volume and key writes are
not reconstructed. Continue from the EMI-populated bank/sequence tables and the
cue path around `0x80161BBC`, `0x80161C20`, and `0x80161CD0`; do not emulate the
entire game boot.

## Inspection workflow

```sh
bin/psx-audio emi-inspect out/extracted/BIN/BGM/BGM054.EMI
bin/psx-audio vab-inspect out/extracted/BIN/BGM/BGM054/0.bin
bin/psx-audio sep-inspect out/extracted/BIN/BGM/BGM054/1.bin --programs --notes
bin/psx-audio render-bgm BGM054 --engine fast -o out/audio/BGM054-fast.wav
bin/psx-audio play-bgm BGMBAT05 --gain 0.1
```

EMI entry numbers are not universal; consult each archive's `emi.json` and use
entry type 6 for VH, type 7 for VB, and type 10 for SEP.

## Build and checks

`bin/psx-audio` configures and builds automatically. For explicit checks:

```sh
cmake -S tools/c/psx-audio -B tools/c/psx-audio/build -DBUILD_TESTING=ON
cmake --build tools/c/psx-audio/build
ctest --test-dir tools/c/psx-audio/build --output-on-failure
git diff --check
```

The four focused tests are `psf_test`, `psx_machine_test`, `audio_core_test`,
and `spu_device_test`. Listening remains necessary for timbre, loop, reverb,
and arrangement regressions until a trusted emulator/hardware PCM or SPU trace
is available.
