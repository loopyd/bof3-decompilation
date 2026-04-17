---
description: Orchestrate the BOF3 function-first decomp and matching workflow
agent: build
subtask: false
---

## Purpose

Run the BOF3 PSX decomp workflow as a matching-first orchestrator.

This command owns:

- lane / tool selection across import, promotion, seeding, repair, and
  one-function matching
- target selection and batching
- worker delegation
- canonical verification and score tracking
- duplicate-aware scheduling
- checkpoint and report refreshes

Delegate one-file implementation work to
`@.opencode/commands/decomp-worker.md`.

Delegate checkpoint/report refresh work to
`@.opencode/commands/decomp-checkpoint.md`.

Use
`@.opencode/commands/decomp-mismatch-routing.md`
to classify whether a remaining mismatch is a pure-C target, a `maspsx`
version-sweep candidate, or a likely dead end.

Use `@.opencode/commands/decomp-lessons.md` for durable matching lessons learned
from prior waves.

Shared-project default:

- prefer the repo-shared Ghidra project defaults under `tmp/bof3_ghidra/main`
  with project name `bof3_main`
- only override project location when you intentionally need an isolated local
  copy or the shared project is busy for a concrete reason
- headless Ghidra export/repair is allowed when it refreshes stale bundles,
  improves the current asm-backed lift, or helps prove a boundary; do not avoid
  it by blanket policy

Function/tooling refactor defaults:

- treat imported runtime programs as canonical `/bins/.../<entry>.bin` targets,
  not ad hoc archive nicknames
- treat `match target` as the lightest identity resolver and `match init` as the
  durable workspace creator
- treat `workspace.json` under `tmp/matching/...` as the handoff point between
  build, diff, view, permuter, and checkpoint/report tooling
- treat `match refresh` as the normal one-command refresh for scoreboard,
  import backlog, frontier backlog, and status outputs

## Mission

- Recover functions under the canonical US v1.1 PSX profile.
- Prefer the smallest useful function-first tasks.
- Follow the lane order: stubbing, lifting, then decomp refinement.
- Get functions compiling first.
- Improve `objdiff_match_percent` aggressively after compile is stable.
- Use pure C by default.
- Stay as close to pure C as practical while chasing exact assembly match.
- Prefer small helpers, defines, structs, macros, refs, and locals before more
  ad hoc exact-match tricks.

North star:

- produce the best matching decomp that still reads like plausible original
  source code
- every readability, PsyQ, helper, and cleanup choice is subordinate to
  producing the same verified canonical object/asm shape as the target
- exact `1:1` assembly is the preferred end state when it can be reached
  without abandoning a clean, pure-C-first ownership model
- prefer real PsyQ functions, headers, and types when certainty is high and
  they preserve the proven object shape
- prefer decomp.me-style recovered C patterns when meaning is still incomplete:
  conservative names, small local helpers, and obvious control flow over
  speculative semantic cleanup
- prefer source that is clean, simple, readable, and easy to improve across the
  wider codebase later
- prefer macros / defines / externs / tables / mappings that preserve codegen
  and make the function easier to understand
- prefer the smallest readable local approximation of the current proven shape
  so the lift stays easy to refactor later instead of hardening into decompiler
  sludge
- if an exact-match bridge still needs a temporary dummy declaration, local
  function-pointer shim, or address-bound helper, keep it narrow, local, and
  mechanically easy to replace later with the real declaration
- prefer helpers that are easy to refactor later into generic C with better
  names, better types, and cleaner shared declarations

Rule of precedence:

1. same canonical object/asm
2. semantic correctness
3. clean pure-C recovered shape
4. future refactorability

If a cleaner spelling, nicer helper, or better-known PsyQ declaration changes
the verified canonical object/asm for the worse, reject it or keep it local
until the same object/asm can be recovered again.

## Canonical Target

- Primary profile: `capcom97-bof3`
- Toolchain:
  - PsyQ 4.0
  - ASPSX 2.56 behavior via `maspsx`
  - `gcc-2.7.2-psx`
- Primary verification:
  - `python3 -m scripts.rebof3 match compiler-report --compiler gcc-2.7.2-psx ...`
- Interactive matching view:
  - `make match_view PROGRAM=... ENTRY=...`
- Final checkpoint verification:
  - full canonical report across `bof3/src`

Treat each shipped artifact as its own target, not one monolithic binary.

Current raw-artifact milestone:

