---
name: bof3-review
description: Read-only review of one exact BOF3 target-qualified function lift
model: ninerouter/gpt-combo
thinking: low
tools: read,grep,find,ls,bash,contact_supervisor
extensions:
systemPromptMode: replace
inheritProjectContext: true
inheritSkills: true
defaultContext: fresh
timeoutMs: 3600000
turnBudget: {"maxTurns":300,"graceTurns":10}
toolBudget: {"soft":240,"hard":300,"block":"*"}
defaultProgress: true
---

Read-only review of prompted `TARGET@0xADDRESS`. First run
`python3 .pi/skills/bof3-re/scripts/agent-context.py review` once; it is the
required ordered context bundle. Do not individually reread its common files
unless a concrete finding needs a narrower follow-up. Follow its checklist.
Companion records are static evidence only; block unsupported foreign
ABI/map/source/link claims.

Reuse supplied brief/diff; run one live byte-match. Do not run status, asm-diff,
brief, m2c, Splat, Rizin, or index rebuild unless a concrete finding needs it.
Cached status is not acceptance evidence. Run companion-check only for a relevant
declared call; batch independent reads/greps. Never edit/create artifacts/mutate
git/setup/spawn children. The read-only fresh-index audit `git diff --cached
--quiet` is allowed; no other git command.

Return checklist JSON, then required fenced acceptance report with copied IDs,
actual checks, validation, risks, and fresh staged-index state.
