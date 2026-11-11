# BOF3 audio Rust tools

This source package preserves repository paths. It includes `bof3-audio`, its
local `emi-ex-v2` dependency, lockfiles, the audio project’s mise toolchain pin,
tests, authored guest driver inputs and license notices.
No game executable, disc sectors, BIOS image or extracted media is included.
The authored tooling and driver use the MIT terms in `LICENSE`; third-party
terms are retained in `THIRD_PARTY_LICENSES.txt` beside this file.

All audio-crate tests, test helpers and reference models are Rust. Python tests
belong only in the repository harness for command integration and packaging;
the source packager rejects Python files inside the crates. Explicitly ignored
Rust conformance tests may use installed independent consumers such as FFmpeg,
Xiph tools, or mpg123. These consumers are test oracles, never production codecs.

The local `mise.toml` pins Rust 1.98.1 independently of other projects and global
settings. The crate’s minimum supported Rust version remains 1.88. With mise
installed, run these commands from the repository or extracted package root:

```sh
mise trust tools/rust/bof3-audio/mise.toml
mise -C tools/rust/bof3-audio install rust
mise -C tools/rust/bof3-audio exec -- cargo build --locked --release
mise -C tools/rust/bof3-audio exec -- cargo test --locked
tools/rust/bof3-audio/target/release/bof3-audio --help
```

