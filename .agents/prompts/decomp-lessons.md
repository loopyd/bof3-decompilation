---
description: Durable BOF3 decomp matching lessons from prior waves
agent: build
subtask: false
---

## Purpose

Capture durable matching lessons that repeatedly improved BOF3 PSX decomp output
without drifting into non-portable or hard-to-maintain source.

Use this as a companion to:

- `@.opencode/commands/decomp.md`
- `@.opencode/commands/decomp-worker.md`
- `@.opencode/commands/decomp-mismatch-routing.md`

Asm is still the source of truth. These are recurring source-shape wins, not
new canon.

Every lesson here is constrained by one rule: keep the lesson only when it
helps recover the same verified canonical object/asm, not merely cleaner C.

## Fast Loop

- Prefer targeted quick checks during manual iteration:
  - `python3 -m scripts.rebof3 match compiler-report --compiler gcc-2.7.2-psx --source-file <path> --run-name <name> --quick`
- `python3 -m scripts.rebof3 match target --program ... --entry ...` is the
  lightest resolver when you only need the canonical workspace/source/bundle
  identity before choosing the next command.
- If you enable stdout manually instead of using `--quick`, the human-readable
  default is `--stdout-view summary --stdout-format brief`.
- `python3 -m scripts.rebof3 match init --program ... --entry ...` or
  `make match_init PROGRAM=... ENTRY=...` should happen once per target; reuse
  the resulting `workspace.json` with `--workspace-json` for build, diff, view,
  and permuter commands.
- For imported runtime programs, prefer canonical selectors such as
  `/bins/BIN/WORLD00/AREA016/6.bin` instead of ad hoc archive nicknames.
- If the next move is unclear from the quick row alone, use:
  - `make match_build PROGRAM=... ENTRY=...`
  - `make match_view PROGRAM=... ENTRY=...`
- If you need durable readiness or backend artifacts, use:
  - `python3 -m scripts.rebof3 match diff --workspace-json <path> --run-backend`
- If the bundle is stale, missing, or structurally weak, rerunning the repo
  headless Ghidra export is a normal evidence-refresh step.
- `make match_build` now defaults to the one-file `compile-one` path.
- The raw artifact build graph may still be archive-backed for many module
  families; treat the current artifact manifest as build ownership/status, not as
  proof that final replacement-ready raw linking is done for that family yet.
- `python3 -m scripts.rebof3 match refresh` is the easiest way to regenerate
  scoreboard, backlog, and status surfaces together before choosing the next
  candidate queue.
- Treat `objdiff_match_percent` as triage. Use side-by-side rows to classify the
  real mismatch.
- After two or three non-improving manual attempts, the standard 60-second
  permuter pass is an important normal part of the loop, not an exotic last
  resort:
  - `make match_permuter PROGRAM=... ENTRY=... MATCH_PERMUTER_ARGS='--timeout-seconds 60 -- --better-only --best-only --stop-on-zero'`
- Always verify a permuter candidate directly before keeping it.

## Lane Lessons

- Do not start one-file matching work for a target that still lacks a canonical
  program row or source mapping.
- Missing canonical runtime programs belong in the import lane first:
  - use `match import-backlog` / `match import-wave`
- Imported zero-function programs belong in the frontier lane next:
  - use `match promote-wave` for `promotable_entry_labels`
  - use `match seed-wave` for `manual_frontier`
- Missing or stale Ghidra program rows for canonical paths belong in the repair
  lane:
  - use `match repair-wave`
- Once source exists, one-file work belongs in the worker lane with a durable
  `workspace.json`.

## Report Surface Lessons

- `match status` writes the decomp snapshot under `tmp/status/<profile>/current/`
  by default and under `reports/decomp-status/current/` with `--tracked-output`.
- `match refresh` is the normal entrypoint for regenerating:
  - scoreboard reports
  - import backlog reports
  - frontier backlog reports
  - status outputs
- For checkpoint planning, pull next-wave candidates from the backlog/status
  surfaces before drilling into one-file compiler reports.

## Cleanup Lessons

- Treat local helper blocks and `internal.h` as part of the same maintained
  surface.
- Once a function settles, remove unused defines, typedefs, externs, and stale
  `internal.h` declarations instead of letting temporary matching scaffolding
  accumulate.
- Prefer real PsyQ declarations once they are certain enough to land cleanly.
- If a temporary dummy declaration or address-bound shim was needed for match
  work, keep it local and shaped so replacing it later is mechanical.
- Prefer the smallest declaration scope that still keeps nearby lifted code
  readable and codegen-stable.
- Prefer pure-C-first helper shapes that are easy to rename, tighten, or
  promote later over one-off exact-match hacks.
- If a cleaner helper stops producing the same object/asm, it is not the right
  helper yet.
- Keep temporary layout helpers readable enough that a later cleanup can rename,
  tighten, or promote them without first undoing decompiler-shaped clutter.
- When a stable contract is already expressed in shared BOF3 headers or existing
  PsyQ-facing declarations, reuse it instead of introducing a second local
  spelling that will drift later.
- After exact match, allow only narrow cleanup that keeps the same verified
  canonical object/asm.

## Decompiler Inputs

- Use `func.s`, `func.m2c.c`, and Ghidra inspection together when available.
- `func.s` gives boundaries and behavior.
- `func.m2c.c` is usually the best first-pass lift seed and should get the
  first read before the Ghidra decompile.
