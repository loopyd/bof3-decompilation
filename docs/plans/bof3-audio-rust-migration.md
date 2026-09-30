<!-- bof3.plan/v1 -->
# BOF3 audio migration to Rust

## Goal and authority

Implement the user-approved migration of the host audio tool to a Rust library
and `bof3-audio` executable under `tools/rust/bof3-audio`. Support music, SFX,
and vocals from EMI archives and XA streams. Complete the game audio execution
path; translating the approximate C renderer or incomplete machine VM is not
completion. Preserve recovered game C in `src/bof3/audio`. The final tool must run without
an external game executable or BIOS: isolate and lift only the used audio-driver
code, ship its assembly source and build a standalone guest driver executable.
Organize the Rust host into cohesive noun folders with single-noun filenames.

This plan records the original five-phase plan approved on 2026-09-23 and the
2026-09-24 additions for a standalone audio driver and Rust source modularization. It owns this migration only; the separate autonomous decompilation
plan and unrelated dirty work remain outside its scope. Execute incomplete
steps in dependency order, refresh evidence before each phase, and record
validation and unresolved blockers here. Plan parsing establishes structure,
not implementation acceptance. No implementation phase is accepted yet.

## Evidence baseline

Initial inspection on 2026-09-23 established:

- The host implementation is `tools/c/psx-audio`, including EMI/VAB/SEP, ADPCM,
  XA, SF2, rendering, machine and SPU code. Its CMake tests are `psf_test`,
  `psx_machine_test`, `spu_device_test`, and `xa_test`.
- `tools/rust/emi-ex` exposes `src/lib.rs`; its crate is `emi-ex-v2` (MIT),
  with Serde/JSON dependencies and archive/pack/CLI tests. Lossless audio use
  still needs verification and extension.
- `tools/python/harness/commands/audio.py` builds the C tool through CMake
  and packages its source. `tools/python/tests/toolchain/test_audio_surface.py`
  checks building, failure diagnostics, packaging, and retired artifacts.
- `bin/harness plans list` passed before this plan existed and listed only
  `autonomous-bof3-decompilation.md`; that existing plan is already modified.
- Starting audio dirt includes modified `audit_all_bgm.py`, `commands.c`,
  `export.c`, and `main.c`; staged/unstaged changes to `test_audio_surface.py`;
  and untracked `commands/audio.py`. Preserve and inspect these changes before
  editing their owners. The wider tree also contains extensive unrelated work.
- C baseline configure/build and all four CTest tests passed; the build emitted
  existing path/message truncation warnings in `main.c` and `tui.c`. All five
  Python audio-surface tests passed (13.86 s), including source-package build.
- Per-archive `bgm-audit` succeeded for all 81 BGM archives; full results are in
  `out/audio-migration/c-bgm-audit.json`. BGM000 reports four sequences, layered
  notes, bends and ignored controllers, reinforcing the need for semantic
  translation validation. Directory-form `bgm-audit` failed with
  `missing-or-invalid-EMI/VH/VB/SEP` despite advertised directory support; the
  per-file audit is the working baseline, not a fix to the retained C tool.
- `out/audio-migration/media-baseline.json` records SHA-256 and byte sizes for
  880 EMI archives, four STR files and the US executable in `out/extracted`.
  `SLUS_004.22` is 1,445,888 bytes with SHA-256
  `0af39fb1ffcf25e4bdf2730173f397b5b5f6c44989114fe9b59b92ab7c0eb21a`.
  This hash identifies an input; runtime-profile verification is still pending.
  STR inputs are `BIN/BMAG_XA/MAGIC00.STR`, `BIN/SCE_XA/S_XA00.STR`,
  `BIN/SCE_XA/VOICE.STR`, and `LOGO/CAPCOM30.STR`.
- `out/audio-migration/starting-status.txt` records the dirty worktree at the
  planning checkpoint. Generated evidence is local, not a distributable fixture.
- The initial EMI Rust cache failure was resolved by copying the already-cached
  locked packages/index entries to `out/audio-migration/cargo-home`, without
  changing global configuration. All 14 pre-change EMI Rust tests passed offline.
- Plan parsing and 46 plan tests passed. The shared gate passed Ruff, docs drift,
  text Rust/Python tests and corpus checks, then failed at existing symbol naming
  debt in battle/UI/world sources and maps. The separately invoked source validator
  passed (`valid=1480 invalid=0`); its log is
  `out/audio-migration/validate-sources.log`.
  These unrelated sources/maps have not been edited by this migration.
- Reference inventory found `out/extracted/track02.wav` (disc track), but no
  independently established sound-runtime trace or reference-render corpus in
  the searched local outputs. The audio spec explicitly records incomplete
  bootstrap/timing. Reference evidence still must be established for A4.

## Implementation evidence (2026-09-23)

- `tools/rust/emi-ex/src/image.rs` adds immutable preservation snapshots and
  validated entry replacement within existing sector allocations. Shared parsing
  exposes the opaque TOC padding word; old EMI CLI/manifest behavior remains
  covered by existing tests. Unchanged entries retain even stale cached words.
  Relocation is explicitly rejected for now; edited packing is not complete.
- `tools/rust/bof3-audio` now contains library/CLI foundations, EMI/XA snapshots,
  container verification and optional whole-file comparison. Other operations
  explicitly fail as unimplemented. Verification reports keep identity,
  translation, encoding and rendering evidence unverified.
- Cargo check/format passed. The EMI crate passes 20 tests; the audio crate
  now passes 47 tests including explicitly invoked local-corpus and original-US
  execution/profile tests. The current full result log is
  `out/audio-migration/rust-tests.log`.
  All 880 EMI archives and four XA streams preserve every byte through container
  APIs. This does not yet satisfy the XML extraction/repacking acceptance gate.
- `machine/executable.rs` loads checked PS-X EXE payloads into 2 MiB RAM with
  explicit physical/KSEG0/KSEG1 aliases, validated header ranges and immutable
  metadata. Tests cover alias bounds, truncation, overflow, preservation of
  trailing bytes, and exact US payload placement. This is a loader, not accepted
  R3000/SPU runtime execution; arbitrary profiles remain unsupported by rendering.