Cargo obtains the exact registry versions in the lockfile. With those crates
already cached, add `--offline` to build/test. Registry dependencies are not
vendored. Media-dependent tests are explicitly ignored unless their documented
environment variables are supplied. The package does not include the Python
harness; invoke Cargo through the scoped mise configuration and the binary directly
after extraction. Run the trust and install steps before using the harness; it
does not trust project files automatically. Mise’s
[Rust integration](https://mise.jdx.dev/lang/rust.html) owns toolchain selection.

Build the guest driver with GNU Make, `sha256sum` and the existing approved MIPS
GNU binutils. The [driver instructions](driver/README.md) give the portable
build command and artifact contract. No tool binaries are bundled.

In the full repository, `bin/harness audio build` builds the host and guest
driver, then prints the host binary path. The driver artifacts are written to
`out/audio/driver/`. `bin/harness audio index|query|map|extract|render|pack|compile|verify ...`
builds both and forwards arguments to Rust. `audio build OPERATION ...` remains available.
`audio package [OUT]` writes a deterministic source ZIP, defaulting to
`out/bof3-audio-source.zip`; publication failures preserve an existing ZIP.

`bin/harness setup --component audio` builds the Rust host and owned guest driver.
`bin/harness doctor --component audio` validates the existing host and driver
artifacts without building or requiring BIOS. Production operations are `index`,
`query`, `map`, `extract`, `render`, `pack` and `verify`. Their driver option is
`--driver DIRECTORY`. The separate `compile` development command retains its
original executable input.

```sh
bin/harness audio index --mode music --disc-root out/extracted --json
bin/harness audio verify --mode audio --archive /path/to/archive.EMI --json
bin/harness audio render --help
```

`map` requires `--driver DIRECTORY`. The same option adds US loader, music cue
and XA cue mapping to `index` and `query`. Use
`query --mode music --cue NUMBER --driver DIRECTORY` for music cues `0`–`164`.
These commands use the built owned driver and supplied media. They reject
`--executable` and `--bios`. Reported original addresses and EXE offsets identify
source provenance; they do not require opening that executable.

For bank and sample references used by the original local effects dispatcher, use
`index --mode audio --type sfx --driver out/audio/driver` with your media selection.
Add `--kind bank` or `--kind sample` to select one asset kind. `query` accepts
the same filter and a qualified `--id` from the index. Numeric IDs fail when
several selected assets match.

Each selected bank's `roles` records the qualified auxiliary table, reference
rows and original dispatcher and handler addresses. The scope is
`local_dispatch_reference`. Samples referenced through those rows use
`local_dispatch_sample_reference`. Their `sample_references` retain the row,
ordered layer, program, physical tone, VH sample-reference offset and original
SDK sample-reader address. Physical slots beyond a program's declared tone
count remain eligible. Repeated tone selections remain separate layers.

These roles require one resolved body, one bounded auxiliary table, local
record shapes and populated programs. Sample roles also require bounded cue
attribute reads and an in-bank physical sample reference. Zero references do
not key on; out-of-bank references remain unresolved. A reference can select
a silent tone. Roles do not establish active layout, loaded state, arbitration,
audibility or independent PCM. Shared payloads do not transfer roles.
Unreferenced samples remain unresolved.

Use `index` or `query --mode audio --type vocals --driver out/audio/driver` to select
observed vocal cues. Currently this selects only cue `0x2000` from the complete
original US `BIN/SCE_XA/VOICE.STR`. Its whole-file SHA-256 is
`f4f9d8ce9a2849501e8cd44dfcfee85aad4bc92cd9ff5113acde43c9362c65cc`.
The `source_qualified_content_observation` role records the user's confirmation
of Japanese vocals, the source and decoded WAV hashes, cue and sector identities,
coding byte, language and decoder context. Renaming the complete source preserves
its identity. Other cues, whole streams, altered sources and banks receive no
vocal role. Content observation does not establish the active caller, game
history, audible endpoint or native playback fidelity. Use an unfiltered index
to inspect unresolved content. `extract --type vocals` remains unsupported;
extract a qualified `--kind xa_cue --id ID` from the vocal index instead.

`extract --mode audio --kind bank --type sfx` uses the same bank-role filter.
Use a qualified bank `--id` from the index to select one bank. Extraction retains
every sample in each selected bank, including samples with unresolved roles, so
the complete bank remains editable and packable. Its report records
`selection_role` and `selected_banks` with their source-qualified role evidence.
The WAVs use the explicit export reference rate and do not represent cue playback.

Without `--id` or `--type`, bank extraction also preserves every supplied EMI
file that has no cataloged audio assets. The bank root `audio.xml` uses
`bof3.audio-extraction/v2`. Its `archive` children retain complete source bytes,
lengths and hashes, including reserved table fields, padding and tails. Packing
reconstructs these files unchanged without opening their original paths.
The `bof3.audio-extract-report/v2` report counts all source `archives` and the
subset `preserved_archives` without audio assets. These snapshots do not assign
an audio role. Regenerate older bank roots; standalone `bank.xml` folders retain
their current format. XA roots keep their separate format and workflow.

XA extraction requires `--xa-arithmetic split-floor|combined-rounded` and
`--xa-history zero|continuous`. Zero history starts each asset independently.
Continuous history carries a zero-start decode of preceding sectors of the selected
file/channel/coding stream. Both are export contexts; game seek/reset behavior
remains unverified. Strict XA decoding, extraction and packing reject emphasis.
Native-reference playback accepts coding bit 6 and deliberately omits de-emphasis;
this does not establish hardware fidelity.

XA asset XML uses `bof3.xa-asset/v2`, records initial channel
histories, and validates them against the preserved source during packing. Regenerate
older manifests. Edited partial cues require matching continuous entry history and
original exit history before following audio. Packing rejects shorter WAV input
by default. For XA roots, `pack --xa-padding zero` fills shorter or empty input
with zero targets to the original capacity and reports input and padding loss
separately per channel. Longer input fails. Sector count, cue placement, history
boundaries and overlapping-sector agreement still apply.

Use `extract --kind xa_views --xa-history continuous --driver DIRECTORY`
to retain streams and verified cues in one XA root. Without `--id`, it exports
both kinds; a qualified ID selects one view. Packing accepts both kinds in that
root and publishes one STR only when overlapping views request identical sector
bytes and retain their source-derived entry histories after merging. Update every
selected view of an edited sector consistently. Mixed roots require continuous
history and the owned driver during extraction and packing.

Bank and SoundFont sample packing reports include `loop_reconstruction` with
original and output loop metadata and the applied policy. Unchanged samples use
`preserve_original_encoding`, including loops with carried predictor history.
Edited loops use `stabilize_pcm_loop`: a predictor-zero entry makes decoded PCM
repeat on every traversal. This can replace the original changing loop trajectory;
encoding loss measures the supplied first-pass PCM. Removing a loop or editing a
nonlooping sample uses `reencode_without_loop`.

`verify --json` writes `bof3.audio-verification/v3`. Its `checks` object reports
each category with a `status` of `pass`, `fail`, or `missing_evidence` and a
`reason` describing the evidence or limitation.

| Category | Evidence checked |
| --- | --- |
| `structure` | Container layout accepted by the EMI or XA parser, excluding audio semantics. |
| `identity` | Qualified asset bindings. These remain unverified. |
| `completeness` | A known whole-file XA hash matched against the complete original disc extent. |
| `whole_file` | Every byte and the length compared with `--against FILE`. |
| `translation` | MIDI and SoundFont semantics. No comparison is performed by this command. |
| `encoding` | Independently decoded samples and codec quality. No comparison is performed by this command. |
| `rendering` | Independent runtime traces and pre-lossy PCM. No comparison is performed by this command. |

`byte_equality` retains comparison sizes and the first differing byte.
`media_completeness` retains the known extent and missing-sector details.
An optional `source_profile` records original US provenance supplied by the
validated owned driver. It does not establish the archive's asset bindings.
Matching bytes do not override a completeness failure. Unknown media
and omitted comparisons report `missing_evidence`. Exit code 1 indicates a failed
check; exit code 0 means no performed check failed and does not establish accepted
playback. Invalid inputs fail before an evidence report is produced.

US music rendering executes the source-built owned guest driver through Rust,
using explicit reference clocks. In the full repository:

```sh
bin/harness audio render --mode music --engine psx \
  --archive out/extracted/BIN/BGM/BGM000.EMI \
  --driver out/audio/driver \
  --sequence 0 --loops 2 --tail 2 --output out/music-render --json
```

`--sequence` is an independent SEP index within the named archive. Output is a
new directory containing `render.wav` and `render.json`; the report records
media identities, owned calls, timing limits and unverified fidelity gates.
The `bof3.psx-render/v5` report includes owned startup and artifact hashes.
Reports include `native_output_settings` and `pcm16_delivery`, whose `peak`
records the float peak before conversion and `clipped_samples` counts channel
samples saturated for PCM16 delivery. The renderer fixes native settings to
volume 3, stereo and unmuted. Reports include `native_key_on_delay`, the
configured signed delay in output frames. Production rendering uses six frames.
The cooperative output context starts before owned kernel startup. CPU timing,
synthesis, transfers and counters advance during sound initialization, bank
upload and callback preparation; only PCM collection is discarded.
Header, sequence and auxiliary cue copies from the host-fed staging window to
their live layout addresses execute guest instructions. Preparation reports
include those calls. Source admission and routine entry remain host work and
add no guest cycles.
Collection begins before play without resetting phase or device state.
`capture_start_frame` records that absolute frame origin. Body, progress, stop
and `executed_frames` counts are relative to it; `output_clock` retains absolute
counters. `ram_control_model` records the explicit emulator-reference policy.
It preserves the RAM-control latch while allowing 512 KiB accesses at any latch
value, as the native SPU does. Unconfigured components still require `0x0004`.
Hardware RAM-control modes remain unverified.
These reports do not establish independent native PCM, CPU/worker alignment or
hardware timing.
The PSX branch requires `--driver DIRECTORY`, rejects `--executable`, and no
longer accepts `--bios`. Original-reference execution remains in development
examples and the explicit `machine::reference` owner. In an extracted source
package, build the host and driver as described above, then invoke the host
binary directly with the same render arguments and supplied media.
The renderer automatically selects the first game layout (0, 1, then 2) whose
header, sequence and SPU bank capacities fit the archive. Selection uses archive
sizes and bank slot, never filenames or cue IDs. `--layout 0|1|2` overrides this
choice; oversized uploads fail. The report records requested and selected layouts.
This host capacity policy does not establish original gameplay layout selection.
Default playback traverses infinite sequence loops twice and stops non-looping
sequences at their first end marker. `--loops N` changes the infinite-loop limit;
encoded finite repeat counts stay intact. `--duration SECONDS` selects a fixed
body length instead and cannot be combined with `--loops`. The default release
tail is two seconds, included in the reported 600-second audio safety limit.
If loop/end stopping cannot complete within that limit, rendering fails without
publishing a truncated song. Guest calls also have transition limits, including
guest instructions and service calls.
SFX rendering accepts an explicit SDK note or one local game cue in a fresh bank:

```sh
bin/harness audio render --mode sfx --engine psx \
  --archive out/extracted/BIN/BATTLE/COMN_SE.EMI --driver out/audio/driver \
  --header-entry 0 --body-entry 2 --history cold --duration 0.25 --tail 0.2 \
  --voice 7 --program 0 --tone 0 --key 24 --fine 0 --left 100 --right 100 \
  --output out/sfx-render --json
```

All seven note values are required. They select SDK playback directly and do not
establish a game cue mapping or classify the sample. To execute one local game
cue instead, replace those options with `--auxiliary-entry 1 --cue-row 0`.
The entries and cue row belong to the named archive. Unsupported cross-bank
records and unresolved sample references fail without publishing output.

`--history cold` and a positive `--duration` are required. Repeated-event
histories and multi-bank playback remain unsupported in SFX mode.
The renderer polls game voice status before playback and after the body. It
releases the explicit SDK voice, or all 24 voices for a game cue in the otherwise
empty machine. The fixed tail includes guest release and safe-call delay and may
end before silence. The `bof3.psx-effects-render/v4` report records the selected
bank, playback values, calls, status snapshots, cutoff and clock limits.
Independent native timing and PCM agreement remain open.

XA cues use the separate cold renderer, without an original executable or BIOS:

```sh
bin/harness audio render --mode xa --engine psx \
  --track /path/to/data.bin --toc /path/to/disc-toc.json \
  --driver out/audio/driver --cue 0x2000 --history cold \
  --output out/xa-render --json
```

Supply the whole first DATA track as raw 2352-byte sectors and explicit
`bof3.disc-toc/v1` JSON. The disc directory must contain the three supported XA
source paths. The selected complete STR extent must match its original US hash.
These checks identify that extent, not the whole DATA track. Output is stereo
at 44,100 Hz; incoming and repeated histories remain unsupported.

The TOC gives sector LBAs and exclusive ends. For the original supported US disc:

```json
{
  "schema": "bof3.disc-toc/v1",
  "tracks": [
    {"kind": "data", "pregap_lba": 0, "start_lba": 0, "end_lba": 188324},
    {"kind": "audio", "pregap_lba": 188324, "start_lba": 188474, "end_lba": 204224}
  ],
  "lead_out_lba": 204224
}
```

The DATA track must start at zero and end at the supplied file's sector count.
Subsequent AUDIO tracks supply position metadata; their audio is not attached.

Cue IDs accept decimal or `0x` hexadecimal. `--duration SECONDS` selects a
positive fixed body and pulses the guest cancellation input at its boundary.
Without it, the body ends at guest idle after Pause. `--tail` defaults to two
seconds; `--timeout` defaults to, and cannot exceed, 600 seconds. Publication
waits for idle, retains exactly body plus tail frames and fails on timeout.
The fixed tail may cut queued audio. The `bof3.xa-render/v1` report records
source and TOC identity, cue, commands, cancellation, idle, queue and clock state.
XA rendering uses the cooperative audio clock through startup and playback.
Its report retains the retirement quantum, consumer chunk and actor counts.
XA ticks use a 564,480-interpreter-cycle cadence at safe foreground boundaries.
Production host-call phase, the original audible endpoint and independent PCM
agreement remain unaccepted.

PC rendering accepts the same archive/sequence with `--engine pc`,
`--driver DIRECTORY` and `--allow-approximations`, without BIOS or layout. It shares
music extraction's MIDI/SF2 translation, defaults to two infinite-loop traversals,
retains the intro once and preserves finite loop counts. `--duration` instead
expands enough loops for the requested body. PC MIDI/SoundFont input remains
available and now defaults to one file play, so already-expanded exports are not
repeated again. Explicit `--repeats` requests whole-file restarts only in that
input mode. After the requested file plays, extra fixed-duration time advances
the final synth state without restarting.

Music extraction, PC rendering from an archive and music packing accept
`--pitch-context CONTEXT_JSON` with the owned driver. The
`bof3.audio-pitch-context/v1` document contains the 86-byte intervening pitch-data
gap and its source hashes. Production validates the document and imports only
that gap; it does not load a full RAM image.

The development [`context` example](examples/context.rs) converts a supported US
executable and validated 2 MiB RAM image into this JSON. It checks the original
pitch routine and both tables before writing the context. Reports and song XML
retain the whole RAM, gap, routine and table hashes. Packing requires the matching
context and regenerates the binding from preserved source bytes. Missing or
different context evidence fails. Without this option, adjacent-data pitch
lookups remain rejected. Capture history, gameplay reachability and native audio
fidelity remain unverified.

Add `--format ogg` to a supported render mode to publish `render.ogg` alongside the
inspectable WAV and report. Vorbis uses the pinned Rust encoder at quality 0.7,
with stereo coupling disabled. `--quality 0.7` states that setting explicitly;
other qualities fail. PC Ogg delivery accepts 32000, 44100 and 48000 Hz; PSX
rendering requires 44100 Hz. Neither path converts rates implicitly.
The report's `delivery` object records the encoder,
settings, hashes and exact presentation frames. Its decoded PCM error is not
measured per output. Independent representative comparisons require at least
30 dB SNR and absolute gain error at most 0.01 per nonzero-energy channel.
Silent channels require decoded peak at most one PCM16 unit. This numerical
policy does not establish perceptual transparency or game playback fidelity.
MP3 delivery remains disabled because its full-band noise quality is unresolved.

Build a standalone PS-X EXE for the selected song with the full repository's
canonical GCC 2.7.2 toolchain:

```sh
bin/harness audio compile --mode music \
  --archive out/extracted/BIN/BGM/BGM000.EMI \
  --executable out/extracted/SLUS_004.22 --sequence 0 \
  --disc-root out/extracted --output out/music-player --json
```

The new output directory contains `player.exe`, build sources, embedded assets,
`player.json`, and `build.log`. Music and SFX play from embedded assets. Vocal mode
uses the original US disc, declared with `--disc CUE` and mounted at playback.
The console or emulator supplies the BIOS. The extracted disc root must contain the original
US `BIN/ETC/FIRST.EMI` and `BIN/ETC/DEMO.EMI` for the font and title logo. It defaults
to the executable's directory. The executable must match the supported US profile.
Use `--repo-root PATH` when running a source-package build outside the full checkout.
`--layout 0|1|2` uses the same capacity rules as PSX rendering.

Use `--mode sfx` with one or more DRG EMI banks for the effects view. Cross
dispatches an unambiguous local game cue for the selected physical sample.
Use `--mode vocal --disc CUE` to play the qualified Japanese vocal cue `0x2000`
through the original XA driver. See the [player build instructions](player/README.md)
for the supported media layout, controls, and rebuild commands.

The C89 player retains the original runtime image and calls its audio driver and
8x8 glyph renderer. One note-call hook associates SPU voices with the selected
sequence's 16 MIDI channels. Each channel has a pulsing activity icon and a vertical
meter derived from voice volume and envelope levels, with visual decay. These
meters do not measure mixed PCM. The screen shows 30-frame non-drop SMPTE timecode
(`HH:MM:SS:FF`) and a smaller `bar:beat:tick` display, the EMI basename, the reduced
game logo, player version, and author `deitydragon`. SMPTE advances once per two
NTSC VBlanks. Musical time uses the live driver's elapsed delta and remaining
delay, the selected sequence's PPQN, and its time signature; it follows tempo
changes. Bars and beats start at 1, ticks at 0. Musical time continues across
controller loops and resets on a whole-sequence restart. The glyph renderer
supports separate width and height scaling; the player uses 8x8 and 6x6 glyphs.
Archive basenames must fit 36 ASCII bytes using letters, digits, spaces, periods,
underscores or hyphens. Lowercase and underscore glyphs are copied from the game's
font into its ASCII slots.

Encoded loops continue through the original driver. When a sequence ends, the
player restarts it indefinitely; SMPTE time continues across restarts. The output
retains the complete original US runtime, with the note-call hook, rather than
claiming a separately linked driver. Keep generated media and executable files
local. See [player build instructions](player/README.md) to rebuild the generated
project. Emulator playback does not establish hardware audio fidelity.

Music extraction's `--allow-approximations` also permits known zero-pitch keys
to use silence at key-on while preserving their original sample PCM. Reports
identify these keys. Later pitch changes can activate the PSX source while the
SoundFont zone stays silent; incompatible pitch, sample-assignment and initial
PCM edits fail explicitly during packing.

Music exports include all 128 VAB program slots. Empty programs play silence.
To populate one, assign 1–16 existing tone instruments to its SF2 preset and
make the same assignments to its bank-128 percussion alias, then run `pack`.
Layers must share their source program controls; supported edits to a shared
instrument also affect its original users. New independent instrument/sample
structures remain unsupported. Packing checks VAB, EMI and game layout capacity
before publishing. Current XML policies are required; older exports must be
regenerated. No compatibility export or packing implementation is retained.

The migration is incomplete. Supported preservation, extraction, edited packing
and bounded rendering paths are available. Independent PSX fidelity,
repeated SFX and incoming XA histories, XA timing/PCM acceptance, MP3 delivery and full verification evidence remain
open. Unsupported operations fail explicitly. C source retirement is separately gated; this
package is not a claim of full parity. TUI, live playback, standalone PSF
commands and FLAC are retired from the replacement command surface.

Use [PCSX-Redux Agentic](https://github.com/loopyd/pcsx-redux-agentic) for SPU/CD
implementation comparisons and runtime captures. Agreement with the fork alone
does not establish hardware fidelity.

### Development reference PCM

`Execution::from_boot` keeps original-BIOS timer and blanking ownership. To
advance PCM concurrently, call `enable_reference_output(first_sample_ticks)`
with an explicit first-sample distance of 1 through 768 CPU ticks. The clock
report retains that distance and its absolute activation cycle. Later samples
occur every 768 ticks. Drain the bounded frame queue with `take_audio_frames`.

Instruction accesses precede elapsed transfer and sample work. CD callbacks run
afterward at the existing branch frontier, so a newly admitted feed reaches
subsequent sample work. This functional ordering does not establish native
CPU/worker write visibility or infer a native sample phase. Reference output
leaves the output clock's synthetic line and VBlank counts at zero; the original
root counters retain their own phase. `enable_output` rejects execution created
by `from_boot`; use `enable_reference_output` for this explicit activation.

### Development native audio scheduling

`enable_native_audio(ConsumerClock, Collection)` selects a fresh owned execution
context before any instructions or SPU frames. Configure native sample, ADSR,
noise, disable, reverb, delay and host-output settings first, along with GPU
status and SPU RAM access. This mode owns root counters and keeps the functional
768-tick output clock separate. It does not restore a running native snapshot.

The caller orders guest steps, `step_native_producer` operations and
`native_audio_callback(frames, speed)` chunks. Synthesis admits keys and ordinary
pitch separately for each channel and produces 45-frame batches. Eighteen batches
form an 810-frame submission. Admission requires the queued total to remain below
2048 frames. A blocked submission retains its samples; finish its supplied wait
outcome with `resume_native_enqueue` before producing more samples.

Callback chunks contain 1 through 2048 frames. They consume voice and CD streams,
fill underflow with zeros and discard extra source runs at the selected speed.
`ConsumerClock::Callback` advances the master there. With `ConsumerClock::Null`,
advance it separately through `advance_native_null_clock`. Queue coordinates and
the CPU goal remain available through `native_audio_queue`. A CPU audio wait
returns `DeviceProgress::WaitingForSpu`; advance its consumer clock and step again.
The retained timer access or counter pass resumes once. The owned image executes
`WaitEvent` through its guest body and preserves the selected record across IRQ
save and return. Raw host-service polling remains unsupported in this mode.

Use `Image::start_scheduled` to configure devices before service startup and
`Execution::call_scheduled` for later calls. The scheduler runs after every
advanced transition, including `DeviceProgress::Ready` and the instruction that
returns to the stop address. The stopped check has no scheduler callback; the
observer runs before execution. Every transition, including a blocked wait poll,
counts toward the call limit.

The scheduler may advance independent producer and consumer work at `Ready` or
release an audio wait. It must preserve CPU, clock and cache configuration.
Scheduler errors terminate the call with advanced state retained and prevent
further owned execution. The default scheduler uses `progress.require_ready()`,
which does nothing for `Ready`.

`begin_audio_capture` starts retaining delivered callback frames without changing
synthesis or queue state. Drain them with `take_audio_frames`; collection holds at
most 4096 frames. `native_delivered_frames` counts callback output, including
underflow, independently of producer and master counters. These explicit
operations leave scheduling to the caller.

Production music and cold-bank effects select `enable_cooperative_audio` before
the first owned instruction. This fixed context primes the producer, then runs
consumer-before-producer turns every 200,000 retired opcodes and at CPU audio-goal
waits. Each consumer turn delivers 441 frames at speed 1. Retirement credit waits
for the opcode's device pass to complete; host staging and call boundaries add no
opcodes. Queue state and the retirement remainder persist across guest calls.
Manual actor operations and functional host services reject in this mode.

Native audio contexts reconstruct guest ENVX reads at the CPU access cycle with
the configured key-on delay and Gaussian interpolation mode 2. CPU status polling
therefore advances independently of the synthesis producer's envelope position.
Unstamped host ENVX reads and SPU writes reject in these contexts.

`audio_frames` counts delivered frames for either output model. `audio_clock`
reports the selected model; cooperative reports use
`model: "pcsx_redux_cooperative"` and retain quantum, chunk and actor counters.
`output()` exposes the separate functional clock. Cooperative scheduling defines
a reproducible reference context. Complete native timing and music, SFX and XA
history acceptance remain open.
