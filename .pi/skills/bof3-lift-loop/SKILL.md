---
name: bof3-lift-loop
description: Autonomously lift a serial BOF3 batch to reviewed exact byte matches. Selects candidates, dispatches bounded executor/reviewer agents, and commits only user-authorized reviewed exact lifts. Not for one hand-guided lift; use `/skill:bof3-re`.
---

# BOF3 lift loop

Read `AGENTS.md` and load `/skill:bof3-re`. Parent owns selection, checkpoints,
git, and commits; `bof3-reverse` writes one function and `bof3-review`
independently reviews it and may record durable cross-function findings in
`docs/specs/` or `docs/agents/lessons.md`. Agents own their model/tool policy. Never run two functions in one
target concurrently: they share `internal.h`.

## Confirm

Before starting, get targets (default reviewed), selection (`quick-wins` default),
budget (start 3–5), scope exclusions, branch, explicit commit authorization, and
explicit decomp.me publication authorization when partials are in scope.
Subagents never commit/push/reset/clean/setup; never commit `inputs/` or secrets.

## Baseline and queue

```sh
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots
# If it reports stale generated evidence:
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots --recover
```

Do not dispatch on dirty tree/index failure. `loop-status` is inspection-only by
default: it fails closed without ranking when snapshots or the index are stale.
Use `--recover` to repair only generated stale evidence serially, then rebuild
the index once after fresh snapshot rechecks. It replaces slow `decomp-status`
baselines: use per-function live byte-match for truth; run decomp-status only on
explicit progress-report request. Queue candidates from one fresh snapshot/index;
after map/Splat edits, continue the bounded queue, then refresh edited target
snapshots and rebuild index once at a checkpoint before requesting another queue.
Never query a stale index. Initialize `out/lift-loop/results.tsv`: `function status commit notes`.
Keep the four columns: parent records bounded relative brief/companion evidence
paths and/or SHA-256 references in `notes`; they aid checkpoints and never
replace live acceptance evidence.

## Serial loop

For each candidate until the requested queue is exhausted or a fatal loop failure:

1. Get one `function-brief.py SELECTOR`, using `TARGET@0xADDRESS` or a shipped
   EMI selector `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`; pass the same selector
   to `bof3-reverse`.
2. If a relevant declared companion call exists, require passing `companion-check`
   before dispatch; a static record alone proves no ABI/ownership.
3. Executor returns mission JSON + its checked acceptance report. It may return an
   exact lift or fully restored escalation; do not override that policy.
4. Parent runs one live `byte-match` only for an exact claim. Do not use status
   cache as acceptance.
5. For an exact claim, dispatch `bof3-review` with brief, diff, and changed-file
   diff. On `needs-fix`, retry executor ≤2 times, then re-match/review; on `block`,
   journal and continue with the next candidate.
6. Only exact + review pass: stage owned source/header/map/Splat facts, verify the
   staged file list, commit `feat(decomp): byte-match <function>`, journal it.
7. A non-exact result never stops the queue: restore the candidate state, use the
   decomp.me final rung below, journal its outcome, then start the next selector
   with a fresh mission context.

Use `subagent_supervisor` replies for child requests, not generic intercom.

## Partial re-lift and decomp.me final rung

After the normal bounded queue and its checkpoint, a user may authorize a
fresh `out/non-exact-lifts.json` pass. Process every current `partial` row
serially, preserving each target-qualified source's pre-mission state. Each
row gets a fresh executor mission, parent evidence check, and independent
review only if it exact-matches.

For every non-exact result, the executor restores its prior state. The parent
then performs the final execution rung: `bin/scratchpad share SELECTOR`.
Publish only if the selector begins at a reviewed Splat `c` or `asm`
`func_XXXXXXXX` boundary and its restored partial source exists. Missing ABI,
call ownership, analyzer confidence, or other lifting evidence does **not** make
an otherwise valid function unshareable. A data-leading, non-function,
unreviewed, or source-less selector is `not shareable: <reason>`. Record the
URL, `not shareable` reason, or publication failure in the journal, then
continue with the next partial. Never alter reviewed map/Splat facts just to
make a scratch payload; a scratch is public escalation evidence, never
acceptance. A prior scratch URL never replaces a current mission or exact gate.

## Stop/report

Do not stop merely because a candidate is non-exact, unshareable, a decomp.me
publish fails, review rejects an exact claim, or a bounded executor mission
escalates. Journal that outcome and continue. Stop only when the requested
queue is exhausted, the user budget is reached, generated-evidence recovery is
fatal, child output conflicts with the owned worktree, or user approval is
required. Print journal, counts, commits, every scratch URL/result, risks, and
next step.

## Child brief

```
Task: lift/review SELECTOR (`TARGET@0xADDRESS`, or shipped EMI
`BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`).
Context: function brief + mission/diff + owned-file diff. First run
`agent-context.py <reverse|review> SELECTOR`.
Authority: executor may edit only owned source/internal.h/map/Splat; reviewer may additionally edit only `docs/specs/**/*.md` or `docs/agents/lessons.md` for durable cross-function findings, never transient selector-specific evidence.
No git writes, setup, other targets, or children; escalation may delete only its
new mission source to restore the tree.
Return protocol/checklist JSON and required acceptance-report.
```

Role protocols are preloaded from `.pi/skills/bof3-re/references/REVERSE/` and
`.pi/skills/bof3-re/references/REVIEW/` by `agent-context.py`.
