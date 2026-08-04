# Harness consistency cleanup

Consolidated remediation plan from the 2026-08-01…03 deep harness audit
(artifacts `.pi-subagents/artifacts/4217d599_reviewer_{0,2}_output.md`) and the
user's automation goals: a reliable reverse → review → cleanup → review loop,
lean agent contexts, and DRY, contradiction-free harness contracts.

## Evidence baseline

Verified 2026-08-04 against the live tree:

- **Fixed already** (this effort, validated by `just check` — 284 passed):
  - `agent-context.py` no longer preloads `docs/specs/**/*.md`; mission context
    dropped from ~260 KB to ~79–80 KB; `test-skill-scripts.py` asserts specs are
    absent and output is under 100 KB.
  - Matching/memory docs moved to `docs/agents/` with all references updated.
- **Still open:**
  - `.pi/agents/bof3-cleanup.md:3` — bare description sentence, not a YAML key;
    PyYAML `ScannerError` on this file alone; the cleanup role cannot load.
  - `.pi/chains/bof.chain.json` — Recon phase runs `context-builder` and `scout`
    in parallel with materially identical prompts (duplicate spend; output-path
    collision was fixed earlier: scout now writes `scout-context.md`).
  - `.pi/chains/bof.chain.json` worker prompt still contains lift instructions
    (`TARGET@0xADDRESS` guidance) under concurrency 3 — conflicts with the
    lift-loop rule "never run two functions in one target concurrently: they
    share `internal.h`" (`.pi/skills/bof3-lift-loop/SKILL.md:11`).
  - No cleanup+re-review gate exists after a successful lift (user-requested
    architecture: exact claim → review pass → scoped cleanup → fresh review).
  - `.pi/skills/bof3-lift-loop/scripts/loop-status.py` reports dirty worktree
    but still emits candidates; a parent ignoring `next_action` can dispatch
    against a dirty tree.
  - `docs/usage.md:129-133` describes the lift loop as committing reviewed
    lifts without stating the explicit user commit authorization required by
    `AGENTS.md:103` and `.pi/skills/bof3-lift-loop/SKILL.md:14-17`.
  - `docs/index.md:10-13` lists six agent references inline and repeats them
    immediately under "Agent operating references" (duplicate maintenance
    surface).
- **Stale audit item (no action):** the chain's `delegate` agent now resolves
  via the pi-subagents builtin (`subagent action=list` shows `delegate
  (builtin)`); no project-owned definition needed.

## Phase 1 — Cleanup role contract repair ✅ completed 2026-08-04

Fixed the one malformed agent front matter and locked the contract.

Changes:

- `.pi/agents/bof3-cleanup.md:3` — change the bare sentence to
  `description: Audit and repair one evidence-backed BOF3 naming, documentation, or organization inconsistency without breaking target identity or matching contracts`.
- `tools/python/tests/test_bof3_cleanup_agent.py` — extend (or add one test)
  that parses every `.pi/agents/*.md` front matter as a YAML mapping and
  asserts each has `name` and `description`.

Validation: focused pytest; `subagent action=list` resolves `bof3-cleanup`.

## Phase 2 — Generic chain hygiene (`bof.chain.json`) ✅ completed 2026-08-04

The chain is the general project-change/refactor workflow, **not** a lifting
pipeline (user correction, 2026-08-02). Keep it generic and non-duplicative.

Changes:

- Remove the `scout` step from the Recon parallel block and drop
  `Scout handoff file: {outputs.scoutResult}` from the planner task;
  `context-builder` already supplies the planner handoff. (`researcher` stays:
  external-docs scope is distinct.)