- Use `func.ghidra.c` mainly to inspect stack/param usage, xrefs, globals,
  tables, and alternate control-flow interpretations.
- The bundled `func.json` is only a summary; it does not retain callers or the
  full xref surface.
- When Ghidra and `m2c` disagree, keep `m2c` as the ownership shape unless
  Ghidra proves a specific fact that the asm also supports.
- Do not let a cleaner-looking Ghidra decompile override a better asm-backed
  `m2c` shape wholesale.
- If `m2c` fails, rerun it once manually; if it still fails, continue from asm
  plus Ghidra inspection without blocking.

## Headless Evidence Lessons

- Use the canonical static evidence flow from `@.opencode/commands/decomp.md`
  instead of inventing one-off local evidence rules.
- Use headless callers for boundary proof and direct callsite proof.
- Use headless refs for fixed data addresses, jump tables, dispatch slots, and
  repeated load/store sites.
- Use metadata-only export when you need the authoritative function row without
  re-reading the full decompile.
- Once headless evidence proves a stable object, prefer the smallest clean local
  helper that preserves the same verified canonical object/asm.

## Address Lessons

- Prefer clean compile-time constructs first:
  - local `#define` blocks
  - local `extern` declarations
  - typed overlay structs
  - local tables
- Keep them only if they are codegen-neutral or improve the canonical object.
- When a clean construct loses the target shape, fall back only as far as
  needed.
- Prefer the smallest readable approximation of the proven shape over a larger
  speculative overlay or a wall of raw offset expressions.
- Prefer local preamble/context blocks over spraying raw magic expressions
  throughout the function body.

Recurring wins:

- `0x80140000` high-base pointer plus offset often beats repeated raw absolute
  casts for state bytes and halfwords.
- `0x801f0000` high-base pointer plus explicit negative/relative offsets often
  matches event-slot and table code better than macro-heavy indexing.
- For stride-based tables, an explicit `offset = index * stride` local often
  helps GCC form the desired address shape.
- Raw absolute `u16`/`u32` loads at fixed offsets sometimes beat typed table
  macros when the original uses byte-derived offsets.
- Headless refs often show that several “different” raw accesses are really one
  table or one global slot, which justifies a small local helper instead of
  repeating guessed casts.
- Headless callers often prove that a nearby label is only an internal target,
  not the real function start; use that before hardening a bad boundary into
  repo code.
- decomp.me-style conservative names and small layout helpers keep the lift easy
  to read now and easy to refactor into better generic C later.
- When several shapes are viable, prefer the cleanest one among the exact-match
  shapes, not the cleanest one overall.

## Control-Flow Lessons

- Early returns often improve branch layout and delay-slot filling.
- Small `goto`-labeled blocks can match original compare chains better than a
  deeply nested or heavily combined boolean expression.
- Reordering a local init or delaying a pointer formation can materially change
  saved-register allocation and frame shape.
- Combined boolean tests often hide the original branch order; split them when
  the asm shows a clearer sequence.

## Packet And Primitive Lessons

- Cache the primitive/packet base once when the target loads it once.
- Remove per-field `volatile` when the target wants the last store in a call
  delay slot and the memory is not semantically volatile.
- For packet/table setup, explicit locals for color bytes, coords, or packet ids
  often improve register reuse and call argument shape.

## Table And Dispatcher Lessons

- Local function-pointer tables or local table copies can recover desired
  dispatch shapes when direct macro indexing produces bad address materialization.
- For tiny state handlers, explicit mode blocks (`mode_1`, `mode_2`, `done`) can
  be better than a compact switch-like C expression.
- Re-reading a fixed table or scratch slot inside the taken branch can sometimes
  match better than hoisting a shared temp.

## Signedness And Width Lessons

- Exact temp width matters. Replacing a `u8` with `u32`, or `u32` with `u16` or
  `s16`, can remove or introduce extra `andi`, sign-extension, or divide shape.
- Signed divide/modulo paths are especially sensitive. Use signed temps when the
  target clearly wants signed arithmetic.
- Explicit load-then-store for counters and flags often matches better than
  compound assignment on volatile or fixed-address objects.

## What Usually Plateaued

- GNU `as` pseudo-op encoding differences such as `move` vs `or`/`addu`.
- Relocation-name-only `jal` diffs.
- Large-numeric `$at` expansion operand-order differences that stayed stable
  across pure-C rewrites.
- Pre-`maspsx` GCC register-allocation issues after several narrow manual tries.
- Scratch-only permuter scaffolding that looked useful in isolation but did not
  survive cleanup into committed repo-owned C.

These are often reasonable one-file ceilings.

## Stub Lessons

- Do not delete newly lifted duplicate or placeholder files just because they are
  rough.
- Keep them compiling.
- Exclude trivial empty placeholders from orchestration baselines with:
  - `--skip-empty-stubs`
- Do not let placeholder rows drive the next candidate queue.

## Promotion Rules

- Start address knowledge local to the function.
- Promote to module `internal.h` after repeated use inside that module.
- Promote to a shared header only when the construct is stable and reused across
  modules.

## Anti-Lessons

- Do not bind absolute symbols with inline asm just to gain a little codegen.
- Do not keep ugly magic-address pointer forms when a local macro/table/extern
  produces the same object shape.
- Do not treat readable cleanup and refactorability as optional polish; if two
  shapes match similarly, keep the one another decomp pass can understand and
  improve.
- Do not let cleanups or semantic naming churn widen scope during a matching
  wave.
