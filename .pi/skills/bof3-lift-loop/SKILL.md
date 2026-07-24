---
name: bof3-lift-loop
description: Run an autonomous, review-gated BOF3 function-lifting loop. Use via `/skill:bof3-lift-loop` or when explicitly asked to lift a batch of functions to byte-match — it selects candidates via bin/rev-query, dispatches bounded executor and read-only reviewer subagents per function, and commits only reviewed exact lifts. Do not use for a single hand-guided lift (use `/skill:bof3-re`).
---

# BOF3 Lift Loop

Autonomously lift BOF3 functions to exact byte-match behind an independent
review gate. Read `AGENTS.md` and load `/skill:bof3-re` first. The loop
orchestrates two bounded project agents: `bof3-reverse` (one-function
executor) and read-only `bof3-review`. The parent workflow owns all git
operations; subagents never commit. The project agent definitions own model,
thinking, and tool policy: both use `ninerouter/gpt-combo` at `low`; keep only
native tools plus `contact_supervisor` in their strict allowlists.

## Phase 1 — Setup (interactive)

Confirm before starting:

- **Targets**: which targets to lift (default: all reviewed targets).
- **Selection**: `quick-wins` (default), `leafs`, `duplicates`, `hotspots`, or `pareto`.
- **Budget**: `MAX_FUNCTIONS` (default 3–5 for a first run).
- **Commit authorization**: the loop commits ONLY reviewed exact lifts. Confirm
  the user authorizes commits and the target branch.
- **Scope**: in-scope / out-of-scope targets or functions.

## Phase 2 — Baseline

```sh
python3 .pi/skills/bof3-lift-loop/scripts/loop-status.py --selection hotspots
```

The dashboard combines worktree/staged state, the existing journal, and ranked
candidates in one JSON document. When candidate selection finds stale generated
Rizin/index evidence, it automatically refreshes stale snapshots with
`bin/rz-project analyze` and rebuilds the generated reverse index with
`bin/index`; it never edits authored or tracked files. Use `--no-recover` only
to diagnose without refreshing. Do not dispatch when it still reports a dirty
tree or index failure. When the gates pass:

```sh
bin/decomp-status --json -o out/lift-loop/baseline.json
```

Initialize the journal `out/lift-loop/results.tsv` with the header
`function	status	commit	notes`.

## Phase 3 — Loop (serial)

Repeat until the budget is spent, no candidates remain, or a stop rule fires:

1. **Pick** the next candidate from `loop-status.py` (or, when refreshing,
   `bin/rev-query <selection> --unlifted --detail minimal --limit 1 --json`).
2. **Brief** it with
   `python3 .pi/skills/bof3-re/scripts/function-brief.py TARGET@0xADDRESS`.
3. **Dispatch** one `bof3-reverse` subagent with the mission brief (template
   below). Do not override its project-defined `checked` acceptance policy with
   inferred or `verified` acceptance: that policy accepts either an exact lift or
   an evidence-backed escalation after the executor restores every mission edit.
   The executor lifts the function and returns a structured result.
4. **Verify** independently in the parent:
   `bin/byte-match TARGET@0xADDRESS` exits 0 AND
   `bin/decomp-status TARGET --json` reports `status == "exact"`.
5. **Supervisor decision**: when a child calls `contact_supervisor`, the parent
   detaches. Reply with `subagent_supervisor({ action: "reply", replyTo:
   REQUEST_ID, message: "..." })`, then wait for that run before resuming the
   loop. Do not use generic `intercom`.
6. **Review gate**: dispatch one read-only `bof3-review` subagent on the
   function. On `needs-fix`, re-dispatch `bof3-reverse` with the findings
   (≤2 bounded retries), then re-verify and re-review. On `block`, escalate.
7. **Commit** only if verified exact AND review `pass`: `git add` that
   function's files (`src/<t>/func_*.c`, its `internal.h`, target map, Splat
   boundary) and commit with a concise `feat(decomp): byte-match <function>`.
8. **Journal** the result (function, status, commit sha, notes).

## Phase 4 — Report

Print the journal, the exact/partial/needs-fix/block counts, the `git log` of
committed lifts, and next steps.

## Guardrails

- Commit ONLY exact byte-matched lifts that passed review — never partial/invalid.
- Subagents never commit/push/reset/clean or run setup; the parent owns git.
- Leave the agent extension list unspecified so Pi retains the parent extension
  set and native supervisor channel. `tools` is an allowlist, not an extension
  loader; do not add unavailable context-mode tools.
- Serial execution: functions in the same target share `internal.h`, so never run
  two executors on one target concurrently.
- Never commit secrets or `inputs/` media.
- After any SDK-map edit, regenerate bindings (`bin/symbols psyq-bindings
  TARGET --write`) and rebuild before re-verifying.

## Stop rules

Stop and report when: the budget is exhausted; no unlifted candidates remain;
the build breaks; a function still fails after its review retries; subagent
outputs conflict; or scope creep / approval is needed.

## Subagent brief templates

### bof3-reverse (executor)

```
## Task
Lift <TARGET@0xADDRESS> to an exact byte-match.

Load `/skill:bof3-re` before starting.

## Context
<paste: bin/rev-query mission TARGET@0xADDRESS --json>

## Authority
Write scope: the target manifest's `source_dir` function source and `internal.h`,
its target map, and Splat boundary only. Forbidden: git commit/push/reset/clean,
rm, setup, other targets. May not spawn children.

## Expected return
Mission JSON plus the project-defined `checked` `acceptance-report`. The report's
outcome-aware criterion is satisfied only by an exact byte match or by an
 evidence-backed escalation with every mission edit restored; do not override the
agent acceptance policy with `verified` unless the launch supplies real runtime
verification commands.
```

### bof3-review (reviewer)

```
## Task
Review the just-matched lift <TARGET@0xADDRESS> for correctness and guideline
compliance. Read-only. Load `/skill:bof3-re` before starting.

## Context
<paste: git diff for the function's files> + the mission brief.

## Authority
Read-only. No edits, no git writes, no children.

## Expected return
JSON: {"function", "verdict": "pass"|"needs-fix"|"block",
"findings": [{"file", "line", "rule", "issue"}]}
```

## References

- `references/MISSION_PROTOCOL.md` — the executor's per-function lift procedure.
- `references/REVIEW_CHECKLIST.md` — the reviewer's guideline/correctness checks.
