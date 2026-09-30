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
path. `bin/harness audio index|query|map|extract|render|pack|verify ...` builds and
forwards arguments to Rust. `audio build OPERATION ...` remains available.
`audio package [OUT]` writes a deterministic source ZIP, defaulting to
`out/bof3-audio-source.zip`; publication failures preserve an existing ZIP.

```sh
bin/harness audio index --mode music --disc-root out/extracted --json
bin/harness audio verify --mode audio --archive /path/to/archive.EMI --json
bin/harness audio render --help
```

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
