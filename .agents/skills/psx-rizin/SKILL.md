---
name: psx-rizin
description: Manually invoked, evidence-driven PlayStation 1 reverse-engineering with Rizin, runtime traces, symbols, overlays, and matching decompilation. Use only when the user explicitly invokes `$psx-rizin` or asks to load this skill.
license: MIT
compatibility: Linux, macOS, or WSL; Python 3.10+; Rizin recommended; optional rz-ghidra and emulator/decompilation tools.
metadata:
  author: OpenAI
  version: "1.0.0"
  invocation: "$psx-rizin"
  platform: "Sony PlayStation / PS1 / PSX"
---

# PSX Rizin

Use only after explicit invocation. Build reproducible findings from authorized
machine code and runtime evidence; treat analyzer, decompiler, and signature
output as hypotheses until corroborated.

## Rules

1. Never redistribute proprietary game, BIOS, or SDK payloads.
2. Record input hashes, revisions, tool versions, and extraction provenance.
3. Qualify every address by file offset, runtime address, and overlay/module.
4. Validate PS-X EXE load addresses and cached/uncached RAM aliases.
5. Inspect MIPS branch, call, and load delay slots before inferring behavior.
6. Analyze mixed code/data in bounded stages; do not start with unrestricted
   global analysis.
7. Validate functions, arguments, types, and names across callers,
   instructions, and runtime evidence where practical.
8. Keep independently loaded overlays in separate namespaces.
9. Record symbol/signature provenance and confidence; preserve source names.
10. In this repository, use the repo's workspace and commands: Rizin snapshots
     land under `out/reverse/<target>/` (via `bin/rz-project`), the cross-target
     query cache under `out/index/`, and matching evidence under
     `out/matching/`, `out/permuter/`, and `out/asm-diff/`. The generic scripts
     below are skill-local helpers; prefer the wired `bin/` entrypoints when they
     cover the task.

## Route the task

Interpret explicit calls as the nearest mode:

```text
$psx-rizin inventory <disc-or-directory>
$psx-rizin inspect-exe <PS-X-EXE>
$psx-rizin analyze <binary> [base-address]
$psx-rizin analyze-overlays <directory>
$psx-rizin function <binary> <runtime-address> [base-address]
$psx-rizin symbols <symbol-source>
$psx-rizin trace <replay-or-scenario>
$psx-rizin replay-coverage <replay-directory>
$psx-rizin build-diff [function-or-target]
$psx-rizin audit <case-directory>
```

A free-form request is valid. State assumptions instead of blocking on minor
syntax ambiguity.

## Read progressively

- Full procedure: [WORKFLOW.md](references/WORKFLOW.md)
- Addressing, ABI, delay slots: [PSX_ABI_AND_ADDRESSING.md](references/PSX_ABI_AND_ADDRESSING.md)
- Rizin commands and staged analysis: [RIZIN_PLAYBOOK.md](references/RIZIN_PLAYBOOK.md)
- Symbols, signatures, types: [SYMBOLS_SIGNATURES_AND_TYPES.md](references/SYMBOLS_SIGNATURES_AND_TYPES.md)
- Overlays and assets: [OVERLAYS_AND_ASSETS.md](references/OVERLAYS_AND_ASSETS.md)
- Runtime traces and replay coverage: [RUNTIME_AND_REPLAYS.md](references/RUNTIME_AND_REPLAYS.md)
- Matching decompilation: [DECOMP_BUILD_DIFF.md](references/DECOMP_BUILD_DIFF.md)
- Command lookup: [COMMAND_REFERENCE.md](references/COMMAND_REFERENCE.md)
- Source catalog: [MANUALS_AND_SOURCES.md](references/MANUALS_AND_SOURCES.md)

Read `WORKFLOW.md` for a broad case. For a focused task, read only the matching
references, then inspect bundled script help before execution.

## Deliver

Report only evidence needed for the request, including:

- input identity and proven address model;
- target/overlay-qualified findings with confidence;
- relevant static and runtime evidence;
- matching status when requested;
- contradictions, unknowns, and the next useful experiment.

Mark unsupported conclusions `[INFERRED]` and give the evidence chain.

Before declaring a broad case complete, verify inventory, address mapping,
overlay identity, function boundaries, indirect control flow, symbol
provenance, runtime coverage, decompiler reconciliation, and exclusion of
proprietary inputs from distributable artifacts.

## Utilities

Repository-wired entrypoints (prefer these):

```text
bin/rz-project      target-qualified Rizin analyze/status/open (writes out/reverse/<target>/)
bin/rev-query       query the generated cross-target index (out/index/)
bin/asm-diff        instruction-level diff of one authored lift
bin/byte-match      raw byte-equality acceptance check
bin/permute         bounded source-shape search (out/permuter/)
bin/decomp-status   exact/partial/invalid lift audit
bin/symbols         check/normalize target-local maps
bin/splat           regenerate reviewed segment output
```

Generic skill-local helpers (run from `.agents/skills/psx-rizin/`):

```text
scripts/psx_exe.py              PS-X EXE inspection and address conversion
scripts/scan_mips.py            raw MIPS triage
scripts/rizin_export.py         Rizin JSON export
scripts/function_artifacts.py   per-function evidence bundle
scripts/symbols_to_rizin.py     reviewed-symbol conversion
scripts/replay_coverage.py      replay-matrix validation
```

> Note: this skill's earlier `bin/psx-rizin`, `bin/lift`, and `bin/build-diff`
> dispatchers are not wired in this repository. Use the repo `bin/` entrypoints
> above; the `scripts/*.py` helpers remain available for generic Rizin work.
