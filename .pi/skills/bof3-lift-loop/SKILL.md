---
name: bof3-lift-loop
description: Autonomously lift a serial BOF3 batch to reviewed exact byte matches. Selects candidates, dispatches bounded executor/reviewer agents, and commits only user-authorized reviewed exact lifts. Not for one hand-guided lift; use `/skill:bof3-re`.
---

# BOF3 lift loop

Read `AGENTS.md`, load `/skill:bof3-re`. Parent owns selection, checkpoints, git, commits. `bof3-reverse` writes one function; `bof3-review` independently reviews it, may record durable cross-function findings in `docs/specs/`/`docs/agents/lessons.md`. Agents own model/tool policy. Never run two functions in one target concurrently (shared `internal.h`).

## Confirm

Targets (default reviewed), selection (`quick-wins`), budget (3–5), exclusions, branch, parallelism (`1`), explicit commit authorization, explicit decomp.me publication authorization (partials in scope). Parallelism = max lanes, not agents per function. For this run, the user explicitly set parallelism to `10`; preserve distinct-target ownership and single-thread each function pipeline.

## Baseline + queue

```sh
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots
# stale evidence:
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots --recover
```

Never dispatch on dirty tree/index. `loop-status` fails closed (`dispatch_allowed: false`) on stale snapshots/index or changes; `--recover` repairs only stale generated evidence serially, then rebuilds index once; queue from one fresh snapshot/index. After map/Splat edits: continue bounded queue, refresh edited target snapshots, rebuild index once at checkpoint. Never query a stale index. Init `out/lift-loop/results.tsv`: `function status commit notes` (evidence paths/SHA-256 allowed; checkpoint aids, never live acceptance).

## Function pipeline

For execution, load [`references/workflow-script.md`](references/workflow-script.md), replace only its selector array, set its unique wave `RUN_KEY`, and submit the full template as one async `subagent({ workflowScript })` call per wave. Do not expand its compact role contracts into repeated parent-authored prose or separate tool calls. The reference script is the executable retry authority, including explicitly repairable reviewer blocks; this file supplies invariants, not a duplicate branch specification. The script handles executor/reviewer retries, retained exact/partial cleanup, gates, and final review; the parent retains queue, generic-lever, integration, git, and freshness authority.

Each function is single-threaded regardless of batch parallelism:

```text
bof3-reverse -> bof3-review -> [retained exact|partial] bof3-cleanup -> live gates -> bof3-review -> integration
```

- One function lane; ≤6 executor attempts incl. first. Keep an ordered per-lane attempt ledger containing every executor result, review, tested lever, expected/actual instruction effect, accept/revert outcome, and host checkpoint. Pass it to every fresh executor/reviewer. Non-exact review returns 1–3 ranked untried experiments, each predicting an observable size/frame, CFG/branch/loop, first-mismatch/offset, or named instruction/register/load/store effect. Launch one variant at a time. After every attempt, `attempt-checkpoint.py` records score, sizes, first mismatch, and all owned files; a non-improving attempt fails its host gate, restores the best checkpoint, exhausts that lever, and terminates the lane for review. Never rely on prompt-only best preservation. A repeated lever requires new evidence predicting a different observable effect. Never use retained-child `resume` for mutation retries: it may detach and return before edits finish, racing review against an unstable worktree; the explicit ledger preserves context instead. Stop early: exact; first unchanged/regressing experiment; rejected semantics/types; approval/safety or external blocker; reviewer `pass` with attested ladder exhaustion. Experiment-free `needs-fix` invalid.
- Every experiment that produces a reviewed, reproducible net match improvement identifies its decisive lever and before integration records only the generic reusable rule in the narrowest playbook/lesson. Do not add function selectors, percentages, or case narratives to the playbook. State whether evidence is exact or partial in the review/journal; partial evidence is a candidate lever, never a universal rule. Function-only effects → `lesson: none` + evidence. Partial→exact levers must be recorded before integration.
- Cleanup runs after every reviewed retained exact or partial. It performs evidence-backed semantic function/source/Splat naming and integrates target-local symbol imports, declarations, weak bindings, metadata, and owned-file consistency; never broaden ownership or invent semantics. Exact cleanup requires fresh `asm-diff`, `byte-match`, `symbols check`, Splat/naming/relocation audit, and fresh review. Partial cleanup is spelling/integration-only: preserve body/ABI/address/boundary/compiler settings and atomic `@status partial`/`@match`/`@residual`, require fresh live score ≥ reviewed best plus symbols/Splat/naming audits and fresh review. Failure restores the reviewed pre-cleanup exact/partial checkpoint.