- boot executable: `build/extracted/SLUS_004.22`
- logo executable: `build/extracted/LOGO/LOGO.EXE`
- raw `BIN` payload-backed EMI entries under `build/extracted/BIN/**/*.EMI#*`
- do not document repacking as part of the normal decomp loop yet
- current build output is mixed-stage: boot and the artifact registry use the
  final canonical raw paths, while many module families still build through
  archive-backed intermediate outputs until their final raw link model is ready

Compiler flags and their codegen implications are part of the target shape.

## Source Of Truth

Use this authority order:

1. original binaries and extracted assets under `disk/` and `build/extracted/`
2. exported function bundles:
   - `func.s`
   - `func.m2c.c`
   - `func.ghidra.c`
3. repo-owned recovered source under `bof3/src`
4. `objdiff` / `asm-differ` results
5. inventory and metadata tables as scheduling aids

Important:

- never trust a guessed function boundary without original asm evidence
- never trust a high-level decompiler lift over the shipped assembly
- treat `m2c` as the preferred first-pass lift aid, not canonical truth
- treat `func.ghidra.c` mainly as an inspection surface, not the default
  ownership model for new repo code
- no guideline in this document overrides the requirement to match the same
  shipped canonical object/asm

## Decompiler Inputs

Use asm, `m2c`, and Ghidra inspection together when available:

- `func.s` is the behavior and boundary truth
- `func.m2c.c` is usually the best first-pass structural seed for the lift
- `func.ghidra.c` is mainly for inspecting stack/param usage and alternate
  control-flow reads
- the bundled `func.json` only keeps function boundary/signature metadata; use
  headless Ghidra queries for callers, callees, and address/xref proof instead
- when `m2c` and Ghidra disagree, start from the `m2c` ownership shape and only
  borrow the specific facts Ghidra proves better
- rerunning the repo headless Ghidra export is a normal refresh step when the
  bundle is stale, missing, or weak as an inspection surface

## Static Evidence Flow

Headless address/xref proof:

- when a real entry point, caller chain, callee edge, table base, or fixed data
  address matters, query the shared Ghidra project directly instead of guessing
- use:
  - `third_party/tools/bof3-ghidra/ghidra.sh function export --program <selector> <addr> --metadata-only --output tmp/ghidra_symbols/<tag>_meta.json`
  - `third_party/tools/bof3-ghidra/ghidra.sh function callers --program <selector> <addr> --output tmp/ghidra_symbols/<tag>_callers.json`
  - `third_party/tools/bof3-ghidra/ghidra.sh function refs --program <selector> <addr> --output tmp/ghidra_symbols/<tag>_refs.json`
- prefer `callers` when boundary proof or direct callsites are still ambiguous
- prefer `refs` for variable addresses, global slots, jump tables, dispatch
  entries, and repeated fixed-address loads/stores
- once headless xrefs prove a stable object, local address-stable
  `#define`/macro/`extern`/table helpers are allowed if they move the function
  toward exact match without widening scope
- do not let xref proof push the code away from clean recovered C if a small
  pure-C helper can express the same fact
- keep the helper only if it preserves or improves the verified canonical
  object/asm; otherwise throw it away

PsyQ certainty ladder:

1. existing repo-proven symbol/debug/map/metadata evidence for this exact target
2. existing repo/shared declarations already used successfully in matching code
3. official PsyQ functions, headers, and types whose contract is certain and
   whose spelling preserves the current target object/asm
4. local conservative placeholders:
   - local prototypes
   - local typedefs
   - local structs
   - local dummy declarations
   - local address-stable helpers

If the certainty level drops, keep the declaration scope local and conservative.
Do not promote speculative PsyQ spellings into shared headers.

Naming tiers:

1. proven PsyQ / shipped / repo-established names
2. conservative decomp.me-style placeholders:
   - `func_80162d00`
   - `unk_1c`
   - `field_18`
   - `temp`
   - `i`
3. comments for likely meaning when confidence is high enough to help, but not
   yet high enough to justify identifier churn

Do not skip directly from unknown behavior to polished semantic names.

`m2c` rule:

- if `func.m2c.c` exists and looks stable, start from it during the loop
- if `func.m2c.c` is missing, stale, or failed, try one manual rerun before
  falling back to an asm + Ghidra-inspection lift:
  - `python3 third_party/tools/m2c/m2c.py -t mipsel-gcc-c func.s`
- if that manual rerun still fails, continue from asm + Ghidra inspection
  without blocking

When both `func.ghidra.c` and `func.m2c.c` are available, prefer `m2c` for the
lift and use Ghidra to confirm or refine specific details rather than replacing
the whole source shape wholesale.

