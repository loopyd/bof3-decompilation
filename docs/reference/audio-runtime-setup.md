# Audio runtime reference setup

The user selected [PCSX-Redux](https://github.com/grumpycoders/pcsx-redux) as the
behavior reference for the Rust audio runtime, limited to behavior needed by the
audio tool. It is registered in `.gitmodules` at `third_party/pcsx-redux`, pinned
by the superproject gitlink. Setup initializes recursive submodules without
advancing branches or resetting local changes. The initially prepared revision
is `28438546c781fbe372a06399c82bed43ca2c6f4d`; verification records all 43 source
repositories in `out/setup/pcsx-redux.json`.

```sh
just setup --component pcsx-redux
just setup --component bios
bin/harness setup --component pcsx-redux
bin/harness doctor
bin/harness runtime status
```

These focused actions also run during full `just setup`. PCSX-Redux setup prepares
the recursive source, verifies required packages/tools, downloads and validates
SDL3, then builds and installs SDL3 locally before building Redux. Build receipts
and command logs live in `out/setup/`. Doctor separately checks Redux prerequisites
and source integrity, SDL3 source/library hashes and Redux built status, including
its receipt and dynamic linkage. Doctor never downloads, installs or builds.
Upstream build instructions and licenses remain in the submodule's
`README.md`, `LICENSE` and `LICENSES.md` (GPL-2.0 for the emulator). Production
audio rendering remains Rust; external execution is reference evidence, not a
fallback renderer. Source preparation alone is not runtime/audio validation.

Setup automatically applies the user-approved customization in
`inputs/patches/pcsx-redux/debugger.patch` before the Redux build. The patch exposes
guarded native step-into to Lua; it adds no packages. Its SHA-256 is
`cd90b2e8e64aafb7b60da9091ca6ab127d5f9fc6dfc4fa9acccf502308850600`.
The runtime target pins its base revision and reconstructs exact before/after images, accepts
an already applied copy, and rejects partial application or unrelated tracked
edits without resetting them. Doctor and runtime receipts include the patch hash;
old binary receipts fail after customization until setup rebuilds successfully.
See [native boundary limits](../../.pi/skills/psx-emulator/references/native.md).

`bin/harness patch list` inventories `inputs/patches/<target>/` without a checkout;
`list --status` adds applied/pristine/unresolved state. `check`, `apply` and
`revert` use the same framework and target policy as setup/doctor. All commands
default to all targets and patches; repeat `--target NAME` to narrow targets or
use `--patch FILE.patch` with one explicit target. Invalid or unresolved state
exits with code 2. Patch changes invalidate the build receipt until setup rebuilds.

The earlier missing-prerequisite check is retained in
`out/audio-migration/driver-redux-prerequisites.log`. Following user authorization,
reinspection found the approved packages installed; this agent made no APT changes.
The SDL3/Redux builds and independent BIOS/EXE-entry capture now pass. This proves
reference execution is available, not Rust parity or audio fidelity. Any additional
dependencies still require specific authorization under repository `AGENTS.md`.

### Reference-build dependency proposal

The approved native packages are listed by name, as requested by the user.
Setup/doctor check their installed status and required `pkg-config` metadata;
they neither pin nor record package versions:

- `libcapstone-dev`, `libcapstone4`
- `libavcodec-dev`, `libavformat-dev`, `libavutil-dev`, `libswresample-dev`
- `libcurl4-openssl-dev`, `libuv1-dev`

The simulation and package filenames, dependencies and archive SHA-256 values
are retained in `out/audio-migration/redux-review/apt-simulation.txt` and
`packages.txt`. Capstone and FFmpeg copyright metadata were inspected from
archives matching those hashes; the installed Curl/libuv copyright inventories
were also inspected. Capstone includes BSD/LLVM terms and GPL packaging files;
FFmpeg's inventory includes LGPL, GPL and permissive component terms. Curl uses
its curl license with separately listed components; libuv principally uses MIT
terms with component/build/documentation exceptions. These native libraries are
for the separate GPL PCSX-Redux reference executable, not the Rust audio package.

SDL3 is a separate source build from the official
[3.4.16 release](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.16).
The reviewed `SDL3-3.4.16.tar.gz` SHA-256 is
`7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68`.
Its top-level license is [zlib](https://www.libsdl.org/license.php); bundled HIDAPI
and YUV components carry their own retained notices. Build/install it only under
`toolchains/pcsx-redux/sdl3-3.4.16`, using installed build tools and libraries.
Do not install further packages or fetch optional dependencies implicitly.
The earlier 3.2.30 archive was inspected during selection but is not proposed.

Setup requires existing Git, CMake, Make, GCC/G++, pkg-config, dpkg-query and ldd;
the current dependency inspection is Linux/Debian-specific. It does not invoke
APT. SDL3 builds from the hash-checked archive under its project prefix; existing
source changes reject rather than being overwritten. The shared Release build
disables SDL tests/examples and retains available audio/video backends. Redux
uses its pinned upstream Release Makefile and the local SDL3 pkg-config/library
paths. Archive, source and resulting library hashes identify SDL3; gitlinks,
binary/SDL hashes, named dependencies and command arguments identify Redux.
These endpoint checks do not prove reproducible binaries or complete host closure.

Use the [psx-emulator skill](../../.pi/skills/psx-emulator/SKILL.md) and
[runtime mission reference](../../.pi/skills/psx-emulator/references/runtime.md)
for maintained Lua scripts and bounded `runtime run` commands. The harness owns
process cleanup and capture validation; audio comparisons remain Rust. The skill
is generic: missions select a Lua script and named `--argument NAME=VALUE` inputs;
loading a PS-X EXE is optional. The `target.lua` mission captures state at a chosen
PC, defaulting to an EXE's entry when supplied. Runtime records any supplied
512 KiB BIOS hash; the US-only acquisition and doctor policy below is unchanged.
Live smoke
and US BIOS handoff evidence is retained in `out/audio-migration/redux-smoke/`,
`redux-boot/` and `redux-boot-verified/`; build/check logs are under
`out/audio-migration/redux-review/`. Production rendering remains entirely Rust.
Generic smoke, BIOS-only target and explicit EXE-target missions also passed in
`redux-generic-smoke/`, `redux-generic-bios/` and `redux-generic-exe/`. New runtime
receipts and capture schemas use the `psx.runtime-*` namespace; historical
receipts remain unchanged and are not accepted by a compatibility path.

## US BIOS

The user authorized this [Archive.org collection](https://archive.org/download/sony-playstation-biosimages242016-10-21/Sony%20-%20PlayStation%20-%20BIOS%20Images/).
The user subsequently restricted preparation to the US `ps-30a.bin` image.
The downloader in `tools/python/harness/toolchain/bios.py` embeds the URL,
archive size/SHA-1 from [item metadata](https://archive.org/metadata/sony-playstation-biosimages242016-10-21)
(retrieved 2026-09-24), and the ROM's 524,288-byte size and SHA-256
`11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef`.
Setup downloads only `ps-30a.7z`, checks the archive and extracted ROM bytes, and
publishes `scph5501.bin`. This revision serves SCPH-5501 (also SCPH-7003 in the
[BIOS version table](https://psx-spx.consoledev.net/kernelbios/)); the filename
denotes the selected US model, not a unique physical console serial number.
The other 23 previously prepared images and cached
archives were discarded after verification; the hash receipt remains in
`out/audio-migration/bios-discard-receipt.json`. Publisher hashes identify these
inputs; they are not independent hardware authenticity evidence.

- Download cache: `toolchains/downloads/bios/ps-30a.7z`.
- Prepared BIOS: `inputs/external/bios/scph5501.bin`.

Both locations are ignored and excluded from the audio source package. Extraction
uses the existing `7z` tool and validates archive member paths. The complete
collection is staged before publication; download/extraction failures preserve
an existing collection. Repeating setup verifies and reuses prepared files offline.
`--force` refreshes downloads against the same pins and verifies identical results;
it does not replace a conflicting or damaged collection silently.

No collection configuration or inventory is required. `just doctor` runs the
harness doctor's `US BIOS` check, scanning regular files under
`inputs/external/bios/` (including subdirectories) for the expected SHA-256.
Filename and extension do not matter; unrelated files and stale inventory text
do not affect a matching ROM. Symlinks are skipped. A missing matching hash or
an unreadable scan with no match fails without downloading or modifying files.
This verifies the input bytes, not successful BIOS bootstrap.

The previous absence of a local BIOS dump is superseded by this prepared
US image. BIOS bootstrap, device timing and independent audio comparison still
require validation under the [migration plan](../plans/bof3-audio-rust-migration.md).
