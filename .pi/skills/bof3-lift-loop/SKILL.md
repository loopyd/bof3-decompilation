---
name: bof3-lift-loop
description: Autonomously lift a serial BOF3 batch to reviewed exact byte matches. Selects candidates, dispatches bounded executor/reviewer agents, and commits only user-authorized reviewed exact lifts. Not for one hand-guided lift; use `/skill:bof3-re`.
---

# BOF3 lift loop

Read `AGENTS.md`, load `/skill:bof3-re`. Parent owns selection, checkpoints, git, commits. `bof3-reverse` writes one function; `bof3-review` independently reviews it, may record durable cross-function findings in `docs/specs/`/`docs/agents/lessons.md`. Agents own model/tool policy. Never run two functions in one target concurrently: shared `internal.h`.

## Confirm

Targets (default reviewed), selection (`quick-wins` default), budget (3–5), exclusions, branch, parallelism (`1` default), explicit commit authorization, explicit decomp.me publication authorization (partials in scope). Parallelism is the maximum number of function lanes, not agents per function.

## Baseline + queue

```sh
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots
# stale generated evidence:
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots --recover
```

Never dispatch on dirty tree/index. `loop-status` is inspection-only, fails closed (`dispatch_allowed: false`) on stale snapshots/index or changes; never dispatch while false. `--recover` repairs only stale generated evidence serially, then rebuilds index once. Queue from one fresh snapshot/index. After map/Splat edits: continue bounded queue, refresh edited target snapshots, rebuild index once at checkpoint. Never query a stale index. Init `out/lift-loop/results.tsv`: `function status commit notes` (notes may carry evidence paths/SHA-256; aids checkpoints, never replaces live acceptance).

## Function pipeline

Each function is single-threaded regardless of batch parallelism:

```text
bof3-reverse -> bof3-review -> [exact 100%] bof3-cleanup -> live gates -> bof3-review -> integration
```

- One executor and one reviewer at a time per function. `needs-fix` returns to the same executor before re-review.
- Run cleanup only after a live 100% instruction/byte match and review pass. Cleanup covers evidence-backed semantic rename, relocation/binding normalization, metadata, and owned-file audit; it must not broaden ownership.
- Cleanup succeeds only when fresh `asm-diff`, `byte-match`, `symbols check`, Splat validation when touched, naming/relocation audit, and fresh review all pass. Otherwise revert cleanup only; retain the reviewed exact pre-cleanup lift.

## Serial loop (default `parallelism=1`)

Per candidate until queue exhausted or fatal loop failure:

1. `function-brief.py SELECTOR` (`TARGET@0xADDRESS` or shipped EMI `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`); pass same selector to `bof3-reverse`.
2. Relevant declared companion call: pass `companion-check` before dispatch; a static record proves no ABI/ownership.
3. Executor returns mission JSON + checked acceptance report: exact lift, or **review-pending escalation** keeping its best coherent clean-C candidate in owned files; must identify pre-mission baseline, best live diff, first mismatch class, attempted rungs, changed files.
4. Parent runs one live `byte-match` only for an exact claim. Status cache is never acceptance.
5. Dispatch `bof3-review` for **both exact and non-exact** results with brief, best live asm diff, changed-file diff, executor rung report. Non-exact: reviewer classifies residual, flags any skipped/misapplied lever, requires focused target-qualified Rizin context when types, symbol roles, layouts, caller contracts, branch targets, lifetimes remain unknown, records a genuinely reusable cross-function lesson before discard. `needs-fix`: retry executor ≤2 times from reviewed best candidate, then re-match/review. `block`: journal, continue.
6. Exact + review pass: dispatch one `bof3-cleanup` for rename, relocation/binding, metadata, and owned-file audit. If cleanup changes files, run fresh live gates and fresh `bof3-review`; revert cleanup only on failure. If cleanup finds nothing, record the successful audit.
7. Only exact + cleanup/audit pass + final review pass: stage owned source/header/map/Splat facts, verify staged list, commit `feat(decomp): byte-match <function>`, journal.
8. Non-exact never stops the queue. Retain a reviewed coherent net improvement with atomic `@status partial`, `@match NN.NN`, `@residual ...`; commit when authorized. Restore only no-progress or semantic/type rejection. Never restore before review: candidate diff is primary diagnostic evidence.

Use `subagent_supervisor` replies for child requests, not generic intercom.

## Parallel loop (optional `parallelism>1`)

Freeze one fresh queue, then run up to `parallelism` independent function pipelines concurrently with `runs.all`, one managed worktree per lane. Requirements:

