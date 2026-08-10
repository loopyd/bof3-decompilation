---
name: bof3-lift-loop
description: Autonomously lift a serial BOF3 batch to reviewed exact byte matches. Selects candidates, dispatches bounded executor/reviewer agents, and commits only user-authorized reviewed exact lifts. Not for one hand-guided lift; use `/skill:bof3-re`.
---

# BOF3 lift loop

Read `AGENTS.md`, load `/skill:bof3-re`. Parent owns selection, checkpoints, git, commits. `bof3-reverse` writes one function; `bof3-review` independently reviews it, may record durable cross-function findings in `docs/specs/`/`docs/agents/lessons.md`. Agents own model/tool policy. Never run two functions in one target concurrently: shared `internal.h`.

## Confirm

Targets (default reviewed), selection (`quick-wins` default), budget (3–5), exclusions, branch, explicit commit authorization, explicit decomp.me publication authorization (partials in scope).

## Baseline + queue

```sh
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots
# stale generated evidence:
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots --recover
```

Never dispatch on dirty tree/index. `loop-status` is inspection-only, fails closed (`dispatch_allowed: false`) on stale snapshots/index or changes; never dispatch while false. `--recover` repairs only stale generated evidence serially, then rebuilds index once. Queue from one fresh snapshot/index. After map/Splat edits: continue bounded queue, refresh edited target snapshots, rebuild index once at checkpoint. Never query a stale index. Init `out/lift-loop/results.tsv`: `function status commit notes` (notes may carry evidence paths/SHA-256; aids checkpoints, never replaces live acceptance).

## Serial loop

Per candidate until queue exhausted or fatal loop failure:

1. `function-brief.py SELECTOR` (`TARGET@0xADDRESS` or shipped EMI `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`); pass same selector to `bof3-reverse`.
2. Relevant declared companion call: pass `companion-check` before dispatch; a static record proves no ABI/ownership.
3. Executor returns mission JSON + checked acceptance report: exact lift, or **review-pending escalation** keeping its best coherent clean-C candidate in owned files; must identify pre-mission baseline, best live diff, first mismatch class, attempted rungs, changed files.
4. Parent runs one live `byte-match` only for an exact claim. Status cache is never acceptance.
5. Dispatch `bof3-review` for **both exact and non-exact** results with brief, best live asm diff, changed-file diff, executor rung report. Non-exact: reviewer classifies residual, flags any skipped/misapplied lever, requires focused target-qualified Rizin context when types, symbol roles, layouts, caller contracts, branch targets, lifetimes remain unknown, records a genuinely reusable cross-function lesson before discard. `needs-fix`: retry executor ≤2 times from reviewed best candidate, then re-match/review. `block`: journal, continue. No mid-queue cleanup.
6. Only exact + review pass: stage owned source/header/map/Splat facts, verify staged list, commit `feat(decomp): byte-match <function>`, journal.
7. Non-exact never stops the queue. Retain a reviewed coherent net improvement with atomic `@status partial`, `@match NN.NN`, `@residual ...`; commit when authorized. Restore only no-progress or semantic/type rejection. Never restore before review: candidate diff is primary diagnostic evidence.

Use `subagent_supervisor` replies for child requests, not generic intercom.

## Post-loop cleanup

Once at batch end (queue complete or budget reached, reviewed exact lifts): run one `bof3-cleanup` pass over the batch's exact functions — cosmetic, evidence-preserving changes only (naming, comment metadata, organization within owned files). After any cleanup edit: re-run fresh live `byte-match` and dispatch a fresh `bof3-review`; both must pass before the function stays eligible. Lift-body edits also first need live `bin/asm-diff TARGET@0xADDRESS --detail normal` with no first-difference. Cleanup breaking byte-match: reverted, never fixed forward.

## Partial re-lift + decomp.me final rung

User-authorized fresh `out/non-exact-lifts.json` pass after queue + checkpoint. Process every `partial` row serially; preserve each source's pre-mission state. Skip rows in `contains_data` (range embeds reviewed `D_*` data; unliftable until Splat segment splits — route to user, never dispatch). Remaining rows: fresh executor mission, parent evidence check, independent review for exact and non-exact results.

Non-exact: executor leaves best coherent candidate review-pending; parent dispatches `bof3-review`, integrates durable cross-function lesson, restores recorded prior state, runs final rung `bin/scratchpad share SELECTOR`. Publish only if selector begins at a reviewed Splat `c`/`asm` `func_XXXXXXXX` boundary and restored partial source exists. Missing ABI, call ownership, analyzer confidence, or other lifting evidence does **not** make an otherwise valid function unshareable. Data-leading/non-function/unreviewed/source-less → `not shareable: <reason>`. Journal URL, `not shareable` reason, or publication failure; continue. Never alter reviewed map/Splat facts for a payload; a scratch is public escalation evidence, never acceptance; a prior scratch URL never replaces a current mission or exact gate.

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
