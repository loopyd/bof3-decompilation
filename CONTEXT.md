# BOF3 Context

Canonical context and language for planning work in the BOF3 multi-binary
decompilation workspace.

## Project in brief

This repository works toward rebuilding *Breath of Fire III* (US, PlayStation)
from reviewed binary evidence as readable, period-appropriate C89. It is both a
reverse-engineering workspace and a collection of independently buildable
binary targets. The aim is to understand each target, recover maintainable
source, and make its compiled output match the original bytes; ordinary
application feature development is not the project model.

The game is not one linked program. The disc contains standalone PS-X
executables plus EMI archives whose entries may contain code, data, graphics,
or audio. An EMI entry becomes a source/build target only after evidence shows
that it contains code and it is explicitly promoted. Consequently, plans must
name the binary or EMI entry they affect and must not assume that symbols,
addresses, or identical payloads are interchangeable across targets.

## Planning model

Work normally advances through four distinct stages:

1. **Discover and classify** disc content using generated evidence in `out/`.
2. **Promote** a reviewed code or mixed code/data payload into tracked layout
   and source ownership.
3. **Lift** one function at a time from MIPS assembly into readable C89.
4. **Match** the compiled function against the original binary and retain
   durable findings in tracked configuration, symbols, source, or specs.

Plans should preserve these boundaries:

- Authored source and durable binary layouts are tracked; disc input and
  generated analysis are not.
- `out/` is disposable evidence, never a source of authored truth.
- A successful C build and an exact binary match are different milestones.
- PsyQ routines are external library code and are declared, not decompiled.
- Runtime or layout claims require binary evidence; unresolved conclusions
  remain explicitly provisional.
- Scope is normally one confirmed target and one function at a time.

For operating procedures, see [reverse engineering](docs/reverse-engineering.md)
and [matching](docs/matching.md). For retained technical evidence, start at
[specs](docs/specs/index.md).

## Repository map

| Path | Role |
| --- | --- |
| `src/exe/` | Authored source for standalone PS-X executables. |
| `src/emi/` | Authored source for confirmed EMI code targets. |
| `include/bof3/` | Shared C89, hardware, and PsyQ declarations. |
| `config/splat/` | Tracked binary segment layouts consumed by Splat. |
| `config/symbols/` | Tracked authored and verified symbol information. |
| `config/analysis/` | Reviewed analyzer replay commands and analysis-only type layouts. |
| `asm/` | Reviewed original assembly baselines. |
| `Makefile` | PSX compiler, assembler, object, matching, and verification build rules. |
| `tools/python/` | Implementation of repository automation and the `harness` command surface. |
| `bin/` | Thin command entry points and PSX compiler/binutils adapters. |
| `docs/specs/` | Durable, reviewed format, runtime, program, and data findings. |
| `third_party/` | Pinned source dependencies used for extraction, analysis, and matching. |
| `toolchains/` | Generated/staged compilers, SDKs, and related local tools. |
| `inputs/` | Ignored user-owned disc media and private setup inputs. |
| `out/` | Regenerable extraction and analysis evidence. |
| `build/` | Regenerable compiler objects and local tool products. |

New authored binary layout belongs in `config/`; new recovered code belongs in
the owning target under `src/`; reusable declarations belong in `include/`;
durable findings belong in `docs/specs/`. Generated or exploratory output
belongs in `out/`, not beside authored source.

The intended long-term source shape is binary-first at the ownership boundary:

```text
src/
  exe/<executable>/
  emi/<family>/<archive>/<slot>/
include/bof3/
```

Keep the archive and slot in the path even after a subsystem is understood;
they define the independently loaded binary and prevent accidental cross-target
linkage. Within a target, recovered filenames, types, and shared declarations
may increasingly use game-domain names as evidence improves. New lifts belong
directly under their canonical executable or promoted EMI target.

## Tool roles

| Tool | How this project uses it |
| --- | --- |
| `just` | Provides the short task interface for setup, extraction, building, checks, and formatting. |
| `bin/harness` | Coordinates BOF3-specific discovery, inspection, promotion, lifting, matching, Ghidra sync, assets, and disc operations. |
| Splat / spimdisasm | Splits normalized binaries from tracked layouts and produces the canonical assembly used as matching evidence. |
| `Makefile` | Builds independent PSX objects and drives focused matching/check workflows. |
| Historical GCC + MASPSX/binutils | Reproduces the period PSX compilation and assembly pipeline used for binary matching. |
| PsyQ | Supplies the external headers and libraries expected by the game; its library routines are not lifted. |
| asm-differ | Compares a compiled function with the original instructions during the normal edit/match loop. |
| `emi-ex` and BOF3 disc tools | Extract disc content and unpack EMI containers before classification. |
| m2c | Optionally creates an initial C reconstruction; its output is a hint, not authoritative source. |
| Ghidra or Rizin | Optionally supports deeper control-flow, data, reference, and symbol analysis. |
| decomp-permuter | Optionally searches source variants after a function is already structurally close to matching. |
| Ruff, pytest, and clang-format | Validate and format the Python tooling and authored C source. |

The authoritative chain is original bytes, tracked layout, canonical Splat
assembly, then compiled comparison. Decompiler output and automated source
search assist that chain but do not override it. Tool versions and roles are
recorded in `tools.lock.toml`; supported commands are defined by `just --list`
and `bin/harness --help`.

## Binaries

- **Disc input**: user-owned US BIN/CUE media in `inputs/disc/`; never tracked.
- **PS-X executable**: a header-wrapped executable on disc, including
  `SLUS_004.22` and `LOGO.EXE`.
- **Load image**: the headerless raw image extracted from a PS-X executable.
  Splat and binary matching use this image; PS-X EXE header metadata is kept
  separately.
- **Executable target**: a tracked standalone binary layout under
  `config/splat/exe/` with source under `src/exe/`.

## EMI

- **EMI archive**: a shipped `.EMI` container. It is not a build or
  decompilation target.
- **EMI entry**: one payload extracted from an archive, identified by its disc
  archive path and slot, for example `BIN/BATTLE/BATTLE.EMI#3`.
- **Payload kind**: catalog classification: `ram`, `image`, `audio`, or
  `unresolved`. It is not a claim that an entry is executable.
- **Code status**: review state: `unknown`, `candidate`, `confirmed`, or
  `rejected`. Explicit review and promotion are required for `confirmed`.
- **Promoted target**: a confirmed code or mixed code/data entry with a tracked
  Splat configuration and source directory beneath `src/emi/`.

## Identity and evidence

- **Content group**: entries with the same payload SHA-256, regardless of
  runtime address.
- **Build target**: payload SHA-256, load address, and entry convention.
  Identical bytes loaded at different addresses remain separate targets until
  relocatability and symbol behaviour are proven.
- **Catalog**: generated local evidence in `out/catalog/`, including entry
  facts, classification, and duplicate groups. It is regenerated, not authored.
- **Durable layout**: tracked Splat configurations and symbol files in
  `config/splat/` and `config/symbols/`. Generated analysis output is not
  durable layout state.

## Source and analysis boundaries

- `src/exe/` owns standalone executable source; `src/emi/` owns promoted EMI
  source.
- A lifted function is one `func_XXXXXXXX.c` file; target-local declarations
  belong in its adjacent `internal.h`.
- `config/analysis/` is reproducible analyzer input, not compiled source or a
  substitute for Splat layouts. Generated projects and exports remain under
  `out/analysis/`.
- PsyQ routines are library code: declare and use them, but do not lift them.
- `out/` is the only generated-artifact root: extraction, normalized images,
  catalogs, Splat products, Ghidra state, drafts/diffs, and asset previews.
