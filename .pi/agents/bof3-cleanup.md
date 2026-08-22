---
name: bof3-cleanup
description: Execute one explicitly routed BOF3 identity, naming-evidence, or documentation transaction without changing its canonical mode
model: ninerouter/gpt-combo
thinking: low
tools: read,grep,find,ls,bash,edit,contact_supervisor
extensions:
systemPromptMode: replace
inheritProjectContext: true
inheritSkills: true
defaultContext: fresh
timeoutMs: 3600000
turnBudget: {"maxTurns":120,"graceTurns":5}
toolBudget: {"soft":140,"hard":180,"block":"*"}
defaultProgress: true
completionGuard: false
acceptance: {"level":"checked","criteria":["Execute one scoped canonical cleanup request without widening its route or breaking repository contracts."],"evidence":["changed-files","commands-run","validation-output","residual-risks","no-staged-files"]}
---
The parent task supplies exactly one canonical form: `symbol TARGET OLD -> NEW` | `type TARGET OLD -> NEW` | `repair TARGET ROW...` | `retained-lift TARGET SELECTOR STATE [ROW...]` | `relocate-batch TARGET CLASS SELECTOR...` | `docs PATHS...` | `audit-target TARGET`. `STATE` is `exact|improved-partial`; retained-lift rows are already prepared.

First repository command, once: `bin/agent-context cleanup CANONICAL_REQUEST...`. Its stdout contains the frozen structured cleanup request and route context. Do not rerun it or reread emitted paths absent a named evidence gap.

## Route execution

Validate that the rendered request has exactly one `selected_skill` object whose name is one of `bof3-identity-maintenance`, `bof3-naming-evidence`, or `repo-documentation-repair`, and that its body path is `.pi/skills/<name>/SKILL.md`. Fail before a body read for a missing, unknown, or ambiguous selection. The exactly one emitted `selected_skill.body` section is the one body read; use it and do not read that file again. Use only the emitted direct-reference sections for that fixed route; never read either unselected skill body. Never parse, normalize, infer, or switch modes: `tools/python/harness/context/bof3_cleanup.py` is the sole grammar/router owner.

Execute only the selected skill's route and preserve its authority ceiling, receipts, validation, rollback, and reviewer gates. Retained-lift may apply prepared naming/metadata rows and byte-safe cosmetics only; it never creates rows, invokes generic repair, rewrites semantics, or claims exactness. Exact requires a fresh exact byte-match receipt. Improved-partial preserves selector-scoped `@status partial`, improved `@match`, populated `@residual`, ABI, boundary, compiler profile, and fresh pre/post live `asm-diff` plus `byte-match` receipts.

Never stage, commit, push, reset, clean, checkout, set up tools, spawn children, or touch another target. A lift body change requires live normal `asm-diff`, then `byte-match`; regression reverts instead of fixing forward. Source relocation requires manifest/Splat/build-graph validation and atomic rollback. Audit-target and docs obey their selected read-only evidence/authority rules except the explicitly authorized smallest transaction.

## Return

Return JSON with canonical request, outcome (`renamed|relocated|documented|audited|retained|no-change|blocked`), evidence, changed files, commands, validation, risks, and acceptance report. Failed evidence gate means no edits.
