# SPU sample playback references

Inspected 2026-09-24. These describe hardware or emulator behavior; original-game
execution and integration evidence live in the
[audio specification](../specs/formats/audio.md#rust-spu-sample-and-voice-execution).

| Source | Use | Authority limit |
| --- | --- | --- |
| [psx-spx SPU sample and pitch reference](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-adpcm-samples) | Block flags, address units, pitch modulation and interpolation arithmetic | Published descriptions require independent hardware validation. |
| [Interpolation ROM values](https://psx-spx.consoledev.net/soundprocessingunitspu/#4-point-gaussian-interpolation) | Numeric hardware coefficient table | Reproducing the table does not validate fetch timing or the final mixer. |
| [DuckStation SPU implementation](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/spu.cpp) | Independent comparison of coefficients, rounding, pitch ceiling and voice state | A moving branch inspected on the date above, not an immutable hardware oracle. No implementation code or dependency is imported. |

All 512 numeric coefficients agree between the inspected sources. Encoded as
little-endian signed 16-bit values, their SHA-256 is
`3e221a5b25de34ca9ad1ea945ad90fbe8171c5e3f7c52af15b232f9c36fbd40d`.
The [Rust ROM data](../../tools/rust/bof3-audio/src/machine/gaussian.rs) preserves
those values literally. Replacing them with a mathematical Gaussian approximation
would change the output.

Two unresolved arithmetic differences matter:

- Published interpolation shifts each product before summing; the inspected
  emulator shifts the sum. At phase zero, four input samples equal to one yield
  negative one versus zero.
- The published pitch limit is `0x4000`; the inspected emulator limits to
  `0x3FFF`. This changes both phase accumulation and block-boundary timing.

[The Rust reader](../../tools/rust/bof3-audio/src/machine/spu_sample.rs) requires an
explicit source model. Its shared block-snapshot and retrigger state follows the
inspected emulator comparison; neither model claims complete hardware behavior.
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
inspected DuckStation source instead uses a fractional timer with different
increments and thresholds. This discrepancy is retained as an unresolved
hardware-validation obligation, not silently reconciled.

The [same SPU register reference](https://psx-spx.consoledev.net/soundprocessingunitspu/)
documents fixed signed main gain, independent CD/external/reverb gains, routing
flags and the four capture rings. The inspected emulator provides an ordering
cross-check: noise before envelope, mute on summed voices, enabled CD input after
voice mute, reverb return before final clipping/main gain, and capture of CD before
SPU input gain plus voices 1/3 after ADSR. No emulator code is imported. In
particular, the `-32768 × -32768` final-gain edge produces an intermediate
`+32768`; wrap versus saturation is not established here, so Rust rejects it.

The [implementation and checks](../specs/formats/audio.md#rust-spu-noise-mixing-and-capture)
are explicitly scheduled component evidence. They do not establish original boot
phase, disabled-SPU transitions, bus timing, CD resampling or
accepted PSX audio fidelity.

## Reverb

The [published register/network description](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-reverb-formula)
and [numeric presets/filter](https://psx-spx.consoledev.net/soundprocessingunitspu/#reverb-buffer-resampling)
were inspected 2026-09-24. The page distinguishes algebraic network behavior from
unresolved bit-level rounding and reports SCPH-1001 measurements; those reports
are external evidence, not captures independently reproduced in this project.
The 39 coefficients sum to 32766. The twenty even-index coefficients sum to
16382, so one upsampling phase is slightly below unity gain.

The previously linked DuckStation source supplies a comparison for intermediate
quantization, saturation, phase scheduling, aliased channel ordering and numeric
edge cases. Its address fold uses one conditional base addition, allowing some
negative/large offsets below the nominal work area. That is not the same as
modulo by work-area size implied by a strict reading of the published buffer
description. Rust retains this as an explicit source model, with a regression
vector for that alias; original hardware behavior remains unresolved.

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
[inspected DuckStation source](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/spu.cpp)
also clears interpolation history at key-on and retains a zero step; this is
behavioral corroboration, not copied implementation or independent hardware
validation. PCSX-Redux commit `28438546c781fbe372a06399c82bed43ca2c6f4d`,
`src/spu/spu.cc`, instead substitutes a minimum increment of one and starts its
Gaussian reader with three sample advances. It is not an exact zero-step oracle.

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

The [inspected DuckStation control-write implementation](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/spu.cpp)
handles a falling SPUCNT enable bit by forcing active envelopes off. Its comment
attributes immediate rather than next-frame behavior to hardware tests, but this
project has not reproduced those measurements. Its `ForceOff` leaves an
already-off voice unchanged, including a manually written ENVX. Pending key
latches, sample state, gains and ENDX are not reset by that transition.

Rust exposes this as an explicit emulator-reference choice for control writes;
disabled-frame execution remains unsupported. Original low-level initialization
can now run repeatedly, with [bounded execution evidence](../specs/formats/audio.md#rust-spu-disable-and-original-hardware-initialization).
This does not verify boot state, write latency, disabled capture/CD behavior or
the full game initializer.