- Never place two live lanes from the same target together: target-local map, Splat, manifest, support source, and `internal.h` are shared. Partition each wave by distinct `TARGET`; defer collisions.
- Every lane owns exactly one selector and runs its executor -> reviewer -> exact-only cleanup -> gates -> re-review serially. Cleanup/audit for one function may overlap another target's lane.
- No lane commits, pushes, edits another target, or shares publicly. A lane may regenerate target-local disposable analysis/index evidence required by its function pipeline; generated `out/`, `build/`, compile DB, Rizin analysis/index, and source stubs are never merge artifacts or shared acceptance state.
- Parent receives each worktree handoff, rejects overlapping or unexpected paths, and integrates completed lanes **one at a time**. Before each integration, refresh against the current parent tree/index plus already integrated lanes, not only `HEAD`; resolve target ownership facts deliberately, never by blind patch application.
- Integration order is deterministic queue order, not completion order. For each exact lane: re-run live gates in the parent worktree, run/confirm cleanup naming + relocation audit, fresh review, then commit/push if authorized. Non-exact lanes are reviewed/journaled and restored or retained by the normal policy.
- Refresh every edited target snapshot serially after integration, then rebuild the global index once per wave/checkpoint. Do not select the next wave from stale evidence.
- A lane failure consumes only that selector; journal it and continue other lanes. Conflicting handoffs, dirty parent state, or failed freshness recovery stop new dispatch.

`parallelism=1` remains the default and uses no worktree fan-out.

## Post-loop audit

At batch end, audit all integrated exact functions for missed evidence-backed rename/relocation cleanup. Do not repeat cleanup already passed in the function pipeline. Any new edit requires fresh live `asm-diff`, `byte-match`, naming/relocation checks, and fresh `bof3-review`; revert a cleanup regression, never fix it forward.

## Partial re-lift + decomp.me final rung

User-authorized fresh `out/non-exact-lifts.json` pass after queue + checkpoint. Process every `partial` row serially; preserve each source's pre-mission state. Skip rows in `contains_data` (range embeds reviewed `D_*` data; unliftable until Splat segment splits — route to user, never dispatch). Remaining rows: fresh executor mission, parent evidence check, independent review for exact and non-exact results.

Non-exact: executor leaves best coherent candidate review-pending; parent dispatches `bof3-review`, integrates durable cross-function lesson, restores recorded prior state, runs final rung `bin/scratchpad share SELECTOR`. Publish only if selector begins at a reviewed Splat `c`/`asm` `func_XXXXXXXX` boundary and restored partial source exists. Missing ABI, call ownership, analyzer confidence, or other lifting evidence does **not** make an otherwise valid function unshareable. Data-leading/non-function/unreviewed/source-less → `not shareable: <reason>`. Journal URL, `not shareable` reason, or publication failure; continue. Never alter reviewed map/Splat facts for a payload; a scratch is public escalation evidence, never acceptance; a prior scratch URL never replaces a current mission or exact gate.

## Cleanup child brief

```
Task: audit/clean one reviewed exact SELECTOR only.
Scope: evidence-backed semantic rename, relocation/binding normalization, metadata, and owned-file consistency. No behavior invention or ownership broadening.
Required: pre/post live asm-diff + byte-match; symbols/Splat and naming/relocation audits when applicable. Exactness must remain 100%.
No git writes, setup, sharing, other targets, or children. Return changed paths, audit evidence, and rollback instruction.
```

## Stop/report

Never stop merely for: non-exact candidate, unshareable, decomp.me publish failure, review rejection of an exact claim, bounded executor escalation. Journal + continue. Stop only: queue exhausted, budget reached, generated-evidence recovery fatal, child output conflicts with owned worktree, user approval required. Print journal, counts, commits, every scratch URL/result, risks, next step.

## Child brief

```
Task: lift/review SELECTOR (`TARGET@0xADDRESS`, or shipped EMI `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`).
Context: function brief + mission/diff + owned-file diff. First run `agent-context.py <reverse|review> SELECTOR`.
Authority: executor edits only owned source/internal.h/map/Splat; reviewer may also edit only `docs/specs/**/*.md` or `docs/agents/lessons.md` for durable cross-function findings.
No git writes, setup, other targets, or children. Non-exact executor leaves best coherent candidate review-pending; only parent restores after review + durable-lesson integration.
Return protocol/checklist JSON + required acceptance-report.
```

Role protocols preloaded from `.pi/skills/bof3-re/references/REVERSE/` and `.pi/skills/bof3-re/references/REVIEW/` by `agent-context.py`.
