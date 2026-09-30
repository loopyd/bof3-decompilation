---
type: Format
title: Audio formats
description: BOF3 audio subsystems — XA ADPCM streaming, VAB sample banks, SEP sequences, SPU emulation, and tooling.
tags: [formats, audio, xa, vab, sep, spu, tooling]
---

# Audio formats

BOF3 has two independent audio paths: **streaming XA ADPCM** (voice, SFX,
magic) delivered through the CD hardware, and **synthesized music** (VAB sample
banks + SEP sequences) driven by the PsyQ `libsnd`/`libspu` runtime.

## Quick start

```sh
bin/harness audio build list                        # browse all 81 BGM tracks
bin/harness audio build play BGM000                 # play by track name (auto-resolves)
bin/harness audio build play BGMBAT02 --gain 0.7    # adjust playback/render volume
bin/harness audio build render BGMBAT04 -o track.ogg # Ogg output (requires vorbis/ogg development libs; see build requirements below)
bin/harness audio build render BGMBAT04 -o track.flac # FLAC output (requires FLAC development lib; see build requirements below)
bin/harness audio build render BGMBAT04 -o track.wav
bin/harness audio build play out/extracted/BIN/BGM/BGM000.EMI        # play directly from EMI without prior extraction
bin/harness audio build play BGM000 -o out.wav                       # render to WAV instead of speakers
bin/harness audio build play out/extracted/BIN/SCE_XA/VOICE.STR -c 0 # play XA voice channel 0
bin/harness audio build render out/extracted/BIN/BGM/BGM000.EMI -o out.wav
bin/harness audio build vab2sf2 out/extracted/BIN/BGM/BGM000.EMI -o bank.sf2
bin/harness audio build emi-inspect out/extracted/BIN/BGM/BGM000.EMI
bin/harness audio build psf-pack out/extracted/SLUS_004.22 -o out/audio/bof3.psflib
bin/harness audio build psf-inspect out/audio/bof3.psflib
bin/harness audio build psf-run out/audio/bof3.psflib -n 100000
bin/harness audio build --examples
```

> **Build requirements:** `bin/harness audio build` is the stable surface. On
> first use it configures the tracked CMake source and builds `bof3-audio` into
> ignored `tools/c/psx-audio/build/`; later calls perform an incremental build.
> Setup requires CMake, a C compiler, and zlib development files. Ogg/Vorbis
> and FLAC support are independent build-time options, enabled when their
> development libraries are detected; ALSA is linked when found.
> Configure/build failures print a setup diagnostic and leave tracked source
> unchanged; no prebuilt audio executable is stored in Git.

Bare track names (`BGM000`) auto-resolve through the extracted track
catalog; bare `.EMI`/`.STR` paths such as `BGM000.EMI` or `VOICE.STR` do
**not** resolve — `play`, `render`, `vab2sf2`, and `emi-inspect` treat the
argument as an existing file path, so the examples above use the generated
extraction locations (`out/extracted/BIN/BGM/`, `out/extracted/BIN/SCE_XA/`;
see “Generated extraction outputs” below).

`bin/harness audio build` is the only supported audio surface; a previous Python wrapper
(`bin/bof3-audio`) was removed and no longer exists.

## Audio content on disc

### BGM tracks (81 archives in `BIN/BGM/`)

Each `BGMxxx.EMI` contains a VAB+SEP bundle:

| EMI type | Role | Format |
| ---: | --- | --- |
| 6 | VAB header (VH) | PS1 sample bank header |
| 7 | VAB body (VB) | ADPCM-encoded samples |
| 10 | SEP sequence | Multi-track MIDI-like events |

Named tracks: `BGMOPN` (opening), `BGMEND` (ending), `BGMSPC` (special),
`BGMBAT00`–`BGMBAT06` (battle 1–6 / boss). Remaining are numbered
(`BGM000`–`BGM197`), some with `A`/`B` variants.

### XA streaming audio (STR files)

| File | Content | Channels |
| --- | --- | --- |
| `BIN/SCE_XA/S_XA00.STR` (94,411,776 raw bytes) | Scenario music/SFX | 8 stereo |
| `BIN/SCE_XA/VOICE.STR` (8,260,096 raw bytes) | Voice clips | 5 mono |
| `BIN/BMAG_XA/MAGIC00.STR` (35,993,088 raw bytes) | Magic effects | 16 mono |
| `LOGO/CAPCOM30.STR` (2,698,080 raw bytes) | Capcom logo (video+audio) | 1 stereo |

### Sound banks (VAB in non-BGM archives)

| Family | Banks | Content |
| --- | ---: | --- |
| BENEMY | 200 | Enemy voice/SFX |
| BPLCHAR | 207 | Player character battle audio |
| BOSS | 155 | Boss battle audio |
| BMAGIC | 128 | Magic effect SFX |
| WORLD00–04 | ~200 | Area-local ambient SFX |
| BATTLE | 10 | Battle SFX |
| ETC | 17 | System/frontend audio |
| PLCHAR | 19 | Player character audio |

## XA ADPCM format

### Sector layout (2336 bytes)

```text
Offset  Size  Field
0x000     4   Subheader: [file_number, channel, submode, coding]
0x004     4   Subheader copy
0x008  2304   18 sound groups × 128 bytes
0x908    20   Unused payload tail
0x91C     4   EDC
```

### Sound group (128 bytes)

The first 16 bytes hold parameters and redundant copies. For 4-bit data,
bytes 4–11 describe eight 28-sample units; bytes 0–3 copy 4–7 and bytes 12–15
copy 8–11. Each parameter has a shift in bits 0–3 and predictor in bits 4–5;
the upper two bits are reserved. Distinct units may have different parameters.
The 112 data bytes are 28 four-byte rows: column `unit / 2` contains the low
or high nibble for that unit. Nibbles from successive rows form its 28 samples.
Mono plays units consecutively; stereo pairs even units left and odd units right.
This gives 224 mono frames or 112 stereo frames per group.