- `machine/cpu.rs` and `machine/bus.rs` now execute MIPS I integer arithmetic,
  multiply/divide, branches, delayed loads, and aligned/unaligned memory access.
  The bus preserves transaction width/full store-register data and byte enables.
  Fifteen CPU tests cover delay slots, load cancellation/merging, conditional
  links, arithmetic edge cases, precise faults, bounded execution and MMIO store
  semantics. The core counts instructions, not cycles; COP0/BIOS/GTE, exception
  dispatch, cache/bus timing and device integration are still unimplemented and
  fail explicitly. Nested branches in delay slots are explicitly unsupported.
  Architectural references are the
  [PSX CPU specification](https://psx-spx.consoledev.net/cpuspecifications/);
  consecutive-load cancellation was cross-checked against an independent
  emulator's documented implementation. No emulator source was copied.
- The original `exe/slus_004_22@0x80171B20` pitch routine executes in this Rust
  core for 4,250 note/center/shift combinations (at most 60 instructions/call),
  matching the separately expressed integer formula derived from its assembly.
  Both executable and table hashes are checked before execution. This validates
  bounded functional execution, not hardware timing or independent reference PCM.
- The original cue dispatcher at `exe/slus_004_22@0x80161C20` executes for all
  255 numeric inputs and reaches the real first callee at `0x8015D300` with
  the expected bank/sequence arguments. The `0xFF` sentinel path returns with
  its stack and saved registers restored. Tests stop at that first callee;
  playback downstream has not been substituted or claimed as passing.
- `machine/profile.rs` now recognizes only the exact full-file US EXE SHA-256;
  unknown, modified and extended executables are rejected. Rust SHA-256 passes
  published vectors and 13 independent Python-hashlib vectors covering padding
  boundaries. `verify --executable PATH` reports profile identity separately
  from `bootstrap: not_implemented` and rendering evidence. The JSON smoke result
  `out/audio-migration/verified-us-profile.json` was independently checked against
  hashlib. Profile recognition does not yet close A1.03's full evidence work.
- Rust formatting, checks and strict Clippy for all audio targets pass after
  replacing obsolete slice-chunk idioms. The crate declares Rust 1.88 as its API
  floor; initial validation used Rust 1.98. After adding RustySynth, locked
  offline checks also passed with the installed Rust 1.88.0 toolchain.
- Original startup/table evidence now lives in the
  [audio specification](../specs/formats/audio.md#verified-us-sound-startup-and-sequence-storage).
  `exe/slus_004_22@0x8015CD00` is the game sound initializer. Its table setup at
  `0x8016D7EC` executes in Rust with the original arguments `(0x80148A50, 2, 4)`;
  both handle pointers and all eight independent sequence records validate.
- The interconnect now models scratchpad, edge-latched I_STAT/I_MASK, DMA global
  control/IRQ state, and counters driven by explicit CPU/dotclock/blanking inputs.
  Stateful BIOS services implement setjmp/longjmp, interrupt-hook registration,
  and pad/counter auto-ack settings. Tests cover saved registers, rejected calls,
  IRQ edges/acknowledgement, DMA flags, timer clock selection, synchronization,
  one-shot/toggle behavior and reset/read-clear flags. DMA channel transfers,
  cycle integration and electrical pulse widths remain unverified; target-zero
  timer-reset timing explicitly fails until its behavior is established.
- `examples/runtime_probe.rs` follows original startup for 4,550 integer
  instructions, then stops at unsupported BIOS CD-ROM removal `A0:72`.
  `out/audio-migration/bootstrap-probe.json` records calls and terminal context;
  the probe bounds CPU and modeled-kernel transitions. This is evidence of the
  next missing service, not accepted bootstrap, scheduler or playback behavior.
- `THIRD_PARTY_LICENSES.txt` records all 12 registry packages in the current
  locked graph. It reuses EMI/Serde/JSON; no native bindings occur in that graph.
  Public source/manifest/license research is under
  `out/audio-migration/dependency-research`. RustySynth 1.3.6 (MIT, no dependencies)
  was specifically approved by the user and is now pinned as `=1.3.6` in the
  manifest and lockfile. The published crate SHA-256 is
  `e3ca93af923df5fc03beddbf464242620fd24daa9e10f9ecd56eb9571eb7ba38`.
  Its manifest declares no dependencies or build script; the package contains
  no native source files. The archive omits its license text, so the inventory
  includes the MIT license from its recorded source commit
  `8cc11fc0b10422adb54107757f100d3d6ae0ef96`, preserving both MeltySynth and
  RustySynth copyright notices. Local audit evidence is in
  `out/audio-migration/rustysynth-dependency-review.json`. Locked offline
  checks pass on Rust 1.98.1 and 1.88.0; strict Clippy passes on 1.98.1.
  The full suite after dependency integration passed all 110 tests with no
  failures or ignored cases, using the original executable, archives, disc
  and independent FFmpeg consumer (`out/audio-migration/rustysynth-tests.log`).
  Formatting, license-inventory coverage, plan parsing and scoped references
  also pass.
  This accepts dependency integration, not PC rendering or MIDI/SF2 fidelity.
  OxideAV Vorbis 0.0.12 and MP3 0.1.3 (MIT) are candidates, not accepted codecs;
  source and transitive-dependency review plus independent conformance remain.
  Shine-RS 0.1.4 was inspected (LGPL-2.0) but has not been selected or installed.
- `out/audio-migration/us-profile-evidence.json` verifies that the full US EXE's
  bytes after the 0x800-byte header equal the normalized payload (SHA-256
  `677754d0d22c88151a5022cd98b8e89af1b0882177d9850faf62676eb7089eff`).
  Load address is `0x80096800`, PC `0x8014AA0C`, SP `0x801FFFF0`.
  The 386-byte pitch table at `exe/slus_004_22@0x8018445C`, full-file offset
  `0xEE45C`, matches documented SHA-256
  `293278b74970e97b814ab68b63edf21d4dcdc6630bd5394fce250aec6cd955b2`.
- Fresh target-qualified Rizin inspection confirms cue wrapper `0x80161C20`
  reads bank/sequence bytes from `0x80181EBA + 4 * (cue & 255)` and the following
  byte before calling `0x8015D300`/`0x8015D49C`. The SDK-map label `SsInit` at
  `0x8017E0B4` is not accepted as a sound bootstrap: its first call targets
  `0x8017E104`, a BIOS B0/0x4C memory-card routine. Resolve actual init behavior
  from original instructions/callers; no map edits have been made.

## Agreed contracts

- **Unchanged round trips:** reproduce original archive/stream bytes exactly.
  XML preserves otherwise unrecoverable encoding, padding, ordering, and opaque
  data, including unrelated entries and trailing bytes.
- **Edited round trips:** preserve identities and supported musical semantics;
  encode changed samples to PSX/XA ADPCM, quantify encoding loss, and explicitly
  reject unsupported transformations. Reuse preservation bytes only when parsed
  component content is unchanged.
- **PSX rendering:** execute the narrowed, lifted audio driver through the Rust
  machine/SPU, preserving original supported audio behavior. The shipped driver
  and its assembly build target replace production game-executable and BIOS
  dependencies. Original executable/BIOS runs remain development evidence only,
  never a production fallback. Unsupported execution fails explicitly; the
  approximate C renderer is not a fidelity oracle.
- **PC rendering:** standard SoundFont synthesis entirely in Rust; measure and
  document differences from PSX playback. Identical PCM is not required.
- **MIDI:** prioritize faithful playback with the accompanying BOF3 SoundFont;
  stock GM devices may substitute timbres.
- **Dependencies:** use entirely Rust implementations; audit licenses and
  transitive dependencies before selection. Implement missing codecs when no
  suitable Rust implementation exists. No C bindings or external rendering
  executables as production fallbacks. Follow repository authorization rules
  for specific dependency additions/installations.
- **Current implementation only:** no fallback or legacy export/packing path.
  Older XML policies require regeneration. Empty VAB programs must be editable
  into populated programs, with explicit supported semantics and capacity checks.
- **Reachability and pruning:** retain the complete used closure for SFX, voice
  and music, including indirect calls, callbacks, interrupts, tables and runtime
  state. Unobserved code is not proven unused. Prune only with auditable evidence;
  do not replace required behavior with success stubs or drop supported paths.
- **Driver source/build:** ship reviewable audio-only assembly and required
  constant data, with deterministic assembly/linking into the driver executable.
  A clean build must not read a game executable, BIOS or extraction cache.
- **Source organization:** use cohesive noun folders and single-noun Rust file
  stems; directory ownership replaces concatenated domain prefixes. Preserve
  Rust-mandated crate roots (`lib.rs`, `main.rs`, `mod.rs`) and existing public
  command behavior while reorganizing. No compatibility facades for old modules.
- **Coverage:** the user explicitly authorizes new migration tests.
- **Test language:** all audio-crate tests, helpers and reference models are
  Rust. Python tests are limited to harness command integration and packaging;
  do not relocate audio-domain tests into Python to bypass this boundary.
- **Non-goals:** TUI, live playback, standalone PSF commands, FLAC, disc-image
  rebuilding, symbol-table edits, and recovered game C changes.

The replacement surface is:

```text
bof3-audio <index|query|map|extract|render|pack|verify> --mode audio|music
```

Audio accepts `--type sfx|vocals`; music selects tracks and instrument banks.
Read a disc root or explicit archive list without prior extraction. Provide
human-readable and versioned JSON output. Keep `audio build` and `audio package`
as harness integration surfaces and expose the new operations directly.

## Execution order for the expanded scope

A1–A4 retain their evidence and interchange work. A6 isolates the standalone
runtime; A7 reorganizes the host source. Their discovery steps can proceed once
their listed prerequisites pass. Preserve stable A1–A5 IDs and historical
checkpoints. A5 final acceptance and C retirement must follow A6 and A7 as well
as the original gates. Original-executable and BIOS requirements in older
checkpoints describe evidence tooling or intermediate behavior, not the final
shipping contract. A6 removes those requirements from every production operation.

## 1. [A1] (in-progress) Establish evidence and the Rust foundation
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: A1.01–A1.03 foundation/profile evidence accepted below; engine selection remains A1.04
- Acceptance: baseline checks/media hashes recorded; Rust foundation and lossless archive checks pass; dependency/profile evidence reviewed

1. [A1.01] (done) Record host behavior, dirty work, and media
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: parent inspected owners and native/Python/Rust baseline results, media hashes, all 81 BGM audits and reference inventory above; baseline gaps classified
- Acceptance: existing native and Python audio checks run; failures classified; source/dirty baseline and local media hashes recorded

Record the current audio implementation and command behavior, executable and
EMI/XA corpus identities, available runtime traces and reference audio. Keep
proprietary media and generated artifacts out of source/package outputs.

2. [A1.02] (done) Create the Rust library and executable
- Owner: parent
- Depends: A1.01
- Blocker: none
- Evidence: parent reviewed crate/lockfile/ownership and 11 dependency license texts; Cargo format/check/Clippy and 47 audio tests pass, including 884-file container preservation
- Acceptance: Cargo formatting/checks/tests pass; module ownership and dependency/license inventory recorded

Create `tools/rust/bof3-audio/{Cargo.toml,Cargo.lock,src,tests}`. Separate archive
formats, identities, sequence translation, sample codecs, interchange formats,
machine/SPU execution, rendering, and verification. Reuse and extend
`tools/rust/emi-ex/src/lib.rs` to preserve every table field, padding byte,
unrelated entry, ordering decision, and trailing byte. Rendering-engine selection
and its separate approval/validation work are tracked in A1.04; later feature
owners are implemented with their A2–A4 operations, not empty scaffolding here.

3. [A1.03] (done) Pin the supported runtime profile
- Owner: parent
- Depends: A1.01
- Blocker: none
- Evidence: parent verified exact-hash US recognition/rejection, executable/table hashes, startup caller and table/tick entries; original pitch/cue/table checks pass and facts are recorded in the audio spec; execution/fidelity remains A4
- Acceptance: supplied US executable identified by hash and verified audio entry/table evidence; other revisions identified and unsupported profiles rejected

Use the available US executable as the initial verified profile. Keep runtime
and format evidence under the relevant `docs/specs/` owners, with target-qualified
addresses and original executable/SDK provenance. Analyzer research follows its
domain skill; no inferred address is promoted to a symbol table.

4. [A1.04] (in-progress) Select audited Rust synthesis and delivery codecs
- Owner: parent
- Depends: A1.02
- Blocker: expanded MP3 fidelity fails the final impulse threshold at 48000 Hz stereo, 8193 frames, 128 kbit/s; unsafe 32000 Hz mono 256/320 kbit/s configurations are explicitly rejected; consumer-specific gapless support and delivery quality/corpus/publication gates remain open
- Evidence: user-approved RustySynth 1.3.6 and the exact eight-package MP3/Vorbis graph are pinned with reviewed licenses; locked build/MSRV/Clippy/default tests and package build pass; codec acceptance remains open
- Acceptance: specific engine/codec choices and transitive licenses verified before selection; required additions authorized; production dependencies contain no native bindings or external renderers

Complete the MP3/Vorbis selection work recorded above. Missing
suitable Rust codecs require implementation, not a native fallback. Installation
and conformance acceptance remain separate. A2 needs the completed foundation
and profile work, not an installed renderer; A4 retains this dependency.

Delivery source-review checkpoint (2026-09-24): the
[codec reference](../reference/audio-codecs.md) records exact archive checksums,
license files, source commits and the eight-package graph proposed for
`oxideav-mp3 =0.1.3` and `oxideav-vorbis =0.0.12`. Pinned transitives are
oxideav-core 0.1.36, oxideav-ogg 0.1.8, bytemuck 1.25.2, thiserror and
thiserror-impl 2.0.21, and syn 3.0.6; existing Syn 2 remains for Serde. No native
source/library files were found. The only build script belongs to thiserror
and probes Rust compiler features. The reviewed MP3 tag API lacks delay/padding
fields, so timing integration is still required. Source evidence is under
`out/audio-migration/codec-review/`; no dependency was added, installed or built.
Explicit dependency approval, Cargo resolution/MSRV, independent conformance,
quality, timing and package validation remain open.

Authorized installation and consumer checkpoint (2026-09-24): the user approved
the exact eight-package graph above. Both direct versions and all six new
transitive versions are locked; fetched archives match the review, all previous
versions/checksums are preserved, and Syn 2 remains alongside Syn 3. License
notices are included. Rust 1.98.1 and MSRV 1.88 all-target builds, Clippy with
warnings denied, the default suite (362 passed, 144 media tests ignored), and
an independent source-package build pass. `installation.json`, `features.txt`
and validation logs are under `out/audio-migration/codec-review/`.
All 16 audio harness checks also pass with the verified local Cargo cache,
including packaged release build and tests without proprietary media; the
initial default-cache run failed because that cache lacked the approved crates.

The new development-only `examples/encoding.rs` exercises the direct encoder
APIs while retaining settings, source/output hashes, times and errors. FFmpeg
6.1.1 decodes all 16 initial Vorbis fixtures to their original lengths, but two
stereo fixtures produce three-mode parser warnings. Six single-frame MP3
fixtures fail consumer probing; longer MP3 signals have an unreported
1057-frame offset and insufficient tail flushing (34 missing input frames for
the 8193-frame cases). Overlap-only noise SNR at 192 kbit/s High is about
17.8 dB mono and 7.6 dB stereo. Three BOF3 music/SFX/voice clips also decode
to their input lengths in Vorbis; 37800 Hz XA input requires verified Rust rate
conversion for MP3, whose encoder currently rejects it. See the codec reference,
`probe.json` and `game-probe.json` for measurement limits. Resolve flushing,
honest delay/padding signaling, independent interoperability, quality and
publication before enabling delivery formats. Dependency approval/build success
does not complete A1.04, A4.02, or authorize C retirement; production rendering
remains WAV-only.

Rust timing and test-language checkpoint (2026-09-24): `codec/mp3.rs` adds a
1152-frame filter flush and an Info/gapless extension with truthful
`OxAV0.1.3` identification, CRCs, encoder delay and computed padding. The
Rust-only mpg123 oracle passes all 36 192-kbit/s rate/channel/length cases,
including first/last impulses and exact decoded lengths. The expanded bitrate
matrix exposed a 12-bit granule-length overflow for 32000 Hz mono at 256/320
kbit/s. The user declined vendoring and chose unchanged registry source; the
adapter rejects those settings without substituting a bitrate. The 144-case
matrix now explicitly rejects 12 configurations and decodes the other 132 to
exact lengths, but the final channel-1 impulse at 48000 Hz stereo, 8193 frames,
128 kbit/s is 1905 against a threshold greater than 8000. Both failure fixtures
and the unapplied patch proposal remain evidence. Keep the fidelity gate open;
do not promote delivery merely because timing checks pass. FFmpeg 6.1.1's
encoder-name restriction still prevents automatic gapless trimming. Xiph
libVorbis independently decodes all 19 retained Ogg fixtures at the expected
lengths. The Rust Vorbis adapter checks Ogg framing, CRCs, identification and
presentation length; a separate Xiph consumer passes all 60 rate/channel/length
cases including both XA rates and boundary impulses. Receipts are
`out/audio-migration/codec-review/mp3-registry-matrix.log` and `ogg-tests.log`.
Detailed scope and sources are in the codec reference; production
rendering remains WAV-only and A1.04/A4.02 are not complete.

Registry-preservation validation: 368 default Rust tests pass (147 ignored),
as do all-target Clippy with warnings denied, Rust 1.88 checks, formatting and
all 17 harness integration/package tests. The explicit MP3 matrix still fails
the amplitude gate above; default-suite success does not supersede it. Registry
MP3 source files match the audited published sources; no patch or vendor path
was installed. Logs use `registry-*` under the codec-review output directory.

Payload/quality follow-up: the MP3 adapter checks reservoir history, granule and
scalefactor bounds and complete big-value symbols before returning encoded data.
Four Rust cases cover malformed streams and reproduce the unchanged encoder's
12-bit overflow without proprietary fixtures. All 132 representable matrix cases
pass these checks and independent length decoding; the same amplitude failure
remains byte-identical to the retained fixture. A Rust preset comparison finds
identical output across Fast/Standard/High/Transparent for the impulse and the
selected music/SFX/noise clips. No preset or bitrate substitution was introduced.
The codec reference records measured SNR and validation limits; `payload-*` and
`quality-*` receipts remain under `out/audio-migration/codec-review/`. The default
suite now passes 372 tests (147 ignored); explicit fidelity acceptance stays open.

Per the user's test-language instruction, the new MP3 consumer and four older
Python validators/vector generators were replaced with Rust. Separate XML/RIFF
parsing, original-byte comparisons, FFmpeg PCM comparisons, all frozen XA
arithmetic/transition hashes and corpus invariants remain. The Rust consumers
pass the full 1020-bank/8385-sample extraction corpus and complete XA stream/cue
corpus. The source packager rejects Python inside the crates; all 17 harness
integration checks pass, including packaged build/tests. No audio-domain test
was moved into Python. Receipts use `rust-consumers-*`, `rust-vectors.log`,
`rust-bank-corpus.log`, `rust-xa-corpus.log` and `timing-rust-tests.log` under
`out/audio-migration/codec-review/`.

RustySynth's MIDI reader discards SysEx and most meta events, and its synthesizer
ignores unsupported controllers and pressure messages. A3/A4 must validate
supported semantics before synthesis and explicitly report or reject unsupported
transformations; successful library parsing does not establish fidelity. Its
stock MIDI loop modes do not establish BOF3 loop equivalence.

## 2. [A2] (in-progress) Index, query, and game mappings
- Owner: parent
- Depends: A1.01, A1.02, A1.03
- Blocker: none
- Evidence: A2.01 direct-media catalog implementation and corpus checks below; runtime mapping remains open
- Acceptance: direct-media index/query/map checks pass with qualified identities, versioned output and evidence-backed mappings

1. [A2.01] (in-progress) Implement inventory and selection
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: Rust catalog/CLI tests, full local-media inventory, resolved loader associations and exact bank-payload groups; content classification remains incomplete
- Acceptance: disc-root/archive-list input and both output formats tested; ambiguous numeric selections require qualification

Implement Rust CLI/catalog owners for `index` and `query`. Qualify identities by
source archive and entry; retain game bank, sample, song, and sequence IDs as
separate fields. XA identities use stream, channel, and cue/sector ranges, never
invented VAB identifiers. Cover music, SFX, vocals and shared-bank selection.

Implemented `bank.rs`, `sequence.rs`, `catalog.rs`, and `catalog_cli.rs`:
`index` and `query` accept a disc root or repeated archive arguments, with human
and versioned JSON output. Qualified identities retain entry numbers, one-based
sample IDs, independent SEP sequence indices/IDs, and XA file/channel/coding
with exact sector ranges. Numeric collisions reject with qualified alternatives.
Inventory retains opaque entry metadata and unresolved tone references. Without
an executable, runtime bank/song IDs remain null and bank/body links are candidates. Header
hashes expose identical VH content without claiming shared sample bodies.
With a verified executable and one resolved VB per bank, `bank_content.rs`
validates sample bounds and groups equal VH/VB hashes and lengths. All 1,020
banks have resolved content identities: 424 distinct payload pairs, including
99 repeated pairs covering 695 bank identities. Independent Python TOC/hashlib
results match every Rust group; corpus checks compare grouped bytes directly.
Changed headers/bodies, archive padding, missing/multiple/short bodies and
different game IDs are covered by migration tests. Identities and playback
contexts remain separate. Evidence and limits are recorded in the
[audio specification](../specs/formats/audio.md#byte-identical-vab-payload-groups).

Eight migration tests cover sparse/layered programs, malformed/truncated inputs,
duplicate names/IDs, CLI selection and output, and interleaved XA streams.
The local corpus inventory passes for 884 media files, 1,020 banks, 119 SEP
bundles and 476 independent sequences. `out/audio-migration/song-catalog.json`
records the direct-media song inventory; `out/audio-migration/rust-tests.log`
records the full Rust suite. This does not validate sequence events or playback.
`--type sfx|vocals` explicitly rejects pending evidence-backed classification;
edited-media song identities require preservation manifests, and unmapped XA
channels/caller bounds remain explicit gaps. Shared byte-content comparison
does not classify playback roles or prove rendering equivalence.
With `--executable`, loader associations and game bank IDs use A2.02 evidence.

```sh
cargo run --manifest-path tools/rust/bof3-audio/Cargo.toml -- index --mode music --disc-root out/extracted --kind song --json
cargo run --manifest-path tools/rust/bof3-audio/Cargo.toml -- query --mode music --disc-root out/extracted --id 'BIN/BGM/BGM000.EMI#entry=1/sequence=0' --json
```

2. [A2.02] (in-progress) Recover runtime associations and emit mappings
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: original US loader/layout execution checks and 1,020 corpus bank associations; broader mappings remain unresolved
- Acceptance: map output cites target-qualified executable/SDK evidence; unresolved associations remain explicit; symbol tables unchanged

Recover cue boundaries, bank loading, sequence selection, and scheduler behavior
from the executable and SDK evidence. Emit evidence-backed associations through
`map`; retain ambiguity and unresolved links without speculative acceptance.

A2.01 and A2.02 share the completed A1 foundation prerequisites: association
research feeds inventory selection, so neither waits for the other's entire
acceptance scope. Both must pass before A2 is accepted.

Implemented `mapping.rs` and `map --executable`, also available as an optional
annotation for `index`/`query`. The pinned US executable's layout initializer
runs for all three layouts (199 instructions each). Original handler argument
checks cover all seven slots per layout and stop before CD/SDK calls. Corpus
mapping resolves 1,020 VH/body associations, 904 auxiliary entries, and 119 SEP
links using loader order; game bank IDs are distinct from VAB header IDs.
Output records target-qualified symbols/addresses and full EXE offsets.
`out/audio-migration/loader-mapping.json` holds the local report. Durable facts
and test limits are in the [audio format specification](../specs/formats/audio.md#verified-us-emi-audio-slot-selection).

Implemented original music cue IDs `0`–`164` in `cue.rs`, with `query --cue` and
`game_song_ids` retaining song-bundle membership separately from SEP indices/IDs.
The original ISO directory independently verifies all 81 BGM file slots, paths,
LBAs, sizes and whole-file hashes in metadata-only `us_music.json`. Tests execute
the original cue file-selector and LBA lookup for every supported cue, validate
each archive against disc sectors, and cover renamed inputs, duplicate-source
ambiguity, edited-byte rejection and selector conflicts. Qualified paths plus
hashes distinguish the byte-identical BGMBAT00/BGMBAT02 archives.

Active load context, caller bounds beyond the supported music prefix, SFX/vocal
classification, XA caller bounds and scheduler behavior remain unresolved.

XA investigation found that all four pre-existing STR inputs are truncated
prefixes of the original disc extents. Initial corpus equality proves preservation
of those supplied files, not complete disc-stream recovery. Missing audio sectors
are 1,347 (MAGIC00), 1,238 (S_XA00), and 18 (CAPCOM30); VOICE loses only trailing
non-audio sectors. Those inputs remain unchanged. New Rust `disc.rs` reads ISO
extents in 2048-byte logical blocks while retaining each XA sector's 2336 bytes.
The recovery example created complete references separately under
`out/audio-migration/full-disc-streams`; independent raw-track SHA-256 comparison
passed for all four. Full-stream extent tests now supplement the original corpus
round trips. Known truncated inputs fail `verify` media-completeness checks even
when byte equality passes; unknown identities remain unchecked.

Original US XA selector execution now agrees with all 896 supported records
(11 S_XA00, 880 MAGIC00, 5 VOICE), including file/channel, start/current LBA
and raw stop-threshold fields. `index`/`query --kind xa_cue --executable EXE`
expose source-qualified cue assets and hexadecimal packed IDs. Whole-file
identity and selected-sector checks prevent edited or incomplete media from
silently inheriting usable cues; mappings retain missing-sector counts and
source completeness. All 26,115 selected audio-sector subheaders match direct
reads of the original raw disc. Tests cover duplicates, edits, truncated
sources and idempotent resolution. The threshold is not an accepted PCM
endpoint; additional channels, caller bounds and hardware scheduler timing
remain unresolved. Original cue initialization, completion/error callbacks,
position polling, threshold comparisons, cancellation and watchdog/retry paths
now pass instruction-level checks with injected responses. All 896 cues enter
the pause state at/above their threshold and remain playing below it. Rust CD
BCD/LBA conversion agrees with the linked SDK and raw disc headers. `map`
reports scheduler evidence; cue metadata includes BCD positions. Response
latency, tick frequency, buffered PCM and audible endpoints remain unverified.
Facts and limits are recorded in the [audio specification](../specs/formats/audio.md#complete-xa-extents-and-us-cue-selection).
Validation after WAV interchange integration: all 95 Rust tests passed with the US
executable, extracted corpus and original data track enabled (none ignored).
Cargo formatting, strict Clippy, plan parsing and scoped Markdown references
passed. This accepts the tested cue-selection/sector bindings and scheduler
control flow, not A2 as a whole or runtime playback; the full log remains
`out/audio-migration/rust-tests.log`.

The earlier 255-input cue check tests lookup arithmetic, not valid-table extent.
No symbol tables or recovered game C were modified; mapping is not runtime
playback acceptance.

```sh
cargo run --manifest-path tools/rust/bof3-audio/Cargo.toml -- query --mode music --disc-root out/extracted --executable out/extracted/SLUS_004.22 --cue 0 --json
BOF3_AUDIO_EXE="$PWD/out/extracted/SLUS_004.22" BOF3_AUDIO_CORPUS="$PWD/out/extracted" BOF3_AUDIO_TRACK="$PWD/inputs/external/Breath of Fire III (USA) (Track 1).bin" cargo test --locked --manifest-path tools/rust/bof3-audio/Cargo.toml --test cue -- --include-ignored
```

## 3. [A3] (in-progress) Editable extraction and reconstruction
- Owner: parent
- Depends: A1.01, A1.02, A1.03
- Blocker: none
- Evidence: verified archive/sample geometry and loader associations support codec work; extraction/reconstruction acceptance remains open
- Acceptance: whole-file unchanged EMI/XA corpus equality and supported edited interchange checks pass; unsupported edits rejected without publication

Codec/interchange work can proceed from the completed foundation and verified
asset geometry while A2 finishes content classification and remaining mappings.
That work does not depend on semantic SFX/vocal labels. A2 remains a required
A5 prerequisite; this dependency refinement does not waive command coverage,
mapping evidence, full round trips or runtime fidelity.

1. [A3.01] (in-progress) Extract editable bank audio and XA
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: bank extraction preserves 1,020 EMI snapshots and 8,385 sample slots; XA extraction validates 31 streams and 896 cues against complete STR bytes, independent WAV/PCM and executable tables; gameplay contexts and complete acceptance remain pending
- Acceptance: WAV/XML structure, loop endpoints, reference pitch/rate contexts, XA layout and preservation payloads validated

Create one folder per bank with PCM WAV files and relative-path XML references.
Write RIFF `smpl` loops with validated endpoint conversion. VAB samples have no
universal playback rate: document an export reference rate and known game pitch/
register/playback contexts. Export XA at its encoded rate/channel layout; retain
multiplexing and cue placement in XML.

Implemented the dependency-free Rust SPU sample decoder with carried/clipped
history, explicit unsupported-header diagnostics, first-end-block handling,
opaque tails and exclusive loop ranges. Independent Python traversal/decoding
matches all sample PCM hashes and metadata: 55 empty slots, 7,392 end/mute,
938 loops, 138,296,704 frames and 130,816 preserved post-end bytes. Of those
loops, 790 differ when predictor history carries into the next traversal;
fixed PCM-loop export must report that approximation. The SPU decoder's FFmpeg
comparison uses an unfiltered synthetic fixture; FFmpeg is never a production
fallback. Evidence and limits are in the
[audio specification](../specs/formats/audio.md#rust-sample-decoding-and-loop-export-limits).
`wave.rs` now reads/writes PCM16 WAV with explicit rates and validated inclusive
`smpl` endpoints. It retains sampler/opaque metadata, chunk order and padding,
updates PCM/fact sizes on edits, and rejects unsupported formats or SPU loop
transformations explicitly. All 1,180 unique sample allocations pass PCM/loop
WAV round trips; the development-only FFmpeg consumer also accepts the stereo
fixture. The 44.1 kHz corpus test rate is an export reference, not a VAB rate.
Facts and limits are in the
[WAV specification section](../specs/formats/audio.md#rust-wav-interchange).
`extract --mode audio --kind bank` now reads direct archives or a disc root,
requires verified executable loader associations, and supports qualified or
unambiguous numeric bank selection. It writes root/bank XML and PCM16 WAVs,
including full original EMI bytes in each bank manifest. Source snapshots,
program/tone metadata, export pitch/rate conventions, sample tails and loop
approximations are explicit. Loader metadata retains auxiliary/song links,
known song IDs and possible layouts/capacities without inventing an active
gameplay layout. Publication stages and verifies output, rejects
existing destinations and cleans owned staging content on ordinary failure.
The four extraction integration tests passed, including an independent Python
XML/RIFF consumer across all 1,020 banks and 8,385 samples. Every preserved EMI
equals its source and the aggregate PCM/loop evidence matches the established
independent reference. See the
[extraction schema and limits](../specs/formats/audio.md#rust-bank-extraction-and-preservation-xml).
Complete gameplay contexts, edited encoding and archive
reconstruction remain unfinished; this does not accept A3.01 or rendering fidelity.

The bank-extraction checkpoint passed 100 Rust tests with no ignored cases when supplied
the original US executable, corpus and disc track; strict Clippy, formatting and
the independent consumer's Ruff checks also passed. Publication-conflict coverage
preserves existing user output. A reviewable 21-sample BGM000 extraction is local
at `out/audio-migration/bgm000-bank-export-with-context`; generated media is not
part of the source package. The harness still builds the retained C tool.

`xa.rs` now decodes 4-/8-bit mono/stereo sectors at either encoded rate, with
explicit initial history and arithmetic selection. Invalid sectors cannot
partially mutate decoder history. Six synthetic tests cover parameters, ordering,
rate/depth, clipping, EOF, independent Python vectors and FFmpeg comparison.
The complete original-disc corpus test compares all 26,599 audio sectors in 31
streams byte-for-byte with FFmpeg under combined rounded prediction. Separate
floor prediction remains available and measured: 68,029,547 of 107,247,168 sample
values differ, by at most 101 PCM16 units. Local per-stream hashes and error metrics
are in `out/audio-migration/xa-decoder-corpus-evidence.json`. The retained C XA
traversal and prior format prose were incorrect; the specification now records
the supported layout and reference disagreement. No third-party dependency or
production renderer was added. See
[XA decoding evidence](../specs/formats/audio.md#rust-xa-decoding-and-arithmetic-evidence).
Hardware arithmetic, seek/reset history, de-emphasis, CD resampling and timed
playback remain unaccepted; the new decoder does not close A3.01 or A4.
After XA decoder integration, all 107 Rust tests passed with the original inputs
and independent FFmpeg consumer enabled; none were ignored. Strict Clippy,
formatting, the Python reference generator's Ruff check, scoped documentation
references, plan parsing and whitespace checks passed.

`extract --mode audio --kind xa_stream|xa_cue` now writes encoded-rate WAVs,
one shared XML snapshot per source STR and asset XML with physical sector/frame
maps, decoder arithmetic/history, and known cue positions/runtime thresholds.
Arithmetic selection is required; zero history per exported asset is reported
as an export convention. Cue extraction requires the verified executable;
streams can omit it with unresolved cue placement. Known truncated STRs fail,
unrecognized extents/unselected sources are reported, and all outputs use the
shared staged publication checks. Focused multiplexing/identity, malformed-input,
CLI and independent consumer checks passed. A reviewed voice cue `0x2000` export
is local at `out/audio-migration/voice-cue-export` (71 sectors, 286,272 frames).
See [XA extraction schemas and limits](../specs/formats/audio.md#rust-xa-extraction-and-preservation).
This implements extraction without accepting reconstruction, gameplay history,
audible endpoints or rendering; A3.01 remains in progress.
The full suite now passes 110 Rust tests with none ignored, including complete
XA stream and cue exports. Independent XML/WAV/FFmpeg checks cover all 31 streams
(26,599 sectors, 84,174,048 frames) and 896 cues (26,115 sectors, 83,196,288 frames);
all source snapshots equal the complete original STRs, and cue placement/thresholds
match raw executable table words. Strict Clippy, formatting, the new Python
consumer's Ruff checks, scoped documentation references and plan parsing pass.

2. [A3.02] (open) Extract songs, independent sequences, and SoundFonts
- Owner: parent
- Depends: A3.01
- Blocker: none
- Evidence: none
- Acceptance: format-1 MIDI/SF2 validated structurally and in an independent consumer; linked-runtime translation and tuning evidence recorded

Create one folder per song with its SoundFont and a format-1 MIDI per independently
selectable sequence. Keep SEP sequences independently selectable, never merged
into simultaneous MIDI tracks. Root XML connects songs, sequence indices, banks,
programs, tones, samples, and relative paths. Translate tempo, event ordering,
program changes, controllers, bends, and loops against the linked runtime.
Derive tuning from the verified game pitch algorithm; preserve layered tones,
key ranges, sample loops and reversible program mappings. Distinguish sample
loops, sequence loops and standard-SF2 approximations.

Preparatory sequence work now has a strict Rust event reader preserving raw
ranges, running status, order, both pitch-bend bytes and post-end data. All 476
corpus sequences (508,471 events) agree with an independent raw-TOC Python
survey. Six focused tests pass, including original-executable execution of
callback registration, delta decoding, program/note dispatch, bend argument
selection and tempo conversion. Original code confirms tenfold internal delta
units, high-byte-only bend handling and integer BPM conversion. Findings and
qualified addresses are in the
[sequence specification](../specs/formats/audio.md#verified-us-sequence-event-framing).
This is framing/context evidence toward A3.01/A3.02, not song extraction or
MIDI semantic acceptance. MIDI loop translation, end/restart handling, scheduler
quantization and instrument contexts remain unfinished; phase dependencies and
completion gates remain unchanged.

Validation for this preparatory change: the six focused tests pass with original
media enabled (`out/audio-migration/sequence-focused-tests.log`); the default
suite passes 77 tests and skips 39 opt-in media/consumer cases
(`out/audio-migration/sequence-default-tests.log`). Strict Clippy across all
targets, formatting, scoped Markdown references, plan parsing and whitespace
checks pass. The earlier full 110-test checkpoint remains historical evidence;
this change does not claim a fresh run of every opt-in case.

The Rust loop-state owner now matches original controller-99 start/end and
controller-6/98 count transitions, including the single saved cursor, count
reload guard, finite repeat delays and infinite-repeat zero delay. Original
instruction tests also follow a synthetic stream's cursor across repeats without
host rewrites. Corpus context checks cover 468 starts, 466 ends and 468 infinite
count assignments; unmatched starts remain visible. See
[loop-transition evidence](../specs/formats/audio.md#sequence-controller-loop-transitions).
This advances sequence semantics without accepting MIDI translation, end/restart
behavior, scheduler timing, full instrument effects or playback fidelity.

Loop integration validation passes all ten focused framing/loop/runtime tests
with the original corpus and executable enabled
(`out/audio-migration/sequence-loop-focused-tests.log`). The default suite passes
79 tests and skips 41 opt-in cases; strict Clippy, formatting, scoped references,
plan parsing and whitespace checks pass. Full XA/bank export tests were not
rerun for this isolated sequence change.

Separate Rust end-marker and per-tick clock models now match bounded original
execution. End checks cover 320 raw-state contexts, indefinite/finite restart,
signed counter wraparound and distinct-successor activation. Clock checks cover
864 original scheduling calls, slow-counter paths, event batching and a tempo
change within a tick; explicit event budgets reject zero-delay cycles. See
[end and scheduling evidence](../specs/formats/audio.md#sequence-end-restart-and-per-tick-scheduling).
Voice release used zero active voices. Hardware callback cadence, self-links,
complete chain use, actual audio, MIDI translation and edited representability
remain pending; these models do not accept the complete music/runtime phases.

This end/clock checkpoint passes 15 focused sequence tests with original media
enabled (`out/audio-migration/sequence-end-clock-tests.log`). The default suite
passes 81 tests and skips 44 opt-in cases
(`out/audio-migration/sequence-end-clock-default-tests.log`). Strict Clippy,
formatting, scoped documentation references, plan parsing and whitespace checks
pass. Full bank/XA exports were not rerun for these isolated sequence models.

The Rust MIDI codec now supports format-0/1 PPQN framing with parsed-content edit
detection. It preserves unchanged track encodings, header extensions, opaque
chunks and unknown meta/SysEx payloads; edited tracks retain event order and use
explicit status. Five tests cover preservation, edit isolation, malformed input,
channel shapes and VLQ boundaries. Independent RustySynth parsing confirms a
format-1 fixture's tempo-map duration and a parsed tempo edit. See
[MIDI interchange scope](../specs/formats/audio.md#rust-midi-interchange).
This is codec groundwork, not BOF3 event acceptance or the independent SF2
playback gate. Music extraction, semantic translation and packing remain open.

This MIDI checkpoint passes 86 default tests with 44 opt-in cases skipped
(`out/audio-migration/midi-default-tests.log`); strict Clippy, formatting, scoped
references, plan parsing and whitespace checks pass. The isolated codec change
did not rerun the full proprietary-media suite. Researched specification rules,
source locations and BOF3 distinctions are retained in the
[MIDI reference](../reference/standard-midi-files.md).

The Rust SF2 writer now emits authored mono-sample banks with layered zones,
shared samples, key/velocity ranges, tuning, envelopes, pan and explicit loop
modes. Six synthetic tests cover table bounds, sample guards, precise rejections
and independent RustySynth structure/playback, including a generated MIDI/SF2
pair with program changes and measured octave frequencies. See
[SF2 writing scope](../specs/formats/audio.md#rust-soundfont-writing) and
[retained source research](../reference/soundfont.md). The default suite passes
92 tests and skips 44 opt-in cases
(`out/audio-migration/soundfont-default-tests.log`). This does not accept actual
BOF3 conversion, arbitrary edited SF2 preservation, PSX fidelity or the complete
song-extraction gate; VAB translation and approximation measurement remain open.

SF2 validation also passes strict all-target Clippy, Rust 1.88 all-target checks,
formatting, scoped references, plan parsing and whitespace checks. The six SF2
tests were rerun successfully after lint cleanup; the full proprietary-media
suite was not rerun for this independent writer addition.

SF2 note-on calibration now executes the original US pitch routine and emits
per-key tuning/error reports and single-key zone parameters. A corpus check
covers all 21,112 tones / 236,302 declared tone-key contexts across 1,020 banks,
comparing original results with independent instruction-derived arithmetic.
It exposes 3,177 zero returns and 17,286 out-of-table accesses (overlapping
categories); 17,604 rows remain explicitly unsupported rather than being omitted.
Supported mappings stay within `0.499899535` cents of the unmodulated SPU
source-point rate. See [calibration evidence and limits](../specs/formats/audio.md#sf2-note-on-tuning-calibration).
The four focused tuning tests pass with original media
(`out/audio-migration/tuning-corpus-tests.log`). This advances bank translation
but does not close it: exceptional runtime contexts, envelopes, sample-loop
approximations and actual bank/sequence export remain unfinished.

The tuning checkpoint passes 95 default tests with 45 opt-in cases skipped
(`out/audio-migration/tuning-default-tests.log`), strict all-target Clippy,
Rust 1.88 all-target checks, formatting, scoped references, plan parsing and
whitespace checks. The focused corpus test was rerun after adding fixed boundary
and corpus-count assertions. Full bank/XA export checks were not rerun for this
isolated calibration addition.

The Rust ADSR component now models register-driven attack/decay/sustain/release,
rate counters, signed level writes, key-on/off and forced-off state. It requires
an explicit arithmetic model because the published pseudocode and inspected
emulator disagree at the exponential threshold and slow-rate floor. Six focused
tests plus an opt-in corpus probe cover the component. The probe compares 250 ms
held / 250 ms release trajectories for all 543 VAB ADSR pairs across 21,112 tones;
125 pairs used by 395 tones differ. See [ADSR evidence and limits](../specs/formats/audio.md#rust-adsr-state-and-evidence-models)
and [retained references](../reference/spu-envelopes.md).
`out/audio-migration/adsr-corpus-tests.log` records seven passing tests. This is
shared groundwork for A3.02/A4: hardware validation, device integration, game-time
overrides and measured SF2 envelope approximations remain open.

The ADSR checkpoint passes 101 default tests with 46 opt-in cases skipped
(`out/audio-migration/adsr-default-tests.log`). Seven focused cases pass with
the corpus enabled after adding boundary/corpus assertions. Strict all-target
Clippy, Rust 1.88 all-target checks, formatting, scoped documentation references,
plan parsing and whitespace checks pass. The full proprietary-media workflow
was not rerun for this isolated component addition.

The execution-order timeline and SEP-to-MIDI translator now expand source loops
without replaying stale running-status channels. They preserve source-event
mappings and original bend bytes in the timeline, write separate conductor and
performance tracks, and report finite-loop policy plus timing/bank-response limits.
All 476 corpus sequences produce independently readable MIDI; 466 reach the
chosen infinite-loop boundary and 10 reach EOT. Original dispatcher comparisons
cover cursor, delay, channel and count transitions, and a synthetic translated
sequence plays through the PC renderer. See [translation evidence and remaining
gates](../specs/formats/audio.md#sep-execution-order-midi-translation).
The focused media checks pass (`out/audio-migration/sequence-midi-tests.log`).
Music-folder/XML publication, SF2 bank binding, measured controller/bend/envelope
response, game cadence and reconstruction remain required; A3.02 stays open.

This checkpoint passes 122 default tests with 49 opt-in cases skipped
(`out/audio-migration/sequence-midi-default-tests.log`) and 20 focused checks with
the original executable/corpus enabled. Strict all-target Clippy, Rust 1.88
all-target checks, formatting, scoped references, plan parsing and whitespace
checks pass. Bank/XA extraction and whole-file packing workflows were not rerun
for this sequence translation component.

Bank-binding groundwork now includes a measured SF2 envelope fitter for either
explicit SPU arithmetic model. It retains RMS and maximum sampled error, phase
boundaries, moving sustain and unfinished release evidence instead of treating
all VAB envelopes as a constant sustain. All 543 corpus pairs / 21,112 tones were
probed under both models. Substantial approximation errors remain: 65/64 pairs
exceed normalized RMS 0.05 for Published/EmulatorReference. See
[fit evidence and limits](../specs/formats/audio.md#measured-sf2-envelope-fitting).
This does not complete bank binding, resolve gameplay overrides or accept PC/PSX
envelope equivalence.

The envelope-fitting checkpoint passes 126 default tests with 50 opt-in cases
skipped (`out/audio-migration/envelope-fit-default-tests.log`) and five focused
checks including both-model corpus fitting (`out/audio-migration/envelope-fit-tests.log`).
Strict all-target Clippy, Rust 1.88 all-target checks, formatting, scoped references,
plan parsing and whitespace checks pass. Archive round trips, full song export
and original-game playback were not rerun or accepted by this fitting work.

The new VAB-to-SF2 binding component joins sample identities, sparse programs,
layered tones, complete per-key pitch contexts and measured envelope fits. Gain/
pan must be supplied explicitly with provenance; no unverified game mapping is
invented. Synthetic banks and original-pitch contexts pass independent consumer
checks. The corpus probe found **zero accepted complete banks**: all 1,020 hit
the current program/tone mode rejection. The initial positive-acceptance assumption
failed; the test now records that incomplete coverage explicitly. See
[binding behavior and remaining gates](../specs/formats/audio.md#vab-bank-binding-with-explicit-rendering-contexts).
At that checkpoint, the next required evidence was the linked runtime's mode and
gain/pan behavior, followed by supported pitch contexts and music-folder publication. This is not bank
conversion acceptance and does not close A3.02.

The binding checkpoint passes 130 default tests with 52 opt-in cases skipped
(`out/audio-migration/bank-soundfont-default-tests.log`). Six focused checks include
synthetic playback, original note-on pitch and corpus **rejection classification**
(`out/audio-migration/bank-soundfont-tests.log`), not a passing corpus conversion
gate. Strict all-target Clippy, Rust 1.88 all-target checks, formatting, scoped
references, plan parsing and whitespace checks pass. Existing archive/XA workflows
were not rerun for this binder; full song export and rendering remain unfinished.

The subsequent [original note-on gain/mode checkpoint](../specs/formats/audio.md#original-note-on-gain-pan-and-tone-modes)
adds a Rust model of the linked runtime's staged stereo volume registers. It agrees
exactly with 2,432 original-US note-on executions across individual and mixed
seven-bit controls. Original execution shows program mode is unread on that path;
tone-mode bit `0x04` routes the voice to reverb. All 256 byte values and all 24
voice-mask positions are covered, including original flush writes to both SPU
reverb-register halves. These are controlled post-bank-open fixtures and a write
witness, not full boot, game-cadence or PCM acceptance.

Four focused checks pass (`out/audio-migration/voice-gain-tests.log`); the default
suite passes 131 tests with 55 opt-in cases skipped
(`out/audio-migration/voice-gain-default-tests.log`). Strict all-target Clippy and
Rust 1.88 all-target checks pass. The binder still rejects modes until SF2 gain/pan
and explicit reverb approximation are implemented and measured. Dynamic volume/
bend behavior, unsupported pitch contexts, song-folder publication and complete
corpus conversion remain open. No C removal or A3.02 completion is authorized by
this bounded execution evidence.

The [static SF2 gain/reverb mapping](../specs/formats/audio.md#sf2-static-gain-and-reverb-mapping)
now binds the recovered volume arithmetic to explicit specification-scale or
RustySynth-1.3.6 fits. Reports retain both predictions and the reference controls/
normalization; the engine's 0.4 attenuation behavior is measured. Reverb generator
16 requires a caller-supplied send/provenance for tone mode 4; no game depth is
inferred. Program mode remains preserved metadata, based on the verified note-on
path. The earlier blanket mode rejection has been replaced by those scoped rules.

The updated 1,020-bank corpus probe constructs and independently loads 494 banks /
5,995 zones. Remaining first-failure categories are 418 silent-gain contexts,
51 out-of-table pitches, 38 zero pitches, 17 invalid sample references and two
SF2 tuning-range failures. All remain explicit rejections. These counts are
structural coverage under a full-send reverb approximation, not game-song or PCM
acceptance. The next work is a reversible representation for silent tones and
evidence-backed handling of those sample/pitch contexts, followed by dynamic
controller measurement and complete song/XML publication.

This checkpoint passes 135 default tests with 55 opt-in cases skipped
(`out/audio-migration/gain-fit-default-tests.log`) and 16 focused checks including
the corpus and independent gain/reverb playback (`out/audio-migration/gain-fit-tests.log`).
The PCM probe exposed an omitted mixer cutoff, now modeled. It also disproved a
partial-source reverb-scaling inference: the complete consumer path uses correct
SF2 percent conversion, retained in the test/reference correction. Strict
all-target Clippy, Rust 1.88 checks and formatting pass. Full runtime audio,
edited reconstruction, song export and C retirement remain open.

The [silent-tone representation](../specs/formats/audio.md#silent-tone-preservation)
now retains original PCM and source identities while linking muted zones to an
explicit playback-only zero sample. Gain fits return no generator pair for
silence; the binder requires a verified maximum ordinary-sequence reference
context and rejects temporary controller mutes or inconsistent fits. Existing
sample/pitch checks still apply. A consumer test restores audible source PCM by
relinking a zone; edited archive reconstruction is still not implemented.

Corpus construction now reaches 825 banks / 14,890 zones, including 5,388 silent
tones in 331 additional banks. The 195 remaining first-failure rejections are
131 out-of-table pitches, 41 zero pitches, 19 invalid sample references and four
tuning-range failures. All constructed banks' retained and synthetic sample PCM
is checked through RustySynth. These results replace the prior 494-bank coverage
count without accepting full song playback or corpus round trips.

Twelve focused checks pass (`out/audio-migration/bank-silence-corpus-tests.log`),
including exact silent playback across 27 controller/velocity combinations,
retained-PCM relinking, malformed contexts and the updated corpus assertions.
The default suite passes 137 tests with 55 opt-in cases skipped
(`out/audio-migration/bank-silence-default-tests.log`); strict all-target Clippy,
Rust 1.88 checks and formatting pass. Archive round trips and full game playback
were not rerun or accepted by this representation change.
The next work is original-runtime evidence for the remaining sample/pitch
contexts, then dynamic-controller comparison and song/XML publication. A3.02,
full execution, edited packing, delivery codecs and C retirement remain open.

US sample-reference checkpoint (2026-09-24):
[Original loader and note-on execution](../specs/formats/audio.md#us-runtime-sample-reference-resolution)
establishes low-byte selection, byte-zero aliasing to sample 2 and the separate
byte-255 noise path. The binder retains encoded and resolved identities; noise
and allocations outside the declared bank remain rejected. The bank parser now
accounts for the transferred size-table entry-zero prefix, verified with nonzero
synthetic cases. Original loader results agree for all 1,020 corpus banks and
8,385 sample starts, packed tone block indices and transfer extents.

Construction now reaches 842 banks / 15,095 zones, with 20 zero aliases and
5,397 silent tones in 332 banks. The 178 first failures are 133 out-of-table
pitches, 41 zero pitches and four tuning-range failures. This does not resolve
the out-of-bank reference in `BIN/BPLCHAR/DRG04_00.EMI`, masked by an earlier
pitch failure. Synthetic setup establishes RAM/register selection, not DMA,
full scheduling or audible fidelity.

Validation: 16 focused checks pass, including the original loader corpus;
the default suite passes 141 tests with 58 opt-in cases skipped after correcting
the catalog fixture's obsolete opaque-prefix assumption. Logs are
`out/audio-migration/sample-reference-corpus-tests.log` and
`out/audio-migration/sample-reference-default-tests.log`. Strict all-target Clippy,
Rust 1.88 checks and formatting pass. Sample/pitch context gaps, dynamic controls,
song/XML publication, edited packing, complete runtime rendering, delivery codecs
and C retirement remain open; this checkpoint does not satisfy A3.02.

3. [A3.03] (open) Pack preserved and edited components safely
- Owner: parent
- Depends: A3.01, A3.02
- Blocker: none
- Evidence: none
- Acceptance: unchanged archives/streams byte-equal originals; edited codec error reported; invalid edits/capacity failures cannot publish output

Accept one song/bank folder or an entire extraction root. Detect edits from
parsed content; reuse XML preservation data only for unchanged components.
Reconstruct original encodings/layout for unchanged assets. Validate edited MIDI/
SF2 against supported events, bank limits, SPU memory, sample alignment, pitch
ranges and loop constraints. Reject unsupported/unrepresentable edits precisely;
never discard events silently. Encode changed samples as PSX/XA ADPCM and report
loss. Preserve XA interleaving and unrelated sectors; reject edits beyond cue/
stream capacity. Verify rebuilt EMI/STR files before publishing to a separate
output location; failure must preserve existing outputs and original media.

Edited-sample codec checkpoint (2026-09-24):
The [Rust PSX ADPCM encoder](../specs/formats/audio.md#rust-edited-sample-psx-adpcm-encoding)
now searches supported predictors/shifts, validates sample/loop block alignment,
requires explicit non-looping padding, enforces optional byte capacity and reports
decoded quantization error. Predictor-zero loop entry yields identical decoded
PCM across traversals; its encoding error remains visible. Unchanged encodings,
prefixes, unused allocation bytes and opaque tails are still the packer's
responsibility. No new dependency or external production executable is used.

Eight focused checks pass, including eight additional loop traversals, independent
FFmpeg VAG consumption and edited excerpts from 1,179 unique nonempty allocations
(264,096 frames; peak error 813, weighted RMS 38.462789). FFmpeg's different
predictor arithmetic is checked explicitly, not used as the SPU fidelity oracle.
All 8,385 original decoder slots retain their independent reference results.
The default suite passes 147 tests with 60 opt-in cases skipped; strict all-target
Clippy, Rust 1.88 checks and formatting pass. Logs use
`out/audio-migration/adpcm-encoder-*.log`.

This is a codec prerequisite, not acceptance of A3.03 or its extraction
dependencies. Manifest parsing/change detection, XA encoding, edited musical
validation, full reconstruction, publication and command wiring remain open.

XA encoding/reconstruction checkpoint (2026-09-24):
[The Rust XA encoder and whole-stream replacement](../specs/formats/audio.md#rust-xa-encoding-and-stream-reconstruction)
now preserve coding, channel/file identity, sector count/placement, subheaders,
spare bytes and unrelated multiplexed sectors. They require exact PCM capacity
and explicit arithmetic/history, report per-channel loss, reuse original sectors
only when decoded content matches under output history, and account for changed
history in following sectors. Present Form 2 EDC is validated/recomputed; absent
EDC remains absent. Known truncated inputs are rejected before reconstruction.
The shared quantization owner preserves PSX encoder behavior without dependencies.

Eleven focused checks pass. Whole-file equality holds for all 31 streams in four
complete STR extents (26,599 audio sectors / 84,174,048 frames). Edited excerpts
cover 61 sectors / 245,952 channel samples, with peak error 1,173 and weighted RMS
48.023856. CAPCOM30 has one single-sector mono stream, so the initial 62-excerpt
test expectation was corrected. Synthetic tests cover all supported XA formats,
both arithmetic models, history effects on later sectors, capacity failures,
malformed input and checksum preservation. FFmpeg independently reproduces
generated four-bit XA; 28 original nonzero disc-sector EDC values check the
checksum polynomial. All four known truncated source prefixes fail as intended.

Validation logs: `out/audio-migration/xa-encoder-focused-tests.log` (11 checks),
`xa-encoder-decoder-regressions.log` (20 PSX/XA checks, including media/consumer
cases), and `xa-encoder-default-tests.log` (155 passed / 63 opt-in skipped).
Strict all-target Clippy, Rust 1.88 and formatting pass. These are in-memory
codec/reconstruction results. XML parsing, edit/conflict detection, partial cue
boundary/padding policy, musical edit validation, publication and CLI packing
remain open; A3.03 and its extraction dependencies are not complete.

XML-reader checkpoint (2026-09-24): the user explicitly authorized an XML parsing
crate. `roxmltree = 0.21.1` is now pinned, with verified published checksum,
MIT/Apache licenses and the upstream ISC tree attribution retained. Its sole
dependency, `memchr 2.8.3`, was already locked; no native binding or external
production executable was added. Dependency evidence and reader behavior are
recorded in the [audio specification](../specs/formats/audio.md#rust-extraction-manifest-reader).

The bounded reader handles real extraction XML, normalized values and relative
paths; helpers validate preservation bytes/hashes, typed source metadata and
schema shape without silently discarding unknown fields. An initial test exposed
the parser's replacement of invalid Unicode numeric references; pre-validation
now rejects those inputs while preserving literal text in comments/CDATA.
DTDs/external entities, namespaces and processing instructions remain unsupported.

Eight focused checks pass, including fresh BGM000 XML/WAV extraction (21 samples /
408,212 frames) and synthetic multiplexed XA preservation. The default suite
passes 162 tests with 64 opt-in cases skipped; strict all-target Clippy, Rust 1.88,
formatting and locked dependency metadata checks pass. Logs use
`out/audio-migration/manifest-reader-*.log`; fetched package provenance is under
`out/audio-migration/dependency-research/roxmltree-0.21.1.*`.

That checkpoint established the XML-reading layer only. The following bank
packing checkpoint advances A3.03 without completing its music/XA obligations.

Bank-packing checkpoint (2026-09-24): `pack --mode audio --input FOLDER
--executable PSX_EXE --output NEW_DIRECTORY [--json]` now accepts standalone bank
folders and bank extraction roots. XML-preserved archives supply the catalog;
the verified US executable checks runtime metadata without requiring original
archive paths. Complete bank schema validation rejects unsupported metadata
edits and duplicate/conflicting identities. Parsed WAV PCM and forward loops
drive edit detection; unchanged samples reuse their original encodings, while
supported block-aligned edits are encoded within fixed allocations with measured
loss and explicit tail-consumption reports. Multiple banks combine per source;
conflicting preservation fails before publication. Whole-file/entry/read-back
checks precede staged output, with source-to-output identities in `pack.json`.
See [bank packing](../specs/formats/audio.md#rust-bank-packing) for exact limits.

The default suite passes 168 checks with 67 opt-in cases skipped; all eight
focused checks pass. The opt-in corpus case reproduces all 809 bank-containing
EMI files byte-for-byte, covering 1,020 banks and 8,385 samples. Its initial
combined invocation also failed a separate fixture that named absent BGM001;
that fixture now uses BGM002 and passes in `bank-pack-focused-final-tests.log`.
The corpus case itself passed in `bank-pack-focused-tests.log` (695.45 seconds
for that invocation). Standalone/root and combined-bank tests remove original
copied inputs before packing, preserve unrelated bytes after edits and reject
conflicting source preservation. Strict all-target Clippy, Rust 1.88, formatting,
documentation references and plan status checks pass. Logs and reproducible
BGM000 CLI extraction/pack/verify artifacts use `out/audio-migration/bank-pack-*`.
At that checkpoint music/SF2 edits and XA manifest packing remained open, as did
full runtime fidelity, delivery codecs, harness migration and C retirement.
A3.03 and the complete migration were not accepted.

XA-packing checkpoint (2026-09-24): the shared `pack --mode audio` command now
dispatches stream/cue extraction roots to XML-backed STR reconstruction. Source
preservation and asset schemas validate identities, runtime cue placement,
sector/frame maps, arithmetic/history and original hashes without requiring the
old STR paths. Fixed-capacity WAV edits encode with measured loss. Partial edits
require compatible continuous-stream entry history and original exit history
before following audio; unsupported history, capacity and metadata changes fail
explicitly. Overlapping asset views must agree on sector bytes. One source is
published only after merged sector and whole-file/read-back checks, with
`bof3.xa-pack-report/v1` evidence. The XML byte bound is now 256 MiB to accommodate
the complete S_XA00 preservation snapshot. See
[XA manifest packing](../specs/formats/audio.md#rust-xa-manifest-packing).

Synthetic selection/packing checks pass across all eight XA rate/channel/depth
formats. An original MAGIC00 cue edit (`0x1000`, first 112 PCM values adjusted)
passes the history gates and preserves every unselected sector; a modified runtime
selector fails without publication. The default suite passes 173 checks with
69 opt-in cases skipped; Clippy and Rust 1.88 checks pass. The full corpus case
passes: all four complete STR sources round-trip byte-for-byte through 31 stream
exports, and the three cue-bearing sources also round-trip through all 896 cue
exports. MAGIC00 contributes 880 cues, S_XA00 11 and VOICE 5. The 218.27-second
case is recorded in `out/audio-migration/xa-pack-corpus-tests.log`.
Other evidence uses `out/audio-migration/xa-pack-*`. Automatic padding for shorter
cues and general seek/reset-dependent edits remain unsupported, and full runtime
fidelity, music interchange/packing, delivery codecs, harness migration and C
retirement remain required. A3.03 and the full migration are still open.

Music-extraction checkpoint (2026-09-24): `extract --mode music` now connects
verified loader associations, bank/SoundFont binding and independent sequence
translation. It requires `--allow-approximations`; the explicit initial policy
uses RustySynth 1.3.6 gain fits, published-model envelope fits, dry reverb and marked
fixed-PCM loops. Each song has its own SF2, separate format-1 MIDI files for all
selectable SEP sequences, source-preserving `song.xml` and root `music.xml` links.
Program/tone/sample identities, game cue IDs, translation/timeline mappings and
complete original EMI bytes remain inspectable. Source event mappings account for
generated MIDI setup events. Parser/preset and read-back checks precede staged
publication. This is not accepted dynamic-controller, scheduler or PSX fidelity.

Four focused checks pass, including layered sparse programs, two synthetic
independent sequences, loop expansion, source-byte equality and compatible
SoundFont/MIDI consumption. BGM004 exports four independent sequences and renders
through the PC CLI (66,150 stereo frames, peak 0.077882, zero clipped samples).
BGM000 and BGM002 are explicitly rejected at out-of-table tone-key pitch contexts,
with no output published. An initial test requested a render longer than its short
synthetic sequence; bounding the requested cutoff to that sequence resolved it.
The default suite passes 175 tests with 71 opt-in checks skipped; strict Clippy
and Rust 1.88 checks pass. Evidence uses `out/audio-migration/music-extract-*`.
See [music extraction](../specs/formats/audio.md#rust-music-extraction-with-explicit-approximations).
Remaining requirements include initialized-game pitch context for rejected banks,
full music coverage, faithful dynamic bindings, edited music reconstruction,
complete runtime/audio evidence, delivery codecs and harness/C retirement gates.
Neither A3 music acceptance nor the overall migration is complete.

Pitch-context checkpoint (2026-09-24): tuning records actual halfword lookup
addresses/values and file offsets, classifies note-on table / earlier SPU table /
adjacent data, and includes the witness in rejected-bank diagnostics. Original
instructions establish the earlier table's separate consumer and show that
tick-mode setup changes an adjacent lookup and the resulting pitch register.
The full corpus has 219,016 / 12,623 / 4,663 reads in those three regions;
acceptance remains unchanged. BGM000's rejected tone spans both the earlier
table and adjacent scheduler data, so the duplicate is not a blanket fix.
See [the runtime-state evidence](../specs/formats/audio.md#adjacent-pitch-data-and-initialized-state).
The full initializer still stops at unsupported BIOS `A0:72`; complete runtime
initialization, reachable-note analysis and music acceptance remain open.
The default suite passes 175 tests (73 opt-in checks skipped); ten focused
tuning/context/music-export checks pass, including the full tone/key corpus.
Strict Clippy, Rust 1.88, formatting, plan parsing and scoped reference checks
pass. Evidence is under `out/audio-migration/pitch-context-*`; the spec retains
the target-qualified addresses and the independent arithmetic comparison.

MIDI inverse checkpoint (2026-09-24): the
[fixed-layout inverse](../specs/formats/audio.md#fixed-layout-midi-to-sep-reconstruction)
rebuilds supported note, program, volume/pan, bend and tempo values from edited
exports while preserving original SEP encoding and entry layout. Correspondence
is regenerated from original bytes and explicit loop limits; all repeated visits
must agree, including unchanged visits. Generated setup, event order/ticks,
channels and loop structure remain fixed. Unsupported edits fail explicitly, and
complete forward retranslation checks the requested result. Export and inverse
now share one channel-setup owner. This is a library prerequisite: music XML
packing, bank/pitch constraints, arbitrary SF2 edits and publication remain open.

All 476 corpus sequences reconstruct into 119 byte-identical complete SEP entries;
consistent velocity edits change 231,340 source events across those entries and
retranslate to the requested events. RustySynth independently parses edited SMFs.
Four synthetic inverse checks cover encoding preservation, mixed edits, conflicting
loop visits and rejection behavior; existing translation and song extraction
checks also pass: 14 focused tests with local media, zero ignored; 235 default
tests pass with 83 opt-in cases skipped. Strict Clippy, Rust 1.88 all-target checks,
formatting, scoped documentation references, plan parsing and whitespace checks
pass. Evidence uses `out/audio-migration/sequence-edit-*`. This does not close
A3.02/A3.03, establish original-game audio fidelity or permit C removal.

Music-packing checkpoint (2026-09-24):
[`pack --mode music`](../specs/formats/audio.md#music-folder-packing) now accepts
one song folder or an extraction root. Shared preparation regenerates export
bindings, sequence mappings and canonical metadata from preserved EMI bytes and
the supported executable before accepting fixed-layout MIDI edits. Changed source
programs must exist in the paired bank. SoundFonts must remain byte-identical to
the regenerated export; arbitrary SF2 inverse work remains required. Shared-bank
songs merge into one source archive, with conflict detection, verified unchanged
whole-file equality, staged read-back and separate reconstruction reports.

Synthetic standalone/root, layered/two-sequence, shared-bank and failure-path
checks pass; BGM004's four sequences round-trip byte-exactly without the copied
source file. Relative MIDI/SF2 paths may be renamed. A synthetic edited-archive
check initially expected only one changed velocity byte; the existing EMI writer
also refreshes the deliberately stale cached first word. The corrected assertion
checks that exact cache update and preserves all other padding/unrelated bytes.
Nine focused music tests and four bank-packing checks pass with local media;
the expensive unchanged full-bank corpus case was not repeated for the mechanical
archive-staging move. The default suite passes 236 tests with 87 opt-in cases
skipped. Strict Clippy, Rust 1.88 all-target checks, formatting, scoped references,
plan parsing and whitespace checks pass.
Evidence uses `out/audio-migration/music-pack-*`. Full music corpus conversion,
edited SF2 reconstruction, broad transformation constraints, initialized runtime
and independent audio fidelity remain open; this does not close A3.02/A3.03 or
permit C removal.

SoundFont-reader checkpoint (2026-09-24): the
[preservation-aware reader](../specs/formats/audio.md#rust-soundfont-reading)
now exposes SF2 table relationships, first global/local zones, raw generators and
modulators, sample links and exact signed PCM24. RIFF boundaries, table indices,
record limits and sample references are checked. Unknown chunks and opaque
terminal payloads remain in original bytes; parsing does not imply synthesis or
game representability. Music packing invokes the reader before its unchanged-SF2
check. This advances the input layer for edited SF2 reconstruction; samples,
instrument/tuning/envelope inverses and broader musical acceptance remain open.
Primary research is retained in the SoundFont reference, with synthetic malformed,
24-bit/stereo/global-zone tests and corpus-reader evidence under
`out/audio-migration/sf2-read-*`. A3.02/A3.03 and C retirement remain unaccepted.

Reader validation passes 25 focused tests with local media, including all 842
constructible corpus banks / 15,095 zones, exact PCM and independent consumer
record counts. The same 178 pitch-related conversion failures remain explicit.
The default suite passes 242 tests with 87 opt-in cases skipped. Strict Clippy,
Rust 1.88 all-target checks, formatting, scoped references, plan parsing and
whitespace checks pass. The final reader-only checks also exercise retained
nonzero guard data; SF2 semantic edits are still rejected by music packing.

SoundFont-sample packing checkpoint (2026-09-24):

The [bounded sample inverse](../specs/formats/audio.md#rust-soundfont-sample-packing)
now rebuilds SF2 PCM and shared-consistent sample-loop edits within existing VAB
allocations. Fresh source binding/export determines identity and edit authority.
Unchanged decoded content, including PCM16 represented exactly as PCM24, retains
original ADPCM; edited PCM24 reports rounding and total decoded loss separately
from PCM16 encoder loss. WAV/SF2 packing share allocation reconstruction. Sample
geometry, headers other than loop points, instruments, tuning, envelopes,
metadata, opaque chunks/padding, guards and generated silence remain immutable.
Release-only loops, inconsistent shared modes, unaligned loops and post-loop
tails reject explicitly. Selected songs sharing a bank must all request identical
rebuilt body bytes, including unchanged copies; supported MIDI and sample edits
can publish together through the existing verified archive staging.

All 842 constructible corpus banks / 15,095 zones retain exact original body bytes;
the existing 178 pitch-context conversion failures remain explicit. Fourteen
focused sample/music/bank checks pass with local media, including edited BGM004,
combined MIDI/sample edits and shared-bank conflicts. The default suite passes
247 tests with 87 opt-in cases skipped. Strict Clippy, Rust 1.88 all-target checks
and formatting pass. Evidence uses `out/audio-migration/sf2-pack-*`. These checks
do not establish independent runtime/audio fidelity, complete music coverage or
the remaining instrument/tuning/envelope inverses. A3.02/A3.03 and C retirement
remain unaccepted.

SoundFont tone-control checkpoint (2026-09-24):

The [tone gain/pan inverse](../specs/formats/audio.md#rust-soundfont-tone-control-packing)
now accepts uniform per-tone SF2 attenuation/pan changes that have an exact
forward fit under the recorded ordinary-sequence gain model. It searches all
seven-bit tone volume/pan pairs, keeps bank/program controls fixed, resolves
equivalent pairs by minimum distance from original controls and reports candidate
counts and fit error. Only tone-record bytes 2/3 change; unchanged controls and
opaque header bytes remain exact. Per-key/global changes, silent-tone remapping,
unrepresentable pairs, topology, tuning and envelope edits reject explicitly.
Music packing validates samples and tone controls together and stages header,
body and sequence replacements; disagreeing selected copies of a shared header
or body fail before publication.

All 842 constructible corpus banks / 15,095 zones retain unchanged header/body
bytes. One eligible nonsilent tone edit in each of 719 banks reconstructs the
requested SF2 bytes and passes independent RustySynth parsing; 123 banks have no
eligible edit under this probe. The existing 178 pitch-context conversion failures
remain. Fourteen focused tone/sample/music checks and seven gain checks pass,
including 2,432 original-US note-on contexts and independent consumer gain PCM
checks. The default suite passes 251 tests with 87 opt-in cases skipped. Strict
Clippy and Rust 1.88 all-target checks pass. Evidence uses
`out/audio-migration/tone-pack-*`. Exact generator reconstruction at the recorded
context does not prove subsequent controller behavior or PSX/PC PCM equivalence.
Broader instrument reconstruction, initialized runtime, complete music coverage
and independent audio fidelity remain open; A3.02/A3.03 and C retirement are not
accepted by this checkpoint.

SoundFont note-on tuning checkpoint (2026-09-24):

The [pitch inverse](../specs/formats/audio.md#rust-soundfont-pitch-packing) accepts
fixed-zone root/coarse/fine edits when original US pitch execution finds an exact
combined-cent match for every key. It searches every byte-valued center and each
of the 32 distinct zero-fine shift groups, rejects unstable/out-of-table and
unrepresentable contexts, and ranks equivalent encodings by distance from the
original bytes. Only validated center/shift bytes change; representation-only
edits preserve original encodings. Reports retain executable identity, candidate
counts and per-key register/table/error evidence. The music packer validates
pitch, gain and sample edits together and rejects shared-bank conflicts before
publication.

All 842 constructible corpus banks preserve unchanged VH/VB bytes, and one
eligible tuning edit in every bank reconstructs the requested note-on cents and
passes independent RustySynth parsing. The 178 preexisting conversion failures
remain explicit. Layered/equivalent tuning, 1,050 ignored-shift-bit comparisons,
unrepresentable per-key edits and shared-song publication checks pass. The default
suite passes 266 tests (90 optional checks skipped); 16 focused packing tests and
the full bank corpus pass. Strict Clippy, Rust 1.88, formatting and scoped links
pass. Logs use `out/audio-migration/pitch-pack-*`; the corpus run took 149 seconds.
Bends/controller equivalence, broader instrument reconstruction, complete music
coverage, full runtime and independent audio comparison remain open. This does
not close A3.02/A3.03 or authorize C retirement.

SoundFont sample-assignment checkpoint (2026-09-24):

[Uniform per-tone reassignment](../specs/formats/audio.md#rust-soundfont-sample-assignment-packing)
now selects existing PCM allocations through regenerated source-qualified SF2
bindings. Only the reference low byte changes; opaque high bytes and unchanged
zero aliases survive. Per-key splitting, generated silence, silent-tone unmuting
and the noise selector reject explicitly. The sample inverse checks shared loop
agreement and any actual PCM/loop edits; losing all tone users does not discard
an original sample. Gain, pitch, reference and sample changes are validated before
publication, including conflicts between selected songs sharing a bank.

All 842 constructible banks preserve unchanged VH/VB bytes. Reassignment in each
of 644 eligible banks reproduces the edited SF2 exactly through fresh binding and
passes RustySynth parsing while preserving the entire body; 198 banks have no
eligible alternative under this probe. The 178 existing conversion failures
remain. Four new default checks cover reference encoding, layered/combined edits,
unused loops, conflicts and unsupported targets. Original-US loader/note-on
coverage now includes 22 reference/prefix cases, including signed high-byte
encodings used by reassignment. Nineteen focused tests, the expanded original
reference check and full corpus pass; the default suite passes 270 tests with
90 optional checks skipped. Strict Clippy, Rust 1.88, formatting and scoped links
pass. Logs use `out/audio-migration/assignment-pack-*`; the corpus took 168 seconds.
An initial malformed loop setup in the per-key test was corrected to exercise the
intended assignment diagnostic. Topology/envelope reconstruction, unmuting, full
runtime and independent audio fidelity remain open. A3.02/A3.03 and C retirement
are not accepted by this checkpoint.

SoundFont silent-tone unmute checkpoint (2026-09-24):
[Explicit unmuting](../specs/formats/audio.md#rust-soundfont-silent-tone-unmuting)
jointly validates uniform relinking to existing PCM and exactly invertible
audible gain/pan. Relinking triggers the gain inverse even when SF2 gain
amounts remain unchanged. Bank/program mutes, gain-only edits retaining silence,
partial relinking, nonuniform controls and generated PCM/loop edits reject.
Returning to the original allocation preserves encoded zero aliases and opaque
reference bytes. The regenerated export removes unused synthetic silence only
after its last user is gone; real sample storage and source bodies survive.
Reports identify unmuted tones and retain gain candidates/approximation evidence.

All 842 constructible banks retain byte-exact unchanged VH/VB round trips.
Each of 332 eligible banks unmutes one tone with exact semantic re-export,
RustySynth parsing and unchanged body; 510 have no eligible muted tone under
this probe. Existing gain, pitch and assignment corpus checks still pass;
178 pitch-context conversion failures remain explicit. Four new default tests
cover both gain models, layered/last-user activation, aliases and rejection
paths. A shared-song integration test rejects conflicting requests without
publication and verifies agreed edits change only the bank header.

The default suite passes 274 tests with 91 optional checks skipped; 12 focused
packing tests, all six music-folder tests and the complete bank corpus pass.
Strict Clippy, Rust 1.88 and formatting pass. Logs use
`out/audio-migration/unmute-*`; the corpus run took 195 seconds. An initial
partial-relink fixture was corrected to use representable gain. The first
music-suite invocation omitted its corpus environment variable; rerunning with
both supplied-media paths passed. Topology/envelope reconstruction, complete
runtime execution and independent audio fidelity remain open. This checkpoint
does not close A3.02/A3.03 or authorize C retirement.

MIDI timing reconstruction checkpoint (2026-09-24):
[Timing reconstruction](../specs/formats/audio.md#midi-timing-reconstruction)
now maps requested cross-track execution ticks back to consumed source deltas.
Repeated visits must agree; finite jumps use the following delta and infinite
jumps force zero delay. Unobserved deltas, unchanged noncanonical encodings,
running statuses and opaque suffixes remain exact. Changed VLQ lengths resize
the selected SEP record while preserving other sequence records and container
padding. Reconstructed MIDI must reproduce all requested events/ticks; generated
offset markers follow relocated source positions and termination follows the
new boundary. Reports separate value edits, delta spans, data lengths and reuse.

All 476 corpus sequences still reconstruct into 119 byte-identical original SEP
entries. Tripled-tick edits change 240,915 deltas and resize 117 entries;
retranslation preserves requested events/ticks and independent RustySynth
durations triple. Fifteen retimed streams run through the original US delta
reader and controller dispatcher with matching requested ticks and relocated
cursors. These are bounded synthetic RAM probes, not full scheduler/audio traces.
Music-folder validation publishes a resized SEP while preserving its following
sequence, unrelated archive entries and existing output on failed republication.

Four new default checks cover resizing, loop constraints, cross-track ordering,
generated boundaries and signed scaling limits. The default suite passes 278
tests with 93 optional checks skipped; 16 focused/media tests, nine original-US
routine tests and the independent-duration corpus check pass. Strict Clippy,
Rust 1.88 and formatting pass. Logs use `out/audio-migration/timing-edit-*`.
Event insertion/removal, channel/loop-structure editing, broader instrument
reconstruction, complete runtime execution and independent audio fidelity remain
open. No aggregate acceptance or C retirement follows from this checkpoint.

## 4. [A4] (open) Rendering and verification
- Owner: parent
- Depends: A3, A1.04
- Blocker: none
- Evidence: none
- Acceptance: actual game audio paths execute in Rust with independent trace/audio evidence; PC playback and delivery codecs verified; evidence categories reported separately

System-control checkpoint (2026-09-24): Rust COP0 status/cause transfers, RFE,
explicit exception delivery and instruction-boundary interrupt admission are
implemented. BIOS critical-section syscall handling executes the original US
entry/exit wrappers. Five default checks and one optional original-wrapper check
cover state preservation, load delay, branch-delay exception evidence, masks and
unsupported cases. The default suite passes 180 tests (74 optional checks skipped);
28 focused CPU/device/COP0 checks pass, with strict Clippy and Rust 1.88 checks.
Evidence uses `out/audio-migration/cop0-*` and the
[system-control specification](../specs/formats/audio.md#rust-system-control-and-critical-sections).
CD-driver removal `A0:72` still requires event and exception-chain state; no
success stub was introduced. Cache/user-mode execution, general BIOS handlers,
runtime scheduling/timing and full audio fidelity remain open. This prerequisite
does not satisfy A4 or retire C.

BIOS-event checkpoint (2026-09-24): event allocation, closure, enable/disable,
polling delivery/consumption and free-slot queries now use guest EvCB RAM.
WaitEvent yields a pending result and retains its original record pointer until
readiness; it does not report success on busy state. At this checkpoint callback
execution, invalid table/handle contexts and nested waits failed explicitly.
Original US event wrappers
execute with an explicitly supplied synthetic table, and five default tests plus
one original-wrapper check pass. BOF3's supplied `SYSTEM.CNF` specifies 22 event
slots; no original BIOS table address/contents are inferred from that count.
See [event-state evidence](../specs/formats/audio.md#rust-bios-event-state).
CD-driver removal still requires initial BIOS event and exception-chain evidence;
full initialization, timed scheduling and audio fidelity remain open.
The default suite passes 185 tests (75 optional checks skipped), and 19 focused
event/device/COP0 checks pass. Strict Clippy, Rust 1.88, formatting, plan parsing
and scoped reference checks pass. Logs use `out/audio-migration/kernel-events-*`.

Callback checkpoint (2026-09-24): DeliverEvent now yields non-null handlers to
the guest CPU loop and resumes its scan after each return, observing changes to
later records. A bounded continuation stack supports nested delivery and waits
inside callbacks, preserves the caller's stack and checks the callback ABI.
This supersedes the preceding callback-rejection boundary. Four default tests
and one optional original-executable check cover ordering, live table edits,
descriptor rebinding, stack isolation, nesting, waits and explicit failures.
The original sequence callback-registration routine runs in a synthetic BIOS
event context; actual game BIOS registration and timing are not inferred.
See [callback evidence and limits](../specs/formats/audio.md#rust-bios-event-callbacks).
The default suite passes 189 tests (76 optional checks skipped), and 24 focused
event/callback/device/COP0 checks pass. Strict Clippy, Rust 1.88 and formatting
pass; logs use `out/audio-migration/event-callback-*`. Initial BIOS state,
CD-driver removal, exception-chain dispatch, nonlocal callback completion,
timed scheduling and independent audio fidelity remain open. A4.01 remains
unaccepted and the host C tool remains in place.

Thread-context checkpoint (2026-09-24):

[Guest PCB/TCB exception contexts](../specs/formats/audio.md#rust-bios-thread-exception-contexts)
now support capture after architectural exception entry and BIOS `B0:17` return.
The implementation validates guest allocation descriptors, preserves opaque
fields, restores live saved registers/HI/LO/status/PC and retains diagnostic
cause semantics. Branch-delay return, pending loads, aliases and saved-state edits
are checked; malformed contexts, read failures, COP2 PC-adjustment cases and
outstanding event/wait continuations reject explicitly. The original US
`0x8017EDDC` wrapper executes three original instructions into the service and
returns to an explicit synthetic thread context. Primary research is retained in
the PSX kernel reference; this does not establish original BIOS boot contents.

Twenty-eight focused CPU/kernel checks pass with local media; the default suite
passes 256 tests with 88 opt-in cases skipped. Strict Clippy and Rust 1.88
all-target checks pass. Evidence uses `out/audio-migration/thread-context-*`.
The fresh full-initializer probe still stops at `A0:72` after 4,550 instructions.
Initial BIOS events/driver state, exception-chain dispatch, nested continuation
handling, scheduler timing and independent audio fidelity remain required; A4.01
and C retirement are not accepted by this checkpoint.

1. [A4.01] (open) Complete original-game PSX audio execution
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: none
- Acceptance: representative music/SFX/vocal runtime traces and reference audio validated independently; unsupported instructions/devices/runtime paths fail explicitly

Complete Rust R3000, scheduler, DMA/IRQ, SPU and CD/XA behavior required by actual
audio paths. Bootstrap the original bank/sequence tables and execute the game's
sound runtime using the verified executable/media. Render bank samples with
explicit playback context; report defaults when no unique game context exists.

Bounded execution groundwork now includes Rust SPU RAM/FIFO transfers, request-mode
DMA4, normal DEV4 bus settings and interconnect/IRQ routing. Seven synthetic tests
plus an opt-in original-executable test pass. The original US dispatcher and its
callees perform 16 upload/download pairs (including 64-byte rounding and RAM wrap)
without code substitution; its timeout also returns without launching DMA.
See [transfer evidence and limits](../specs/formats/audio.md#rust-spu-ram-transfers-and-dma4).
The device uses explicit transaction servicing, not hardware timing. IRQ9, voice
integration, full boot, actual interrupt handlers and reference-audio acceptance
remain required; this component work does not close A4.01.

The transfer checkpoint passes 108 default tests with 47 opt-in cases skipped
(`out/audio-migration/spu-transfer-default-tests.log`); the eight focused transfer
tests pass with the original executable (`out/audio-migration/spu-transfer-tests.log`).
Strict all-target Clippy, Rust 1.88 all-target checks, formatting, scoped reference
checks, plan parsing and whitespace checks pass. The full proprietary-media
export/round-trip workflow was not rerun for this machine component addition.

Voice checkpoint (2026-09-24): Rust sample readers now connect shared SPU RAM to
24 voice envelopes and fixed stereo gains, with repeat-address handling,
Gaussian interpolation, pitch modulation, key/end flags and explicit MMIO/frame
boundaries. Reverb sends are retained for the future mixer. Published and emulator
sources disagree on interpolation rounding and the pitch ceiling, so callers
must select a named model; no hardware acceptance is inferred. Nine new default
checks and two optional media checks cover arithmetic, register behavior, sample
progression and the original US final flush block across all 24 voice bits.
BGM000/BGM004 exercise 37 samples, 10 repeated loops and 808,388 voice frames.
See [voice implementation and evidence limits](../specs/formats/audio.md#rust-spu-sample-and-voice-execution).
The default suite passes 198 tests (78 optional checks skipped); 19 focused
sample/voice/transfer checks pass. Logs use `out/audio-migration/spu-voice-*`.
Main mixing/gain, noise, sweeps, reverb, capture, CD input, hardware key/fetch timing,
IRQ9 and whole-runtime/reference-audio acceptance remain required. This advances
A4.01 without closing it or authorizing C removal.

Noise/mixer checkpoint (2026-09-24): the published shared noise generator now
feeds all 24 voices independently of pitch, while retaining ADPCM address/ENDX
progression. Fixed main/input gains, voice mute and separate reverb sends/returns
are modeled. The explicit output-frame API mixes supported dry paths and writes
CD/voice capture rings; enabled reverb or an audible tail rejects before execution.
Ten new default checks cover all 64 noise rates against a separate integer oracle,
mixing order, signed gains, input routing, capture wrap and MMIO integration.
See [noise/mixer implementation and limits](../specs/formats/audio.md#rust-spu-noise-mixing-and-capture).
The default suite passes 208 tests (78 optional checks skipped); 29 focused
noise/mixer/output/sample/voice/transfer checks pass, including original-media
sample traversal and unchanged US register-flush/transfer instructions. Strict
Clippy and Rust 1.88 all-target checks pass. Logs use
`out/audio-migration/spu-noise-*`. Noise phase and fractional-timer differences,
the final negative-gain rail, sweeps, reverb, CD processing, timing/IRQ9 and
whole-runtime reference-audio validation remain open. This supersedes the prior
checkpoint's missing noise/mixing/capture components, not its acceptance gates;
A4.01 remains open and C remains required.

Sweep checkpoint (2026-09-24): main and voice volume sweeps now retain independent
gain/counter state and share the explicit ADSR arithmetic models. Current voice
gain reads are mapped; unverified writes reject. Six new checks cover signed and
exponential ramps, all eight modes/128 rates/five starting levels, register state,
voice/main output ordering and key preservation. The default suite passes 214
tests (78 optional checks skipped); 42 focused envelope/SPU checks pass, including
the original-media cases. ADSR corpus results remain unchanged after sharing the
rate calculation. Strict Clippy and Rust 1.88 all-target checks pass; evidence is
under `out/audio-migration/spu-sweep-*`. See
[implementation and retained timing differences](../specs/formats/audio.md#rust-spu-volume-sweeps).
Sweep write latency, inactive-voice clocking and arithmetic differences still
require hardware evidence. This removes the blanket sweep rejection without
closing A4.01, accepting final audio fidelity or permitting C retirement.

Reverb checkpoint (2026-09-24): an explicitly selected emulator arithmetic model
now connects the stereo reverb network, published 39-tap resampling, ESA/preset
MMIO and shared SPU RAM to the final mixer. Disabled writes preserve stored output
and filter/cursor progression. Seven new default checks and one optional US
preset-identity check cover signed arithmetic, reflection routing, alias ordering,
FIR gain/phase, wrap/history, disabled output and FIFO-to-mixer integration.
The Room vector matches a separate scalar calculation for 88,200 frames and final
RAM; its 32 preset values also match the supplied executable exactly. See
[reverb evidence and unresolved differences](../specs/formats/audio.md#rust-spu-reverb-execution).
The default suite passes 221 tests (79 optional checks skipped); all 50 focused
envelope/SPU checks pass. Strict Clippy and Rust 1.88 all-target checks pass; logs
use `out/audio-migration/spu-reverb-*`. Initial literal-vector assumptions about
unity FIR gain and independent aliased channel state were corrected from numeric
coefficients and operation ordering before acceptance of these component checks.
Hardware address folding, rounding, phase/timing, IRQ9, actual preset selection
and independent original-game audio remain open. A4.01 and C retirement remain
unaccepted.

CD sample-processing checkpoint (2026-09-24): selected XA sectors now feed an
explicit resampling model and separate drive volume matrix before SPU input.
History/phase survives chunks and sectors; both rates and all eight channel/depth
coding combinations are exercised. Published and emulator alignment remain named
choices; only the emulator reference provides the implemented half-rate filter.
Five new default checks and one optional original-prefix check cover signed
vectors, chunk equivalence, gain staging/mute/failures and XA-to-SPU capture.
The VOICE prefix supplies 16 selected sectors / 75,264 frames, not complete cue
execution. See [processing evidence and limits](../specs/formats/audio.md#rust-cdxa-sample-processing)
and [retained external sources](../reference/cd-audio.md).
The default suite passes 226 tests (80 optional checks skipped); 19 focused
CD/XA/output checks pass, including the existing independent FFmpeg decoder check.
Strict Clippy and Rust 1.88 all-target checks pass. Logs use
`out/audio-migration/cd-audio-*`. No extraction-rate change or new dependency was
introduced. Drive commands/MMIO, FIFO and timed sector delivery, seek/filter/mute
state, emphasis, large-gain saturation and hardware fidelity remain open; this
does not close A4.01 or permit C retirement.

SPU initialization checkpoint (2026-09-24): the original low-level US initializer
now executes against the connected register/FIFO/voice components. Cold, warm
and repeated-cold calls preserve the stack/saved registers and verify dummy
sample upload, all 24 voice settings and the warm path's selective preservation.
A separately selected emulator-reference disable transition clears active
envelopes while retaining inactive state and pending keys; rejected control writes
leave the device unchanged. See [execution evidence and limits](../specs/formats/audio.md#rust-spu-disable-and-original-hardware-initialization)
and [external provenance](../reference/spu-samples.md#disable-transition).
The default suite passes 227 tests (81 optional checks skipped); 34 focused
checks pass, including original initializer/flush/transfer execution and the
ADSR corpus. Strict Clippy, Rust 1.88, formatting, plan parsing and scoped links
pass. The focused corpus invocation initially omitted its media environment
variable and passed after supplying the existing absolute corpus path. This
direct callee check uses untimed servicing, without voice-frame scheduling.
Full startup still stops at BIOS CD-driver removal; disabled-frame execution,
timing, IRQ9 and independent runtime/audio acceptance remain open. Logs use
`out/audio-migration/spu-bootstrap-*`. A4.01 remains open and C remains required.
An initial assumption that the dummy block encoded zero PCM failed an added
decode check; its original nibbles instead establish alternating `224, 0`.
The check and specification now retain that distinction from zero voice gain.

Exception-chain checkpoint (2026-09-24): C0 enqueue and exact-head removal now
use guest ExCB RAM, preserving aliases, opaque words and detached links. Invalid
graphs and unresolved non-head removal reject explicitly; the documented BIOS
bug is not replaced with a corrected list search. Four default checks and an
original-executable check cover registration and the game's priority-1 removal
caller in a synthetic table context. See [implementation and provenance](../specs/formats/audio.md#rust-bios-exception-chain-registration).
The default suite passes 231 tests (82 optional checks skipped); 29 focused
kernel/COP0 checks pass, with strict Clippy, Rust 1.88 and formatting. Logs use
`out/audio-migration/exception-chain-*`. Initial BIOS allocation/driver state,
non-head stack effects, exception dispatch, full startup and independent audio
remain open. A0:72 still rejects; this does not close A4.01 or permit C retirement.

Priority-chain execution checkpoint (2026-09-24): the bounded Rust dispatcher
captures thread context, executes guest verifier/second handlers in priority
order, observes live next links and supports hook exit or explicit functional
return. Early `B0:17` discards only continuations owned by that exception after
successful restoration. Synthetic ordering/error checks and original US
VBlank/SIO handlers pass; the latter use explicit device inputs and a synthetic
chain. See [scope and evidence](../specs/formats/audio.md#rust-bios-priority-chain-execution).
The default suite passes 261 tests (89 optional checks skipped), and 34 focused
kernel/COP0 checks pass including original executable checks. Strict Clippy,
Rust 1.88 and formatting pass; logs use `out/audio-migration/exception-dispatch-*`.
The initial focused command named two nonexistent test targets; the corrected
invocation passed. BIOS boot contents, CD-driver removal, timed dispatch, nested
exceptions, full startup and independent audio comparison remain open. A4.01
remains open and C remains required.

BIOS ROM checkpoint (2026-09-24): immutable 512 KiB ROM mapping, explicit raw RAM
input and a bounded guest-only BIOS probe are implemented. Four default checks
and eight probe cases validate mapping, input rejection, RAM/ROM calls, syscall
entry/RFE, limits and unsupported MMIO. The original game's CD-removal caller and
following critical-section exit were inspected; BIOS-owned state and non-head
removal effects remain unresolved. See [probe contract and evidence](../specs/formats/audio.md#rust-bios-rom-execution-probe).
The default suite passes 265 tests (89 optional checks skipped); the focused
CPU/device/firmware suite passes 31 tests (one optional original-wrapper check
skipped). Strict Clippy and Rust 1.88 all-target checks pass. Logs use
`out/audio-migration/firmware-*` and `cd-removal-*`. No BIOS was found in the
inspected project input/media/toolchain paths; a local BIOS or independent runtime
capture was requested. Synthetic probe success is not BIOS boot, full audio
execution or fidelity acceptance. A4.01 and C retirement remain open.

2. [A4.02] (open) Implement PC synthesis and delivery formats
- Owner: parent
- Depends: A4.01
- Blocker: none
- Evidence: none
- Acceptance: Rust MIDI/SF2 synthesis, WAV inspection and conformant MP3/Ogg outputs tested; PSX/PC differences measured before lossy encoding

Use the selected Rust SoundFont engine for exported MIDI/SF2. Produce WAV for
inspection and MP3/Ogg Vorbis for delivery. Support explicit duration and loop
count; default to two loop traversals, a release tail and a reported safety
timeout. Validate timing including encoder delay and document synthesis/loop
approximations. Never claim fidelity from comparison to the approximate C path.

The new `render --mode music --engine pc --midi FILE --soundfont FILE --output
NEW_DIR` path produces verified WAV/report output with RustySynth 1.3.6. It validates
supported MIDI playback, rejects missing presets instead of substituting timbres,
preserves rational tempo timing/order, and reports clipping, duration limits and
engine approximations. Whole-file repeats are explicitly distinct from game loop
traversals. See [PC rendering contracts and evidence](../specs/formats/audio.md#rust-pc-interchange-rendering).
Six default tests and an opt-in FFmpeg WAV-decoding comparison pass
(`out/audio-migration/pc-render-tests.log`). Archive-driven rendering, BOF3 loop
policies, translation acceptance, PSX comparison and delivery encoders remain
unfinished; this does not close A4.02 or authorize C retirement.

The PC rendering checkpoint passes 114 default tests with 48 opt-in cases skipped
(`out/audio-migration/pc-render-default-tests.log`), seven focused cases including
independent WAV decoding, strict all-target Clippy, Rust 1.88 all-target checks,
formatting, scoped references, plan parsing and whitespace checks. Proprietary-media
extraction/round-trip workflows were not rerun for this interchange renderer.

PC archive/loop checkpoint (2026-09-24): `render --engine pc --archive` now
shares music extraction's MIDI/SF2 preparation and accepts one independent SEP
index with explicit `--allow-approximations`. Default playback expands two
infinite-loop traversals, keeps the intro once and preserves finite loop counts.
Fixed duration expands enough source loops; the translated MIDI is synthesized
once. Generic MIDI playback now defaults to one file play, avoiding doubled
exported loops; explicit whole-file restarts remain available. Extra fixed body
time after the requested file plays advances final synth state without restarting.
Tests compare archive output byte-for-byte with exported MIDI/SF2 playback,
exercise independent selection and finite/infinite loops, and verify publication.

The full BGM probe attempts 324 physical sequence slots: 31 succeed, 292 reject
unresolved pitch-table contexts and one rejects a missing preset. Among the 165
verified gameplay selections, 11 succeed. This exposes existing translation
gates rather than bypassing complete-bank validation. A retained two-loop BGM004
comparison measures PC body duration 167 frames shorter than the Rust PSX
runtime and different output amplitude; it is not independent hardware evidence.
See [contracts and measurements](../specs/formats/audio.md#direct-pc-archive-rendering)
and `out/audio-migration/pc-archive-*` / `pc-psx-bgm004-comparison.json`.
Validation passes: 345 default Rust tests (122 ignored), 23 focused integration
checks including independent FFmpeg WAV decoding, 15 harness/package checks,
Clippy, Rust 1.88 all-target checks and formatting. Runtime fidelity, unresolved
bank/game contexts, delivery codecs and full acceptance remain open.

Program initialization and population checkpoint (2026-09-24): original US SEP
initialization assigns program by channel index, pan 64 and volume 127. Explicit
silent presets preserve empty-program playback; SF2 preset assignments can now
populate those programs from 1–16 existing tone instruments, including layers.
Packing rebuilds ordered VAB blocks and validates alias agreement, source program
controls and EMI/game layout capacity. New independent instruments/samples remain
open. Current XML policies are mandatory; no legacy export branch remains.
See [runtime, editing contracts and evidence](../specs/formats/audio.md#initial-channel-programs-and-editable-empty-programs).

The 324-slot PC probe now succeeds for 32 selections; BGM053 sequence 0 is the
added success and all 31 previous short WAVs are byte-identical. Remaining 292
pitch-context failures and the 11/165 gameplay-selection success count are
unchanged. Default checks pass 349 tests with 125 media/consumer cases ignored;
focused integration proves unchanged whole-EMI equality, original-runtime silence
and populated-program playback, while layered SF2 re-export reproduces PC PCM.
The 15 harness/package checks pass. Receipts use
`out/audio-migration/program-population-*` and `program-init-pc-comparison.json`.
This checkpoint does not close A3/A4 acceptance or authorize C retirement.

3. [A4.03] (open) Implement verification reports and corpus coverage
- Owner: parent
- Depends: A4.01, A4.02
- Blocker: none
- Evidence: none
- Acceptance: versioned verify reports separate structural validity, identities, byte equality, translation differences, encoding error and rendering evidence

Exercise edited samples, layered instruments, program changes, tempo, bends,
sample/sequence loops, independent sequence selection, shared banks and XA
channels. Test malformed input, ambiguous IDs, manifest conflicts, unsupported
events, capacity failures and failed publication. Missing references or unresolved
runtime/mapping/codec/corpus failures block the corresponding completion claim.

CD decoder host-register checkpoint (2026-09-24):
[The banked host interface](../specs/formats/audio.md#rust-cd-decoder-host-registers)
now connects command/parameter and result FIFOs, separate response-readiness and
interrupt-delivery boundaries, mask/acknowledgement, IRQ2 and existing drive-volume
ports to the interconnect. Configuration explicitly supplies initial volume and
empty host state. A separate drive owner must accept commands and produce
responses; no success stub, disc-status inference or timing fallback is added.
Overlapping/undrained requests and unsupported data/reset/manual-XA paths reject.

The original US command routine submits Getstat, Setloc, Setmode and Setfilter
with matching parameter bytes. Original response handling drains and acknowledges
explicit responses. A prior completion is supplied through that handler to
establish SDK idle state. Initial probing exposed VSync(-1)'s unconditional GPU
status read and the lack of a prior completion; the isolated fixture now declares
both inputs rather than bypassing instructions or patching a completion global.
These checks are not whole-driver, GPU-timing or BIOS-bootstrap acceptance.

Six new default tests cover FIFO/status transitions, partial acknowledgements,
delayed IRQ delivery, edge relatching, volume routing and unsupported accesses.
Twelve focused host/original-driver/CD-audio tests pass. The default suite passes
284 tests with 94 optional checks skipped; strict Clippy, Rust 1.88 and formatting
pass. Research is retained in [CD references](../reference/cd-audio.md#cd-decoder-host-interface);
logs and bounded original instruction listings use `out/audio-migration/cd-host-*`.
Fresh startup still stops at `A0:72` after 4,550 instructions. Drive semantics,
sector FIFO/DMA3, response scheduling, original BIOS state and independent audio
fidelity remain open. A4 acceptance and C retirement remain gated.

CD sector FIFO/DMA3 checkpoint (2026-09-24):

[The explicit data/DMA3 path](../specs/formats/audio.md#rust-cd-sector-data-and-dma3)
now executes the original US `CdGetSector` transfer routine for both supported
block sizes, whole and split transfers, with three service budgets: 12 synthetic
cases. Sixteen supplied VOICE data fields also copy exactly, totalling 37,440
bytes. Six new default tests cover cursor/request transitions, transfer counts,
address aliases/wrap, priority gating, IRQ acknowledgement and failed transfers.
This advances the earlier host-register checkpoint's sector FIFO/DMA3 obligation;
drive command execution, sector arrival, bus timing and CPU stalls remain open.
The selected FIFO model is explicit emulator evidence, and conflicting overread
descriptions cause rejection instead of invented bytes. Original BIOS startup
and independent audio fidelity remain acceptance gates. Logs and original
instruction listings use `out/audio-migration/cd-sector-*`. The full default suite
passes 290 tests with 96 optional tests skipped; eight focused sector tests and
the existing original host-driver check pass. No A4 gate or C retirement is closed.

Correction to this checkpoint: its initial VOICE test incorrectly divided an
extracted 2,336-byte-sector STR into 2,352-byte chunks. That run proved arbitrary
block copying, not original sector boundaries. The subsequent corrected test
reads 16 raw sectors from `BOF3_AUDIO_TRACK`, checks each BCD header against LBA,
and reproduces all 37,440 data-field bytes through original `CdGetSector`.

CD command/sector-routing checkpoint (2026-09-24):

[The functional drive](../specs/formats/audio.md#rust-cd-drive-commands-and-sector-routing)
now supplies command-derived responses and sector routing for explicit ready
disc state. Setloc, mode/filter, SeekL/SeekP, ReadN/ReadS, Getstat/Getparam/GetlocL
and Pause have stateful behaviour; mechanical completion and host IRQ boundaries
remain explicit. Unsupported commands, mode bits, XA auto-selection, overlaps,
bad sectors and mismatched positions reject without inventing execution.

The original US SDK writer and response handler execute the game's observed
`0xC8` mode, SeekP, filter and ReadS command pattern. A sixteen-sector raw VOICE
prefix contains one selected XA, four filtered audio and eleven ordinary data
sectors; original INT1 handling and DMA copy those eleven data blocks exactly.
GetlocL follows every latest header. A 256-sector check preserves 16 selected
XA sectors and 75,264 resampled frames against direct same-model decoding,
filters 64 audio sectors and retains 176 data sectors. The first test assumption
that VOICE held only audio was corrected after raw-disc evidence exposed the
ordinary data sectors. These are routing and original SDK checks, not complete
game scheduler or independent hardware playback acceptance.

Five new default checks pass; the full suite passes 295 tests with 98 optional
tests skipped. Nine focused command/original-runtime/corrected-sector checks pass;
strict Clippy and Rust 1.88 checks pass. Logs and bounded original instruction
listings use `out/audio-migration/cd-drive-*`. Full initialization, drive queues,
mechanical/IRQ timing, BIOS startup and independent audio evidence remain open.
All phase acceptance gates and host C retirement remain unchanged.

Original XA scheduler/drive checkpoint (2026-09-24):

[The integrated execution fixture](../specs/formats/audio.md#original-xa-scheduler-with-drive-responses)
now runs original cue initialization, game state handlers, SDK commands and
callbacks through reading, GetlocL threshold detection, fade-out, Pause and idle.
The missing callback link was the SDK outer interrupt wrapper at US `0x80177264`;
the earlier low-level handler alone drained responses but left game state 2
waiting for completion. No game state/completion globals are patched.

Original cue `0x2000` setup produces mode `0xC8`, Setloc/SeekP, filter `(1,0)`,
Setloc/ReadS. Two explicit input schedules both traverse `5 → 6 → 7 → 0`, stop
after the reported position reaches the threshold and leave the drive paused
with its volume matrix zero. One sector per scheduler call processes 992 sectors
and 62 selected XA sectors; four per call processes 1,008 and 63. The respective
291,648 / 296,352 resampled frames are functional decode counts, not verified
audible durations or physical clock measurements.

The original game acknowledges ordinary data without requesting it. A serialized
INT1 delivery API now atomically selects new data and publishes the response,
reports displacement of an unrequested prior block and protects active reads.
Pending host work, missing configuration and invalid blocks fail without
replacement. Direct presentation remains strict. Multi-sector queues, overrun
and retry timing are still unimplemented.

Four new default delivery tests and two original-runtime tests pass. The full
suite passes 299 tests with 100 optional tests skipped; strict Clippy and Rust
1.88 checks pass. Evidence and bounded listings use `out/audio-migration/xa-loop-*`.
The fixture explicitly supplies ready host/drive state, prior SDK completion,
GPU status and guest handler-call boundaries; it does not execute BIOS exception
entry/save/restore. Full bootstrap, clock integration, audio queue timing and
independent reference PCM remain open. No acceptance gate or C retirement closes.

XA queue/SPU checkpoint (2026-09-24):

[The queue/consumption path](../specs/formats/audio.md#rust-xa-audio-queue-and-spu-consumption)
now connects selected sector decoding to SPU output frames. The explicit
emulator-reference queue drops arrivals above ten buffered frames without
committing histories, distinguishes muted admission from buffered output, and
applies current drive-matrix gain at consumption. Empty frames and dropped/muted
sectors are reported separately. Malformed input preserves queue/history state;
failed SPU output retains the queued frame without claiming whole-SPU rollback.

Five default tests cover admission boundaries, predictor/interpolator history,
mute/volume ordering, capture and failures. A 256-sector raw VOICE check consumes
75,264 frames through SPU gain/capture with no drops or underruns at a supplied
steady phase. The original cue scheduler fixture now consumes queued audio
through SPU as well: its two input schedules retain their 291,648 / 296,352 frame
counts, nonzero output, idle completion and final zero drive matrix, without
queue loss. Dry SPU state and frame/sector boundaries remain explicit inputs.

The full suite passes 304 tests with 101 optional tests skipped; six focused
queue/media checks and two original-scheduler checks pass, alongside strict
Clippy and Rust 1.88. Logs use `out/audio-migration/cd-queue-*`. Physical queue
lifecycle, seek/reset/command-mute behaviour, original runtime bootstrap,
CPU/device clock integration and independent reference audio remain open.
No phase acceptance or host C retirement is claimed.

CD lifecycle/restart checkpoint (2026-09-24):

[Command application](../specs/formats/audio.md#rust-cd-command-lifecycle-and-cue-restart)
now couples drive transitions to decoder/data-buffer resets and command mute.
Setloc and continuing reads preserve buffers; seek/read startup clears both,
Pause clears audio, and Mute/Demute remains independent of ADPCTL mute. Reset
reports discarded frames/bytes, preserves cumulative statistics and permits the
next validated stream/format to bind with fresh history. Invalid commands and
seek during active DMA3 preserve prior state and fail explicitly.

Five new default checks and an original cue restart pass. The latter runs
`0x2000 → 0x2001` through original game/SDK callbacks with 4,704 pending frames
and an unrequested raw-disc data block. SeekP and located ReadS clear stale
buffers; all output from the next selected channel-1 block matches fresh
same-model decoding through SPU. Both existing full-cue schedules still finish
in idle with unchanged frame counts and final zero drive matrix. No game state
or completion flag is patched, and these remain explicit-boundary fixtures.

Evidence uses `out/audio-migration/cd-lifecycle-*`. EOF-based stream rebinding,
general overlaps/DMA cancellation, original runtime bootstrap, hardware timing
and independent PCM remain open. No phase acceptance or C retirement closes.

XA selection/EOF checkpoint (2026-09-24):

[Drive selection and queue rebinding](../specs/formats/audio.md#rust-xa-selection-and-eof)
now distinguish selection release from decoder reset. Automatic selection,
explicit channel-255 filtering, matching EOF and Setfilter are implemented;
same-coding handoffs preserve predictors, interpolation history and queued
output. Invalid sectors preserve queue state. Coding changes still fail
explicitly. Five new default checks and a full 3,536-sector raw VOICE check pass;
VOICE yields 71 selected channel-0 sectors, one EOF and 333,984 frames matching
continuous same-model decoding. Cross-channel handoffs are tested synthetically,
since that corpus has none after the selected EOF. All 27 focused checks pass,
including original SDK/scheduler/cue-restart paths. The initial corpus probe's
incorrect handoff expectation and corrected results remain in
`out/audio-migration/cd-selection-*`. General coding transitions, physical timing,
runtime bootstrap and independent PCM remain open; no phase or C-retirement
acceptance is claimed.

XA coding-transition checkpoint (2026-09-24):

[Runtime coding transitions](../specs/formats/audio.md#rust-xa-coding-transitions)
now preserve decoder and interpolation history across all eight supported
mono/stereo, rate and bit-depth formats under the emulator-reference model.
Mono retains right-channel history; mute/drop retain their distinct admission
effects. File/channel changes still require selection release. Three new checks
cover 192 separately generated vectors, rollback, unsupported-model boundaries
and identity release. Existing fixed-format, raw-media and original scheduler
checks pass. The retained Python reference shares numeric coefficients but
implements separate state/arithmetic; its corrected initial tap-direction probe
and subsequent checks are recorded in `out/audio-migration/xa-coding-*`.
Emphasis, physical timing, runtime bootstrap and independent game PCM remain
open. This does not accept A4 or authorize C retirement.

Original CD interrupt-hook checkpoint (2026-09-24):

[COP0/SDK callback integration](../specs/formats/audio.md#original-sdk-cd-interrupt-hook-integration)
now passes the full VOICE cue under both service schedules using the original
installed SDK hook and IRQ2 registration, with the existing BIOS context bridge.
The fixture checks 963/792 exception entries and returns, restored foreground
registers/HI/LO/EPC/SR and cleared controller interrupts. Prior cue state, sector,
audio-frame and Pause assertions remain unchanged. Original callback setup
reaches `A0:72` after 4,542 instructions; the fixture retains its installed hook
but does not continue initialization past that unsupported call. Empty ExCB
and live PCB/TCB RAM are explicitly supplied, not recovered BIOS allocations.
Evidence is retained under `out/audio-migration/cd-irq-*`.
The user confirmed no BIOS dump is available. BIOS removal/boot state, clocks,
complete sound bootstrap and independent PCM remain unresolved. No phase or
C-retirement gate closes from this integration check.

By this checkpoint a local BIOS dump had been prepared. Its `ps-30a`
SHA-256 is `11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef`.
A bounded `bios_probe` using PCSX-Redux's reset PC/status rejected status
`0x10900000` before execution because the current COP0 mask drops bits; context
and diagnostics are retained in `out/audio-migration/sony-reset-*`. No altered
status or skipped BIOS call was used to claim boot success. Resolve reset-state
semantics, then bootstrap/timing and independent audio evidence remain A4 work.

BIOS reset/memory checkpoint (2026-09-24): the pinned PCSX-Redux SR seed includes
reserved bit 23. A named reference-reset constructor preserves that exact seed;
ordinary guest SR writes retain their mask. Probe contexts reject conflicting
reset inputs rather than silently altering them. The original US BIOS now
executes 87 instructions through supported memory-controller setup and register
clearing. ROM/RAM/expansion bus configuration latches, 8 MiB RAM aperture mirroring
over 2 MiB physical RAM, and BIOS common/CD delay values are covered by synthetic
checks and a hash-gated original-ROM check. The next boundary is cache control:
PC `0xBFC00234` writes `0x804` to `0xFFFE0130`. Cache isolation/clearing, complete
kernel boot, device clocks and reference audio remain A4 work. No boot or
rendering acceptance gate closes; see [reset evidence](../reference/psx-kernel.md#reference-reset-and-original-bios-memory-setup)
and `out/audio-migration/redux-*`.

BIOS cache checkpoint (2026-09-24): functional instruction fetch/cache state now
supports the original tag/code clearing loops, physical tags and aliases,
per-word validity/refill, stale cached instructions after ordinary RAM writes,
uncached bypass and scratchpad gating. Isolated CPU accesses do not corrupt RAM
or redirect host/DMA accesses. Unsupported control modes and partial cache
accesses fail. A dirty-cache/nonzero-RAM fixture executes 1,528 original BIOS
instructions and proves clearing without RAM modification. Disabled COP0 debug
address/mask state and the BIOS's already-zero TAR initialization are supported;
active breakpoints and other TAR writes remain rejected.

The original prefix now reaches 17,378 instructions and stops at a POST byte
store to `0x1F802041`, PC `0xBFC01A74` (branch delay slot). Full suite: 324
passed/106 ignored; 12 focused cache/firmware checks, including both original-ROM
fixtures, pass. Clippy and Rust 1.88 checks pass. Read
[cache evidence](../reference/psx-kernel.md#bios-cache-clearing-and-debug-initialization)
and `out/audio-migration/redux-cache-*`. The previous 87-instruction cache-control
boundary is superseded; full kernel boot, device timing, audio reference evidence
and C retirement remain open.

BIOS POST/EXP1 checkpoint (2026-09-24): the bus records retail POST byte
writes and models an absent cartridge only within the configured 512 KiB EXP1
window. The original ROM checks the missing signature and proceeds without
interception. A longer bounded probe reaches 2,727,265 instructions, one guest
syscall exception and POST `7`, then stops at RAM PC `0x8005429C` on SPU key-off
write `0x1F801D8C`: the probe must select SPU arithmetic models explicitly.
The hash-gated ROM fixture reproduces this boundary. Full suite: 326 passed,
106 ignored; 14 focused firmware/cache checks pass including both ROM fixtures;
Clippy and Rust 1.88 checks pass. See
[POST/EXP1 evidence](../reference/psx-kernel.md#bios-post-and-absent-expansion-rom)
and `out/audio-migration/redux-post-*`, `redux-exp1-*`. The POST stop is superseded;
full BIOS boot, scheduling, independent audio evidence and C retirement remain
open. No phase acceptance gate closes.

BIOS SPU checkpoint (2026-09-24): key-on/off readback latches now survive
frame consumption without retriggering keys. The probe explicitly selects SPU
arithmetic/disable/reverb models and an optional FIFO transfer clock. The latter
uses 16 system ticks per halfword and the probe's Redux two-base-tick instruction
model; it does not establish memory stalls, voice frames, DMA arbitration or
hardware timing. The bounded original-ROM run reaches 19,247,628 instructions,
four guest syscall exceptions and 54,328 transferred halfwords, then stops on
GPU-status read `0x1F801814` at `0x8005A4E8`. A hash-gated regression fixture
reproduces the boundary and SPU RAM snapshot. Full suite: 330 passed/107 ignored;
21 focused checks including original ROM/game fixtures and nine context checks
pass. Clippy and Rust 1.88 checks pass. See
[SPU boot evidence](../reference/psx-kernel.md#bios-spu-initialization-and-transfer-clock)
and `out/audio-migration/redux-spu-*`. Complete boot, device scheduling,
independent runtime/audio comparison and C retirement remain open. No phase
acceptance gate closes.

BIOS shell-load checkpoint (2026-09-24): inspection of pinned Redux confirms
its executable-loading handoff at `0x80030000`, independent of the FastBoot
option. Rust executes the exact US ROM to that boundary in 2,695,618 instructions,
then overlays only the verified US EXE payload and sets PC/SP while preserving
kernel RAM, GP and devices. No synthetic kernel tables or HLE service dispatcher
are used. The original callback initializer now returns after 5,281 instructions
(two syscalls), resolving `A0:72`. After adding transient ENDX write readback, the
original sound initializer returns after 67,615 instructions (four syscalls);
checks confirm sequence tables and tick mode. Exact ENDX overwrite timing remains
unverified. Original EXE startup separately reaches CD initialization at
`0x80176E80`; it still needs the CD host/drive scheduler. Full suite: 332 passed,
109 ignored; seven focused original-media/voice checks, Clippy and Rust 1.88 pass.
See [shell-load evidence](../reference/psx-kernel.md#bios-shell-handoff-and-original-sound-initialization)
and `out/audio-migration/redux-handoff-*`. The full-shell GPU stop is bypassed by
this documented EXE-loading contract, not treated as GPU implementation or full
disc-boot success. Bank/media integration, audio/device clocks, independent
runtime/PCM comparison and C retirement remain open; no phase gate closes.

Bank runtime checkpoint (2026-09-24): bounded guest execution now delivers
interrupts to the real BIOS and advances the explicit base instruction/transfer
clocks. Original VAB head, partial body transfer, completion wait and SEP open
routines succeed for BGM000, BGM021 and BGM068. SPU RAM matches archive bytes,
including SDK-rounded final DMA padding. The BGM000 media test also verifies
untouched surrounding SPU RAM, transfer results, guest IRQs and SEP handle zero.
Default suite: 334 passed/110 ignored; the new media test, Clippy and Rust 1.88
checks pass. Evidence and routine addresses are recorded in
[the kernel reference](../reference/psx-kernel.md#original-vab-transfer-and-sep-opening-through-the-bios)
and `out/audio-migration/bios-bank-*`. Buffers are host-staged: original CD
loading, sequence scheduling, timed voice frames and independent PCM fidelity
remain open. This checkpoint does not close A4 or permit C retirement.

Sequence IRQ checkpoint (2026-09-24): the original game start wrapper installs
the linked global sequence scheduler through the SDK VBlank callback and real
BIOS. Four archive/sequence selections each complete 120 injected VBlank IRQs,
advance event cursors and write SPU key-on registers. The BGM000 media regression
also stops sequence zero, selects sequence one independently, and checks all
inactive sequence records for byte equality. Default suite remains 334 passed /
110 ignored; the expanded media regression, Clippy and Rust 1.88 checks pass.
See [the kernel reference](../reference/psx-kernel.md#original-sequence-scheduler-through-vblank-interrupts)
and `out/audio-migration/sequence-bios-*`. These are injected IRQ iterations,
not timed playback: integrated video/SPU clocks, voice frames, loop/end controls
and independent runtime/PCM comparison remain required. A4 and C retirement
remain open.

Clocked PCM checkpoint (2026-09-24): an explicit Redux NTSC scanline model now
advances SPU output, FIFO transfer, root counters and VBlank on the guest's base
instruction clock. The original BIOS/kernel, sequence callback and voice code
produce PCM for four archive/sequence selections and two roughly 60-second
runs. The BIOS timer-1 HBlank dependency is serviced; unsupported timer-0
HBlank gating/dot-clock modes fail explicitly. ffprobe validates the generated
WAVs. Default suite: 336 passed/110 ignored; the expanded clocked media test,
Clippy, Rust 1.88 and formatting checks pass. See
[clocked output evidence](../reference/psx-kernel.md#clocked-spu-output-with-the-original-sequence-runtime)
and `out/audio-migration/output-clock-*`, `clocked-v3-*`, `clocked-long-*`.
The diagnostic uses a declared activation phase and base CPU costs. Independent
trace/PCM agreement, full device timing, broader media coverage, loop/end/tail
controls and production `render` integration remain open; no A4 completion or
C retirement is claimed.

PSX command checkpoint (2026-09-24): verified single-bank/SEP preparation now
belongs to the Rust library. `render --mode music --engine psx` accepts explicit
archive, executable, BIOS, sequence, duration and optional layout/tail/safety
limit, then publishes verified WAV and runtime evidence JSON. It records
reference-clock limitations and does not claim independent PCM agreement.
Original stop execution produces a measured release tail and exact requested
frame count. The final layout-zero corpus check passes 162/165 short selections;
the opening/ending selections exceeding its next-bank boundary pass with
explicit layout two, including longer checks. Original cue-specific layout
selection remains unverified. Default Rust tests: 338 passed/112 ignored;
four focused renderer/media tests, Clippy, Rust 1.88, the live harness command
and 15 Python audio/package checks pass. See
[PSX command evidence](../reference/psx-kernel.md#duration-limited-psx-music-render-command)
and `out/audio-migration/psx-render-*`. Explicit duration is required while
loop-count stopping/default two traversals remain unfinished. SFX/XA runtime,
independent fidelity, remaining conversion/corpus failures, codecs and complete
acceptance still block A4 completion and C retirement.

Automatic layout checkpoint (2026-09-24): rendering now selects the first
executable-derived layout whose VH, SEP and aligned VB capacities fit the bank
slot; no cue IDs or filenames influence selection. Explicit `--layout` remains
an override, and JSON records requested and resolved layouts. All 165 corpus
selections pass automatically (162 layout zero, three layout two). Five focused
renderer/media tests, the default suite (338 passed/113 ignored), Clippy and
Rust 1.88 pass. See `out/audio-migration/auto-layout-*.log` and
`psx-render-corpus-auto-layout/summary.json`. This resolves host layout selection;
original gameplay context and independent fidelity remain separate open gates.

Loop-render checkpoint (2026-09-24): PSX music now defaults to two observed
infinite-loop traversals or the first end marker, preserving encoded finite
repeat counts. `--loops N` and fixed `--duration` are mutually exclusive. A
read-only post-IRQ instruction observer records original loop/end handlers;
original stop executes at a safe call return with a measured release tail.
Timeouts and nonreturning callbacks fail without publication. Reduced SEP sets
now open their parsed sequence count instead of an unconditional four.
BGM000 one/default-two bodies are 110.316281/208.968617 seconds, with identical
first-body PCM prefixes. Ten original sequence checks, seven focused renderer
checks, the default Rust suite (338 passed/116 ignored), Clippy, Rust 1.88 and
15 harness/package checks pass. All 165 short corpus renders also pass with
byte-equal WAVs versus the prior automatic-layout run
(`out/audio-migration/loop-render-duration-comparison.json`). See
[loop-render evidence](../reference/psx-kernel.md#original-runtime-loop-count-stopping)
and `out/audio-migration/loop-render-*`. This supersedes earlier PSX loop-count
blockers; independent fidelity, SFX/XA rendering, remaining conversion/corpus
failures, codecs and complete acceptance still block A4 completion/C retirement.

SF2 pitch-context checkpoint (2026-09-24): nearest legal root selection now
preserves combined pitch when the source-center root alone exceeds generator
limits. Original center/shift stay reversible. The key-121/center-46/shift-114
case uses root 53/coarse -120/fine -98; independent RustySynth frequency testing
passes. Five corpus tone/key rows become representable (17,599 remain
unsupported). Complete-bank acceptance stays 842: the four previous first
range failures now expose later table-context failures, giving 137 pitch-table
and 41 zero-pitch first failures. No bank completion is inferred from this fix.

A new original-initialization probe checks all 30,119 distinct key contexts.
Both pitch tables remain unchanged, but adjacent halfwords at US `0x80184440`,
`0x80184450`, and `0x80184452` change, producing 14 changed returns. This confirms
the state-context gate; active-scheduler stability and zero-step representation
remain unresolved. See [retained pitch evidence](../reference/soundfont.md#root-range-and-initialized-pitch-checks)
and `out/audio-migration/tuning-initialized-probe-after-sources.json`.
Validation passes: 339 default Rust tests (116 ignored), 15 focused media-backed
tuning/bank/pitch-packing checks, Clippy, Rust 1.88 all-target checks and all 15
harness/package tests. Corpus expectations preserve the 178 unresolved banks.

Stopped-pitch checkpoint (2026-09-24): explicit key-on-only SF2 silence now
represents verified zero-step/zero-held-value keys while retaining original PCM.
XML/bindings and song reports identify the approximation; unsupported lookup,
held-value, pitch and reassignment edits reject. The original runtime fixture
confirms a later downward bend activates a stopped voice, unlike static SF2
silence. This is Rust-machine evidence, not independent hardware validation.
The corpus probe resolves 311 zero-key rows in 219 tones/100 banks; two rows
still need out-of-bank sample context. Opt-in whole-bank acceptance rises to
843, adding PL034 with four mapped keys. Forty previous zero-first failures
expose later table failures: 177 banks remain rejected, and strict conversion
stays at 842/178. Six focused checks pass, including original-runtime activation,
consumer behavior, edit rejection, all-bank VH/VB equality and whole-EMI CLI
round trip. See [references](../reference/soundfont.md#stopped-pitch-approximation)
and `out/audio-migration/zero-pitch-final-tests.log`. Validation also passes:
343 default Rust tests (119 media checks ignored), 30 integration checks, 15
harness/package checks, Clippy, Rust 1.88 all-target checks, formatting and
documentation references. Logs use `out/audio-migration/zero-pitch-*`.
A3/A4 acceptance and C retirement remain open.

Earlier-table checkpoint (2026-09-24): an active BGM004 probe covers 1,700,000
reference-clock output frames, observes 78 table reads and no CPU table stores,
and verifies both original pitch-table hashes. Its subsequent IRQ-isolated
snapshot executes all 30,119 distinct corpus pitch contexts. The same three
adjacent halfwords/14 changed returns remain state-dependent. Both table hashes
are now required by the tuning reference; witnessed earlier-table reads are
supported while adjacent runtime-data reads remain rejected. All 12,623 such
corpus rows become representable, reducing unsupported key rows to 4,976.
Whole-bank coverage stays 842 strict / 843 opt-in: later adjacent-data keys still
block those banks. No whole-tone acceptance is inferred from a supported key.
See [runtime snapshot and conversion evidence](../specs/formats/audio.md#verified-earlier-table-reads)
and `out/audio-migration/earlier-pitch-active-rom-observer.json`. The final observer
covers RAM and immutable BIOS-ROM instructions. All 324 PC corpus outcomes
remain unchanged and all 31 successful WAVs are byte-identical; 244 diagnostics
now identify later adjacent-data failures (`earlier-pitch-pc-comparison.json`).

Validation passes: 345 default Rust tests (122 ignored), eight strict bank checks
and 19 other focused media checks, including complete corpus and inverse-pitch
packing, Clippy, Rust 1.88 all-target checks, formatting and 15 harness/package
checks. An initial combined media command omitted the BIOS environment variable;
the affected runtime test passes with the required input supplied. Diagnostic
IRQ/queue failures are retained separately from the completed snapshot evidence.
Logs use `out/audio-migration/earlier-pitch-*`. Mutable-data conversion, independent
hardware/audio fidelity and complete workflow acceptance remain open.

Independent reference acceptance remains part of A4/A6. Require bounded
execution, explicit completion markers and validated capture sizes/identities
before accepting reference evidence. Extend comparisons to music/SFX/voice
schedulers, DMA/IRQ histories, sequence selection and release tails; record the
timing model and scenario coverage. Retain failed or mismatched evidence.
Audio semantics and codec tests remain Rust; Python covers harness integration.
Rust consumes hashed captures without launching an emulator or generating its
runner. Reference boot success does not accept audio fidelity, pruning, BIOS
removal or C retirement; no emulator becomes a production rendering fallback.

Independent runtime checkpoint (2026-09-24): Rust now consumes hashed harness
captures without launching an emulator. Full RAM/GPR/PC equality passes at US EXE
entry and after the original callback initializer. A generic `call.lua` mission
invokes a supplied function from EXE-entry context with explicit argument/return
registers; optional selected-PC traces retain repeated observations and reject
overflow. Sound initialization has matching GPR/PC but 27 differing RAM bytes.
The original ENVX reader stores one for each voice in Redux versus zero in Rust,
and constructs a different zero-envelope mask. A 24-event independent trace,
retirement addresses and Rizin disassembly account for all bytes; the
[runtime specification](../specs/formats/audio.md#independent-bios-handoff-and-sound-initialization-comparison)
records identities and limits. No byte masks or production emulator quirk were
added to turn this failure into acceptance. Next resolve SPU key/envelope timing
and readback, then extend device/PCM comparison. The opt-in boot suite is four
passing checks and one sound-state comparison failure. Default all-target Cargo
tests pass 378 checks with 149 media/reference cases ignored; focused Clippy passes.
This is progress in A4 evidence, not full runtime fidelity, A6 pruning or C
retirement. Logs/captures remain under `out/audio-migration/redux-review/` and
`out/audio-migration/redux-initialization/`.

## 5. [A5] (open) Harness integration and C retirement
- Owner: parent
- Depends: A1, A2, A4
- Blocker: none
- Evidence: none
- Acceptance: full replacement workflow/package/repository checks pass; host C removed only after all prior gates pass; recovered game C preserved

1. [A5.01] (in-progress) Switch harness builds, commands, and source packaging
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: Cargo build/direct verify and unpacked source-package build/tests pass; harness/setup validation checkpoint below
- Acceptance: Cargo-backed build/direct operations and audio package checks pass; unpacked source package builds without proprietary media

Update `tools/python/harness/commands/audio.py` and owning CLI registration to
invoke Cargo and Rust, exposing the seven operations while retaining `audio build`
and `audio package`. Package Rust source, lockfile, required local crates and
licenses without proprietary media. Update `test_audio_surface.py` and affected
harness tests; preserve existing user changes. Update audio usage documentation.

Harness/setup validation checkpoint (2026-09-24): all seven direct audio commands
invoke Rust through Cargo-backed builds. `audio build` without an operation builds
and prints the executable path. `audio package` publishes a deterministic ZIP
with both local Rust crates, lockfiles, tests and licenses, excluding proprietary
media. The unpacked package built and passed Cargo tests using cached registry
dependencies. Live build, package and BGM000 structural verification passed. The
original five C harness/package checks passed before switching; all 15 replacement
audio checks pass. Shared dispatch types keep the expanded CLI below 600 lines.

The final 25 focused setup/doctor tests pass, covering download integrity, failed
publication/refresh, conflicting output, offline reuse, filename-independent BIOS
discovery, missing/wrong hashes, symlinks, unreadable input and focused task routing.
An earlier combined run had 71 passes and one unrelated module-ceiling failure
(`commands/docs_drift.py`, 611 lines). Live `just doctor` passes US BIOS validation
but fails existing GCC 2.7.2 (`--version` exits -31) and compiler-wrapper checks.
Ruff/format, shell syntax, documentation links and plan validation pass. Evidence
is under `out/audio-migration/audio-*`, `bios-*` and `harness-rust-*`. Runtime,
codec and complete acceptance gates remain open; host and recovered C are retained.

2. [A5.02] (open) Accept the workflow and retire host C
- Owner: parent
- Depends: A5.01, A6, A7
- Blocker: none
- Evidence: none
- Acceptance: all original and standalone-driver/modularization acceptance gates pass before tools/c/psx-audio deletion; post-removal checks pass and src/bof3/audio is preserved

Remove `tools/c/psx-audio` only after complete workflow acceptance. Remove obsolete
C build/package references and document retired commands. Review acceptance
evidence and ownership before deletion; no unresolved runtime, mapping, codec or
corpus failure permits retirement.

## 6. [A6] (in-progress) Narrow and ship the standalone audio driver
- Owner: parent
- Depends: A1.02, A1.03
- Blocker: none
- Evidence: none
- Acceptance: the shipped audio-only assembly builds reproducibly into a standalone driver; all production operations run without a game executable or BIOS; original music/SFX/voice behavior and independent reference gates pass

1. [A6.01] (in-progress) Establish audio entry points and complete dependency closure
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: parent ran bounded direct-flow and post-bootstrap BGM004/BGM053 audits, original SFX table inspection and six focused checks; coverage gaps remain explicit
- Acceptance: target-qualified static and dynamic evidence covers music, SFX and voice roots, indirect dispatch, required data/state and unresolved reachability edges

Inventory initialization, bank loading, sample upload, sequence selection,
scheduling, controller handling, voice allocation/release, SFX triggering,
vocal/XA cue playback, DMA, IRQ and CD service entry points. Trace representative
and boundary cases against the existing original-runtime path and independent
references. Build a function/basic-block and data-reference closure from these
roots, including function pointers, callbacks, interrupt handlers, jump tables,
relocations, shared globals and initialization order. Record per-function/data
provenance, source profile/hash, callers and inclusion rationale. Missing dynamic
coverage or unresolved indirect targets prevents pruning the affected closure.

Reachability checkpoint (2026-09-24): the Rust `driver/` noun modules now
record target-qualified direct instruction closure, delay slots, calls,
indirect/exception boundaries and bounded SFX dispatch-table candidates.
Post-bootstrap BGM004/BGM053 probes add register targets, external re-entry
points and memory footprints without mutating the guest. Both expose low-RAM
BIOS-populated execution and sixty B0:17 calls; no BIOS-ROM instructions in the
interval does not mean BIOS independence. The re-entry evidence accounts for
34 original instructions missed by initial direct traversal. Final audits cover
all measured executable PCs, but initialization/data ownership and dynamic
SFX/voice/XA coverage remain open. No code has been pruned or shipped as a driver.
See [evidence, counts and limits](../specs/formats/audio.md#standalone-driver-reachability-audit).
Receipts use `out/audio-migration/driver-closure-*`; 354 default tests pass with
126 opt-in cases ignored, six focused checks pass including original US table
evidence, and Clippy/MSRV checks pass. This starts A6.01, not its acceptance.

Initialization checkpoint (2026-09-24): read-only preparation observation now
covers all guest calls through initialization, layout, VAB/SEP loading and
callback registration. Tests show unchanged full RAM/SPU RAM, register/call
state, instruction counts and PCM, and explicit abort on observer failure.
BGM004/BGM053 probes identify the same startup BIOS services, including setjmp,
HookEntryInt, SPU events and CD-event/handler removal; their bank-transfer event
counts differ with media size. Hook registration explains the previously found
low-RAM re-entry. Both traces observe 3,765 original instructions and seed a
14,612-instruction conservative closure. Saved context and service arguments are
evidence for deriving the standalone ABI, not an approved BIOS snapshot to ship.
See [preparation evidence](../specs/formats/audio.md#driver-preparation-dependencies)
and `out/audio-migration/driver-initialization-*`. Eight focused checks pass;
bootstrap/data ownership, other runtime paths and A6 acceptance remain open.

Explicit SFX checkpoint (2026-09-24): shared VAB preparation now supports banks
without SEP data and fixes final transfer validation to use the selected VAB ID.
Original explicit-tone key-on and per-voice release execute for COMN_SE/BATL_SE
on voices 7 and 23; the zero-volume BATL tone remains silent. The o32 caller
argument area is reserved/restored, and invalid context fails before guest mutation.
Both final probes cover all 4,251 observed original instructions in an
8,190-instruction conservative closure, with 147 unresolved sites retained.
See [SFX evidence](../specs/formats/audio.md#explicit-sfx-tone-execution) and
`out/audio-migration/driver-sfx-*`. Ten focused checks pass. These are direct
SDK calls; game cue/auxiliary loading, voice allocation, multi-tone effects,
voice/XA paths, complete data closure and independent fidelity remain open.
No pruning, BIOS removal or phase acceptance is claimed.

Game cue checkpoint (2026-09-24): a source-qualified type-8 staging adapter now
executes the original slot-1 dispatcher, retaining its layer order, repeated
tones and post-key-on volume calls. All 11 COMN_SE and four BATL_SE rows pass
fresh-bank probes: 32 key-on calls, 13 audible rows and two silent rows. Each
conservative closure covers every observed original PC, with 9,279 instructions
and 151 unresolved sites. Twelve focused checks pass, including invalid identity,
capacity, record and staged-data cases without guest execution. See
[cue evidence](../specs/formats/audio.md#game-sfx-cue-dispatch) and
`out/audio-migration/driver-cues-*`. Host staging still excludes CD transport;
other handlers, cross-bank state, repeated-cue arbitration, voice/XA, complete
data closure and independent fidelity remain open. This advances A6.01 without
accepting pruning or standalone startup.

The same 15 cases now include effective memory footprints for executable-resident
instructions: 5,204–5,289 footprints across 3,547–3,566 addresses per run, with
all WAV bytes unchanged. `out/audio-migration/driver-cues-memory/` records the
data-access evidence; BIOS/kernel, DMA and host-staging accesses remain excluded,
so complete data ownership is still open.

Cue-history checkpoint (2026-09-24): the original separate 24-voice status poll
is now callable and included in driver roots. Scheduled original cues preserve
equal-value retriggering, lower-value suppression, accepted history, stale status
after release and acceptance after an explicit poll; changing first voice takes
the original alternate path. Two histories cover all observed original PCs in
9,345-instruction closures with 153 unresolved sites retained. The single-cue
timeline retains identical WAV bytes. Fifteen focused checks pass, including
schedule rejection and all original status-query masks, without patched guest
activity flags. See [history evidence](../specs/formats/audio.md#cue-history-and-voice-status-polling)
and `out/audio-migration/driver-history-*`. Poll cadence, other handlers,
cross-bank state, voice/XA, complete data ownership and independent fidelity
remain open; A6.01 remains in progress.

Seven-slot checkpoint (2026-09-24): local cue dispatch now selects each original
handler for slots 0–6 and bounds the last auxiliary buffer before live sound
controls. Original cue attributes may alias another packed program block; the
adapter preserves that lookup while checking VH bounds. Seventeen focused driver
checks pass, including audible representatives of every handler and exact
program-3 call arguments. The broader audit prepares all 904 local auxiliary
tables, but only 8,575 of 8,608 records dispatch: 33 are rejected before execution
by the populated-tone range guard. Their original runtime behavior is unresolved,
so corpus acceptance remains open. See
[seven-slot evidence](../specs/formats/audio.md#seven-slot-cue-dispatch) and
`out/audio-migration/driver-slots-*`. This advances A6.01; multi-bank histories,
cross-bank modes, voice/XA, complete data ownership, independent fidelity and
standalone startup remain unaccepted. A6.02 pruning is still gated.

Checkpoint validation: 356 default Rust checks pass (135 media checks ignored),
17 focused driver checks pass with media, and 15 harness/package checks pass.
Clippy, Rust 1.88 checks, formatting, documentation references, plan structure
and whitespace checks pass. Repeated corpus dispatch results are identical;
the final audit exits unsuccessfully for the 33 unresolved records, with zero
skipped rows. These failures remain acceptance gates rather than waived cases.

Physical-tone checkpoint (2026-09-24): the original SDK reads physical tone
slots beyond declared counts and explicitly skips sample-zero slots. The cue
adapter now preserves these paths while guarding actual SDK tone reads and
sample references. All 32 supported prior rejections match direct original
dispatch in calls, return values, tone reads and PCM hashes. The corpus audit
now dispatches 8,607 of 8,608 records across all 904 prepared tables. The remaining
DRG04_00 row selects sample 3 from a two-sample bank; direct diagnostic playback
reads beyond the uploaded body and fails ADPCM decoding. Required bank/state
context remains unresolved and the adapter rejects it explicitly. See
[physical-slot evidence](../specs/formats/audio.md#physical-tone-slots-and-sample-zero-returns)
and `out/audio-migration/driver-range-*`. Twenty focused checks pass. A6.01 remains
in progress; pruning must retain used physical slots despite declared counts.

XA closure checkpoint (2026-09-24): read-only tracing now covers the first
S_XA, MAGIC and VOICE cues with one and four raw sectors per scheduler call.
All six supplied-input runs reach Pause through original selector, state
handlers, SDK IRQ wrapper and game callback. The union contains 1,549 observed
original instructions within a 2,226-instruction conservative closure; all 62
unresolved sites remain retained. Reports qualify addresses by executable/file
offset and hash consumed raw sectors. The shared observer now belongs to
`src/driver/trace.rs`; explicit host-call boundaries prevent invented re-entry
edges. Three synthetic observer checks, all 23 driver checks and five original
XA tests pass. The default suite passes 359 tests with 139 media cases ignored;
strict Clippy and Rust 1.88 checks pass. See
[XA closure evidence](../specs/formats/audio.md#xa-driver-closure-with-supplied-kernel-inputs)
and `out/audio-migration/driver-transport-*`. These fixtures supply kernel RAM,
CD readiness/responses, dry SPU state and tick timing. Initialization still stops
at A0:72. Full initialization/data ownership, cue histories, independent fidelity
and BIOS-free execution remain open; A6.01 is not complete and no pruning is
authorized.

Booted-ROM XA checkpoint (2026-09-24): the original US CD reset dispatcher
at `0x80175534`, mode 2, completes callback initialization from BIOS shell-handoff
state. Six S_XA/MAGIC/VOICE runs now execute the original exception vector and
ROM/RAM kernel, without HLE service dispatch or supplied kernel tables. Their
command sequences, scheduler states, stop positions, sector counts and dry PCM
hashes equal the supplied-kernel cases at both explicit sector schedules.
The combined audit covers 1,596 original instructions in a 2,457-instruction
conservative closure and retains 66 unresolved sites. Added the reset dispatcher
to the root inventory; its fuller device-reset branch remains unverified.
Service argument/return-address evidence distinguishes five zero-handle
CloseEvent calls from demonstrated removal of live CD events. The fixture
machinery lives in `tests/support/transport/machine.rs`; the test root retains
its discoverable cases. All six XA runtime checks pass, including the ROM/kernel
comparison. See [booted-ROM evidence](../specs/formats/audio.md#xa-callback-initialization-and-original-rom-dispatch)
and `out/audio-migration/driver-startup-*`. Full CD reset currently reaches an
unsupported GPUSTAT read in VSync; ready CD state, prior SDK completion, GPU
status, dry SPU context and timing remain supplied. A6.01 stays in progress;
no BIOS independence, complete data closure or pruning acceptance is claimed.

CD reset checkpoint (2026-09-24): the Rust drive now handles command 0x0A
with separate acknowledgement/completion, stopped reading, unmuted mode 0x20,
and preserved filter, pending location, transfer header, data cursor and queued
XA output. An explicit PCSX-Redux host-reset model supplies IRQ mask 0x1F and
the reference volume matrix without seeding responses or SDK completion. The
original mode-1 reset now establishes readiness through Getstat, Init and Demute;
the full-reset fixture no longer injects prior completion or writes the IRQ mask.
All fixtures begin at LBA 0, so original Setloc/Seek commands establish cue
positions. Six full-reset S_XA/MAGIC/VOICE cases match callback-only playback
outcomes after the three recorded initialization commands. The observed union
increases to 1,749 original game instructions; the conservative closure remains
2,457 instructions with 66 unresolved sites. See
[reset semantics and evidence](../specs/formats/audio.md#original-cd-reset-and-host-reset-state)
and `out/audio-migration/driver-reset-*`. An empty IRQ mask demonstrably stalls
the original initializer; the reset default is source-backed, not a fallback.
GPUSTAT, initial disc authentication/spinning state, dry SPU context and timing
remain explicit inputs. The production GPU read, full hardware timing, data
ownership, independent fidelity and standalone driver gates remain open.
A6.01 stays in progress and no pruning is authorized.

VSync dependency checkpoint (2026-09-24): all observed VSync calls in the
six full-reset XA scenarios pass -1 through three SDK return sites. GPUSTAT
zero/all-one comparisons preserve command sequences, scheduler states, sector
counts, stop positions and dry PCM hashes; every GPU read is accounted for by
those calls. The original negative VSync branch returns the software counter
and restores the GPU/timer-derived registers without using their values.
A separate interrupt-free test compares original instructions with two
fixture-only NOP substitutions across GPU, Timer-1, negative-argument and
software-counter values. All 54 bounded calls preserve the instruction path,
final registers, RAM writes and guest data after excluding only the two edited
code words. No production instruction or executable is patched. See
[VSync input evidence](../specs/formats/audio.md#negative-vsync-peripheral-input-dependencies)
and `out/audio-migration/driver-vsync-*`. This identifies a bounded pruning
candidate, not authority to remove all GPU/timer behavior or the live software
counter. Interrupt timing, other caller domains, full dependency closure and
standalone execution remain open; A6.01 remains in progress.

Interrupt replay checkpoint (2026-09-24): the two fixture-only negative-VSync
NOP candidates now replay full original CD reset and six booted-ROM XA scenarios
with GPUSTAT and Timer-1 counter reads unavailable. Across 3,691,008 instructions,
2,423 completed IRQs and 1,534 VSync calls, each pair preserves PC sequence,
guest-call return registers, normalized RAM, IRQ counts, commands, scheduler
states and PCM hashes. Source and production code remain unmodified. See
[interrupt replay evidence](../specs/formats/audio.md#negative-vsync-candidate-through-original-interrupts)
and `out/audio-migration/driver-candidate-*`. Nine XA runtime checks and 362
default checks pass (144 media cases ignored), as do Clippy and Rust 1.88 checks.
Mise trust blocked that launcher in this session; checks used the installed
matching Rust 1.98.1 without altering trust. Other interrupt phases, physical
timing, full caller/data closure, independent fidelity and standalone startup
remain open. A6.01 remains in progress and A6.02 pruning remains gated.

Retirement checkpoint (2026-09-24): the development trace now records successful
ROM/kernel steps as well as original executable instructions, with actual fetched
words, pre-step address registers, bus widths, consumed/stored byte lanes and
cache-isolation flags. Six full-reset XA runs account for all 3,691,008 observed
boundaries as 3,689,304 retired steps and 1,704 syscall traps; existing outcomes
remain equal. This adds 678 BIOS-ROM and 661,082 other-RAM memory accesses that
the earlier EXE-only observer could not see. Four Rust observer tests and all 376
default tests pass (147 ignored). See
[retired dependency evidence](../specs/formats/audio.md#retired-xa-instruction-and-memory-dependencies)
and `out/audio-migration/driver-retirement-*`. Host/DMA producers, wider histories,
music/SFX coverage, complete initialization ownership, independent fidelity and
standalone startup remain open; no pruning or phase acceptance is claimed.
All nine XA runtime checks and 17 harness/package checks also pass, along with
all-target Clippy, Rust 1.88, formatting, scoped references and plan parsing.
2. [A6.02] (open) Extract the audio closure and prune unused leaves
- Owner: parent
- Depends: A6.01
- Blocker: none
- Evidence: none
- Acceptance: the extracted audio slice replays baseline traces and every removed function/block/data region has a retained reachability justification; no supported SFX/voice/music path is lost

Extract only used driver functions and required tables/state from the supported
executable into an isolated development artifact. Narrow conservatively in
bounded passes: remove proven unused leaves and unreachable branches, then
recompute static references and replay coverage. Include assembly leaves with
hardware side effects even if they do not call another function. Trace absence
alone is not proof of dead code. Keep an explicit retained/pruned/unresolved
inventory and before/after hashes; stop each pass on new unresolved edges or
behavior differences. Lift and name driver-used code only, without modifying
recovered `src/bof3/audio` or unrelated game systems.

3. [A6.03] (open) Replace game and BIOS bootstrap with explicit audio state
- Owner: parent
- Depends: A6.02
- Blocker: none
- Evidence: none
- Acceptance: the isolated driver starts and runs all covered audio paths from deterministic owned state without loading BIOS or unrelated game code; traces match the development reference

Identify every retained dependency on BIOS services, shell handoff, exception
chains, event/thread state, timers, heap/stack setup, CD services and game-owned
initialization. Implement only the required behavior as driver startup code or
explicit Rust host services with a documented ABI. Seed derived tables/state by
reviewed initialization logic, not opaque BIOS RAM snapshots. Preserve scheduler,
IRQ/DMA ordering, SPU state, sample transfers and CD/XA behavior. Remove unused
shell, GPU and other non-audio execution only when closure evidence permits it.
No generic BIOS emulation fallback, stub success or hardcoded song-specific
bootstrap is permitted. Unsupported required services remain explicit blockers.

4. [A6.04] (open) Ship assembly source and a reproducible driver build target
- Owner: parent
- Depends: A6.03
- Blocker: none
- Evidence: none
- Acceptance: a clean source checkout builds the named audio-driver executable twice with identical bytes, entry/ABI metadata and a checked symbol/relocation map, without game executable, BIOS or generated extraction inputs

Create the `bof3-audio-driver` build target and connect it to the existing audio
build/package integration. Ship the narrowed, lifted assembly, required constant
data, linker/layout description and provenance; do not require end users to
extract code from their game executable. Specify load/entry addresses, stack,
mutable state ownership, host-call ABI, interrupts and supported relocation
rules. Keep the guest driver separate from the Rust host binary. Use an approved
existing assembler/linker or implement the needed deterministic build support;
new dependencies still require specific authorization. Build tools must not
become external runtime/rendering fallbacks. Package driver source/build inputs
and licenses; exclude full game executables, BIOS images and game audio media.

5. [A6.05] (open) Route every production operation through the owned driver
- Owner: parent
- Depends: A6.04
- Blocker: none
- Evidence: none
- Acceptance: index/query/map/extract/render/pack/verify use owned driver code/data and no production path opens a game executable or BIOS; required media remains explicitly supplied

Replace production executable-derived pitch, mapping, initialization and runtime
calls with the built driver and its reviewed tables/ABI. Preserve source-qualified
asset identities and profile provenance without requiring the original binary.
Remove production `--executable`/`--bios` requirements and obsolete CLI branches;
do not silently accept ignored options. Keep original-reference execution in
separate development validation tools only. Normal setup/doctor must not require
a BIOS for the audio tool; any retained BIOS/PCSX-Redux preparation and hash
validation is explicitly for optional reference work. Preserve existing user
input files. Update CLI, harness, reports and package documentation together.

6. [A6.06] (open) Validate standalone equivalence and dependency removal
- Owner: parent
- Depends: A6.05, A4.01
- Blocker: none
- Evidence: none
- Acceptance: clean packaged builds and full music/SFX/voice corpus tests pass with executable/BIOS paths absent and no access to them; driver traces/audio and independent reference comparisons meet the declared gates

Run differential original-runtime versus narrowed-driver tests for bank/sample
loading, voices, programs/layers, pitch/bends, tempo, loops, scheduler, DMA/IRQ,
SFX, vocals and XA cues/interleaving. Compare traces and pre-lossy PCM separately;
diagnose every difference instead of accepting a narrower corpus. Keep independent
emulator/hardware evidence as the fidelity gate. Verify clean startup, malformed
inputs, missing driver/corrupt driver, unsupported services and capacity failures.
Build and exercise the unpacked source package with game executable/BIOS and
prior extraction caches unavailable; inspect file access/dependency closure as
well as command success. Retire the old production boot/executable paths only
after replacement checks pass. This does not relax the original codec, edited
round-trip or whole-corpus acceptance requirements.

## 7. [A7] (in-progress) Modularize the Rust source tree
- Owner: parent
- Depends: A1.02
- Blocker: none
- Evidence: reviewed ownership map and pinned inventories cover the baseline source, tests and examples; implementation remains in progress
- Acceptance: cohesive noun directories and single-noun Rust filenames cover production code, tests and examples; dependencies, docs and package manifests resolve without compatibility modules and all applicable checks pass

1. [A7.01] (done) Define module ownership and the rename map
- Owner: parent
- Depends: none
- Blocker: none
- Evidence: reviewed [layout proposal](#rust-audio-layout-proposal); pinned source/test maps, dependency audit and Cargo metadata under `out/audio-migration/layout-*`
- Acceptance: current modules and public APIs have an explicit destination/owner map, dependency direction and no ambiguous filename or ownership collisions

Inventory the flat Rust source and group by cohesive responsibility: archives,
identity/catalog, banks, sequences, codecs, interchange, machine/devices, driver,
rendering and verification/publication. Select singular noun directory names
where natural; use nested domains for distinct owners rather than concatenated
filenames such as `soundfont_sample_pack.rs`. File stems should be one noun such
as `samples.rs`, `packing.rs`, `controls.rs` or `reader.rs` under the owning folder;
crate roots follow Rust conventions. Define public APIs and dependency direction,
including the standalone guest source/build boundary. Avoid miscellaneous dumping
grounds, cyclic imports and one-line facades preserving old paths.

Ownership checkpoint (2026-09-24): the
[module map](#rust-audio-layout-proposal) records unique noun-based
destinations for all 125 current Rust source files, including 105 moves. It
defines domain responsibilities, intended dependency direction, API visibility,
guest/host source separation and staged move order. The pinned source inventory
is `out/audio-migration/driver-context-module-map.json`. Dependency cycles and
test/example integration-root destinations still require audit before A7.01
acceptance. No source relocation or compatibility facade has been introduced;
A7.02 and A7.03 retain their dependencies.

Ownership acceptance (2026-09-24): all 125 source and 142 test/example files have
unique destinations. The 434-edge dependency scout and owning type/import review
identify three cycles; the map assigns explicit shared-record/helper boundaries
to remove each during its move group. Public type destinations, private scope,
compiled JSON placement and Cargo integration roots are specified. Current Cargo
metadata confirms 104 test targets and 14 examples. Source/test hashes and map
rows are checked; this completes A7.01 planning, not those implementation fixes
or runtime acceptance. A7.02 begins with the independent codec group; A7.03 still
depends on A6.05.

2. [A7.02] (done) Move format and interchange modules in bounded groups
- Owner: parent
- Depends: A7.01
- Blocker: none
- Evidence: all bounded format/interchange checkpoints below pass; final source/test scope, dependency audit, media and package receipts are recorded under `out/audio-migration/layout-media-*`
- Acceptance: archive/bank/sequence/codec/interchange modules follow the approved ownership map and unchanged bytes, identities, edit validation and rendering checks survive each move

Move cohesive groups mechanically, updating module declarations, imports, tests,
examples and documentation links together. Separate renames from semantic fixes;
retain source/media provenance when paths move. Reuse existing meaningful tests
and the authorized migration coverage. Check for stale filenames and hidden
cross-domain dependencies after each group; do not retain legacy re-export
modules or broaden public APIs merely to avoid fixing callers.

Codec checkpoint (2026-09-24): moved ADPCM decoding/encoding, quantization
and CD EDC into `src/codec/`, with their four former integration roots registered
under `tests/codec.rs`. Updated callers and format-document links without root
compatibility exports or codec algorithm changes. All 494 test functions remain
discoverable; integration targets change from 104 to 101 and runnable examples
remain 14. Default tests pass (356 passed, 138 ignored), as do all-target checks,
Clippy, formatting, Rust 1.88 compatibility and the 15 harness/package checks.
The additional missing-project-pin check also passes: the harness fails before
invoking mise when the audio configuration is absent. The real scoped harness
release build succeeds (`out/audio-migration/layout-harness-build.log`).
The independent PCM corpus, edited ADPCM excerpts and two independent VAG
consumer checks pass; disc EDC is validated separately with its track fixture.
Receipts: `out/audio-migration/layout-codec-*`, `layout-tests-after.log`,
`layout-cargo-after.json` and `layout-harness-tests.log`. Remaining format and
interchange groups keep A7.02 in progress.

Document/archive/bank checkpoint (2026-09-24): moved eight production files into
their mapped `document/`, `archive/` and `bank/` owners and five test files under
three integration roots. Visibility, codecs, XML validation, publication and
media semantics are unchanged; callers and live format links use the new owners.
All 494 test functions remain discoverable (356 default passes, 138 ignored;
99 integration roots, 14 examples). Seven media checks pass, including exact
container preservation of 880 EMI files/four XA streams, original-disc extents,
all 1,020 bank exports with independent PCM/RIFF/XML validation, malformed-input
publication checks and XML reconstruction of the original BGM000 bank. All-target
check, Clippy, Rust 1.88, formatting, 16 harness/package checks and scoped links
pass. Receipts and the 13-file move map are under
`out/audio-migration/layout-format-*`. A7.02 continues with sequence/interchange;
the catalog and SoundFont cycle splits remain implementation obligations.

Sequence/interchange checkpoint (2026-09-24): moved eleven production files into
`sequence/` and `interchange/`, and ten test files under their two integration
roots. Updated callers, shared-test relative paths and live format links. Local
`translation` imports distinguish SEP translation from standard MIDI syntax;
there are no root compatibility exports or new public APIs. Parser, scheduling,
translation and edit behavior are unchanged. All 494 qualified test functions
match the prior inventory after normalizing only the recorded path changes;
356 default tests pass, 138 remain ignored, and Cargo discovers 91 integration
roots plus the same 14 examples. All 14 sequence runtime/corpus checks pass,
covering 476 selectable sequences, whole SEP-entry equality, edited events,
retimed streams, original loop/end/delta dispatch and independent SMF consumers.
This does not establish independent PSX playback fidelity or bank-response
equivalence. All-target check, Clippy, Rust 1.88, formatting, 16 harness/package
checks and scoped documentation links pass. Receipts and the 21-file move map:
`out/audio-migration/layout-sequence-*`. Shared runtime/music helpers retain their
current owners until their later move group; references already account for the
relocated sequence tests. A7.02 remains in progress for the catalog cycle split
and SoundFont/music/XA groups; A7.03 still requires A6.05.

Catalog cycle checkpoint (2026-09-24): moved the 23 catalog, loader, music/XA cue
and bank-content records to `catalog/model.rs`. The model owns only data and the
three existing asset predicates; loading/resolution stays in `catalog/mod.rs`,
`catalog/loader.rs`, `catalog/content.rs`, `catalog/music.rs` and `xa/cue.rs`.
`numeric_id` is visible only to the parent catalog module so its existing selector
can call the relocated predicate. No public compatibility exports were retained.
The XA decoder and six tests moved to their mapped parent/child paths to host the
cue module without temporary path attributes. Compiled music identity metadata
moved to `catalog/music.json` with SHA-256
`38ab71df6410dbcb998846024db54fe314747e6c378ef2b13eb34e55f38057da` unchanged.

Fresh lexical dependency scouting (129 nodes, 432 edges), followed by inspection
of the shared model and resolver imports, finds no remaining catalog cycle; only
the two planned SoundFont cycles remain. This is source-dependency evidence,
not a runtime-fidelity claim. The rebuilt CLI's representative BGM000 `map` and
`index` JSON are byte-identical to the retained pre-move output. All 494 qualified
test functions remain discoverable after normalizing the recorded moves (356
default passes, 138 ignored; 87 integration roots and 14 examples). All 12
catalog/XA media checks pass, including full loader associations, all supported
music-cue mappings, renamed/ambiguous source selection and disc-backed XA extents.
Clippy, Rust 1.88, formatting, 16 harness/package checks, release build and scoped
documentation links pass. Receipts, CLI snapshots, dependency audit and the
13-file move map are under `out/audio-migration/layout-catalog-*`. A7.02 continues
with SoundFont cycle removal and the remaining SoundFont/music/XA moves.

SoundFont checkpoint (2026-09-24): removed both remaining reviewed cycles. The
reader and hydra parser now share six passive types in
`soundfont/reader/model.rs`. Silent-program creation takes present program IDs,
sample rate, SF2 bank and percussion choice, returning created indices/programs;
the bank binder owns report updates and the unchanged diagnostic. Empty-program
population remains supported. Thirteen production files and twenty test/helper
files follow the planned SoundFont paths. Shared fixtures are registered once
per integration root; no root compatibility exports were added.

Dependency scouting now reports 131 modules, 434 edges and no cycles, with the
reader/model and binder/silence boundaries inspected directly. Seven BGM004
SF2/MIDI/XML assets are byte-identical before/after; its extraction report differs
only in the requested output directory. BGM000 still rejects its unsupported
pitch-table context; that limitation was not bypassed for the comparison.
All 494 qualified tests remain discoverable (356 default passes, 138 ignored;
74 integration roots and 14 examples). Nine SoundFont/program media checks have
passing results across the corpus run and the corrected pitch-test rerun. The
corpus evidence continues to distinguish accepted structures, rejected pitch
contexts and measured envelope error; it does not establish full PSX/PC fidelity.

One pre-existing ignored pitch test assumed every SF2 instrument had a root key.
The exact pre-change binder/helper source hashes matched a retained source
package, whose unchanged test reproduced the same panic. The test now applies
equivalent tuning edits and compares pitches only for report-qualified VAB tone
instruments, leaving reserved silent-program instruments unchanged. Its 1,050
original pitch-bit checks and unsupported-edit assertions pass. The original
failed run, baseline reproduction and successful rerun are retained separately.
Clippy, Rust 1.88, formatting, focused fixture checks and 16 harness/package checks
pass; the current corrected test is verified inside the source ZIP. Receipts,
the move map, cycle audit and export hashes are under
`out/audio-migration/layout-soundfont-*`; the preliminary helper split is under
`layout-silence-*`. A7.02 remains in progress for the remaining music/XA and shared
sample/voice owners; machine/application moves still depend on A6.05.

Final A7.02 checkpoint (2026-09-24): moved the remaining sixteen music, XA,
sample and voice arithmetic source files, fifteen test files and six shared
helpers to their planned owners. Music fixtures are registered once, and the XA
transition fixture retains its original JSON bytes with the corrected relative
include. All A7.02 destinations exist. The remaining 97 source/test moves belong
to the deferred machine, application and runtime-helper owners; example naming
also remains for the later layout checks. The source audit finds 134 modules,
434 dependency edges and no cycles. No codec, musical or runtime algorithm was
changed in this final group.

All 494 qualified tests remain discoverable after normalizing recorded paths
(356 default passes, 138 ignored; 62 integration roots and 14 examples). All 31
selected media checks pass: 21 cover bank/music extraction and packing, program
population, sample references and original pitch/gain behavior; ten cover XA
codecs, streams/cues, XML round trips, edited sectors and capacity rejection.
The XA checks preserve complete original files for 31 streams, 26,599 sectors and
84,174,048 frames, while continuing to report decoder-arithmetic differences
separately from unverified game/hardware playback. Seven BGM004 SF2/MIDI/XML
assets remain byte-identical; the extraction report differs only in the output
directory. Clippy, Rust 1.88, formatting, 16 harness/package checks, the release
build and scoped documentation checks pass. Current validation commands below
use the audio project's mise pin and consolidated archive test target.

Evidence: `out/audio-migration/layout-media-validation.json`, the corresponding
move/discovery/scope/dependency/export receipts and retained test/build logs.
This accepts A7.02's format/interchange relocation. A7, A6 and the full migration
remain incomplete: A7.03 still requires A6.05; no pruning, BIOS/game-executable
independence, independent playback fidelity or C retirement is accepted here.

3. [A7.03] (open) Organize machine, driver and application modules
- Owner: parent
- Depends: A7.02, A6.05
- Blocker: none
- Evidence: none
- Acceptance: machine/device, standalone-driver, rendering, verification and CLI owners follow the noun hierarchy and no production BIOS/game-executable dependency is reintroduced

Apply the same policy to CPU/bus/device modules, audio scheduling, guest driver
source/build ownership, rendering and command orchestration. Keep format parsing,
interchange translation, execution and publication responsibilities separate.
Align test/example folders with their owning domains; use explicit test targets
where needed so Cargo still discovers all intended coverage. Preserve the module
size and single-owner contracts; do not hide complexity in a large `mod.rs`.

4. [A7.04] (open) Verify paths, packages and the final source layout
- Owner: parent
- Depends: A7.03
- Blocker: none
- Evidence: none
- Acceptance: Cargo formatting/checks/tests/Clippy/MSRV, scoped harness checks, references and unpacked package build/driver execution pass; obsolete module paths and flat compound filenames are absent

Audit tracked and packaged Rust/assembly paths against the ownership map, fixing
build manifests, include paths, feature gates, examples and documentation anchors.
Verify public CLI behavior and media results remain stable across the structural
change. Retain final path mapping and check receipts in the plan evidence. New
runtime/media acceptance failures block A5 retirement even if the rename checks
pass.

Toolchain checkpoint (2026-09-24): audio owns
[`tools/rust/bof3-audio/mise.toml`](../../tools/rust/bof3-audio/mise.toml), pinning
Rust 1.98.1 while Cargo retains the declared 1.88 minimum. Harness audio builds
run Cargo through mise in that crate directory; source packages require and
include the same configuration. The unpacked-package build/test gate uses that
pin. Setup and direct commands live in the
[audio README](../../tools/rust/bof3-audio/README.md). No global or text-project
configuration is changed. The layout proposal below belongs to this scoped plan;
its former agents-directory copy and navigation row were removed after preserving
an exact recovery copy and checking all transferred table rows.

### Rust audio layout proposal

Project-scoped layout proposal retained in this original plan. The maps record
the pre-move baseline; completed groups are reported in A7.02. Reviewed
destination map for [migration phase A7](#7-a7-in-progress-modularize-the-rust-source-tree).
Destinations are relative to `tools/rust/bof3-audio/src`; moves and runtime
acceptance remain incomplete.
The pinned source inventory is `out/audio-migration/layout-module-map.json`;
test/example destinations are in `layout-test-map.json` in the same directory.

#### Ownership and dependency direction

- `archive`, `bank`, `sequence` and `xa` own their preserved media layouts and
  format-specific reconstruction. `catalog` owns qualified identities and
  evidence-backed loader/cue associations; it does not mutate symbols.
- `codec` owns shared sample arithmetic and CD checksums. `interchange` owns
  MIDI/WAVE syntax. `soundfont` owns SF2 syntax, game-bank binding, approximations
  and inverse edits. These responsibilities remain distinct modules within that
  owner; packing code must not bypass source validation.
- `sample` owns shared PCM edit detection and runtime sample-reference semantics.
  `voice` owns verified pitch and gain calculations. `document` owns XML syntax
  and confined manifest reads. `digest` and `publication` remain independent
  shared mechanisms, not miscellaneous helper folders.
- `music` orchestrates song extraction/packing; `render` orchestrates PC and PSX
  execution. `packing` handles extraction roots. `verification` retains separate
  structural, identity, translation, byte-equality and playback evidence.
  `cli` adapts commands to these owners; low-level owners must not import CLI.
- `machine` owns CPU, bus, devices, kernel reference execution and audio-runtime
  adapters. CD and SPU files move into device folders, with voice state below
  SPU. `driver` owns used-code closure evidence and the standalone-driver host
  interface. Owned guest assembly/build inputs belong under
  `tools/rust/bof3-audio/driver/`, separate from host `src/driver/` and proprietary
  development inputs. Phase A6 must establish that guest boundary before the
  machine/application move phase.

The desired dependency direction is command/orchestration to domain operations
and then parsers/codecs/runtime primitives. Format parsers must not depend on
command or rendering orchestration. The audit below records current exceptions and their required resolution; the
current tree does not yet follow every destination boundary.

#### API and move rules

Each existing module's destination API is its destination path with `.rs` and a
terminal `/mod` removed, using `::` between components. Items keep their names
and visibility unless a separately justified semantic change is required.
Private modules remain accessible only to their existing consumers; use the
narrowest required visibility in new parents. Do not add old-path re-exports or
broaden public APIs to avoid updating callers. New `mod.rs` files declare their
children; they are not compatibility facades. `lib.rs`, `main.rs` and `mod.rs`
are conventional exceptions to the single-noun filename rule.

Execute the map in bounded groups after A7.01 acceptance: codec/document and
archive/bank first, then sequence/interchange, soundfont/music/XA. Move machine,
driver and application owners only after A7.02 and A6.05. Update imports,
qualified type paths, `#[path]`/`include!` references, tests, examples, docs and
package inputs in the same group; compare behavior with the retained pre-move
checks. Test/example discovery follows the explicit map below. Machine boot/reference files must not be
retired solely because a destination is listed here.

#### Source destinations

The inventory covers all 125 baseline source files with unique destinations.
An unchanged path remains the same owner. New directory module declarations
are additional structural files, not extra copies of existing implementations.

| Baseline source | Destination |
| --- | --- |
| `adpcm.rs` | `codec/adpcm/mod.rs` |
| `adpcm_encode.rs` | `codec/adpcm/encoder.rs` |
| `archive.rs` | `archive/mod.rs` |
| `archive_pack.rs` | `archive/packing.rs` |
| `bank.rs` | `bank/mod.rs` |
| `bank_content.rs` | `catalog/content.rs` |
| `bank_manifest.rs` | `bank/manifest.rs` |
| `bank_soundfont.rs` | `soundfont/bank.rs` |
| `catalog.rs` | `catalog/mod.rs` |
| `catalog_cli.rs` | `cli/catalog.rs` |
| `cd_edc.rs` | `codec/edc.rs` |
| `cli.rs` | `cli/mod.rs` |
| `cue.rs` | `catalog/music.rs` |
| `digest.rs` | `digest.rs` |
| `disc.rs` | `archive/disc.rs` |
| `driver/closure.rs` | `driver/closure.rs` |
| `driver/flow.rs` | `driver/flow.rs` |
| `driver/mod.rs` | `driver/mod.rs` |
| `driver/roots.rs` | `driver/roots.rs` |
| `envelope_fit.rs` | `soundfont/envelope/mod.rs` |
| `extract.rs` | `bank/extraction.rs` |
| `extract_cli.rs` | `cli/extraction.rs` |
| `gain_fit.rs` | `soundfont/gain.rs` |
| `lib.rs` | `lib.rs` |
| `machine/adsr.rs` | `machine/spu/voice/envelope.rs` |
| `machine/bank.rs` | `machine/audio/bank.rs` |
| `machine/boot.rs` | `machine/boot.rs` |
| `machine/bus.rs` | `machine/bus.rs` |
| `machine/cd_audio.rs` | `machine/cd/audio.rs` |
| `machine/cd_data.rs` | `machine/cd/data.rs` |
| `machine/cd_dma.rs` | `machine/cd/dma.rs` |
| `machine/cd_drive.rs` | `machine/cd/drive.rs` |
| `machine/cd_host.rs` | `machine/cd/host.rs` |
| `machine/cd_position.rs` | `machine/cd/position.rs` |
| `machine/cd_queue.rs` | `machine/cd/queue.rs` |
| `machine/cop0.rs` | `machine/cpu/cop0.rs` |
| `machine/cpu.rs` | `machine/cpu/mod.rs` |
| `machine/cues.rs` | `machine/audio/cues.rs` |
| `machine/dma.rs` | `machine/dma.rs` |
| `machine/effects.rs` | `machine/audio/effects.rs` |
| `machine/event_calls.rs` | `machine/kernel/event/calls.rs` |
| `machine/events.rs` | `machine/kernel/event/mod.rs` |
| `machine/exception_calls.rs` | `machine/kernel/exception/calls.rs` |
| `machine/exception_chains.rs` | `machine/kernel/exception/chains.rs` |
| `machine/executable.rs` | `machine/image/executable.rs` |
| `machine/execution.rs` | `machine/execution.rs` |
| `machine/firmware.rs` | `machine/image/firmware.rs` |
| `machine/gaussian.rs` | `machine/spu/coefficients.rs` |
| `machine/interconnect/cache.rs` | `machine/interconnect/cache.rs` |
| `machine/interconnect/cd.rs` | `machine/interconnect/cd.rs` |
| `machine/interconnect/mapped.rs` | `machine/interconnect/mapped.rs` |
| `machine/interconnect/memory.rs` | `machine/interconnect/memory.rs` |
| `machine/interconnect.rs` | `machine/interconnect.rs` |
| `machine/interrupts.rs` | `machine/interrupts.rs` |
| `machine/kernel.rs` | `machine/kernel/mod.rs` |
| `machine/mod.rs` | `machine/mod.rs` |
| `machine/music.rs` | `machine/audio/music.rs` |
| `machine/music_progress.rs` | `machine/audio/progress.rs` |
| `machine/output_clock.rs` | `machine/clock.rs` |
| `machine/profile.rs` | `machine/image/profile.rs` |
| `machine/spu_clock.rs` | `machine/spu/clock.rs` |
| `machine/spu_dma.rs` | `machine/spu/dma.rs` |
| `machine/spu_mixer.rs` | `machine/spu/mixer.rs` |
| `machine/spu_noise.rs` | `machine/spu/noise.rs` |
| `machine/spu_reverb.rs` | `machine/spu/reverb.rs` |
| `machine/spu_sample.rs` | `machine/spu/sample.rs` |
| `machine/spu_transfer.rs` | `machine/spu/transfer.rs` |
| `machine/spu_voice_ports.rs` | `machine/spu/voice/ports.rs` |
| `machine/spu_voices.rs` | `machine/spu/voice/mod.rs` |
| `machine/spu_volume.rs` | `machine/spu/volume.rs` |
| `machine/thread_context.rs` | `machine/kernel/thread.rs` |
| `machine/timers.rs` | `machine/timers.rs` |
| `machine/xa_filter.rs` | `machine/cd/filter.rs` |
| `main.rs` | `main.rs` |
| `manifest.rs` | `document/manifest.rs` |
| `mapping.rs` | `catalog/loader.rs` |
| `midi.rs` | `interchange/midi.rs` |
| `music_document.rs` | `music/document.rs` |
| `music_extract.rs` | `music/extraction.rs` |
| `music_manifest.rs` | `music/manifest.rs` |
| `music_pack.rs` | `music/packing.rs` |
| `pack.rs` | `packing.rs` |
| `pack_cli.rs` | `cli/packing.rs` |
| `pc_archive.rs` | `render/pc/archive.rs` |
| `pc_render.rs` | `render/pc/mod.rs` |
| `psx_render.rs` | `render/psx.rs` |
| `publication.rs` | `publication.rs` |
| `quantization.rs` | `codec/quantization.rs` |
| `render_cli.rs` | `cli/rendering.rs` |
| `sample_edit.rs` | `sample/editing.rs` |
| `sample_pack.rs` | `bank/packing.rs` |
| `sample_reference.rs` | `sample/reference.rs` |
| `sequence.rs` | `sequence/mod.rs` |
| `sequence_clock.rs` | `sequence/clock.rs` |
| `sequence_edit.rs` | `sequence/editing.rs` |
| `sequence_end.rs` | `sequence/termination.rs` |
| `sequence_events.rs` | `sequence/events.rs` |
| `sequence_loops.rs` | `sequence/loops.rs` |
| `sequence_midi.rs` | `sequence/midi.rs` |
| `sequence_timeline.rs` | `sequence/timeline.rs` |
| `sequence_timing_edit.rs` | `sequence/timing.rs` |
| `soundfont.rs` | `soundfont/mod.rs` |
| `soundfont_assignment_pack.rs` | `soundfont/packing/assignment.rs` |
| `soundfont_empty.rs` | `soundfont/silence.rs` |
| `soundfont_pitch_pack.rs` | `soundfont/packing/pitch.rs` |
| `soundfont_program_pack.rs` | `soundfont/packing/program.rs` |
| `soundfont_read.rs` | `soundfont/reader/mod.rs` |
| `soundfont_sample_pack.rs` | `soundfont/packing/sample.rs` |
| `soundfont_stopped.rs` | `soundfont/envelope/snapshot.rs` |
| `soundfont_tables.rs` | `soundfont/tables.rs` |
| `soundfont_tone_pack.rs` | `soundfont/packing/tone.rs` |
| `tuning.rs` | `voice/tuning.rs` |
| `verify.rs` | `verification.rs` |
| `voice_gain.rs` | `voice/gain.rs` |
| `wave.rs` | `interchange/wave.rs` |
| `xa.rs` | `xa/mod.rs` |
| `xa_cue.rs` | `xa/cue.rs` |
| `xa_encode.rs` | `xa/encoder.rs` |
| `xa_extract.rs` | `xa/extraction.rs` |
| `xa_manifest.rs` | `xa/manifest.rs` |
| `xa_pack.rs` | `xa/packing.rs` |
| `xa_rebuild.rs` | `xa/reconstruction.rs` |
| `xa_reference.rs` | `xa/reference.rs` |
| `xa_selection.rs` | `xa/selection.rs` |
| `xml.rs` | `document/xml.rs` |

#### Dependency and API audit

The 125-module import/qualified-path inventory has 434 directed edges and three
cyclic groups (`out/audio-migration/layout-dependencies.json`). It is lexical
scouting; owning imports and type definitions were then inspected for the groups
below. The two `super::*` uses belong to publication's local tests and the
interconnect's own mapped-memory implementation, not cross-domain APIs. No
parser/codec dependency on CLI or rendering orchestration was found.

| Baseline cycle | Required split during its move group |
| --- | --- |
| `catalog`, `bank_content`, `mapping`, `cue`, `xa_cue` | Move passive catalog/report records into `catalog/model.rs`; keep collection, resolution and execution in their domain modules. Move content-identity resolution to `catalog/content.rs`. |
| `bank_soundfont`, `soundfont_empty` | Make silent-program creation take present program IDs, sample rate, SF2 bank and alias choice, returning its sample/instrument indices and program records. The binder owns report mutation and diagnostics; the silence helper must not import binder options/reports. |
| `soundfont_read`, `soundfont_tables` | Move hydra record types into `soundfont/reader/model.rs`. Reader and table parser both import that model; the model imports neither implementation. |

These are implementation obligations, not claims that the cycles are already
removed. Validate the affected group after the split and move; do not combine
musical/runtime behavior changes with relocation. Current passive type API
moves override the module-wide rule above:

| Baseline owner and types | Destination API |
| --- | --- |
| `catalog::{Catalog, Source, Entry, Asset, AssetData, XaStream, SectorRange}` | `catalog::model`, same type names |
| `mapping::{Layout, Slot, Report, Reference, Association}` | `catalog::model`, same type names |
| `cue::{MusicFile, DiscEvidence, MusicMap, Cue}` | `catalog::model`, same type names |
| `xa_cue::{XaCue, Binding, CueMapping, XaMap, SchedulerEvidence}` | `catalog::model`, same type names |
| `bank_content::{Content, Group}` | `catalog::model`, same type names |
| `soundfont_read::{Generator, Modulator, Zone, Preset, Instrument, Sample}` | `soundfont::reader::model`, same type names |

Keep passive record helpers with their types and orchestration methods with
their operations; inherent Rust implementations may live separately from their
type. Preserve serialization and validation behavior. The listed catalog type
names do not collide. The reader's `Font`, `Limits` and `Chunk` stay with the
reader; the authored SoundFont types remain separate. Silent-program `Program`
records stay in `soundfont::silence`; its private helper change adds no public
compatibility surface.

Move the compiled disc-evidence asset `src/us_music.json` to
`src/catalog/music.json` with its consumer and update `include_str!`; preserve
its bytes/hash. The driver guest source does not exist yet and has no current
file to rename. The map establishes its owner, not extraction acceptance.

#### Test and example discovery

Cargo metadata currently reports one library, one binary, 104 integration test
targets and 14 runnable examples. The map covers all 126 test Rust files and 16
example Rust files, including 22 test helpers/children and two example support
modules. It preserves the binary name `bof3-audio` and every test function,
ignore gate and fixture; target/module names change with their owners.

Use noun-named integration roots for each top-level test domain. Each root
registers its mapped child modules with normal `mod` or `#[path]` declarations;
add intermediate `mod.rs` declarations where needed. Existing parent test files
can own both tests and children. These files make Cargo discover tests, not
provide legacy API facades. Do not leave a moved test undiscovered or duplicate
it under an old root. Compare discovered test names/counts and ignore status
before and after each group, then run the group's meaningful media checks.

The examples remain 14 runnable noun-named targets; only their eight compound
names change. `support/events.rs` remains included support; the shared observer
moves to `src/driver/trace.rs` for examples and XA evidence tests. Neither is a
standalone target. Fix relative `#[path]` imports and local module consumers
together. Python reference/validation scripts and JSON fixtures keep
their current locations and bytes; update any references from moved Rust files.
Use fresh Cargo metadata and package-build verification to check discovery.
The source/test moves for one group occur together; later groups remain at
current paths until their phase dependencies permit relocation.

Paths below are relative to `tools/rust/bof3-audio`.

| Baseline test/example | Destination |
| --- | --- |
| `tests/adpcm.rs` | `tests/codec/adpcm/decoder.rs` |
| `tests/adpcm_corpus.rs` | `tests/codec/adpcm/corpus.rs` |
| `tests/adpcm_encode.rs` | `tests/codec/adpcm/encoder.rs` |
| `tests/adsr.rs` | `tests/machine/spu/voice/envelope.rs` |
| `tests/bank_content.rs` | `tests/catalog/content.rs` |
| `tests/bank_runtime.rs` | `tests/machine/audio/bank.rs` |
| `tests/bank_silence.rs` | `tests/soundfont/silence/binding.rs` |
| `tests/bank_soundfont.rs` | `tests/soundfont/bank.rs` |
| `tests/bank_stopped.rs` | `tests/soundfont/envelope/snapshot.rs` |
| `tests/boot.rs` | `tests/machine/boot.rs` |
| `tests/cache.rs` | `tests/machine/interconnect/cache.rs` |
| `tests/catalog.rs` | `tests/catalog/cases.rs` |
| `tests/cd_audio.rs` | `tests/machine/cd/audio.rs` |
| `tests/cd_delivery.rs` | `tests/machine/cd/delivery.rs` |
| `tests/cd_drive.rs` | `tests/machine/cd/drive.rs` |
| `tests/cd_drive_runtime.rs` | `tests/machine/cd/drive/runtime.rs` |
| `tests/cd_edc.rs` | `tests/codec/edc.rs` |
| `tests/cd_host.rs` | `tests/machine/cd/host.rs` |
| `tests/cd_host_runtime.rs` | `tests/machine/cd/host/runtime.rs` |
| `tests/cd_lifecycle.rs` | `tests/machine/cd/lifecycle.rs` |
| `tests/cd_queue.rs` | `tests/machine/cd/queue.rs` |
| `tests/cd_sector.rs` | `tests/machine/cd/sector.rs` |
| `tests/cd_sector_runtime.rs` | `tests/machine/cd/sector/runtime.rs` |
| `tests/cd_selection.rs` | `tests/machine/cd/selection.rs` |
| `tests/cli.rs` | `tests/cli/cases.rs` |
| `tests/common/assignment_corpus.rs` | `tests/support/corpus/assignment.rs` |
| `tests/common/assignment_music.rs` | `tests/support/music/assignment.rs` |
| `tests/common/bank_soundfont.rs` | `tests/support/soundfont.rs` |
| `tests/common/music.rs` | `tests/support/music/mod.rs` |
| `tests/common/music_progress_runtime.rs` | `tests/support/runtime/progress.rs` |
| `tests/common/pitch_corpus.rs` | `tests/support/corpus/pitch.rs` |
| `tests/common/pitch_music.rs` | `tests/support/music/pitch.rs` |
| `tests/common/timing_corpus.rs` | `tests/support/corpus/timing.rs` |
| `tests/common/timing_music.rs` | `tests/support/music/timing.rs` |
| `tests/common/timing_runtime.rs` | `tests/support/runtime/timing.rs` |
| `tests/common/tone_controls.rs` | `tests/support/soundfont/controls.rs` |
| `tests/common/tone_corpus.rs` | `tests/support/corpus/tone.rs` |
| `tests/common/unmute_corpus.rs` | `tests/support/corpus/activation.rs` |
| `tests/common/unmute_music.rs` | `tests/support/music/activation.rs` |
| `tests/cop0.rs` | `tests/machine/cpu/cop0.rs` |
| `tests/cpu.rs` | `tests/machine/cpu/cases.rs` |
| `tests/cue.rs` | `tests/catalog/music.rs` |
| `tests/devices.rs` | `tests/machine/devices.rs` |
| `tests/digest.rs` | `tests/digest.rs` |
| `tests/disc.rs` | `tests/archive/disc.rs` |
| `tests/driver/closure.rs` | `tests/driver/closure.rs` |
| `tests/driver/cues.rs` | `tests/driver/cues.rs` |
| `tests/driver/effects.rs` | `tests/driver/effects.rs` |
| `tests/driver/fixtures.rs` | `tests/driver/fixtures.rs` |
| `tests/driver/history.rs` | `tests/driver/history.rs` |
| `tests/driver/limits.rs` | `tests/driver/limits.rs` |
| `tests/driver/preparation.rs` | `tests/driver/preparation.rs` |
| `tests/driver/slots.rs` | `tests/driver/slots.rs` |
| `tests/driver.rs` | `tests/driver.rs` |
| `tests/empty_programs.rs` | `tests/soundfont/silence/programs.rs` |
| `tests/envelope_fit.rs` | `tests/soundfont/envelope/cases.rs` |
| `tests/event_callbacks.rs` | `tests/machine/kernel/event/callbacks.rs` |
| `tests/exception_chains.rs` | `tests/machine/kernel/exception/chains.rs` |
| `tests/exception_dispatch.rs` | `tests/machine/kernel/exception/dispatch.rs` |
| `tests/executable.rs` | `tests/machine/image/executable.rs` |
| `tests/execution.rs` | `tests/machine/execution.rs` |
| `tests/extract.rs` | `tests/bank/extraction.rs` |
| `tests/firmware.rs` | `tests/machine/image/firmware.rs` |
| `tests/gain_fit.rs` | `tests/soundfont/gain.rs` |
| `tests/kernel_events.rs` | `tests/machine/kernel/event/cases.rs` |
| `tests/manifest.rs` | `tests/document/manifest.rs` |
| `tests/manifest_extraction.rs` | `tests/document/extraction.rs` |
| `tests/mapping.rs` | `tests/catalog/loader.rs` |
| `tests/midi.rs` | `tests/interchange/midi.rs` |
| `tests/music_extract.rs` | `tests/music/extraction.rs` |
| `tests/music_pack.rs` | `tests/music/packing.rs` |
| `tests/music_programs.rs` | `tests/music/programs.rs` |
| `tests/output_clock.rs` | `tests/machine/clock.rs` |
| `tests/pack.rs` | `tests/packing.rs` |
| `tests/pc_archive.rs` | `tests/render/pc/archive.rs` |
| `tests/pc_render.rs` | `tests/render/pc/cases.rs` |
| `tests/pitch_context.rs` | `tests/voice/context.rs` |
| `tests/preservation.rs` | `tests/archive/preservation.rs` |
| `tests/program_population.rs` | `tests/soundfont/packing/population.rs` |
| `tests/psx_render.rs` | `tests/render/psx.rs` |
| `tests/runtime.rs` | `tests/machine/runtime.rs` |
| `tests/sample_pack.rs` | `tests/bank/packing.rs` |
| `tests/sample_reference.rs` | `tests/sample/reference.rs` |
| `tests/sequence_clock.rs` | `tests/sequence/clock.rs` |
| `tests/sequence_edit.rs` | `tests/sequence/editing.rs` |
| `tests/sequence_events.rs` | `tests/sequence/events.rs` |
| `tests/sequence_loops.rs` | `tests/sequence/loops.rs` |
| `tests/sequence_midi.rs` | `tests/sequence/midi.rs` |
| `tests/sequence_runtime.rs` | `tests/sequence/runtime.rs` |
| `tests/sequence_timeline.rs` | `tests/sequence/timeline.rs` |
| `tests/sequence_timing_edit.rs` | `tests/sequence/timing.rs` |
| `tests/soundfont.rs` | `tests/soundfont/cases.rs` |
| `tests/soundfont_assignment_pack.rs` | `tests/soundfont/packing/assignment.rs` |
| `tests/soundfont_pitch_pack.rs` | `tests/soundfont/packing/pitch.rs` |
| `tests/soundfont_read.rs` | `tests/soundfont/reader/cases.rs` |
| `tests/soundfont_sample_pack.rs` | `tests/soundfont/packing/sample.rs` |
| `tests/soundfont_tone_pack.rs` | `tests/soundfont/packing/tone.rs` |
| `tests/soundfont_unmute_pack.rs` | `tests/soundfont/packing/activation.rs` |
| `tests/spu_bootstrap.rs` | `tests/machine/spu/bootstrap.rs` |
| `tests/spu_clock.rs` | `tests/machine/spu/clock.rs` |
| `tests/spu_mixer.rs` | `tests/machine/spu/mixer.rs` |
| `tests/spu_noise.rs` | `tests/machine/spu/noise.rs` |
| `tests/spu_output.rs` | `tests/machine/spu/output.rs` |
| `tests/spu_reverb.rs` | `tests/machine/spu/reverb.rs` |
| `tests/spu_samples.rs` | `tests/machine/spu/sample.rs` |
| `tests/spu_transfer.rs` | `tests/machine/spu/transfer.rs` |
| `tests/spu_voice_ports.rs` | `tests/machine/spu/voice/ports.rs` |
| `tests/spu_voices.rs` | `tests/machine/spu/voice/cases.rs` |
| `tests/spu_volume.rs` | `tests/machine/spu/volume.rs` |
| `tests/thread_context.rs` | `tests/machine/kernel/thread.rs` |
| `tests/timers.rs` | `tests/machine/timers.rs` |
| `tests/tuning.rs` | `tests/voice/tuning.rs` |
| `tests/voice_gain.rs` | `tests/voice/gain.rs` |
| `tests/wave.rs` | `tests/interchange/wave.rs` |
| `tests/xa.rs` | `tests/xa/cases.rs` |
| `tests/xa_coding.rs` | `tests/xa/coding.rs` |
| `tests/xa_corpus.rs` | `tests/xa/corpus.rs` |
| `tests/xa_cue.rs` | `tests/xa/cue.rs` |
| `tests/xa_encode.rs` | `tests/xa/encoder.rs` |
| `tests/xa_extract.rs` | `tests/xa/extraction.rs` |
| `tests/xa_loop_runtime.rs` | `tests/machine/cd/xa/loops.rs` |
| `tests/xa_pack.rs` | `tests/xa/packing.rs` |
| `tests/xa_rebuild.rs` | `tests/xa/reconstruction.rs` |
| `tests/xa_runtime.rs` | `tests/machine/cd/xa/runtime.rs` |
| `tests/xa_scheduler.rs` | `tests/machine/cd/xa/scheduler.rs` |
| `tests/xa_selection.rs` | `tests/xa/selection.rs` |
| `examples/bank_probe.rs` | `examples/bank.rs` |
| `examples/bios_probe.rs` | `examples/firmware.rs` |
| `examples/closure.rs` | `examples/closure.rs` |
| `examples/coverage.rs` | `examples/coverage.rs` |
| `examples/cues.rs` | `examples/cues.rs` |
| `examples/effects.rs` | `examples/effects.rs` |
| `examples/initialization.rs` | `examples/initialization.rs` |
| `examples/limits.rs` | `examples/limits.rs` |
| `examples/program_init_probe.rs` | `examples/program.rs` |
| `examples/recover_xa.rs` | `examples/recovery.rs` |
| `examples/runtime_probe.rs` | `examples/runtime.rs` |
| `examples/support/events.rs` | `examples/support/events.rs` |
| `examples/support/trace.rs` | `src/driver/trace.rs` |
| `examples/tuning_init_probe.rs` | `examples/tuning.rs` |
| `examples/zero_pitch_probe.rs` | `examples/pitch.rs` |
| `examples/zero_runtime_probe.rs` | `examples/silence.rs` |

## Validation commands and final acceptance

Run the current audio checks before migration and port relevant native coverage.
Use repository-relative commands; store generated build/evidence outputs under
ignored `out/` or temporary directories. Initial baseline commands:

```sh
cmake -S tools/c/psx-audio -B out/audio-migration/c-baseline -DBUILD_TESTING=ON
cmake --build out/audio-migration/c-baseline
ctest --test-dir out/audio-migration/c-baseline --output-on-failure
just check-unit toolchain/test_audio_surface.py
```

Current foundation and integration checks (audio uses its scoped mise pin):

```sh
mise -C tools/rust/bof3-audio exec -- cargo fmt --check
mise -C tools/rust/bof3-audio exec -- cargo check --locked --all-targets
mise -C tools/rust/bof3-audio exec -- cargo test --locked
mise -C tools/rust/bof3-audio exec -- cargo test --locked --manifest-path ../emi-ex/Cargo.toml
BOF3_AUDIO_CORPUS="$PWD/out/extracted" mise -C tools/rust/bof3-audio exec -- cargo test --locked --test archive preservation::local_corpus -- --ignored --nocapture
BOF3_AUDIO_EXE="$PWD/out/extracted/SLUS_004.22" mise -C tools/rust/bof3-audio exec -- cargo test --locked --test executable local_us_executable -- --ignored
BOF3_AUDIO_EXE="$PWD/out/extracted/SLUS_004.22" mise -C tools/rust/bof3-audio exec -- cargo test --locked --test runtime -- --ignored --nocapture
just check-unit toolchain/test_audio_surface.py
just check
bin/harness plans status bof3-audio-rust-migration.md
```

Record concrete corpus/consumer commands and hashes as fixtures and CLI contracts
are implemented; placeholders are not passing evidence. Final acceptance requires:

- Whole-file equality for unchanged EMI and XA corpus round trips, including
  padding, ordering, unrelated content and trailing bytes.
- Supported edited musical/sample semantics and all rejection/publication cases
  in A3/A4; no silent unsupported-event loss.
- Independent compatible-consumer MIDI/SF2 structure and representative playback.
- PSX execution against independently established runtime traces/reference audio,
  with explicit profile support and no fallback to approximate rendering.
- A6 reachability/pruning evidence, shipped audio-only assembly and deterministic
  driver builds; every production operation works without a game executable or
  BIOS, including a clean unpacked-package run and dependency/file-access audit.
- A7 noun-folder/single-noun filename ownership, no legacy module facades, resolved
  references and unchanged behavior after modularization.
- Measured PC/PSX differences before lossy encoding; MP3/Ogg conformance and
  timing including encoder delay; explicit durations, loops, tails and timeout.
- Passing Cargo checks/tests, relevant Python checks, package-build verification,
  repository checks, and post-retirement checks; failures and blockers reported.

For the current sandbox, prefix Cargo checks with
`CARGO_HOME="$PWD/out/audio-migration/cargo-home"` and use `--offline` plus
`--target-dir out/audio-migration/rust` (or `emi-baseline` for the EMI crate).
Cargo runs tests from their crate directory, so the corpus environment path must
be absolute. The earlier relative-path corpus invocation failed before inspection;
the absolute-path invocation passed all 884 files.

Known work still pending: establish independent runtime/audio references, select
and validate Rust delivery codecs, complete interchange and audio execution,
isolate/build the standalone driver, remove production executable/BIOS
dependencies, and finish noun-based Rust modularization. These remain required
work, not accepted limitations or authorization to narrow scope. Only accepted
evidence closes a phase.
