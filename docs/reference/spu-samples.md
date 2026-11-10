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
The [Rust ROM data](../../tools/rust/bof3-audio/src/machine/spu/coefficients.rs) preserves
those values literally. Replacing them with a mathematical Gaussian approximation
would change the output.

Rust exposes two interpolation arithmetic policies:

- `Published` shifts each product by 15 before summing. `EmulatorReference`
  shifts each product by 14, then shifts the sum by one, matching the current
  native scalar arithmetic. At phase zero, four input samples equal to one
  yield negative one and zero respectively.
- The published pitch limit is `0x4000`. Rust `EmulatorReference` and the native
  pitch handler clamp the raw register to `0x3FFF`; native modulation converts
  that value to a separate 16.16 step. Phase accumulation and block timing can
  differ. Zero-step behavior is described below.

[The Rust reader](../../tools/rust/bof3-audio/src/machine/spu/sample.rs) requires an
explicit arithmetic model. Its block snapshots and scheduled retriggers remain
component policies. Neither model reproduces the full native reader or
claims complete hardware behavior.
Both reader models align start addresses to 16-byte blocks. Native-reference
manual repeat writes also align the address and retain explicit validity,
including address zero. An accepted native MMIO KON write clears loop-start
protection immediately; scheduled playback start preserves any later manual
repeat write. Start retains the repeat address and validity while resetting
decoder and interpolation state. Native
ENDX occurs at block fetch. Only exact flags `0x03` with a valid repeat select
looping. Other supported end-marked blocks finish their buffered samples before
a parked next cursor produces `Frame::stopped`. The voice owner then suppresses
noise and forces the component envelope off before output. Published behavior is
unchanged.

Native MMIO accepts KON only after the sample-start register has been written;
written zero is valid. Accepted KON/KOFF requests survive later zero and
other-voice writes. KON clears ENDX at acceptance; native ENDX writes are
ignored and reads expose live flags. Scheduled start uses the latest start
register. Published MMIO retains its latest-mask keys and temporary ENDX
readback; standalone `Voices::apply_keys()` remains atomic. These policies do
not establish CPU/worker phase or full runtime PCM.

Native-reference voices require an explicit immutable `i32` key-on delay.
Negative values act as zero delay. Owned and reference production profiles use
six output frames; Published behavior is unchanged. During the delay, output,
capture and modulation-source output are zero. Sample consumption, interpolation
and ADSR remain frozen. Pending frequency changes install before the delay
check, while shared noise keeps advancing. Key-off retains release state during
the delay, and retriggering resets the countdown.

`Player::current_address()` returns the next native decode cursor or published
block address as `Option<u16>`. A parked native cursor can still have buffered
samples. These component rules do not establish complete native ADSR stop state,
bus timing, native batching, initial-state provenance, IRQ latency or hardware
fidelity.

After a native-reference envelope enters `Off`, a previously started voice
continues consuming and decoding its stream with the last installed pitch step.
It discards samples without changing the interpolation window, sampling ADSR or
sending audio. Pending pitch and modulation writes do not alter the silent walk.
Fetches still update repeat and ENDX metadata and reach the interconnect's IRQ
comparator. A parked end cursor stays parked; never-started voices remain
stationary. The frame that enters `Off` completes its ordinary processing.
This uses the selected envelope's phase. Native `On` correspondence is qualified
only with native-reference ADSR, and native worker batching remains unresolved.

Its `Published` decoder clips each sample and retained history to `i16`.
The native-reference decoder retains raw signed 32-bit samples and history,
matching the [native ADPCM class](../../third_party/pcsx-redux-agentic/src/spu/adpcm.cc).
`Player::history()` exposes
these as separate `DecoderState` variants. Unsupported headers and arithmetic
overflow reject before history changes. Native ordinary playback clamps or mutes
new samples when storing them in the interpolation window. Source playback
returns the raw signal without Gaussian interpolation, clipping or store-time
muting. Role changes retain the shared native window/source storage and
sample-consumption cursor. `Player::tick(ram, preceding_voice, signal)` requires
`Signal::Ordinary { unmuted }` or `Signal::ModulationSource`; published readers
reject the latter. `Frame::sample` is `i32`.
The existing [ADSR source differences](spu-envelopes.md) remain independently
selectable. A future accepted PSX renderer must resolve these discrepancies from
independent traces/audio, rather than silently choose whichever source passes.

The [pitch owner](../../tools/rust/bof3-audio/src/machine/spu/pitch.rs) retains
published signed pitch interpretation and low-16-bit wrapping before clamping.
Native-reference pitch uses the clamped raw register and a supplied signed
post-envelope source. Nonzero input scales raw pitch by `(32768 + source) / 32768`
with division toward zero, then clamps it to `1..=0x3FFF`. Integer frequency is
`44100 * pitch / 4096`; the 16.16 step is `((frequency / 10) << 16) / 4410`, with
a minimum of one. Zero or absent input retains the installed step after pending
pitch writes. Key-on preserves pitch state. `Player::write_pitch()` applies an
already scheduled write, including repeated values; `Player::tick()` consumes
that state. Native-reference PMON roles follow each ordered halfword write.
A source supplies modulation and is excluded from dry output and reverb sends,
while its capture signal remains available. Native source voices supply their
wide post-envelope signal to target pitch calculation. Capture clips that signal
to `i16`. Envelope multiplication rejects products outside the signed 32-bit
domain. Native batch boundaries, write timing and complete device behavior remain
unresolved.

