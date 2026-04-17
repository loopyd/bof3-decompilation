---
description: One-file BOF3 PSX decomp and matching worker loop
agent: build
subtask: true
---

## Purpose

Run the repo-local one-file BOF3 matching workflow for a single assigned
function.

This command is the worker playbook delegated by
`@.opencode/commands/decomp.md`.

For policy, naming, literal spelling, per-function preambles, stub-lane rules,
and permuter gates, follow `@.opencode/commands/decomp.md`.

For durable matching lessons and recurring source-shape wins, also read
`@.opencode/commands/decomp-lessons.md`.

Use `@.opencode/commands/decomp-mismatch-routing.md` when you need to decide
whether a last mismatch is still worth chasing in pure C, should be tested with
`maspsx` / ASPSX-version behavior, or is likely a toolchain ceiling.

## Contract

- You own exactly one repo source file at a time.
- Stay within that file unless the orchestrator explicitly expands scope.
- Do not revert or interfere with other workers' edits.
- Use `tmp/` for experiments.
- If a scratch copy needs local includes like `internal.h`, preserve the same
  relative layout or copy the needed headers with it.
- Fix the scratch layout, not the repo include line.
- Only promote a repo edit after verifying canonical match does not regress.
- If your final function shape no longer needs a local helper block or an
  `internal.h` declaration, remove the dead declaration instead of leaving the
  file noisier than necessary.
- Keep the function as close to pure C as practical.
- Keep any temporary exact-match shim easy to refactor away later. Prefer local
  typedefs, prototypes, helpers, or macros over shared churn.
- Prefer small local context blocks above the function over making the function
  body carry repeated raw addresses or bulky scratch scaffolding.

## Preconditions

- Prefer canonical imported overlay selectors such as
  `/bins/BIN/WORLD00/AREA016/6.bin` once a runtime entry has been imported.
- This worker assumes the target already has, or is explicitly getting, a
  single repo-owned source file under `bof3/src/...` or `bof3/stubs/...`.
- If the workspace reports `blocked_missing_source_mapping`, route back to the
  orchestrator for `promote-wave`, `seed-wave`, `repair-wave`, or stub/lift
  creation instead of forcing one-file matching work.

## Goal Order

1. if the function is only being stubbed, keep it in `bof3/stubs/...`
2. keep the function semantically correct
3. compile under canonical `gcc-2.7.2-psx + maspsx`
4. improve canonical `objdiff_match_percent`
5. reach exact match while staying as close to clean pure C as practical

Priority rule:

- every helper, declaration, typedef, struct, macro, PsyQ spelling, and cleanup
  is only acceptable if it preserves or improves the same verified canonical
  object/asm for this function
- if two shapes both match, keep the cleaner one
- if the cleaner one stops matching, keep the uglier proven one until you can
  recover the same object/asm cleanly

## Read Order

Before changing anything, inspect:

1. repo-owned source file
2. original `func.s`
3. current targeted canonical diff output
4. `func.m2c.c` if present
5. `func.ghidra.c` for inspection context
6. headless Ghidra `function export/callers/refs` output when the real entry,
   caller set, callee set, or fixed data addresses still matter

In Ghidra, also inspect:

- callers and callees
- data references and cross-references
- stack variables and parameter uses
- nearby globals, tables, and repeated offsets
- rerun the repo headless Ghidra export when the current bundle is stale,
  missing, or visibly weaker than the asm-backed understanding you need

Important:

- `func.json` is only the bundled per-function summary. It does not keep the
  full caller/xref evidence surface.
- Use the static evidence flow in `@.opencode/commands/decomp.md` for exact
  headless commands, PsyQ certainty, and naming tiers.
- Treat headless JSON exports as normal one-file evidence. They are the
  preferred way to prove ambiguous entries, data slots, dispatch tables, and
  repeated fixed-address accesses before calling something a ceiling.
- Use xref evidence to justify cleaner helpers, not to excuse uglier code by
  default.
- Do not keep any helper just because it looks nicer; keep it because it still
  produces the same object/asm.

`m2c` rule:

