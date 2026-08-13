---
name: bof3-lift-loop
description: Autonomously lift a serial BOF3 batch to reviewed exact byte matches. Selects candidates, dispatches bounded executor/reviewer agents, and commits only user-authorized reviewed exact lifts. Not for one hand-guided lift; use `/skill:bof3-re`.
---

# BOF3 lift loop

Read `AGENTS.md`, load `/skill:bof3-re`. Parent owns selection/checkpoints/git. Reverse writes one function; review is independent and may record durable findings. Never run same-target functions concurrently (`internal.h`).

## Confirm

Confirm targets (default reviewed), selection (`quick-wins`), budget (3–5), exclusions, branch, parallelism (`1`), commit authorization, and decomp.me publication authorization. Parallelism counts lanes; preserve distinct-target ownership and single-thread each function.

## Baseline + queue

```sh
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots
# stale evidence:
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots --recover
```

Never dispatch dirty/stale. `loop-status` fails closed; `--recover` serially repairs generated evidence and rebuilds index. After map/Splat edits refresh affected snapshots, then index once. Init `out/lift-loop/results.tsv`: `function status commit notes` (evidence paths/SHA-256 allowed; checkpoint aids, never live acceptance).

## Function pipeline

Render/verify [`references/workflow-script.md`](references/workflow-script.md); submit unchanged as `subagent.workflowScript` with mission and lane launch. Script owns state, iterations, cleanup, review, integration; parent owns queue, launch, closure, freshness.

Each function is single-threaded regardless of batch parallelism:

```text
bof3-reverse -> bof3-review -> [retained exact|partial] bof3-cleanup -> live gates -> bof3-review -> integration
```

- One function/lane. Rendered workflow owns loop, 20-attempt ceiling, ledger, baseline checkpoint, restore, stops; do not duplicate.
- An attempt is a substantive investigation pass: reviewer queues at least three distinct evidence-backed experiments; executor runs the complete queue plus related safe variants before returning; reviewer independently reloads live evidence. Fewer than three distinct experiments blocks the next retry instead of burning an iteration. Never checkpoint/restore between attempts—the shared worktree carries discoveries forward. Three consecutive non-improving queues automatically advance one rung: `clean-c` → `static-allocation` → `compiler-profile` (`bin/flag-search`) → one bounded `permuter` coordinator → `compiler-ceiling`. An improvement resets the stall count but does not move backward to an exhausted rung. Rung changes are ledger entries; reviewers must queue experiments for the active rung, not resume broad spelling churn. At the end, restore the baseline only when the final live score did not improve.
- Record a reusable decisive lever in the narrowest playbook/lesson before integration; function-only → `lesson: none`.
- Cleanup every retained exact/partial: evidence-backed naming and target-local integration only. Exact requires fresh diff/bytes/symbols/Splat/relocation + review. Partial is spelling-only; preserve body/ABI/boundary/compiler and atomic status/match/residual, score ≥ best + audits/review. Failure restores pre-cleanup.

## Serial loop (`parallelism=1`)

Create lane, render/verify, launch exact script with returned launch. Before dispatch run `function-brief.py SELECTOR` and relevant companion check. Accept only exported manifest/patch + retained state + cleanup/review. Restore no-progress, rejected semantics/types, cleanup regression.

## Parallel loop (optional `parallelism>1`)

Freeze one queue; launch target-distinct rendered workflows as independent async calls, each parent-managed with `worktree:false` and unique absolute `sessionDir`. Never outer `runs.all` (aliases `run-0/session.jsonl`). Requirements:

- Never place two live lanes from the same target together. Partition by distinct `TARGET`; defer collisions.
- Create with `python3 .pi/skills/bof3-lift-loop/scripts/lane-worktree.py create --key WAVE-LANE --selector SELECTOR`; it bootstraps executable modes, tools, `.venv`/`inputs`, compilers, disposable `out/`, and returns an absolute `session_dir` under the parent repository.
- Render/verify, then call `subagent({workflowScript:SCRIPT, mission:{title:"Byte-match SELECTOR", objective:"Reach verified 100% within 20 attempts"}, ...CREATE_RESULT.launch})`. Use the returned launch object unchanged; it pins absolute `cwd`/`sessionDir`, `worktree:false`, and `async:true`. Never omit or reconstruct launch fields. Launch calls back-to-back; nested phases serialize.
- Do not pass workflow-level `acceptance.level:"reviewed"`: reviewed is achieved, not requestable. Writer agents use their configured acceptance; independent review is an explicit subsequent `bof3-review` run. If host-level writer review is needed, use `acceptance:{review:{required:true,...}}`, never `level:"reviewed"`; omit acceptance for read-only reviewer calls.
- No lane commits, pushes, stages, edits another target, or shares publicly. Generated `out/`, `build/`, compile DB, analyzer/index state, source stubs, and `.pi-subagents/` are never merge artifacts.
- After retained exact/improvement, the workflow runs `bof3-cleanup` and consolidation review. Accept explicit `pass` for exact and `pass|retain-improved-partial|retain-as-improved-partial` for a partial; warnings do not block a reviewed partial unless verdict is `block|needs-fix`. Then a host gate calls `lane-worktree.py integrate`: require clean parent at unchanged lane base, export/digest/check/apply only non-ignored Git changes, `git diff --check`, commit the reviewed transaction, and remove the lane. Generated ignored `build/`/`.pi-subagents/` state is expected in lanes and excluded rather than treated as a handoff failure. Any failure rolls parent changes back and preserves the lane for inspection. The workflow returns `integrated` only after commit + cleanup; otherwise close the mission `failed` with lane status/ledger summary. Reject failed capture, unexpected paths, `src/emi/`, absolute-path stubs, unapproved `INCLUDE_ASM`, cleanup/review rejection, or score regression.
- Parallel completions serialize through the clean-parent/unchanged-HEAD gate: after one integration advances `main`, siblings fail closed and must rerun from the new base. No automatic three-way merge.
- Refresh edited target snapshots serially after consolidation, then rebuild the global index once. A lane failure consumes only its selector; dirty parent, overlap, stale/conflicting handoff, failed rollback, or failed freshness recovery stops consolidation/new dispatch.

`parallelism=1` remains the default and still uses one managed worktree; higher values fan out isolated lane worktrees.

## Post-loop audit

Audit retained lanes for missed integration only; do not repeat passed cleanup. New exact edits require live `asm-diff`/`byte-match` and review; partial edits require unchanged-or-better live score and metadata. Revert regression.

## Partial re-lift + decomp.me final rung

User-authorized fresh `out/non-exact-lifts.json` pass after queue + checkpoint. Process `partial` rows serially, preserving each source's pre-mission state. Skip `contains_data` rows (range embeds reviewed `D_*` data; unliftable until Splat segment splits — route to user, never dispatch). Remaining rows: same rendered executor↔review loop, then parent evidence check.

Exhausted non-exact: lesson, restore prior state, then `bin/scratchpad share SELECTOR`. Publish only reviewed Splat `c`/`asm` function start with restored source. Data/non-function/unreviewed/source-less → journal not-shareable reason. Scratch is escalation evidence, never acceptance or map/Splat authority.

## Stop/report

Never stop for: non-exact candidate, unshareable, publish failure, review rejection of an exact claim, bounded escalation. Journal + continue. Stop only: queue exhausted, budget reached, evidence-recovery fatal, child output conflicts with owned worktree, user approval required. Print journal, counts, commits, scratch URLs/results, risks, next step.

Role protocols are preloaded by `agent-context.py`; the reference workflow supplies compact child tasks and ownership limits.
