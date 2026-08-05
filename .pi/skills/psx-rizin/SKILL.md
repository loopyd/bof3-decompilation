---
name: psx-rizin
description: Evidence-driven PlayStation 1 reverse-engineering with Rizin, runtime traces, symbols, overlays, and matching decompilation. Use only when the user explicitly invokes `$psx-rizin` or asks to load this skill.
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
machine code and runtime evidence; analyzer, decompiler, and signature output
are hypotheses until corroborated.

## Rules

1. Never redistribute proprietary game, BIOS, or SDK payloads.
2. Record input hashes, revisions, tool versions, and extraction provenance.
3. Qualify every address by file offset, runtime address, and overlay/module.
4. Validate PS-X EXE load addresses and cached/uncached RAM aliases.
5. Inspect MIPS branch, call, and load delay slots before inferring behavior.
6. Analyze mixed code/data in bounded stages; never start with unrestricted
   global analysis.
7. Validate functions, arguments, types, and names across callers,
   instructions, and runtime evidence where practical.
8. Keep independently loaded overlays in separate namespaces.
9. Record symbol/signature provenance and confidence; preserve source names.
10. Here, use repo workspace/commands: Rizin snapshots under
     `out/reverse/<target>/` (via `bin/rz-project`), query cache under
     `out/index/`, matching evidence under `out/matching/`, `out/permuter/`,
     `out/asm-diff/`. The generic scripts below are skill-local helpers;
     prefer wired `bin/` entrypoints when they cover the task.

## Snapshot readiness

Run the bundled read-only summary before analysis or index work (one target
or all configured):

```sh
python3 .pi/skills/psx-rizin/scripts/snapshot-status.py [TARGET]
```

Emits manifest identity, binary hash, Rizin snapshot freshness, reverse index
readiness as one JSON document; never runs analysis or changes files. Per stale
target: run the reported `bin/rz-project analyze TARGET`, then rebuild the index
with `bin/index`.

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

Free-form requests are valid; state assumptions instead of blocking on minor
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

Repository-wired entrypoints (prefer these): `bin/rz-project` for
target-qualified Rizin analyze/status/open (writes `out/reverse/<target>/`)
and `bin/rev-query` for the generated cross-target index (`out/index/`).
Lift-side commands (`asm-diff`/`byte-match`/`permute`/`decomp-status`/
`symbols`/`splat`) follow the bof3-re Fast evidence table.

> Note: this skill's earlier `bin/psx-rizin`, `bin/lift`, `bin/build-diff`, and
> `scripts/*.py` helpers are not wired in this repository. Use the repo `bin/`
> entrypoints above for analysis, symbol import, and replay coverage.