Alternative scratch route:

- for some stubborn functions, `func.s` -> `m2c` -> permuter can be a viable
  way to discover a better structural candidate
- use that as a scratch lane, not as the final ownership model
- any useful candidate still needs to be folded back into readable repo-owned C
  and verified canonically against the real function

## Verifier Roles

- `make match_view` is the fast instruction-level viewer. Treat it like the
  local `asm-differ` loop for deciding the next source-shape move.
- `python3 -m scripts.rebof3 match diff --workspace-json <path> --run-backend`
  is the durable backend confirmation path. Treat it as the object-aware check
  before declaring a helper, cleanup, or declaration change acceptable.
- `objdiff_match_percent` is a triage signal, not the policy. Final acceptance
  is still the same verified canonical object/asm.

## Scratch And Cleanup Discipline

- Treat committed repo code like the cleaned version of a decomp.me scratch:
  one target function, small local context block, and conservative names.
- Keep scratch-only context in local preambles, `tmp/`, or workspace artifacts
  before promoting it into repo-owned declarations.
- Keep permuter-only tricks, scratch compile hacks, and throwaway macros out of
  committed source unless the cleaned final form still needs them.
- After exact match, one narrow cleanup pass is allowed:
  - simplify local helpers
  - replace temporary local declarations with proven PsyQ or repo-owned ones
  - remove dead scaffolding
  only if the verified canonical object/asm stays the same
- If cleanup loses the match, revert the cleanup and keep the proven shape.

## Lane Routing

Route work to the shallowest viable lane first.

1. `python3 -m scripts.rebof3 match refresh` when you need fresh scoreboard,
   backlog, and status outputs before choosing targets.
2. `python3 -m scripts.rebof3 match import-wave` for code-candidate EMI entries
   that still lack canonical program rows.
3. `python3 -m scripts.rebof3 match promote-wave` for imported zero-function
   programs whose frontier state is `promotable_entry_labels`.
4. `python3 -m scripts.rebof3 match seed-wave` for imported zero-function
   programs whose frontier state is `manual_frontier` and whose seed strategy
   looks credible.
5. `python3 -m scripts.rebof3 match repair-wave` when a canonical program row
   already exists but the shared Ghidra project is missing or stale for that
   program.
6. `@.opencode/commands/decomp-worker.md` only after a concrete repo source file
   or one-file lift target exists. Do not assign a one-file worker to a target
   that still lacks source mapping.
7. `@.opencode/commands/decomp-checkpoint.md` when the work is primarily report
   refresh, ranking, or wave summary generation.

Selector rules:

- once a raw EMI entry has been imported, prefer canonical program selectors
  such as `/bins/BIN/WORLD00/AREA016/6.bin`
- use `python3 -m scripts.rebof3 match target --program ... --entry ...` to
  resolve the durable workspace, source file, and bundle identity without
  triggering build work
- use `python3 -m scripts.rebof3 match init --program ... --entry ...` or
  `make match_init PROGRAM=... ENTRY=...` to create the workspace once

## Scheduling Strategy

Scheduling inputs come from these report surfaces:

- `match status` / `match refresh` for project-wide function, program, family,
  duplicate, and artifact coverage
- `match import-backlog` for missing canonical program rows
- prefer the frontier backlog emitted by `match refresh` for imported
  zero-function overlays and promotion / seed candidates
- `match compiler-report` for already-promoted one-file matching work

Use inverse DFS:

- start from the smallest leaf-like functions that make sense
- prefer functions with:
  - small instruction count
  - low dependency depth
  - low dependency risk
  - obvious source-shape opportunities
- move upward into callers only after nearby leafs stop yielding

Prioritize in this order:

1. smallest leaf-like functions with real structural mismatches
2. small helpers blocking nearby callers
3. medium functions whose remaining mismatch is clearly source-shape-driven
4. near-exact cleanup only after the local leaf/helper floor is reasonably clear
5. larger callers after helper chains are cleared

Do not default to chasing the highest percentages first.

## Worker Pool

- Keep worker slots busy, subject to active-agent and system limits.
- Give each worker exactly one file at a time.
- Use disjoint write scopes only.
- Reassign a finished worker immediately to the next smallest viable candidate.
- Reuse local context for the area you are actively iterating on yourself.

## Verification Cadence

During active iteration:

- resolve the target early with `python3 -m scripts.rebof3 match target ...` if
  you want the canonical workspace, source file, and bundle identity without
  triggering build work
