---
name: psx-rizin
description: Evidence-driven PlayStation 1 reverse-engineering with Rizin, runtime traces, symbols, overlays, and matching decompilation. Use for a concrete analyzer question, including autonomous BOF3 missions.
license: MIT
metadata:
  compatibility: Linux, macOS, or WSL; Python 3.10+; Rizin for static analysis; optional emulator and matching tools.
  author: OpenAI
  version: "1.0.0"
  invocation: "$psx-rizin"
  platform: "Sony PlayStation / PS1 / PSX"
---

# PSX Rizin

Use Rizin for static analysis. Findings must be reproducible from authorized machine
code + runtime evidence.
Analyzer/decompiler/signature output = hypothesis until corroborated.

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
10. Repo workspace: snapshots `out/reverse/snapshots/<encoded-target>.json` (`bin/harness analysis rz-project`), index `out/index/` (`bin/harness analysis query`), matching
`out/matching/`, `out/permuter/`, `out/asm-diff/`. Prefer wired `bin/` entrypoints that cover the task.

## Snapshot readiness

Read-only summary before analysis/index work (one target or all):

```sh
python3 .pi/skills/psx-rizin/scripts/snapshot-status.py [TARGET]
```

Emits manifest identity, binary hash, snapshot freshness, index readiness as one JSON; never runs analysis or changes files. Stale: `bin/harness analysis rz-project analyze TARGET`, then `bin/harness analysis index`.

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

Free-form requests valid; state assumptions; don't block on minor syntax ambiguity.

## Read progressively

| Topic | File |
|---|---|
| Full procedure | [workflow.md](references/workflow.md) |
| Addressing/ABI/delay slots | [psx-abi-and-addressing.md](references/psx-abi-and-addressing.md) |
| Rizin commands/staged analysis | [rizin-playbook.md](references/rizin-playbook.md) |
| Symbols/signatures/types | [symbols-signatures-and-types.md](references/symbols-signatures-and-types.md) |
| Overlays/assets | [overlays-and-assets.md](references/overlays-and-assets.md) |
| Runtime/replay coverage | [runtime-and-replays.md](references/runtime-and-replays.md) |
| Matching decompilation | [decomp-build-diff.md](references/decomp-build-diff.md) |
| Command lookup | [command-reference.md](references/command-reference.md) |
| Explicit external research only; provenance, not routine instructions | [manuals-and-sources.md](references/manuals-and-sources.md) |

Broad case: read workflow.md. Focused task: matching reference only, then bundled script help.

## Deliver

Report needed evidence only: input identity + proven address model; target/overlay-qualified findings + confidence; relevant static + runtime evidence; matching status when requested; contradictions/unknowns/next experiment. Mark unsupported conclusions `[INFERRED]` with the evidence chain.

Broad-case completion verifies: inventory, address mapping, overlay identity, function boundaries, indirect control flow, symbol provenance, runtime coverage, instruction reconciliation, exclusion of proprietary inputs from distributable artifacts.
## Utilities

Repo-wired: `bin/harness analysis rz-project` — target-qualified analyze/status/open (writes `out/reverse/snapshots/<encoded-target>.json`); `bin/harness analysis query` — cross-target index (`out/index/`). Lift-side (`asm-diff`/`byte-match`/`permute`/`lift status`/`source symbols`/`source splat`) follow the bof3-re evidence table.

> Note: legacy `bin/psx-rizin`, `bin/lift`, `bin/build-diff`, and generic `scripts/*.py` helpers are NOT wired here. `scripts/snapshot-status.py` is the supported read-only readiness check above; use `bin/` entrypoints for analysis, symbol import, replay coverage.

Return tool calls, wall time and method with mission result; parent archives
measurements and supplies relevant prior evidence. Converge reusable findings into
skill references. User documentation and observation ledgers are not runtime inputs.
