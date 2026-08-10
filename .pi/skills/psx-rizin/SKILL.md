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

Explicit invocation only. Findings reproducible from authorized machine code + runtime evidence. Analyzer/decompiler/signature output = hypothesis until corroborated.

## Rules

1. Never redistribute proprietary game/BIOS/SDK payloads.
2. Record input hashes, revisions, tool versions, extraction provenance.
3. Qualify every address: file offset + runtime address + overlay/module.
4. Validate PS-X EXE load addresses and cached/uncached RAM aliases.
5. Inspect MIPS branch/call/load delay slots before inferring behavior.
6. Analyze mixed code/data in bounded stages; never unrestricted global analysis.
7. Validate functions/args/types/names across callers, instructions, runtime evidence where practical.
8. Keep independently loaded overlays in separate namespaces.
9. Record symbol/signature provenance + confidence; preserve source names.
10. Repo workspace: snapshots `out/reverse/snapshots/<encoded-target>.json` (`bin/rz-project`), index `out/index/` (`bin/rev-query`), matching `out/matching/`, `out/permuter/`, `out/asm-diff/`. Prefer wired `bin/` entrypoints when they cover the task.

## Snapshot readiness

Read-only summary before analysis/index work (one target or all):

```sh
python3 .pi/skills/psx-rizin/scripts/snapshot-status.py [TARGET]
```

Emits manifest identity, binary hash, snapshot freshness, index readiness as one JSON; never runs analysis or changes files. Stale target: `bin/rz-project analyze TARGET`, then `bin/index`.

## Route the task

| Call | Action |
|---|---|
| `$psx-rizin inventory <disc-or-directory>` | disc inventory |
| `$psx-rizin inspect-exe <PS-X-EXE>` | EXE parse |
| `$psx-rizin analyze <binary> [base-address]` | static analysis |
| `$psx-rizin analyze-overlays <directory>` | overlay analysis |
| `$psx-rizin function <binary> <runtime-address> [base-address]` | function + callers |
| `$psx-rizin symbols <symbol-source>` | symbol import |
| `$psx-rizin trace <replay-or-scenario>` | runtime trace |
| `$psx-rizin replay-coverage <replay-directory>` | replay coverage |
| `$psx-rizin build-diff [function-or-target]` | matching diff |
| `$psx-rizin audit <case-directory>` | case audit |

Free-form requests valid; state assumptions instead of blocking on minor syntax ambiguity.

## Read progressively

| Topic | File |
|---|---|
| Full procedure | [WORKFLOW.md](references/WORKFLOW.md) |
| Addressing/ABI/delay slots | [PSX_ABI_AND_ADDRESSING.md](references/PSX_ABI_AND_ADDRESSING.md) |
| Rizin commands/staged analysis | [RIZIN_PLAYBOOK.md](references/RIZIN_PLAYBOOK.md) |
| Symbols/signatures/types | [SYMBOLS_SIGNATURES_AND_TYPES.md](references/SYMBOLS_SIGNATURES_AND_TYPES.md) |
| Overlays/assets | [OVERLAYS_AND_ASSETS.md](references/OVERLAYS_AND_ASSETS.md) |
| Runtime/replay coverage | [RUNTIME_AND_REPLAYS.md](references/RUNTIME_AND_REPLAYS.md) |
| Matching decompilation | [DECOMP_BUILD_DIFF.md](references/DECOMP_BUILD_DIFF.md) |
| Command lookup | [COMMAND_REFERENCE.md](references/COMMAND_REFERENCE.md) |
| Source catalog | [MANUALS_AND_SOURCES.md](references/MANUALS_AND_SOURCES.md) |

Broad case: read WORKFLOW.md. Focused task: matching reference only, then bundled script help.

## Deliver

Report only needed evidence: input identity + proven address model; target/overlay-qualified findings + confidence; relevant static + runtime evidence; matching status when requested; contradictions, unknowns, next useful experiment. Mark unsupported conclusions `[INFERRED]` with the evidence chain.

Broad-case completion requires verifying: inventory, address mapping, overlay identity, function boundaries, indirect control flow, symbol provenance, runtime coverage, decompiler reconciliation, exclusion of proprietary inputs from distributable artifacts.

## Utilities

Repo-wired (prefer): `bin/rz-project` — target-qualified analyze/status/open (writes `out/reverse/snapshots/<encoded-target>.json`); `bin/rev-query` — cross-target index (`out/index/`). Lift-side (`asm-diff`/`byte-match`/`permute`/`decomp-status`/`symbols`/`splat`) follow the bof3-re evidence table.

> Note: earlier `bin/psx-rizin`, `bin/lift`, `bin/build-diff`, and generic legacy `scripts/*.py` helpers are NOT wired here. `scripts/snapshot-status.py` is the supported read-only readiness check above; use repo `bin/` entrypoints for analysis, symbol import, replay coverage.
