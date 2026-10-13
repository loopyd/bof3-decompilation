# Audio runtime reference setup

[PCSX-Redux Agentic](../../third_party/pcsx-redux-agentic/README.md) supplies
emulator observations for the Rust audio runtime. Production audio rendering and
comparison remain Rust. Source preparation or successful execution does not
establish Rust parity, audio fidelity or complete hardware coverage.

## Build and inspect

```sh
just setup --component pcsx-redux
just setup --component bios
bin/harness doctor
bin/harness runtime status
bin/harness runtime missions
```

Full `just setup` includes these components. Emulator setup builds the registered
`third_party/pcsx-redux-agentic` submodule and its recursive pins with local SDL3.
It never advances branches or resets dirty source. Build receipts and logs live
in `out/setup/`; query status and receipts for source/binary identities.

The root stage-0 gitlink, fork HEAD and recursive revisions must agree. Tracked
bytes/modes must match committed source. Unknown or ignored build inputs refuse;
named generated LuaJIT inputs require the exact preceding successful v2 receipt.
Doctor/runtime require matching source, prerequisites, SDL3 hashes and dynamic
linkage. Doctor inspects only; it never downloads, installs or builds.
[Emulator integration](../agents/pcsx-redux-agentic.md) owns build, mission invocation and
skill installation; [Native](../../.pi/skills/psx-emulator/references/native.md)
owns checked boundaries and extension validation.

Select exactly one builtin `--mission NAME` or trusted external `--script PATH`.
Both preload canonical helpers; only external scripts accept noncanonical
`--module NAME=PATH`. Supply data through `--input NAME=PATH` and literal arguments
through `--argument NAME=VALUE`. An EXE is optional. Runtime hashes any supplied
512 KiB BIOS; acquisition/doctor enforce the US policy below. The harness owns
process bounds, cleanup and capture validation. Native customizations belong in
committed fork source and its build receipts.

## Dependencies and licenses

Additional dependencies require specific authorization under `AGENTS.md`.
Setup uses existing Git, CMake, Make, GCC/G++, pkg-config, dpkg-query and ldd;
Linux/Debian package inspection does not invoke APT. Approved native packages:

- `libcapstone-dev`, `libcapstone4`
- `libavcodec-dev`, `libavformat-dev`, `libavutil-dev`, `libswresample-dev`
- `libcurl4-openssl-dev`, `libuv1-dev`

Setup/doctor check installed status and pkg-config metadata; package versions are
neither pinned nor recorded. The fork README owns the full native build requirements.
These libraries serve the separate GPL-2.0 emulator, not the Rust audio package.
Retain fork `LICENSE`/`LICENSES.md` and dependency notices: Capstone includes BSD/
LLVM terms and GPL packaging; FFmpeg includes LGPL/GPL/permissive terms; Curl uses
its curl license with component terms; libuv principally uses MIT with exceptions.

SDL3 uses the official [3.4.16 release](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.16),
archive SHA-256 `7322236cd12090c3eb40b9728be4d49c76f66ad17d04369584d4ecad5cf77c68`.
Its top-level license is [zlib](https://www.libsdl.org/license.php); HIDAPI/YUV
notices remain. Build/install only under `toolchains/pcsx-redux-agentic/sdl3-3.4.16` with
existing tools/libraries; no implicit package installation or optional dependencies.
Changed SDL source refuses overwrite. Shared Release builds disable SDL tests/
examples and retain available audio/video backends. Emulator Make uses the local
SDL3 pkg-config/library paths. Archive/source/library hashes, gitlinks, executable
hashes, dependencies and command arguments identify inputs and products; endpoint
checks prove neither reproducible binaries nor complete host closure.

## US BIOS

Preparation is restricted to the authorized US `ps-30a.bin` from this
[Archive.org collection](https://archive.org/download/sony-playstation-biosimages242016-10-21/Sony%20-%20PlayStation%20-%20BIOS%20Images/).
[BIOS owner](../../tools/python/harness/toolchain/bios.py) pins the URL, archive
size/SHA-1 from [item metadata](https://archive.org/metadata/sony-playstation-biosimages242016-10-21),
and 524288-byte ROM SHA-256
`11052b6499e466bbf0a709b1f9cb6834a9418e66680387912451e971cf8a1fef`.
Setup downloads only `ps-30a.7z`, validates archive/member paths and ROM bytes,
then publishes `scph5501.bin` using existing `7z`. This revision serves SCPH-5501
and SCPH-7003 per the [BIOS table](https://psx-spx.consoledev.net/kernelbios/).
The filename denotes the selected model, not a unique console serial number.
Publisher hashes identify inputs without independent hardware authenticity proof.

- Ignored download cache: `toolchains/downloads/bios/ps-30a.7z`.
- Ignored prepared ROM: `inputs/external/bios/scph5501.bin`.

Both are excluded from the audio source package. Setup stages the single ROM
before publication; failures preserve existing prepared files. Repeated setup
validates/reuses them offline. `--force` refreshes against the same pins; conflicting
or damaged prepared bytes refuse silent replacement. No collection config or
inventory is required.

`just doctor` scans regular files under `inputs/external/bios/`, including
subdirectories, for the expected SHA-256. Names/extensions and unrelated files do
not affect a match; symlinks are skipped. Missing hash or unreadable scan without a
match fails without downloading or modifying files. This validates bytes, not BIOS
bootstrap. Bootstrap, device timing and independent audio comparison require
validation against the [audio contract](../specs/formats/audio.md#ps1-spu-emulation).
