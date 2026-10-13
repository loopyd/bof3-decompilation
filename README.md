# BOF3 reverse-engineering workspace

Recover independently loaded *Breath of Fire III* binaries as readable C89,
using target-qualified evidence and byte checks.

## Prerequisites

- **Linux on x86_64.** Managed compiler and SDK binaries require this host.
- `just` and `uv` (used by `just setup` to create the Python 3.12+ virtual
  environment)
- `git` (fetches submodules), `cargo` (builds the local Rust
  tools), and `meson` (>= 0.58.0) with a host C compiler (`gcc` or `clang`)
  and `ninja` (>= 1.8.2) to build the Rizin toolchain
- `cmake` (>= 3.20) — every lift verification rebuilds the authored object
  through CMake; `bin/harness lift asm-diff` and `bin/harness build` fail closed without it
- `7z` — first-time setup downloads and extracts a staged PsyQ `.7z`, and the
  same host tool extracts the `BreathOfFireIIIv1.1.7z` media archive when no
  already-extracted CUE/BIN set exists
- A licensed CUE/BIN set of *Breath of Fire III* (US release) under
  `inputs/external/`
- Internet access for the first-time toolchain download

## Quick start

```sh
just setup
just doctor
```

`just setup` initializes retained dependencies, user-authorized media, and
local toolchains. `just doctor` verifies the result. See
[Tool usage](docs/agents/tool-usage.md) for the complete ordered workflow.

Work on an extracted executable image or EMI entry — never the EMI archive.
Original bytes and target manifests are the source of truth.

## Lift one function

```sh
bin/harness source splat TARGET
bin/harness lift m2ctx TARGET@0xADDRESS
bin/harness lift m2c TARGET@0xADDRESS -o out/candidate.c
# Edit the metadata-owned lift source under src/bof3/<subsystem>/.
bin/harness lift asm-diff TARGET@0xADDRESS
bin/harness lift byte-match TARGET@0xADDRESS
```

Lift identity and target ownership come from explicit manifest claims, maps,
Splat, and parsable `@source`/`@behavior` metadata — never directory ancestry
or filenames. Read [Function matching](docs/agents/matching.md) before
proposing a lift.

## Status

```sh
bin/harness lift status [TARGET...]  # matching status of tracked lifts
bin/harness source symbols check    # symbol maps and naming debt
just check                          # practical validation gate
```

## Documentation

Start with [docs/INDEX.md](docs/INDEX.md) to find the guide for your task.

| Task | Reference |
| --- | --- |
| Complete ordered tool workflow | [Tool usage](docs/agents/tool-usage.md) |
| Build and run PS1 emulator missions | [Emulator integration](docs/agents/pcsx-redux.md) |
| Build and use the PS1 decompiler | [rz-psxdec](third_party/rz-psxdec/README.md) and [workspace lifecycle](tools/retdec-psx/README.md) |
| Lift and match one function | [Function matching](docs/agents/matching.md) |
| Resolve asm-diff symptoms | [Matching playbook](docs/agents/matching-playbook.md) |
| Memory macros and qualifiers | [Memory API](docs/agents/memory-api.md) |
| Understand target identity and ownership | [Context](docs/agents/project-context.md) |
| Read reviewed format/runtime/data findings | [Specs](docs/specs/) |
| Avoid known reverse-engineering mistakes | [Lessons](docs/agents/lessons.md) |

Run commands with `--help` or `--example` for usage. Read
[CONTRIBUTING.md](CONTRIBUTING.md) before contributing.
