# Standard MIDI Files research reference

Research checked 2026-09-24 UTC for the
[Rust audio migration](../plans/bof3-audio-rust-migration.md).
These authored notes retain source provenance and implementation decisions;
the linked publisher documents remain the specification authority.

## Sources and locations

- [MIDI Association SMF specification page](https://midi.org/standard-midi-files-specification):
  official discovery point for RP-001, Standard MIDI Files 1.0. Its download is
  named `RP-001_v1-0_Standard_MIDI_Files_Specification_96-1-4.pdf`.
  The linked Google Drive viewer did not expose the document during research;
  its contents were not assumed to match a separately retrieved file bytewise.
- [MIDI Association SMF overview](https://midi.org/standard-midi-files):
  publisher's introductory companion page.
- [MMA complete MIDI 1.0 specification, v96.1, third edition](https://www.freqsound.com/SIRA/MIDI%20Specification.pdf):
  inspected publisher-authored document hosted by FreqSound. This is historical
  MIDI 1.0 documentation, not a claim about the latest MIDI specifications.
  The SMF section identifies RP001, revised February 1996. The copyrighted PDF
  is linked rather than reproduced here.

Locations below use one-based physical PDF pages, not printed section pagination:

| Topic in the inspected PDF | Physical pages |
| --- | --- |
| SMF title, revision and introduction | 128–130 |
| Variable-length quantities and chunks | 131–132 |
| Header, division and formats | 133–134 |
| Track messages, SysEx and meta framing | 135–136 |
| Meta-event definitions | 137–139 |
| Format-1 example and timing discussion | 142–143 |

## SMF rules used by the codec

Compact paraphrase of the
[SMF section](https://www.freqsound.com/SIRA/MIDI%20Specification.pdf#page=131):

- Chunks: four-byte ID, big-endian u32 payload length; no alignment padding.
  `MThd` comes first: three big-endian u16 fields, format/count/division;
  honor extensions beyond six bytes. Tracks use `MTrk`.
- Formats: 0 = one multichannel track; 1 = synchronous tracks;
  2 = independent patterns. Division selects PPQN or SMPTE
  (−24/−25/−29/−30; −29 means drop-frame), then ticks/frame.
- Each event has a delta, including zero. VLQs use seven-bit groups,
  continuation bit 7, maximum four bytes (`0x0fffffff`).
- Channel statuses `80`–`EF`: `C`/`D` take one data byte, others two;
  data is seven-bit. Meta/SysEx cancel running channel status.
- Meta: `FF type length payload`; type <128. Honor unknown types and extended
  lengths. `FF 2F 00` terminates the track.
- SysEx: `F0 length payload`; `F7` frames continuation or escape data.
  Packet framing does not imply a complete message.
- Tempo: `FF 51 03`, microseconds/quarter; default 500000. Format-1 tempo map:
  first track. Time signature: `FF 58 04`, numerator/log2-denominator/
  metronome-clocks/32nds-per-quarter; default 4/4.

## MIDI channel messages and controller assignments

Additional primary sources checked 2026-09-24 UTC:

- [MIDI Association message summary](https://midi.org/summary-of-midi-1-0-messages),
  table 1: channel-message layouts and pitch-bend representation.
- [MIDI Association controller summary](https://midi.org/midi-1-0-control-change-messages),
  tables 3 and 3a: controller assignments and registered parameters.

Both are publisher overviews, not replacements for the complete specification.
The following compact lookup is paraphrased from those tables. Status bytes are
hexadecimal; controller numbers and data values are decimal. `n` is the encoded
channel 0–15, conventionally displayed as channels 1–16.

| Status | Data bytes | Meaning |
| --- | --- | --- |
| `8n` | key, velocity | Note off |
| `9n` | key, velocity | Note on |
| `An` | key, pressure | Pressure for one key |
| `Bn` | controller, value | Control or channel-mode change |
| `Cn` | program | Program selection, encoded 0–127 |
| `Dn` | pressure | Channel pressure |
| `En` | low, high | Bend: `low + (high << 7)`; center 8192 |

Bend sensitivity belongs to the receiver's configuration; the message alone
does not specify a semitone range. See the
[message summary](https://midi.org/summary-of-midi-1-0-messages).

| Controller(s) | Assigned function |
| --- | --- |
| 0 / 32 | Bank selection MSB / LSB |
| 1 / 33 | Modulation MSB / LSB |
| 7 / 39 | Channel volume MSB / LSB |
| 10 / 42 | Pan MSB / LSB |
| 11 / 43 | Expression MSB / LSB |
| 64 | Sustain pedal: 0–63 off, 64–127 on |
| 91 / 93 | Effects depth; defaults are reverb / chorus sends |
| 98 / 99 | NRPN selection LSB / MSB |
| 100 / 101 | RPN selection LSB / MSB |
| 6 / 38 | Data entry MSB / LSB for the selected parameter |
| 120 / 121 / 123 | All sound off / reset controllers / all notes off |

RPN `(MSB=0, LSB=0)` selects bend sensitivity; data entry supplies semitones
and cents. These assignments come from the
[controller summary](https://midi.org/midi-1-0-control-change-messages), not BOF3.

### Repository application and limits

The [PC renderer](../../tools/rust/bof3-audio/src/pc_render.rs) accepts notes,
program changes, bends and a checked subset of controllers. Its current bank
selection uses CC0; CC32, pressure, RPN/NRPN and SysEx playback are rejected.
The parser can preserve messages that the renderer cannot execute. The
[rendering specification](../specs/formats/audio.md#rust-pc-interchange-rendering)
owns the complete supported surface and timing policy.

BOF3 loop controls reuse controller numbers with different runtime semantics.
Export must translate those controls before ordinary MIDI playback. Source
program IDs and accompanying SoundFont mappings must remain explicit; standard
controller assignments do not establish game-bank identities or GM timbres.
Likewise, encoding a valid MIDI bend does not establish equivalence to the
game's tone-dependent pitch calculation.

## BOF3 evidence and translation decisions

The [audio specification](../specs/formats/audio.md#verified-us-sequence-event-framing)
owns the executable addresses, raw-state comparisons and corpus counts behind
these distinctions. They were established from the verified US executable and
local media, not inferred from the MIDI standard.

| BOF3 finding | Consequence for interchange |
| --- | --- |
| SEP has a six-byte `pQES` bundle header and independently selectable sequences with 13-byte headers. | Export each selectable sequence as its own format-1 MIDI; never merge alternatives into simultaneous tracks. |
| SEP tempo/end meta events omit SMF length fields; the runtime retains the meta running class. | Translate through separate parsers; copying SEP bytes into an `MTrk` is invalid. |
| The understood runtime paths use note, controller, program, bend and meta callbacks. | Acceptance by a general MIDI parser does not establish a representable game edit. |
| Runtime delta decoding multiplies by ten and accumulates an internal counter. | Do not multiply exported PPQN merely because internal delays use this scale. |
| Bend consumes both bytes but the verified callback uses only the high byte. | Preserve both original bytes for round trips; assess changed bend resolution explicitly. |
| Tempo becomes integer BPM through `60000000 / tempo`, followed by scheduler quantization. | Standard MIDI timing and PSX scheduling require a measured comparison. |
| Controller 99 values 20/30 select loop start/end; controllers 6/98 supply counts in the verified loop context. Count 127 repeats indefinitely. | Preserve game loop meaning explicitly; stock NRPN interpretation is insufficient. |
| Controller loops share one saved cursor/count per sequence. End-marker play count has separate restart/stop behavior. | Distinguish controller loops, complete-sequence repetition and sample loops. |

See the authoritative [loop transitions](../specs/formats/audio.md#sequence-controller-loop-transitions)
and [end/scheduler evidence](../specs/formats/audio.md#sequence-end-restart-and-per-tick-scheduling)
before implementing expansion, duration or loop-count policies. Hardware cadence
and audible fidelity are still separate acceptance gates.

The [forward SEP translator](../specs/formats/audio.md#sep-execution-order-midi-translation)
now uses a conductor track plus an ordered performance track. Its execution-order
timeline handles live running status after loop jumps and records an explicit
finite boundary for infinite loops. This is separate from the generic codec;
song-folder publication, bank binding and reconstruction remain unfinished. The
migration contract prioritizes playback with the accompanying BOF3 SoundFont;
stock GM timbres are not the fidelity target.

## Reusable timing example and implementation map

This repository-authored example matches the two-track fixture in
[the MIDI tests](../../tools/rust/bof3-audio/tests/interchange/midi.rs). At 96 PPQN,
put tempo 500000 microseconds/quarter at conductor tick 0 and tempo 1000000
at tick 96. End both tracks at tick 192. The first quarter lasts 0.5 seconds,
the second 1 second: total 1.5 seconds. Changing the second tempo to 250000
reduces the total to 0.75 seconds. For each interval with constant tempo:

```text
seconds = delta_ticks * microseconds_per_quarter / (PPQN * 1_000_000)
```

Accumulate intervals across tempo changes; do not apply the final tempo to the
whole song. This is interchange timing, not a measurement of PSX scheduler
cadence. The fixture exercises program changes, same-tick note ordering and
bend encoding as well as tempo. Its independent consumer check measures MIDI
duration; it does not establish game playback fidelity.

| Work to resume | Owning source |
| --- | --- |
| SMF parsing, preservation and edited serialization | [interchange/midi.rs](../../tools/rust/bof3-audio/src/interchange/midi.rs) |
| Game sequence execution order and finite loop expansion | [sequence/timeline.rs](../../tools/rust/bof3-audio/src/sequence/timeline.rs) |
| SEP-to-format-1 translation | [sequence/midi.rs](../../tools/rust/bof3-audio/src/sequence/midi.rs) |
| Accepted playback events and sample-frame timing | [pc_render.rs](../../tools/rust/bof3-audio/src/pc_render.rs) |
| Accompanying instrument format and synthesis differences | [SoundFont reference](soundfont.md) |

Keep publisher rules, observed BOF3 behavior and current implementation limits
distinct when extending these notes. The online specification and message-table
links above were rechecked on 2026-09-24 UTC. This file retains the research
summary for offline use; it does not embed the copyrighted specification PDF.

## Current Rust implementation and verification

[interchange/midi.rs](../../tools/rust/bof3-audio/src/interchange/midi.rs) owns framing and preservation;
[the audio spec](../specs/formats/audio.md#rust-midi-interchange) owns its public
scope. As of this research checkpoint:

- Formats 0/1 with PPQN 1–32767 are supported. Format 2, SMPTE timing and zero
  PPQN are rejected explicitly.
- Parsed events retain absolute ticks and order. Unchanged tracks reuse exact
  original bytes, including running status and noncanonical VLQs. Edited tracks
  use explicit statuses and canonical VLQs; untouched tracks, header extensions
  and opaque chunks retain their original encoding and placement.
- Unknown meta and SysEx payloads are preserved. The codec does not validate
  metadata meaning, conductor-track placement, multipart SysEx protocol or
  BOF3 edit representability. Those validations must precede production playback
  or packing; successful parsing is not musical acceptance.
- Invalid chunk bounds, track counts, channel payloads, end markers, backward
  ticks and excessive deltas fail. Changing track count requires constructing a
  new file explicitly rather than silently reusing an incompatible layout.

[Five MIDI tests](../../tools/rust/bof3-audio/tests/interchange/midi.rs) cover exact unchanged
preservation, isolated edits, malformed inputs, all channel-message shapes,
VLQ boundaries and opaque metadata. The independent RustySynth 1.3.6 MIDI reader
accepts a synthetic two-track fixture: 96 PPQN, two quarter notes at 500000 then
1000000 microseconds/quarter gives 1.5 seconds. Editing the second tempo to
250000 gives 0.75 seconds. This checks consumer parsing and timing, not SoundFont
structure, rendered PCM or BOF3 sequence translation.

The implementation checkpoint passed 86 default tests with 44 opt-in cases
skipped. Historical logs live under ignored `out/audio-migration/`; source tests
and the [plan](../plans/bof3-audio-rust-migration.md) retain reproducible scope
and outstanding acceptance work. No proprietary media is included here.

The subsequent [PC playback path](../specs/formats/audio.md#rust-pc-interchange-rendering)
uses stricter event validation before passing messages to RustySynth. It rejects
unsupported semantic events and missing bank/program presets, accumulates rational
tempo time, preserves same-tick ordering, and reports frame/block quantization.
Whole-file repeats in that inspection command are not BOF3 controller-loop
translation. The generic MIDI codec's broader preservation support remains useful
for round trips even when a retained message is not accepted for playback.