The frame-driven component exposes scheduled operations; it does not reproduce
SPU bus arbitration, CPU/worker alignment of the configured key-on delay,
minimum retrigger spacing, first-block manual
repeat-address races, hardware fetch behavior or IRQ comparison timing. Per-voice
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
[FeedXA](../../third_party/pcsx-redux-agentic/src/spu/xa.cc). The native float
callback sums both streams, applies signed Q15 main gain and then folds to mono
when configured. It leaves the result unclamped. Sweep writes retain the current
main gain. Explicit host settings select volume division, mono and mute.
CD bypasses the voice and reverb path. External input is unimplemented.
PCM16 export rounds and saturates at the file boundary. The unresolved `+32768`
final-gain rail remains a Published-policy rejection.

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
supplies the current comparison for signed rounding, saturation, alternating
left/right calculations and FIR phase. The
[Rust reference model](../../tools/rust/bof3-audio/src/machine/spu/reverb.rs)
keeps sends and interpolated returns wide, applies signed wet gain before
interpolation, and preserves immediate RAM alias ordering. Signed offsets fold
repeatedly inside the work area. The lower fold adds `0x3ffff-base`, while the
upper fold subtracts `0x40000-base`; this asymmetry differs from ring modulo.

The [native register handler](../../third_party/pcsx-redux-agentic/src/spu/registers.cc)
disables the work area for ESA values through `0x0200` and for `0xffff`.
Rewriting the same valid base preserves the cursor. Base changes preserve FIR
histories. A disabled base freezes the cursor, phase and filters, returning zero
on the left while the right can repeat a stored sample. The reference model
preserves these native quirks without claiming hardware agreement.

Native-reference voice sends latch at scheduled key-on from the voice-enable
bit and SPUCNT bit 7. Register or master-enable changes and key-off retain the
latch; retriggering refreshes it. Published sends retain their live mask. This
describes send eligibility, not reverb RAM-write enable, native CPU/batch alignment of startup delay or wet-output startup.

[Implementation checks](../specs/formats/audio.md#rust-spu-reverb-execution)
cover Room preset identity in the supplied US executable, integer PCM/RAM
comparisons and shared-memory output. A bounded native capture also checks
steady stored-RAM output with writes disabled. Game preset selection, hardware
bit accuracy, startup phase and bus timing remain unresolved.

## Zero-step playback

The [published pitch and interpolation model](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-adpcm-pitch)
adds the pitch step to the sample-position counter. A zero step freezes that
position; it does not inherently mute the voice. With three cleared history
samples at key-on, phase-zero Gaussian interpolation gives
`floor(-first_decoded_sample / 32768)`. This is -1 for positive first samples,
+1 for -32768, and zero for the remaining nonpositive values. Setting zero
after playback has advanced can hold another value.

[Reader tests](../../tools/rust/bof3-audio/tests/spu_samples.rs) distinguish the
published frozen position from native-reference progression. Both use a
block-relative 16.16 counter. The
current [PCSX-Redux Agentic interpolator](../../third_party/pcsx-redux-agentic/src/spu/interpolation.h)
substitutes a minimum increment of one in `setStep`
and starts its Gaussian reader with three sample advances in `keyOn`.
Rust `EmulatorReference` follows that priming. An explicit zero-pitch write
installs the minimum increment of one when the frequency change is consumed.
Zero or absent modulation input preserves the installed step, and key-on
preserves pitch state. For ordinary unmuted playback, its first
window contains zero followed by three decoded samples, and its block-relative
counter starts at `0x20000`. An unmodulated explicit zero-pitch write can therefore
produce sound immediately and continues to advance slowly. This differs from the published frozen model.
Direct native-handler/class comparisons cover scheduled block endings and
supplied looping blocks. They do not establish CPU/fetch alignment, complete mute
transitions, native noise timing, mixed PCM or hardware behavior.

The [corpus probe](../../tools/rust/bof3-audio/examples/zero_pitch_probe.rs)
reports published held values and native first values separately in
`bof3.zero-pitch-probe/v2` output. Its 65-frame check verifies each model's
counter policy after an explicit zero write without modulation. It does not
establish behavior after prior modulation or lasting silence from an initially
zero native value. Supply the US executable and extracted corpus root as its two arguments.
Keys 108/120 in
`BIN/BPLCHAR/DRG04_00.EMI#entry=0`, program 2/tone 0, remain unresolved because
sample reference 3 exceeds the declared two samples. No adjacent SPU bytes are
invented. Probe output retains source identities and archive hashes. See the explicit
[SoundFont approximation](soundfont.md#stopped-pitch-approximation) for its limits.

## Disable transition

[Rust control writes](../../tools/rust/bof3-audio/src/machine/interconnect.rs)
require explicit `DisableModel::EmulatorReference` selection when SPUCNT bit 15
falls with voices configured. Scheduling a voice frame while bit 15 is clear
also requires that model. Control writes
preserve active and off envelopes, manual ENVX, pending keys, sample state,
gains and ENDX. Invalid transfer/IRQ changes still reject atomically. The
[native control-write handler](../../third_party/pcsx-redux-agentic/src/spu/registers.cc)
updates control/noise state and IRQ acknowledgement without forcing envelopes
off. Bounded native observations retain an active sustained voice, sample and
capture progression, and a new key-on while bit 15 is clear and bit 14 is set.
Snapshots include elapsed device time; they do not establish write/sample latency.

Caller-scheduled Rust frames likewise consume pending keys and advance voices
and capture with bit 15 clear. The mixer adds no bit-15 gate. It retains the
published bit-14 send gate; native frames instead mute ordinary sample stores
before interpolation. Existing native window contents and noise can still reach
dry and reverb sends. CPU/write latency, batch timing and full mixed PCM remain
unresolved. Original low-level initialization can run
repeatedly, with [bounded execution evidence](../specs/formats/audio.md#rust-spu-disable-and-original-hardware-initialization).
These observations do not verify hardware disable semantics, boot state,
disabled CD behavior, complete PCM agreement or the full game initializer.
