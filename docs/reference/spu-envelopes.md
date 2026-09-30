# SPU envelope research

Sources inspected 2026-09-24 UTC:

- [psx-spx: SPU volume and ADSR generator](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-volume-and-adsr-generator)
  describes register fields and envelope arithmetic.
- [DuckStation's SPU implementation](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/spu.cpp),
  particularly `VolumeEnvelope::Reset`, `VolumeEnvelope::Tick`,
  `Voice::UpdateADSREnvelope` and `Voice::TickADSR`, supplies independent emulator
  behavior to compare. This URL tracks a live branch; no immutable revision was
  established by the research fetch. No emulator code or dependency is vendored.

## Unresolved differences

The published pseudocode starts exponential-increase slowdown above `0x6000`;
the inspected emulator includes equality. The published minimum counter increment
is applied after exponential adjustment; the emulator applies its minimum before
the adjustment. At sufficiently slow increasing exponential rates, those orders
can distinguish continued progression from a frozen envelope.

The emulator also makes phase-target checks on frames without an envelope step.
That matters after manual envelope-level writes. Counter reset on register writes
and signed manual-level behavior require device-level validation, including when
writes take effect relative to output frames. The two sources do not constitute
independently captured hardware traces.

## Repository evidence and boundaries

[The Rust ADSR component](../../tools/rust/bof3-audio/src/machine/adsr.rs) requires
an explicit arithmetic model. Shared state-transition behavior follows the
inspected emulator; the `Published` model selects the two published arithmetic
rules above, not a separately verified full hardware implementation.

[Tests](../../tools/rust/bof3-audio/tests/adsr.rs) retain literal boundary traces,
counter-rate alias checks, phase transitions, frozen rates and write/retrigger
cases. A bounded corpus probe compares 250 ms held notes plus 250 ms release
from VAB register pairs. It finds divergent trajectories in 125 of 543 distinct
pairs, used by 395 of 21,112 tone records. Register pair `89BA/514C` reaches the
threshold after eight ticks; the ninth yields levels 27648 versus 25344.

Those results compare models, not measured game output. They do not account for
game-time ADSR overrides, key scheduling, ADPCM termination or volume controls.
[The audio specification](../specs/formats/audio.md#rust-adsr-state-and-evidence-models)
owns integration status; the [migration plan](../plans/bof3-audio-rust-migration.md)
retains the unresolved rendering and SoundFont approximation gates.

## Volume sweeps

The same published generator describes signed stereo sweeps using direction,
phase, exponential mode and seven rate bits. Inverted linear steps reverse
direction; exponential decrease computes its step independently of phase, but
phase still selects the saturation range. Fixed-to-sweep writes require a
hardware delay that the frame-driven Rust component does not establish.

The inspected emulator preserves current gain on sweep setup, resets its counter,
stops at the relevant endpoint and clocks gains after applying them to output.
Its current main-volume writes change level without rearming a stopped sweep.
Current voice-volume reads are implemented there, but corresponding writes are
not established; Rust rejects them. The emulator can also skip inactive voices
when IRQ9 is disabled. Rust currently clocks all voice sweeps per requested frame,
an interpretation of the published generator rather than independently validated
inactive-voice timing. The two existing arithmetic models choose threshold/floor
rules only, not complete alternative device timing models.

[Implementation and validation](../specs/formats/audio.md#rust-spu-volume-sweeps)
retain these boundaries. The independent integer-vector hashes include 512
little-endian signed samples followed by a little-endian `u32` counter and one
active-state byte for each mode/rate/start context:

- Published: `a2f5dc635564de06a2b8554ade8d5ce234098c58d1c70c4de181687a1d989316`.
- Emulator arithmetic: `7388ac278502c579d5758840515eeee52682d7e05e4b63890527f5924a85635e`.

## SoundFont approximation target

The approved RustySynth 1.3.6 published source was inspected locally for envelope
fitting. Its `.cargo_vcs_info.json` pins repository commit
`8cc11fc0b10422adb54107757f100d3d6ae0ef96`, package path `rustysynth`:

- [Volume envelope](https://github.com/sinshu/rustysynth/blob/8cc11fc0b10422adb54107757f100d3d6ae0ef96/rustysynth/src/volume_envelope.rs):
  linear attack and exponential decay/release with coefficient 9.226.
- [Region envelope setup](https://github.com/sinshu/rustysynth/blob/8cc11fc0b10422adb54107757f100d3d6ae0ef96/rustysynth/src/region_ex.rs):
  release is clamped to at least 0.01 seconds.
- [SoundFont math](https://github.com/sinshu/rustysynth/blob/8cc11fc0b10422adb54107757f100d3d6ae0ef96/rustysynth/src/soundfont_math.rs):
  float timecents/gain conversion and the approximately 0.001 audibility cutoff.

This consumer behavior is an explicit approximation target, not a replacement
definition of SF2 or evidence for PSX hardware. The
[repository fit and corpus measurements](../specs/formats/audio.md#measured-sf2-envelope-fitting)
report both overall and maximum sampled error. A low time-weighted RMS can hide
a large short transient; neither measure establishes game-audio fidelity.
