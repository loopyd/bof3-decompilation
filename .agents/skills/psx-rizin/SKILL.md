---
name: psx-rizin
description: "Create and maintain reproducible Rizin or radare2 analysis projects for PlayStation 1 MIPS binaries and arbitrary mapped code/data blobs, including PS-X EXEs, overlays, library members, embedded code ranges, rz-ghidra/r2ghidra decompilation, cross-binary function/type correlation, symbol mapping, xrefs, and deterministic evidence export. Use for PSX reverse engineering, raw MIPS analysis, persistent analyzer projects, shared-function or shared-type discovery, PsyQ signature correlation, or Rizin/radare2 automation."
---

# PSX Rizin Analysis

Use the current installed Rizin command surface when available and modern
radare2 as a version-checked fallback. Do not teach historical commands as the
default merely because the projects share ancestry. Treat the analyzer project
as a disposable workbench: original bytes, reviewed binary layouts, tracked
replay commands, and compiled source remain authoritative.

## Required reading

- Read [psx-inputs.md](references/psx-inputs.md) before opening a PS-X EXE,
  normalized executable image, or extracted overlay.
- Read [commands.md](references/commands.md) before native Rizin/radare2 work.
- Read [projects-and-replay.md](references/projects-and-replay.md) before saving
  projects or promoting analyzer state.
- Read [decompilers.md](references/decompilers.md) before using rz-ghidra or
  r2ghidra.
- Read [evidence.md](references/evidence.md) before promoting names, types,
  boundaries, constants, PsyQ identities, or cross-target matches.
- Read [symbol-type-mapping.md](references/symbol-type-mapping.md) when mapping
  semantic binary/PsyQ evidence back to address symbols or adding typedef aliases.
- Read [cross-binary-correlation.md](references/cross-binary-correlation.md) when
  finding shared functions, compatible signatures, types, constants, or library
  identities across independently mapped blobs.
- Read [github-research.md](references/github-research.md) when inspecting
  upstream versions, releases, source, or issues with GitHub CLI.
- Read [rzpipe-automation.md](references/rzpipe-automation.md) before building
  batch correlation, deterministic export, project verification, or analyzer
  regression tooling around Rizin/radare2.

## Core workflow

1. Identify one exact input and record its hash, kind, target identity, runtime
   load address, and known entry point or function boundary.
2. Detect tools and versions. In this repository run
   `bin/harness analysis doctor`; natively record `rizin -V` or `r2 -v` and
   probe required commands with `<command>?`.
3. Open raw input as MIPS, 32-bit, little-endian at its verified load address.
   Never open a container archive as code. Validate several known instructions
   before analysis.
4. Start with bounded analysis. Define or correct reviewed functions and data,
   then deepen reference/string analysis only when needed. Do not accept bulk
   analyzer guesses as facts.
5. Replay reviewed target-local function names, data flags, comments, C types,
   and type placements from tracked inputs. Keep address-based names until the
   semantic name is proven.
6. Use strings, xrefs, call shapes, access widths, repeated layouts, and
   official SDK declarations to form hypotheses. Check each promoted fact
   against disassembly or raw bytes.
7. Use rz-ghidra/r2ghidra only after function boundaries and calling context are
   credible. Decompiler output is a control-flow and naming hint, not matching
   authority or a compilable-source guarantee.
8. Save generated project state under the generated-artifact root. Export JSON
   deterministically, sort by address/name, and ensure a clean input plus
   tracked replay/types can reproduce the reviewed state.

## Repository adapter

For this repository prefer the adapter for repeatable operations so
engine/version differences remain in one place. Drop to the detected native
engine for focused interactive work or commands the adapter does not expose,
then capture reviewed results in replay rather than relying on session state:

```sh
bin/harness analysis doctor
bin/harness analysis init <target>
bin/harness analysis query <target> functions
bin/harness analysis query <target> strings
bin/harness analysis query <target> xrefs
bin/harness analysis export <target>
bin/harness analysis graph [target]
```

Projects belong under `out/analysis/projects/`; deterministic exports belong
under `out/analysis/exports/`. Reviewed replay commands live under
`config/analysis/<target>.r2`; analysis-only shared C types live in
`config/analysis/bof3_objects.h`. Reinitialize after changing tracked replay or
types. Do not hand-edit generated exports.

## Naming and PsyQ policy

- Keep separate projects and target-local bindings for separate executables or
  overlays, even when bytes or addresses repeat.
- Keep compiled symbols address-based (`func_XXXXXXXX`, `DAT_XXXXXXXX`) until a
  meaning is reviewed. Add semantic aliases without losing address traceability.
- Promote a PsyQ identity only when the official SDK prototype, call shape, and
  assembly agree. A library name does not prove a runtime address.
- Record PsyQ archive/header provenance and target-local address independently.
  Do not lift library code or assume the same offset across binaries.
- Preserve unknown struct fields and exact offsets. Do not force a decompiler
  guess into a public type.

## Acceptance invariant

Analyzer state is reproducible only when the exact input bytes, load/architecture
settings, tracked reviewed replay, and tracked type inputs recreate the reviewed
names, comments, functions, and deterministic exports. A native project database
alone is not durable evidence.
