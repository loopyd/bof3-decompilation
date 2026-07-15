---
name: psx-rizin
description: "Create reproducible stateless Rizin or radare2 analysis evidence for PlayStation 1 MIPS binaries and mapped code/data blobs. Use for raw mapping, bounded analysis, functions, flags, comments, xrefs, graphs, rz-ghidra/r2ghidra, replay, deterministic export, or r2pipe automation; use a domain skill to interpret or promote the resulting evidence."
---

# PSX Rizin Analysis

Use the current installed Rizin command surface when available and modern
radare2 as a version-checked fallback. Do not teach historical commands as the
default merely because the projects share ancestry. Treat every analyzer session
as a disposable workbench: original bytes, reviewed binary layouts, tracked
replay commands, and compiled source remain authoritative.

## Required reading

- Read [psx-inputs.md](references/psx-inputs.md) before opening a PS-X EXE,
  normalized executable image, or extracted overlay.
- Use `$decomp-loop` and its PSX MIPS correctness reference before interpreting
  pipeline hazards, MMIO/DMA, scratchpad/cache, or COP2/GTE instructions. This
  analyzer skill records evidence; it does not own CPU-semantic policy.
- Read [commands.md](references/commands.md) before native Rizin/radare2 work.
- Read [projects-and-replay.md](references/projects-and-replay.md) before
  recording replay or promoting analyzer state.
- Read [decompilers.md](references/decompilers.md) before using rz-ghidra or
  r2ghidra.
- Read [r2pipe-automation.md](references/r2pipe-automation.md) before building
  batch correlation, deterministic export, project verification, or analyzer
  regression tooling around Rizin/radare2.

## Core workflow

1. Identify one exact input and record its hash, kind, target identity, runtime
   load address, and known entry point or function boundary.
2. Detect tools and versions through the stateless
   `harness.analyzer.find_best_engine()` adapter; natively record `rizin -V` or
   `r2 -v` and probe required commands with `<command>?`.
3. Open raw input as MIPS, 32-bit, little-endian at its verified load address.
   Never open a container archive as code. Validate several known instructions
   before analysis.
4. Start with bounded analysis. Define or correct reviewed functions and data,
   then deepen reference/string analysis only when needed. Do not accept bulk
   analyzer guesses as facts.
5. Replay reviewed target-local function names, data flags, comments, C types,
   and type placements from tracked inputs. Keep address-based names until the
   owning domain workflow approves a semantic interpretation.
6. Export strings, xrefs, call shapes, access widths, and repeated layouts as
   bounded supporting evidence. Interpretation and promotion belong to the
   owning domain workflow.
7. Use rz-ghidra/r2ghidra only after function boundaries and calling context are
   credible. Decompiler output is a control-flow and naming hint, not matching
   authority or a compilable-source guarantee.
8. Write snapshots and exports under the generated-artifact root. Export JSON
   deterministically, sort by address/name, and ensure a clean input plus
   tracked replay/types can reproduce the reviewed state.

## Repository adapter

For this repository prefer the adapter for repeatable operations so
engine/version differences remain in one place. Drop to the detected native
engine for focused interactive work or commands the adapter does not expose,
then capture reviewed results in replay rather than relying on session state:

Use the stateless Python adapters in
`tools/python/harness/analyzer.py` and `snapshot.py` for repeatable queries and
normalized output. Generated snapshots belong under
`out/reverse/<target>/snapshot.json`; reviewed replay commands and analysis-only
types remain under `config/analysis/`. Do not rely on persistent analyzer
projects or hand-edit generated snapshots.

## Analyzer naming policy

- Keep separate target-qualified sessions and bindings for separate executables or
  overlays, even when bytes or addresses repeat.
- Keep compiled symbols address-based (`func_XXXXXXXX`, `DAT_XXXXXXXX`) until a
  meaning is reviewed. Add semantic aliases without losing address traceability.
- Preserve unknown struct fields and exact offsets. Do not force a decompiler
  guess into a public type.

## Acceptance invariant

Analyzer state is reproducible only when the exact input bytes, load/architecture
settings, tracked reviewed replay, and tracked type inputs recreate the reviewed
names, comments, functions, and deterministic exports. A native project database
alone is not durable evidence.
