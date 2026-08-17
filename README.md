# BOF3 reverse-engineering workspace

A reverse-engineering project that recovers independently loaded *Breath of
Fire III* binaries as readable C89, backed by target-qualified evidence and byte
checks.

## Prerequisites

- **Linux on x86_64** — the supported host. First-time setup downloads
  Linux-only managed binaries: the canonical compiler is a statically linked
  Linux ELF (`toolchains/gcc-2.7.2-psx/gcc`), PSn00b ships
  `*-linux.zip` artifacts, and the compiler-variant catalog is pinned to
  `linux-x86_64`. macOS and Windows are not supported; a compatibility layer
  (for example WSL or a VM) may work but is not tested.
- `just` and `uv` (used by `just setup` to create the Python 3.12+ virtual
  environment)
- `git` (fetches submodules), `cargo` (builds the local `bof3-disk`/`emi-ex`
  tools), and `meson` (>= 0.58.0) with a host C compiler (`gcc` or `clang`)
  and `ninja` (>= 1.8.2) — Meson's native build backend compiles the Rizin
  toolchain and fails closed without either
- `cmake` (>= 3.20) — every lift verification rebuilds the authored object
  through CMake; `bin/asm-diff` and `bin/build` fail closed without it
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
[Tool usage](docs/usage.md) for the complete ordered workflow.

Work on an extracted executable image or EMI entry — never the EMI archive.
Original bytes and target manifests are the source of truth.

## Lift one function

```sh
bin/splat TARGET
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS -o out/candidate.c
# Edit the metadata-owned lift source under src/bof3/<subsystem>/.
bin/asm-diff TARGET@0xADDRESS
bin/byte-match TARGET@0xADDRESS
```

Lift identity and target ownership come from explicit manifest claims, maps,
Splat, and parsable `@source`/`@behavior` metadata — never directory ancestry
or filenames. Read [Function matching](docs/agents/matching.md) before
proposing a lift.

## Status

```sh
bin/decomp-status [TARGET...]   # live matching status of tracked lifts
bin/symbols check               # symbol maps and naming debt
just check                      # full practical validation gate
```

## Documentation

Start with [docs/index.md](docs/index.md) for the audience-oriented map.

| Task | Reference |
| --- | --- |
| Complete ordered tool workflow | [Tool usage](docs/usage.md) |
| Lift and match one function | [Function matching](docs/agents/matching.md) |
| Resolve asm-diff symptoms | [Matching playbook](docs/agents/matching-playbook.md) |
| Memory macros and qualifiers | [Memory API](docs/agents/memory-api.md) |
| Understand target identity and ownership | [Context](docs/agents/project-context.md) |
| Read reviewed format/runtime/data findings | [Specs](docs/specs/) |
| Avoid known reverse-engineering mistakes | [Lessons](docs/agents/lessons.md) |

Run `--help` or `--example` on commands. Want to help? Read
[CONTRIBUTING.md](CONTRIBUTING.md).
