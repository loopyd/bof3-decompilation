---
name: bof3-lift-loop
description: Autonomously lift a serial BOF3 batch to reviewed exact byte matches. Selects candidates, dispatches bounded executor/reviewer agents, and commits only user-authorized reviewed exact lifts. Not for one hand-guided lift; use `/skill:bof3-re`.
---

# BOF3 lift loop

Read `AGENTS.md` and load `/skill:bof3-re`. Parent owns selection, checkpoints,
git, and commits; `bof3-reverse` writes one function and `bof3-review` is
read-only. Agents own their model/tool policy. Never run two functions in one
target concurrently: they share `internal.h`.

## Confirm

Before starting, get targets (default reviewed), selection (`quick-wins` default),
budget (start 3–5), scope exclusions, branch, and explicit commit authorization.
Subagents never commit/push/reset/clean/setup; never commit `inputs/` or secrets.

## Baseline and queue

```sh
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots
bin/decomp-status --json -o out/lift-loop/baseline.json
```

Do not dispatch on dirty tree/index failure. `loop-status` recovery is generated
state only. Queue candidates from one fresh snapshot/index; after map/Splat edits,
continue the bounded queue, then refresh edited target snapshots and rebuild index
once at a checkpoint before requesting another queue. Never query a stale index.
Initialize `out/lift-loop/results.tsv`: `function status commit notes`.

## Serial loop

For each candidate until budget/stop:

1. Get one `function-brief.py TARGET@0xADDRESS`; pass it with the canonical
   `TARGET[#INDEX]@0xADDRESS` selector to `bof3-reverse`.
2. If a relevant declared companion call exists, require passing `companion-check`
   before dispatch; a static record alone proves no ABI/ownership.
3. Executor returns mission JSON + its checked acceptance report. It may return an
   exact lift or fully restored escalation; do not override that policy.
4. Parent runs one live `byte-match`. Do not use status cache as acceptance.
5. Dispatch `bof3-review` with brief, diff, and changed-file diff. On `needs-fix`,
   retry executor ≤2 times, then re-match/review; on `block`, stop/escalate.
6. Only exact + review pass: stage owned source/header/map/Splat facts, verify the
   staged file list, commit `feat(decomp): byte-match <function>`, journal it.

Use `subagent_supervisor` replies for child requests, not generic intercom.

## Stop/report

Stop on budget, no candidates, build/index failure, unresolved review retries,
conflicting child output, or required approval. Print journal, counts, commits,
risks, and next step.

## Child brief

```
Task: lift/review TARGET[#INDEX]@0xADDRESS.
Context: function brief + mission/diff + owned-file diff. First run
`agent-context.py <reverse|review> TARGET[#INDEX]@0xADDRESS`.
Authority: executor may edit only owned source/internal.h/map/Splat; reviewer read-only.
No git writes, setup, other targets, or children; escalation may delete only its
new mission source to restore the tree.
Return protocol/checklist JSON and required acceptance-report.
```

See `references/MISSION_PROTOCOL.md` and `references/REVIEW_CHECKLIST.md`.