- treat `func.m2c.c` as the default first-pass lift aid, not canonical truth
- if `func.m2c.c` looks structurally useful, prefer starting from it before
  borrowing from Ghidra
- if `func.m2c.c` is missing, stale, or failed, try one manual rerun:
  - `python3 third_party/tools/m2c/m2c.py -t mipsel-gcc-c func.s`
- if that still fails, continue from asm + Ghidra inspection without blocking
- do not treat `func.ghidra.c` as the default paste-in source; use it to verify
  specific stack, xref, table, global, or control-flow details

Use the assembly to decide what is actually wrong.

Common classes:

- address materialization
- signedness
- load/store width
- delay-slot placement
- call ordering
- `$at` expansion shape
- pseudo-op encoding

## Core Loop

1. Resolve identity with `python3 -m scripts.rebof3 match target --program ...
   --entry ...` when you need the canonical workspace, source file, and bundle
   identity without triggering build work.
2. Create the workspace once with `python3 -m scripts.rebof3 match init ...` or
   `make match_init PROGRAM=... ENTRY=...`, then keep reusing its
   `workspace.json` for the rest of the worker loop.
   Prefer direct `match ... --workspace-json <path>` calls when you already
   have the workspace path.
3. Baseline the current function with a targeted canonical report.
4. If readiness is unclear, inspect the durable diff/readiness report first:
   - `python3 -m scripts.rebof3 match diff --workspace-json <path>`
   - or `make match_diff PROGRAM=... ENTRY=...`
   If the workspace is blocked on missing bundle, build status, or source
   mapping, fix that blocker first instead of treating it as a codegen mismatch.
   Refreshing the bundle with the repo headless Ghidra path is normal here.
5. Remember that `make match_build` defaults to the one-file `compile-one`
   build; use the full-build fallback only when `compile-one` is unavailable or
   explicitly required.
6. If stdout metrics alone do not make the next move obvious, run the fast
   side-by-side instruction viewer:
   - `make match_build PROGRAM=... ENTRY=...`
   - `make match_view PROGRAM=... ENTRY=...`
   Treat this as the local `asm-differ` loop.
7. Identify the smallest real mismatch class from the side-by-side rows, not
   just from the percent.
8. Try the narrowest pure-C source-shape change that could affect it.
9. When you figure out an offset, table, fixed base, or tiny layout, declare it
   immediately above the function as a local preamble.
10. Re-run the targeted canonical report.
11. Reopen `make match_view` when the score moves but the remaining structural gap is
   still unclear.
12. Remember that later source can still perturb earlier assembly; do not judge a
    rewrite only by the first changed row.
13. After two or three non-improving manual attempts, if the function still
     looks structurally reachable, run the standard bounded permuter pass:
     - `make match_permuter PROGRAM=... ENTRY=... MATCH_PERMUTER_ARGS='--timeout-seconds 60 -- --better-only --best-only --stop-on-zero'`
     - if the repo lift is a poor seed but `func.m2c.c` looks structurally useful,
       also consider:
       - `make match_permuter PROGRAM=... ENTRY=... MATCH_PERMUTER_ARGS='--variant m2c --timeout-seconds 60 -- --better-only --best-only --stop-on-zero'`
14. If the 60-second pass improves the result, fold the best candidate back into
      the manual loop, clean it up into readable repo-owned C, and keep working.
15. Promote only if the result is better or meaningfully cleaner at the same
      score.
16. If the standard pass does not help and the mismatch still looks reachable,
      either do more manual source-shape work or run another 60-second pass after
      a new local maximum is reached.
17. Stop only when you reach exact match, a plausible ceiling, or a scope
      boundary that must be reported.

Use `match diff --run-backend` before declaring a helper/declaration cleanup
finished. Treat it as the durable object-aware confirmation path after the fast
`make match_view` loop.

For rapid prototype checks, prefer:

- `python3 -m scripts.rebof3 match compiler-report --compiler gcc-2.7.2-psx --source-file <path> --run-name <name> --quick`
- `python3 -m scripts.rebof3 match diff --workspace-json <path> --run-backend`
  when you want durable asm-differ / objdiff artifacts instead of only stdout
  report rows