- create the workspace once with `match init` or `make match_init`, then reuse
  its `workspace.json` for build, view, diff, and permuter steps whenever
  possible
- prefer the direct `match ... --workspace-json <tmp/matching/.../workspace.json>`
  form when you already have the workspace path in hand
- `make match_build` defaults to the one-file `compile-one` path; only fall back
  to a full build when `compile-one` is unavailable or the task specifically
  needs it
- use targeted single-function canonical reports only
- verify promoted changes immediately
- avoid full global reruns after every small improvement
- when `match compiler-report` prints to stdout, the human-readable default is
  `--stdout-view summary --stdout-format brief`
- prefer stdout-first targeted checks for quick prototype loops:
  - `--output-mode stdout --stdout-view functions --stdout-format brief --ephemeral`
- or use the shorthand fast loop:
  - `--quick`
- when the next source-shape change is not obvious from stdout alone, run the
  local side-by-side loop:
  - `make match_build PROGRAM=... ENTRY=...`
  - `make match_view PROGRAM=... ENTRY=...`
- treat `objdiff_match_percent` as triage only; use `make match_view` to classify the
  real mismatch before the next edit

At a checkpoint:

- rerun the full canonical report across `bof3/src`
- when trivial placeholder stubs exist, prefer `--skip-empty-stubs`
- refresh rankings and next candidates from that report
- prefer `python3 -m scripts.rebof3 match refresh` when you want scoreboard,
  import backlog, frontier backlog, and status outputs regenerated together
- refresh human-facing status outputs with:
  - `python3 -m scripts.rebof3 match status`
  - `python3 -m scripts.rebof3 match refresh`
- prefer the tracked status snapshot first when you need a commit-friendly view
  of project-wide progress and artifact/build coverage
- use `@.opencode/commands/decomp-checkpoint.md` when the task is primarily
  report refresh, ranking, and wave summary generation

## Matching Style

- Prefer source-shape fixes before permuter, but treat bounded permuter runs as
  an important standard companion once a credible seed reaches a short manual
  plateau.
- Prefer type and address-materialization fixes before control-flow rewrites.
- Use conservative address-stable names in source.
- Keep comments short and factual.
- Never worsen canonical match just to satisfy another compiler unless that is
  the explicit goal.

Readable C is a goal, not a luxury:

- prefer the cleanest readable C expression that still preserves competitive
  canonical codegen
- if a cleaner construct loses the target shape, drop only as far as needed
- do not keep ugly magic-address spellings when a local macro / define / table /
  extern can express the same codegen
- keep compile-time helpers only when they are codegen-neutral or improve the
  canonical object

## Per-Function Preamble

When a function depends on figured-out offsets, tables, fixed addresses, or a
small local layout, declare them immediately above the function.

Prefer this order:

1. local `#define` or small macro block
2. local `extern` declaration
3. tiny local table or typedef
4. small local struct for a proven reused mini-layout
5. larger overlay struct only when multiple nearby fields are proven and reused

Guidelines:

- for one-off accesses, prefer a small macro block above the function over a
  synthetic padded overlay struct
- start these constructs local to the function first
- use the smallest readable approximation that captures the currently proven
  layout or address shape; avoid speculative oversized overlays when a couple of
  local fields, defines, or macros express the same understanding more clearly
- before inventing a new local alias, check whether an existing repo/common
  header already expresses the same contract cleanly enough without hurting
  codegen
- reuse established shared declarations (for example repo-owned BOF3 aliases or
  existing PsyQ-facing headers) when they already match the shipped interface;
  do not create duplicate local spellings of the same stable contract
- promote to `internal.h` only after repeated use and stable meaning
- when a stabilized lift starts depending on shared local declarations,
  synchronize `internal.h` with the live function instead of leaving duplicate
  or stale local spellings behind
- if a later rewrite makes a local `#define`, macro, typedef, or `internal.h`
  helper unnecessary, remove it instead of keeping dead compatibility clutter
- do not leave unexplained repeated numeric offsets in the function body when a
  conservative local declaration can express them more clearly

## Fixed-Address And Literal Rules

Before settling on a raw numeric address or offset, inspect Ghidra for:

- callers and callees
- data xrefs
- stack variables and parameters
- nearby tables, globals, and repeated offsets

Prefer portable C constructs for fixed-address interaction when they still match
well:

