---
name: bof3-lift-loop
description: Run an autonomous, review-gated BOF3 function-lifting loop. Use when the user invokes `$bof3-lift-loop` to lift a batch of functions to byte-match — it selects candidates via bin/rev-query, dispatches bounded bof3-reverse executor subagents per function, gates each exact match through a read-only bof3-review subagent, and commits only reviewed exact lifts. Do not use for a single hand-guided lift (use $bof3-re).
---

# BOF3 Lift Loop

Autonomously lift BOF3 functions to exact byte-match behind an independent
review gate. Read `AGENTS.md` and load `$bof3-re` first. The loop orchestrates
two bounded subagents: `bof3-reverse` (executor — lifts one function) and
`bof3-review` (reviewer — read-only guideline/correctness gate). The parent
workflow owns all git operations; subagents never commit.

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
git status --short                      # tree must be clean before starting
bin/decomp-status --json -o out/lift-loop/baseline.json
```

Initialize the journal `out/lift-loop/results.tsv` with the header
`function	status	commit	notes`.

## Phase 3 — Loop (serial)

Repeat until the budget is spent, no candidates remain, or a stop rule fires:

1. **Pick** the next candidate:
   `bin/rev-query <selection> --unlifted --detail minimal --limit 1 --json`.
2. **Brief** it: `bin/rev-query mission TARGET@0xADDRESS --json`.
3. **Dispatch** one `bof3-reverse` subagent with the mission brief (template
   below). The executor lifts the function and returns a structured result.
4. **Verify** independently in the parent:
   `bin/byte-match TARGET@0xADDRESS` exits 0 AND
   `bin/decomp-status TARGET --json` reports `status == "exact"`.
5. **Review gate**: dispatch one `bof3-review` subagent (read-only) on the
   function. On `needs-fix`, re-dispatch `bof3-reverse` with the findings
   (≤2 bounded retries), then re-verify and re-review. On `block`, escalate.
6. **Commit** only if verified exact AND review `pass`: `git add` that
   function's files (`src/<t>/func_*.c`, its `internal.h`, target map, Splat
   boundary) and commit with a concise `feat(decomp): byte-match <function>`.
7. **Journal** the result (function, status, commit sha, notes).

## Phase 4 — Report

Print the journal, the exact/partial/needs-fix/block counts, the `git log` of
committed lifts, and next steps.

## Guardrails

- Commit ONLY exact byte-matched lifts that passed review — never partial/invalid.
- Subagents never commit/push/reset/clean or run setup; the parent owns git.
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

## Context
<paste: bin/rev-query mission TARGET@0xADDRESS --json>

## Authority
Write scope: src/<target>/func_<ADDR>.c, its internal.h, the target map, and the
Splat boundary only. Forbidden: git commit/push/reset/clean, rm, setup, other
targets. May not spawn children.

## Expected return
JSON: {"function", "status": "exact"|"partial"|"escalated", "match_percent",
"files_changed": [...], "matching_aids": [...], "notes"}
```

### bof3-review (reviewer)

```
## Task
Review the just-matched lift <TARGET@0xADDRESS> for correctness and guideline
compliance. Read-only.

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