If you switch `match compiler-report` to stdout manually, the human-readable
default is `--stdout-view summary --stdout-format brief`.

Use file-backed report runs when you need backend artifacts or a durable record.

## One-File Tactics

Prioritize these kinds of changes first:

1. signed vs unsigned local/temp normalization
2. local temp introduction/removal
3. declaration ordering when the function is already structurally close
4. xref-proven address base spelling
5. local address-stable defines / macros / externs / tiny tables for proven
   globals or dispatch slots
6. helper removal when a helper forces bad address folding
7. condition/early-return reshaping
8. loop shape changes
9. argument-width or cast placement changes
10. decomp.me-style conservative cleanup that preserves the current codegen

When the function wants cleaner context, prefer:

- local `#define` or macro blocks for one-off offsets
- local address-stable `#define` / macro / `extern` spellings for proven globals,
  tables, or dispatch entries
- local `extern` declarations for proven fixed objects
- tiny local tables or typedefs
- small local structs or layout helpers that approximate the proven shape
  closely enough to keep the lift readable and easy to refactor later
- existing repo/common headers or PsyQ-facing declarations when they already
  provide the correct contract and keep the function simpler
- real PsyQ function/type/header spellings when certainty is high
- decomp.me-style conservative naming/patterns when semantics are not proven
- if the exact call target is proven but the final declaration is not ready, a
  tiny local prototype, typedef, dummy declaration, or address-stable
  function-pointer shim is acceptable temporary scaffolding
- only keep these helpers when they preserve or improve canonical codegen
- it is acceptable to keep a slightly lower-level local helper when headless
  xrefs or asm prove it and that helper is what gets the function to `1:1`
- temporary scaffolding should have an obvious delete/refactor path once the
  real PsyQ or repo-owned declaration is ready
- prefer simple readable locals, helpers, macros, and structs over repeating raw
  magic expressions throughout the body
- if a cleaner helper and a rougher helper do not produce the same object/asm,
  prefer the one that matches and leave the cleanup for later
- brief comments for likely meanings when confidence is high
- if two shapes match similarly, keep the cleaner C rather than the more
  decompiler-literal spelling
- when repeated helpers become stable and shared, sync them into `internal.h`
  instead of duplicating divergent local spellings
- when a cleanup or rewrite makes a helper unnecessary, delete it and keep the
  file / `internal.h` pair tidy

Scratch discipline:

- treat repo code as the cleaned form of a one-function decomp.me scratch
- keep temporary typedefs, structs, macros, and declarations in a small local
  preamble first
- only promote repeated stable context into `internal.h`
- keep permuter-only macros, compile tricks, and scratch-only helper spellings
  out of committed source

Do not start with:

- broad header rewrites
- semantic renaming churn
- asm promotion
- non-portable `__asm__` tricks
- address-bound shims when a small pure-C helper can still express the same
  thing
- committing scratch-only permuter scaffolding when a cleaned local helper would
  do
- deleting rough placeholders; keep them in the stub lane and report them

## Stop Conditions

Stop and report back when:

- the workspace is still blocked on import / promotion / source-mapping work
  that exceeds the one-file lane
- the function reaches exact match
- the function reaches a plausible pure-C ceiling
- the remaining mismatch is clearly a pseudo-op or relocation-name artifact
- headless callers/refs have already been checked anywhere boundary or fixed
  address ambiguity could still explain the gap
- further work would require broader scope than one file
- a nicer pure-C or PsyQ-shaped rewrite keeps losing the same-object/asm
  requirement

## After Exact Match

Once a function reaches exact match:

1. allow one narrow cleanup pass only
2. remove dead local scaffolding first
3. replace temporary local declarations with proven PsyQ or repo-owned
   declarations only if the same verified canonical object/asm survives
4. rerun the durable backend diff before keeping the cleanup

## Required Output

When you finish the file, report:

- whether the repo file changed
- old vs new canonical match percent
- mismatch count change
- the exact source-shape change that helped, if any
- whether a manual `m2c` rerun was attempted and whether it helped
- whether `match diff` exposed any non-mismatch blocker such as missing bundle,
  build status, or source mapping
- what blocked further progress, if not exact
- file paths changed
