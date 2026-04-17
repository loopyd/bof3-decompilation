---
description: Refresh BOF3 canonical reports and summarize the next decomp wave
agent: build
subtask: true
---

## Purpose

Run a BOF3 checkpoint pass focused on reporting, ranking, and next-wave
planning.

This command is the checkpoint/report companion to:

- `@.opencode/commands/decomp.md`
- `@.opencode/commands/decomp-worker.md`
- `@.opencode/commands/decomp-mismatch-routing.md`

## Contract

- Do not do broad code edits as part of this command.
- Prefer verification, report refresh, ranking, and summary.
- If a small local fix is unavoidable to complete verification, keep it
  explicit and narrow.

## Primary Goals

1. refresh the canonical scoreboard
2. summarize what improved in the current wave
3. identify the next smallest viable candidates
4. separate likely pure-C wins from likely toolchain dead ends

## Current Report Surfaces

Prefer these generated surfaces instead of rebuilding ad hoc summaries:

- local status snapshot: `tmp/status/<profile>/current/`
- tracked status snapshot: `reports/decomp-status/current/`
- backlog and scoreboard reports: `tmp/matching/_reports/`

Useful files:

- `status.md` for the human-readable checkpoint summary
- `status.json` / `scoreboard.json` for machine-readable totals and coverage
- `import_backlog_*.json|tsv` for missing canonical program rows
- `frontier_backlog_*.json|tsv` for imported zero-function programs waiting on
  promotion or seed work
- `scoreboard_*.json|tsv` for function/program/family rollups

## Canonical Checkpoint Flow

1. Ensure the current worktree is in a reportable state.
2. Rerun the full canonical report across `bof3/src`:
      - `python3 -m scripts.rebof3 match compiler-report --compiler gcc-2.7.2-psx --source-prefix bof3/src ...`
3. Regenerate the shared report surfaces with:
      - `python3 -m scripts.rebof3 match refresh`
   Add `--tracked-output` when the checkpoint is meant to update the shared
   report surface under `reports/decomp-status/current/`.
   `match refresh` also carries the current artifact registry summary,
   including how much of the build graph is still placeholder-, archive-, or
   raw-output-backed.
4. Use `python3 -m scripts.rebof3 match status` only when you specifically want
   the status snapshot refreshed without re-emitting the full `_reports` set.
5. Read the human-facing summary first, then the raw artifacts if needed.
6. Build the next candidate queue from the fresh report surfaces in lane order:
      - import backlog reps/members for missing canonical program rows
      - frontier backlog promotion candidates for imported zero-function programs
      - frontier backlog seed candidates for imported zero-function programs
      - compiler-report candidates for already-promoted one-file matching work
7. Cross-check stubborn blockers against:
    - `func.s`
    - `func.m2c.c`, or one manual rerun if it is missing and worth retrying
    - `func.ghidra.c` for inspection context
8. Summarize:
      - total successful functions
      - average and median match
      - exact matches
      - biggest gains since the last checkpoint if that comparison is available

## Ranking Rules

Prefer the next wave in this order:

1. representative import-backlog rows that unlock missing canonical program rows
2. imported zero-function programs with `promotable_entry_labels`
3. imported zero-function programs with credible `manual_frontier` seeds
4. smallest leaf-like functions with real structural mismatch classes
5. small helpers directly feeding nearby callers
6. functions in the same local area as a recent successful fix
7. low-match functions with clear pure-C levers such as address materialization,
   cast placement, or helper folding
8. near-exact cleanup only after the local bottom-up floor is reasonably clear

Deprioritize:

- one-file worker work for targets that still have no canonical program row or
  no source mapping
- pure pseudo-op dead ends
- relocation-name-only differences
- high-percent near-matches at a likely pure-C ceiling unless a new lever
  appeared

## What To Extract From The Report

Always pull out:

- top import backlog representatives and deferred duplicate groups
- top frontier promotion / seed candidates
- top exact or near-exact functions
- bottom small functions worth trying first
- active wave gains
- current worker-friendly queue

Useful slices:

- queued `import_representative` and `import_member` rows
- `promotable_entry_labels` vs `manual_frontier` frontier rows
- smallest non-exact functions
- smallest low-match functions with leaf-like behavior
- functions `>= 90%` for secondary cleanup only
- functions with one mismatch
- functions in the same module/family as recent wins

## Speed Rules

For fast iteration:

- use single-function reports during active implementation waves
- use a full canonical rerun only at a real checkpoint
- let `match status` / `match refresh` carry the human-facing summary and tracked
  snapshot output instead of rebuilding ad hoc summaries by hand

If discussing report-speed improvements, recommend these in order:

1. cache unchanged rows between runs
2. parallelize compile/report jobs in the report tool
3. avoid rerunning the full report after every tiny edit

## Output

When this command finishes, report:

- report paths
- which path is the local/tracked status root and which path is the `_reports`
  refresh root
- total canonical summary in concise human-readable form first
- exact matches gained this wave
- notable improvements
- next 5-10 smallest viable candidates
- each next candidate's lane: import, promote, seed, repair, or one-file match
- which candidates look blocked by pseudo-op or toolchain issues
- whether any blockers need a manual `m2c` retry before being called ceilings