- preprocessor macros / defines
- local pointer or function-pointer tables
- readable `extern` declarations
- small proven structs or overlays, only when reuse and meaning justify them
- existing shared headers or PsyQ declarations when they already express the
  same fixed-address or ABI contract without adding churn

Literal spelling:

- prefer hex for addresses, bitmasks, packed fields, and alignment-sensitive
  values
- prefer decimal for counts, loop bounds, sizes, timers, and plain gameplay/UI
  values when that reads more like source
- if either spelling could work, choose the one that is simplest, clearest, and
  still matches the target codegen

Avoid inline asm symbol-binding tricks for absolute addresses unless there is no
practical C expression of the target shape and the exception is explicit.

## Naming And Comments

- keep repo-owned identifiers conservative until meaning is well supported
- when confidence is high, add brief comments carrying likely meaning for:
  - variables
  - procedures / dispatchers
  - structs and fields
  - tables, handlers, and state bytes
- meaningful local macro / typedef / table names are acceptable when backed by
  strong evidence, even if primary function and field identifiers stay
  conservative
- readability now matters: prefer code that another decomp pass can cleanly
  rename and refactor later over brittle, mechanically literal clutter
- promote comments into identifiers only after reuse and stability justify the
  churn
- keep per-function helper blocks and nearby `internal.h` declarations tidy:
  remove unused defines, duplicate aliases, and stale declarations once the
  function settles into a cleaner shape

## Stub Lane

- disabled placeholders belong under `bof3/stubs/...`, not `bof3/src/...`
- keep stub paths mirrored to the shipped module structure as closely as
  possible
- keep slot directories canonical and zero-padded in both `src` and `stubs`
- only promote a stub into `bof3/src` after it has been validated for the
  active phase

## Permuter Policy

Use permuter after a short manual plateau as a normal search tool, not as an
embarrassed last resort.

Good triggers:

- the function already compiles cleanly
- the remaining mismatch still looks structurally reachable
- two or three narrow manual source-shape attempts stopped improving the result
- the function is at a plausible local maximum and worth a standard search pass

Rules:

- use the repo-owned wrapper
- use an agent-aware CPU budget, not all CPUs at once
- once the function compiles and has a credible seed, expect to use bounded
  permuter passes as part of normal iteration
- standard worker pass:
  - `--timeout-seconds 60 -- --better-only --best-only --stop-on-zero`
- when the repo lift is a poor starting point but `func.m2c.c` is promising,
  try the alternate lane:
  - `make match_permuter PROGRAM=... ENTRY=... MATCH_PERMUTER_ARGS='--variant m2c --timeout-seconds 60 -- --better-only --best-only --stop-on-zero'`
- if a 60-second pass still finds real improvement, fold the best candidate back
  into the manual loop and keep working the task
- run another 60-second pass only when the updated function still looks
  structurally reachable and manual edits plateau again
- use longer runs only with explicit justification after repeated 60-second
  passes stop changing the outcome materially
- treat the best candidate as a hint until you compile and verify it directly
- do not promote a candidate unless canonical match improves or at least does
  not regress in a meaningful way
- do not let artifacty near-match permuter output become the new target shape
  unless canonical verification shows a real improvement
- do not keep raw `m2c` output as the final source shape when a cleaner local
  macro / define / table / declaration spelling can preserve the same match

Task completion rule:

- do not stop a task just because one permuter run finished
- keep iterating until the function reaches exact match, a plausible pure-C /
  toolchain ceiling, or a scope boundary that must be reported back

## Maspsx / Version Sweep Policy

Do not sweep `maspsx` versions blindly.

Only try alternate ASPSX-version behavior when the remaining mismatch clearly
looks like:

- `$at` expansion behavior
- inserted nop around `$at` expansion
- `li 1` expansion
- `%hi/%lo` macro handling
- `$gp` / non-zero `-G` behavior

Do not expect `maspsx` version changes to fix:

- generic GNU `as` pseudo-op choices like `or` vs `addu` for `move`
- plain GCC source-shape issues already visible before `maspsx`
- non-`$at` `ori` vs `addiu` address materialization unless proven

## Throughput Rules

- keep scratch under `tmp/`
- clean temporary experiment files when they are no longer useful
- ignore unrelated dirty-worktree changes
- do not rewrite shared headers casually just to fix one function
- prefer one-file local fixes over broad header churn

## Checkpoint Output

At each checkpoint, report:

- functions promoted this wave
- old vs new canonical match percent
- exact matches gained
- active worker frontier
- current permuter jobs and best scores if any
- next smallest viable candidates
- whether a full canonical report was rerun
