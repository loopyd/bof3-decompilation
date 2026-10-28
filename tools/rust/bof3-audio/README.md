# BOF3 audio Rust tools

This source package preserves repository paths. It includes `bof3-audio`, its
local `emi-ex-v2` dependency, lockfiles, the audio project’s mise toolchain pin,
tests and dependency license notices.
No game executable, disc sectors, BIOS image or extracted media is included.
The crates declare MIT licensing; third-party terms are retained in
`THIRD_PARTY_LICENSES.txt` beside this file.

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

In the full repository, `bin/harness audio build` builds and prints the binary
path. `bin/harness audio index|query|map|extract|render|pack|compile|verify ...` builds and
forwards arguments to Rust. `audio build OPERATION ...` remains available.
`audio package [OUT]` writes a deterministic source ZIP, defaulting to
`out/bof3-audio-source.zip`; publication failures preserve an existing ZIP.

```sh
bin/harness audio index --mode music --disc-root out/extracted --json
bin/harness audio verify --mode audio --archive /path/to/archive.EMI --json
bin/harness audio render --help
```

For bank and sample references used by the original local effects dispatcher, use
`index --mode audio --type sfx --executable PSX_EXE` with your media selection.
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
Unreferenced samples and XA roles remain unresolved, and `--type vocals` fails
until vocal-role evidence is available.

`extract --mode audio --kind bank --type sfx` uses the same bank-role filter.
Use a qualified bank `--id` from the index to select one bank. Extraction retains
every sample in each selected bank, including samples with unresolved roles, so
the complete bank remains editable and packable. Its report records
`selection_role` and `selected_banks` with their source-qualified role evidence.
The WAVs use the explicit export reference rate and do not represent cue playback.

XA extraction requires `--xa-arithmetic split-floor|combined-rounded` and
`--xa-history zero|continuous`. Zero history starts each asset independently.
Continuous history carries a zero-start decode of preceding sectors of the selected
file/channel/coding stream. Both are export contexts; game seek/reset behavior
remains unverified. XA asset XML uses `bof3.xa-asset/v2`, records initial channel
histories, and validates them against the preserved source during packing. Regenerate
older manifests. Edited partial cues require matching continuous entry history and
original exit history before following audio. Packing rejects shorter WAV input
by default. For XA roots, `pack --xa-padding zero` fills shorter or empty input
with zero targets to the original capacity and reports input and padding loss
separately per channel. Longer input fails. Sector count, cue placement, history
boundaries and overlapping-sector agreement still apply.

Use `extract --kind xa_views --xa-history continuous --executable PSX_EXE`
to retain streams and verified cues in one XA root. Without `--id`, it exports
both kinds; a qualified ID selects one view. Packing accepts both kinds in that
root and publishes one STR only when overlapping views request identical sector
bytes and retain their source-derived entry histories after merging. Update every
selected view of an edited sector consistently. Mixed roots require continuous
history and the executable during extraction and packing.

Bank and SoundFont sample packing reports include `loop_reconstruction` with
original and output loop metadata and the applied policy. Unchanged samples use
`preserve_original_encoding`, including loops with carried predictor history.
Edited loops use `stabilize_pcm_loop`: a predictor-zero entry makes decoded PCM
repeat on every traversal. This can replace the original changing loop trajectory;
encoding loss measures the supplied first-pass PCM. Removing a loop or editing a
nonlooping sample uses `reencode_without_loop`.

`verify --json` writes `bof3.audio-verification/v2`. Its `checks` object reports
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
An optional `runtime_profile` identifies the executable, not the archive's asset
bindings. Matching bytes do not override a completeness failure. Unknown media
and omitted comparisons report `missing_evidence`. Exit code 1 indicates a failed
check; exit code 0 means no performed check failed and does not establish accepted
playback. Invalid inputs fail before an evidence report is produced.

US music rendering executes the supplied BIOS and game sound
runtime through Rust, using explicit reference clocks:

```sh
bin/harness audio render --mode music --engine psx \
  --archive out/extracted/BIN/BGM/BGM000.EMI \
  --executable out/extracted/SLUS_004.22 --bios inputs/external/bios/scph5501.bin \
  --sequence 0 --loops 2 --tail 2 --output out/music-render --json
```

`--sequence` is an independent SEP index within the named archive. Output is a
new directory containing `render.wav` and `render.json`; the report records
media identities, original calls, timing limits and unverified fidelity gates.
The exact supported US executable and SCPH-5501 BIOS hashes are required.
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
publishing a truncated song. Guest calls also have instruction limits.
PC rendering accepts the same archive/sequence with `--engine pc`,
`--executable` and `--allow-approximations`, without BIOS or layout. It shares
music extraction's MIDI/SF2 translation, defaults to two infinite-loop traversals,
retains the intro once and preserves finite loop counts. `--duration` instead
expands enough loops for the requested body. PC MIDI/SoundFont input remains
available and now defaults to one file play, so already-expanded exports are not
repeated again. Explicit `--repeats` requests whole-file restarts only in that
input mode. After the requested file plays, extra fixed-duration time advances
the final synth state without restarting.

Build a standalone PS-X EXE for the selected song with the full repository's
canonical GCC 2.7.2 toolchain:

```sh
bin/harness audio compile --mode music \
  --archive out/extracted/BIN/BGM/BGM000.EMI \
  --executable out/extracted/SLUS_004.22 --sequence 0 \
  --disc-root out/extracted --output out/music-player --json
```

The new output directory contains `player.exe`, build sources, supplied media,
`player.json`, and `build.log`. The player needs no disc after loading; the console
or emulator supplies its BIOS. The extracted disc root must contain the original
US `BIN/ETC/FIRST.EMI` and `BIN/ETC/DEMO.EMI` for the font and title logo. It defaults
to the executable's directory. The executable must match the supported US profile.
Use `--repo-root PATH` when running a source-package build outside the full checkout.
`--layout 0|1|2` uses the same capacity rules as PSX rendering.

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
SFX/XA runtime rendering, MP3/Ogg delivery and full verification evidence remain
open. Unsupported operations fail explicitly. C source retirement is separately gated; this
package is not a claim of full parity. TUI, live playback, standalone PSF
commands and FLAC are retired from the replacement command surface.

Use [PCSX-Redux Agentic](../../../third_party/pcsx-redux-agentic/) for SPU/CD
implementation comparisons and runtime captures. Agreement with the fork alone
does not establish hardware fidelity.
