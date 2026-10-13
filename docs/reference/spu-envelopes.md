# SPU envelope research

Hardware descriptions and the current native comparator:

- [psx-spx: SPU volume and ADSR generator](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-volume-and-adsr-generator)
  describes register fields and envelope arithmetic.
- [PCSX-Redux Agentic ADSR](../../third_party/pcsx-redux-agentic/src/spu/adsr.cc)
  and [register handler](../../third_party/pcsx-redux-agentic/src/spu/registers.cc)
  supply native envelope arithmetic and guest read/write behavior to compare.
  No emulator implementation is imported into the Rust component. Native
  behavior still requires independent hardware validation.

## Unresolved differences

The published pseudocode starts exponential-increase slowdown above `0x6000`;
Rust `EmulatorReference` includes equality. Rust `Published` applies its minimum
counter increment after exponential adjustment; `EmulatorReference` applies it
before. At sufficiently slow increasing exponential rates, those orders can
distinguish continued progression from a frozen envelope.

The native ADSR unit also includes equality at `0x6000`, but slows by adding eight
to a rate-table index, capped at 127. Its numerator/denominator and fractional
accumulator do not establish either Rust counter-floor policy. Native ENVX reads
reconstruct the level at the reader's cycle, including zero before the first
key-on envelope step. Manual ENVX writes are unimplemented there. Rust's
non-step-frame target checks, counter reset on register writes and signed
manual-level behavior therefore remain explicit component policies requiring
device-level validation. Neither source supplies independently captured hardware
traces or write timing.

## Repository evidence and boundaries

[The Rust ADSR component](../../tools/rust/bof3-audio/src/machine/adsr.rs) requires
an explicit arithmetic model. The models select the threshold and counter-floor
rules above within shared frame-driven state transitions. They do not constitute
separately verified hardware implementations or complete native device models.

[Tests](../../tools/rust/bof3-audio/tests/adsr.rs) retain literal boundary traces,
counter-rate alias checks, phase transitions, frozen rates and write/retrigger
cases. A bounded corpus probe compares 250 ms held notes plus 250 ms release
from VAB register pairs. It finds divergent trajectories in 125 of 543 distinct
pairs, used by 395 of 21,112 tone records. Register pair `89BA/514C` reaches the
threshold after eight ticks; the ninth yields levels 27648 versus 25344.

Those results compare models, not measured game output. They do not account for
game-time ADSR overrides, key scheduling, ADPCM termination or volume controls.
[The audio specification](../specs/formats/audio.md#rust-adsr-state-and-evidence-models)
owns integration status. Rendering fidelity and SoundFont approximation still
require independent validation.

## Volume sweeps

The same published generator describes signed stereo sweeps using direction,
phase, exponential mode and seven rate bits. Inverted linear steps reverse
direction; exponential decrease computes its step independently of phase, but
phase still selects the saturation range. Fixed-to-sweep writes require a
hardware delay that the frame-driven Rust component does not establish.

The [Rust sweep component](../../tools/rust/bof3-audio/src/machine/spu_volume.rs)
preserves current gain on sweep setup, resets its counter, stops at the relevant
endpoint and clocks gains after applying them to output. Its current main-volume
writes change level without rearming a stopped sweep; current voice-volume writes
remain rejected. Rust clocks all voice sweeps per requested frame, leaving
inactive-voice timing unvalidated.

The [native voice-volume decoder](../../third_party/pcsx-redux-agentic/src/spu/volume.cc)
approximates a sweep-mode write as a fixed level. It does not run the hardware
sweep envelope, and the native register handler leaves main-volume writes
unimplemented. The two Rust arithmetic models choose threshold/floor rules only;
native comparison and independent device timing remain separate obligations.

[Implementation and validation](../specs/formats/audio.md#rust-spu-volume-sweeps)
retain these boundaries. The independent integer-vector hashes include 512
little-endian signed samples followed by a little-endian `u32` counter and one
active-state byte for each mode/rate/start context:

- Published: `a2f5dc635564de06a2b8554ade8d5ce234098c58d1c70c4de181687a1d989316`.
- EmulatorReference arithmetic: `7388ac278502c579d5758840515eeee52682d7e05e4b63890527f5924a85635e`.

## SoundFont approximation target

The approved [RustySynth](https://github.com/sinshu/rustysynth) 1.3.6 published source was inspected locally for envelope
fitting. Its `.cargo_vcs_info.json` pins repository commit
`8cc11fc0b10422adb54107757f100d3d6ae0ef96`, package path `rustysynth`:

- Volume envelope in `rustysynth/src/volume_envelope.rs`:
  linear attack and exponential decay/release with coefficient 9.226.
- Region envelope setup in `rustysynth/src/region_ex.rs`:
  release is clamped to at least 0.01 seconds.
- SoundFont math in `rustysynth/src/soundfont_math.rs`:
  float timecents/gain conversion and the approximately 0.001 audibility cutoff.

This consumer behavior is an explicit approximation target, not a replacement
definition of SF2 or evidence for PSX hardware. The
[repository fit and corpus measurements](../specs/formats/audio.md#measured-sf2-envelope-fitting)
report both overall and maximum sampled error. A low time-weighted RMS can hide
a large short transient; neither measure establishes game-audio fidelity.