For 8-bit data, four signed-byte units use parameters 4–7 and one column each,
giving 112 mono or 56 stereo frames. These layouts follow
[PSX-SPX's XA format research](https://psx-spx.consoledev.net/cdromformat/#cdrom-xa-audio-adpcm-compression).
The previous contiguous-group/interleaved-nibble description was incorrect.

### Coding byte

- bits 0–1: mono (0), stereo (1), reserved (2–3)
- bits 2–3: 37800 Hz (0), 18900 Hz (1), reserved (2–3)
- bits 4–5: 4-bit (0), 8-bit (1), reserved (2–3)
- bit 6: emphasis; bit 7: reserved

BOF3 uses only `0x00` (mono 37800 Hz) and `0x01` (stereo 37800 Hz).

### Multiplexing

Selection retains both file and channel numbers, plus coding metadata. Game
cue tables determine applicable sector strides; a generic stream need not have
one fixed interleave period. EOF sectors (submode & 0x80) remain valid audio.

### Rust XA decoding and arithmetic evidence

[The Rust decoder](../../../tools/rust/bof3-audio/src/xa/mod.rs) handles 4-bit and
8-bit units, mono/stereo and both rates, with separate clipped channel histories.
Callers supply the file/channel/coding key, initial history and arithmetic choice.
History carries between groups and sectors; EOF does not implicitly reset it.
Zero history is an explicit export convention, not an established game seek state.
Subheader/parameter disagreements, other stream identities, non-Form-2 audio,
reserved coding/parameters and unsupported emphasis fail without changing history.
No error correction, de-emphasis, CD resampling, mixing or drive timing is implied
by this decoder; the separate [CD processing component](#rust-cdxa-sample-processing)
now owns explicit resampling and drive gain.

Reference arithmetic differs: the inspected
[DuckStation CD decoder](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/cdrom.cpp)
separately floors each predictor product divided by 64; the
[FFmpeg 6.1.1 XA decoder](https://raw.githubusercontent.com/FFmpeg/FFmpeg/n6.1.1/libavcodec/adpcm.c)
adds the products and 32 before the arithmetic right shift. `SplitFloor` and
`CombinedRounded` expose those choices explicitly. Both clip the result before
retaining history. Neither is accepted here as proof of hardware playback.

[Synthetic checks](../../../tools/rust/bof3-audio/tests/xa/cases.rs) cover ordering,
rates/depths, all four predictors, shifts 0–12, clipping, history, EOF and malformed
inputs. An independent [Rust integer model](../../../tools/rust/bof3-audio/tests/reference/xa.rs)
reproduces the frozen reference hashes for both arithmetic variants; FFmpeg
additionally checks filtered 4-bit fixtures.
The [disc corpus comparison](../../../tools/rust/bof3-audio/tests/xa/corpus.rs) reads
complete original extents and compares every 4-bit audio sector to FFmpeg using
explicit zero history per file/channel/coding stream. It passes byte-for-byte for
`CombinedRounded`: 31 streams, 26,599 sectors, 84,174,048 frames and 107,247,168
interleaved sample values. The variants differ at 68,029,547 values, with maximum
absolute difference 101 PCM16 units. These are decoder-arithmetic differences,
not measured encoding loss or a PSX/PC renderer fidelity result.

The complete corpus uses only coding 0 and 1, with matching parameter copies and
no reserved predictors/shifts. CAPCOM30 includes one silent mono sector on channel
0 alongside 143 stereo sectors on channel 1; stream selection keeps them distinct.
The retained C `xa.c` uses the older incorrect unit traversal, and its XA test
checks silence only. It is not a fidelity oracle; the Rust code is independently
implemented. XA extraction and XML-driven packing are described below; complete
game playback remains open.

### Rust XA encoding and stream reconstruction

[The XA encoder](../../../tools/rust/bof3-audio/src/xa/encoder.rs) replaces one
existing sector's audio at its encoded rate, depth and channel layout. It requires
exact PCM capacity, a file/channel/coding identity, explicit initial histories
and either decoder arithmetic model above. It does not silently pad, truncate,
resample, change coding or move cues. Emphasis and malformed source sectors remain
unsupported. Errors leave caller-owned bytes and encoder history unchanged.

[Shared quantization](../../../tools/rust/bof3-audio/src/codec/quantization.rs) now owns
the PSX/XA greedy unit search and decoded loss records. XA searches four predictors,
shifts 0–12 for four-bit units and 0–8 for eight-bit units. For integer PCM,
eight-bit shifts above eight supply no residual values beyond the shift-eight
range, so they offer no extra precision. Each chosen unit uses clipped decoded
history; the finished sector is decoded again before returning. Channel-specific
reports retain peak, RMS, signed mean and signal-to-noise error, initial/final
history, selected predictors and arithmetic. Hardware CD resampling and audible
fidelity are not established by these metrics.

Subheader copies, including EOF/EOR bits, and the 20-byte spare region remain
unchanged. Eight-bit mode also preserves unused parameter bytes. Active parameter
copies are written consistently. [Form 2 EDC handling](../../../tools/rust/bof3-audio/src/codec/edc.rs)
follows the [sector-format and EDC description](https://psx-spx.consoledev.net/cdromformat/#cdrom-sector-format):
the reflected polynomial is `0xD8018001`, with zero initial state and a
little-endian checksum covering the first 2,332 extracted bytes. Absent EDC stays
zero; present EDC must validate before editing and is recalculated afterward.
No Form 1 ECC repair or disc-image rewriting is implied.

[Whole-stream reconstruction](../../../tools/rust/bof3-audio/src/xa/reconstruction.rs)
selects every audio sector with the requested identity, retaining the exact sector
count and placement. It verifies all sector identities and byte equality of every
unselected sector. Each original sector is reused unchanged if it decodes to the
requested PCM under the current output history. Otherwise it is encoded again,
including following audio affected by an earlier edit's history. EOF never resets
history implicitly. Partial selections now use the boundary policy in the XA
packing section below; an arbitrary isolated edit is not assumed independent.

Known truncated source hashes fail before rebuilding. Known complete extents are
reported as verified; other inputs remain unverified. Returned data stays in
memory. The XML packing path below adds manifest validation, overlap/conflict
checks and publication. Automatic cue padding remains unsupported.

[Sector encoding tests](../../../tools/rust/bof3-audio/tests/xa/encoder.rs),
[reconstruction tests](../../../tools/rust/bof3-audio/tests/xa/reconstruction.rs) and
[EDC checks](../../../tools/rust/bof3-audio/tests/codec/edc.rs) establish:

- All eight supported rate/channel/depth combinations pass synthetic tests under
  both arithmetic models, including measured channel loss, unused-byte retention,
  carried history, explicit reset and rejected malformed/capacity-changing input.
- FFmpeg independently reproduces generated four-bit mono/stereo audio at both
  rates under `CombinedRounded`. Eight-bit coverage uses the independently
  established decoder vectors and synthetic reconstruction, not that consumer.
- The checksum check vector is `123456789` → `0x6EC2EDC4`. Twenty-eight original
  nonzero Form 1 EDC values independently validate the shared polynomial. Form 2
  field placement, present/absent checksums and corruption rejection have separate
  synthetic coverage; all 26,599 corpus audio sectors omit EDC.
- All 31 streams in the four complete original STR extents reproduce whole input
  files byte-for-byte: 26,599 selected sectors / 84,174,048 frames. Unrelated
  sectors remain unchanged; all four known truncated prefixes are rejected.
- Edited excerpts cover up to two sectors per stream: 61 sectors / 245,952 channel
  sample values, peak error 1,173 and weighted RMS 48.023856. Every seventeenth
  input value is increased by 257 with saturation. CAPCOM30's silent mono stream
  has only one sector, correcting the initial test's assumption of 62 excerpts.
  This does not validate full edited corpus publication or game playback.

The shared-search refactor retains the prior PSX encoder results, including its
1,179 edited corpus excerpts and independent VAG-consumer comparison.

### Rust XA extraction and preservation

[XA extraction](../../../tools/rust/bof3-audio/src/xa/extraction.rs) supports full
file/channel/coding streams and independently selected verified game cues:

```sh
bof3-audio extract --mode audio --kind xa_stream --archive FILE.STR \
  --xa-arithmetic combined-rounded --output NEW_DIR [--executable SLUS_004.22]
bof3-audio extract --mode audio --kind xa_cue --archive FILE.STR \
  --executable SLUS_004.22 --xa-arithmetic split-floor --output NEW_DIR [--id ID]
```

Repeated archives and `--disc-root` are supported. `--id` uses the catalog's
qualified identities or unambiguous numeric channel/cue IDs. Cues require the
verified executable and recognized media. Streams can omit the executable;
their cue placement then remains unresolved. Known truncated STR snapshots fail
before publication, including snapshots whose remaining audio happens to be
complete. Reports identify unrecognized extents and sources without selected
assets, rather than implying that every input has been exported.

Arithmetic is required explicitly as `split-floor` or `combined-rounded`.
Each exported asset starts with zero decoder history, a documented export
convention. WAVs retain the encoded rate and mono/stereo layout; bank-only
`--reference-rate` is rejected. PCM concatenates selected sectors without inserting
silence for multiplexed gaps. This is not a timed capture of the game's CD/SPU path.

Root `audio.xml` (`bof3.audio-extraction/v1`) references one source snapshot per
selected STR and one manifest/WAV folder per selected asset. Source XML
(`bof3.xa-source/v1`) contains the entire original STR as hexadecimal text, byte
length, hash, source identity and completeness evidence. It preserves interleaving,
unrelated sectors, payload tails and EDC bytes once, avoiding duplication per cue.
Asset XML (`bof3.xa-asset/v1`) uses relative source/WAV references and records
stream identity/coding, arithmetic, initial/final histories, hashes, and each
physical sector's decoded frame offset and submode. Keep the extraction root
together; individual XA folders refer to its shared source snapshots.

With executable evidence, asset manifests retain cue IDs, table addresses,
sector ranges/strides, WAV frame ranges, start positions and scheduler thresholds.
Encoded extents and audible endpoints remain distinct: XML labels the latter
unverified. Source snapshots are pinned to inventory hashes. The exporter shares
the bank exporter's staged publication, read-back checks, existing-output rejection
and ordinary-failure cleanup; this is not an adversarial multi-writer transaction.

[XA extraction checks](../../../tools/rust/bof3-audio/tests/xa/extraction.rs) use an
[independent Rust XML/WAV/FFmpeg consumer](../../../tools/rust/bof3-audio/tests/xa/consumer.rs)
and raw executable-table reads. The complete-disc check passed for 31 streams
(26,599 sectors, 84,174,048 frames) and 896 cues (26,115 selected sectors,
83,196,288 frames). Every exported WAV matches independent FFmpeg decoding with
the recorded zero-history/rounded-arithmetic context; every preserved source
equals its original STR. Cue geometry and thresholds match raw executable words.
A verified local export of voice cue `0x2000`
contains 71 sectors and 286,272 mono frames. Edited packing, unsupported emphasis,
game seek/reset history and hardware playback remain unaccepted.

### Byte-identical VAB payload groups

After the executable-backed loader resolves a single VB entry for a VH,
[bank content comparison](../../../tools/rust/bof3-audio/src/catalog/content.rs)
checks that all declared samples fit the body. It groups banks only when both
VH and VB payload lengths and SHA-256 values match. The complete declared VB
payload is hashed, including bytes outside sample spans; archive-sector padding
is excluded. Missing, multiple or undersized bodies remain unresolved with
diagnostics. Header equality alone is insufficient.

The original 880-EMI corpus contains 1,020 banks and 424 distinct VH/VB payload
pairs. There are 99 repeated pairs covering 695 bank identities. Independent
Python TOC parsing and `hashlib` grouping agree with every Rust group; the
[corpus mapping test](../../../tools/rust/bof3-audio/tests/catalog/loader.rs) also
compares each group's header/body bytes directly. Local comparison evidence is
`out/audio-migration/bank-content-reference.json`.

`map` emits `shared_banks`; `index`/`query` with a verified executable annotate
each resolved bank's `content`. Archive-qualified identities, game bank IDs,
load slots and playback contexts remain separate even when payloads match.
[Edited-fixture tests](../../../tools/rust/bof3-audio/tests/catalog/content.rs)
cover changed headers/bodies, untouched archive padding, missing/multiple
bodies, sample bounds and identical payloads assigned to distinct game IDs.
This is byte-content evidence, not SFX/vocal classification or PCM equivalence.

### Complete XA extents and US cue selection

ISO directory lengths count 2048-byte logical blocks. A raw XA extraction keeps
2336 bytes for each of those sectors, including duplicated subheaders and the
trailing bytes. The existing `out/extracted` STR files were exact, truncated
prefixes of the original disc extents; dividing the ISO length by 2336 instead
of 2048 reproduces their sector counts. Original files were preserved.

| Stream | Complete sectors | Prior extracted sectors | Missing audio sectors |
| --- | ---: | ---: | ---: |
| MAGIC00 | 15,408 | 13,509 | 1,347 |
| S_XA00 | 40,416 | 35,434 | 1,238 |
| VOICE | 3,536 | 3,101 | 0 |
| CAPCOM30 | 1,155 | 1,013 | 18 |

[Rust disc access](../../../tools/rust/bof3-audio/src/archive/disc.rs) reads the full
extents; the [recovery example](../../../tools/rust/bof3-audio/examples/recover_xa.rs)
validates the supplied US executable and writes only to a new output directory.
Local complete references are under `out/audio-migration/full-disc-streams`.
Independent raw-track/hash comparison and
[disc tests](../../../tools/rust/bof3-audio/tests/archive/disc.rs) cover the full extents.
`verify` identifies known full/truncated snapshots by whole-file hash and reports
completeness separately from byte equality. A known truncated source exits with
failure even if unchanged comparison bytes match. Unknown hashes remain unchecked.

In `exe/slus_004_22`, selector `0x80163744` (full EXE offset `0xCD744`) splits the
packed cue argument at bit 12: the upper value selects a stream and the low 12
bits select its cue index. The three file slots at `0x80183230` (file `0xED230`)
are `681` (S_XA00), `434` (MAGIC00), and `682` (VOICE). Table pointers at
`0x8018323C` (file `0xED23C`) lead to `0x801839C4`, `0x801832C4`, and
`0x80183298`, respectively; full-file offsets follow
`address - 0x80096800 + 0x800`.

A table word's high bit advances the channel and skips an additional word when
moving to the next cue. Low 15 bits are interleave-unit offsets, with stride 8
for stream 0 and 16 for streams 1/2. The original leaf routine selects filter
file 1 and computes a start LBA plus a separate stop threshold 150 sectors below
the next offset's LBA. This threshold is not yet accepted as an audible endpoint.
[Original-instruction checks](../../../tools/rust/bof3-audio/tests/xa_runtime.rs)
verify all 896 supported positive-range records against the original selector's
file, channel, start/current LBA and raw stop-threshold fields:

| Stream | Supported packed IDs | Table words | Cues | Selected audio sectors |
| --- | --- | ---: | ---: | ---: |
| S_XA00 | `0x0000`–`0x000A` | 15 | 11 | 10,962 |
| MAGIC00 | `0x1000`–`0x136F` | 896 | 880 | 14,856 |
| VOICE | `0x2000`–`0x2004` | 10 | 5 | 297 |

For low-15-bit offsets `start` and `end`, the selected sectors are
`start * stride + channel + n * stride`, for `0 <= n < end - start`.
The last selected sector, not a contiguous exclusive endpoint, determines
whether the cue fits the stream. VOICE channels 0–4 contain 71, 63, 61, 60
and 42 interleave units respectively.

[Cue bindings](../../../tools/rust/bof3-audio/src/xa/cue.rs) require recognized
whole-file hashes and check every selected sector's file/channel membership.
An incomplete cue has no selectable asset; its mapping reports the missing
sector count. Complete cues within a truncated source remain selectable with
the source's truncation status retained. Unknown or edited media do not acquire
cue IDs from their filenames. Multiple matching sources require qualified IDs.
[Disc-backed tests](../../../tools/rust/bof3-audio/tests/xa/cue.rs) independently
read all 26,115 selected sectors' subheaders from the original raw track.

`index` and `query --mode audio --kind xa_cue --executable EXE` expose these
source-qualified identities; `--id` accepts decimal or `0x`-prefixed packed IDs.
These prefixes do not establish caller bounds beyond them. Other channels
(including S_XA00 channels 4–7), SFX/vocal classification, scheduler stop timing
and playback remain unresolved. Archive and stream preservation retains the
unmapped sectors.

### US XA scheduler control flow

The [scheduler tests](../../../tools/rust/bof3-audio/tests/xa_scheduler.rs) execute
original US instructions with explicitly injected CD responses. They establish
control flow, not response latency or audible duration. Addresses below belong
to `exe/slus_004_22`; full executable offsets are `address - 0x80096800 + 0x800`.

| Runtime address | Full EXE offset | Verified role |
| --- | --- | --- |
| `0x801636A0` | `0xCD6A0` | Initialize cue, seek position and completion callback |
| `0x80163858` | `0xCD858` | Accept completion code 2, copy eight response bytes; other codes record error |
| `0x801638B0` | `0xCD8B0` | Dispatch state, handle cancellation and watchdog |
| `0x80183274` | `0xED274` | Nine state-handler addresses |
| `0x80163C58` | `0xCDC58` | State 5: poll/consume GetlocL position and compare threshold |
| `0x80163D20` | `0xCDD20` | State 6: reduce CD mix to zero, then request Pause (instruction evidence; hardware path not executed) |
| `0x80175ADC` | `0xDFADC` | Linked SDK logical-sector to BCD position conversion |
| `0x80175BE0` | `0xDFBE0` | Linked SDK BCD position to logical-sector conversion |

[CD position conversion](../../../tools/rust/bof3-audio/src/machine/cd_position.rs)
adds/removes the 150-sector lead-in, validates BCD digits, seconds below 60 and
sector numbers below 75, and rejects unrepresentable positions. It agrees with
the linked SDK for all supported cue starts/thresholds and boundary cases;
raw headers independently agree for all 26,115 mapped audio sectors.

Initialization installs the completion callback and enters state 1. State 5
handles position polling on alternate scheduler calls. A usable response must
have positive completion status and SDK last-command byte `0x801857A1 == 0x10`.
It converts the copied GetlocL position, updates current LBA, and enters state 6
when `current_lba >= runtime_stop_threshold_lba`. Tests exercise positions one
sector below, exactly at and one sector above every cue's threshold. Thus the
threshold really is 150 logical sectors before the next table offset; it is
not merely a coordinate conversion artifact.

The state watchdog triggers when its counter reaches 181 scheduler calls. It
retries from state 1 when the prior retry count is below 6 and response status
bit `0x10` is clear; otherwise it enters state 8. A cancellation flag also
redirects states below 6 to state 8. These are call counts, not proven durations.

`map` reports these entry points, the state table and tested condition;
cue metadata includes BCD start/threshold positions. Drive/IRQ latency, scheduler
frequency, CD buffering, mix writes and PCM delivery remain unverified.
[GetlocL reports the newest buffered sector](https://psx-spx.consoledev.net/cdromdrive/#getlocl-command-10h-int3ammassasectmodefilechannelsmci),
so its threshold alone cannot establish the final audible sample.

## VAB format (BOF3 variant)

### VabHdr (32 bytes at 0x00)

| Offset | Size | Field |
| ---: | ---: | --- |
| `0x00` | 4 | Magic `"VABp"` (bytes `70 42 41 56`) |
| `0x04` | 4 | Version (7) |
| `0x0C` | 4 | fsize (VH + VB total) |
| `0x10` | 2 | reserved0 (`0xEEEE`) |
| `0x12` | 2 | ps (programs) |
| `0x14` | 2 | ts (total tones) |
| `0x16` | 2 | vs (VAGs) |
| `0x18` | 1 | mvol |
| `0x19` | 1 | pan |

The `fast` renderer applies this bank volume and pan together with the selected
program and tone attributes. It does not apply a post-render bass boost or EQ.

### BOF3 quirks vs standard PsyQ

| Aspect | Standard PsyQ | BOF3 |
| --- | --- | --- |
| ProgAtr region | After VabHdr | Padded to 0x800 bytes |
| VagAtr start | After ProgAtr | Fixed at **0x820** |
| VagAtr count | `ts` (flat) | **`ps × 16`** (2D) |
| VAG size units | 8-byte | 8-byte |
| VAG pointer table | Per-sample sizes | Per-sample sizes; accumulate preceding entries for offsets |
| VAG first block | Format-dependent | 16 bytes of zero ADPCM data; transferred and played, not stripped |

### VagAtr (32 bytes each, at 0x820)

Key fields: vol (byte 2), pan (3), center note (4), unsigned pitch tune (5,
preserved as the full byte and added to playback pitch as `tune / 128`
semitones),
min/max note (6–7), adsr1 (bytes 16–17 u16 LE), adsr2 (18–19),
parent program (20–21 i16 LE), and vag index (22–23 i16 LE, 1-based). The
parent program selects the `ProgAtr` volume and pan used by the renderer.
Pitch bend range is tone-local: bytes 12–13 hold the downward/upward range in
semitones; the SEP bend value scales that range rather than a fixed MIDI ±2.
At playback, PsyQ `SsPitchFromNote` quantizes combined fine tune to 16 steps
per semitone and uses the linked integer lookup table before writing the SPU
pitch value (`0x1000` = 44100 Hz). The linked implementation is
`exe/slus_004_22@0x80171B20`; its 193-entry table is at runtime address
`0x8018445C` (raw payload offset `0xEDC5C`). The table's 386 bytes and the table
compiled into `spu.c` have the same SHA-256,
`293278b74970e97b814ab68b63edf21d4dcdc6630bd5394fce250aec6cd955b2`.
Addresses `0x800FDA84` and `0x800FDC5C`, previously attributed to this table,
contain zeros in the mapped raw payload and are not pitch-table evidence.

The linked routine masks the fine argument to 16 bits, adds the unsigned tone
shift, divides by 8, and carries one semitone when the result reaches 16. It
then forms a signed 16-bit semitone from `note + 60 - center + carry`, divides
that value by 12 with truncation toward zero, and indexes
`table[(remainder * 16) + fine_index]`. The quotient minus five shifts the
table value by octaves. It returns the low 16 bits without clamping; the
`0x4000` maximum belongs to the SPU pitch-counter step and is applied by each
renderer at playback.

### Note-on pitch and tone selection (libsnd voice manager)

BOF3 BGM is stock PsyQ `libsnd` SEP playback wrapped by a Capcom EMI bank
loader; there is no custom Capcom sequence language. The normal music note-on
path is `_SsNoteOn -> _SsVmKeyOn -> _SsVmSelectToneAndVag`, which is distinct
from the explicit-tone SFX dispatcher `SsUtKeyOnV` (the routine at `0x8016E400`
that calls `note2pitch` at `0x8016E73C` with a caller-supplied tone). The BGM
renderer must not be modeled on the SFX path.

Verified against the linked binary and corroborated by VGMTrans and stock
libsnd:

- **Initial pitch uses `fine = 0`.** The normal note-on call is
  `note2pitch(note, 0, center, shift)`. The channel's current pitch bend is
  **not** folded into a new note. Bend is applied only when a pitch-bend event
  arrives, via `_SsVmPitchBend` (`0x801728E0`) iterating active voices through
  `_SsVmPBVoice` (`0x801726E0`, which has a single caller). Folding the stored
  channel bend into note-on pitch makes notes sound sharp until the next bend
  event.
- **Tone selection layers every matching tone.** For a logical program and
  note, `_SsVmSelectToneAndVag` walks the program's up-to-16 tones and selects
  **all** with `VagAtr.min <= note <= VagAtr.max`, allocating one SPU voice per
  layer. It does not pick only the first match, the nearest center note, or a
  fallback tone.
- **Logical program -> physical tone block is packed.** Tone blocks are stored
  only for non-empty programs; the physical block is the count of non-empty
  programs preceding the logical program (stock libsnd caches it in
  `ProgAtr.reserved1`). The embedded `VagAtr.prog` (bytes 20-21) names the
  logical program and is the renderer's lookup key.

#### Fast-path pitch faithfulness audit (BGMBAT04 off-key investigation)

| Component | Status | Evidence |
| --- | --- | --- |
| EMI -> VH/VB/SEP loading | byte-identical to direct bins | SHA-256 (VH@0x800, SEP@0x3000, VB@0x8800) |
| `spu_pitch_from_note` | faithful | disasm `0x80171B20` |
| `pitch_table` | byte-identical | dump @`0x8018445C` |
| `voice_pitch` bend arithmetic | faithful | disasm `_SsVmPBVoice@0x801726E0` |
| SPU pitch counter / Gaussian / key-on | matches | psx-spx + DuckStation |
| VagAtr center/shift parse | correct (`[4]`/`[5]`) | disasm + VGMTrans |
| note-on bend folding | **bug** | libsnd uses base pitch; bend only via events |

Differential: `BGMBAT02.EMI` sounds correct while `BGMBAT04.EMI` is sharp; both
share the same code and structurally similar VABs (single-note piano tones with
`center > note` and `shift` 54/59/62), so the defect is data-triggered rather
than a per-tone formula error. Piano (prog 0) has `pbmin = pbmax = 0` and
disjoint tone ranges, so the bend-fold fix does not change its pitch; if piano
remains sharp after the fix, a renderer pitch trace must pinpoint the cause.

## SEP format (Sequence Package)

The EMI catalog labels type 10 as "SEQ", but the container is **SEP**
(multi-track). All 81 BOF3 music files have exactly 4 sequences.

### File header (6 bytes)

Magic `70 51 45 53` ("SEQp" LE), version u16 BE = 0.

### Per-sequence header (13 bytes)

seq_id (u16 BE), resolution (u16 BE, always 48), tempo (3 bytes BE,
µs/quarter), time signature (2 bytes), data_size (u32 BE).

### Event encoding

MIDI-like: VLQ delta times, running status, note on/off,
program change, control change, pitch bend. Only meta events are tempo and EOT.

Pitch bend occupies two MIDI-style data bytes. The direct renderer uses the
second as the 7-bit coarse value centered at 64; the first is the fine byte and
is not consumed by the linked coarse bend calculation. Meta events omit SMF lengths:
tempo is `FF 51 tt tt tt` and EOT is `FF 2F`.

NRPN extensions: loop start (20), loop end (30), VAB attribute control.

## Rust MIDI interchange

Retained [MIDI research notes](../../reference/standard-midi-files.md) identify
the inspected specification, page locations and BOF3 translation distinctions.

[The MIDI codec](../../../tools/rust/bof3-audio/src/interchange/midi.rs) reads and writes
Standard MIDI Files, formats 0 and 1, with nonzero PPQN timing. Format 2 and
SMPTE division fail explicitly; independent SEP sequences must use separate
files. This is interchange framing, not SEP translation or a game-event validator.

The framing follows the MIDI Manufacturers Association's
[SMF specification](https://midi.org/standard-midi-files-specification), also
available in its [complete specification](https://www.freqsound.com/SIRA/MIDI%20Specification.pdf).
Track events carry absolute ticks in memory and retain their order at equal
ticks. SMF meta and SysEx events cancel running channel status; SEP's meta
handling is different and uses its separate reader. Delta and payload-length
VLQs are bounded to four bytes. Each track must end with a zero-length end marker.

Unchanged tracks reuse their exact original bytes, including running status and
non-canonical VLQs. Equality is checked against parsed events before reuse.
Edited tracks emit explicit channel status; unchanged neighboring tracks, opaque
chunks and header extensions retain their bytes and positions. Unknown meta and
SysEx payloads, including extended known meta payloads, remain opaque. No unknown
event is dropped. Metadata meaning, multipart SysEx protocol and BOF3-supported
transformations require separate validation before playback or packing.

The writer rejects backward ticks, oversized deltas, invalid channel data,
missing/misplaced end markers and implicit track-count changes. Rebuilding a
different track layout requires explicitly constructing a new file. The reader
also rejects truncated chunks/events, header-count mismatches, duplicate headers,
unsupported file statuses and running status used after cancellation.

[Five tests](../../../tools/rust/bof3-audio/tests/interchange/midi.rs) cover exact preservation,
edited-track isolation, all channel-message shapes, VLQ boundaries, unknown data
and malformed input/edits. RustySynth independently accepts a format-1 conductor
and performance fixture and computes its duration as 1.5 seconds; changing a
parsed tempo event produces 0.75 seconds. This checks structural consumption and
tempo timing, not SoundFont playback or BOF3 musical equivalence. Bounded forward
translation and fixed-layout inverse validation have separate evidence below;
complete music packing and playback acceptance remain open.

### SEP execution-order MIDI translation

[The timeline](../../../tools/rust/bof3-audio/src/sequence/timeline.rs) traverses
one freshly initialized sequence using the shared message reader and verified
single-cursor loop transitions. It decodes each jump target with the **current**
running status rather than replaying statically parsed events. Finite loops use
the delta consumed after the loop end; infinite loops consume it but force repeat
delay zero. Overwritten and unmatched starts do not create a loop stack. Steps
retain source cursors, message ranges, logical ticks, effective status, consumed
deltas, next cursor/delay and loop state.

Default inspection policy stops at the second traversal of an infinite loop,
preserving the original step's jump/count result and recording the policy stop
separately. It does not invent execution of the subsequent source EOT. Finite
counts remain source-controlled; ordinary EOT stops this trace without modeling
whole-sequence restarts or successor activation. Event/tick budgets, unknown
controllers, malformed input and deltas overflowing the positive signed runtime
delay after multiplication by ten fail explicitly. Opaque post-EOT bytes remain
outside traversal. Logical ticks are not scheduler calls or hardware time.

[The translator](../../../tools/rust/bof3-audio/src/sequence/midi.rs) writes one
format-1 SMF per selected sequence: a conductor track for header/tempo and one
ordered performance track. Sequence index and game ID remain distinct. Every
executed source event maps to a generated track/event position. Game loop controls
become text markers; notes, programs and volume/pan values retain their ordering
and numbers. Bend low bytes become zero for playback, reflecting the linked
callback; the timeline retains original values. Tempo reports retain both source
microseconds/quarter and the linked integer BPM. Source PPQN is unchanged.

Generated all-notes-off events terminate the chosen finite boundary, followed by
track end markers. The expanded MIDI should be rendered once, without an extra
whole-file repeat. Time-signature metronome/32nd fields use SMF defaults 24/8.
The translation report retains limits on bank/program/channel binding, tone bend
ranges, measured gain/pan/envelope response and quantized game timing. Original
SEP bytes must still be preserved in XML; a MIDI expansion is not a lossless
replacement. Song-folder extraction now uses this component with shared channel
setup; the fixed-layout inverse below does not accept arbitrary MIDI edits.

[Timeline tests](../../../tools/rust/bof3-audio/tests/sequence/timeline.rs) retain
literal cursor/tick expectations for finite/infinite loops, dynamic channels,
overwritten starts and rejection limits. The
[original-dispatch test](../../../tools/rust/bof3-audio/tests/sequence/runtime.rs)
compares every pre-EOT step for counts 0/1/2/3/127 against unchanged executable
instructions and retains its earlier literal assertions. It uses controlled RAM
contexts, not independent gameplay audio traces.

[Translation tests](../../../tools/rust/bof3-audio/tests/sequence/midi.rs) check
conductor/performance order, source mappings, bend normalization, validation,
independent MIDI timing and a synthetic SEP→MIDI/SF2→PCM path producing 100 Hz.
All 476 corpus sequences translate independently with the two-traversal policy:
466 stop at infinite-loop boundaries, 10 at EOT; 930,601 executed events, at most
13,684 per sequence, produce 3,818,755 total SMF bytes. There are 14,264 executed
nonzero bend low bytes normalized for playback. RustySynth independently parses
every MIDI and agrees with the rational scheduler's duration within one 44.1 kHz
frame. Corpus structure/timing agreement does not establish instrument fidelity.

<a id="fixed-layout-midi-to-sep-reconstruction"></a>
### Event-preserving MIDI-to-SEP reconstruction

[The inverse validator](../../../tools/rust/bof3-audio/src/sequence/editing.rs)
regenerates correspondence from original SEP bytes, sequence headers and explicit
loop limits. It never trusts manifest event mappings. Song export and inverse
validation share channel initialization in the forward translator; callers may
explicitly select translation without that setup. The parsed MIDI container must
retain format, PPQN, tracks, header/chunk contents and event order. Alternate
running-status/VLQ encodings alone are not edits. Event insertion/removal, channel
changes, authored loop/end marker changes and generated setup edits fail with
track/event or source-cursor diagnostics. Generated termination events must follow
the requested final tick; timing reconstruction is described below.

Supported fixed-width edits are note key/velocity, program, volume/pan values,
high-byte bend, header tempo and executed tempo events. MIDI note-off with zero
release velocity normalizes to the source note-on/zero-velocity representation;
nonzero release velocity fails. Nonzero MIDI bend low bytes fail because the linked
callback cannot represent them; original ignored SEP low bytes survive unchanged.
Tempo must remain a nonzero 24-bit value. Each source-byte proposal includes every
executed visit, including unchanged visits; conflicting repeated or overlapping
visits fail rather than choosing one edit. Unchanged deltas, explicit/running statuses,
unvisited bytes and opaque suffixes retain their original encodings.

The reconstructed sequence is translated again and must reproduce every requested
event after supported note-off normalization. Multi-sequence entry reconstruction
rejects duplicate/out-of-range selections and preserves other sequences, IDs,
header fields, order and padding. Changed sequence lengths relocate later records
without changing those records' bytes. Reports separate message-value source-byte/
event changes, delta-span changes, data lengths, header tempo and unchanged-byte
reuse. The music packing path below now calls
this library component with manifest and bank-program validation. Arbitrary SF2
edits, broader transformations and complete musical acceptance remain required. Forward playback
approximations, scheduler fidelity and independent audio acceptance remain open.

[Four synthetic checks and one corpus check](../../../tools/rust/bof3-audio/tests/sequence/editing.rs)
cover mixed edits, running channel/meta status, noncanonical deltas, ignored bend
bytes, opaque suffixes, other-sequence isolation, finite/infinite loop agreement,
container edits and explicit unsupported-event rejection. All 476 corpus sequences
reconstruct into 119 byte-identical complete SEP entries. Consistent note-velocity
edits change 231,340 source events across all 119 entries; complete retranslation
agrees with the edited MIDI and RustySynth independently parses those SMFs. These
checks establish preservation and bounded inverse behavior, not independent
gameplay fidelity or full song-folder packing. Evidence is retained under
`out/audio-migration/sequence-edit-*`.

### MIDI timing reconstruction

[The delta inverse](../../../tools/rust/bof3-audio/src/sequence/timing.rs)
derives requested ticks in original execution order across both MIDI tracks.
The first tick constrains the initial delta; subsequent differences constrain
the delta consumed after the preceding message, including a finite loop-end
jump. Every visit consuming one source delta must agree. Infinite jumps force
zero delay, so their consumed but ignored deltas retain their original encoding
unless another observable visit constrains them. No delay is invented after the
selected final execution boundary. Backward execution order, conflicting visits,
nonzero infinite-jump delay and positive signed overflow after tenfold runtime
scaling reject explicitly.

Only changed deltas receive new minimal VLQ encodings. Unchanged noncanonical
encodings, running statuses, unvisited data and opaque suffixes remain exact.
Delta spans overlapping executed messages or other changed deltas reject. Value
edits apply at original offsets before delta resizing. Reconstruction updates the
selected sequence length and preserves following sequence records and container
padding. Full retranslation verifies the requested ticks/events; generated
source-offset marker text follows relocated positions after the input markers
have passed unchanged-text validation. Initial tempo/setup stay at tick zero,
and generated all-notes-off/end events must match the new final tick.

[Timing tests](../../../tools/rust/bof3-audio/tests/sequence/timing.rs) cover
VLQ growth/shrinkage, combined value edits, other-sequence isolation, finite and
infinite loops, repeated-visit disagreement, cross-track ordering, termination
placement and signed scaling limits. The
[corpus probe](../../../tools/rust/bof3-audio/tests/support/corpus/timing.rs)
triples all 476 sequences' ticks, checks every reconstructed event and verifies
tripled duration through RustySynth: 240,915 deltas change and 117 entries resize.
All 119 original SEP entries still round-trip byte-exactly.
[Original-US execution](../../../tools/rust/bof3-audio/tests/common/timing_runtime.rs)
checks 15 streams through delta reader `0x8016AAD4` and dispatcher `0x8016D0E0`,
including relocated loop targets and requested ticks. These synthetic RAM probes
do not establish original scheduler timing or independent audio fidelity.
[Music publication](../../../tools/rust/bof3-audio/tests/support/music/timing.rs)
checks resized SEP publication with untouched other sequences and archive entries.
Logs use `out/audio-migration/timing-edit-*`; event insertion/removal, channel
changes, loop-structure editing and full musical acceptance remain open.

## Rust SoundFont reading

[The reader](../../../tools/rust/bof3-audio/src/soundfont/reader/mod.rs) and
[table validator](../../../tools/rust/bof3-audio/src/soundfont/tables.rs) inspect
SF2 2.00, 2.01 and 2.04 without passing through the writer's narrower authored-bank
model. The [primary-source notes](../../reference/soundfont.md#reader-research-and-preservation)
record table, sample and hierarchy evidence. This is structural reading, not
generator/modulator synthesis, edit approval or BOF3 semantic reconstruction.

The reader validates RIFF extent, padded chunk boundaries, required list/table
order and uniqueness, fixed record widths, monotonic indices and terminal counts.
Preset identities remain bank/program pairs; zone links cannot target terminal or
out-of-range records. A first global zone is distinct from local linked zones;
layers, raw generators and custom modulators remain ordered and separate.
Unknown operator values are retained for later semantic validation. Ambiguous
duplicate generators, range/link ordering errors and incomplete local zones fail
explicitly rather than adopting consumer-specific ignoring behavior. Duplicate
sample/instrument names remain separate records. Terminal names/payloads, unknown
chunks, unused PCM, padding and metadata remain in `original_bytes()`; that method
is explicitly not a serializer for mutations of the public parsed view.

PCM16 samples become exact signed PCM24 values by multiplication by 256. For
2.04 `sm24`, the corresponding unsigned low byte completes each value without
rounding. Invalid low-byte lengths or pre-2.04 low-byte chunks fail explicitly.
Sample headers retain raw start/end/loop positions, rate, pitch correction, root
key and link/type. PCM access uses header bounds without applying zone address
offsets. Stereo links must be reciprocal left/right records; ROM metadata may be
read, but ROM PCM requests fail. Invalid sample bounds/type/rate/root fail;
nonportable length/rate, missing/nonzero guard data and loops outside the sample
are diagnostics for later playback/edit validation. Defaults bound files to
256 MiB, 65,536 chunks, one million table records and 32 million sample points.

[Reader tests](../../../tools/rust/bof3-audio/tests/soundfont/reader/cases.rs) cover
writer/consumer agreement, exact PCM, global zones, custom modulators, unknown
operators/chunks, odd padding, duplicate names, opaque terminal payloads,
24-bit signed extremes, stereo/ROM handling, malformed indices/links, resource
limits, every fixture truncation and deterministic byte mutations without panics.
Music packing uses this reader for the bounded sample inverse below. Instrument
reconstruction remains open; successful parsing must never be treated as approval
to discard unsupported SF2 behavior.

The existing corpus binding check now also reads all 842 constructible banks
(15,095 instrument zones), checks every sample's exact PCM against the authored
bank and compares preset/instrument counts with independent RustySynth parsing.
The 178 existing conversion rejections remain: 133 pitch-table contexts, four
tuning ranges and 41 zero pitches. This validates generated-bank structure and
sample reading, not those rejected contexts or arbitrary external SF2 semantics.
Twenty-five focused tests with local media pass; evidence is retained under
`out/audio-migration/sf2-read-*`.

## Rust SoundFont sample packing

[The sample inverse](../../../tools/rust/bof3-audio/src/soundfont/packing/sample.rs)
compares an edited SF2 against a fresh binding/export regenerated from preserved
VAB bytes and the supported executable. It accepts fixed-length PCM edits and
shared-consistent nonlooping/continuous sample loops. Sample count, storage
positions, allocation sizes, headers other than loop points, generator/modulator
records other than sample mode and separately validated tone gain/pan/tuning/assignment, metadata,
opaque chunks, terminals and padding must remain unchanged. The sample-only API
still rejects tone edits; the bank API validates all four inverses before returning.
Unowned PCM, guard samples and generated silence cannot be edited. Added/removed
records, release-only loops, instrument topology, unsupported tuning/envelope changes and
allocation reconstruction fail explicitly. All instrument zones
referring to one shared sample must agree on its loop mode. Loop endpoints must
align to 28-frame blocks, with the loop ending at the sample end. Disabling a loop
leaves its unused SF2 header points unchanged; edits to ignored points reject.

Content comparison uses exact signed PCM24 and loop meaning. SF2 2.01 PCM16 and
2.04 PCM24 with zero low bytes therefore reuse identical original ADPCM, including
opaque allocation tails and original predictor-dependent loops. Edited PCM24
rounds to nearest PCM16, ties toward positive infinity, with endpoint saturation.
[Shared sample reconstruction](../../../tools/rust/bof3-audio/src/sample/editing.rs),
also used by WAV packing, encodes only changed PCM/loops within the original
allocation and verifies the reconstructed decode. Unused allocation tails and
unrelated body bytes survive. Sub-PCM16 edits may retain original ADPCM while
still reporting quantization loss; they are not classified as unchanged content.

The `bof3.audio.sf2-sample-pack/v1` report records baseline/input SF2 hashes,
byte equality, source-qualified sample mappings, content/encoded change flags,
encoded hashes, retained/replaced tail sizes, PCM24 rounding and total decoded
loss separately from the encoder's PCM16 loss. It does not establish runtime
pitch, interpolation, mixing or PC/PSX audio fidelity.

[Tests](../../../tools/rust/bof3-audio/tests/soundfont/packing/sample.rs) cover shared
layered samples, PCM16/24 equivalence and rounding loss, added/removed/moved loops,
loop conflicts, allocation preservation and rejection of unsupported edits.
All 842 constructible corpus banks retain their original body bytes through this
inverse. The existing 178 pitch-context conversion failures remain explicit.
Music-folder checks cover edited BGM004 PCM, combined MIDI/sample edits and
disagreeing selected copies of a shared bank. Evidence uses
`out/audio-migration/sf2-pack-*`; broad SF2 reconstruction remains open.

## Rust SoundFont tone-control packing

[The tone inverse](../../../tools/rust/bof3-audio/src/soundfont/packing/tone.rs)
accepts uniform instrument-zone `initialAttenuation` and `pan` edits when a
seven-bit VAB tone volume/pan pair reproduces both requested generators exactly
under the recorded gain model. It regenerates the original ordinary-sequence
reference from preserved VAB bytes before searching all 16,384 pairs. Bank and
program controls stay fixed. Every key zone belonging to the tone must request
the same pair; separate layered tones remain independently editable. Unchanged
controls retain original bytes without canonicalization. Nonuniform/global-zone
edits, unrepresentable pairs and invalid ranges fail explicitly. Silent tones
require the joint [unmute transform](#rust-soundfont-silent-tone-unmuting).
Topology, key-range and envelope
reconstruction remain open; note-on tuning and existing PCM assignments are
handled below.

Several VAB pairs can produce the same quantized generators. The inverse chooses
the pair with minimum summed absolute volume/pan distance from the original,
then lowest volume and pan, and reports the number of matching candidates. It
changes only bytes 2 and 3 of the selected 32-byte tone records, reparses the
header and verifies the forward fit. This preserves the requested export-context
controls, not equivalence under all subsequent controller changes. The existing
gain approximation and its measured error remain explicit; no nearest-generator
substitution is accepted when an exact inverse is absent.

The `bof3.audio.sf2-tone-pack/v1` report records source/output header hashes,
program/tone/instrument identities, original/chosen controls, candidate counts
and the selected forward fit, including game register magnitudes and approximation
error. Header hashes describe the final combined header, including separately
validated pitch edits. The enclosing bank packer validates remaining SoundFont content
through the sample inverse before accepting results. Music packing stages header,
body and sequence replacements together; selected songs sharing either bank entry
must agree, including unchanged copies.

[Tests](../../../tools/rust/bof3-audio/tests/soundfont/packing/tone.rs) exercise both
gain models, layered tone edits, exact SF2 re-export, opaque header preservation,
combined PCM/control edits, ambiguous center-pan choices and explicit rejection
of unsupported edits. Independent RustySynth parsing checks the regenerated
banks. Corpus and original-US note-on checks are retained under
`out/audio-migration/tone-pack-*`; these are not full-runtime or audio-fidelity
acceptance.

## Rust SoundFont pitch packing

[The pitch inverse](../../../tools/rust/bof3-audio/src/soundfont/packing/pitch.rs)
accepts instrument `overridingRootKey`, `coarseTune` and `fineTune` edits against
the fresh original-US export. It compares their combined pitch in integer cents,
`-100*root + 100*coarse + fine`; the unchanged MIDI key contributes the same term
on both sides. Equivalent generator representations may therefore re-export with
different individual values while preserving requested note-on pitch. Every
original single-key zone must remain present and match exactly; there is no
nearest-pitch fallback. Sample-header pitch/rate, scale tuning, key ranges,
topology, modulators and unrelated generator records remain fixed.

The bank API takes an optional original-executable `tuning::Reference`; a pitch
edit requires it. Music-folder packing supplies its existing identified US
reference. The inverse first re-executes original center/shift over the source
key range to validate baseline semantics. It then searches all 256 center bytes
and 32 distinct shift groups by executing the original pitch routine, including
each candidate's actual table lookup. Reads outside both verified tables, zero steps and SF2
tuning-range failures cannot become candidates.

In `exe/slus_004_22@0x80171B48..0x80171B64` (full EXE `0xDBB48..0xDBB64`), the
unsigned tone shift byte is added to the fine argument then divided by eight.
For this zero-fine-argument note-on context, the low three shift bits have no
effect. One representative per group suffices; all eight byte encodings count
toward the reported number of matches. Selection minimizes total absolute
distance from original center/shift, then center and shift, retaining original
ignored bits when possible. Search probes do not populate the long-lived export
cache. Runtime bends with a nonzero fine argument remain outside this equivalence.

Only tone-record bytes 4/5 are patched. Gain/pan and sample edits are validated
separately before any result is returned or published; header/body/sequence
publication and shared-bank conflict rules remain unchanged. The
`bof3.audio.sf2-pitch-pack/v1` report records edited program/tone/instrument
identities, original/chosen center/shift, candidate counts, requested cents and
selected per-key measurements, with executable identity, SPU registers, table
lookup witnesses and SF2 quantization error. Representation-only edits retain
original VH/VB bytes but still record their validated requests.

[Tests](../../../tools/rust/bof3-audio/tests/soundfont/packing/pitch.rs) cover required
executable evidence, layered tuning plus gain edits, equivalent representations,
ignored-bit preservation, opaque header bytes, independent consumer parsing and
rejection of an unrepresentable one-cent key edit or scale-tuning change. The
original routine is also checked across 1,050 low-bit comparisons. Music-folder
tests reject conflicting tuning requests to a shared bank before publication and
accept agreeing requests while preserving unrelated entries. Corpus evidence is
retained under `out/audio-migration/pitch-pack-*`; note-on reconstruction does not
establish bend/controller equivalence, complete bank coverage or PSX/PC PCM fidelity.

## Rust SoundFont sample assignment packing

[The assignment inverse](../../../tools/rust/bof3-audio/src/soundfont/packing/assignment.rs)
accepts changes to a fixed tone's `sampleID` generators when every key zone
selects the same existing PCM allocation. It resolves the requested SF2 sample
through the freshly regenerated bank binding, preserving source-qualified sample
identities. Separate layered tones may select different samples. Added samples,
per-key assignment requiring tone splitting and generated-silence targets
reject explicitly. Silent tones require the joint unmute transform below.
The source and target sample-header
pitch/rate contexts must agree; other sample metadata remains fixed.

The [verified US selector](#us-runtime-sample-reference-resolution) truncates the
tone's signed sample-reference word to one byte. Reassignment changes the low
byte at tone offset 22 and preserves its high byte at offset 23. Unchanged raw
references retain their exact encoding, including zero aliases. Allocation 255
cannot be selected as PCM because byte 255 selects noise; it rejects even if an
unreferenced decoded sample exists in the exported font. The reconstructed word
is resolved again before accepting the patch.

All edited users of a shared sample must agree on loop mode. The existing sample
inverse validates the requested mode/points and encodes any actual PCM or loop
change within that allocation. Reassignment alone retains the entire body,
including a looped sample that loses its last tone. Unreferenced sample loop
intent continues to come from its SF2 header. A target with no valid loop points
cannot become looping just by retaining the old tone's sample mode. Gain, pitch,
assignment and sample changes are validated together before returning header/body
bytes or publishing music archives; disagreeing shared-song requests fail before
publication.

The `bof3.audio.sf2-assignment-pack/v1` report records each changed
program/tone/instrument, original and chosen signed reference, SF2 sample index
and game sample ID. Gain report header hashes cover the final combined header.
[Four tests](../../../tools/rust/bof3-audio/tests/soundfont/packing/assignment.rs)
cover opaque high-byte/zero-alias preservation, layered assignment and exact
SF2 re-export, no-user loop retention, combined PCM editing, shared-loop conflicts,
per-key rejection, generated silence and the noise slot. Independent RustySynth
parsing checks rebuilt fonts. Original-US loader/note-on checks include raw
`0xA500` and `0xA503`, verifying both preserved-high-byte cases. Corpus and
music-folder publication evidence uses `out/audio-migration/assignment-pack-*`.
Broader topology/envelope editing, noise synthesis and
independent full-runtime/audio fidelity remain separate obligations.

## Rust SoundFont silent-tone unmuting

The enclosing bank packer accepts an explicit unmute when every fixed key zone
of a muted tone is relinked from generated silence to the same existing PCM
allocation and requests uniform, exactly invertible gain/pan. Relinking alone
is sufficient when the desired SF2 gain remains unity/center: it still triggers
the exhaustive gain inverse. Gain-only edits that retain the synthetic silence
sample reject. Bank/program controls remain fixed; their mutes cannot be
overridden by tone controls and produce a no-exact-inverse diagnostic.

The assignment inverse receives the gain-validated header and checks that the
selected tone is audible under its recorded ordinary-sequence context. Existing
sample-header tuning, noise-selector, fixed-zone and shared-loop restrictions
still apply. Returning to the original allocation preserves the entire encoded
reference, including raw zero aliases and opaque high bytes. Assignment reports
mark the change with `unmuted`; the gain report retains selected controls,
candidate count and approximation error. No claim of dynamic-controller or
independent PSX/PC audio equivalence follows from this inverse.

Generated silence PCM, guard data and loop points remain immutable, including
when its final user is removed. Input SF2 storage/layout stays fixed. Fresh
re-export omits the now-unused synthetic sample when no muted tones remain;
all real source PCM and allocation identities survive. Otherwise, unchanged
sample bodies and exact edited SF2 re-export are checked independently.

[Focused checks](../../../tools/rust/bof3-audio/tests/soundfont/packing/activation.rs)
exercise both gain models, layered unmuting, unchanged-gain activation, alias
preservation, last-user removal and rejected incomplete edits. The
[corpus probe](../../../tools/rust/bof3-audio/tests/support/corpus/activation.rs)
selects one eligible muted tone per bank, validates fresh semantic re-export
and independent RustySynth parsing, and requires byte-exact source PCM.
[Music-folder checks](../../../tools/rust/bof3-audio/tests/support/music/activation.rs)
reject disagreeing shared-song requests before publication and verify agreed
edits change only the bank header. Evidence uses `out/audio-migration/unmute-*`;
arbitrary topology/envelope editing and full runtime fidelity remain open.

## Rust SoundFont writing

[The SF2 writer](../../../tools/rust/bof3-audio/src/soundfont/mod.rs) emits the
16-bit SoundFont 2.01 subset. Its typed model retains distinct preset bank/program
identities, ordered layered zones, shared mono samples, key/velocity ranges,
per-zone root/coarse/fine tuning, scale tuning, pan, attenuation, volume envelopes
and no-loop/continuous/until-release modes. It writes the required RIFF lists,
cross-table indices, terminal records and 46 zero guard points per sample.
See [retained specification research](../../reference/soundfont.md).

Invalid names, identities, references, parameter ranges, loop endpoints and index
capacity fail explicitly. Optional sample/loop hardware-portability limits produce
diagnostics without modifying PCM or loop boundaries. This model authors SF2;
it does not parse or preserve arbitrary edited SF2, translate VAB parameters or
choose musical approximations. Custom modulators and other unmodeled generators
are not accepted inputs. Unspecified synthesis behavior uses SoundFont consumer
defaults, which are not asserted to match BOF3.

[Six synthetic tests](../../../tools/rust/bof3-audio/tests/soundfont/cases.rs) check
structure and independent RustySynth parsing/playback. They verify layered shared
samples, range gating, pan, octave tuning, continuous/until-release loops and
release tails. A generated format-1 MIDI and generated SF2 play together through
RustySynth's sequencer with the expected program changes, 100/200 Hz sections and
silent final tail. This is writer interoperability evidence only. Actual VAB
conversion, ADSR/tuning approximation reports, game music playback and edited SF2
reconstruction remain pending; the complete song/renderer gates remain open.

### VAB bank binding with explicit rendering contexts

[The bank binder](../../../tools/rust/bof3-audio/src/soundfont/bank.rs) constructs
an authored SoundFont from VH/VB bytes and a rendering context for **every** tone.
The qualified source, VH/VB entry indices, runtime bank ID and VAB header ID remain
separate in its report. Sparse logical program numbers are retained; each source
tone becomes a separate SF2 instrument with one calibrated zone per source key.
All matching instruments remain layered in the program preset. Samples are shared
by their original one-based IDs. Empty, unreferenced sample allocations retain
report entries without fabricated PCM; referencing an empty slot fails.

Contexts explicitly supply gain/pan and provenance, a fit matching source ADSR,
and complete pitch rows at the chosen export rate. `ToneContext::from_reference`
executes the supported original note-on routine through the tuning owner. The
binder does not infer game gain/pan from unverified header/program/tone fields.
It rejects missing, duplicate, extra or inconsistent contexts, unsupported keys,
invalid references, tone modes other than 0/4, nonzero vibrato/portamento fields,
unterminated sample allocations and format-capacity failures. Source metadata is
retained; provenance supplied by a caller is not automatic evidence approval.

Local ADPCM loop ranges become continuous SF2 sample loops, including during
release. A loop whose predictor history changes requires explicit permission
through `allow_predictor_loop_approximation` and is flagged in the report.
Optional percussion aliases preserve the program number in SF2 bank 128 for
channel-10 consumers; this requires base bank zero. Other explicit preset banks
can use 0–128 without that alias. The writer's sample/loop portability diagnostics
remain visible. Full original archive bytes still belong in preservation XML.

[Five default tests](../../../tools/rust/bof3-audio/tests/soundfont/bank.rs) verify
sparse programs, layers, shared/empty sample identities, rejection paths and
explicit loop policy. RustySynth independently reads generated banks and plays
both layers, the percussion alias and key-range silence. An opt-in synthetic-bank
test uses the real US executable for every note-on tuning context.

The initial corpus probe rejected all 1,020 banks at a conservative mode check;
the original assumption of positive coverage failed. Subsequent original-runtime
evidence and the [static mapping below](#sf2-static-gain-and-reverb-mapping) now
permit program modes retained as metadata and tone modes 0/4, with explicit
reverb approximation. The gain-fit checkpoint constructed 494 banks; subsequent
[silent-tone preservation](#silent-tone-preservation) raised that to 825 banks /
14,890 zones. The [US sample-reference resolution](#us-runtime-sample-reference-resolution)
now raises construction to 842 banks / 15,095 zones, each loaded in RustySynth.
The other 178 banks are rejected: 133 reads outside the verified pitch table,
41 zero-pitch contexts and four unrepresentable tuning ranges. Counts
classify the first rejection per bank, not all unsupported tones. These are
structural construction results, not full bank/song playback acceptance. Dynamic
controls, rejected contexts, reverb fidelity and music-folder publication remain
open; synthetic interoperability cannot authorize removing the C tool.

### US runtime sample-reference resolution

[The resolver](../../../tools/rust/bof3-audio/src/sample/reference.rs) models the
identified US ordinary note-on path, not a universal VAB convention. It retains
the encoded signed word, selected low byte, resolved one-based sample allocation
and address-table slot separately. The binder also retains the source PCM and
playback SF2 sample identities, including any synthetic silent sample.

Evidence uses the exact executable profile identified in
[the note-on evidence below](#original-note-on-gain-pan-and-tone-modes).
Offsets include the PS-X EXE header; original instructions execute in the Rust
machine without replacing the loader or note-on arithmetic.

| US runtime address | File offset | Observed responsibility |
| --- | --- | --- |
| `0x80173C50` | `0xDDC50` | SDK `SsVabOpenHeadSticky` entry. |
| `0x80173CB0` | `0xDDCB0` | Common bank loader; packs active program tone blocks and sample address slots. |
| `0x80173FD8` | `0xDDFD8` | Accumulates size-table entries, including entry zero, into sample starts. |
| `0x801730B8` | `0xDD0B8` | Tone selector reads the sample word and stores its low byte. |
| `0x8016FB18` | `0xD9B18` | Selects paired sample-address slots using signed division of `(byte - 1)` by two. |
| `0x801714D8` | `0xDB4D8` | Distinguishes byte 255 from ordinary PCM; noise entry is `0x80171C1C`. |

Byte zero aliases sample 2: division truncates toward zero and the even branch
reads program slot 0 at offset 14, the same slot as byte 2. Values such as 256
and −256 therefore select that same allocation. Byte 255 selects SPU noise and
is explicitly rejected by the PCM SoundFont binder. References beyond the
declared allocations also fail: an end-of-bank or adjacent SPU address does not
establish available sample media.

Size-table entry zero is a transferred prefix in eight-byte units. The first
sample starts after that prefix; later starts accumulate the declared sample
sizes. `Bank.body_prefix_bytes` and sample offsets retain this layout. The original
loader agrees with all 1,020 corpus banks, all 8,385 sample starts, packed tone
block indices and total transfer extents. All corpus prefixes are zero; synthetic
banks verify nonzero prefixes. An older catalog fixture assumed the field was
opaque and failed after the correction; its body and expected offsets now include
the prefix rather than suppressing that coverage.

The corpus has 25 raw zero references and one other out-of-range reference:
`BIN/BPLCHAR/DRG04_00.EMI` uses sample 3 with only two declared allocations.
Twenty zero aliases occur in the 842 constructed banks. The out-of-bank case
remains rejected if reached, but an earlier pitch failure masks it in first-error
counts; the absence of a sample-error category does not resolve every reference.
Constructed banks retain 5,397 silent tones across 332 banks.

[Five focused checks](../../../tools/rust/bof3-audio/tests/sample/reference.rs)
cover resolver limits, prefix offsets, 22 synthetic original-loader/note-on cases,
three noise-branch cases and the corpus loader comparison. Three require local
media. The synthetic note-on setup marks the bank ready after opening; it does
not prove DMA completion, SPU PCM, full game scheduling or audible equivalence.
Additional binder and silence checks verify raw-zero identity, identical resolved
PCM and prefix-independent sample decoding through RustySynth.

### Original note-on gain, pan and tone modes

[The gain model](../../../tools/rust/bof3-audio/src/voice/gain.rs) reproduces
initial left/right SPU volume registers for ordinary sequence note-on contexts.
It accepts explicit seven-bit bank/program/tone volume, velocity, channel volume,
sequence left/right volume, three pans and mono state. It does not assign those
contexts to songs or convert them into SF2 generators.

Evidence uses the original US executable SHA-256
`0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a`,
target `exe/slus_004_22`, payload load address `0x80096800`. The raw payload hash
is `677754d0d22c88151a5022cd98b8e89af1b0882177d9850faf62676eb7089eff`.
Bounded Rizin 1.0.0 disassembly (commit
`cc06c1dedbe47901faecc7e9825ca957aca81216`) was checked against Rust-machine
execution of the unmodified original instructions. Offsets below refer to the
full PS-X EXE, including its header.

| US runtime address | File offset | Observed responsibility |
| --- | --- | --- |
| `0x8017102C` | `0xDB02C` | Note-on: channel-volume scaling, matching tones, voice allocation and register staging. |
| `0x801736F0` | `0xDD6F0` | Select opened bank pointers and packed tone block for the logical program. |
| `0x801730B8` | `0xDD0B8` | Select all tones whose key ranges contain the note. |
| `0x801721FC` | `0xDC1FC` | Calculate stereo gain/pan, square amplitudes and stage pitch/volume. |
| `0x80172604` | `0xDC604` | Set or clear the selected voice's reverb bit from tone-mode bit `0x04`. |
| `0x801709C4` | `0xDA9C4` | Final flush block sends pending key-off, key-on and reverb masks to SPU registers. |

For nonnegative seven-bit inputs, division truncates after each step:

```text
effective_velocity = velocity * channel_volume / 127
bank_gain = effective_velocity * (16383 * bank_volume) / 16129
gain = bank_gain * program_volume * tone_volume / 16129
left = gain * sequence_left / 127
right = gain * sequence_right / 127
for pan in [tone_pan, program_pan, channel_pan]:
    if pan < 64: right = right * pan / 63
    else:        left = left * (127 - pan) / 63
if mono: left = right = max(left, right)
left_register = left * left / 16383
right_register = right * right / 16383
```

Pan 63 and 64 both leave the stereo amplitudes unchanged at that stage.
Successive pans can attenuate both sides; collapsing them into one pan before
the integer operations loses behavior. Squaring occurs after all three pans
and mono selection. This is not an equal-power pan formula.

In the tested note-on path, program-mode byte `ProgAtr + 3` is never read.
Tone-mode byte `VagAtr + 1` is copied to `0x8018E7EC`; bit `0x04` controls
the corresponding bit in masks at `0x8018DB54/0x8018DB56`, preserving other
voices. The original flush block reads the SPU base `0x1F801C00` from
`0x80184458` and writes those masks to `0x1F801D98/0x1F801D9A`, identified as
voice reverb enables by the
[SPU register reference](https://psx-spx.consoledev.net/soundprocessingunitspu/).
This proves routing, not reverb preset, depth, processing or audible equivalence
to a SoundFont consumer. Other tone-mode bits have no effect on the inspected
note-on outputs; their use on other runtime paths remains unestablished.

[Four checks](../../../tools/rust/bof3-audio/tests/voice/gain.rs) cover endpoints
and malformed contexts, 2,432 original-routine gain comparisons (1,408 single-field
sweeps plus 1,024 mixed contexts), all 256 program/tone mode values, and set/clear
flush writes for all 24 voices. Three checks require `BOF3_AUDIO_EXE`. The bus
witness records MMIO writes without synthesizing audio. Fixtures install a
controlled post-bank-open state; they do not execute the bank loader, full
initialization or scheduler. Special handle `0x21`, live controller updates,
non-seven-bit inputs, master gain, reverb processing and PCM remain outside this
model. The subsequent static fit below uses this arithmetic; dynamic-controller
comparison and full execution remain required.

### SF2 static gain and reverb mapping

[The gain fitter](../../../tools/rust/bof3-audio/src/soundfont/gain.rs) records the
source context, original register pair, chosen model, integer SF2 attenuation/pan,
predictions for both models and maximum channel error. Registers are normalized
by `16383 * sqrt(2)`, so full-volume game center maps to the selected unattenuated
equal-power center model. This is an explicit comparison scale, not calibrated
PSX output amplitude. A bounded search around the analytic angle/attenuation
checks quantized neighbors; no global optimum is claimed for consumer cutoffs.

- `SpecificationScale` uses the specified centibel attenuation scale with an
  explicitly selected equal-power pan curve. It does not establish a universal
  SF2 consumer pan implementation.
- `RustySynth136` models the inspected release's 0.4 attenuation multiplier,
  f32 arithmetic, channel-center offset and steady-state mixer cutoff at 0.001
  with master volume one. An engine-targeted fit changes loudness in consumers
  using specification-scale attenuation. Both predictions remain in the report.

`Gain::from_bank` binds bank/program/tone volume and tone/program pan at velocity
127, channel/sequence volume 127, channel pan 64 and stereo mode. Reference SF2
playback uses velocity 127, CC7/39 and CC11/43 all 127, CC10=64 and CC42=0.
Those controls are required for the reported static comparison; ordinary MIDI
defaults differ. Other explicit contexts can be fitted but their controls are
then baked into the fit. Silent register pairs return `parameters: None` and
zero predictions, requiring the explicit zero-PCM representation below;
finite attenuation is not portable exact silence.
Dynamic velocity, volume and pan behavior is not accepted by this reference fit.

Program mode is preserved without a generator because the verified ordinary
note-on path does not read it. Tone mode 4 requires `Options.reverb` containing
a send in 0..1000 tenths of a percent and provenance; no default game depth is
invented. Generator 16 receives that send for enabled tones and zero for mode 0.
CC91 must be zero to avoid an additional channel send. The caller's policy and
unresolved PSX preset/depth are recorded. Full send is an explicit approximation
used by the corpus probe, not recovered game behavior.

[Three fitting/consumer tests](../../../tools/rust/bof3-audio/tests/soundfont/gain.rs)
cover 60 quantized contexts, explicit silence/range handling, ten steady-state PCM
comparisons against RustySynth and dry/full/half reverb sends. Measured normalized
channel gains agree with the engine prediction within 0.00001. A first prediction
missed its per-channel mixer cutoff; the failed PCM check exposed this and the
model now includes it. A separate preliminary inference that reverb saturated at
generator 10 was disproved: the later 0.01 conversion makes generator 500 half
amplitude, with one-quarter wet-tail energy relative to 1000. The test retains
that correction. These are synthetic consumer comparisons, not original-game
reference audio. [Source provenance](../../reference/soundfont.md#gain-and-effects-consumer-differences)
retains the specification and inspected implementation locations.

### Silent tone preservation

The binder retains every original decoded sample in the SF2, including PCM used
only by muted tones. A bank that needs silence gets one additional, explicitly
labeled zero-PCM sample: 64 points, loop `[8,56)`, the declared export rate and
normal SF2 guard points. No game sample ID is invented for it. `Report.silence_sample`
identifies this playback-only sample; each tone retains `source_sf2_sample`,
`sf2_sample` and `silent` alongside its original sample reference and metadata.
Tones, key ranges, programs, layers, tuning rows and ADSR contexts remain present.

Silence is accepted only when the gain fit matches the original bank/program/tone
fields at maximum seven-bit velocity/channel/sequence volumes, center channel
pan and stereo mode. The verified nonnegative volume chain cannot increase a
zero register pair by reducing those controls or applying channel pan. A fit of
a temporary zero velocity or controller value cannot permanently silence an
otherwise audible tone. The binder recomputes retained fits and rejects
conflicting registers, generators or source contexts. Runtime changes to bank/
program/tone parameters and non-seven-bit controls remain outside this model.

Zero playback PCM stays silent regardless of consumer attenuation conventions,
pitch, envelope or effects. Gain edits alone cannot unmute that sample; relink
the zone to its preserved source PCM. Those source/playback identities prepare
reconstruction, but manifest publication and edited game packing remain open.
Invalid sample references, unsupported pitches, incomplete tuning and other
bank checks still reject silent tones instead of hiding unsupported execution.

[Two tests](../../../tools/rust/bof3-audio/tests/soundfont/silence/binding.rs) preserve distinct
source samples behind shared silent playback, check exact-zero PCM across 27
velocity/volume/pan combinations with sustain/reverb/release, restore audible
output by relinking a zone, and reject temporary/forged silence and invalid
sample/pitch contexts. The corpus binds 5,388 silent tones in 331 additional
banks. All 825 constructed banks' sample PCM is compared with RustySynth's read
data, including retained source samples and synthetic silence. This is artifact
and consumer evidence, not a completed archive round trip or game-audio oracle.

### SF2 note-on tuning calibration

[The tuning reference](../../../tools/rust/bof3-audio/src/voice/tuning.rs) executes
`exe/slus_004_22@0x80171B20` (full EXE offset `0xDBB20`) in private Rust-machine
RAM loaded from the exact-hash executable. It selects tone block/tone zero through
globals at `0x8018E7DF`, `0x8018E7E4`, `0x8018E25C` (file offsets `0xF87DF`,
`0xF87E4`, `0xF825C`) and supplies a synthetic tone record at `0x80010000`.
Each bounded call uses fine argument zero and runs original instructions through
return; no pitch callee or lookup table is substituted. This is isolated note-on
calibration, not a boot of the complete game audio runtime.

The report retains the original 16-bit result, witnessed lookup address/value,
declared-table index, unmodulated SPU step, chosen SF2 sample rate,
root/coarse/fine generators and signed cents
error. Following the [SPU pitch reference](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-adpcm-pitch),
the unmodulated step is capped at `0x4000`; source-point rate is
`step * 44100 / 4096`. Nearest-cent SF2 tuning compensates both the keyboard/root
interval and the explicit export sample rate. Single-key zones preserve the
template's sample, envelope, pan, volume, velocity range and loop choices. Sample
header pitch correction must be zero for this calibration contract.

The original routine can wrap its result to zero or index before its declared
193-entry table. Zero steps, reads outside both verified pitch tables and excessive SF2 generator
ranges remain explicit unsupported rows; requesting their zones fails. No key
is silently omitted. Reading adjacent EXE bytes in isolated RAM does not prove
their initialized-game state or give those accesses table authority.

[Corpus checks](../../../tools/rust/bof3-audio/tests/voice/tuning.rs) cover 21,112 tones
in 1,020 banks: 236,302 tone/key contexts, with 30,119 distinct key/center/shift
inputs and 1,549 distinct tone tuning/range contexts. Original execution agrees
with independent instruction-derived arithmetic for every result, including
out-of-table reads in the explicitly initialized test RAM. Counts are 3,177 zero
results, 32,977 results above the unmodulated cap, and 17,286 accesses outside the
note-on table (categories overlap). After accepting reads inside the separately
verified earlier pitch table, 4,976 rows remain unsupported;
supported rows have maximum absolute tuning error `0.499899535` cents.

These are complete declared tone ranges, not proof that game sequences select
every key. The error bounds compare source-point rates, not synthesized PCM.
Pitch modulation, noise, active-voice bend behavior, interpolation, ADSR and
full bank conversion remain separate work. Rizin snapshot identity was fresh;
the unrelated reverse-index coverage check was stale, so this evidence used
bounded disassembly of the identified raw payload rather than index queries.

Root-range follow-up (2026-09-24): root, coarse and fine generators jointly
encode the requested pitch. The converter keeps the source center as root when
it fits; otherwise it chooses the nearest legal root whose coarse/fine values
fit, preserving combined nearest-cent tuning. Source center/shift remain
separate reversible metadata. A true failure means no root in 0..127 works.

The corpus context key 121, center 46, shift 114 returns original register 192.
Keeping root 46 needs an out-of-range coarse generator. Root 53, coarse -120,
fine -98 represents the same requested pitch. RustySynth playback of a synthetic
64-frame sine verifies the resulting frequency within 0.2%; numerical tuning
error remains below half a cent. This resolves five tone/key rows across
BGM005A, BGM016A, BGM041, BGM131 and BGM143. Four formerly first-failing banks
then expose later pitch-table rejections; accepted complete banks remain 842.
Current first failures are 137 pitch-table contexts and 41 zero pitches.

#### Stopped-pitch SoundFont mapping

Strict tuning continues to reject zero-step rows. Bank conversion can explicitly
opt into key-on-only silence for known pitch-table zero steps with a zero held
initial sample. Music extraction uses this under `--allow-approximations`.
The source PCM remains in the SoundFont; per-tone binding `stopped_keys` records
the affected keys. XML emits `<stopped_key tone="T" key="K"
playback_sf2_sample="I" scope="key_on_only"/>` within the program. Song reports
include `playback_approximations`; both optional collections are omitted when
empty to preserve existing exports.

Unknown pitch lookups and nonzero held values fail. Repacking regenerates the
binding from preserved source bytes, validates each key's expected sample, and
retains unchanged encodings. Stopped-tone pitch/reassignment edits, generated
silence edits and source PCM edits that encode a nonzero initial held value are
unsupported. Other source PCM edits retain the usual encoding-loss report.
Later pitch changes can make the original voice audible while the SF2 zone stays
silent; original-runtime and consumer tests demonstrate this limitation. See
[retained evidence and corpus counts](../../reference/soundfont.md#stopped-pitch-approximation).

#### Adjacent pitch data and initialized state

The unsigned halfword load at `exe/slus_004_22@0x80171BEC` (full EXE offset
`0xDBBEC`) uses a **signed** semitone remainder. Negative indices are original
behavior, not a host MIDI-numbering or unsigned-arithmetic correction to make.
The 386 bytes at `0x80184284` (offset `0xEE284`) are an exact duplicate of the
declared table at `0x8018445C` (`0xEE45C`), with the same recorded pitch-table
SHA-256. A separate original routine at `0x8016986C` (`0xD386C`) loads the earlier
table at `0x80169920` (`0xD3920`). Executing that routine with explicit center,
fine, key and shift arguments agrees with note-on for 60 in-table cases.
This establishes a second table consumer, not initialized-state equivalence for
every negative note-on index. The local PsyQ 4.7 `vm_n2p.o` and `s_n2p.o` objects
have different layouts; they are not matching-game proof.

The earlier table ends at `0x80184406` (exclusive; offset `0xEE406`). The
86-byte gap before the note-on table includes mutable scheduler data. For
example, `0x80184440` and `0x80184444` (offsets `0xEE440`, `0xEE444`) initially
contain words 60 and 1. Executing original tick-mode setup at `0x8016D9CC`
(`0xD79CC`) with argument 1 changes them to 5 and 0 in isolated EXE RAM.
The game initializer's call at `0x8015CD24` (`0xC6D24`) supplies this argument
in its delay slot. Key 0, center 61, shift 16 reads `0x80184440`: the original
pitch return changes from 1 to 0 across that setup call. A subsequent argument-60
call restores that value to 60 and the pitch return to 1. These are bounded
original-instruction observations, not a complete sound initialization or timing
claim. The full initializer still stops at unsupported BIOS `A0:72` after 4,550
instructions in the current developer probe.

[Pitch-context checks](../../../tools/rust/bof3-audio/tests/voice/context.rs)
pin both consumers and this state-dependent counterexample. Tuning now witnesses
the actual halfword read and records its runtime address, full EXE offset, value,
region and `isolated_executable` state. It requires exactly one such load; the
table index is derived from that observed address rather than repeated host
pitch arithmetic. The complete tone/key corpus splits into **219,016** note-on
table reads, **12,623** reads in the earlier SPU table and **4,663** adjacent-data
reads. At that initial witness checkpoint, tuning and rejection counts remained
unchanged; the active-snapshot follow-up below supports the earlier table.

BGM000 program 2, tone 0, key 0 (center 83, shift 65) reads earlier-table
address `0x8018430C` (offset `0xEE30C`, value 5,235, resulting pitch 81), while
key 22 in the same declared range reads adjacent `0x8018444C` (`0xEE44C`).
Thus the duplicate alone does not justify accepting this whole tone. Rejection
diagnostics include the witness. Earlier-table reads are now supported per key;
adjacent runtime-data reads remain rejected, and no declared keys are dropped.
Scheduler-dependent values and sequence-reachable contexts remain required
evidence for those remaining failures.

#### Verified earlier-table reads

Both 193-halfword pitch tables are now checked against `US_PITCH_SHA256` when
constructing the exact-US tuning reference. A witnessed read inside either table
can produce an SF2 mapping; its signed index relative to the note-on table and
its actual address/region remain in the report. Reads into the intervening gap
still fail. BGM000 program 2/tone 0/key 0 now maps original register 81 from
`0x8018430C` (full EXE offset `0xEE30C`); its same-tone key 8 subsequently fails
at `0x8018440C` (`0xEE40C`, adjacent data), so the bank is not silently accepted.

The extended [initialized-state probe](../../../tools/rust/bof3-audio/examples/tuning_init_probe.rs)
executes original BGM004 sequence 0 for 1,700,000 reference-clock frames, watches
CPU store attempts overlapping either table, and checks both hashes afterward.
The final observer decodes instructions in RAM and immutable BIOS ROM; other
execution regions fail rather than silently escaping the observation.
It observes 78 table halfword reads at US `0x80171AE4` (full EXE `0xDBAE4`), no
table stores, and both original table hashes. IRQ admission is then disabled
for 30,119 isolated synthetic key/center/shift calls against that RAM snapshot;
output is drained while those bounded calls run. The same three adjacent
halfwords and 14 changed returns seen after initialization remain different
from raw EXE data. This is a post-playback snapshot, not simultaneous synthetic
gameplay or proof across every possible game state.

Receipt: `out/audio-migration/earlier-pitch-active-rom-observer.json`, including BIOS,
executable/archive identities, initialization calls, lookup witnesses and hashes.
Initial diagnostic attempts exposed IRQ interference and an undrained output
queue; the completed probe explicitly isolates and drains its synthetic phase.
All 12,623 earlier-table corpus rows now map within half a cent, reducing rejected
key rows from 17,599 to 4,976. Full-bank totals remain 842 strict / 843 with the
stopped-pitch approximation: remaining adjacent-data failures still reject those
banks. The complete PC corpus rerun preserves all 324 success/failure outcomes
and all 31 successful WAVs byte-for-byte; 244 diagnostics now identify a later
adjacent-data failure. See `out/audio-migration/earlier-pitch-pc-comparison.json`.
This does not close bank conversion or independent runtime-fidelity gates.

### Rust ADSR state and evidence models

[The Rust ADSR component](../../../tools/rust/bof3-audio/src/machine/adsr.rs)
advances explicit attack/decay/sustain/release state once per output-frame call.
It supports register decoding, rate counters, linear/exponential steps, frozen
rates, phase targets, signed level writes, retriggering and separate key-off /
forced-off behavior. A zero sustain level does not automatically turn a voice off.
The returned level is after the tick; the [voice owner](#rust-spu-sample-and-voice-execution)
uses the current level for output before advancing this envelope.

Callers must choose `Published` or `EmulatorReference` arithmetic. The references
disagree on exponential slowdown at the threshold and the ordering of the
slow-rate minimum; shared transition/write semantics follow the inspected
emulator. See [source provenance and exact differences](../../reference/spu-envelopes.md).
Neither model is accepted as hardware-validated. Sample interpolation and bounded
mixing are now connected below; MMIO timing, key-on delay and full runtime/device
integration remain unimplemented.

[Six focused tests and one opt-in corpus test](../../../tools/rust/bof3-audio/tests/adsr.rs)
cover literal phase traces, counter timing, rate aliases, mode disagreements,
signed writes and frozen/zero-level behavior. The corpus probe uses all 543 VAB
ADSR pairs across 21,112 tones with 250 ms held + 250 ms release trajectories.
It finds differences in 125 pairs / 395 tone records; both models finish release
within that probe for 19,595 tone records. Equal final status does not imply equal
trajectories. These bounded model comparisons do not establish gameplay ADSR
overrides, a general SF2 envelope approximation or reference-audio fidelity.

### Measured SF2 envelope fitting

[The envelope fitter](../../../tools/rust/bof3-audio/src/soundfont/envelope/mod.rs) approximates
one explicit `Published` or `EmulatorReference` ADSR model with SF2 attack, decay,
fixed sustain and release generators. It uses bounded coordinate search over
representable integer parameters, seeded by observed phase durations. It is
used by the explicit music export path below. A successful fit does not establish
hardware ADSR or whole-song playback acceptance.

The default probe holds notes for 0.25, 1 and 2 seconds, each followed by 2 seconds
of release at 44.1 kHz. It samples every 441 frames, plus the first 64 frames,
phase changes, key-off and endpoints. Error uses normalized positive envelope
level and elapsed-frame weights. Reports include weighted RMS, maximum error at
probe points, seed error, observed attack/decay boundaries, sustain range and
releases still active at the horizon. Maximum error between sampled points is
not bounded. Callers can supply up to 16 positive holds, each hold/release at
most 60 seconds, and a stride of 1–44,100; invalid probes fail explicitly.

The fit target is specifically RustySynth 1.3.6's volume envelope, including its
10 ms minimum release, exponential cutoff and float generator conversion.
Delay and hold use the SF2 zero-time value; neither game scheduler timing nor
engine sample-block timing is included. See [retained consumer-source evidence](../../reference/spu-envelopes.md#soundfont-approximation-target).
A constant SF2 sustain cannot reproduce a changing PSX sustain, and clipping a
parameter to the SF2 range does not establish representability of the original
envelope. The fit is an approximation with measured error, not a fidelity gate
or a globally optimal solution. Other SF2 consumers may differ.

[Four focused tests and an opt-in corpus check](../../../tools/rust/bof3-audio/tests/soundfont/envelope/cases.rs)
cover moving sustain, fast transients, frozen release, model selection, invalid
probes and the minimum release. A generated constant-sample SoundFont verifies
the target curve against actual RustySynth synthesis at selected points, with
effects disabled and a block-aligned key-off. Normalized error there is below
0.003; oscillator/filter startup and arbitrary sample-block phases are excluded.

The full corpus probe fits all 543 ADSR pairs / 21,112 tone records under both
models. Both find 393 pairs with changing sustain, 24 with a release still active
at the probe horizon and 15 without a completed attack. For `Published`, 65 pairs
used by 393 tones exceed normalized RMS 0.05; `EmulatorReference` gives 64 pairs /
391 tones. The largest RMS errors are approximately 0.182219 and 0.182201,
respectively, for `9BFF/4F8D`; its maximum sampled error is approximately 0.799204.
These substantial differences remain exposed for future bank conversion and
PC/PSX comparisons. Gameplay ADSR overrides, held durations outside the probes,
sample termination and gain/pan/mixer behavior remain separate work.

## Rust music extraction with explicit approximations

[`music_extract`](../../../tools/rust/bof3-audio/src/music/extraction.rs) connects
catalog/runtime associations, bank-to-SF2 binding and independent SEP translation:

```sh
bof3-audio extract --mode music --archive BGM004.EMI --executable SLUS_004.22 \
  --output NEW_DIRECTORY --allow-approximations --json
```

`--disc-root` or repeated `--archive` inputs are supported; `--id` selects one
source-qualified song (ambiguous numeric IDs fail). `--kind song` is optional.
`--loops N` controls each infinite controller-loop expansion, defaulting to two
traversals; zero is rejected. Music does not accept bank reference-rate or XA
arithmetic options. The approximation flag is required: this initial path is
inspectable interchange, not acceptance of complete musical semantics or fidelity.

Each song folder contains `bank.sf2`, one `sequence-NNN.mid` per independently
selectable sequence, and `song.xml` (`bof3.music-song/v1`). Each MIDI is format 1
with conductor and performance tracks; different SEP sequences are never merged
into simultaneous tracks. `music.xml` (`bof3.music-extraction/v1`) links qualified
song/bank identities and relative paths. Song XML distinguishes sequence index,
source sequence ID, game cue IDs, VAB header ID and runtime bank ID; program/tone
and source/playback sample mappings are explicit. Binding and timeline/translation
evidence are escaped JSON text within named XML elements. Every song manifest
also retains the complete source EMI, including unrelated entries and padding.
The root `extract.json` uses `bof3.music-extract-report/v1`.

The supported US executable supplies loader associations and isolated original
note-on pitch execution for every tone key. SoundFont construction preserves
layers and shared samples, uses bank 0 plus explicit bank-128 percussion aliases,
and rejects unsupported tone/sample/pitch contexts. The initial approximation
policy targets RustySynth 1.3.6 gain response at unity sequence/channel controls,
fits the published SPU ADSR model, permits marked fixed-PCM loop approximations
and exports dry reverb. Actual game reverb parameters are not inferred.

Generated tick-zero MIDI setup selects bank 0 and program equal to the channel
index (0–15), matching the original US SEP initializer. Channel volume is 127
and pan is 64; full expression, centered bend and dry effects remain export
contexts. Other gameplay overrides are not established.
Source-to-MIDI event indices account for the inserted setup events. Source
programs, CC7/CC10 and normalized bend values retain order, but standard SF2
controller curves and persistent channel bends still differ from tone-local,
existing-voice-only game behavior. Scheduler quantization, initial gameplay
state, dynamic-controller equivalence and measured PSX/PC PCM differences remain
open. These differences are reported, not silently treated as faithful conversion.

MIDI byte round trips, RustySynth's independent MIDI/SF2 parsers and explicit
preset checks precede publication. Missing presets cannot fall back silently to
GM timbres. Files and manifests are read back and published through the existing
staging helper; failures leave no output. Render an already expanded MIDI with
`--repeats 1`, followed by an explicit release tail, to avoid multiplying its loop
traversals again. One file play is now the generic PC renderer's default;
explicit `--repeats` requests additional whole-file restarts.

[Extraction checks](../../../tools/rust/bof3-audio/tests/music/extraction.rs) cover
two independent synthetic sequences, layered sparse programs, shared samples,
percussion aliases, tempo/controller/bend mappings, finite loop expansion,
archive preservation, compatible-consumer parsing/playback and rejected inputs.
BGM004 exports four independent sequences; indices 0 and 1 retain game cues 6
and 2 respectively. A retained CLI render of sequence 0 produces 66,150 stereo
frames (one-second cutoff plus 0.5-second tail), peak 0.077882, no clipped samples.
This is an executable consumption check, not a PSX audio comparison.

BGM000 and BGM002 currently fail because tone key 0 reaches outside the verified
pitch-table context. Their rejected tone ranges are retained rather than silently
omitted. Resolving these initialized-game pitch contexts, complete music coverage,
semantic music packing and full playback acceptance remain required. Logs and
the BGM004 review artifacts are under `out/audio-migration/music-extract-*`.

### Music-folder packing

`pack --mode music --input FOLDER --executable PSX_EXE --output NEW_DIR [--json]`
accepts one `song.xml` folder or a `music.xml` extraction root.
[The packer](../../../tools/rust/bof3-audio/src/music/packing.rs) needs the original
supported executable but can reconstruct without the original EMI files: the
complete archive is retained in each song manifest. Paths resolve under the input
root; MIDI/SF2 references may be renamed. Duplicate manifest paths, duplicate song
identities, duplicate MIDI paths, ambiguous root/folder manifests and disagreeing
preserved copies of one source archive fail before publication.

[Shared preparation](../../../tools/rust/bof3-audio/src/music/document.rs) owns
the exporter and packer's original-source bank binding, SoundFont construction,
sequence translation and canonical song metadata. Packing regenerates these from
the preserved archive, supported executable and declared positive loop limit;
XML mapping claims cannot authorize an edit. The
[manifest validator](../../../tools/rust/bof3-audio/src/music/manifest.rs) checks
schema, runtime/export contexts, qualified identities, sequence headers, source
hashes, generated setup counts, bank/program/tone/sample mappings and complete
binding/timeline/translation JSON. Unknown fields, altered evidence, missing or
reordered children and scalar changes fail. JSON formatting may change; descriptive
limitation strings are validated as notes but are not reconstruction inputs.

SoundFonts pass the bounded PCM/loop and tone-control inverses described above;
other instrument and metadata edits still fail explicitly. MIDI files are parsed and passed to the
event-preserving inverse; hashes stored in XML describe the original export, not an
authorization to ignore edited contents. Edited source program values must exist
in the paired VAB. Bank headers change only validated tone volume/pan, center/shift
and sample-reference bytes and retain sample geometry, layering and allocations; source tone
contexts are revalidated during bank
binding. Rebuilt sequences must reproduce the requested MIDI events under the
same bounded translation, including agreement between repeated source visits.

Multiple songs sharing one bank/source merge their selected SEP, bank-header and
sample-body replacements into one archive. Every selected song sharing a header
or body, including an unchanged copy, must request identical rebuilt entry bytes.
[Shared archive staging](../../../tools/rust/bof3-audio/src/archive/packing.rs)
checks every replacement and unrelated entry, requires whole-file byte equality
when unchanged, reparses read-back files and publishes only after report read-back
verification. Changed entries refresh the EMI cached first word; unchanged entries
retain even a stale cache. Allocations, padding and unrelated data are preserved.
Source strings are identities, never output paths: generated files use
`archives/<source-name-sha256>.EMI`. `pack.json` has schema
`bof3.music-pack-report/v1`, with separate archive equality/change, per-sequence
reconstruction and per-song SoundFont sample/loss and tone-control reports. Existing destinations
survive failed publication.

[Packing tests](../../../tools/rust/bof3-audio/tests/music/packing.rs) exercise
standalone/root reconstruction, renamed references, two independent sequences,
layered programs, two songs sharing one bank, supported MIDI, sample and tone-control edits,
source-copy/shared-bank conflicts, metadata tampering, unsupported instrument,
program edits, inconsistent timing requests, escaped paths
and failed publication. Original BGM004 (four sequences) round-trips byte-exactly
after deleting the copied source EMI. These checks do not establish whole-music
corpus conversion, arbitrary SF2 reconstruction or independent runtime/audio
fidelity; initialized pitch-context failures still restrict song export. Evidence
is under `out/audio-migration/music-pack-*`, `out/audio-migration/sf2-pack-*`
and `out/audio-migration/tone-pack-*`;
C retirement remains gated.

## Rust PC interchange rendering

The [PC renderer](../../../tools/rust/bof3-audio/src/pc_render.rs) and
[CLI](../../../tools/rust/bof3-audio/src/render_cli.rs) synthesize supplied SMF/SF2
through the approved, pinned RustySynth 1.3.6. This is an executable playback path
for interchange assets, including the explicitly approximate song export above.
Complete game-to-PC playback acceptance remains unfinished.

```sh
bof3-audio render --mode music --engine pc --midi song.mid --soundfont bank.sf2 --output new-render --json
```

The new output directory contains stereo PCM16 `render.wav` and versioned
`render.json`. Defaults are 44,100 Hz, one file play, a two-second
release tail and a 600-second output safety limit. `--repeats` controls file
restarts, each resetting engine state; it does not identify or expand BOF3 loop
regions. `--duration` sets the body before the tail. After the requested file
plays, remaining body time advances the final synth state without another
restart; notes/releases/effects continue according to the file's final events.
The report records this interval as `post_sequence_frames`. `--tail`, `--timeout`
and `--sample-rate` are explicit controls. Durations
are rounded to output frames; the safety limit includes the tail and fails before
publication. Output must be absent; staged WAV/report bytes are read back and
verified before the shared publication helper renames the directory.

Playback validation is stricter than the preservation codec. It accepts notes,
programs, bends, supported bank/modulation/volume/pan/expression/sustain/effect
controllers and supported all-notes/controllers operations. NRPN/RPN, pressure,
SysEx, unknown semantic metadata and malformed metadata fail with track/event/tick
diagnostics. Format-1 tempo changes must be on track zero. Source text metadata
does not affect synthesis and is counted. MIDI/SoundFont input hashes identify
the supplied assets. Every audible note must select a present SF2 bank/program;
the engine's fallback timbre substitution is rejected. Channel 10 uses the
engine's percussion-bank convention, starting at bank 128.

Scheduling accumulates rational PPQN time across tempo changes, then floors the
total to output frames. Same-tick order is track index followed by event order.
The engine uses eight-frame blocks, so changes may be delayed up to seven
additional frames. The report records output/body/tail frames, requested/started
repeats, duration cutoff, safety limit, event counts, floating-point peak and
PCM16 clipping. Settings are 256 voices, effects enabled and master volume 0.5;
voice stealing, engine SF2 approximations, undithered PCM quantization and a fixed
tail shorter than some envelopes/effects are disclosed. This is not PSX timing
or waveform acceptance. Archive rendering is described below; MP3/Ogg delivery
remains unsupported. The harness now forwards audio commands to Rust.

[PC renderer tests and the opt-in consumer check](../../../tools/rust/bof3-audio/tests/pc_render.rs)
cover tempo/order and fractional-frame accumulation, 100/200 Hz synthetic output,
repeat equality, live pan/bend changes, release, clipping, rejected events/presets/durations, CLI cutoff,
publication conflicts and absence of output on failure. The independent engine
MIDI reader agrees on the 1.5-second tempo fixture. FFmpeg 6.1.1 independently
decodes the exported WAV to identical PCM16 in the opt-in test; it is never a
production renderer or dependency. These synthetic checks do not measure
BOF3-to-SF2 translation or PC/PSX differences.

### Direct PC archive rendering

The [archive adapter](../../../tools/rust/bof3-audio/src/pc_archive.rs) shares
canonical music preparation with extraction. It requires one song entry in the
selected EMI, its supported US executable, an independent SEP index, and explicit
acceptance of the same MIDI/SF2 approximations:

```sh
bof3-audio render --mode music --engine pc --archive BGM004.EMI \
  --executable SLUS_004.22 --sequence 0 --allow-approximations \
  --loops 2 --output new-render --json
```

The default expands two infinite-loop traversals, retains the intro once and
preserves encoded finite loop counts. It then renders the resulting MIDI once,
with no synth reset between loop traversals. `--duration` and `--loops` are
mutually exclusive. Fixed duration expands enough traversals to cover the body,
checking actual rational-tempo scheduling after an initial loop-period estimate;
the existing event/tick bounds reject runaway expansion. A non-looping source
ends once, then its final synth state advances until the requested body boundary.
The common release tail and output safety limit still apply.

Only the selected sequence is translated and checked for playable presets;
unsupported events in another independently selectable sequence cannot block it.
Bank conversion still validates the complete bank. MIDI/SF2 inputs and whole-file
`--repeats` form a separate input mode; BIOS and layout are PSX-only options.
Archive output uses `bof3.audio.pc-archive-render/v1`, retaining source/executable
hashes, qualified song/bank/sequence IDs, translation stop reason and mappings,
expanded loop limit, approximations and the nested PC rendering report.

[Archive rendering checks](../../../tools/rust/bof3-audio/tests/pc_archive.rs)
compare PCM against extracted MIDI/SF2 played once, retain independent sequence
selection, verify intro/finite/infinite-loop and fixed-duration behavior, and
exercise option failures and verified publication. These establish shared
translation and control behavior; they do not resolve dynamic controller/bend
differences, scheduler timing, unresolved bank contexts or independent PSX PCM
fidelity.

The current 2026-09-24 CLI probe covers all 324 physical sequence slots in 81
BGM archives: 32 render successfully and 292 reject unresolved pitch-table
contexts. Verified channel defaults and explicit empty-program presets enable
BGM053 sequence 0; all 31 previous short WAVs remain byte-identical.
Of the 165 selections in the verified gameplay cue table,
11 render and 154 reject. Successful short probes contain 2,646 frames at
44,100 Hz (0.05-second body plus 0.01-second tail); failed selections publish
nothing. Receipts are `out/audio-migration/pc-archive-corpus-program-init/summary.json`,
`program-init-pc-comparison.json` and `pc-archive-corpus-index.json`; this distinguishes physical slots from proven
gameplay selections and does not imply complete music coverage.

A two-loop BGM004 sequence-0 comparison uses the same archive SHA-256
`da72a55d21e9578545c2353a13c462db607b787891e382fbc4d2d8bb7aa8144c` and the
supported US executable. PC body length is 1,682,690 frames; Rust-machine PSX
body length is 1,682,857, a PC-minus-PSX difference of -167 frames (-3.786848 ms).
Both add a 22,050-frame release tail. Full-output PCM16 peaks are 6,720/17,934
and RMS values 1,004.50/2,789.81 (PC/PSX). Unshifted common-span PCM RMSE is
2,951.02; no alignment, gain normalization or lossy encoding is applied, and
this is not a perceptual score. `out/audio-migration/pc-psx-bgm004-comparison.json`
pins both WAV hashes and links the complete reports. The PSX side still uses
unverified reference clocks, so this is an implementation comparison, not an
independent audio-fidelity oracle.

## Rust SPU RAM transfers and DMA4

[SPU transfer state](../../../tools/rust/bof3-audio/src/machine/spu_transfer.rs),
[DMA4](../../../tools/rust/bof3-audio/src/machine/spu_dma.rs) and the
[interconnect](../../../tools/rust/bof3-audio/src/machine/interconnect.rs) now
provide normal 512 KiB sound RAM, a 32-halfword FIFO, manual writes and request-mode
DMA in both directions. `service_spu` explicitly applies pending control and
services a bounded number of halfwords. It does not advance CPU clocks or model
bus wait states; CPU stepping alone does not service the transfer device.

The [SPU transfer reference](https://psx-spx.consoledev.net/soundprocessingunitspu/#spu-memory-access)
describes eight-byte TSA units, a separate advancing RAM address, FIFO requests
and normal `RAM_CTRL=4`. The model retains programmed TSA while wrapping the
internal byte address. DMA completion can precede FIFO drain. The
[memory-control reference](https://psx-spx.consoledev.net/memorycontrol/#1f801014h-dev4-spu-delaysize-200931e1h-use-220931e1h-for-spu-ram-reads)
identifies `DEV4_CTRL=0x200931E1` for ordinary access and `0x220931E1` for stable
reads. The interconnect accepts word writes of these two configurations, retaining
them for original read/modify/write helpers; other configurations and partial
accesses fail explicitly. DMA reads reject the unstable ordinary-access setting.
References checked 2026-09-24 UTC; no hardware transfer trace has been accepted.

DMA4 supports 1–16 words per request slice, incrementing/decrementing RAM
addresses, RAM mirrors, block-count zero as 65,536 blocks, DPCR gating, cancellation
and completion/per-slice IRQ notification through the existing DMA controller.
CPU SPU ports support aligned halfword/word accesses and the established even-SB
halfword/odd-SB-ignore behavior. Unsupported transfer modes, FIFO overflow,
manual reads, IRQ9 enable, buffered direction changes/stops, masked MMIO and
unimplemented hardware fail explicitly. The frame-driven voice component below
reads this RAM; capture and reverb accesses remain unimplemented.
Transfer status is component state, not complete
SPUSTAT emulation or proof of IRQ latency.

### Original US transfer dispatcher

All addresses below belong to the exact-hash `exe/slus_004_22` profile from
[startup evidence](#verified-us-sound-startup-and-sequence-storage). File offsets
include the PS-X EXE header (`address - 0x80096000`). Bounded Rizin 1.0.0 disassembly
of the identified payload established these paths; symbol tables are unchanged.

| Runtime address | Full EXE offset | Observed instruction behavior |
| --- | --- | --- |
| `0x80168960` | `0xD2960` | Variadic dispatcher: command 2 shifts the byte address by the global shift at `0x80183AD8` (originally 3), saves TSA at `0x80183AC8` and writes the transfer-address port. |
| `0x801689F0` / `0x80168A60` | `0xD29F0` / `0xD2A60` | Commands 1/0 select upload/download, retain direction at `0x80183B00`, check TSA and set transfer-control bits. |
| `0x80168ACC` | `0xD2ACC` | Command 3 checks control, calls the direction-specific bus helper, rounds byte length up to 64, writes MADR/BCR and starts DMA4 with `0x01000201` or `0x01000200`. |
| `0x80168E68` / `0x80168E94` | `0xD2E68` / `0xD2E94` | Original helpers adjust DEV4 DMA timing bits for upload/download while retaining other fields. |

The dispatcher preserves the CPU pointer at `0x80183B04` and rounded block count
at `0x80183B08`; DMA MADR itself retains only 24 bits. A nonmatching control value
eventually returns `-2` without launching DMA. A rounded upload includes bytes
beyond the requested length, so future callers must establish readable padding;
the test initializes that entire range deliberately.

[Seven synthetic tests plus an opt-in original-executable test](../../../tools/rust/bof3-audio/tests/spu_transfer.rs)
cover these transfers and explicit failures. The original dispatcher and its
callees execute unchanged for 16 upload/download pairs: lengths 1, 63, 64, 65,
127, 128, 129 and 256, each at an ordinary address and across the RAM boundary.
All rounded bytes return exactly, with expected registers and preserved ABI state.
The paired calls execute 5,152 instructions; the separate timeout case executes
30,760. This controlled initialization and explicit device servicing do not
establish full game boot, interrupt-handler execution, hardware timing or PCM
fidelity. Local disassembly and checks are retained under
`out/audio-migration/spu-transfer-*`; proprietary instructions are not copied into
the source fixtures.

## Rust SPU sample and voice execution

[The sample reader](../../../tools/rust/bof3-audio/src/machine/spu_sample.rs)
now fetches ADPCM blocks from the shared 512 KiB SPU RAM, preserves decoder and
three-sample interpolation history across blocks and loops, handles repeat-address
updates and RAM wrap, and reports end/mute flags after consuming the final frame.
Key-on resets prediction and interpolation histories while retaining the repeat
register. Addresses use eight-byte units with the low block-address bit ignored.
Reserved decoder headers still fail explicitly. Whole-block fetches are snapshots;
concurrent RAM writes, internal fetch cadence and IRQ comparisons are not modeled.

The four-tap interpolation ROM contains 512 signed values, independently compared
between the published reference and inspected emulator. Its little-endian `i16`
SHA-256 is `3e221a5b25de34ca9ad1ea945ad90fbe8171c5e3f7c52af15b232f9c36fbd40d`.
No emulator implementation or new dependency is included. Callers must select
`Published` (separate product rounding, pitch ceiling `0x4000`) or
`EmulatorReference` (sum rounding, ceiling `0x3FFF`). Both retain signed pitch
modulation and low-16-bit wrapping before clamping. These are conflicting source
models, not accepted hardware alternatives; [retained research](../../reference/spu-samples.md)
records the discrepancy and remaining evidence obligation.

[Voice execution](../../../tools/rust/bof3-audio/src/machine/spu_voices.rs) connects
24 sample readers to explicitly selected ADSR models, manual envelope writes,
fixed signed stereo gain, key-on/off and end flags. Pitch modulation uses the
preceding voice's post-envelope signal before stereo gain; voice zero is excluded.
Repeat-address writes affect future jumps, while sample-start writes take effect
on the next key-on. A mute flag zeros the envelope after the current ADPCM output;
noise-selected voices ignore that mute flag while retaining sample-address and
end-flag progression.
Started voices continue sample progression after envelope silence; no inactive
voice IRQ behavior is claimed.

The interconnect requires explicit `configure_spu_voices` model selection before
voice MMIO becomes available. Eight registers per voice and key/modulation/reverb/
end masks share the existing SPU width/alias handling. `step_spu_voices` applies
pending key latches at a caller-selected boundary and returns per-voice outputs
plus the reverb-send mask. It reads the RAM written by FIFO/DMA transactions.
This intermediate API does not apply final mixing or capture; the separate
[output-frame API](#rust-spu-noise-mixing-and-capture) now owns those operations.
Noise needs explicit initial state. Unsupported flag accesses and disable
transitions without a [selected model](#rust-spu-disable-and-original-hardware-initialization)
fail explicitly. Key timing, retrigger spacing,
initial inactive fetches and CPU/device synchronization remain open. An execution
error can follow earlier voices' updates; callers must stop rather than retry a
partially executed frame.

[Sample checks](../../../tools/rust/bof3-audio/tests/spu_samples.rs) cover all 256
interpolation phases with independently calculated signed boundary vectors,
modulation glitches, block/loop transitions, zero pitch, address wrap, retrigger
history and malformed data. [Voice checks](../../../tools/rust/bof3-audio/tests/spu_voices.rs)
cover all 24 voices, gain polarity, envelope ordering, modulation before panning,
release and mute. BGM000/BGM004 checks traverse 37 original samples, including
10 repeated loops, for 808,388 frames, comparing sample progression against
linear decoding. This uses reference pitch `0x1000`, not verified game playback
contexts or independent reference audio.

[MMIO checks](../../../tools/rust/bof3-audio/tests/spu_voice_ports.rs) execute the
original US final register-flush block at `exe/slus_004_22@0x801709C4` through
`0x80170A34` (exclusive; full EXE offset `0xDA9C4`). It loads staged key-off masks
from `0x80190C58/5A`, key-on masks from `0x8018DB50/52` and reverb masks from
`0x8018DB54/56`, clears the staged key masks and writes SPU ports through the base
at `0x80184458`. Each of 24 synthetic staged voice selections reaches the actual
Rust MMIO latches, starts fetching an uploaded ADPCM block and preserves its
reverb route. Original instructions are unchanged; bank loading, scheduler calls,
full flush-function execution and final audio fidelity are not established.

## Rust SPU noise, mixing and capture

[The noise component](../../../tools/rust/bof3-audio/src/machine/spu_noise.rs)
implements the published shared 16-bit generator and six-bit SPUCNT clock. The
caller supplies its initial level and countdown; no original boot phase is
inferred. A frame uses the old noise level, then clocks the generator once,
including frames with no selected noise voices. NON, key-on and pitch changes do
not restart or retune this shared generator. VxPitch still advances the selected
voice's ADPCM address and ENDX state. Noise replaces the interpolated sample
before ADSR, stereo gain and next-voice pitch modulation. The inspected emulator
uses a different fractional timer; hardware agreement remains unverified. See
[retained noise and mixer research](../../reference/spu-samples.md#noise-mixing-and-capture).

[The mixer](../../../tools/rust/bof3-audio/src/machine/spu_mixer.rs) preserves wide
voice sums and separate reverb sends, signed CD/external input gains, voice mute,
fixed main gain and its independently writable current-volume registers. Voice
mute suppresses voice dry/wet sends while enabled CD/external inputs remain
routed. Input reverb flags cannot enable an otherwise disabled input. Reverb
input is clamped independently; the accumulated dry plus scaled reverb return is
clamped before main gain. That ordering follows the inspected emulator, not an
accepted hardware trace. The unverified final `+32768` full-negative-gain rail
fails explicitly. Main/voice sweeps now use the component below.

`Interconnect::step_spu_output` executes one caller-scheduled frame and returns
stereo PCM. It requires configured voice models, SPUCNT enable and normal RAM
control `0x0004`; `configure_spu_noise` additionally supplies explicit noise state.
Inputs must already be at 44.1 kHz after drive-side processing. This is not a CD
device, XA resampler or CPU-clock scheduler. Use either this API or the intermediate
voice API once per frame, never both. Enabled reverb or either nonzero return gain
fails before voice execution unless an explicit reverb model is configured:
disabled reverb can still emit stored output, so a dry fallback would lose audible
data. The [reverb engine](#rust-spu-reverb-execution) now connects the separate
mixer return boundary to shared RAM. Later execution errors can follow
partial voice advancement and require stopping, not retrying the same frame.

Completed output frames write four independent 1 KiB capture rings in SPU RAM:
CD left/right at `0x000/0x400` before SPU input gain, and voices 1/3 at
`0x800/0xC00` after ADSR but before pan/mute. A shared two-byte cursor wraps after
512 frames and supplies STAT bit 11 for its current half. This component starts
the cursor at zero; original boot position, capture bus cadence and IRQ comparison
are not established. Sample reads precede these explicit frame writes.

[Noise tests](../../../tools/rust/bof3-audio/tests/spu_noise.rs) compare 65,536
frames at each of 64 clocks against an independently calculated Python integer
trace (little-endian samples SHA-256
`734409073f749d57a47b078e194d6f9377bd137aff6d3a635f9d3aa843f363c7`).
[Mixer/capture tests](../../../tools/rust/bof3-audio/tests/spu_mixer.rs) cover
clipping order, signed gains, mute/input/send routing, tails, register failures
and ring wrap. [Output integration tests](../../../tools/rust/bof3-audio/tests/spu_output.rs)
exercise all 24 noise voices at different pitches, continued ENDX without noise
muting, per-frame clocking, capture before pan/mute/input gain and rejection before
pending keys are consumed. These ten new checks validate component behavior;
independent hardware traces/audio, CD execution, timing and full
original-game runtime acceptance remain required.

## Rust SPU volume sweeps

[The volume component](../../../tools/rust/bof3-audio/src/machine/spu_volume.rs)
retains raw registers, signed current gains, rate counters and terminal state for
both channels of each voice and the main mixer. Fixed writes expand signed
15-bit gain; sweep writes preserve that current gain and restart their counter.
Linear/exponential, increasing/decreasing and phase-inverted modes share the
ADSR rate calculation and its explicitly selected arithmetic model. Rate `0x7F`
freezes without clamping. Endpoint saturation stops a sweep; a subsequent main
current-level write does not rearm it. Key-on does not reset stereo gain.

Voices use their current gains before advancing sweeps; final mixing similarly
advances main sweeps after output. `configure_spu_voices` binds the main sweep
arithmetic as well as voice envelopes. Standalone mixer sweeps require explicit
configuration; fixed gain needs no model. Current voice-volume reads at
`0x1F801E00..0x1F801E5F` expose signed levels with the existing SPU width/alias
rules. Writes to those ports remain unsupported for lack of hardware evidence;
main current-volume writes remain available.

These are scheduled effective writes, not a claim about hardware write latency.
The component clocks voice sweeps on every requested frame, including inactive
voices. That is an interpretation of the published per-frame generator; the
inspected emulator can skip inactive voices when IRQ9 is disabled. This difference,
write timing and the existing arithmetic disagreements require independent
hardware evidence. See [retained sweep research](../../reference/spu-envelopes.md#volume-sweeps).

[Sweep checks](../../../tools/rust/bof3-audio/tests/spu_volume.rs) cover all eight
modes, 128 rates and five signed starting levels across 512 frames per context,
against separately calculated integer traces for both models. Literal checks
cover saturation, frozen rates, reset/rearm behavior and model boundaries.
[Integration checks](../../../tools/rust/bof3-audio/tests/spu_output.rs) verify
voice/main output ordering, key preservation and all 24 current-volume register
pairs. Existing ADSR corpus results are unchanged: 543 pairs, 125 differing pairs
and 395 affected tone records. Passing these checks does not establish original
runtime timing or reference-audio fidelity.

## Rust SPU reverb execution

[The reverb engine](../../../tools/rust/bof3-audio/src/machine/spu_reverb.rs)
implements stereo reflections, four comb paths and two all-pass stages through
the shared SPU RAM. The interconnect maps ESA and all 32 preset registers after
`configure_spu_reverb(EmulatorReference)`. This explicit model selects inspected
emulator quantization, saturation, channel ordering and address folding; it is
not an accepted hardware implementation. No emulator code or dependency is imported.
See [external evidence and discrepancies](../../reference/spu-samples.md#reverb).

The 22.05 kHz network receives and returns 44.1 kHz frames through the published
39-tap FIR. Initial filter histories and phase are zeroed component state, not
recovered BIOS state. ESA writes reset the RAM cursor, preserving filter history.
SPUCNT reverb enable gates RAM writes while reads, output, filters and cursor
continue. Wet gain remains in the mixer, after the reverb return. Capture follows
reverb execution at the explicit frame boundary; bus arbitration and IRQ9 remain
unmodeled. A configured engine runs even when wet gains are zero, preserving
state for later volume changes. An unconfigured audible reverb path still fails.

The source model's address calculation adds the base once on overflow and masks
to 18 halfword-address bits. Large or negative offsets can alias below the nominal
work area; this differs from interpreting the published description as modulo by
work-area size. The published Room preset includes zero reflection destinations,
so this is a material validation question. The model preserves the alias rather
than silently substituting different memory behavior. Aliased right-channel
reads can observe left-channel writes from the same network step.

[Reverb tests](../../../tools/rust/bof3-audio/tests/spu_reverb.rs) cover signed
rounding, cross-channel reflection, write-disabled output, FIR gain/phase,
cursor wrap, history retention, alias order and the special `-32768` reflection
coefficient. A separate scalar integer calculation matches 88,200 Room frames
with periodic asymmetric impulses, including 44,100 frames with RAM writes
disabled. PCM SHA-256 is
`e847c13ababdb98655b02a9ddb408361a517572a621bd3f029b2a80ef4eee6d0`;
final RAM SHA-256 is
`63861ac1df8d3e27ddfc746fa1bcc5b18337702ba7b2d50481e531b84bbc46b3`.
The exact-hash US executable contains the 32 published Room register values at
full-file offset `0xEE024`, runtime `exe/slus_004_22@0x80184024`. This is preset
identity, not proof that a game path selected or executed that preset.

[Output integration checks](../../../tools/rust/bof3-audio/tests/spu_output.rs)
upload stored audio through the SPU FIFO, read it through the configured reverb,
observe wet/main gain and preserve RAM while writes are disabled. Independent
game traces and hardware audio, exact rounding, address aliases, startup phase,
CPU/device timing and original preset-selection execution remain acceptance work.

## Rust CD decoder host registers

[The host interface](../../../tools/rust/bof3-audio/src/machine/cd_host.rs) maps
the four banked byte registers at `0x1F801800..0x1F801803` after explicit
`Interconnect::configure_cd_host` configuration. The supplied volume coefficients
and empty FIFO/interrupt state are a test/execution context, not recovered BIOS
defaults. The existing physical/cached/uncached I/O aliases apply. Registers use
byte accesses; the separately configured data port also supports halfword reads.

Command submission captures a byte and sets BUSY; `take_cd_command` transfers
the parameter FIFO to the separate drive owner. `stage_cd_response` publishes
response bytes and clears BUSY; `raise_cd_interrupt` signals them independently.
`respond_cd` is convenience for explicitly coincident boundaries, not an assumed
latency. The host does not interpret commands, invent disc status or advance
clocks. No command automatically receives a success response.

Parameter capacity is 16 bytes. Result reads retain padded 16-byte wrap while
readiness clears after the actual payload. Odd banks expose interrupt flags,
even banks their mask; acknowledgement and parameter clearing are separate from
result consumption. IRQ2 enters the existing edge-latched interrupt controller.
ATV/ADPCTL writes use the existing drive-volume component. Retained
[register research](../../reference/cd-audio.md#cd-decoder-host-interface)
is external evidence, not full hardware acceptance.

Overlapping commands/responses, parameter overflow, absent configuration,
decoder reset and manual XA controls fail explicitly. Sector requests and data
reads require the separately configured data FIFO described below.
Responses require 1–16 bytes and interrupt type 1–5. A later response cannot
replace one awaiting delivery, acknowledgement or draining. Reading the response
before its interrupt does not permit another command to overwrite that state.
Drive-command execution, response queues/latencies and XA
playback status remain implementation obligations.

[Six host checks](../../../tools/rust/bof3-audio/tests/cd_host.rs) exercise FIFO
capacity/wrap, busy/readiness boundaries, mask/partial acknowledgement, IRQ
relatching, volume application/mute and rejected accesses. The
[original US driver check](../../../tools/rust/bof3-audio/tests/cd_host_runtime.rs)
runs command submission at `exe/slus_004_22@0x80176734` (full EXE `0xE0734`)
for Getstat, Setloc, Setmode and Setfilter. It verifies parameter bytes and return
values, then executes response handling at `0x80175C60` (`0xDFC60`) to drain and
acknowledge explicit input. A prior completion response establishes SDK idle
state through that original handler; no completion flag is patched.

The fixture supplies a read-only GPU-status value of zero for `VSync(-1)` at
`0x80174700` (`0xDE700`), whose instructions read GPU status before returning
the software counter. This is an explicit isolated input, not a GPU model,
display clock or full callback/interrupt lifecycle. Rizin 1.0.0 inspection uses
the existing hash-verified US payload at base `0x80096800`; logs and bounded
instruction listings use `out/audio-migration/cd-host-*`. Fresh full startup
still stops after 4,550 instructions at BIOS `A0:72`. BIOS state, complete CD
execution, scheduler timing and independent audio fidelity remain open.

## Rust CD sector data and DMA3

[The data FIFO](../../../tools/rust/bof3-audio/src/machine/cd_data.rs) accepts an
explicitly supplied 2,048- or 2,340-byte block. It does not select sectors or
schedule their arrival. Configuration selects `Model::EmulatorReference`: repeated
BFRD assertion preserves the read cursor, clearing BFRD rewinds a partial block,
and exhaustion clears readiness and the consumed block. Byte, halfword and DMA
word reads share one cursor. Direct presentation rejects unread-block replacement;
the serialized INT1 selection path below can displace an unrequested block.
Unavailable data and overreads fail explicitly, without fabricated padding. The
[external references](../../reference/cd-audio.md#sector-data-and-dma3) disagree
about overread values; this model is not hardware acceptance.

[DMA3](../../../tools/rust/bof3-audio/src/machine/cd_dma.rs) supports burst-mode
device-to-RAM transfers, incrementing or decrementing addresses, explicit DPCR
enable and existing DICR/IRQ acknowledgement. Zero BCR count means 65,536 words;
non-chopped bursts preserve visible MADR/BCR while the internal cursor advances.
Low address bits are retained but ignored for word access. Addresses wrap at
24 bits; access outside the 8 MiB RAM mirror region reports a bus error.
Chopping, request mode, reverse direction, pause and snooping remain unsupported.
`Interconnect::service_cd` supplies a word budget, not elapsed clocks, arbitration
or CPU stalls. A failed overread retains prior writes and does not signal completion.

The original US `CdGetSector` at `exe/slus_004_22@0x80175A78` (full EXE offset
`0xDFA78`) calls the transfer helper at `0x80177154` (`0xE1154`). Its instructions
request data, write DEV5_CTRL=`0x00020943` and COM_DELAY=`0x1323`, enable DMA3,
set MADR/BCR and CHCR=`0x11000000`, wait for completion, then restore
COM_DELAY=`0x1325`. The Rust bus accepts those observed control configurations;
other bus timings are not inferred. Bounded Rizin 1.0.0 listings end at helper
address `0x80177250` and use `out/audio-migration/cd-sector-*-rizin.json`.

[Six device tests](../../../tools/rust/bof3-audio/tests/cd_sector.rs) cover mixed
access widths, rewind/exhaustion, zero count, address wrapping, priority gating,
budgets, RAM aliases, IRQ acknowledgement and failure preservation.
[Original routine tests](../../../tools/rust/bof3-audio/tests/cd_sector_runtime.rs)
execute 12 combinations of block size, service budget and whole/split transfers.
They also copy the first 16 raw VOICE sectors' 2,340-byte data fields exactly:
37,440 bytes, read through `DiscImage::read_sector` from `BOF3_AUDIO_TRACK`.
Each raw header's BCD position must match its disc LBA. Correction: the initial
test incorrectly grouped a 2,336-byte extracted STR into 2,352-byte chunks.
That run established arbitrary block copying only; the corrected raw-disc test
now supplies sector-aligned evidence. These tests establish transfer execution with
explicit input, not CD command execution, sector scheduling, XA decoding or
full BIOS startup. Independent drive timing and audio fidelity remain open.

## Rust CD drive commands and sector routing

[The functional drive](../../../tools/rust/bof3-audio/src/machine/cd_drive.rs)
requires an explicit already-authenticated, closed, spinning Mode-2 disc context,
including position, mode and filter. It implements Getstat, Setloc, Setmode,
Setfilter, Getparam, GetlocL, SeekL/SeekP, ReadN/ReadS and Pause transactions.
Known invalid parameters yield INT5; unsupported commands and mechanical overlap
fail without changing drive state. This is not an Init, BIOS or authentication
fallback. The host owner still supplies response/IRQ boundaries, and the caller
explicitly completes mechanical transitions; no clock duration is assigned.

Sector arrival requires a completed read seek and the next expected LBA.
Raw sync, Mode-2 header, duplicated subheaders and EDC are checked before state
advances. Form-2 sectors may omit EDC under the existing codec contract; Form-1
checks cover EDC, not ECC repair. Routing returns selected XA, filtered audio,
or a data block with its INT1 response. The latest header includes filtered
sectors, preserving the distinction between GetlocL position and audible data.
Neither data queue overruns nor deferred delivery/retry timing are implemented.
Automatic XA channel selection without a filter and CDDA/report/autopause/ignore
mode bits reject explicitly. Read-error retry behaviour remains unsupported.

[Five default checks](../../../tools/rust/bof3-audio/tests/cd_drive.rs) cover
command errors, seek/read/pause/resume transitions, routing, header progression,
malformed sectors and the existing host acknowledgement contract.
[Original US SDK checks](../../../tools/rust/bof3-audio/tests/cd_drive_runtime.rs)
submit commands through `0x80176734` (full EXE `0xE0734`) and consume drive-produced
responses through `0x80175C60` (`0xDFC60`). The explicit prior completion and
GPUSTAT input remain as documented for the host-register fixture. Mode `0xC8`,
SeekP, filter and ReadS follow bounded instruction evidence from game handlers
`0x801639D0` (`0xCD9D0`) and `0x80163AF0` (`0xCDAF0`), inspected with Rizin 1.0.0
against the hash-verified US payload. The subsequent scheduler fixture below
executes those handlers and callbacks together; timing remains separate.

The first 16 raw VOICE sectors include one selected XA sector, four rejected
audio channels and eleven ordinary data sectors. Original INT1 handling and
`CdGetSector` copy all eleven 2,048-byte data blocks exactly; GetlocL returns each
latest raw header through the original command/response path. A separate
256-sector routing check selects 16 XA sectors, filters 64 audio sectors and
retains 176 data sectors. Selected XA input and 75,264 resampled stereo frames
match direct decoding with the same explicit arithmetic/history model. This
proves routing and history preservation, not independent audio fidelity.

The initial 162-instruction Rizin request exceeded its read block and produced
invalid trailing records; only subsequent bounded listings are evidence.
Research is retained in [CD references](../../reference/cd-audio.md#drive-commands-and-routing);
checks and listings use `out/audio-migration/cd-drive-*`. Full drive queues,
seek/sector/IRQ timing, SPU scheduling, BIOS startup and reference audio remain open.

## Original XA scheduler with drive responses

[The integrated fixture](../../../tools/rust/bof3-audio/tests/xa_loop_runtime.rs)
executes the US cue initializer `exe/slus_004_22@0x801636A0` (full EXE `0xCD6A0`),
tick dispatcher `0x801638B0` (`0xCD8B0`), their state handlers, SDK command path
and game completion callback against the functional drive. It does not patch
game state or completion flags. Explicit fixture inputs are the ready disc/host,
a prior completion through the SDK, GPUSTAT zero for VSync(-1), disjoint guest
call stacks and service boundaries. Mechanical transitions complete at supplied
boundaries; commands still come from original instructions.

The SDK outer handler at `0x80177264` (`0xE1264`) is required: it calls the
low-level response handler at `0x80175C60`, examines returned dispatch bits, and
invokes the ready/sync callbacks at `0x80185784` / `0x80185780`. Initial probing
called only the low-level handler and stalled in game state 2 with the completion
flag unset. Executing the outer handler resolves that failure without injecting
a callback result. Bounded Rizin 1.0.0 listings and the original executed bytes
establish this distinction; no symbol table or recovered C was changed.

For cue `0x2000`, original setup emits mode `0xC8`, Setloc, SeekP, filter `(1,0)`,
Setloc and ReadS, reaching state 5 after 17,215 fixture transitions. Raw VOICE
delivery then drives GetlocL callbacks, threshold detection, fading, Pause and
idle. Both tested service schedules traverse states `5 → 6 → 7 → 0`, observe a
reported LBA at/above the game's stop threshold, finish with a paused drive and
zero drive-volume matrix, and retain the selected file/channel.

| Explicit test schedule | Raw sectors | Selected XA | Data sectors | Resampled frames |
| --- | ---: | ---: | ---: | ---: |
| One sector per scheduler call | 992 | 62 | 705 | 291,648 |
| Four sectors per scheduler call | 1,008 | 63 | 719 | 296,352 |

These schedules are deliberately different input orderings, not measured game
clocks. The differing output lengths demonstrate why functional completion does
not establish the audible endpoint. Resampled frame counts describe decoded
input with explicit zero history and the emulator reference model. The queue
fixture below now also consumes these frames through the SPU at explicitly
supplied boundaries; neither test establishes physical timing or reference PCM.

The original XA path acknowledges ordinary INT1 data without requesting it.
`Interconnect::deliver_cd_data` therefore selects a block and publishes INT1 in
one checked transaction. Prior command/response/IRQ work must be drained. It
reports displaced unread bytes when the old block is unrequested; an active
BFRD read remains protected. Clearing BFRD explicitly releases/rewinds that old
selection. Missing configuration, invalid sizes and pending work fail before
replacing data or publishing a response. Four
[delivery tests](../../../tools/rust/bof3-audio/tests/cd_delivery.rs) cover these
transitions, atomic failures and IRQ wiring. Direct `present_cd_data` retains
its strict no-replacement contract. This serialized path does not implement
multiple pending sectors, buffer overruns or asynchronous retry timing.

The fixture invokes the outer handler as a separate guest function when its
chosen interrupt boundary permits, preserving the suspended CPU object. It does
not execute the BIOS exception vector, TCB save/restore or interrupt priorities.
Continuous CPU/device clocks, complete BIOS startup, XA/SPU queue timing and
independent reference audio remain required. Logs use `out/audio-migration/xa-loop-*`;
external buffer-selection evidence is retained in
[CD references](../../reference/cd-audio.md#serialized-data-selection).

## Rust CD/XA sample processing

[The CD audio component](../../../tools/rust/bof3-audio/src/machine/cd_audio.rs)
connects selected XA sectors to stereo 44.1 kHz frames through the existing
explicit decoder arithmetic and retained filter history. It supports mono/stereo
and 4/8-bit sectors at both encoded rates, with no implicit channel switch, EOF
reset, end padding or tail flush. Extraction remains at encoded rate.

`Published37800` implements the published seven-phase filter alignment and rejects
18.9 kHz, for which that source gives no complete algorithm. `EmulatorReference`
uses the inspected emulator's 37.8 kHz alignment and distinct half-rate filter.
Both require explicit selection; neither establishes hardware fidelity. See
[coefficient provenance and differences](../../reference/cd-audio.md). Reset
contexts have zero filter history, position zero and phase six. Phase/history
survive every chunk and sector boundary. In the half-rate reference, 120 input
frames initially emit 279 frames with phase three pending; subsequent complete
sectors retain that phase rather than silently pad to an idealized duration.
Malformed stereo input or failed sector decoding leaves processing state intact.
Emphasis remains unsupported.

Drive volume is separate from SPU input gain. Its four staged coefficients cover
both direct and crossed routes, latch together on ADPCTL apply, floor each product
separately and saturate the sum. The caller supplies initial coefficients; no BIOS
default is inferred. Gains exceeding double volume per output reject before
latching because the hardware's larger-gain saturation is unresolved. Reserved
control bits also reject. The component can mute supplied XA output independently
of CDDA, but does not decide mute's effect on decoding, interpolation or queued
frames. The CD host register owner above now decodes its bank/offset writes;
drive-command execution, sector scheduling and FIFO/audio lifecycle remain open.

[CD processing checks](../../../tools/rust/bof3-audio/tests/cd_audio.rs) compare
three signed resampling vectors against a separate integer calculation, preserve
sample-by-sample versus whole-buffer equivalence, exercise all eight coding
combinations, and cover gain staging, mono, swap, mute, saturation and failures.
A bounded original VOICE prefix provides 16 selected sectors and 75,264 output
frames at coding `0x00`; this is not complete stream/cue playback evidence.
[SPU integration](../../../tools/rust/bof3-audio/tests/spu_output.rs) confirms
that drive-routed XA reaches capture before SPU CD gain and final main gain.
Actual game seek/filter/mute state, timed delivery, buffer underflow/overflow,
de-emphasis and independent hardware audio remain required.

## Rust XA audio queue and SPU consumption

[The XA queue](../../../tools/rust/bof3-audio/src/machine/cd_queue.rs) connects
sector decoding/resampling to frame consumption. Its required
`Model::EmulatorReference` follows the inspected emulator's admission rule:
more than ten buffered frames drops the arriving sector without committing
predictor or interpolation history. Ten or fewer permits admission. Drops,
muted sectors, actual frame consumption and empty output are counted separately.
Unlike the comparison implementation's early return, Rust still validates a
dropped sector on a private decoder candidate; malformed input fails without
changing the queue, statistics or either history. No hardware capacity or clock
latency is inferred from this rule.

XA mute is sampled at sector admission. An admitted muted sector advances ADPCM
predictor history, but does not resample or append output. Previously queued
frames remain available. `Interconnect::step_spu_cd_output` applies the host's
current drive-volume matrix as each frame is consumed, then uses existing SPU
CD gain, routing, reverb and final gain. Capture therefore sees post-drive,
pre-SPU-gain samples. The low-level `Volume::apply` convenience mute flag does
not govern queued-frame lifetime; the queue handles admission mute separately.
Empty queue output is zero under the selected reference model and does not
advance decoder history. Seek/read/Pause reset and CD command mute are now
connected through the command lifecycle below. EOF selection release and coding
transitions are implemented under the explicit emulator model below; physical
timing remains unresolved.

Configuration is explicit and cannot replace a live queue. Selected sectors enter
through `enqueue_cd_audio`; there is no automatic stream switch or sample-clock
inference. Queue consumption commits only after successful SPU output. A failure
retains its queued frame, but the existing SPU engine can have advanced internal
state before a later failure: this is not a transaction over the whole SPU.

[Five default queue checks](../../../tools/rust/bof3-audio/tests/cd_queue.rs)
exercise the eleven/ten-frame boundary, preserved histories after drops and
errors, muted predictor/interpolator distinction, buffered volume/mute changes,
capture staging, empty output and failed SPU consumption. A raw-disc check feeds
256 VOICE sectors with explicit 150-sector/s and 44,100-frame/s boundaries:
16 selected XA sectors produce 75,264 SPU output frames, with no queue drops or
underruns. Output matches direct same-model decoding through the declared gains;
this is a routing comparison, not an independent fidelity oracle.

The original scheduler fixture above now uses this queue and SPU path too. Its
one-/four-sector-per-tick schedules consume 291,648 / 296,352 frames respectively
without drops or underruns, produce nonzero output, and still finish in idle
with the drive-volume matrix zero. Their dry SPU register context and 294 output
frames per delivered sector are supplied test inputs, not recovered BIOS state
or CPU-cycle timing. Logs use `out/audio-migration/cd-queue-*`; queue evidence and
its limitations are retained in
[CD references](../../reference/cd-audio.md#xa-queue-admission-and-output).
Independent reference PCM, original runtime bootstrap, continuous clock
integration and lifecycle transitions remain required before PSX rendering
acceptance or C retirement.

## Rust CD command lifecycle and cue restart

`Interconnect::apply_cd_drive_command` executes a captured command on a drive
candidate before committing its device effects. Under the explicit emulator
reference, entering a seek/read transition clears queued audio, releases its
stream binding and clears selected data/BFRD. Setloc alone only records a target.
A read command already continuing at the next sector does not reset either
buffer, including when Setloc specifies that same sector. Pause clears audio
while preserving the data selection. Mute/Demute maintain a separate command
mute flag; admission is muted if either it or the host ADPCTL bit is set.
Changing mute leaves already queued frames intact and does not cancel a seek.

The returned command effect reports whether audio reset occurred and how many
audio frames/data bytes were discarded. The queue retains cumulative statistics,
including reset count and discarded frames. After reset, the next successfully
validated selected sector binds file/channel/coding with zero predictor and
interpolation history, retaining configured arithmetic and resampling models.
A malformed candidate cannot establish that binding. A file/channel change
requires selection release; coding changes use the emulator-reference transition
path described below, preserving device history.

Known parameter errors produce INT5 without reset; unsupported commands and
seek/reset during active DMA3 fail before changing drive, audio or data state.
Response/IRQ delivery and mechanical completion remain explicit caller-owned
boundaries. The standalone `Drive::command` API remains available for isolated
drive checks; integrated execution must use the interconnect method to apply
buffer/mute effects. No command silently installs a BIOS or drive boot state.

[Five lifecycle checks](../../../tools/rust/bof3-audio/tests/cd_lifecycle.rs)
cover fresh stream/format binding, Setloc versus seek, invalid/DMA-active failures,
independent mute sources, Pause data preservation and continuing-read behaviour.
The [original scheduler fixture](../../../tools/rust/bof3-audio/tests/xa_loop_runtime.rs)
now applies these effects to all commands. A restart from cue `0x2000` to `0x2001`
begins with 4,704 buffered frames plus an acknowledged unrequested data sector.
Original initialization, SeekP and located ReadS clear the old buffers and cause
two command-application resets. Channel 1's first accepted block then matches
fresh same-model decoding through every SPU output frame. No game state or
completion flag is patched. This establishes the functional restart path with
declared guest/drive/SPU context, not physical reset latency or reference audio.

The two existing full-cue service schedules still reach idle with their prior
frame counts and zero final drive matrix. Logs use
`out/audio-migration/cd-lifecycle-*`; external reset/mute evidence is retained in
[CD references](../../reference/cd-audio.md#decoder-reset-and-command-mute).
General command overlap, DMA cancellation, emphasis filtering, original
runtime bootstrap, clocks and independent PCM remain acceptance obligations.

## Rust XA selection and EOF

The functional drive now retains XA file/channel selection independently from
Setfilter's configured pair. With filtering disabled, the first eligible audio
sector acquires selection; channel 255 is skipped unless explicitly selected by
the enabled filter. Other file/channel pairs cannot interrupt it. A matching EOF
is delivered as audio and releases selection before decoder admission, while
an unrelated EOF is filtered out. EOR does not release selection. Setfilter
releases selection even with filtering disabled; Setmode alone preserves it.
Seek/read startup and Pause clear selection with the existing decoder reset.
These are emulator-reference behaviours, not established hardware timing.

`Queue::release_selection` and validated EOF permit the next selected file/channel
to bind while preserving ADPCM and interpolation histories and queued output.
The interconnect applies this release for successful Setfilter and reports it
separately from a full audio reset. A backlog-dropped EOF still releases
selection; a dropped first sector acquires the new binding without committing
decoded history. Muted EOF updates predictors only, as other muted sectors do.
Malformed sectors preserve all queue state. Coding changes use the runtime
transition path below; full reset instead establishes a fresh coding context.

[Selection checks](../../../tools/rust/bof3-audio/tests/cd_selection.rs) cover
automatic and explicit selection, unrelated EOF, EOR, Setfilter, all three EOF
admission outcomes, retained history/output, invalid data and emphasis rejection.
The media check scans all 3,536 raw VOICE sectors: 71 selected channel-0 sectors,
one EOF and 333,984 frames equal continuous decoding under the same model. That
extent has no cross-channel handoff after the selected EOF; synthetic cases
exercise handoffs. It drains output per selected sector and makes no clock or
independent reference-audio claim. Original SDK, scheduler and cue-restart
checks remain passing. Evidence uses `out/audio-migration/cd-selection-*`;
[selection references](../../reference/cd-audio.md#xa-selection-and-eof) retain
the external comparison boundary. Emphasis filtering, runtime bootstrap, device
clocks and independent audio validation remain open.

## Rust XA coding transitions

The emulator-reference queue accepts sector-to-sector changes among coding bytes
`00/01/04/05/10/11/14/15` hex: mono/stereo, 37.8/18.9 kHz and 4/8-bit ADPCM.
Coding changes do not release file/channel selection. Rebinding those identities
still requires EOF, Setfilter or full reset. The standalone `XaAudio` export
decoder keeps its fixed-stream contract; dynamic changes belong to the runtime
queue's selected-sector path.

Transitions preserve left/right ADPCM predictors, interpolation rings, shared
cursor and phase. Mono processing updates only left history and duplicates the
interpolated left result to both outputs. Right history survives for later
stereo input. Muted sectors change predictors without resampling; dropped
sectors change neither predictor nor interpolation history. Previously queued
output retains its original samples. Reserved coding fields, emphasis and
malformed units fail without committing queue state. Coding transitions with
the separate `Published37800` resampler fail explicitly because its dynamic
format behaviour has not been established.

[Three transition checks](../../../tools/rust/bof3-audio/tests/xa/coding.rs)
include 192 vectors: all 64 format pairs, each queued/muted/dropped, followed by
a return to the initial format. They check output counts and full PCM hashes,
plus malformed-input rollback, model boundaries and identity release. The
retained [Rust reference model](../../../tools/rust/bof3-audio/tests/reference/xa.rs)
implements separate decoder/resampling state and arithmetic, reusing numeric
coefficient tables whose fixed-format vectors are checked separately. The Rust
transition test compares every generated count/hash with the unchanged
[fixture](../../../tools/rust/bof3-audio/tests/reference/xa_transitions.json).
Its initial full-rate tap direction was corrected against the primary source;
the failed probe remains in `out/audio-migration/xa-coding-focused.log`.

Existing fixed-format vectors, raw VOICE and original game scheduler/restart
checks pass. Evidence uses `out/audio-migration/xa-coding-*` and
[coding references](../../reference/cd-audio.md#xa-coding-transitions).
These synthetic transitions establish the selected model's functional state
behaviour, not measured hardware filters, clocks or independent game PCM.

## PS1 SPU emulation

### Evidence boundary

Use each source only for the layer it owns:

| Evidence | Owns | Does not establish |
| --- | --- | --- |
| Original `SLUS_004.22` bytes and linked routines | BOF3/PsyQ SEP parsing, VAB interpretation, note-to-pitch conversion, voice allocation, and scheduler behavior | Undocumented electrical/timing behavior inside the SPU |
| Sony PsyQ 4.7 headers in `toolchains/psyq/4.7/include/` | Public structure layouts, field types, and API contracts | Linked implementation details or BOF3 call policy |
| [psx-spx SPU specification](https://psx-spx.consoledev.net/soundprocessingunitspu/) | Hardware registers, ADPCM, pitch counter, interpolation, ADSR, volume, transfer, noise, modulation, and reverb behavior | BOF3's sequence semantics or game scheduler |
| [DuckStation `src/core/spu.cpp`](https://github.com/stenzek/duckstation/blob/master/src/core/spu.cpp) | Tested implementation cross-check for the hardware specification | BOF3/PsyQ-specific parsing and allocation |

External implementations are corroboration, not code to import blindly. Preserve
their license boundaries and verify constants or algorithms against psx-spx and,
where possible, original bytes or hardware traces.

### Renderer architecture

`render_bgm()` is the current rendering seam. It is a direct SEP/VAB renderer,
not execution of BOF3's linked sound runtime; it must be described as an
approximate offline renderer rather than a game-faithful engine.

The exact path is split below that seam:

| Module | Contract | Current state |
| --- | --- | --- |
| `psf.c` | PSF1/MiniPSF load, overlay, CRC, PC/SP, and package | Implemented and tested |
| `psx_machine.c` | Bounded R3000 execution and PSF hardware boundary | Partial vertical slice |
| `spu_device.c` | SPU registers, 24 voices, live ADPCM, pitch, Gaussian interpolation, ADSR, sound RAM, FIFO/DMA | Implemented (noise, pitch modulation, volume sweeps, and reverb included); register-timing exactness incomplete |
| `render.c` | Direct SEP/VAB scheduling into `spu_device.c` | Implemented `fast` engine with 24-voice stealing; renders through the live SPU device |

The machine intentionally faults on unsupported instructions, BIOS calls, and
hardware addresses. The complete game PSF currently reaches its first CD-ROM
register access at `PC=0x80176E80` after 33,470 interpreted instructions. PSF1
does not provide CD-ROM hardware, so the exact player requires a bootstrap that
installs audio assets before entering the game sound runtime; emulating the full
game boot is not the target architecture.

### Hardware coverage

The SPU advances at 44.1 kHz, or once per `0x300` CPU clocks. Voice and main
register changes are therefore sample-clocked on hardware; the current device
applies most writes immediately. This timing difference matters to exact game
execution but not to an offline `fast` event scheduled on output frames.

| Hardware contract | Register-driven `spu_device.c` | Direct `fast` renderer |
| --- | --- | --- |
| 24 equivalent voices | Implemented | Implemented; oldest voice is stolen when full |
| 512 KiB SPU RAM and 8-byte address units | Implemented with wrapping | Uses the same register-driven SPU RAM |
| 16-byte ADPCM blocks, 28 samples | Live decode | Uses the same live decoder |
| Loop start/end/repeat flags and ENDX | Implemented | Uses the same flag and ENDX path |
| Predictor and interpolation history across loops | Preserved by live decode | Preserved by live decode |
| Zero interpolation history on key-on | Implemented | Implemented by zero-padding before sample index zero |
| `VxPitch`: `0x1000` = 44.1 kHz, clamp above `0x4000` | Implemented | Equivalent ratio after PsyQ note conversion |
| 4-point, 512-entry Gaussian interpolation | Implemented | Uses the same implementation |
| ADSR attack/decay/sustain/release | Implemented | Uses the same implementation |
| Signed fixed voice/main volume | Implemented | Writes fixed SPU voice/main volume registers |
| Volume sweep mode | Implemented; sweep levels step per the sweep envelope | Implemented via the shared live device path |
| KON, KOFF, ENDX | Implemented | Modeled as note events, not registers |
| Pitch modulation (PMON) | Implemented; voice N modulates from voice N−1 | Implemented via the shared live device path |
| Noise source (NON) | Implemented; shared noise source replaces ADPCM | Implemented via the shared live device path |
| Per-voice and master reverb | Implemented; half-rate SPU-RAM feedback pipeline | Implemented; enabled when a tone requests reverb |
| SPUCNT enable/mute and delayed status | Registers stored; behavior/timing incomplete | Not applicable |
| Manual/DMA transfer FIFO timing and IRQ | Data transfer implemented; FIFO timing and IRQ incomplete | Not applicable |
| CD/XA and external-input mixing/capture | Not implemented | Separate XA decoder; not mixed through SPU |

DuckStation explicitly zeroes the previous-block interpolation samples at key-on
to avoid clicks in *Breath of Fire III*. Keep this as a BOF3 regression invariant.
The fast path now decodes loop starts again with predictor and Gaussian history
retained from the loop end instead of replaying a cached PCM loop.

### ADPCM decode

Integer predictor formula used by the host decoder and independently present
in [DuckStation's SPU decoder](https://raw.githubusercontent.com/stenzek/duckstation/master/src/core/spu.cpp):

```c
sample = (int16_t)(nibble << 12) >> shift;   // sign-extend + shift
sample += (prev1 * filter_pos) >> 6;
sample += (prev2 * filter_neg) >> 6;
clamp(sample, -32768, 32767);
```

Filter coefficients:

| Filter | pos | neg |
| ---: | ---: | ---: |
| 0 | 0 | 0 |
| 1 | 60 | 0 |
| 2 | 115 | −52 |
| 3 | 98 | −55 |
| 4 | 122 | −60 |

Reserved shifts 13–15 decode as shift 9; reserved filters 5–15 use zero
coefficients. On hardware, ADPCM predictor and Gaussian sample history continue
across loop jumps rather than resetting at the loop-start block. Both the
register-driven device and the `fast` renderer preserve this history because
the renderer routes through the same live decoder; no predecoded loop cache is
used.

### Rust sample decoding and loop export limits

[The Rust decoder](../../../tools/rust/bof3-audio/src/codec/adpcm/mod.rs) decodes complete
16-byte SPU ADPCM blocks to 28 signed PCM frames, carrying clipped predictor
history between blocks. Standalone sample inspection starts with zero history
and includes the first end block's PCM. Bytes after that block remain opaque
preservation data. Empty samples and bounded data without an end marker remain
distinct. Unsupported predictors, shifts and flag bits fail explicitly; the
current Rust API does not implement the reserved-value behavior described above.
Samples do not intrinsically specify a playback rate, envelope or pitch context.

Loop ranges use the latest local start marker and an exclusive PCM endpoint;
RIFF `smpl` requires converting that endpoint to its inclusive counterpart.
A repeat marker without a local start requires game repeat-address context and
is rejected by standalone sample decoding. Loop history continues across a
jump, consistent with the [SPU loop flags](https://psx-spx.consoledev.net/soundprocessingunitspu/#flag-bits-in-adpcm-header).
The decoder reports whether re-decoding the loop with carried history reproduces
the first-pass PCM. This check does not model Gaussian interpolation or ADSR.

[Synthetic checks](../../../tools/rust/bof3-audio/tests/codec/adpcm/decoder.rs) cover all five
predictors and shifts 0–12, signed nibble order, clipping/history, malformed
headers, loop markers and opaque tails. An installed FFmpeg 6.1.1 independently
decodes the unfiltered synthetic VAG fixture identically; it is a development
check, not a production dependency or a general SPU fidelity oracle.

[Corpus checks](../../../tools/rust/bof3-audio/tests/codec/adpcm/corpus.rs) agree with
independent Python TOC/sample-table traversal and integer decoding for all
8,385 sample slots (1,180 unique encoded allocations). There are 55 empty slots,
7,392 end/mute samples and 938 looped samples, totaling 138,296,704 decoded
frames. The 130,816 bytes after end markers remain undecoded. For 790 loops,
the next traversal differs from repeating the first-pass PCM; future WAV/SF2
export must retain and report this approximation. Local reference evidence is
`out/audio-migration/adpcm-reference.json`; PCM/identity/loop metadata aggregate
SHA-256 is `cf7f115ccb8e468b1e321657f4715ab4ffd5573cc0bbcd37a911b1f644522eae`.
This validates decoding and sample structure, not full PSX rendering.

### Rust edited-sample PSX ADPCM encoding

[The encoder](../../../tools/rust/bof3-audio/src/codec/adpcm/encoder.rs) accepts mono
PCM16 and produces complete 16-byte blocks of 28 frames. It searches all five
supported predictors and shifts 0–12 per block, using greedy nearest reconstructed
samples and selecting the least squared block error. This is deterministic, not
a global optimization over nibble sequences or blocks. Predictor arithmetic is
shared with the existing SPU decoder; every chosen block and the final sample
are decoded again before returning output.

Encoding starts from zero history, the same standalone reference used by sample
extraction. It does not choose playback rate, alter pitch or establish game voice
initialization. Non-looping samples end with an end/mute flag. Empty input remains
empty. Input lengths must be multiples of 28 unless zero-target padding is
explicitly requested; padded frames add duration and undergo quantization too.
The report separates original-frame error from padding error, with peak absolute
error, RMS, signed mean and signal-to-noise ratio. The ratio is absent for silent
input or exact reconstruction rather than serializing infinity.

Forward loops must be nonempty, align both endpoints to 28 frames and end at the
sample end. The encoder rejects moved endpoints and post-loop tails, even when
padding is requested. It forces predictor zero at the loop-entry block, making
that block independent of incoming history; subsequent blocks then receive the
same history on every traversal. This can increase loop-entry quantization loss,
which remains in the report. It establishes repeated ADPCM PCM, not Gaussian
interpolation or audible seam quality. A one-block loop uses combined flag 7.

An optional byte capacity must align to 16 and fit every encoded block. Capacity
failure returns an error without truncation. The encoder does not fabricate
unused allocation bytes: archive reconstruction must retain unrelated padding,
prefixes and opaque tails according to its edit policy. Unchanged assets must
reuse their original encoding, not pass through this lossy encoder.

[Eight checks](../../../tools/rust/bof3-audio/tests/codec/adpcm/encoder.rs) include six
default tests for exact endpoints/silence, deterministic filtered encoding,
independently recomputed error metrics, alignment/capacity rejection, explicit
padding, full-scale discontinuities and eight further loop traversals. Two opt-in
checks cover an independent consumer and bounded edited corpus excerpts:

- FFmpeg 6.1.1 accepts a generated VAG wrapper containing 2,240 encoded frames.
  Its [PSX decoder](https://github.com/FFmpeg/FFmpeg/blob/n6.1.1/libavcodec/adpcm.c#L2033-L2075)
  combines predictor terms with truncation toward zero and retains unclipped
  predictor history. The SPU model separately floors products and clips feedback.
  This fixture matches the independent consumer's arithmetic exactly, while its
  PCM differs from the SPU model by peak 110 / RMS 66.480918. FFmpeg also treats
  flag 7 differently, so this non-looping conformance check is not loop or hardware
  fidelity acceptance. FFmpeg remains development-only.
- Each of 1,179 unique nonempty corpus allocations supplies at most its first
  eight blocks. Every seventeenth decoded frame is increased by 257 with
  saturation, then encoded within the original capacity. All 264,096 edited
  frames pass structural and loss-report checks: peak error 813, weighted RMS
  38.462789 against the SPU decoding model. This is excerpt coverage, not full
  sample or rebuilt-archive acceptance; loss is measured on edited input.

The existing all-sample decoder reference also remains unchanged: 8,385 slots,
138,296,704 frames and the aggregate hash above. XA cue/manifest reconstruction,
parsed edit detection, unchanged/edited archive reconstruction and verified publication
remain unfinished. This codec alone does not enable the `pack` command.

### Rust WAV interchange

[The WAV reader/writer](../../../tools/rust/bof3-audio/src/interchange/wave.rs) supports
little-endian mono/stereo PCM16 with explicit sample rates. It validates RIFF
and chunk lengths, byte rates, frame alignment, duplicate structural chunks,
sampler sizes and frame bounds. RF64, RIFX, segmented `wavl`, non-PCM16 encodings
and unsupported format extensions fail explicitly. Output size is checked
before allocating the RIFF buffer. Empty PCM data remains representable.

Sampler loops count frames, including for stereo. Internal exclusive endpoints
convert to/from RIFF's inclusive `smpl` endpoint; a one-frame loop therefore has
equal on-disk start/end indices. This follows Microsoft's
[RIFF chunk rules](https://learn.microsoft.com/en-us/windows/win32/xaudio2/resource-interchange-file-format--riff-),
[PCM format definition](https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatex)
and [1994 sampler chunk specification](https://billposer.org/Linguistics/Computation/riffnew.pdf).

Parsing and reserialization retain chunk order, unknown payloads, odd-byte
padding, optional PCM format suffixes, sampler tuning/SMPTE/manufacturer fields
and sampler-specific data. If PCM frame count changes, `fact` is updated.
Sampler period/tuning is independent metadata and is not silently reset when
the format rate changes. Multiple, alternating, backward, fractional and finite
loops remain inspectable; requesting normal SPU loop semantics rejects them.
Opaque metadata preservation does not establish a game mapping for those fields.

[WAV checks](../../../tools/rust/bof3-audio/tests/interchange/wave.rs) cover exact unchanged
WAV bytes, edited PCM/rates/counts, inclusive endpoints, malformed inputs and
unsupported loops. An independent FFmpeg consumer reads the synthetic stereo
PCM fixture with sampler/opaque chunks. The sample corpus test also serializes
and reparses all 1,180 unique sample allocations, preserving PCM and loop
metadata for the 8,385 source slots. Its 44.1 kHz rate is an explicit export
reference, not an intrinsic VAB rate. Predictor-history loop differences must
be retained separately in XML; `smpl` cannot express them. Bank extraction below
now records those differences; edited archive reconstruction remains unimplemented.

### Rust bank extraction and preservation XML

[Bank extraction](../../../tools/rust/bof3-audio/src/bank/extraction.rs) is available as:

```sh
bof3-audio extract --mode audio --kind bank --archive FILE.EMI \
  --executable SLUS_004.22 --output NEW_DIR [--id ID] [--reference-rate HZ] [--json]
```

Use repeated `--archive` arguments or `--disc-root` for direct media input.
Omitting `--id` selects all banks. Numeric selections must be unambiguous;
qualified identities use the catalog's source and header-entry identity.
The exact US executable profile supplies verified loader associations. A bank
needs one resolved body large enough for every declared sample; ambiguous or
missing bodies fail before publication. SFX/vocal classification and music
extraction remain unsupported, with explicit diagnostics; XA uses the separate
stream/cue extraction surface above.

The root `audio.xml` (`bof3.audio-extraction/v1`) references relative bank
manifests. Each folder's `bank.xml` (`bof3.audio-bank/v1`) records qualified
source/header/body and game-bank identities, executable hash and pitch routine
addresses, all parsed program/tone fields, diagnostics and sample WAV paths.
Loader metadata retains auxiliary/song links, known game-song IDs and all three
possible runtime layouts with their addresses/capacities; the active layout
remains unresolved.
The `preservation` element contains the entire original EMI as hexadecimal text
with its byte length and SHA-256, including unrelated entries, padding, ordering
and trailing bytes. Each bank folder therefore retains its own source snapshot.
No executable bytes are embedded. Source hashes are checked against inventory
before extraction; snapshots do not authorize ignoring subsequent content edits.

WAVs contain mono PCM16 and `smpl` metadata. The default reference rate is
44,100 Hz, corresponding to SPU pitch register 4096; `--reference-rate` accepts
integer rates exactly representable by a nonzero 14-bit register. No resampling
occurs. Unity note 60 is an export convention. Original tone center/shift, key
ranges, bends and envelopes remain separate XML fields; actual gameplay voice
contexts are unresolved. Sample records retain encoded/WAV hashes, frame counts,
decoded and trailing byte counts, termination and exclusive loop endpoints.
`pcm_repeat_is_stable=false` explicitly marks the fixed-loop approximation.

Output must be absent. Extraction uses a sibling staging directory, verifies
written WAVs and manifest bytes, then renames it into place. Ordinary failures
remove owned staging content; existing output is rejected. This is a single
writer publication protocol, not a race-free or crash-durable multi-writer
transaction. XML uses UTF-8, escaped attributes and explicit character references
for whitespace; XML-invalid path characters are rejected.
These rules follow the W3C XML 1.0
[character range](https://www.w3.org/TR/xml/#charsets) and
[attribute normalization](https://www.w3.org/TR/xml/#AVNormalize) requirements.

[Extraction tests](../../../tools/rust/bof3-audio/tests/bank/extraction.rs) and a
[development-only Rust consumer](../../../tools/rust/bof3-audio/tests/bank/consumer.rs)
verify XML/RIFF independently, including all 1,020 banks and 8,385 samples.
Every embedded archive equals its input; WAV PCM and loop metadata reproduce
the independent corpus reference (138,296,704 frames, 790 approximate loops).
Synthetic checks cover layered tones, escaped paths, unsupported ADPCM,
ambiguous selection and failed publication. This proves extraction preservation,
not archive packing or PSX rendering fidelity; bank packing is checked separately below.

### Rust extraction manifest reader

[The manifest reader](../../../tools/rust/bof3-audio/src/document/manifest.rs) uses
`roxmltree = 0.21.1`, explicitly authorized by the user's XML-crate approval on
2026-09-24. Its published source is pure Rust with no build script; the only
dependency is the already-locked `memchr 2.8.3`. The crate declares Rust 1.60 and
MIT OR Apache-2.0 licensing. The archive SHA-256 is
`f1964b10c76125c36f8afe190065a4bf9a87bf324842c05701330bba9f1cacbb`, and its
recorded source commit is `67644e16f43c34cadc9e163163dd1aaf7ebe205e`.
The [license bundle](../../../tools/rust/bof3-audio/THIRD_PARTY_LICENSES.txt)
retains both published licenses and the ISC notice for the upstream ego-tree
implementation credited in `src/lib.rs`. The crate and existing transitive
version are pinned in [Cargo.lock](../../../tools/rust/bof3-audio/Cargo.lock).

Parsing is restricted to UTF-8 XML 1.0, with default limits of 256 MiB, 200,000
nodes and 64 element levels. File reads are bounded too. DTDs, external entity
resolution, namespaces and processing instructions are unsupported. Comments
remain annotations; CDATA/text and normalized attribute values are retained as
parsed content. This follows the relevant
[XML character and normalization rules](https://www.w3.org/TR/xml/#AVNormalize),
not raw string matching of serialized XML.

The pinned parser has a permissive behavior that the reader must not inherit:
[its character-reference conversion](https://github.com/RazrFalcon/roxmltree/blob/67644e16f43c34cadc9e163163dd1aaf7ebe205e/src/tokenizer.rs#L990)
replaces invalid Unicode scalar references with U+FFFD. An initial malformed-input
test exposed this for `&#xD800;`. A lexical validation pass now rejects invalid
numeric references and unsupported declarations before parsing, while retaining
literal reference-like text inside comments/CDATA and escaped ampersands. The
dependency itself is not patched. Correct XML parsing alone is not asset validation.

Reader helpers enforce unique required children, reject unrecognized fields when
given a schema shape, compare typed scalar values to source metadata, validate
hexadecimal preservation length/scope/SHA-256, and resolve regular input files
within the extraction root. Numeric spelling changes such as leading zeros do
not count as semantic changes; altered values produce explicit diagnostics.
Canonical path checks reject absolute paths and symlink escapes. Parent-relative
links are allowed when their resolved target stays within the extraction root,
as required by XA asset-to-source references.

[Six parser checks](../../../tools/rust/bof3-audio/tests/document/manifest.rs) cover
normalization, Unicode, malformed/ambiguous input, declarations, limits, source
hashes, scalar conflicts and path confinement. Two
[extraction integration checks](../../../tools/rust/bof3-audio/tests/document/extraction.rs)
read real writer output: synthetic multiplexed XA with escaped source names and
relative snapshot links, and a fresh original BGM000 export with 21 samples /
408,212 frames. Embedded EMI bytes, program/tone/sample scalar metadata, WAV
hashes and decoded PCM agree with the source. The bank check requires local media.
These checks do not cover the complete extraction corpus or accept archive packing.

The reader supports the bank and XA packing paths below. Its byte limit includes
the complete 94,411,776-byte S_XA00.STR encoded as hexadecimal XML. Music manifest
packing still requires schema-specific edit decisions and reconstruction.

### Rust bank packing

[`pack`](../../../tools/rust/bof3-audio/src/pack.rs) accepts one bank folder or
an extraction root:

```sh
bof3-audio pack --mode audio --input EXTRACTED_FOLDER \
  --executable SLUS_004.22 --output NEW_DIRECTORY --json
```

The supplied US executable validates runtime profile, slot layouts and song
associations. Original EMI files are not needed: the catalog is reconstructed
from XML preservation bytes. [Bank schema validation](../../../tools/rust/bof3-audio/src/bank/manifest.rs)
checks identities, header/body selection, programs, tones, diagnostics, loader
layouts, auxiliary/song links and export context against that catalog. Unknown,
missing, duplicate or conflicting fields fail explicitly. Numeric spelling and
collection order may vary without changing their meaning. XML metadata is
preservation evidence; edits to programs, tuning and allocation tables are not
supported. Older manifests missing required fields must be re-extracted.

[`sample_pack`](../../../tools/rust/bof3-audio/src/bank/packing.rs) validates each
sample's original hashes, frame/allocation counts, termination and loop evidence,
then reads its confined relative WAV path. Edit detection compares parsed PCM
and forward `smpl` loop ranges with a fresh decode of preserved ADPCM. Unchanged
samples reuse every encoded byte, including unstable predictor loops and opaque
tails, even when harmless RIFF formatting changes the WAV file hash.

Supported edits retain mono PCM16 and the recorded export rate, fit the original
allocation and align to 28-frame ADPCM blocks. Loop endpoints must be block
aligned, with the exclusive end at sample end. Removing `smpl` removes the loop;
non-looping samples may shorten or grow within capacity. The encoder reports
quantization loss; unconsumed allocation bytes remain intact, and consuming old
tail bytes is reported. Emptying a nonempty sample, implicit padding/resampling,
unsupported loops, pitch/timecode/vendor metadata and ancillary chunks are
rejected. Decoding each replacement verifies its PCM and loop result before use.

Multiple selected banks from one source combine into one EMI. Conflicting
preserved source bytes or body replacements fail. Identical bank content at
different physical identities remains independent. Unchanged archives require
whole-file equality. Changed archives use fixed entry allocations and preserve
unrelated entries, table padding, sector padding and trailing bytes; only changed
payloads and their cached TOC first words need change. Entry and whole-file
read-back checks precede publication through the existing private staging helper.
The output parent must exist and the destination must be absent; the existing
single-writer publication limits still apply.

Published files use `archives/<SHA-256-of-source-identity>.EMI`; historical source
paths never become output paths. `pack.json` (`bof3.audio-pack-report/v1`) maps
identities to files and separates byte equality, changed entries, sample encoding
loss and limitations. [Sample tests](../../../tools/rust/bof3-audio/tests/bank/packing.rs)
and [packing tests](../../../tools/rust/bof3-audio/tests/pack.rs) cover unchanged
and edited samples, loops, capacity/metadata failures, standalone/root use,
missing original inputs, combined bank edits, preservation conflicts and CLI
publication. The opt-in whole-corpus check (`BOF3_AUDIO_CORPUS` and
`BOF3_AUDIO_EXE`) passed on 2026-09-24: all 809 bank-containing EMI archives,
1,020 banks and 8,385 samples reproduced whole original files byte-for-byte.
The fresh BGM000 CLI extract/pack/verify smoke output is 266,240 bytes with SHA-256
`e1caf8633ce6be70524bd288b5af1c4b6042e4ca5cb3b02600affd68b8013a91`.
Evidence is retained under `out/audio-migration/bank-pack-*`.

This does not validate complete game playback, musical edits or disc rebuilding.
XA manifests use the separate path below.

### Rust XA manifest packing

[`xa_pack`](../../../tools/rust/bof3-audio/src/xa/packing.rs) is selected by
`pack --mode audio --input EXTRACTION_ROOT --output NEW_DIRECTORY [--executable
SLUS_004.22] [--json]` when `audio.xml` names `xa_stream` or `xa_cue`. The root must
retain its source snapshots; an isolated asset folder without that root is not a
self-contained input. The executable is required for cue roots and any recorded
runtime metadata. Stream roots exported without runtime metadata can pack without
an executable. Original STR paths need not exist.

Source manifests validate complete preserved bytes, hashes, extent status and
sector counts. Known truncated extents fail; unrecognized extents remain explicitly
unverified. [Asset validation](../../../tools/rust/bof3-audio/src/xa/manifest.rs)
reconstructs physical identities from preserved media, checks profile-backed cue
placement and BCD fields, and validates file/channel/coding, decoder arithmetic,
zero export history, original final history, sector-to-frame mapping and original
WAV/selected-sector hashes. Unknown fields and conflicting references fail.
Original WAV hashes are provenance checks; parsed input PCM determines edits, so
harmless RIFF formatting changes do not force re-encoding.

WAV edits retain the encoded rate/channel layout and exactly fill the selected
sector capacity. There is no implicit padding, resampling, insertion, relocation,
new loop metadata or ancillary-metadata loss. The explicit arithmetic model and
zero per-asset export history remain synthesis assumptions, not established game
seek/reset behavior. Codec loss is measured before any hardware resampling.

[`xa_selection`](../../../tools/rust/bof3-audio/src/xa/selection.rs) encodes
selected sectors without repeatedly rebuilding a whole STR for every cue. A
selection must be one ordered contiguous slice of its multiplexed stream.
Unchanged sectors reuse original bytes. Changed partial selections must have the
same entry history as a zero-initialized continuous-stream decode and must restore
the original exit history before any following unselected audio. Otherwise packing
rejects the edit with a boundary-specific diagnostic; callers can retain a
convergent tail or edit the complete stream. This preserves following sectors
under the declared decoder context, without claiming independent hardware fidelity.

All asset views of a shared physical sector must request identical encoded bytes;
an edited cue conflicting with an unchanged overlapping view fails rather than
silently overriding it. Rebuilding preserves unrelated sectors, source length,
multiplexing, cue placement, subheaders and spare bytes. Sector checks, whole-file
equality for unchanged sources and file read-back precede staged publication.
Output files are `streams/<SHA-256-of-source-identity>.STR`. `pack.json` uses
`bof3.xa-pack-report/v1` and records identities, changed sectors, byte equality,
per-asset encoding/error evidence and remaining limitations. The same existing
single-writer publication limits as bank packing apply.

[Selection tests](../../../tools/rust/bof3-audio/tests/xa/selection.rs) cover
entry/exit history rejection, accepted partial edits and unchanged cue reuse.
[Packing tests](../../../tools/rust/bof3-audio/tests/xa/packing.rs) cover all eight
supported rate/channel/depth combinations, missing original files, interleaving
and opaque-sector preservation, malformed metadata, capacity failures, CLI dispatch
and failed-publication cleanup. On 2026-09-24 the original disc/executable corpus
test passed whole-file equality for all four complete STR sources through 31
stream exports and for the three cue-bearing sources through 896 cue exports
(880 MAGIC00, 11 S_XA00 and 5 VOICE). The edited MAGIC00 cue (`0x1000`) test also
passed, preserving unselected sectors and rejecting runtime metadata conflicts.
Evidence is retained
under `out/audio-migration/xa-pack-*`. Complete game playback, automatic shorter-cue
padding, general seek/reset-dependent cue edits, musical packing and disc rebuilding
remain open.

### Gaussian interpolation

4-point interpolation using the hardware's 512-entry gaussian table
(extracted from DuckStation, verified against nocash specs):

```c
out  = (gauss[0x0FF - i] * oldest) >> 15;
out += (gauss[0x1FF - i] * older)  >> 15;
out += (gauss[0x100 + i] * old)    >> 15;
out += (gauss[0x000 + i] * new)    >> 15;
```

The interpolation index is pitch-counter bits 4–11. Bits 12 and above select
the decoded source sample. Steps above `0x4000` clamp to `0x4000` after optional
pitch modulation, so high pitches may skip decoded samples while still using
the same four-point interpolation window.

### ADSR envelope

From nocash PSX specs, cross-referenced with DuckStation:

```text
AdsrCycles = 1 << max(0, shift - 11)
AdsrStep   = step_value << max(0, 11 - shift)

if exponential AND increasing AND level > 0x6000:
    AdsrCycles *= 4    (or step /= 4 for rate < 40)

if exponential AND decreasing:
    AdsrStep = AdsrStep * level / 0x8000
```

| Phase | Mode | Direction | Step |
| --- | --- | --- | --- |
| Attack | Linear/Exp | Increase | +7,+6,+5,+4 |
| Decay | Exp (fixed) | Decrease | −8 |
| Sustain | Linear/Exp | Prog | +7..+4 / −8..−5 |
| Release | Linear/Exp | Decrease | −8 |

ADSR1: bit15=attack_mode, bits14-10=attack_shift, bits9-8=attack_step,
bits7-4=decay_shift, bits3-0=sustain_level (`(N+1)*0x800`).

ADSR2: bit15=sustain_mode, bit14=sustain_dir, bits12-8=sustain_shift,
bits7-6=sustain_step, bit5=release_mode, bits4-0=release_shift.

### Voice and transfer invariants

- KON clears the corresponding ENDX bit, resets ADSR level to zero, resets
  interpolation/predictor history, and starts attack from the configured start
  address. Writing a start address does not redirect an already playing voice.
- KOFF enters release from any ADSR phase. It does not immediately silence the
  voice.
- ADPCM flag bit 0 sets ENDX after the current block. With repeat bit 1 set it
  jumps to the repeat address; otherwise playback ends and release begins.
  Flag bit 2 copies the current block address into the repeat address.
- Fixed volume is signed and negative values invert phase. Bit 15 selects a
  separate sweep envelope, not a fixed magnitude.
- SPU RAM is not CPU-mapped. Transfer and voice addresses are in 8-byte units;
  the transfer FIFO port moves 16-bit values and the hardware FIFO holds 32
  halfwords. DMA uses channel 4.
- SPUCNT controls enable/mute, transfer mode, reverb, noise clock, and CD or
  external input. SPUSTAT reflects several changes after hardware delay rather
  than immediately.

PMON modulates voice `n` from voice `n-1`; voice 0 cannot be modulated. NON
replaces ADPCM with a shared hardware noise source, so `VxPitch` does not set
noise frequency. Reverb is a half-rate SPU-RAM feedback pipeline with per-voice
send bits and master enable/output controls. These features are implemented in
`spu_device.c` as coherent units (noise via the shared noise source, PMON from
the previous voice output, per-voice send bits, and the half-rate reverb
pipeline); they are not approximated by post-render DSP.

## Runtime loading

| EMI type | Handler | Action |
| ---: | --- | --- |
| 6 | `func_80162790` | Release prior owner, copy VH to RAM |
| 7 | `func_80162898` | `SpuSetTransferMode(0)`, `SsVabClose`, `SsVabOpenHeadSticky` |
| 8 | `func_801629F0` | Copy auxiliary audio (ADSR override table, `INFERRED`) |
| 9, 10 | `func_80162A6C` | Copy sequence to RAM |

### Executable sound control

The following behavior is derived from `SLUS_004.22` instructions and game
callers, not from PsyQ object implementations:

| Address | Binary-supported role |
| ---: | --- |
| `0x8015CEBC` | Audio shutdown/reset: all-key-off, closes active VAB IDs, releases seven active slots through a linked routine, disables reverb, and ends the sound runtime |
| `0x8015D044` | Polls the key state of all 24 SPU voices into the game voice-state table |
| `0x8015DF18` | Queued cue dispatcher used by overlays; its cases issue `SsUtKeyOnV` and detailed voice-volume operations |
| `0x80161BBC` | Ensures one logical audio bank is active, starting the EMI stream when the selected bank differs |
| `0x80161C20` | Starts and records a selected cue through game wrappers at `0x8015D300` and `0x8015D49C` |
| `0x80161CD0` | Updates a selected cue through the game wrapper at `0x8015D554` |

`0x8015CEBC` is therefore not the music scheduler or bootstrap entry point. A
standalone PSF bootstrap must reproduce the tables populated by the EMI audio
lifecycle before calling the cue-start path.

The linked calls observed below the game wrappers include addresses
`0x8016AE7C`, `0x8016B9CC`, `0x8016D6C4`, and `0x8016DBA0`. Their exact roles
and signatures remain address-based until all game-binary callers agree.

### Verified US sound startup and sequence storage

The Rust profile recognizes the original full `SLUS_004.22` by SHA-256
`0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a`.
Runtime addresses below belong to `exe/slus_004_22`; full-file offsets follow
`address - 0x80096800 + 0x800`. Recognition is not playback acceptance.

| Runtime address | Verified behavior |
| --- | --- |
| `0x8014EAA4` | Startup caller invokes game sound initialization at `0x8015CD00`. |
| `0x8015CD00` (file `0xC6D00`) | Initializes sound, sequence storage, tick mode, reverb and game audio state; returns at `0x8015CE68`. |
| `0x8016B2AC` | Calls callback initialization, the SPU initialization wrapper, and libsnd state initialization. |
| `0x8016D7EC` | Receives `(0x80148A50, 2, 4)` from the game; initializes two handles with four independent sequence-state records each. |
| `0x8016D9CC` | Receives tick mode `1` from the game and checks the video mode. Full callback cadence remains unverified. |

The table initializer stores handle/sequence counts at `0x80190B88`/`0x80190B8A`
and handle pointers at `0x80190308`. Each sequence state occupies `0xAC` bytes;
the second handle points to `0x80148D00`. The linked initializer executes in Rust
and its eight initialized records and two pointers pass direct memory checks.
The original pitch routine passes 4,250 instruction-derived formula comparisons;
the cue dispatcher passes checks for 255 numeric inputs plus the `0xFF` sentinel
return. Those checks prove the indexed lookup behavior, not 255 valid cue records;
the valid cue-table bounds remain unresolved.
These are bounded functional checks, not independent timing or PCM references.

The SDK-map label `SsInit` at `0x8017E0B4` is unsuitable as bootstrap evidence:
its first call reaches the BIOS B0/0x4C memory-card routine. The map is unchanged.
The Rust bootstrap probe executes original game/libsnd instructions and modeled
kernel services; it currently stops at unsupported BIOS CD-ROM removal `A0:72`
after 4,550 integer instructions. It does not skip this service or claim playback.
See [Rust profile](../../../tools/rust/bof3-audio/src/machine/profile.rs),
[runtime checks](../../../tools/rust/bof3-audio/tests/runtime.rs), and the
[migration plan](../../plans/bof3-audio-rust-migration.md) for remaining gates.

### Rust SPU disable and original hardware initialization

The US startup chain calls `0x8016B2DC` (full EXE `0xD52DC`), then
`0x80168F20` (`0xD2F20`) with argument zero. After callback initialization, that
routine calls the low-level hardware initializer `0x80168374` (`0xD2374`), which
returns through `0x80168604`. These addresses belong to `exe/slus_004_22` and
the exact profile identified above. Bounded Rizin 1.0.0 inspection and execution
use unchanged original instructions; no symbol-table names were promoted.

The [initializer check](../../../tools/rust/bof3-audio/tests/spu_bootstrap.rs)
starts at this low-level entry in an explicit zeroed device context. Argument
zero uploads the sixteen-byte dummy block at `0x80183AF0` (file `0xEDAF0`) to
SPU byte address `0x1000`, sets all 24 voices to zero gain/ADSR parameter registers, pitch `0x3FFF`
and start address `0x0200`, and writes key-on then key-off masks. It enables
SPUCNT `0xC000`, RAM control `4` and the SPU DMA priority field. Argument one
preserves voice/sample, modulation/noise and CD/external-gain settings, while
clearing master/wet gain, reverb sends and ten staging halfwords at `0x8018E230`.
Cold/warm/repeated-cold paths execute 47,550 / 3,743 / 47,550 instructions with
stack and saved registers intact. Unrelated SPU RAM remains unchanged.
The dummy block is not zero PCM: predictor zero, shift seven and repeated 7/0
nibbles decode to alternating `224, 0` with repeat flags. Its encoded SHA-256 is
`d761d406af2a4a5a15f67c924378ed88d1f85c13f1a37fc7366f59789b3bcd65`.
Zero voice gains suppress its output; no zero-filled waveform is inferred.

Repeated calls require a falling SPUCNT bit-15 transition. The interconnect now
accepts an explicitly configured `DisableModel::EmulatorReference`: active
envelopes become off with zero level at the accepted control write. Already-off
manual ENVX, pending keys, sample history/position, gains and ENDX are retained.
Transfer/IRQ validation precedes envelope changes, so a rejected write cannot
partially silence the device. The [source and limits](../../reference/spu-samples.md#disable-transition)
remain distinct from independently reproduced hardware evidence.

The test services at most one FIFO halfword after each integer instruction as
an explicit transaction schedule, without clock or voice-frame simulation.
Consequently the written key masks do not establish latch timing or audible
initialization. Disabled-frame execution still rejects. Full startup continues
to stop at BIOS `A0:72`; this direct callee check neither skips that call in the
bootstrap path nor supplies missing BIOS state. CPU/SPU timing, IRQ9, disabled
capture/CD behavior and independent trace/audio acceptance remain required.

### Rust system control and critical sections

[COP0 state](../../../tools/rust/bof3-audio/src/machine/cop0.rs) and the
[CPU](../../../tools/rust/bof3-audio/src/machine/cpu.rs) now implement status/cause
transfers, delayed MFC0 loads and RFE. Explicit exception delivery records EPC,
branch-delay/direction, target and bad-address information, and shifts the status
stack. The scheduler can supply an external interrupt line at an instruction
boundary; mask/current-enable checks decide delivery. This API does not advance
device time or establish interrupt latency. New CPUs start with zero status for
bounded loaded-program execution, not a claimed hardware-reset/BIOS-boot state.

The [kernel](../../../tools/rust/bof3-audio/src/machine/kernel.rs) handles BIOS
critical-section syscalls 1 and 2 through those transitions. Entry returns the
prior enable state; exit preserves the return-value register. The original US
wrappers at `exe/slus_004_22@0x8017EE0C` and `0x8017EE1C` (full EXE offsets
`0xE8E0C`, `0xE8E1C`) execute through their syscall and return instructions in
the Rust machine. Exit, entry and repeated entry verify enabled/disabled state
and return values. These are isolated wrapper checks, not BIOS-handler dispatch
or a completed game initialization.

[Checks](../../../tools/rust/bof3-audio/tests/cop0.rs) also cover transfer delay,
cause write masks, taken/untaken delay-slot exceptions, RFE in a return jump's
delay slot, address errors, hardware/software interrupt masks and ROM-vector
selection. Cache-isolated/user execution, unsupported COP0 registers/commands,
unknown syscalls and branch-delay syscall HLE remain explicit failures. Ordinary
unsupported machine faults are not silently converted into architectural traps.
The developer probe records COP0 state and modeled syscalls.

`A0:72` remains unsupported: research establishes CD-event closure and handler
removal obligations, including an original BIOS chain-removal bug. No CD-removal
no-op or invented initial BIOS event/chain state was added. Its implementation,
general exception handlers, scheduler timing and the full sound runtime remain
open. Retained [external references](../../reference/psx-kernel.md) distinguish
hardware rules and replacement-BIOS comparisons from original US evidence.

### Rust BIOS event state

[Event operations](../../../tools/rust/bof3-audio/src/machine/events.rs) use the
guest EvCB descriptor at RAM `0x120` (base pointer) and `0x124` (byte size).
Each `0x1C`-byte record retains class, status, spec, mode, handler and two opaque
words. No host-only event registry or assumed BIOS allocation is installed.
Missing/malformed descriptors fail before use. Handle lookup retains the low
16-bit slot rule; operations outside the allocated table fail explicitly rather
than pretending to reproduce unchecked BIOS memory effects.

The kernel now supports OpenEvent, CloseEvent, EnableEvent, DisableEvent,
TestEvent, DeliverEvent/UnDeliverEvent and free-slot queries. Allocation
chooses the first free record, returns `0xFFFFFFFF` on exhaustion and reuses
closed slots. Enable/disable do not allocate a free record; closure changes its
status without discarding other bytes. Delivery requires matching class and spec
plus busy status. Readiness is consumed once. Non-null callbacks execute through
the continuation described below; null callback handlers are skipped.

WaitEvent reports a pending call while leaving the CPU at the BIOS entry and
retaining the original event-record pointer. A later scheduler retry consumes
readiness and returns; disable/close does not release an already-busy wait.
Rebinding the table descriptor does not redirect that wait. Newly called waits
on free/disabled records return zero. Nested/changed wait contexts are rejected.
The developer probe reports a pending wait instead of spinning or claiming
completion. BIOS race timing and general thread/interrupt scheduling are not
modeled by this state transition API.

[Checks](../../../tools/rust/bof3-audio/tests/kernel_events.rs) cover RAM layout,
capacity/reuse, opaque fields, handle aliases, exact delivery matching,
readiness consumption, waiting, relocation, malformed inputs and callback yielding.
Original `exe/slus_004_22` wrappers execute in a synthetic table context:
OpenEvent `0x8017ED3C` (full EXE `0xE8D3C`), EnableEvent `0x8017ED7C`
(`0xE8D7C`), DeliverEvent `0x8017ED2C` (`0xE8D2C`), TestEvent `0x8017ED6C`
(`0xE8D6C`) and CloseEvent `0x8017ED4C` (`0xE8D4C`).

The supplied US `SYSTEM.CNF` has SHA-256
`f9d16f9537d12e07bd3a6f502ce375762de1d6e844b8ca075fe45a66121ef6ca`
and specifies hexadecimal `EVENT = 16` (**22 slots**), with `TCB = 4`.
This count does not establish the original BIOS allocation address or initial
event contents. The test's explicit 22-record allocation is not game bootstrap
evidence. CD-driver removal, initial BIOS events, general exception-chain execution
and full sound initialization remain open; `A0:72` still rejects.

### Rust BIOS event callbacks

[Delivery continuations](../../../tools/rust/bof3-audio/src/machine/event_calls.rs)
yield each matching callback to the normal guest CPU loop. Delivery captures the
table range, class and spec, but reads later records after each handler returns.
A handler can therefore enable or close a later record before it is examined.
Polling readiness already written before a later callback failure remains visible;
this supersedes the earlier polling-only implementation's atomic rejection policy.

The HLE bridge reserves `0xBFC00100` as a return trap and allocates a separate
16-byte O32 argument home area on an aligned guest RAM stack. It checks stack and
callee-saved register restoration, supports nested deliveries up to depth 64 and
restores the caller's stack and return address when scanning finishes. This is a
functional ABI bridge, not original BIOS code, an emulated TLB exception vector,
or proof of the original BIOS frame layout or volatile register values. Unaligned
or inaccessible handlers, invalid stacks, ABI violations and excessive nesting
fail explicitly. Instruction faults retain unfinished continuation state.

WaitEvent may yield inside a callback and resume through the same continuation.
Its pending context includes callback depth; a distinct nested wait cannot reuse
that context. The execution owner must bound guest instructions and HLE transitions
and reject nonlocal completion with outstanding continuations. The developer probe
does both and reports pending continuations. The bounded exception dispatcher
below permits explicit `B0:17` unwinding of callbacks created within that exception.
Other nonlocal callback completion and timed event scheduling remain unsupported.

[Callback checks](../../../tools/rust/bof3-audio/tests/event_callbacks.rs) cover
live later-record changes, descriptor rebinding, nested delivery, callback waits,
stack home-area isolation, invalid handlers, ABI violations, nesting limits and
guest faults. The optional original-executable check runs US `0x80161DC8` and its
callees as a synthetically registered BIOS event handler and checks the five
sequence callback pointers it installs. It does not establish that the game
registers this routine as a BIOS event, nor provide original BIOS timing or audio
reference evidence. Full runtime fidelity remains an acceptance gate.

### Rust BIOS exception-chain registration

[ExCB operations](../../../tools/rust/bof3-audio/src/machine/exception_chains.rs)
read the guest descriptor at `0x100`/`0x104` and require four eight-byte priority
records. C0 service `02` inserts a sixteen-byte node at the selected head and
returns zero; service `03` removes the exact head pointer and returns that
pointer. The node's first word links the next node, followed by the conditional
second-handler pointer, first-handler pointer and an opaque word. Registration
does not execute handlers. Removal preserves every word of the detached node,
including its old next link; unused priority-record words remain unchanged.

Raw pointers retain physical/KSEG0/KSEG1 alias bits. Removal compares those
pointer values exactly; aliases are normalized only for bounds/overlap checks.
The implementation rejects missing descriptors, invalid priorities, bad RAM
ranges, overlapping metadata/nodes, duplicate links, cycles and chains exceeding
4096 nodes before writing. A bus fault during the two enqueue stores is terminal,
not a promised atomic transaction. Each kernel call reloads the descriptor and
reads guest links, so guest edits and table rebinding remain visible.

Published BIOS evidence describes broken non-head removal using an uninitialized
stack value. Rust therefore rejects absent/non-head removal explicitly, without
silently repairing the linked list. The inspected OpenBIOS comparison supports
the head/return behavior but repairs the non-head case; its repaired search is
not adopted. See [external evidence](../../reference/psx-kernel.md#exception-chain-registration).
Original BIOS allocation/content and bug-specific stack effects remain unknown;
`A0:72` CD-driver removal still rejects, rather than claiming an empty driver.

Four default [checks](../../../tools/rust/bof3-audio/tests/exception_chains.rs)
cover all priorities, alias identity, opaque state, rebinding, malformed graphs
and bounded traversal. The optional US check executes original wrappers at
`exe/slus_004_22@0x8017F39C` / `0x8017F3AC` (full EXE `0xE939C` / `0xE93AC`),
then the original priority-1 removal caller `0x8017F27C` (`0xE927C`). The latter
requests node `0x8018DB40` and enters/exits a critical section. A synthetic ExCB
head makes that context explicit; the caller restores its stack/registers and
returns one. These checks establish bounded registration semantics, not original
BIOS initial state, exception dispatch, CD removal, interrupt timing or audio.

### Rust BIOS thread exception contexts

[Thread contexts](../../../tools/rust/bof3-audio/src/machine/thread_context.rs)
resolve the guest PCB descriptor at RAM `0x108`/`0x10C`, then its current-thread
pointer into the TCB allocation described at `0x110`/`0x114`. Each TCB occupies
`0xC0` bytes. Missing descriptors, overlapping metadata, invalid aliases/ranges,
non-slot pointers and non-used thread records reject explicitly; the model does
not invent BIOS allocation addresses or initial thread contents.

`Kernel::save_exception` captures after CPU exception delivery and before any
handler runs. General registers occupy TCB `+8+4*r`, excluding zero and K0; return
PC is `+0x88`, HI/LO `+0x8C/+0x90`, status `+0x94`, diagnostic cause `+0x98`.
Opaque words and unused register slots remain unchanged. Bus write failures are
terminal, not promised atomic captures. Interrupts at a COP2 opcode reject because
the observed BIOS PC-adjustment behavior requires GTE evidence. Saved PCs outside
supported aligned guest RAM also reject. This is a functional context operation,
not execution of BIOS exception-vector instructions.

BIOS `B0:17` (`ReturnFromException`) reloads the live PCB/TCB mapping, allowing
guest changes to saved state and current-thread selection. It reads and validates
the entire return context before changing CPU state, restores GPRs/HI/LO/status,
sets K0 to the return PC and applies RFE. Cause remains the current COP0 value;
the TCB copy is diagnostic. Branch-delay exceptions retain the branch EPC, and
return restarts that branch unless the guest changed the saved PC. Pending CPU
loads are resolved at the architectural/kernel boundary. Outstanding HLE event
callbacks or waits reject capture and standalone return. Within an active
exception dispatch, explicit `B0:17` can discard its owned continuations after
successful restoration, as described below. General nested exception handling
and thread scheduling remain open.

[Checks](../../../tools/rust/bof3-audio/tests/thread_context.rs) cover register and
opaque-state preservation, live aliases/descriptor rebinding, saved-state edits,
branch restart, pending loads, malformed contexts, read-fault atomicity, COP2
rejection and outstanding-continuation guards. The original US wrapper at
`exe/slus_004_22@0x8017EDDC` (full EXE `0xE8DDC`) executes three original
instructions to `B0:17` and resumes the explicit saved thread rather than its
caller. Its synthetic PCB/TCB is not evidence of BIOS boot contents. Retained
[references](../../reference/psx-kernel.md#thread-exception-contexts) and
`out/audio-migration/thread-context-*` separate these bounds from original-game
startup and independent audio fidelity. CD-driver removal `A0:72` remains
unsupported until initial BIOS event/chain state is established.

### Rust BIOS priority-chain execution

[`Kernel::begin_exception`](../../../tools/rust/bof3-audio/src/machine/kernel.rs)
captures the current guest thread after architectural exception entry, then
[dispatches](../../../tools/rust/bof3-audio/src/machine/exception_calls.rs) the four
ExCB priorities in order. The caller supplies an aligned RAM stack, GP and an
explicit missing-hook policy. Each handler gets a separate sixteen-byte O32
argument home area and returns through reserved trap `0xBFC00104`. Handlers run
through the ordinary guest CPU; stack/callee-saved ABI violations reject. This
functional bridge does not reproduce original BIOS volatile registers, frame
layout, instruction count or cycles.

The table is captured on entry; each later priority head is read live. A node's
verifier and second pointer are captured together before execution. A null
verifier skips both; a nonzero result is copied into A0 and invokes a nonnull
second handler. The next link is read after guest handlers complete. Cycles,
overlapping visited nodes, malformed pointers and more than 4096 nodes per
priority reject. The execution owner must separately bound guest instructions
and transitions; a guest fault leaves the continuation unfinished.

Completion restores the live `HookEntryInt` jump buffer and resumes its saved
RA with V0 set to one. Invalid hook PC/stack rejects. Without a hook, callers
choose explicit rejection or functional `ReturnFromException`; neither supplies
an original BIOS boot image. Entry rejects preexisting callbacks/waits and active
exception dispatch. Guest `B0:17` may end the chain early and discard callbacks
or waits created within it, only after successful TCB restoration. Failed
restoration retains them. Nested exceptions/syscalls remain unsupported.

Five default [tests](../../../tools/rust/bof3-audio/tests/exception_dispatch.rs)
cover ordering, conditional calls, live link/table changes, hook exit, normal
event callback return, explicit early return, failed restoration, malformed
graphs, bounds, ABI errors and guest faults. The optional US check executes
verifier `exe/slus_004_22@0x8017F31C` (full EXE `0xE931C`) and second handler
`0x8017F2B4` (`0xE92B4`). The verifier tests VBlank in I_MASK/I_STAT; the second
writes zero to SIO control `0x1F80104A` and runs its original countdown. Those
pointers are assigned to priority-1 node `0x8018DB40` by the original initializer
at `0x8017F1FC`. The test uses a synthetic ExCB and three explicit device input
pairs, not the original boot chain or a production SIO model.

[External comparison evidence](../../reference/psx-kernel.md#priority-chain-execution)
and `out/audio-migration/exception-dispatch-*` retain provenance. BIOS allocation,
CD-driver removal, timed IRQ dispatch, full runtime bootstrap and independent
audio reference comparison remain open. The developer probe reports outstanding
exception state and rejects premature completion; it does not fabricate initial
BIOS tables or automatically dispatch interrupts.

### Original SDK CD interrupt hook integration

The [XA scheduler fixture](../../../tools/rust/bof3-audio/tests/xa_loop_runtime.rs)
now also exercises COP0 interrupt entry and the existing BIOS context bridge,
with the original SDK hook dispatching CD callbacks. Target is verified US
`exe/slus_004_22`; full EXE offsets are runtime address minus `0x80096000`.

| Runtime address | Full EXE offset | Observed role |
| --- | --- | --- |
| `0x801748E4` | `0xDE8E4` | Callback-initializer wrapper |
| `0x80174A7C` | `0xDEA7C` | Installs setjmp/HookEntryInt state before CD removal |
| `0x80174914` | `0xDE914` | Registers IRQ2 callback through original SDK code |
| `0x80174B58` | `0xDEB58` | SDK interrupt dispatcher reached by installed hook |
| `0x80174C24` | `0xDEC24` | Acknowledges the selected I_STAT bit before callback |
| `0x80177264` | `0xE1264` | Existing CD completion/ready callback wrapper |

Running the initializer wrapper reaches unsupported `A0:72` after 4,542
instructions with hook buffer `0x80184640` installed. This prefix is retained;
the fixture does **not** continue it past CD removal or claim initializer
completion. Separate foreground calls use that installed hook and original
IRQ2 registration. Fixture-owned RAM explicitly supplies an empty ExCB table
at `0x80003000`, one used TCB at `0x80005000` and PCB at `0x80006000` through
their standard descriptors. These allocations and the empty BIOS chain are
not asserted to match Sony boot state. No SDK global or game completion flag
is patched to manufacture callback success.

At supplied instruction boundaries, CD replies assert the device line,
I_STAT/I_MASK drive COP0 pending state, and `Cpu::take_interrupt` enters the
exception. The kernel saves the foreground TCB and transfers to the original
setjmp hook. Original code acknowledges I_STAT, services the CD controller and
invokes game callbacks, then `B0:17` restores the interrupted thread. Every
return checks saved GPRs except BIOS-scratch `k0`, HI/LO, EPC and restored SR;
all entered exceptions must finish and leave no pending controller interrupt.

Full cue `0x2000` completes through states `5 → 6 → 7 → 0` at both retained
service schedules: 963 interrupts for one sector/tick and 792 for four.
The earlier raw-sector, PCM-frame, stop-threshold, Pause and zero-final-volume
checks still pass unchanged. Evidence uses `out/audio-migration/cd-irq-*`,
including bounded original disassembly and source hashes. This establishes
original SDK callback integration with the functional BIOS bridge, not BIOS
vector instructions, physical timing, full bootstrap or independent game PCM.
The user reports no BIOS dump is available; `_96_remove` and the corresponding
boot-state evidence gap remain explicit.

### Rust BIOS ROM execution probe

[Firmware storage](../../../tools/rust/bof3-audio/src/machine/firmware.rs) accepts
exactly 512 KiB, preserves every byte and records SHA-256. The interconnect can
attach one immutable image at the physical/cached/uncached BIOS windows. Byte,
halfword and word reads require alignment and bounds. Missing ROM, replacement,
ordinary/masked ROM stores, unsupported aliases and image sizes reject explicitly;
store rejection is a modeling limit, not a claim that hardware raises that fault.
[`Ram::from_bytes`](../../../tools/rust/bof3-audio/src/machine/bus.rs) separately
accepts exactly 2 MiB. Constructing the bus from RAM leaves devices at model
defaults and does not restore an emulator save-state.

The bounded developer [bios_probe](../../../tools/rust/bof3-audio/examples/bios_probe.rs)
executes every instruction through the CPU, including RAM BIOS vectors and ROM
addresses reserved as traps by the separate HLE kernel. It does not call that
kernel. Syscalls enter the architectural exception vector and execute supplied
guest handler instructions; other unsupported execution stops with fault context.
The caller must choose ROM execution or HLE dispatch explicitly, never intercept
real ROM addresses as HLE continuation traps.

```sh
cargo run --locked --manifest-path tools/rust/bof3-audio/Cargo.toml \
  --example bios_probe -- /local/BIOS.bin /local/context.json
```

The context is strict JSON with schema `bof3.bios-probe-context/v1` and fields:

| Field | Meaning |
| --- | --- |
| `provenance` | Nonempty caller-supplied description; not authenticated evidence. |
| `ram` | Raw RAM filename, resolved relative to the context file; null means explicit zero-filled RAM. |
| `entry_pc`, `status` | Aligned initial PC and supported COP0 status bits, as JSON integers. |
| `registers` | Exactly 32 unsigned GPR values; register zero must be zero. |
| `stop_pc` | Optional aligned stop address; null runs until fault or limit. |
| `transition_limit` | Required bound from 1 through 10,000,000, including syscall delivery. |

HI/LO, Cause/EPC/BadVaddr/TAR start at zero, PRID is the CPU model's fixed two,
and no delayed load/branch is pending. Inputs are therefore explicit bounded-call
contexts, not complete save-states or a verified reset environment. JSON input is
bounded to 64 KiB; ROM/RAM reads are bounded to their exact supported sizes. The
`bof3.bios-probe/v1` report records input hashes, initial context, stop reason,
CPU state, final RAM hash and the last sixteen transitions. BIOS revision,
clocks, hardware reset and audio fidelity remain unverified. Reaching `stop_pc`
is only the selected execution boundary, never automatic runtime acceptance.

Four [tests](../../../tools/rust/bof3-audio/tests/firmware.rs) cover byte identity,
mapping/bounds, immutable attachment, raw RAM/device separation, RAM-vector ROM
calls, and syscall/RFE execution from the ROM exception vector. Eight additional
probe checks cover successful calls, HLE-trap-address collisions, syscall entry,
loop limits, unsupported MMIO, invalid JSON fields/limits and short ROM images.
All ROM instructions in these fixtures are synthetic, with no proprietary BIOS
payload. Evidence uses `out/audio-migration/firmware-*`.

The original US startup caller `exe/slus_004_22@0x80174B30` (full EXE `0xDEB30`)
calls wrapper `0x8017ED14` (`0xE8D14`), which selects A0 service `72`. Its next call
at `0x80174B38` (`0xDEB38`) invokes `0x8017EE1C` (`0xE8E1C`), an explicit syscall-2
critical-section exit. This confirms the caller's sequence, not the BIOS body's
effects. No BIOS dump was found in the inspected project input/media/toolchain
paths. A supplied BIOS/context or independently established trace is still needed
to resolve boot allocations and CD-removal bug effects. The existing HLE startup
probe remains stopped at `A0:72`; no service-success substitution was added.
[External references](../../reference/psx-kernel.md#bios-rom-and-cd-removal-evidence)
and `out/audio-migration/cd-removal-*` retain the research boundary.

### Independent BIOS handoff and sound initialization comparison

The prepared US BIOS supersedes the earlier missing-ROM limitation. The generic
PCSX-Redux mission harness and Rust [capture reader](../../../tools/rust/bof3-audio/tests/reference/runtime.rs)
bind ROM/EXE/script/capture hashes and the pinned emulator revision. Live evidence
under `out/audio-migration/redux-generic-exe/` establishes equality of all 2 MiB
RAM, 32 GPRs and PC at `exe/slus_004_22@0x8014AA0C`, before its first instruction.
RAM SHA-256 is `6e9f0ab9998c4703ff1168dfd0d614a817561026a5463e199d42697d271b403a`.
The Rust ROM path retires 2,695,618 instructions; Redux reports 6,284,283 cycles.
Those counters measure different quantities and are not a timing equality claim.

Starting from that context, host-directed calls set PC, RA=`0x80010000` and zero
A0..A3, preserving other CPU state. The callback initializer at `0x801748E4`
returns with identical RAM, GPRs and PC. The sound initializer at `0x8015CD00`
returns with identical GPRs and PC but **27 differing RAM bytes**. The unmasked
opt-in [comparison](../../../tools/rust/bof3-audio/tests/boot.rs) remains failing.

At `0x80170714` (full EXE offset `0xDA714`), the original `lhu v0,12(a1)` reads
each voice's ENVX; `0x80170720` (`0xDA720`) stores it at
`0x8018DBF6 + voice*0x34`. When zero, the code at `0x8017072C..0x80170738`
sets that voice's bit in the mask at `0x80190C1C`. A 24-event Redux trace at
`0x80170720` records `v0=1` for every voice. Rust leaves these 24 fields zero,
and its mask is `0x00FFFFFF` versus Redux's zero, accounting for all 27 bytes.
The pinned Redux `src/spu/registers.cc:498` explicitly returns one for a newly
started voice or a pending envelope before sample processing. This explains a
reference behavior; it does not establish physical key/envelope timing or justify
hardcoding one in the Rust model. Resolve scheduled SPU frames and readback before
claiming sound-state parity. Device state, pending loads, full CPU state and PCM
are not captured by these missions.

Evidence: `out/audio-migration/redux-initialization/{callback,sound,sound-envelope-all}/`,
`redux-review/{boot-comparison,initialization-comparison,initialization-accesses}.log`
and `redux-review/envelope-reader.txt`. A separate trace-limit mission rejects
overflow with exit code one. These observations do not prove BIOS-free pruning,
audio fidelity or acceptance of the complete runtime.

### Verified US sequence event framing

For the verified US executable, `exe/slus_004_22@0x80161DC8` (full EXE offset
`0xCBDC8`) installs callbacks used by the dispatcher at `0x8016D0E0`
(file `0xD70E0`). The dispatcher selects a `0xAC`-byte sequence record through
the handle table at `0x80190308`. Record offset `+4` is the event cursor,
`+0x11` the running status class, and `+0x12` the current channel. The original
registration and bounded dispatcher/handler paths execute in the
[Rust runtime tests](../../../tools/rust/bof3-audio/tests/sequence/runtime.rs).
These tests supply synthetic RAM contexts and stop at real downstream callees
where stated; they do not establish voice allocation, scheduler timing or audio.

| Event | Original behavior |
| --- | --- |
| `9n key velocity` | Reads both data bytes and the following delta before calling the note callback at `0x8016A974`; zero velocity reaches that same callback. |
| `Bn controller value` | Calls `0x8016A55C`; that handler reads the value and dispatches selected controllers. Loop-control transitions are validated below; volume, pan and instrument semantics remain separate. |
| `Cn program` | Handler `0x8016AA5C` writes the channel program at record `+0x2C+channel`, then consumes the following delta. |
| `En low high` | Dispatcher consumes but discards `low`; handler `0x8016A4A4` passes only `high` to `0x801728E0`, with the selected bank/program. This is not standard 14-bit MIDI bend behavior. |
| `FF 51 tt tt tt` | Handler `0x8016A79C` reads three tempo bytes without a MIDI length prefix and stores integer `60000000 / tempo` BPM. Tick scheduling further quantizes timing. |
| `FF 2F` | Calls end handling at `0x8016CF1C`; bounded restart/counter transitions are verified below, separately from controller loops and audible voice release. |

The dispatcher recognizes status classes `90`, `B0`, `C0`, `E0` and `F0`;
other channel-message classes are not interchangeable with standard MIDI.
Running status retains the channel separately from the status class. Within
the meta class, `2F` selects end handling and other subtypes enter the tempo
callback; the Rust reader supports only the understood `FF 51` and `FF 2F`
forms and rejects other meta/system events explicitly.

The delta reader at `0x8016AAD4` (file `0xD4AD4`) consumes a VLQ, multiplies
its value by ten and accumulates it at record `+0x80`; event delay is stored
at `+0x88`. This internal unit does not by itself change the exported PPQN.
The runtime tests cover all eight handle/sequence contexts, non-canonical zero
VLQs, multi-byte deltas, all channels, program running status, note arguments,
all 128 bend low-byte values and non-integral tempo-to-BPM conversions.

[The Rust event reader](../../../tools/rust/bof3-audio/src/sequence/events.rs)
retains event order, encoded byte ranges, explicit/running status, both bend
bytes and the opaque suffix after end. It rejects truncated or unsupported
events, zero tempo, non-seven-bit channel data and VLQs longer than four bytes.
Its result reports `semantic_validation: framing_only`: controller meanings,
loop translation, usable delay ranges, pitch response and scheduler timing
are separate gates. No MIDI/SF2 export or playback claim follows from parsing.

All 476 original SEP sequences pass the
[framing checks](../../../tools/rust/bof3-audio/tests/sequence/events.rs).
An independent raw-TOC Python survey agrees on 508,471 events: 462,686 notes,
7,149 controllers, 1,902 program changes, 36,258 bends and 476 end markers.
Every sequence has one preserved zero byte after end. The corpus contains no
tempo-change events; synthetic original-runtime checks cover that path.
Controller counts are `6:282`, `7:1905`, `10:3842`, `98:186`, `99:934`.
These counts establish framing coverage, not interpretation of those controllers.

#### Sequence controller loop transitions

The same US callback registration selects `0x8016A278` (full EXE offset
`0xD4278`) for controller 99, `0x8016A140` (`0xD4140`) for controller 98,
and `0x80169AB8` (`0xD3AB8`) for controller 6. These use one sequence-wide
loop context, independent of the current channel. It is not a nested-loop stack.

| Record offset | Loop state |
| --- | --- |
| `+0x0C` | Saved event cursor |
| `+0x10` | Count already active |
| `+0x16` | Controller-99 selector |
| `+0x27` | Count assignment pending |
| `+0x28` | Remaining count |

Controller `99=20` sets the pending flag and captures the cursor **after**
consuming the following delta. A second start overwrites this cursor without
resetting an active count. The next controller 6 or 98 assigns its value as the
count only when pending and not already active. Replaying that count event
after a jump therefore does not reload a finite count.

Controller `99=30` behaves as follows:

- Count zero clears the active flag and proceeds. Count one decrements to zero
  and also proceeds; both execute the body once when supplied at its start.
- Counts 2–126 decrement, then jump to the saved cursor while still nonzero.
  The delay consumed **after the end marker** applies to the jumped-to event;
  the original delta before that target is not reread.
- Count 127 jumps without decrementing and forces delay zero. The following
  delta is still consumed by the original reader, including its accumulator
  side effect, but does not become the repeat delay.

The [Rust loop model](../../../tools/rust/bof3-audio/src/sequence/loops.rs)
preserves these transitions and rejects other NRPN contexts. It models cursor
and count control only: it does not replace program-info lookup, other controller
effects, end-of-sequence handling or the scheduler. Callers provide the initial
saved cursor explicitly. Zero-delay loops still require bounded rendering.

[Original-executable tests](../../../tools/rust/bof3-audio/tests/sequence/runtime.rs)
compare model state and delays for both count controllers, counts
0/1/2/3/126/127, alternating channels, overwritten starts and repeated count
events. Controller 6 executes the actual program-info getter with an unopened
bank, which returns early; no callee is patched or substituted. A separate
synthetic stream lets original handlers advance and repeat the event cursor
without host cursor rewrites, covering program replay, implicit controller
status and finite/infinite repeat delays. This establishes bounded event control
flow, not real-time cadence or PCM fidelity.

The physical corpus scan finds 468 starts (`99=20`), 466 ends (`99=30`),
282 counts via controller 6 and 186 via controller 98. Every count is 127;
all occur in the supported loop context. These are physical event counts,
not executed traversal counts. Unmatched starts are preserved; no automatic
pairing, nesting interpretation or end-marker insertion is justified.

#### Sequence end, restart and per-tick scheduling

In `exe/slus_004_22`, end handling at `0x8016CF1C` (full EXE offset
`0xD6F1C`) increments the halfword completed-play counter at record `+0x48`.
Its wraparound is retained, and comparisons use signed 16-bit values. The
requested play count at `+0x46` is separate from controller-loop count `+0x28`.

- Requested count zero restarts indefinitely: copy restart cursor `+8` to
  current cursor `+4`, clear elapsed units `+0x80`, delay `+0x88` and count-pending
  byte `+0x27`. The saved controller-loop cursor `+0x0C` is retained.
- A nonzero count restarts while the incremented signed completed count is
  below the requested signed count. It performs the same resets and also copies
  the restart cursor into the saved controller-loop cursor.
- Otherwise, flags `+0x90` become `(flags & ~0x0B) | 0x204`, byte `+0x2B`
  clears, and the saved loop cursor resets. Current cursor and elapsed units
  are retained. Delay becomes the sign-extended halfword tick quantum `+0x70`.
  End handling does not consume the byte following `FF 2F`.

Restart does not clear the controller-loop active/count fields `+0x10/+0x28`.
Signed counter wraparound is observable runtime behavior, not permission to
accept arbitrary edited play counts. The
[Rust end model](../../../tools/rust/bof3-audio/src/sequence/termination.rs) exposes these
raw transitions; user-facing representability checks remain separate.

On stop, a successor handle in byte `+0x3C` other than `FF` causes activation
of the sequence selected by byte `+0`. The original callee at `0x8016D3C4`
(file `0xD73C4`) sets that target's requested/completed counts to `1/0`, copies
its restart cursor into its current cursor, sets byte `+0x2B` to one and sets
flags to `(flags & ~0x30E) | 1`. It does not reset the target delay.
The ending routine also calls the original voice-release owner at `0x8017301C`.
Tests use zero active voices and a distinct successor: voice release envelopes,
self-links and real game chain use remain separate execution evidence gates.

The scheduler at `0x8016CE0C` (file `0xD6E0C`) compares the current signed
delay to the signed halfword quantum. With delay above the quantum, positive
slow counter `+0x6E` merely decrements; zero reloads that counter from the quantum
and decrements delay by one; negative subtracts the quantum from delay. When
delay is at or below the quantum, the scheduler dispatches events, consuming
zero-delay events in the same call. Nonzero next delays accumulate until they
reach the current quantum; the remainder is saved as delay. It rereads the
quantum after handlers, so a tempo change affects that same call.

[The Rust clock model](../../../tools/rust/bof3-audio/src/sequence/clock.rs)
matches this arithmetic and event order, including wrapping operations. A
required event budget prevents zero-delay cycles from running forever; budget
exhaustion or callback failure cannot be reported as successful rendering.
This is one scheduling call, not an established timer frequency or hardware IRQ
model. Original timing still depends on callback cadence, initialization and
SPU/device execution.

[Original-executable checks](../../../tools/rust/bof3-audio/tests/sequence/runtime.rs)
compare end-model fields in 320 contexts spanning all eight handle/sequence
records, zero/finite/negative play counts, signed counter boundaries and two
quantum values. Additional tests execute distinct-successor activation, 864
scheduling calls across delay/quantum/slow-counter combinations, and a tempo
change within a scheduling call. No original callee is replaced. These tests
establish functional transitions, not PCM fidelity or hardware cadence.

### Verified US EMI audio slot selection

For the same US executable hash, the original layout initializer at
`exe/slus_004_22@0x80161808` (full EXE offset `0xCB808`) executes 199 instructions
for each selector `0`, `1`, and `2`. It fills seven 20-byte records at
`0x8014677C` (file `0xB077C`), containing SPU base, VH destination, auxiliary
destination, sequence destination, VAB ID, and flags. All three layouts assign
VAB IDs `0`–`6`; the active layout is gameplay context, not an archive property.
The three seven-word allocation tables start at `0x80182348`, `0x8018239C`, and
`0x801823F0` (files `0xEC348`, `0xEC39C`, `0xEC3F0`): SPU bases, VH capacities,
and sequence capacities. Zero allocations do not authorize loading a bank there.

The following addresses are all in `exe/slus_004_22`:

| Runtime address | Full EXE offset | Instruction-backed association |
| --- | --- | --- |
| `0x80162B08` | `0xCCB08` | Stages the EMI TOC second word as the handler's load argument. |
| `0x80162790` | `0xCC790` | Type 6 selects a logical audio slot from that argument, follows its resource ID, and chooses its VH destination. |
| `0x80162898` | `0xCC898` | Type 7 opens the selected VH with its VAB ID and SPU base. |
| `0x801629F0` | `0xCC9F0` | Type 8 copies to the selected slot's auxiliary destination. |
| `0x80162A6C` | `0xCCA6C` | Types 9/10 copy to its sequence destination and set flag bit 1. |
| `0x801635D0` | `0xCD5D0` | Sequence-open callsite passes that pointer, VAB ID, and four sequences. |

Body, auxiliary, and sequence handlers use the most recently selected VH slot;
they do not select a bank from their own TOC second word. An audio payload before
a verified VH therefore needs prior load context. The VAB header ID is separate:
all 1,020 local corpus headers declare zero, while their loader slots span `0`–`6`.
Following loader order associates all 1,020 bodies, 904 auxiliary entries, and
119 SEP bundles. This does not classify sample content as SFX or vocals.

[Rust mapping checks](../../../tools/rust/bof3-audio/tests/catalog/loader.rs) execute
the initializer and original argument paths for all 21 layout/slot combinations.
They stop before CD copy and SDK bank/sequence opening calls: transfer behavior,
SPU timing, scheduler behavior, and rendered audio remain separate acceptance gates.

### Verified US music cue and disc-file identities

The original data track SHA-256 is
`94835d58c8b19c39b551039010ee9669861f1421958002b2f6927bb2d50f2f55`.
Its ISO directory independently verifies the 81 BGM paths, LBAs, sizes, and
whole-file hashes recorded in the Rust
[identity metadata](../../../tools/rust/bof3-audio/src/catalog/music.json); no game
payloads are embedded. File slots `209`–`289` agree with the US executable's
LBA table at `exe/slus_004_22@0x80182444` (full EXE offset `0xEC444`). The linked
lookup routine is `exe/slus_004_22@0x80162160` (file `0xCC160`).

The cue prefix at `exe/slus_004_22@0x80181EB8` (file `0xEBEB8`) has 165
consecutive records of `(u16 file_slot, u8 game_bank, u8 sequence_index)`.
These cover all 81 verified BGM files and 165 independent sequence selections.
The next record is not a supported music file; caller-enforced bounds beyond
this media-backed prefix remain unverified. Original instructions at
`exe/slus_004_22@0x80161BBC` (file `0xCBBBC`) select the file and avoid reloading
the current file. The cue dispatcher separately uses the bank/sequence bytes.
[Rust checks](../../../tools/rust/bof3-audio/tests/catalog/music.rs) verify the ISO extents,
archive bytes, and original file-selection/LBA routines for all supported cues.

`query --mode music --cue NUMBER --executable PSX_EXE` resolves cues `0`–`164`
against supplied archives. Song bundles retain multiple `game_song_ids`; SEP
sequence indices and encoded sequence IDs remain separate. Full archive hashes
support renamed originals; changed bytes require preserved identity evidence
from interchange rather than filename guessing. Identical shipped archives
(`BGMBAT00`/`BGMBAT02`) retain distinct identities by qualified disc path plus
hash. Multiple supplied copies without that qualification require `--id`.

### PSF1 image contract

The PSF1 module enforces:

- PSF version `0x01`, compressed-program CRC, and bounded zlib expansion.
- A valid PS-X EXE program no larger than the PSF1 limit.
- Text overlays fully contained in 2 MiB PlayStation RAM.
- `_lib` recursion limited to 10 levels.
- `_lib`, current image, then contiguous `_lib2` and later overlay order.
- Initial PC/SP inherited from the first/deepest base image.
- First applicable `_refresh` tag, otherwise the outer EXE region marker.

For the local BOF3 executable, `psf-pack` followed by `psf-inspect` reports:

```text
PC:      0x8014AA0C
SP:      0x801FFFF0
RAM:     0x96800-0x1F7000
refresh: 60Hz
```

## Tooling

### Evidence and limits

| User-visible capability | Evidence | Limit |
| --- | --- | --- |
| PSF pack/inspect and bounded `psf-run` | native `psf_test`; native `psx_machine_test` | The machine faults on unsupported CPU/BIOS/hardware paths and does not bootstrap BOF3's audio scheduler. |
| Direct BGM render | `spu_device_test` covers live voice looping, key-off, and pitch cap; source audit in `audio_audit.c` | It is an approximate offline SEP/VAB renderer; it does not execute the game runtime. |
| XA decode to WAV | native `xa_test` decodes a synthetic audio sector and parses its WAV output | CD/XA is not mixed or captured through the SPU. |
| VAB WAV/SF2 and SEP MIDI export | CLI paths are implemented in `vab.c`, `sf2.c`, `sep.c`, and `export.c` | No fixture or retail-media golden output is claimed. |
| Ogg/FLAC output | writers in `ogg.c`/`flac.c` (build-time feature detection) | Codec support depends on the libraries detected by CMake on the build host; no codec-output golden is claimed. |

The SPU's reverb, noise, pitch modulation, and volume sweeps are implemented
in `spu_device.c` and used by the `fast` renderer; exact DMA/FIFO/IRQ timing and
CD/XA mixing/capture remain unsupported, and no command claims hardware-fidelity
timing.


### C tool (`tools/c/psx-audio/`)

Self-contained C11 library + CLI. Uses miniaudio for playback.
Gaussian table and ADSR from DuckStation (hardware-verified).

```sh
bin/harness audio build <command>           # incrementally builds, then runs ignored build/bof3-audio
```

| Command | Description |
| --- | --- |
| `play <vh> <vb> <sep>` | Render BGM + play through speakers |
| `play-xa <str> [-c CH]` | Decode XA + play |
| `play-vag <vh> <vb> [-v N]` | Play VAG sample(s) |
| `render <vh> <vb> <sep> -o out.wav` | Render BGM to WAV |
| `xa-decode <str> -o out.wav [-c CH]` | Decode XA to WAV |
| `xa-inspect <str>` | List XA streams |
| `vab-extract <vh> <vb> -o DIR` | Extract VAGs to WAV |
| `vab-inspect <vh>` | Show VAB info |
| `sep-inspect <sep> [--programs] [--notes]` | Show SEP info and optional program/note histograms |
| `sep2mid <sep> -o out.mid` | Export to Standard MIDI |
| `psf-pack <PS-X EXE> -o out.psflib` | Package a PSF1 executable |
| `psf-inspect <file.psf>` | Validate and compose a PSF1/MiniPSF image |
| `psf-run <file.psf> [-n N]` | Run a bounded machine diagnostic |

The CLI exposes only the direct SEP/VAB renderer. BOF3 linked-runtime
execution is not a supported render mode: the bounded PSF machine is a
separate diagnostic, and its missing game-owned scheduler/table bootstrap
prevents it from producing audio.

### ffmpeg validation

ffmpeg has `adpcm_xa`, `adpcm_psx` decoders and `psxstr` demuxer for
cross-validating our output:

```sh
ffmpeg -f psxstr -i wrapped_2352.str -vn output.wav
```

## File locations

| Path | Content |
| --- | --- |
| `tools/c/psx-audio/` | C library + CLI source |
| `tools/c/psx-audio/util.h` | Shared helpers and Gaussian table |
| `tools/c/psx-audio/adpcm.c` | ADPCM decode core |
| `tools/c/psx-audio/spu.c` | ADSR envelope |
| `tools/c/psx-audio/spu_device.c` | Register-driven SPU device (live decoder, sweeps, noise, PMON, reverb) |
| `tools/c/psx-audio/psx_machine.c` | Bounded PSF1 R3000 runtime under construction |
| `tools/c/psx-audio/psf.c` | PSF1/MiniPSF image loader and writer |
| `tools/c/psx-audio/render.c` | Approximate `fast` BGM renderer |
| `tools/c/psx-audio/third_party/miniaudio.h` | Audio playback (v0.11.25) |
| `bin/harness audio build` | C tool wrapper |
| `out/extracted/BIN/BGM/` | Extracted BGM archives |
| `out/extracted/BIN/SCE_XA/` | Extracted XA streams |
| `out/extracted/BIN/**/emi.json` | Per-archive EMI entry manifests |

## Open questions

- Standalone bootstrap: recover the EMI-populated bank/sequence tables needed
  by `0x80161C20` and the callback cadence that services active sequences.
- Linked sequence calls: prove names and signatures for `0x8016AE7C`,
  `0x8016B9CC`, `0x8016D6C4`, and `0x8016DBA0` from game-binary callers.
- Type-8 semantics: ADSR override table structure plausible but unconfirmed.
- SPU RAM layout: VAB base addresses not documented.
- Area→BGM mapping: lives in scenario controller code, not yet lifted.


## Initial channel programs and editable empty programs

The exact US executable's SEP open routine at `0x8016B38C` calls the initializer
at `0x8016B4B8`. Its loop at `0x8016B580..0x8016B59C` writes channel index to
sequence-record offset `0x2C+channel`, pan 64 to `0x17+channel`, and volume 127
to `0x4E+channel*2`. The [original-runtime test](../../../tools/rust/bof3-audio/tests/sequence/runtime.rs)
checks all 16 channels across different handles and sequence records. MIDI setup
uses these program defaults and retains source event ordering.

BGM053 contains populated program 10 with nine tones. Original note-on calls
at `0x8017102C` for its empty program 1 return zero without allocating a voice.
The probe `out/audio-migration/program-init-bgm053.json` records initial programs
0–15 and four calls: program 10 returns voices 1 and 2, while program 1 returns
zero twice. Media hashes, original initialization calls and arguments accompany
the trace. This is execution evidence within the Rust machine, not independent
hardware fidelity evidence.

The exported SF2 gives every zero-tone VAB slot an explicit silent preset and
percussion alias. These reference one separate, nonlooping zero sample and
instrument; XML distinguishes their playback indices from original VAB samples
and tones. Standard SF2 consumers may allocate a short silent voice, unlike the
game's no-allocation behavior. Reports state that approximation.

To populate an empty program, edit its SF2 preset to reference one to sixteen
existing VAB tone instruments, in the desired layer order, and give its bank-128
percussion alias the same assignments. These are tone templates: packing copies
their validated, edited VAB rows into a new program block and changes the owning
program reference. Templates must share program volume, priority, mode and pan.
Their supported gain, tuning, sample assignment and PCM edits still pass through
the existing inverses. Shared instrument edits also affect the original programs
that use those instruments. Creating independent new instrument/sample structures,
changing preset-level generators/modulators, or replacing populated program
assignments remains unsupported and rejects explicitly.

Packing inserts new 512-byte tone blocks in ascending program order, updates
program/tone totals and declared size, and preserves old blocks, sample-size
table, body and opaque trailer. Growth must fit the original EMI sector
allocation and a verified game VH/SEP/SPU layout. Alias conflicts, more than
16 layers and capacity failures publish nothing. The shared generated silent
instrument is not a VAB tone template and its unrepresented edits reject.
All 128 MIDI program selections are representable; unpopulated slots stay silent.

Current XML requires `initial_programs="channel_index"` and
`empty_programs="silent"`. There is no older-export reconstruction branch;
missing or different policies require re-extraction with the current tool.
[Population tests](../../../tools/rust/bof3-audio/tests/soundfont/packing/population.rs)
verify layered reconstruction and exact PC playback after re-export.
[Music integration](../../../tools/rust/bof3-audio/tests/music/programs.rs)
checks whole-file unchanged BGM053 equality, failed capacity publication, and
both original-runtime and PC playback before and after program 1 gains a tone.
Receipts are `out/audio-migration/program-population-*.log`.


## Standalone driver reachability audit

The development-only [driver audit](../../../tools/rust/bof3-audio/src/driver/closure.rs)
and [closure probe](../../../tools/rust/bof3-audio/examples/closure.rs) begin the
standalone-driver work. They do not extract, prune or authorize removal of code.
Reports qualify addresses by executable SHA-256, runtime address and full-file
offset; RAM aliases map to the same payload offset. Bounded traversal retains
branch delay slots, conditional alternatives, direct call targets and possible
continuations. Indirect targets, register returns, exception handlers and unknown
opcodes remain unresolved. Call targets are candidates, not recovered function
boundaries; data closure and pruning authorization are explicitly false.

The exact-US [root inventory](../../../tools/rust/bof3-audio/src/driver/roots.rs)
includes verified sound/sequence/pitch initialization, music selection, SFX
`exe/slus_004_22@0x8015DF18` (file `0xC7F18`) and XA selector/start/callback/tick
routines. Original SFX instructions at `0x8015DF1C..0x8015DF50` mask cue bits
`0xF00`, shift by eight, load `0x8018232C + selector*4` and call through the
loaded register, with the original delay slot retained. All sixteen possible
snapshot reads are recorded. Seven entries point into the executable:
`0x8015E994`, `0x8015EFAC`, `0x8015F5C8`, `0x8015FBE4`, `0x80160200`,
`0x8016081C`, `0x80160E38`. The remaining words overlap non-code values;
this does not prove a sixteen-handler table or that selectors 7–15 are legal.
Allowed caller domains and initialization writes remain unresolved. Reviewed
symbols and partial recovered C are corroborating leads, not replacement proof.

Post-bootstrap sequence-0 probes for BGM053 and BGM004 request 44,100 reference
sample frames and include original play/stop calls; stop completion brings each
to 44,101. The BGM053 instruction audit includes 14,214 candidate instructions,
118 direct call entries, 1,503 delay slots and 191 unresolved sites (139 register
returns, 30 other indirect transfers, 20 breaks and two syscalls). These are
conservative candidates, not a minimal driver or a function count. BGM004's
context-expanded audit includes 13,544 instructions and 184 unresolved sites.

Both intervals execute 54,180 instructions in low RAM populated by BIOS boot,
with no BIOS-ROM instruction during the measured interval. Each enters vector
`0xB0` with selector `0x17` sixty times. Absence of ROM execution therefore does
not establish BIOS independence. The observer records re-entry after low-RAM
PC `0xE9C` at original `0x80174AE4` (file `0xDEAE4`), and returns after
`0x1014` to the guest idle boundary. `0x80174AE4` is a resumed instruction
inside an initialization/control path, not an inferred function entry. Its
conditional call reaches `0x80174B58`. Adding observed re-entry roots accounts
for 34 original instructions absent from direct-root traversal. No measured
executable-resident PC remains outside either final context-expanded audit.

The BGM053 trace records 1,220 distinct executable-instruction memory footprints
across 537 addresses; BGM004 records 1,075 across 476. Each footprint carries
source PC/file offset, effective address, width, access direction and hit count.
IRQ/SPU registers occur among them. These are pre-instruction address observations
for unchanged executable-resident code, not complete data ownership: memory
accesses performed by low-RAM/ROM code, bootstrap initialization, cache-content
provenance, other sequences and dynamic SFX/voice/XA paths remain unmeasured.
Register dispatch targets and re-entry predecessors are likewise contextual
observations, not proofs of all possible targets or callable entry boundaries.

Receipts: `out/audio-migration/driver-closure-final-bgm053.json`,
`driver-closure-final-bgm004.json`, bounded `driver-closure-*-rizin.txt`, and
`driver-closure-rizin-status.json`. Reproduce with the release example
`closure US_EXE BIOS EMI FRAMES`; omit the last three arguments for a static
root-inventory audit. Six focused [checks](../../../tools/rust/bof3-audio/tests/driver/closure.rs)
cover delays, indirect/exception boundaries, aliases, external targets, limits,
profile rejection and the original cue table snapshot. The next required work
is initialization/data ownership and complete SFX/voice/callback coverage;
no leaf pruning or standalone execution gate is accepted yet.


### Driver preparation dependencies

The [music preparation observer](../../../tools/rust/bof3-audio/src/machine/music.rs)
now labels every guest instruction in six stages: initialization, layout, VAB
header, sample upload, SEP open and callback registration. It runs after IRQ
admission and cannot mutate the machine. The ordinary preparation entry uses
the same implementation with an empty observer. Observer failure aborts the
call; BIOS boot, host archive staging and separate layout scouting are outside
this observation scope. [Tests](../../../tools/rust/bof3-audio/tests/driver/preparation.rs)
compare full RAM/SPU RAM, registers, guest-call results and instruction counts,
then identical PCM after 8,192 reference frames. The observed instruction count
equals successful guest instructions plus handled syscalls across every call.

The [initialization probe](../../../tools/rust/bof3-audio/examples/initialization.rs)
records BIOS vector/selector, stage, return PC, arguments and the jump buffer
passed to `HookEntryInt`. Both BGM004 and BGM053 initialize through the same
observed service sequence. Initialization uses 67,619 observed instructions:
66,411 executable-resident, 1,119 other RAM and 89 BIOS-ROM instructions.
The service calls are:

| Service | Calls | Observed role/context |
| --- | ---: | --- |
| A0:13 | 1 | Save jump buffer at `0x80184640`; return PC `0x80174AE4` |
| A0:72 | 1 | Remove CD driver |
| A0:A3 | 1 | Remove CD interrupt handlers, called from BIOS `0xBFC07320` |
| B0:08 / B0:0C | 1 each | Open/enable event class `0xF0000009`, spec `0x20`, mode `0x2000`, no callback; handle `0xF1000000` |
| B0:09 | 5 | CD removal closes five handles, all zero in this boot context |
| B0:19 | 1 | Register jump buffer `0x80184640` as interrupt-entry hook |
| B0:5B | 1 | PAD auto-ack argument zero |
| C0:03 | 2 | Priority-zero chain records `0xA00091D0` and `0xA00091E0` |
| C0:0A | 1 | Counter selector 3, auto-ack argument zero |

At hook registration the saved return PC is `0x80174AE4`, SP `0x80185620`,
FP `0x801FFF00`, S0 `0x80184608`, GP `0xA0010FF0`, and S1–S7 are zero.
These are observations of BIOS-backed startup, not constants approved for a
standalone initialization image. They connect the preceding playback re-entry
at `0x80174AE4` to the original setjmp/hook setup. Mutable state ownership,
register liveness and independently initialized replacements remain to be proven.

PCSX-Redux at commit `28438546c781fbe372a06399c82bed43ca2c6f4d` maps A0:A3 to
`dequeueCDRomHandlers` in
[its dispatch table](../../../third_party/pcsx-redux/src/mips/openbios/kernel/handlers.c).
[CD deinitialization](../../../third_party/pcsx-redux/src/mips/openbios/cdrom/cdrom.c)
closes five events and removes the two handlers described by
[its CD state machine](../../../third_party/pcsx-redux/src/mips/openbios/cdrom/statemachine.c).
This corroborates the original BIOS trace; it does not authorize substituting
OpenBIOS behavior for unverified retail behavior or importing that implementation.

BGM053 upload executes 486,071 observed instructions and uses 24 B0:07 event
deliveries, 5,399 B0:0B event tests and 24 B0:17 returns. BGM004 uses 1,740,618
instructions, 86 deliveries, 19,320 tests and 86 returns. Header stages use
3,151/3,458 instructions respectively; both use 199 for layout, 1,485 for SEP
open and 2,769 for callback registration. The two preparation traces observe
3,765 distinct original instructions and seed the same 14,612-instruction
conservative closure. Its 249 unresolved sites remain explicit; initialization
observation does not establish complete data ownership or SFX/voice coverage.

Reproduce with `initialization US_EXE BIOS EMI`. Receipts are
`out/audio-migration/driver-initialization-bgm053.json`,
`driver-initialization-bgm004.json` and `driver-initialization-*.log`.
No BIOS path was removed or replaced by a service-success stub in this step.

### Explicit SFX tone execution

[Shared bank preparation](../../../tools/rust/bof3-audio/src/machine/bank.rs)
initializes and uploads VABs without a SEP; music uses the same implementation
before opening its sequence. It checks header/body and optional sequence ownership,
layout capacities, DMA padding and transferred SPU bytes. Existing music checks
still establish identical RAM/SPU RAM, registers, call results, counts and PCM.

US `SsVabTransBodyPartly` at `0x80174354` (EXE offset `0xDE354`) returns `-2`
while data remains, then the selected VAB ID. The `move v0,s2` at `0x80174468`
confirms the final result; expecting zero only worked for bank zero. Both effects
archives below load into runtime bank 1 despite having VH header ID zero, and
their final transfers return 1.

The [explicit-tone adapter](../../../tools/rust/bof3-audio/src/machine/effects.rs)
calls original `SsUtKeyOnV` at `0x8016E400` (EXE offset `0xD8400`). Its arguments
are voice, bank, program, tone, note, fine, left volume and right volume. The
first four use argument registers, the others caller SP+16..+28. The adapter
reserves a 32-byte o32 argument area and restores caller SP, including on observer
failure. Original voice indices are 0–23; `-1` is rejected, not an allocation
request. Before guest mutation, the adapter validates the populated program/tone,
canonical sample reference and explicitly supported note/fine/volume range 0–127.

The adjacent wrapper at `0x8016E794` (EXE offset `0xD8794`) validates the same
voice range, sets the current voice and calls `_SsVmKeyOffNow` at `0x8017212C`
with `a0=0`; success returns zero. It releases voice 23. In contrast, original
initialization calls `0x801737E8` with 16, setting `0x8018E264`. `SsUtAllKeyOff`
at `0x8016DEBC` loops only over that managed count, so it cannot prove release
of every explicit SFX voice.

The [effects probe](../../../tools/rust/bof3-audio/examples/effects.rs) observes
bank preparation, callbacks, key-on, playback and per-voice release, excluding
BIOS boot and host staging. It records original PCs, BIOS services, indirect
targets, external re-entry, voice registers, WAV evidence and conservative closure.
Final probes use 1,024 body frames and 44,100 tail frames at the reference clock:

| Archive | Selection (bank 1, program 0, fine 0, L/R 100) | Pitch | PCM peak | Nonzero scalar samples |
| --- | --- | ---: | ---: | ---: |
| `BIN/BATTLE/COMN_SE.EMI` | tone 0, voice 7, note 24 | 4096 | 1277 | 1410 |
| `BIN/BATTLE/BATL_SE.EMI` | tone 2, voice 23, note 41 | 4155 | 8461 | 705 |

Both produce 45,124 stereo frames; envelopes fall from 32767 to zero after release.
BATL tone 2 has left pan and fine tuning in its VH, explaining its zero right
voice volume and non-4096 pitch at the center note. BATL tone 0 has zero volume;
successful key-on correctly remains silent. [Media checks](../../../tools/rust/bof3-audio/tests/driver/effects.rs)
exercise all three cases, selected-voice key-on/off masks, ABI arguments, stack
restoration and rejection without guest mutation. Later runtime flushes write
zero to key registers; tests accumulate their latches throughout playback.

Each final probe observes 4,251 distinct original instructions, all covered by
the 8,190-instruction conservative closure. Its 147 unresolved sites comprise
102 returns, 30 other indirect targets, 13 breaks and two syscalls. This does not
authorize pruning: game cue dispatch, auxiliary loading, voice selection,
multi-tone effects and voice/XA coverage remain open. The dispatcher consumes
type-8 auxiliary data absent from initial EXE cue arrays; direct SDK playback
does not establish game cue identities.

Reproduce with `effects US_EXE BIOS EMI 0 2 VOICE 0 TONE NOTE 0 100 100 1024 44100 WAV`.
Receipts: `out/audio-migration/driver-sfx-final-common.json`,
`driver-sfx-final-battle.json`, their WAV files and bounded `driver-sfx-*-rizin.txt`
disassemblies. The original executable and BIOS remain required; these probes
are not independent fidelity evidence or acceptance of standalone startup.

### Game SFX cue dispatch

The [cue adapter](../../../tools/rust/bof3-audio/src/machine/cues.rs) now stages
source-qualified type-8 records and executes the original dispatcher at
`exe/slus_004_22@0x8015DF18` (full EXE offset `0xC7F18`). The first validated
path supported local-bank records for runtime slot 1;
[seven-slot coverage](#seven-slot-cue-dispatch) extends that path. Cross-bank
records and unverified flags still fail explicitly. It does not implement an alternate
host allocator or reconstruct the guest's layer, volume or retrigger decisions.

Original layout initialization writes the slot-1 VH pointer to `0x80148A1C`
at `0x801618AC` (EXE offset `0xCB8AC`). Its auxiliary destination is
`0x8014871C`; the next slot starts at `0x80148798`, bounding staging to 124
bytes. The type-8 handler at `0x801629F0` selects the loader record's auxiliary
destination. Staging follows that destination but does not execute CD transport.
The adapter verifies archive identity, header ownership, complete four-byte
records, destination bounds and agreement between staged RAM and the selected
table before calling the game. Row indices cannot exceed the supplied payload.

Handler 1 at `0x8015EFAC` (EXE offset `0xC8FAC`) reads these fields:

| Byte | Observed use for supported records |
| --- | --- |
| 0 | Zero retains the local bank; nonzero redirection remains unsupported. |
| 1 | Low seven bits select the program; bit 7 enables the subsequent `SsUtSetDetVVol` call. |
| 2 | High nibble selects the first tone; low nibble enters cue arbitration state. |
| 3 | Low five bits select the first voice; bits 5–6 select zero to three additional layers. Bit 7 is unverified. |

The handler reads tone attributes through original program/tone offset tables,
then the dispatcher keys layers in reverse order. This is not reliably equivalent
to selecting consecutive tones. These original key-on calls were observed:

| Archive/row | Ordered `(voice, bank, program, tone, note, fine, L, R)` calls |
| --- | --- |
| `COMN_SE.EMI` / 0 | `(23,1,0,1,24,0,20,20)`, `(22,1,0,0,24,0,25,25)` |
| `COMN_SE.EMI` / 8 | `(23,1,1,1,24,0,13,66)`, `(22,1,1,0,24,0,67,12)` |
| `BATL_SE.EMI` / 2 | `(18,1,0,6,28,10,0,89)`, `(17,1,0,6,28,10,0,89)`, `(16,1,0,4,28,10,90,0)` |

For each of these calls, the original dispatcher next invokes `SsUtSetDetVVol`
at `0x8016F8F8` with that voice and `(6143,6143)`. The repeated battle tone 6,
fine value inherited from the first tone, and post-key-on volume calls must be
preserved when modeling this path; an independent tone list is insufficient.
The call selector is `0x100 | row`, meaningful only with the currently loaded
archive/table. It is not a globally unique bank or song identity.

`SsUtSetDetVVol` itself writes the cached left/right halfwords at
`0x8018E808 + voice*16` and `0x8018E80A + voice*16`, then sets dirty bits 0–1
at `0x8018E0E8 + voice`; it does not directly write SPU MMIO. The bounded
`driver-cues-volume-rizin.txt` disassembly and observed call arguments identify
this additional state needed by the driver's later flush path.

The [cue probe](../../../tools/rust/bof3-audio/examples/cues.rs) records these
calls, preparation, BIOS services, register targets, external re-entry, closure
and pre-lossy WAV output. Each of the 11 COMN_SE rows and four BATL_SE rows was
run from a fresh prepared bank for 1,024 body frames plus 44,100 release-tail
frames. There were 32 key-on calls in total. Thirteen rows produced nonzero PCM;
COMN_SE row 8 and BATL_SE row 0 stayed silent. All 15 runs succeeded without
uncovered observed original PCs. Each conservative closure has 9,279 instructions
and 151 unresolved sites: 105 returns, 31 other indirect targets, 13 breaks and
two syscalls. Distinct observed original PCs range from 4,624 to 4,792.

[Media checks](../../../tools/rust/bof3-audio/tests/driver/cues.rs) assert ordered
arguments, program selection, volume calls and audible/silent outcomes. They
also reject foreign archives, invalid entries/rows, capacity overflow, unsupported
flags, missing programs, invalid voices/layers and changed staged data without
executing guest instructions. Shared probe observation lives in
[driver/trace.rs](../../../tools/rust/bof3-audio/src/driver/trace.rs).

The shared observer also records effective memory addresses for original
executable instructions. Repeating all 15 runs with this instrumentation leaves
every WAV byte-identical. Each run records 5,204–5,289 instruction/address/width/kind
footprints spanning 3,547–3,566 addresses, including the cue-byte reads, layout's
header-pointer write and handler's program/tone table reads described above.
Merged loads/stores report their aligned word, not exact byte enables. BIOS/kernel
instructions, DMA accesses and host staging remain outside this memory evidence;
it is not a complete data-ownership or initialization proof. These expanded
receipts are under `out/audio-migration/driver-cues-memory/`, with byte-equality
results in `summary.json`.

Reproduce with `cues US_EXE BIOS EMI 0 2 1 ROW@0 1024 44100 WAV`.
`out/audio-migration/driver-cues-corpus/summary.json` records the 15 runs;
per-row JSON/WAV files and `driver-cues-*-rizin.txt` retain detailed evidence.
Fresh-bank results do not establish repeated-cue arbitration, other handlers,
cross-bank state, CD loading, voice/XA behavior, full data closure or independent
fidelity. BIOS/game-executable removal and leaf pruning remain unaccepted.

### Cue history and voice-status polling

The original cue dispatcher consumes a game-owned status snapshot; SDK tick
service alone does not refresh it. `exe/slus_004_22@0x8015D044` (full EXE offset
`0xC7044`) loops over all 24 voices, calls `SpuGetKeyStatus` at `0x801682E0`
with masks `1 << voice`, and stores each result in the word array at
`0x8018E140 + voice*4`. Its original callsite is `0x8014AB68` (EXE offset
`0xB4B68`). This proves a separate poll dependency, not its full gameplay cadence.
The [cue adapter](../../../tools/rust/bof3-audio/src/machine/cues.rs) exposes this
original call explicitly and reads the snapshot without altering it; dispatch
does not silently poll or synthesize activity from PCM.

The [scheduled probe](../../../tools/rust/bof3-audio/examples/cues.rs) now reports
`bof3.audio.driver-cues/v2`. Its EVENTS argument is an ordered comma-separated
list of `ROW@FRAME`, `poll@FRAME` and `off@FRAME`. Equal frame numbers preserve
the stated order; `off` releases voices keyed by preceding scheduled cues.
All 1–64 events must precede the explicit body endpoint. Guest execution can
advance output past a requested frame, so each event reports its actual frame,
before/after game state, original calls, notes and volume calls. The existing
single-cue case (`0@0`) produces the same WAV bytes as the earlier probe.

The observed arbitration path uses:

| State | Address | Observed role |
| --- | --- | --- |
| Initial dispatch gate | `0x8018232A`, signed byte | Zero takes the initial trigger path; a successful initial trigger stores `-1`. |
| Previous first voice | `0x8018B3F0`, signed halfword | A different first voice permits a new trigger. |
| Previous arbitration | `0x8018B3EC`, signed halfword | Compared with the current record's low nibble. |
| Current first voice / arbitration | `0x8018B318` / `0x8018B3E8`, halfwords | Updated by the selected record handler even when the cue is suppressed. |
| Polled voice state | `0x8018E140 + voice*4`, word | A nonzero snapshot suppresses a lower arbitration value on the same first voice. |

At `0x8015E460`, the original comparison is current arbitration less than
previous arbitration. Only that lower-value, same-voice path consults the
snapshot at `0x8015E6E0`; nonzero returns without key-on. Equal values retrigger.
Suppression preserves the previous accepted voice/arbitration, distinct from the
current record fields. These statements cover the observed slot-1 path, not
every entry/control mode of the game sound system.

Two explicit histories exercise this behavior. The unmodified COMN_SE schedule
repeats row 0, changes to row 1 during playback, releases, polls and retriggers.
A separate probe archive changes only the arbitration nibbles of rows 1 and 2
from 10 to 9 and 8. Its input hashes and exact edits are recorded in
`out/audio-migration/driver-history-input.json`; original media remains unchanged.
After row 0 starts voices 22/23, a poll at frame 1,024 records status 2 for both.
The lower-value row 1 is suppressed. Releasing the voices makes their SPU
envelopes reach zero, but the game snapshot remains 2: another row-1 request is
still suppressed until the explicit poll records zero. It then triggers and
updates the accepted arbitration value to 9. Row 2 can subsequently trigger
with value 8 because its first voice is 18 rather than 22. Status describes the
polled voice state, not whether its sample produces audible PCM.

The poll takes 2,839 guest instructions in these cases. A poll scheduled at frame
1,024 makes the following same-frame cue execute at frame 1,031 under the current
reference clock; the probe does not erase this execution cost. The two histories
observe 4,792/4,820 distinct original instructions and 5,384/5,514 memory
footprints. Both conservative closures contain 9,345 instructions, with all
observed original PCs covered and 153 unresolved sites: 107 returns, 31 other
indirect targets, 13 breaks and two syscalls. The poll is now an explicit root
in the [driver inventory](../../../tools/rust/bof3-audio/src/driver/roots.rs).

[History coverage](../../../tools/rust/bof3-audio/tests/driver/history.rs) checks
all 24 original masks, live envelopes versus stale/refreshed snapshots, equal
and lower arbitration, rejection without updating accepted history, and a change
of first voice. It edits only in-memory auxiliary data, never patches guest
activity flags. [Schedule checks](../../../tools/rust/bof3-audio/examples/support/events.rs)
reject malformed, out-of-order and oversized schedules. Fifteen focused driver
checks pass. Receipts are `driver-history-common.json`,
`driver-history-priority.json`, `driver-history-single.json` and bounded
`driver-history-*-rizin.txt` under `out/audio-migration/`.

Reproduce the unmodified history with EVENTS
`0@0,poll@1024,0@1024,1@2048,off@3072,poll@4096,0@4096`, body 8,192 frames
and tail 44,100. Exact main-loop polling cadence, other handlers, cross-bank
arbitration, additional SDK status cases and independent timing/fidelity remain
open. No automatic status refresh, pruning or BIOS-free acceptance is inferred.

### Seven-slot cue dispatch

Local-bank dispatch now selects the original handler with `(slot << 8) | row`
for slots 0–6. This selector remains qualified by the loaded archive, header and
auxiliary table. Original US handler addresses are `0x8015E994`, `0x8015EFAC`,
`0x8015F5C8`, `0x8015FBE4`, `0x80160200`, `0x8016081C` and `0x80160E38`;
full EXE offsets are each address minus `0x80096000`. The local-mode paths use
their own bank constants and VH/auxiliary pointers. Redirection remains outside
the validated path.

The initialized auxiliary buffers start at `0x801486A0 + slot*0x7C`.
Every buffer is bounded to 124 bytes. The last ends at `0x80148A04`, where
original sound initialization writes live control halfwords, followed by another
at `0x80148A06`; this is not a buffer inferred from cue counts. Staging preserves
these controls. The adapter uses initialized loader destinations and bounds
them by the next destination or this final live-state boundary.

Cue attributes and SDK playback selection use distinct lookups. The original
halfword table at `0x801821E0 + program*2` does not necessarily select that
program's packed VAB tone block. For `BENEMY/ENEMY086.EMI`, program 3 has packed
block 3 but its cue attribute offset is zero. Row 6 still calls the original
SDK with `(19,6,3,1,24,67,20,20)` followed by `(18,6,3,0,24,67,60,60)`.
The adapter preserves that lookup and requires attribute reads through the tone
offset table at `0x801821E8` to remain within the loaded VH payload. It no longer
requires the attribute offset to equal the packed program block. An out-of-range
lookup is rejected before guest execution.

[Seven-slot checks](../../../tools/rust/bof3-audio/tests/driver/slots.rs) verify
actual handler entry, SDK bank identity, bounded staging and nonzero PCM using
BGM004, COMN_SE, AREA102, DRG08_00, DRG08_02, DRG08_01 and ENEMY086. The DRG08
checks use row 4: their row-0 tones are silent. A separate check covers ENEMY086
row 6's program alias and rejection of a corrupted lookup. All 17 focused driver
checks pass. Per-slot row-0 probes and the program-3 probe retain JSON/WAV
receipts under `out/audio-migration/driver-slots-probes/` and
`driver-slots-program3.*`; these observed original PCs are all covered by their
conservative closures. These are representative checks, not full PCM coverage.

The [corpus audit](../../../tools/rust/bof3-audio/examples/coverage.rs), invoked
as `coverage US_EXE BIOS CORPUS_ROOT`, prepares a fresh bank for each auxiliary
table and dispatches its rows in file order without output-clock rendering or
status polling. It records archive hashes, entries, original call arguments and
errors. A runtime failure stops that table; a pre-execution rejection permits
later rows to be checked. The command publishes JSON and exits unsuccessfully
when any table, row or archive remains unverified.

The local audit found 904 auxiliary tables and 8,608 records across all seven
slots. All tables prepared successfully; no record uses nonzero redirection or
the unverified high control bit. Dispatch succeeded for 8,575 records. The
remaining 33 were rejected without execution because their requested tone
ranges exceed their programs' declared tone counts. Their actual runtime
behavior remains unresolved; this is an adapter coverage gap, not a finding
that the source media is invalid. The complete report is
`out/audio-migration/driver-slots-coverage-final.json`, with the initial audit in
`driver-slots-coverage.json` and inventory in `driver-slots-inventory.json`.
The corpus gate remains open. Multi-bank histories, cross-bank modes, voice/XA,
complete data ownership, independent fidelity and BIOS-free startup also remain
unaccepted; no leaf pruning is authorized by this coverage.

### Physical tone slots and sample-zero returns

The next bounded audit resolves 32 of the 33 preceding tone-range rejections.
Original `SsUtKeyOnV` at `exe/slus_004_22@0x8016E400` uses the selected physical
32-byte tone slot without comparing its index with the program's declared tone
count. Its setup at `0x801736F0` obtains the packed block from the loaded program
table; the first tone load is at `0x8016E5DC` (full EXE offset `0xD85DC`). The
sample-reference halfword is loaded at `0x8016E5E8`. Zero takes the branch at
`0x8016E648` to the `-1` return path, without key-on. Nonzero references continue
through the original parameter, pitch and voice routines. This explains both
skipped layers and playable slots beyond the declared count.

Examples from fresh-bank execution:

| Archive, header, row | Original ordered `(voice, bank, program, tone)` | SDK results |
| --- | --- | --- |
| MAGIC018, 0, 1 | `(23,1,0,4)`, `(22,1,0,3)` | `-1`, `22`; program declares four tones |
| BPLD015, 8, 4 | `(19,5,1,1)`, `(18,5,1,0)` | `19`, `-1`; program declares one tone, but physical slot 1 selects sample 5 |
| AREA078, 0, 11 | `(17,2,2,7)`, `(16,2,2,6)` | `-1`, `-1`; program declares six tones |

The adapter now bounds requested layers to the sixteen physical slots and keeps
the original attribute-lookup bounds. It checks the SDK's computed tone address
before its first load, requiring a complete aligned slot within the loaded
tone blocks. It then accepts sample zero or an in-bank sample reference. Other
references fail with the sample number, address and loaded sample count. This
runtime guard observes original selection rather than reconstructing a
consecutive layer list. A guard failure can follow earlier guest state changes
or layers; it stops execution before the unsafe tone load and is not an atomic
rollback. Existing identity/record/staging rejections still precede dispatch.

The [limits probe](../../../tools/rust/bof3-audio/examples/limits.rs) compares
fresh executions through direct original dispatch and the guarded adapter for
each rejection in a prior coverage receipt. It checks the archive hash against
that receipt. The direct path is development-only evidence, not a production
fallback. Calls, tone reads, SDK results, PCM hashes and errors are recorded
separately for each path. All 32 supported pairs match exactly, including 24
audible and eight silent outcomes over 1,024 body and 44,100 tail frames. The
comparison remains within the same Rust machine; it is not independent fidelity
evidence. Reproduce with `limits US_EXE BIOS CORPUS_ROOT COVERAGE_JSON`, using
`driver-slots-coverage-final.json` as the prior receipt.

The remaining case is `BPLCHAR/DRG04_00.EMI`, header 0, row 5. It selects sample
3 from a bank declaring only two samples. Direct diagnostic dispatch returns
success for voice 18, but playback reaches `0x5EE10`, exactly the end of the
15,520-byte body uploaded at `0x5B170`, and fails on ADPCM predictor 5. The
adapter rejects the sample reference before that tone is read. Required loaded
bank/state context and the intended playback behavior remain unresolved; the
decoder has not been weakened to consume unrelated RAM.

The updated corpus audit prepares all 904 tables and visits all 8,608 rows:
8,607 dispatch successfully and only this missing-context case is rejected.
No rows are skipped. [Focused checks](../../../tools/rust/bof3-audio/tests/driver/limits.rs)
cover exact call arguments and return values, audible and silent outcomes,
corrupt sample references, out-of-bank SDK tone pointers and the remaining
source cue. Twenty focused driver checks pass. Receipts are
`out/audio-migration/driver-range-coverage.json`, `driver-range-comparison.json`,
`driver-range-limits.json` and `driver-range-{keyon,setup}-rizin.txt`.
Standalone extraction must retain referenced physical tone slots even when
declared counts omit them. Full corpus playback, shared-bank context, voice/XA,
independent fidelity and pruning acceptance remain open.

The remaining sample's address is now checked against the original loader,
not inferred solely from the decoder failure. At `exe/slus_004_22@0x80173FD8`
(full EXE offset `0xDDFD8`), the loop accumulates size-table entries through the
declared sample count and writes the resulting address pairs into program-table
offsets 12/14. DRG04_00's first four size entries are `(0,1896,44,0)` in eight-byte
units. Its two allocations total 15,520 bytes. The live address slot that sample
3 selects, at `VH + 0x20 + 16 + 12`, contains `0x5EE10 / 8`: the terminal address,
not an unreported allocation. The focused missing-context test now asserts this
live value and the zero third size entry. The 37-bank dragon inventory in
`out/audio-migration/driver-context-dragon-inventory.json` retains source hashes,
entry extents, size words and declared tone references. Only DRG04_00 exists for
that prefix locally; related dragon banks do not prove a replacement allocation.
See `driver-context-loader-rizin.txt` and `driver-context-test.log` for the new
bounded evidence. Whether the game selects this row, or supplies additional
state when doing so, remains unproven; it cannot yet be pruned or substituted.


### XA driver closure with supplied kernel inputs

The [XA runtime fixture](../../../tools/rust/bof3-audio/tests/xa_loop_runtime.rs)
now uses a [read-only observer](../../../tools/rust/bof3-audio/src/driver/trace.rs)
and [transport evidence collector](../../../tools/rust/bof3-audio/tests/support/transport.rs)
to audit original `exe/slus_004_22` instructions during callback setup and the
first S_XA (`0x0000`), MAGIC (`0x1000`) and VOICE (`0x2000`) cues. The executable
SHA-256 is `0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a`.
Each report includes runtime/file offsets, cue identity, consumed 2352-byte raw
sector range and its SHA-256, register-dispatch targets, kernel re-entry edges
and CPU memory footprints. Guest instructions must match original image bytes.
Explicit host calls start separate trace intervals rather than creating inferred
kernel re-entry edges.

| First cue | Original instructions observed | Conservative closure | Raw sectors at one / four sectors per tick |
| --- | ---: | ---: | ---: |
| S_XA `0x0000` | 1,548 | 2,226 | 864 / 880 |
| MAGIC `0x1000` | 1,517 | 2,193 | 224 / 240 |
| VOICE `0x2000` | 1,549 | 2,226 | 992 / 1,008 |

Each of the six runs reaches reading, position-threshold detection, fade-out,
Pause and idle through the original selector, scheduler, SDK IRQ wrapper and
game callback. The union has 1,549 observed instructions, all accounted for by
a 2,226-instruction closure. Its 62 unresolved sites comprise 42 register
returns, 19 other indirect transfers and one syscall. Observing particular
targets does not prove that these are their only possible targets. MAGIC's first
cue contains no ordinary data sectors; S_XA and VOICE exercise that delivery path.

Callback setup observes A0:13, B0:19, B0:5B and C0:0A, then stops at unsupported
A0:72 after 4,542 original instructions. The final service boundary is observed,
not completed. During scheduling, B0:17 is handled by the supplied fixture
kernel: S_XA makes 356 / 203 calls, MAGIC 66 / 25, and VOICE 963 / 792 under the
two schedules. These counts describe the explicit schedules, not hardware time.

The fixture supplies kernel RAM, GPU status, ready CD host/drive state, prior
SDK completion, CD responses and dry SPU context. It does not boot BIOS.
Memory footprints exclude synthetic kernel/host/DMA accesses and exact merged
byte enables. Consequently these runs establish neither complete initialization
and data ownership nor independent playback fidelity, full cue/history coverage,
BIOS independence or dead-code proof. No instruction is pruned and the report's
data-closure/pruning flags remain false.

Reproduce with the project-scoped mise toolchain and original media:
`cargo test --locked --offline --test xa_loop_runtime original_xa_stream_families_have_bounded_instruction_closures -- --ignored --nocapture`,
setting `BOF3_AUDIO_EXE` and `BOF3_AUDIO_TRACK`. JSON lines use
`bof3.audio.driver-transport/v1`; local receipts are
`out/audio-migration/driver-transport-{0000,1000,2000}-{1,4}.json`.
Three synthetic observer tests validate nonmutation, changed-instruction
rejection and host-call boundary handling; all 23 driver and five XA runtime
checks pass. This is A6.01 evidence, not standalone-driver acceptance.


### XA callback initialization and original ROM dispatch

The [XA fixture machinery](../../../tools/rust/bof3-audio/tests/support/transport/machine.rs)
now offers an explicit original-ROM comparison mode. It executes the verified US
BIOS to the existing shell handoff, loads the verified US executable, and calls
`exe/slus_004_22@0x80175534` (full EXE offset `0xDF534`) with mode 2. Original
instructions select `0x80176DAC`, clear four SDK callback slots, call ResetCallback,
and register `0x80177264` on IRQ 2. The dispatcher returns 1. No fixture kernel
tables are written and no BIOS service is HLE-dispatched in this variant.
The separate supplied-kernel fixture remains a comparison input, not production
fallback behavior.

Original initialization proceeds through the previously unsupported A0:72
boundary, including A0:A3, five B0:09 calls and two C0:03 calls. All five
CloseEvent arguments are zero in this shell-handoff state; the trace therefore
does not establish removal of live CD event handles. The dequeue calls name
priority 0 entries `0xA00091D0` and `0xA00091E0`. Reports preserve arguments and
return PCs rather than interpreting call count alone as state mutation.

The first S_XA, MAGIC and VOICE cues run with one and four sectors per game tick.
All six cases complete real COP0 exception entry, original BIOS/SDK save/restore,
callback execution and interrupt return. Assertions verify restored registers
(except the established kernel scratch register r26), HI/LO, PC and status.
Command sequences, scheduler states, threshold positions, media counts and
pre-encoding dry PCM SHA-256 match the supplied-kernel runs under identical
explicit schedules. This compares two kernel paths on the same Rust CPU/SPU;
it is not an independent emulator or hardware fidelity reference.

| First cue | Original game instructions observed | Conservative closure | Low-RAM instructions at one / four sectors per tick |
| --- | ---: | ---: | ---: |
| S_XA `0x0000` | 1,595 | 2,457 | 278,607 / 171,167 |
| MAGIC `0x1000` | 1,564 | 2,424 | 47,817 / 18,625 |
| VOICE `0x2000` | 1,596 | 2,457 | 818,316 / 699,182 |

Each observed interval also executes 89 ROM instructions; BIOS boot is counted
separately in the boot receipt. The union adds 47 observed game instructions and
231 conservative candidates to the supplied-kernel audit. Its 66 unresolved
sites are 46 register returns, 19 other indirect transfers and one syscall.
Every observed original game PC is retained. The root inventory now includes
the CD reset dispatcher, including its unverified full-reset branch.

Mode 2 initializes callbacks, not the whole drive. A separate diagnostic of
mode 1 stops at `exe/slus_004_22@0x80174720` in VSync when it reads GPUSTAT
`0x1F801814`; that device read remains unsupported in the production machine.
The comparison fixture explicitly supplies GPU status, ready host/drive state,
prior SDK completion, response boundaries, sector phase and dry SPU context.
It neither proves full device initialization nor eliminates BIOS dependencies.
Memory footprints still exclude kernel/host/DMA data accesses. No RAM snapshot
is shipped, no code is pruned and A6.01 remains incomplete.

Reproduce from the repository root with `BOF3_AUDIO_EXE`, `BOF3_AUDIO_BIOS` and
`BOF3_AUDIO_TRACK` set to absolute local media paths:

```sh
mise -C tools/rust/bof3-audio exec -- cargo test --locked --offline   --test xa_loop_runtime original_xa_scheduler_uses_booted_rom_kernel_state   -- --ignored --nocapture
```

The report schema is `bof3.audio.driver-transport/v2`, identifying the kernel
model, boot hashes, service arguments, outcomes and consumed raw-sector hashes.
Receipts are `out/audio-migration/driver-startup-original_us_rom-*-*.json`,
`driver-startup-supplied_kernel-*-*.json`, `driver-startup-comparison.log`, and
bounded `driver-startup-*-rizin.txt` listings. All six original XA runtime tests
pass, including the ROM/supplied-kernel comparison.


### Original CD reset and host reset state

The [Rust drive](../../../tools/rust/bof3-audio/src/machine/cd_drive.rs) now handles
command `0x0A`, named `CdlReset` in the pinned PCSX-Redux source. On accepted
reset it stops reading, unmutes, sets mode `0x20`, releases XA selection and
acknowledges with INT3/status `0x02`. A separate caller-supplied completion
boundary produces INT2/status `0x02` and leaves the drive paused. Pending
mechanical transitions still reject overlapping reset commands. Bad parameter
counts return INT5 without changing drive state.

Reset preserves the filter, pending Setloc target, current position and latest
transfer header. The [host integration](../../../tools/rust/bof3-audio/src/machine/interconnect/cd.rs)
also preserves the data cursor and already queued XA output; Init does not imply
an immediate audio-buffer flush. Subsequent read/seek operations retain their
separately documented behavior. These semantics follow the inspected
[CD implementation](../../../third_party/pcsx-redux/src/core/cdrom.cc), including
`StopReading`, the `CdlReset` command/IRQ cases, and the SPU stream submission
path in [xa.cc](../../../third_party/pcsx-redux/src/spu/xa.cc), at submodule commit
`28438546c781fbe372a06399c82bed43ca2c6f4d`. Reset latency and hardware fidelity are
not inferred from those source paths.

The [host reset constructor](../../../tools/rust/bof3-audio/src/machine/cd_host.rs)
explicitly selects the emulator-reference reset state: interrupt mask `0x1F`,
active volume matrix `(128,0,128,0)`, empty parameter/response queues and no pending
interrupt or command. It seeds neither media nor an SDK completion response.
The existing empty-interface constructor remains an explicit test/input context;
there is no automatic fallback between the two. Reconfiguration rejects without
replacing an active host. With mask zero, the original initializer reaches
`exe/slus_004_22@0x80176A1C` and exhausts the bounded wait after Getstat; that
negative diagnostic is retained in `driver-reset-empty-host.log`.

The [full-reset runtime case](../../../tools/rust/bof3-audio/tests/xa_loop_runtime.rs)
boots the verified US BIOS, loads the verified game image and calls the original
`0x80175534` dispatcher with mode 1. It executes Getstat `0x01`, Init `0x0A` and
Demute `0x0C` through the original SDK and ROM interrupt paths, returns 1, and
then plays the first S_XA, MAGIC and VOICE cues at both explicit sector schedules.
The fixture no longer injects a prior completion response or writes an IRQ mask.
All variants start the supplied spinning/authenticated drive at LBA 0; original
location commands establish each cue position.

All six full-reset runs match callback-only runs in subsequent command sequences,
scheduler states, stop positions, sector counts and pre-encoding dry PCM hashes.
The three initialization commands are asserted separately and remain in the
reports. Full reset adds three completed interrupts per scenario relative to the
prior-completion fixture. The observation counts are:

| First cue | Original game instructions observed | Conservative closure |
| --- | ---: | ---: |
| S_XA `0x0000` | 1,748 | 2,457 |
| MAGIC `0x1000` | 1,717 | 2,424 |
| VOICE `0x2000` | 1,749 | 2,457 |

The union retains all 1,749 observed game PCs and all 66 unresolved closure sites.
Reports use `bof3.audio.driver-transport/v2` with
`initialization: device_reset`, boot identities, service arguments, complete
command outcomes and consumed-sector hashes. The root inventory now records
modes 1 and 2 as observed with explicit device responses and timing inputs.

This clears the supplied SDK-readiness assumption for these cases, not the whole
startup gate. GPUSTAT is still an explicit fixture input and unsupported in the
production machine; disc authentication/spinning state, dry SPU context,
mechanical response boundaries and sector/tick timing remain supplied. Complete
kernel/data ownership, hardware timing, independent reference audio and the
BIOS-free shipped driver remain unaccepted. No code is pruned.

Reproduce with absolute `BOF3_AUDIO_EXE`, `BOF3_AUDIO_BIOS` and
`BOF3_AUDIO_TRACK` paths:

```sh
mise -C tools/rust/bof3-audio exec -- cargo test --locked --offline --test xa_loop_runtime original_cd_reset_establishes_readiness_before_xa_playback -- --ignored --nocapture
```

Three new checks in [drive](../../../tools/rust/bof3-audio/tests/cd_drive.rs),
[host](../../../tools/rust/bof3-audio/tests/cd_host.rs) and
[lifecycle](../../../tools/rust/bof3-audio/tests/cd_lifecycle.rs) cover reset
boundaries, retained state/buffers and host reset/IRQ behavior. Runtime receipts
are `out/audio-migration/driver-reset-{0000,1000,2000}-{1,4}.json` and
`driver-reset-final-corpus.log`; earlier failed diagnostics remain separate.


### Negative VSync peripheral input dependencies

Original `exe/slus_004_22@0x80174700` (full EXE offset `0xDE700`, SDK `VSync`)
reads GPUSTAT at `0x80174720` and Timer-1 at `0x80174724`. The original pointer
words resolve to `0x1F801814` and `0x1F801110`. For a negative argument, the
branch at `0x80174738` falls through its delay slot, loads the software frame
counter from `0x801856C4`, and jumps to the epilogue at `0x80174830`. It restores
`s0` and `s1`; the GPU value and computed timer delta are not used on that path.
The software counter remains a live input. Nonnegative waiting/display paths
are outside this conclusion.

The [bounded input check](../../../tools/rust/bof3-audio/tests/driver/inputs.rs)
executes the original US instructions with arguments `0x80000000`, `0xFFFF0000`
and `0xFFFFFFFF`, GPU inputs `0`, `0xFFFFFFFF`, `0x14802000`, `0xA5A55A5A`, and
timer inputs `0`, `0xFFFF`, `0xFFFFFFFF`. It also varies the software counter
through `0x12345678`, zero and `0xFFFFFFFF`. A fixture-only candidate replaces
just the two peripheral loads with NOPs and rejects any attempted access to
either unavailable peripheral. Across 54 calls, the original/candidate results
match in all general registers, HI/LO, the 26-instruction path, RAM write trace,
and whole guest RAM after normalizing only the two intentionally changed code
words. The returned counter varies with its input. No production image or
assembly source is changed by this test.

The [XA transport observer](../../../tools/rust/bof3-audio/tests/support/transport.rs)
also records each VSync argument and return PC, while the
[bus fixture](../../../tools/rust/bof3-audio/tests/support/transport/machine.rs)
counts actual GPU reads. In six full-reset S_XA/MAGIC/VOICE scenarios, every
call has argument -1 and every GPU read is explained by those calls. Return PCs
are `0x8017621C` (file `0xE021C`), `0x8017625C` (`0xE025C`) and `0x80176904`
(`0xE0904`). Observed original-code Timer-1 reads occur only at `0x80174724`.

| First cue | GPU reads at one / four sectors per tick |
| --- | ---: |
| S_XA `0x0000` | 470 / 148 |
| MAGIC `0x1000` | 150 / 68 |
| VOICE `0x2000` | 534 / 164 |

Repeating those complete XA runs with GPUSTAT all ones instead of zero preserves
command sequences, state transitions, stop positions, media counts and
pre-encoding dry PCM hashes. Original executable instructions remain unchanged
in these runtime comparisons. Reports retain `gpu_input` and `vsync_calls` in
`bof3.audio.driver-transport/v2`; the watch rejects an unproven nonnegative
VSync context rather than extending this finding silently.

This is a bounded value-dependency result. The isolated candidate excludes
interrupts, MMIO side effects, faults and hardware bus timing; equal instruction
counts do not prove equal physical timing. Runtime coverage is limited to the
six stated scenarios on the Rust machine. It does not prove all audio callers
are negative, authorize deleting positive VSync paths or eliminate the live
software counter. GPUSTAT remains unsupported in the production machine until
the owned driver and its complete narrowing gates are accepted.

Reproduce the isolated check with `BOF3_AUDIO_EXE` set to the absolute US image:

```sh
mise -C tools/rust/bof3-audio exec -- cargo test --locked --offline --test driver inputs -- --ignored --nocapture
```

For the complete comparison, also set `BOF3_AUDIO_BIOS` and `BOF3_AUDIO_TRACK`,
then run `--test xa_loop_runtime original_audio_vsync_calls_do_not_depend_on_gpu_status_bits`
with `-- --ignored --nocapture`. Receipts are
`out/audio-migration/driver-vsync-{0000,1000,2000}-{1,4}.json`,
`driver-vsync-rizin.txt`, `driver-vsync-peripherals.log` and
`driver-vsync-media-final.log`. A6.01 and pruning acceptance remain open.

### Negative VSync candidate through original interrupts

The [XA replay check](../../../tools/rust/bof3-audio/tests/xa_loop_runtime.rs)
now compares the same two fixture-only NOP substitutions through full original
CD reset, booted-ROM exception dispatch and the six first-cue XA scenarios.
The unmodified baseline supplies GPUSTAT `0x14802000`; the candidate rejects
all attempted GPUSTAT and Timer-1 counter reads. It also rejects nonnegative
VSync arguments. Neither the source executable nor production machine code is
changed. The substitutions remain at `exe/slus_004_22@0x80174720` and
`0x80174724`, full EXE offsets `0xDE720` and `0xDE724`.

Each pair preserves the exact executed PC sequence, all 32 registers plus
HI/LO, status and PC at every guest-call return, the final 2 MiB RAM image after
normalizing only those two instruction words, and IRQ entry/return counts.
Existing interrupt checks require interrupted GPRs other than `k0`/`k1`, HI/LO,
status and resume PC to survive original exception return. Commands, scheduler
states, raw/selected/data-sector counts, stop locations and pre-encoding dry PCM
hashes also match. The baseline observer still verifies original instruction
bytes and conservative closure coverage. Modified executions use their own
comparison receipts; they are not mislabeled as original-byte traces.

| First cue | Sectors per tick | Instructions compared | Completed IRQs | Negative VSync calls |
| --- | ---: | ---: | ---: | ---: |
| S_XA | 1 | 609,610 | 359 | 470 |
| S_XA | 4 | 331,498 | 206 | 148 |
| MAGIC | 1 | 153,891 | 69 | 150 |
| MAGIC | 4 | 80,419 | 28 | 68 |
| VOICE | 1 | 1,414,455 | 966 | 534 |
| VOICE | 4 | 1,101,135 | 795 | 164 |

The six comparisons cover 3,691,008 instructions, 2,423 completed IRQs and
1,534 negative VSync calls, with zero GPUSTAT reads in the candidate. These
are explicit functional response/sector schedules in the Rust machine. They
do not establish all interrupt phases, hardware bus timing, positive VSync
behavior, complete caller/data closure, independent fidelity or BIOS-free
startup. Retaining the same instruction count is not physical-timing evidence.
The live software counter remains required, and production pruning is not
authorized by this result.

Receipts are `out/audio-migration/driver-candidate-comparison.json`, the six
`driver-candidate-original-{0000,1000,2000}-{1,4}.json` baseline reports and
`driver-candidate-suite.log`. The comparison receipt includes original EXE/BIOS
hashes and both substituted words. Run `--test xa_loop_runtime
negative_vsync_candidate_preserves_full_reset_and_interrupt_execution` with
`-- --ignored --nocapture` and the same absolute EXE/BIOS/track variables as
the preceding checks. All nine XA runtime cases pass; the default suite passes
362 checks with 144 media cases ignored, and strict Clippy and Rust 1.88 checks
pass. In this session the scoped mise config was untrusted, so validation used
the already-installed pinned Rust 1.98.1 directly without changing trust settings.

### Retired XA instruction and memory dependencies

The development observer in `src/driver/retirement.rs` now records the actual
fetched instruction returned by successful CPU steps, including BIOS ROM and
kernel RAM. Pre-step registers retain effective addresses across delayed-load
commits. Each memory record distinguishes the effective address, bus address and
width, consumed/stored byte lanes and cache-isolation state. Merged loads still
read a full bus word: their consumed lanes do not remove device read side effects.
Original executable words are checked against the pinned image before recording.

The six full-reset S_XA/MAGIC/VOICE scenarios, at one and four sectors per tick,
preserve the existing command, scheduler, stop and dry PCM comparisons. The
`bof3.audio.driver-transport/v3` report adds a `retirement` section and separate
`syscall_traps`. The fixture asserts complete accounting of pre-step observations
on original-ROM paths: 3,689,304 retired instructions plus 1,704 syscall traps
equal all 3,691,008 observed instruction boundaries.

| Instruction source | Retired executions | Unique instructions | Memory accesses | Unique access records |
| --- | ---: | ---: | ---: | ---: |
| Original executable | 1,604,176 | 1,748 | 490,739 | 1,957 |
| BIOS ROM | 4,326 | 328 | 678 | 111 |
| Other RAM, including kernel | 2,080,802 | 681 | 661,082 | 369 |

Unique instructions include address and word; access records also include the
effective/bus addresses, lanes and isolation state. Four Rust checks cover load
delay, ordinary/merged byte lanes, ROM/kernel recording and rejected mismatched
observations. The older `memory` report field remains the distinct pre-step
EXE-only observation, including attempted instructions; it must not be mistaken
for the wider retired-access evidence.

Receipts are `out/audio-migration/driver-retirement.jsonl`,
`driver-retirement-summary.json`, and `driver-retirement-complete-runtime.log`.
Reproduce with the full-reset command above. This closes the observer's ROM/kernel
visibility gap for these scenarios. It does not identify every data producer:
host writes, DMA and HLE dispatch remain outside retired CPU access records.
Full cue histories, music/SFX coverage of this observer, initialization ownership,
independent timing/audio evidence and standalone startup remain open. No code or
data is pruned, no BIOS state is shipped, and A6.01/A6.02 are not accepted.
