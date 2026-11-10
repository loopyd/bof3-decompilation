# SPU envelope research

Hardware descriptions and the current native comparator:

- [psx-spx: SPU volume and ADSR generator](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-volume-and-adsr-generator)
  describes register fields and envelope arithmetic.
- [PCSX-Redux Agentic ADSR](../../third_party/pcsx-redux-agentic/src/spu/adsr.cc)
  and [register handler](../../third_party/pcsx-redux-agentic/src/spu/registers.cc)
  supply native envelope arithmetic and guest read/write behavior to compare.
  No emulator implementation is imported into the Rust component. Native
  behavior still requires independent hardware validation.

## Arithmetic models and boundaries

[The Rust ADSR component](../../tools/rust/bof3-audio/src/machine/spu/voice/envelope.rs)
requires an explicit arithmetic model. `Published` retains its Q15 accumulator,
strict `level > 0x6000` exponential-increase threshold and frozen-rate rules.
It applies its minimum counter increment after exponential adjustment.

`EmulatorReference` follows the current native denominator table. Its counter
counts elapsed frames since the last level step. Exponential increase at
`level >= 0x6000` uses `min(rate + 8, 127)`, without a frozen-rate sentinel.
ADSR writes, key-off and phase changes preserve this counter; key-on resets it.
Decay enters sustain when the upper four level bits are at most the sustain
field. Release stops only when its calculated level becomes negative. An
exponential release can remain active at zero. Native-reference voice output
uses the freshly stepped envelope; published output uses the preceding level.

Native ENVX reads reconstruct the level at the reader's cycle, including zero
before the first key-on envelope step. The native guest register handler ignores
manual ENVX writes. Rust's signed level injection remains a component policy.
Neither arithmetic model establishes hardware timing or complete device fidelity.

## Repository evidence and boundaries

[Existing checks](../../tools/rust/bof3-audio/tests/adsr.rs) cover literal phase
traces, rate intervals, model differences, signed component injection and key or
register transitions. The opt-in corpus probe compares VAB register pairs over
250 ms held notes followed by 250 ms release. Counter units differ between
models, so it compares level and phase rather than counter values.

These checks do not account for game-time ADSR overrides, key scheduling, ADPCM
termination or volume controls. The
[audio specification](../specs/formats/audio.md#rust-adsr-state-and-evidence-models)
owns integration limits. SoundFont fitting remains an approximation of the
selected model; complete gameplay and hardware validation remain separate.

## Volume sweeps

The same published generator describes signed stereo sweeps using direction,
phase, exponential mode and seven rate bits. Inverted linear steps reverse
direction; exponential decrease computes its step independently of phase, but
phase still selects the saturation range. Fixed-to-sweep writes require a
hardware delay that the frame-driven Rust component does not establish.

The [Rust sweep component](../../tools/rust/bof3-audio/src/machine/spu/volume.rs)
preserves current gain on sweep setup, resets its counter, stops at the relevant
endpoint and clocks gains after applying them to output. Published current-main-volume
writes change level without rearming a stopped sweep; current voice-volume writes
remain rejected. Published-model voice sweeps clock per requested frame, leaving
inactive-voice timing unvalidated.

The [native voice-volume decoder](../../third_party/pcsx-redux-agentic/src/spu/volume.cc)
approximates a sweep-mode write as a fixed level. It does not run the hardware
sweep envelope, and the native register handler leaves main-volume writes
unimplemented. `VoiceVolume` selects this fixed approximation for native-reference
voices through the configured sample model. Gain remains fixed between writes,
including sweep writes. Native stereo products use signed Q14 levels and divide
by `0x4000` toward zero. Published voice gain clips its input to `i16`, applies
its Q15 level with an arithmetic shift and clips the result. Stereo sends remain
`i32` through accumulation; capture remains separately clipped to `i16`.

`VoiceVolume::level()` exposes a Q15 component level for both policies, twice
the native Q14 level. It does not establish native current-volume bus readback.
The separate `Volume` component retains its Q15 counters, selected threshold/floor
rules and frozen `0x7F` rate for Published voice sweeps and Published main gain.
Native-reference final output applies fixed Q15 main gain, and sweep writes
retain the current level. The standalone `adsr::Model::EmulatorReference`
rate policy governs envelope steps. Bus timing, hardware fidelity and full PCM
remain separate obligations.

[Implementation and validation](../specs/formats/audio.md#rust-spu-volume-sweeps)
retain these boundaries. The independent integer-vector hashes include 512
little-endian signed samples followed by a little-endian `u32` counter and one
active-state byte for each mode/rate/start context:

- Published: `a2f5dc635564de06a2b8554ade8d5ce234098c58d1c70c4de181687a1d989316`.
- Q15 EmulatorReference rate arithmetic: `7388ac278502c579d5758840515eeee52682d7e05e4b63890527f5924a85635e`.

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