## Serial loop (`parallelism=1`)

Use the reference script unchanged except selectors/`RUN_KEY`; it is the executable retry/cleanup authority. Before dispatch run `.pi/skills/bof3-re/scripts/function-brief.py SELECTOR` and relevant `companion-check`. Parent accepts only retained state + cleanup gate + final review: exact commits as `feat(decomp): byte-match <function>`; authorized partial commits keep atomic metadata. Restore no-progress, rejected semantics/types, or cleanup regression. Use `subagent_supervisor` for child requests.

## Parallel loop (optional `parallelism>1`)

Freeze one fresh queue, then execute the reference workflow script with up to `parallelism` target-distinct selectors. It uses `runs.all` for each phase and ordinary JavaScript for bounded per-lane branching/retries. Requirements:

- Never place two live lanes from the same target together: target-local map, Splat, manifest, support source, `internal.h` are shared. Partition each wave by distinct `TARGET`; defer collisions.
- Every lane owns exactly one selector and the script runs its bounded executor ↔ reviewer retry loop -> retained exact/partial cleanup -> gates -> re-review serially. Cleanup/audit may overlap another target's lane.
- Managed-worktree launches require one absolute project-owned `sessionDir` **per top-level lane** (for example `$PWD/.pi-subagents/sessions/<batch>/<lane>`); one shared directory makes parallel `run-0/session.jsonl` writers collide. Launch lanes separately when the workflow API cannot set child session directories. RE lanes also require the ignored `.venv`, binaries, and generated evidence; absent a project setup hook that provisions them, use distinct-target direct lanes instead of worktrees. Never inherit a relative session directory: cleanup removes it with the worktree.
- No lane commits, pushes, edits another target, or shares publicly. A lane may regenerate target-local disposable analysis/index evidence required by its pipeline; generated `out/`, `build/`, compile DB, Rizin analysis/index, and source stubs are never merge artifacts or shared acceptance state.
- Parent receives each worktree handoff, rejects overlapping/unexpected paths, and integrates lanes **one at a time**. Before each integration, refresh against the current parent tree/index plus already integrated lanes, not only `HEAD`; resolve target ownership facts deliberately, never by blind patch application.
- Integration order is deterministic queue order, not completion order. Exact lane: re-run live gates in the parent worktree, confirm cleanup naming + relocation audit, fresh review, then commit/push if authorized. Non-exact lanes follow the normal review/journal/restore-or-retain policy.
- Refresh edited target snapshots serially after integration, then rebuild the global index once per wave/checkpoint; never select the next wave from stale evidence.
- A lane failure consumes only that selector; journal and continue other lanes. Conflicting handoffs, dirty parent state, or failed freshness recovery stop new dispatch.

`parallelism=1` remains the default; no worktree fan-out.

## Post-loop audit

Audit retained lanes for missed integration only; do not repeat passed cleanup. New exact edits require live `asm-diff`/`byte-match` and review; partial edits require unchanged-or-better live score and metadata. Revert regression.

## Partial re-lift + decomp.me final rung

User-authorized fresh `out/non-exact-lifts.json` pass after queue + checkpoint. Process `partial` rows serially, preserving each source's pre-mission state. Skip `contains_data` rows (range embeds reviewed `D_*` data; unliftable until Splat segment splits — route to user, never dispatch). Remaining rows: same ≤6-attempt executor↔review loop, then parent evidence check.

Exhausted non-exact: integrate durable lesson, restore recorded prior state, run final rung `bin/scratchpad share SELECTOR`. Publish only if selector starts at a reviewed Splat `c`/`asm` `func_XXXXXXXX` boundary and restored partial source exists. Missing ABI/call ownership/analyzer confidence/lifting evidence does **not** block a valid function. Data-leading/non-function/unreviewed/source-less → `not shareable: <reason>`. Journal URL/reason/failure; continue. Never alter reviewed map/Splat facts for a payload; a scratch is public escalation evidence, never acceptance; a prior URL never replaces a current mission or exact gate.

## Stop/report

Never stop for: non-exact candidate, unshareable, publish failure, review rejection of an exact claim, bounded escalation. Journal + continue. Stop only: queue exhausted, budget reached, evidence-recovery fatal, child output conflicts with owned worktree, user approval required. Print journal, counts, commits, scratch URLs/results, risks, next step.

Role protocols are preloaded by `agent-context.py`; the reference workflow supplies compact child tasks and ownership limits.
