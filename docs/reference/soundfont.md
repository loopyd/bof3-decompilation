# SoundFont research reference

Research checked 2026-09-24 UTC for the
[Rust audio migration](../plans/bof3-audio-rust-migration.md).
The inspected primary document is Creative/E-mu's
[SoundFont Technical Specification, version 2.04](https://www.synthfont.com/sfspec24.pdf),
hosted by SynthFont. The cover identifies 2.04; inherited page footers sometimes
say 2.01. This reference describes the 16-bit subset emitted by the repository,
not every SoundFont extension.

## Specification locations and retained rules

Physical PDF pages 12–16 describe RIFF structure; 20 describes sample storage;
21–29 describe preset/instrument/sample tables; 30–38 define generators.
The following are paraphrased research notes, not a reproduced specification.

- RIFF form `sfbk` contains ordered `INFO`, `sdta`, `pdta` lists.
  Required tables connect preset bags/generators to instruments and their
  bags/generators to samples. Tables include terminal records.
- `smpl` stores little-endian signed PCM16. Each sample has 46 zero guard points.
  Sample and loop positions count points, with exclusive ends.
- Zone key/velocity ranges precede other generators. Instrument/sample links
  come last. Overlapping zones permit layering.
- Continuous looping and looping until release are distinct modes.
  Timing uses timecents; attenuation uses centibels.
- Portable sample recommendations include 48 points, 32-point loops, eight-point
  loop margins and rates 400–50000 Hz. These are distinct from mandatory framing
  validity. See sections 6–8 of the
  [inspected specification](https://www.synthfont.com/sfspec24.pdf#page=20).

## Repository implementation and limits

[soundfont/mod.rs](../../tools/rust/bof3-audio/src/soundfont/mod.rs) writes authored 2.01
banks containing mono PCM, explicitly identified presets, shared samples,
layered instrument zones, ranges, tuning, pan, attenuation, volume envelopes and
loop modes. Typed inputs and checked indices reject invalid references,
ambiguous identities and unrepresentable values. Portability diagnostics retain
requested sample/loop boundaries rather than silently padding their content.

It does not read arbitrary edited SF2 files, translate VAB envelopes
or tuning, implement custom modulators, or prove PSX-equivalent playback.
Unspecified synthesis behavior uses consumer defaults. BOF3-specific translation
and measured differences remain owned by the
[audio specification](../specs/formats/audio.md#rust-soundfont-writing).

[Six tests](../../tools/rust/bof3-audio/tests/soundfont.rs) inspect table bounds and
guard points, then load output in the independent RustySynth 1.3.6 consumer.
Synthetic playback checks shared-sample layers, pan, key/velocity limits, octave
tuning, sustained and release-only loops, and release tails. A generated format-1
MIDI drives generated SF2 presets through that consumer's sequencer, verifying
program changes and 100/200 Hz sections. This establishes writer interoperability
on synthetic data; it does not accept BOF3 bank conversion or audio fidelity.

The separate [VAB binder](../specs/formats/audio.md#vab-bank-binding-with-explicit-rendering-contexts)
now connects decoded samples, source identities, explicit gain/pan, original-routine
pitch contexts and fitted envelopes to this writer. Its synthetic interoperability
checks pass. Static gain fitting and explicit reverb policy now allow structural
construction of 842 corpus banks after
[US sample-reference resolution](../specs/formats/audio.md#us-runtime-sample-reference-resolution),
including an explicit silent-tone representation that retains original PCM.
The other 178 first failures are pitch-related; an out-of-bank sample reference
also remains unresolved behind an earlier pitch rejection.
Full real-bank playback and song export remain unaccepted. The
[envelope fit](../specs/formats/audio.md#measured-sf2-envelope-fitting) and
[tuning calibration](../specs/formats/audio.md#sf2-note-on-tuning-calibration)
retain their own approximation and unsupported-context reports.

## Gain and effects consumer differences

Checked 2026-09-24 UTC. Specification physical pages 32, 36 and 38 define
reverb generator 16 in tenths of a percent (0..1000) and attenuation generator 48
in centibels (0..1440). See the
[publisher-authored specification](https://www.synthfont.com/sfspec24.pdf#page=32).
An equal-power pan curve is an explicit fitting-model choice here, not a claim
that every compatible consumer has identical pan or modulation behavior.

Inspected the local published RustySynth 1.3.6 source, pinned to commit
`8cc11fc0b10422adb54107757f100d3d6ae0ef96`:

- [Voice setup and mixing](https://github.com/sinshu/rustysynth/blob/8cc11fc0b10422adb54107757f100d3d6ae0ef96/rustysynth/src/voice.rs)
  applies 40% of the specified initial attenuation, squares velocity and channel
  volume/expression, and combines channel/instrument pan before sine/cosine gains.
- [Region generator conversion](https://github.com/sinshu/rustysynth/blob/8cc11fc0b10422adb54107757f100d3d6ae0ef96/rustysynth/src/region_pair.rs)
  converts reverb units to percent; voice setup subsequently multiplies by 0.01.
  The complete path therefore uses the specified reverb-send scale. Reading only
  the first conversion incorrectly suggests saturation at generator 10.
- [Synthesizer mixer](https://github.com/sinshu/rustysynth/blob/8cc11fc0b10422adb54107757f100d3d6ae0ef96/rustysynth/src/synthesizer.rs)
  omits a channel block when both previous/current gains are below 0.001.

Synthetic PCM tests confirm the attenuation model and half/full reverb scaling.
The [static gain mapping](../specs/formats/audio.md#sf2-static-gain-and-reverb-mapping)
records both model predictions, normalization, reference controllers and limits.
No dependency source was modified; these findings do not establish PSX reverb
equivalence or complete song fidelity.

## Reader research and preservation

Rechecked the publisher-authored [2.04 specification](https://www.synthfont.com/sfspec24.pdf)
on 2026-09-24. Physical page 20, section 6.2 pairs each PCM16 word with the low
byte in `sm24`; the latter payload includes an extra byte when needed for even
length. Physical pages 21–29, sections 7.2–7.10 define monotonic table indices,
terminal counts, first global zones and sample links. Terminal payloads are
conventional values, not identities to invent or silently normalize. Duplicate
sample/instrument names must not cause records to disappear. Physical page 45,
section 8.5 distinguishes instrument absolute generators from additive preset
generators; reading raw records does not implement that synthesis model.

[The Rust reader](../../tools/rust/bof3-audio/src/soundfont/reader/mod.rs) retains raw
generators/modulators, linked-sample metadata and original file bytes, and exposes
exact signed PCM24 values. Its strict framing, resource limits and rejection policy
are documented in the [audio spec](../specs/formats/audio.md#rust-soundfont-reading).
Unknown data is retained, not interpreted as a supported BOF3 edit. This provides
the input layer for reconstruction; semantic SF2-to-game inversion remains open.

## Root-range and initialized-pitch checks

The SF2 root key is part of the playback-rate encoding, not an obligation to
copy a VAB tone's center byte. For each existing single-key zone, the converter
keeps that preferred root when possible and otherwise chooses the nearest legal
root with representable coarse/fine generators. Original center/shift stay in
preservation metadata; combined quantized cents stay unchanged. The actual
key-121/center-46/shift-114/register-192 case uses root 53, coarse -120 and fine
-98. Independent RustySynth frequency measurement covers this extreme encoding.
See [tuning calibration](../specs/formats/audio.md#sf2-note-on-tuning-calibration).

The initialized-state probe at
[`tuning_init_probe.rs`](../../tools/rust/bof3-audio/examples/tuning_init_probe.rs)
uses the exact US BIOS/executable and original BGM000 bank preparation, then
runs 30,119 distinct corpus key/center/shift contexts with isolated synthetic
tone records. It observes the original LHU instruction's effective address/value
and compares the original return against EXE-only calibration. Both 386-byte
pitch tables retain SHA-256
`293278b74970e97b814ab68b63edf21d4dcdc6630bd5394fce250aec6cd955b2`.
The following adjacent halfwords differ after initialization:

| US address | Full EXE offset | EXE value | Initialized value | Distinct lookup contexts |
| --- | --- | --- | --- | --- |
| `0x80184440` | `0xEE440` | 60 | 5 | 14 |
| `0x80184450` | `0xEE450` | 0 | 1 | 15 |
| `0x80184452` | `0xEE452` | 127 | 0 | 6 |

Fourteen returns change: ten from 1 to 0 and four from 3 to 0. Eight changed
returns read `0x80184440`; six read `0x80184452`. This demonstrates why uninitialized
adjacent bytes cannot define unconditional SoundFont pitches. It does not prove
those initialized values remain invariant under active scheduling or all game
contexts. Adjacent-data contexts remain explicit conversion failures; reads
inside either verified pitch table are covered by the follow-up below. Zero-step
rows need the separate opt-in policy below.

Receipts: `out/audio-migration/tuning-initialized-probe.json` (before root fix),
`tuning-initialized-probe-after-sources.json` (observed initialized loads, source
identities and alternate-root mapping), and `tuning-root-*.log`. BIOS, executable,
archive hashes and original initialization calls are included in the latter probe.
No proprietary payload is included in source or packaged references.

Validation: 339 default Rust tests pass (116 media checks ignored); all 15
focused tuning/bank/pitch-packing checks pass with media, including the complete
1,020-bank audit. RustySynth parsing and retained-source reconstruction remain
covered for 842 constructible banks. Clippy, Rust 1.88 all-target checks, formatting,
references, plan parsing and all 15 harness/package checks pass. The initial
corpus assertions correctly detected changed failure counts; revised expectations
retain all 178 unsupported banks and pass. Logs use `out/audio-migration/tuning-root-*`.

## Earlier-table playback snapshot

The extended original-runtime probe covers 1,700,000 BGM004 sequence-0 output
frames, then 30,119 distinct corpus pitch inputs in an IRQ-isolated snapshot.
It observes 78 table reads, no CPU table-store attempts, unchanged hashes for
both 193-entry tables, and the same three changed adjacent halfwords/14 changed
returns as the initial-state probe. The completed receipt is
`out/audio-migration/earlier-pitch-active-rom-observer.json`; it observes both
RAM and immutable BIOS-ROM instructions. Failed diagnostic
attempts are retained separately. See [addresses, scope and controls](../specs/formats/audio.md#verified-earlier-table-reads).

The converter now verifies both table hashes and supports witnessed reads inside
either table. It retains signed source indices and actual lookup identities.
The 12,623 earlier-table rows become representable; 4,976 corpus rows still fail.
Adjacent runtime data remains rejected. Complete-bank totals stay 842 strict and
843 with stopped-pitch approximation because later keys expose those remaining
failures. This is bounded original-execution evidence, not a hardware PCM oracle
or unconditional claim about every gameplay state.

## Stopped-pitch approximation

The [zero-step evidence](spu-samples.md#zero-step-playback) permits a narrow
key-on silence approximation, not a universally muted tone. With explicit
`allow_stopped_pitch_approximation`, the bank converter maps known in-table
zero-step keys whose first interpolated value is zero to a shared silent SF2
sample. Original PCM and identities remain in the font and preservation XML.
Other keys retain ordinary tuning. Unknown lookup regions and nonzero held
values still fail. Music extraction enables this only through its existing
`--allow-approximations` contract and reports affected keys.

Later pitch changes can activate the original source while a static SF2 silence
zone stays silent. A media-backed test executes the supplied US BIOS and original
runtime with an edited BGM000 fixture: key 108, center 60, shift 0, bend range
12 semitones and a held envelope. No bend gives peak 0; a downward bend produces
peak 6977, with first audible frame 11715. An upward octave remains zero through
16-bit pitch wrapping, so it cannot demonstrate activation. The trace probe
records original pitch calls and SPU writes in
`out/audio-migration/zero-runtime-probe-downward.json`. This verifies behavior
within the Rust machine, not independent hardware PCM or timing fidelity.

[Packing tests](../../tools/rust/bof3-audio/tests/bank/) preserve unchanged
VH/VB bytes and a complete edited-fixture EMI through CLI extraction/repacking.
[Stopped-tone pitch/reassignment edits](../../tools/rust/bof3-audio/tests/voice/tuning.rs), [synthetic-silence edits, and source edits
whose re-encoded first sample changes the held value](../../tools/rust/bof3-audio/tests/soundfont/packing/) reject explicitly. Later
PCM edits retaining a zero held value remain supported with encoding-loss reports.

With this opt-in, the complete 1,020-bank audit accepts 843 banks, each parsed by
RustySynth and reconstructed byte-exactly. The added bank is
`BIN/PLCHAR/PL034.EMI#entry=0`, with four stopped-key rows. Forty of the previous
41 first zero-pitch failures expose later table failures, leaving 177 unresolved
banks. Strict conversion remains 842 accepted / 178 rejected. See
`out/audio-migration/zero-pitch-final-tests.log` and the pinned corpus probe;
these counts do not establish full music conversion or rendering acceptance.


## Empty VAB program interchange

The BOF3 exporter now represents all 128 program IDs, including zero-tone slots,
with explicit presets and percussion aliases. Empty presets use a separate
nonlooping zero sample. A compatible consumer can allocate a short silent voice;
the original US game allocates none. This is a documented approximation, not
GM timbre substitution. The channel defaults and note-on result are established
by the [game runtime evidence](../specs/formats/audio.md#initial-channel-programs-and-editable-empty-programs).

An empty preset can be populated by assigning existing VAB tone instruments,
with matching percussion-alias assignments. The Rust inverse validates layers,
source program controls, sample assignments and allocation capacity. Creating
new independent instrument/sample structures is not yet supported. XML records
the current policies; older export reconstruction is intentionally absent.
