---
name: bof3-lift-loop
description: Autonomously lift a serial BOF3 batch to reviewed exact byte matches. Selects candidates, dispatches bounded executor/reviewer agents, and commits only user-authorized reviewed exact lifts. Not for one hand-guided lift; use `/skill:bof3-re`.
---

# BOF3 lift loop

Read `AGENTS.md` and load `/skill:bof3-re`. Parent owns selection, checkpoints,
git, commits; `bof3-reverse` writes one function, `bof3-review` independently
reviews it and may record durable cross-function findings in `docs/specs/` or
`docs/agents/lessons.md`. Agents own their model/tool policy. Never run two
functions in one target concurrently: they share `internal.h`.

## Confirm

Get targets (default reviewed), selection (`quick-wins` default), budget
(start 3–5), scope exclusions, branch, explicit commit authorization, explicit
decomp.me publication authorization when partials are in scope.

## Baseline and queue

```sh
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots
# If it reports stale generated evidence:
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots --recover
```

Never dispatch on dirty tree/index failure. `loop-status` is inspection-only:
it fails closed (`dispatch_allowed: false`) when snapshots or the index are
stale or changes exist; never dispatch while false. `--recover` repairs only
generated stale evidence serially, then rebuilds the index once after fresh
snapshot rechecks. It replaces slow `decomp-status` baselines: truth is
per-function live byte-match; decomp-status only on explicit progress-report
request. Queue from one fresh snapshot/index; after map/Splat
edits, continue the bounded queue, then refresh edited target snapshots and
rebuild the index once at a checkpoint before requesting another queue. Never
query a stale index. Initialize `out/lift-loop/results.tsv`: `function status
commit notes` (four columns; `notes` may carry evidence
paths/SHA-256 refs — aids checkpoints, never replaces live acceptance).

## Serial loop

Per candidate, until the queue is exhausted or a fatal loop failure:

1. Get one `function-brief.py SELECTOR` (`TARGET@0xADDRESS` or shipped EMI
   `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`); pass the same selector to
   `bof3-reverse`.
2. A relevant declared companion call requires passing `companion-check`
   before dispatch; a static record proves no ABI/ownership.
3. Executor returns mission JSON + checked acceptance report: an exact lift, or
   a **review-pending escalation** that leaves its best coherent clean-C
   candidate in the owned files. It must identify the pre-mission baseline,
   best live diff, first mismatch class, attempted rungs, and changed files.
4. Parent runs one live `byte-match` only for an exact claim. Status cache is
   never acceptance.
5. Dispatch `bof3-review` for **both exact and non-exact** results with the
   brief, best live asm diff, changed-file diff, and executor rung report.
   For non-exact work the reviewer must classify the residual, identify any
   skipped/misapplied lever, require focused target-qualified Rizin context when
   types, symbol roles, layouts, caller contracts, branch targets, or lifetimes
   remain unknown, and record a genuinely reusable cross-function lesson before
   the candidate is discarded. `needs-fix`: retry executor ≤2
   times from the reviewed best candidate, then re-match/review; `block`:
   journal, continue. No mid-queue cleanup — cleanup runs once at batch end.
6. Only exact + review pass: stage owned source/header/map/Splat facts,
   verify the staged list, commit `feat(decomp): byte-match <function>`,
   journal it.
7. A reviewed non-exact result never stops the queue. Parent captures the
   reviewer verdict and reusable lesson path in the journal, restores every
   owned candidate file to the recorded pre-mission state, verifies a clean
   worktree/index, then runs the decomp.me final rung below and starts the next
   selector with a fresh mission context. Never restore before review: the
   candidate diff is primary diagnostic evidence.

Use `subagent_supervisor` replies for child requests, not generic intercom.

## Post-loop organization

Cleanup runs once at batch end: after the queue completes
(or budget is reached) with reviewed exact lifts, run one organization
cleanup pass via `bof3-cleanup` over the batch's exact functions before the
next queue per bof3-re's `Order of operations` — cosmetic,
evidence-preserving changes only (naming, comment metadata, organization
within owned files).

After any cleanup edit, re-run a fresh live `byte-match` and dispatch a fresh `bof3-review`;
both must pass before the function stays eligible. Lift-body edits also
first need live `bin/asm-diff TARGET@0xADDRESS --detail normal` with no
first-difference. A cleanup that breaks byte-match is reverted, never
fixed forward.

## Partial re-lift and decomp.me final rung

After the bounded queue and checkpoint, a user may authorize a fresh
`out/non-exact-lifts.json` pass. Process every current `partial` row
serially, preserving each source's pre-mission state. Skip rows in the
report's `contains_data` list (range embeds reviewed `D_*` data; unliftable
until the Splat segment splits — route to the user, never dispatch). Each
remaining row gets a fresh
executor mission, parent evidence check, and independent review only if it
exact-matches.

For every non-exact result, the executor leaves the best coherent candidate
review-pending. The parent dispatches `bof3-review`, integrates any durable
cross-function lesson, then restores the recorded prior state and runs the
final execution rung: `bin/scratchpad share SELECTOR`. Publish only
if the selector begins at a reviewed Splat `c` or `asm` `func_XXXXXXXX`
boundary and its restored partial source exists. Missing ABI, call ownership,
analyzer confidence, or other lifting evidence does **not** make an otherwise
valid function unshareable. A data-leading, non-function, unreviewed, or
source-less selector is `not shareable: <reason>`. Record the URL,
`not shareable` reason, or publication failure in the journal, then continue.
Never alter reviewed map/Splat facts to make a scratch payload; a scratch is
public escalation evidence, never acceptance. A prior scratch URL never
replaces a current mission or exact gate.

## Stop/report

Do not stop merely because a candidate is non-exact, unshareable, a decomp.me
publish fails, review rejects an exact claim, or a bounded executor mission
escalates. Journal and continue. Stop only when the queue is exhausted, the
budget is reached, generated-evidence recovery is fatal, child output
conflicts with the owned worktree, or user approval is required. Print
journal, counts, commits, every scratch URL/result, risks, next step.

## Child brief

```
Task: lift/review SELECTOR (`TARGET@0xADDRESS`, or shipped EMI
`BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`).
Context: function brief + mission/diff + owned-file diff. First run
`agent-context.py <reverse|review> SELECTOR`.
Authority: executor edits only owned source/internal.h/map/Splat; reviewer
may also edit only `docs/specs/**/*.md` or `docs/agents/lessons.md` for
durable cross-function findings.
No git writes, setup, other targets, or children. A non-exact executor leaves
its best coherent candidate review-pending; only the parent restores it after
review and durable-lesson integration.
Return protocol/checklist JSON and required acceptance-report.
```

Role protocols are preloaded from `.pi/skills/bof3-re/references/REVERSE/` and
`.pi/skills/bof3-re/references/REVIEW/` by `agent-context.py`.
