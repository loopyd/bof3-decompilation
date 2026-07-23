# BOF3 Lift Loop — quick start

An autonomous, review-gated loop that lifts BOF3 functions to exact byte-match.
It picks candidates, dispatches a bounded executor subagent per function, gates
each exact match through a read-only reviewer subagent, and commits only
reviewed exact lifts.

## What runs

- **`bof3-reverse`** (executor) — loads `/skill:bof3-re`, then lifts one
  `TARGET@0xADDRESS` to byte-match. Edits only that function's
  source/header/map/Splat. Never commits.
- **`bof3-review`** — loads `/skill:bof3-re` and performs the read-only
  guideline + correctness gate (`pass` / `needs-fix` / `block`).
- **Parent loop** (this skill) — owns selection, verification, git, and the journal.

## Prerequisites

1. Reverse index built: `bin/rz-project analyze TARGET` for any stale target,
   then `bin/index` (`bin/rev-query status` shows coverage).
2. Restart Pi after adding or changing project-local agents or skills so it
   reloads the resources.

## Launch

```
/skill:bof3-lift-loop
```

It confirms targets / selection (`quick-wins` default) / budget (3–5) / commit
authorization, then loops until the budget is spent or a stop rule fires:

```
pick quick-win → rev-query mission → bof3-reverse lifts → byte-match verify
  → bof3-review gate → commit iff exact+pass → journal → next
```

## Guardrails

- Commits **only** exact byte-matched lifts that passed review.
- Subagents never commit/push/reset/clean or run setup; the parent owns git.
- Serial per target (functions in a target share `internal.h`).
- `needs-fix` → ≤2 bounded executor retries, then re-verify/re-review.
  `block` → escalate to the user.

## Troubleshooting

- `stale Rizin snapshot` → `bin/rz-project analyze TARGET`, then `bin/index`.
- `reverse index not found` → `bin/index`.
- Project agents or skills not visible → trust the project, then restart Pi.

## See also

- `SKILL.md` — full loop protocol + subagent brief templates.
- `references/MISSION_PROTOCOL.md` — executor procedure.
- `references/REVIEW_CHECKLIST.md` — reviewer checks.
- `/skill:bof3-re` — hand-guided single-function lifting.
