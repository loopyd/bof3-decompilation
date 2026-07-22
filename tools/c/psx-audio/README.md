# BOF3 PSX audio tool

This directory contains the C11 audio library and CLI behind `bin/psx-audio`.
It decodes BOF3 XA, VAB, and SEP data, drives a register-backed SPU music
renderer, and hosts the executable-backed PSF/R3000 path under development.

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
- Use the exact 193-entry table referenced by linked pitch conversion at
  `SLUS_004.22@0x80171B20`, not floating-point `pow()` conversion. The table is
  at runtime `0x8018445C` and byte-matches the one compiled into `spu.c`.
- Apply bank, program, tone, sequence-channel, velocity, and pan attributes.
- Keep the tone fine shift as the unsigned byte declared by PsyQ `VagAtr`.
- Keep pitch bend on `SepEvent.data2`. The linked bend path receives this
  centered coarse value; `data1` is the MIDI-style fine byte and replacing the
  coarse value with it causes an audible regression.
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

The corpus audit found valid VH/VB sizes, VAG ranges, per-sample zero prefixes,
and ADPCM end flags in all 81 BGM archives. `BGMBAT04` has 63 intentionally
unmapped program-0/note-38 events; the linked selector returns no voice for
these, so `fast` no longer invents a nearest-center fallback. `BGM054` and
`BGM067` sequence 0 are identical but use different VAB/sample banks. All BGM
tone vibrato and portamento fields are zero.

## Likely remaining fast-path differences

Prioritize evidence and audible impact in this order:

1. Determine from linked writes and VAB mode fields whether affected voices use
   reverb, pitch modulation, noise, or volume sweeps before implementing them.
2. Implement hardware reverb as the documented SPU-RAM half-rate feedback
   pipeline, not generic post-render reverb.
3. Match libsnd priority and voice allocation only if voice-pressure traces show
   current oldest-voice stealing changes the affected passages.

## Reversed linked pitch conversion

`exe/slus_004_22@0x80171B20` is called by `_SsVmPBVoice@0x801726E0`. Its MIPS
instructions prove this contract:

```text
fine_index = ((uint16_t)fine + tone.shift) / 8
carry = fine_index >= 16
fine_index -= carry * 16
semitone = (int16_t)(note + 60 - tone.center + carry)
octave = semitone / 12
remainder = semitone - octave * 12
pitch = table[remainder * 16 + fine_index]
pitch = octave > 5 ? pitch << (octave - 5) : pitch >> (5 - octave)
return (uint16_t)pitch
```

The routine does not clamp to `0x4000`; that limit belongs to SPU pitch-counter
playback. The direct renderer clamps after conversion, matching that layer
boundary. The table occupies 386 bytes at runtime `0x8018445C`, raw payload
offset `0xEDC5C`, and hashes to
`293278b74970e97b814ab68b63edf21d4dcdc6630bd5394fce250aec6cd955b2`.

Analysis identity: raw payload
`out/binaries/exe/slus_004_22.bin`, SHA-256
`677754d0d22c88151a5022cd98b8e89af1b0882177d9850faf62676eb7089eff`,
mapped at `0x80096800` by `config/targets/exe/slus_004_22/target.toml` and
inspected with Rizin 0.8.2. Ghidra pseudocode was unavailable in the installed
Rizin build; conclusions above come from the MIPS instructions and delay slots.

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
bin/psx-audio render BGM054 --engine fast -o out/audio/BGM054-fast.wav
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
