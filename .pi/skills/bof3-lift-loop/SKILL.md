---
name: bof3-lift-loop
description: Autonomously lift a serial BOF3 batch to reviewed exact byte matches. Selects candidates, dispatches bounded executor/reviewer agents, and commits only user-authorized reviewed exact lifts. Not for one hand-guided lift; use `/skill:bof3-re`.
---

# BOF3 lift loop

Read `AGENTS.md`, load `/skill:bof3-re`. Parent owns selection, checkpoints, git, commits. `bof3-reverse` writes one function; `bof3-review` independently reviews it, may record durable cross-function findings in `docs/specs/`/`docs/agents/lessons.md`. Agents own model/tool policy. Never run two functions in one target concurrently (shared `internal.h`).

## Confirm

Targets (default reviewed), selection (`quick-wins`), budget (3–5), exclusions, branch, parallelism (`1`), explicit commit authorization, explicit decomp.me publication authorization (partials in scope). Parallelism = max lanes, not agents per function.

## Baseline + queue

```sh
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots
# stale evidence:
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots --recover
```

Never dispatch on dirty tree/index. `loop-status` fails closed (`dispatch_allowed: false`) on stale snapshots/index or changes; `--recover` repairs only stale generated evidence serially, then rebuilds index once; queue from one fresh snapshot/index. After map/Splat edits: continue bounded queue, refresh edited target snapshots, rebuild index once at checkpoint. Never query a stale index. Init `out/lift-loop/results.tsv`: `function status commit notes` (evidence paths/SHA-256 allowed; checkpoint aids, never live acceptance).

## Function pipeline

Each function is single-threaded regardless of batch parallelism:

```text
bof3-reverse -> bof3-review -> [exact 100%] bof3-cleanup -> live gates -> bof3-review -> integration
```

- One executor/reviewer pair per function; ≤6 executor attempts incl. first. Non-exact review returns 1–3 ranked untried experiments (lever, expected effect, evidence, accept/revert); resume same executor, one variant at a time, preserve best coherent result, re-review. Stop early: exact; rejected semantics/types; approval/safety or external blocker; reviewer `pass` with attested ladder exhaustion. Experiment-free `needs-fix` invalid.
- Partial→exact final review identifies the decisive experiment; parent records a reusable rule in the narrowest playbook/lesson before integration; function-only → `lesson: none` + evidence.
- Cleanup only after live 100% instruction/byte match + review pass: evidence-backed semantic rename, relocation/binding normalization, metadata, owned-file audit; never broaden ownership. Cleanup passes only with fresh `asm-diff`, `byte-match`, `symbols check`, Splat validation when touched, naming/relocation audit, fresh review; failure reverts cleanup only, retaining the reviewed exact pre-cleanup lift.

## Serial loop (default `parallelism=1`)

Per candidate until queue exhausted or fatal failure:

1. `function-brief.py SELECTOR` (`TARGET@0xADDRESS` or shipped EMI `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`); pass same selector to `bof3-reverse`.
2. Relevant declared companion call: pass `companion-check` before dispatch; a static record proves no ABI/ownership.
3. Executor returns mission JSON + checked acceptance report: exact lift, or **review-pending escalation** keeping its best coherent clean-C candidate in owned files; identifies baseline, best live diff, first mismatch class, rungs, changed files.
4. Parent runs one live `byte-match` only for an exact claim. Status cache is never acceptance.
5. Review exact and non-exact with brief, best live diff, owned diff, rung ledger, prior handoffs. Non-exact: ≤6-attempt contract; unknown types/symbols/layout/ABI/CFG/lifetimes → focused target-qualified Rizin first; no unchanged retry. `block` only for rejected semantics/types, invalid ownership/boundary, approval/safety, or external tool failure — not ordinary non-exactness. Exhausted coherent partial → reviewer `pass`, empty experiments, attestation.
6. Partial→exact final review identifies the decisive experiment; parent records any reusable rule. Then exact-only cleanup/audit per the pipeline bullet.
7. Only exact + cleanup/audit pass + final review pass: stage owned source/header/map/Splat facts, verify staged list, commit `feat(decomp): byte-match <function>`, journal.
8. Non-exact never stops the queue. Retain a reviewed coherent net improvement with atomic `@status partial`/`@match NN.NN`/`@residual ...`; commit when authorized. Restore only no-progress or semantic/type rejection; never restore before review — candidate diff is primary diagnostic evidence.

Use `subagent_supervisor` replies for child requests, not generic intercom.

## Parallel loop (optional `parallelism>1`)

Freeze one fresh queue, then run up to `parallelism` independent function pipelines concurrently with `runs.all`, one managed worktree per lane. Requirements:

- Never place two live lanes from the same target together: target-local map, Splat, manifest, support source, `internal.h` are shared. Partition each wave by distinct `TARGET`; defer collisions.
- Every lane owns exactly one selector and runs its bounded executor ↔ reviewer retry loop -> exact-only cleanup -> gates -> re-review serially. Cleanup/audit may overlap another target's lane.
- Managed-worktree launches pass an absolute project-owned `sessionDir` (for example `$PWD/.pi-subagents/sessions/<batch>`). Never inherit the project-relative session directory: children resolve it inside temporary worktrees, and cleanup then removes `session.jsonl` before handoff.
- No lane commits, pushes, edits another target, or shares publicly. A lane may regenerate target-local disposable analysis/index evidence required by its pipeline; generated `out/`, `build/`, compile DB, Rizin analysis/index, and source stubs are never merge artifacts or shared acceptance state.
- Parent receives each worktree handoff, rejects overlapping/unexpected paths, and integrates lanes **one at a time**. Before each integration, refresh against the current parent tree/index plus already integrated lanes, not only `HEAD`; resolve target ownership facts deliberately, never by blind patch application.
- Integration order is deterministic queue order, not completion order. Exact lane: re-run live gates in the parent worktree, confirm cleanup naming + relocation audit, fresh review, then commit/push if authorized. Non-exact lanes follow the normal review/journal/restore-or-retain policy.
- Refresh edited target snapshots serially after integration, then rebuild the global index once per wave/checkpoint; never select the next wave from stale evidence.
- A lane failure consumes only that selector; journal and continue other lanes. Conflicting handoffs, dirty parent state, or failed freshness recovery stop new dispatch.

`parallelism=1` remains the default; no worktree fan-out.

## Post-loop audit

At batch end, audit all integrated exact functions for missed evidence-backed rename/relocation cleanup. Do not repeat cleanup already passed in the function pipeline. Any new edit requires fresh live `asm-diff`, `byte-match`, naming/relocation checks, and fresh `bof3-review`; revert a cleanup regression, never fix forward.

## Partial re-lift + decomp.me final rung

User-authorized fresh `out/non-exact-lifts.json` pass after queue + checkpoint. Process `partial` rows serially, preserving each source's pre-mission state. Skip `contains_data` rows (range embeds reviewed `D_*` data; unliftable until Splat segment splits — route to user, never dispatch). Remaining rows: same ≤6-attempt executor↔review loop, then parent evidence check.

Exhausted non-exact: integrate durable lesson, restore recorded prior state, run final rung `bin/scratchpad share SELECTOR`. Publish only if selector starts at a reviewed Splat `c`/`asm` `func_XXXXXXXX` boundary and restored partial source exists. Missing ABI/call ownership/analyzer confidence/lifting evidence does **not** block a valid function. Data-leading/non-function/unreviewed/source-less → `not shareable: <reason>`. Journal URL/reason/failure; continue. Never alter reviewed map/Splat facts for a payload; a scratch is public escalation evidence, never acceptance; a prior URL never replaces a current mission or exact gate.

## Cleanup child brief

```
Task: audit/clean one reviewed exact SELECTOR only.
Scope: evidence-backed semantic rename, relocation/binding normalization, metadata, and owned-file consistency. No behavior invention or ownership broadening.
Required: pre/post live asm-diff + byte-match; symbols/Splat and naming/relocation audits when applicable. Exactness must remain 100%.
No git writes, setup, sharing, other targets, or children. Return changed paths, audit evidence, rollback instruction.
```

## Stop/report

Never stop for: non-exact candidate, unshareable, publish failure, review rejection of an exact claim, bounded escalation. Journal + continue. Stop only: queue exhausted, budget reached, evidence-recovery fatal, child output conflicts with owned worktree, user approval required. Print journal, counts, commits, scratch URLs/results, risks, next step.

## Child brief

```
Task: lift/review SELECTOR (`TARGET@0xADDRESS`, or shipped EMI `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`).
Context: function brief + mission/diff + owned-file diff. First run `agent-context.py <reverse|review> SELECTOR`.
Authority: executor edits owned source/internal.h/map/Splat. Reviewer may edit only `docs/specs/**/*.md` or `docs/agents/lessons.md`; parent owns matching-playbook edits.
No git writes, setup, other targets, or children. Non-exact executor leaves best coherent candidate review-pending; only parent restores after review + durable-lesson integration.
Return protocol/checklist JSON + required acceptance-report.
```

Role protocols preloaded from `.pi/skills/bof3-re/references/REVERSE/` and `.pi/skills/bof3-re/references/REVIEW/` by `agent-context.py`.
