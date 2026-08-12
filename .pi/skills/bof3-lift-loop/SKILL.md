---
name: bof3-lift-loop
description: Autonomously lift a serial BOF3 batch to reviewed exact byte matches. Selects candidates, dispatches bounded executor/reviewer agents, and commits only user-authorized reviewed exact lifts. Not for one hand-guided lift; use `/skill:bof3-re`.
---

# BOF3 lift loop

Read `AGENTS.md`, load `/skill:bof3-re`. Parent owns selection/checkpoints/git. Reverse writes one function; review is independent and may record durable findings. Never run same-target functions concurrently (`internal.h`).

## Confirm

Targets (default reviewed), selection (`quick-wins`), budget (3–5), exclusions, branch, parallelism (`1`), explicit commit authorization, explicit decomp.me publication authorization (partials in scope). Parallelism = max lanes, not agents per function. For this run, the user explicitly set parallelism to `10`; preserve distinct-target ownership and single-thread each function pipeline.

## Baseline + queue

```sh
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots
# stale evidence:
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots --recover
```

Never dispatch dirty/stale. `loop-status` fails closed; `--recover` serially repairs generated evidence and rebuilds index. After map/Splat edits refresh affected snapshots, then index once. Init `out/lift-loop/results.tsv`: `function status commit notes` (evidence paths/SHA-256 allowed; checkpoint aids, never live acceptance).

## Function pipeline

For execution, render [`references/workflow-script.md`](references/workflow-script.md) with `scripts/render-workflow.py`, verify it, and submit its exact contents directly as `subagent.workflowScript`. No model copies or rewrites orchestration. `scripts/lane-worktree.py` creates one parent-managed lane worktree; launch the rendered workflow with `cwd` set there and `worktree:false`, so every nested phase shares it. The parent owns queue, worktree lifecycle, handoff consolidation, git, and freshness.

Each function is single-threaded regardless of batch parallelism:

```text
bof3-reverse -> bof3-review -> [retained exact|partial] bof3-cleanup -> live gates -> bof3-review -> integration
```

- One function lane; ≤10 executor attempts incl. first. Keep an ordered per-lane attempt ledger containing every executor result, review, tested lever, expected/actual instruction effect, accept/revert outcome, and host checkpoint. Pass it to every fresh executor/reviewer. Non-exact review audits the ladder, then asks what other untried experiments live evidence suggests. It returns 1–3 safe concrete candidates with predicted observable effects. Run one at a time. `attempt-checkpoint.py` records each result; no-progress restores best, exhausts only that experiment, then repeats open discovery. Never rely on prompt-only best preservation. A repeated lever requires new evidence predicting a different observable effect. Never use retained-child `resume` for mutation retries: it may detach and return before edits finish, racing review against an unstable worktree; the explicit ledger preserves context instead. Stop only for exact, rejection/blocker, ten attempts, or reviewed ladder + open-discovery exhaustion. Experiment-free `needs-fix` is invalid.
- Reviewed improvements record the decisive generic lever in the narrowest playbook/lesson before integration—never selector/percentage/case narrative. Mark exact vs partial; function-only → `lesson: none`.
- Cleanup every retained exact/partial: evidence-backed naming and target-local integration only. Exact requires fresh diff/bytes/symbols/Splat/relocation + review. Partial is spelling-only; preserve body/ABI/boundary/compiler and atomic status/match/residual, score ≥ best + audits/review. Failure restores pre-cleanup.

## Serial loop (`parallelism=1`)

Create one lane with `lane-worktree.py create`, render/verify its script, then launch the exact script with lane `cwd` and `worktree:false`. Before dispatch run `.pi/skills/bof3-re/scripts/function-brief.py SELECTOR` and relevant `companion-check`. Parent accepts only the exported manifest/patch + retained state + cleanup gate + final review: exact commits as `feat(decomp): byte-match <function>`; authorized partial commits keep atomic metadata. Restore no-progress, rejected semantics/types, or cleanup regression. Use `subagent_supervisor` for child requests.

## Parallel loop (optional `parallelism>1`)

Freeze one queue; launch target-distinct rendered workflows as independent async calls, each parent-managed with `worktree:false` and unique absolute `sessionDir`. Never outer `runs.all` (aliases `run-0/session.jsonl`). Requirements:

- Never place two live lanes from the same target together. Partition by distinct `TARGET`; defer collisions.
- Create with `python3 .pi/skills/bof3-lift-loop/scripts/lane-worktree.py create --key WAVE-LANE --selector SELECTOR`; it bootstraps executable modes, tools, `.venv`/`inputs`, compilers, and disposable `out/`.
- Render/verify with `render-workflow.py`, then call `subagent({workflowScript: SCRIPT, cwd:"../.bof3-lift-worktrees/WAVE-LANE", worktree:false, sessionDir:"/absolute/project/.pi-subagents/sessions/WAVE/LANE", async:true, ...})`. Launch calls back-to-back. Nested phases serialize in that cwd.
- No lane commits, pushes, stages, edits another target, or shares publicly. Generated `out/`, `build/`, compile DB, analyzer/index state, source stubs, and `.pi-subagents/` are never merge artifacts.
- After success run `lane-worktree.py export --key WAVE-LANE --selector SELECTOR`; its JSON manifest and binary patch are authoritative. After consolidation or rejection run `lane-worktree.py remove --key WAVE-LANE`. Reject failed/partial capture, unexpected target paths, `src/emi/`, absolute-path stubs, unapproved `INCLUDE_ASM`, cleanup rollback failure, or final-review rejection. Discard rejected preserved worktrees only through `worktree.discard` with its handoff path.
- Consolidate serially in queue order onto clean parent: verify base/overlap, apply/check patch, rerun gates + fresh review, commit only if authorized. Advanced `HEAD`: three-way only after explicit conflict review, else rerun. Atomic ownership transactions only.
- Refresh edited target snapshots serially after consolidation, then rebuild the global index once. A lane failure consumes only its selector; dirty parent, overlap, stale/conflicting handoff, failed rollback, or failed freshness recovery stops consolidation/new dispatch.

`parallelism=1` remains the default and still uses one managed worktree; higher values fan out isolated lane worktrees.

## Post-loop audit

Audit retained lanes for missed integration only; do not repeat passed cleanup. New exact edits require live `asm-diff`/`byte-match` and review; partial edits require unchanged-or-better live score and metadata. Revert regression.

## Partial re-lift + decomp.me final rung

User-authorized fresh `out/non-exact-lifts.json` pass after queue + checkpoint. Process `partial` rows serially, preserving each source's pre-mission state. Skip `contains_data` rows (range embeds reviewed `D_*` data; unliftable until Splat segment splits — route to user, never dispatch). Remaining rows: same ≤10-attempt executor↔review loop, then parent evidence check.

Exhausted non-exact: lesson, restore prior state, then `bin/scratchpad share SELECTOR`. Publish only reviewed Splat `c`/`asm` function start with restored source. Data/non-function/unreviewed/source-less → journal not-shareable reason. Scratch is escalation evidence, never acceptance or map/Splat authority.

## Stop/report

Never stop for: non-exact candidate, unshareable, publish failure, review rejection of an exact claim, bounded escalation. Journal + continue. Stop only: queue exhausted, budget reached, evidence-recovery fatal, child output conflicts with owned worktree, user approval required. Print journal, counts, commits, scratch URLs/results, risks, next step.

Role protocols are preloaded by `agent-context.py`; the reference workflow supplies compact child tasks and ownership limits.
