# SPU sample playback references

These describe hardware or emulator behavior; original-game
execution and integration evidence live in the
[audio specification](../specs/formats/audio.md#rust-spu-sample-and-voice-execution).

| Source | Use | Authority limit |
| --- | --- | --- |
| [psx-spx SPU sample and pitch reference](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-adpcm-samples) | Block flags, address units, pitch modulation and interpolation arithmetic | Published descriptions require independent hardware validation. |
| [Interpolation ROM values](https://psx-spx.consoledev.net/soundprocessingunitspu/#4-point-gaussian-interpolation) | Numeric hardware coefficient table | Reproducing the table does not validate fetch timing or the final mixer. |
| [PCSX-Redux Agentic interpolator](../../third_party/pcsx-redux-agentic/src/spu/interpolation.cc), [Gaussian table](../../third_party/pcsx-redux-agentic/src/spu/gauss.h) and [register handler](../../third_party/pcsx-redux-agentic/src/spu/registers.cc) | Current native coefficients, rounding, pitch ceiling and voice observations | Native implementation comparison requires separate runtime and hardware evidence. No emulator implementation is imported into the Rust component. |

All 512 numeric coefficients agree between the published table and the fork's
`gauss512` table. Encoded as
little-endian signed 16-bit values, their SHA-256 is
`3e221a5b25de34ca9ad1ea945ad90fbe8171c5e3f7c52af15b232f9c36fbd40d`.
The [Rust ROM data](../../tools/rust/bof3-audio/src/machine/gaussian.rs) preserves
those values literally. Replacing them with a mathematical Gaussian approximation
would change the output.

The retained Rust policies and current native arithmetic differ:

- Rust `Published` interpolation shifts each product by 15 before summing;
  `EmulatorReference` shifts the sum by 15. At phase zero, four input samples
  equal to one yield negative one versus zero. The native Gaussian interpolator
  shifts each product by 14, then shifts the sum by one, retaining a guard bit.
- The published pitch limit is `0x4000`; Rust `EmulatorReference` and the native
  pitch register handler limit to `0x3FFF`. This changes both phase accumulation
  and block-boundary timing. Native zero-step behavior also differs as described
  below.

[The Rust reader](../../tools/rust/bof3-audio/src/machine/spu_sample.rs) requires an
explicit arithmetic model. Its shared block-snapshot and retrigger state is a
scheduled component policy. Neither model reproduces the full native reader or
claims complete hardware behavior.
The existing [ADSR source differences](spu-envelopes.md) remain independently
selectable. A future accepted PSX renderer must resolve these discrepancies from
independent traces/audio, rather than silently choose whichever source passes.

The frame-driven component exposes scheduled operations; it does not reproduce
SPU bus arbitration, key-on delay, minimum retrigger spacing, first-block manual
repeat-address races, inactive-voice fetches or IRQ comparison timing. Per-voice
outputs and reverb sends are intermediate signals. The bounded output-frame
component described below now supplies noise, mixing and capture;
[sweeps](spu-envelopes.md#volume-sweeps) and the explicitly selected
[reverb model](#reverb) are also connected. Drive-side CD execution and hardware
timing remain required.

## Noise, mixing and capture

The [published noise algorithm](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-noise-generator)
uses feedback bits 15, 12, 11 and 10, XORed with one. Each output frame subtracts
`4 + step` from a shared countdown; underflow shifts the level once and adds
`0x20000 >> shift`, twice if needed. SPUCNT bits 8–9 select step and 10–13 select
shift. The Rust implementation requires caller-supplied initial state. The
[native noise generator](../../third_party/pcsx-redux-agentic/src/spu/noise.cc)
uses a fractional accumulator with different increments and thresholds. This
discrepancy remains an unresolved hardware-validation obligation.

The [same SPU register reference](https://psx-spx.consoledev.net/soundprocessingunitspu/)
documents fixed signed main gain, independent CD/external/reverb gains, routing
flags and the four capture rings. The
[native mixer](../../third_party/pcsx-redux-agentic/src/spu/spu.cc) applies noise
before ADSR, captures voices 1/3 after ADSR and before voice gain, and adds reverb
before output clipping. CD audio enters a separate stream through
[FeedXA](../../third_party/pcsx-redux-agentic/src/spu/xa.cc). The native register
handler leaves main and external input gain writes unimplemented, so this
comparison does not validate the Rust gain/routing model. The
`-32768 × -32768` final-gain edge produces an intermediate
`+32768`; wrap versus saturation is not established here, so Rust rejects it.

The [implementation and checks](../specs/formats/audio.md#rust-spu-noise-mixing-and-capture)
are explicitly scheduled component evidence. They do not establish original boot
phase, disabled-SPU transitions, bus timing, CD resampling or
accepted PSX audio fidelity.

## Reverb

The [published register/network description](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-reverb-formula)
and [numeric presets/filter](https://psx-spx.consoledev.net/soundprocessingunitspu/#reverb-buffer-resampling)
distinguish algebraic network behavior from
unresolved bit-level rounding and report SCPH-1001 measurements; those reports
are external evidence, not captures independently reproduced in this project.
The 39 coefficients sum to 32766. The twenty even-index coefficients sum to
16382, so one upsampling phase is slightly below unity gain.

The [native reverb unit](../../third_party/pcsx-redux-agentic/src/spu/reverb.cc)
supplies the current comparison for quantization, saturation and alternating
left/right scheduling. Its `wrapOffset` repeatedly folds offsets at the work-area
boundaries. The [Rust reverb model](../../tools/rust/bof3-audio/src/machine/spu_reverb.rs)
retains one conditional base addition, allowing some negative/large offsets
below the nominal work area. This differs from the native fold and from strict
modulo by work-area size. The existing alias vector preserves that diagnostic
policy; original hardware behavior remains unresolved.

[Implementation evidence](../specs/formats/audio.md#rust-spu-reverb-execution)
includes exact Room preset identity in the supplied US executable, separate
integer PCM/RAM vectors and shared-memory output tests. It does not establish
game preset selection, hardware bit accuracy, initial filter phase or bus timing.

## Zero-step playback

The [published pitch and interpolation model](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-adpcm-pitch)
adds the pitch step to the sample-position counter. A zero step freezes that
position; it does not inherently mute the voice. With three cleared history
samples at key-on, phase-zero Gaussian interpolation gives
`floor(-first_decoded_sample / 32768)`. This is -1 for positive first samples,
+1 for -32768, and zero for the remaining nonpositive values. Setting zero
after playback has advanced can hold another value.

[Reader tests](../../tools/rust/bof3-audio/tests/spu_samples.rs) cover both Rust
sample models, initial/frozen positions, fetch counts and history reset. The
current [PCSX-Redux Agentic interpolator](../../third_party/pcsx-redux-agentic/src/spu/interpolation.h)
substitutes a minimum increment of one in `setStep`
and starts its Gaussian reader with three sample advances in `keyOn`.
It is not an exact zero-step oracle.

The [corpus probe](../../tools/rust/bof3-audio/examples/zero_pitch_probe.rs)
resolves 311 in-table zero-step tone/key rows across 219 tones in 100 banks.
All hold zero in both Rust readers; 217 tones have nonzero maximum ordinary
sequence gain, and two are gain-muted. Two additional rows, keys 108/120 in
`BIN/BPLCHAR/DRG04_00.EMI#entry=0`, program 2/tone 0, remain unresolved because
sample reference 3 exceeds the declared two samples. No adjacent SPU bytes are
invented. The receipt `out/audio-migration/zero-pitch-probe-pinned.json` retains
source identities and archive hashes. See the explicit
[SoundFont approximation](soundfont.md#stopped-pitch-approximation) for its limits.

## Disable transition

[Rust control writes](../../tools/rust/bof3-audio/src/machine/spu_voice_ports.rs)
retain an explicit `EmulatorReference` policy that forces active envelopes off
when SPUCNT enable falls. Already-off voices retain manually written ENVX;
pending key latches, sample state, gains and ENDX remain unchanged. The
[native control-write handler](../../third_party/pcsx-redux-agentic/src/spu/registers.cc)
updates control/noise state and IRQ acknowledgement without that forced-off
transition. ENVX reflects the envelope steps elapsed at the reader's cycle.
These differences require
focused native and hardware evidence.

The retained Rust policy does not establish native control-write behavior;
disabled-frame execution remains unsupported. Original low-level initialization
can now run repeatedly, with [bounded execution evidence](../specs/formats/audio.md#rust-spu-disable-and-original-hardware-initialization).
This does not verify boot state, write latency, disabled capture/CD behavior or
the full game initializer.