- Remove the lift paragraph from the worker task ("If this task lifts a
  TARGET@0xADDRESS function …") and replace with one sentence: lift work is
  out of scope for this chain; route `TARGET@0xADDRESS` work to
  `/skill:bof3-lift-loop`.
- `tools/python/tests/` — add one chain-contract test: chain JSON parses;
  exactly one repository-recon handoff feeds the planner; worker task text
  contains no `TARGET@0xADDRESS` lift instructions and names the lift loop as
  the route for lifts.

Validation: focused pytest; `python3 -m json.tool .pi/chains/bof.chain.json`.

## Phase 3 — Post-lift cleanup + re-review gate (`bof3-lift-loop`) ✅ completed 2026-08-04

Implement the user-requested gate in the serial lift loop only.

Changes:

- `.pi/skills/bof3-lift-loop/SKILL.md` — after an exact lift passes
  `bof3-review`, add an optional cleanup pass: dispatch `bof3-cleanup` for
  cosmetic, evidence-preserving changes only (naming, comment metadata,
  organization within owned files); then require a fresh live `byte-match` and
  a fresh `bof3-review` pass before the function is eligible. A cleanup that
  breaks byte-match is reverted, never "fixed forward".
- `.pi/agents/bof3-cleanup.md` — state the cosmetic-only boundary and the
  mandatory post-cleanup live `byte-match` re-verification.
- `tools/python/tests/test_bof3_lift_loop_acceptance.py` — assert the skill
  documents the cleanup → fresh byte-match → fresh review gate.

Validation: focused pytest; no live lift changes.

## Phase 4 — Loop-status fail-closed dispatch ✅ completed 2026-08-04

Changes:

- `.pi/skills/bof3-lift-loop/scripts/loop-status.py` — when staged or unstaged
  changes exist, suppress candidate emission (or emit explicit
  `dispatch_allowed: false` that the skill requires the dispatcher to honor).
- `tools/python/tests/test_bof3_lift_loop_status.py` — update the
  dirty-worktree test to assert dispatch suppression.

Validation: focused pytest.

## Phase 5 — Documentation consistency ✅ completed 2026-08-04

Changes:

- `docs/usage.md:129-133` — state that the loop commits only reviewed exact
  lifts **after explicit user commit authorization**.
- `docs/index.md` — delete the inline six-link sentence (lines 10-13); keep the
  "Agent operating references" list as the single surface.
- `tools/python/tests/test_bof3_lift_loop_acceptance.py` — assert the usage
  text carries the explicit authorization wording.

Validation: focused pytest; local Markdown link scan.

## Phase 6 — Python harness DRY and naming consistency ✅ completed 2026-08-04
 
Evidence baseline (verified 2026-08-04 by AST scan of `tools/python/harness`
and wrapper inventory of `bin/`):
 
**Verified strengths — do not re-flag:**
 
- 22/26 command modules share `run_main` (`commands/_common.py`); the four
  exceptions are justified (`__init__`, helper `_asm_diff_output`, the `lift.py`
  multi-command dispatcher, and `permute.py` — see finding below).
- Downloads/extraction are centralized (`toolchain/helpers.py:download_file`,
  `toolchain/releases.py:extract_archive` with link rejection);
  `gcc_archive._download_to_cache` is intentionally distinct (atomic publish +
  digest verification), not a DRY violation.
- AST hash scan found no identical function bodies; `bin/` wrappers share no
  identical bodies and are thin (4–43 lines; `bin/cc` is a documented build
  adapter, exempt per `docs/usage.md`).
- `toolchain/base.py` uses a proper Template Method (`run` = install → build →
  verify); subclass similarity is contract, not duplication.
 
**Findings (smallest safe fixes):**
 
1. **DRY — `--root` boilerplate ×18.** Eighteen command modules repeat
   `parser.add_argument("--root", type=Path, default=repo_layout().root)`.
   Add `add_root_argument(parser)` to `commands/_common.py` and replace all 18.
2. **DRY — `--example` handling ×7, three different shapes.**
   `commands/lift.py:373,408`, `analysis_sequence.py:118`, `flag_search.py:61,68`,
   `symbols.py:420`, `emi_target.py:51`, `permute.py:289-292` (raw argv scan),
   `build.py` (handler check). Add one shared example mechanism in
   `_common.py` (e.g. `add_example_argument(parser, text)` + a `run_main`
   early-return), then align all seven.
3. **DRY — `permute.py:289-310` re-implements `run_main`.** It duplicates the
   parse + `except (FileNotFoundError, RuntimeError, ValueError)` → `error:` → 2
   mapping. Move `--example` and selector resolution into the handler and call
   `run_main(build_parser, argv)`. Lock behavior first with the existing
   characterization tests for the permute coordinator.
4. **Naming — same name, different semantics (readability hazards).**
   Rename the loser, keep the owner:
   - `archive_path_looks_valid`: `toolchain/releases.py:21` (symlink+file+suffix)
     vs `toolchain/psyq.py:179` (delegates to `archive_file_looks_valid`).
   - `index_path`: `reverse_index.py:25` (`out/index/reverse.sqlite`) vs
     `psyq/signatures.py:60` (PsyQ signature index).
   - `function_name`: `layout.py:31` (section property) vs
     `commands/permute.py:23` (module function).
5. **Watch-list — decomposed 2026-08-04 (user directive: skip nothing).**
   Mechanical verbatim segment moves; originals re-export, so import surfaces
   are unchanged. Characterization suites locked behavior; only monkeypatch
   module targets moved with the code.
   - `commands/rev_query.py` 806→274 (+ `_rev_query_graph`, `_rev_query_priority`,
     `_rev_query_mission`)
   - `emi/catalog.py` 624→228 (+ `catalog_verify`, `catalog_bootstrap`)
   - `psyq/signatures.py` 577→383 (+ `signature_calls`)
   - `decomp_status.py` 514→283 (+ `decomp_status_preflight`)
   - Gate sweep then caught 4 more: `commands/lift.py` 418→~250 (+ `_lift_m2c`),
     `commands/symbols.py` 422→~230 (+ `_symbols_psyq`),
     `match/asm_diff.py` 457→~40 (+ `_asm_diff_payload`, `_asm_diff_run`),
     `toolchain/psyq.py` 470→~150 (+ `psyq_discovery`)
   - **Seal:** `test_harness_dry.py::test_harness_modules_stay_decomposed`
     fails any harness module over 450 lines — decompose before growing.
 
Changes land only in `tools/python/harness/` and its tests; no target source,
map, Splat, or toolchain behavior changes.
 
Validation: `just check`; focused pytest for `commands/_common` consumers and
the permute characterization suite; `bin/byte-match` unchanged (no lift impact).

## Acceptance criteria

- `just check` passes end-to-end.
- All `.pi/agents/*.md` front matter parses; `bof3-cleanup` is invocable.
- Chain has one recon handoff and no lift dispatch in the generic fanout.
- Lift-loop skill documents cleanup → fresh byte-match → fresh review.
- `loop-status` cannot emit dispatchable candidates on a dirty tree.
- `commands/_common.py` owns `--root` and `--example`; `permute.py` reuses
  `run_main`; the three semantic name collisions are resolved.

## Blockers and ownership

- Each phase edits only the files listed; no target source, map, or Splat
  changes are in scope.
- Commit only with explicit user authorization (one commit per phase or one
  reviewed aggregate, user's choice).

## Non-goals

- Redesigning `bof.chain.json` into a lifting chain (it stays generic).
- Relocating lifted sources into semantic folders: CMake globs recursively,
  but `symbols check`, function-context generation, lift resolution, and Splat
  `@source` all assume `source_dir/func_XXXXXXXX.c`. Relocation needs a
  separate evidence-backed tooling plan first; doing it now would create
  split-brain metadata.
- Bulk symbol renaming — proceeds per-lift through existing review gates.
- Vendored `third_party/` cleanup.
