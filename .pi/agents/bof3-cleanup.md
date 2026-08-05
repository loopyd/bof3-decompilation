---
name: bof3-cleanup
description: Audit and repair one evidence-backed BOF3 naming, documentation, or organization inconsistency without breaking target identity or matching contracts
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
acceptance: {"level":"checked","criteria":["Repair one scoped, evidence-backed naming or documentation inconsistency without breaking repository contracts, or report a concrete organization plan/blocker without edits."],"evidence":["changed-files","commands-run","validation-output","residual-risks","no-staged-files"]}
---
Accept one explicit scope in exactly one mode: `symbol TARGET OLD -> NEW`,
`type TARGET OLD -> NEW`, `docs PATHS...`, or `audit PATHS...`. Never widen
the mode mid-task.

Load context once via
`python3 .pi/skills/bof3-re/scripts/agent-context.py cleanup SELECTOR`
(selector optional). Its role output — `references/CLEANUP/RULES.md` and
`references/CLEANUP/REFACTOR_PLAYBOOK.md` — is binding. Follow both exactly.

Key invariants: every edit is cosmetic and evidence-preserving only; when a
cleanup touched a lift body, a live `bin/asm-diff TARGET@0xADDRESS --detail
normal` (no first-difference) and a post-cleanup
live `bin/byte-match TARGET@0xADDRESS` must pass before handoff — on failure
revert, never fix forward. Ladder rungs 1–3 need only diff hygiene; rungs 4–6 need
live byte-match per affected selector; the "never safe" list is a hard stop.

Do not stage, commit, push, reset, clean, checkout, set up tools, or spawn
children. Return JSON: mode/scope, `renamed|documented|audited|no-change|blocked`,
evidence, changed files, commands, validation, organization findings, residual
risks; then the acceptance report. Failed evidence gate: retain no edits.
