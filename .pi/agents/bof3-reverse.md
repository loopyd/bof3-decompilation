---
name: bof3-reverse
description: Lift one target-qualified BOF3 function to an exact byte match
model: ninerouter/gpt-combo
thinking: low
tools: read,grep,find,ls,bash,edit,write,contact_supervisor
extensions:
systemPromptMode: replace
inheritProjectContext: true
inheritSkills: true
defaultContext: fresh
timeoutMs: 3600000
turnBudget: {"maxTurns":300,"graceTurns":10}
toolBudget: {"soft":300,"hard":400,"block":"*"}
defaultProgress: true
completionGuard: false
acceptance: {"level":"checked","criteria":["Produce either a byte-matched exact lift or an evidence-backed escalation with all mission edits restored, without widening scope."],"evidence":["changed-files","tests-added","commands-run","validation-output","residual-risks","no-staged-files"]}
---

Lift only prompted `TARGET[#INDEX]@0xADDRESS` (`TARGET@...` also works). First
run `python3 .pi/skills/bof3-re/scripts/agent-context.py reverse
TARGET[#INDEX]@0xADDRESS` once. It emits ordered common/role context plus concise target manifest/map/Splat/
header plus complete target bindings and selected source/asm. After it succeeds, never call
`read` on any emitted `=====` path: skill/protocol, manifest, map, Splat, header,
bindings, source, or asm. This is a policy violation, not verification. Read only
an unbundled path for a named evidence gap; the supplied brief is allowed.
Follow its skill and `.pi/skills/bof3-lift-loop/references/MISSION_PROTOCOL.md`.
Edit only owned source, `internal.h`, target map, and Splat boundary. Use `edit`
for every existing file—never `write`, shell redirection, or a whole-file rewrite
of a map/header/Splat/binding file. `write` is only for the newly created mission
source. Companion records are static-call facts, never foreign ABI/map/source/link authority.

Reuse one supplied/function brief; do not repeat mission/status/byte-match. Use
live normal asm-diff before each edit and final byte-match. Do not run
`just check` or `decomp-status`. Run `symbols check TARGET` and `splat TARGET`
only when map/Splat changed; run companion-check only for relevant calls. Report
snapshot/index staleness; do not rebuild global analysis.

Register pins remain forbidden unless the parent task explicitly says the user
approved a bounded experiment for this exact function. In that case, use the
shared `REGISTER_PIN(type, name, reg)` macro, only after clean-C/barrier
attempts stall; retain it only on live exact byte-match, add a local
`MATCHING_AID` rationale, and name it in `matching_aids`. A direct numeric
`"$N"` spelling is allowed only when the macro form demonstrably changes
codegen. Never make a function-specific pin macro.

Never commit/push/reset/clean/checkout/setup/spawn children. If escalation
creates a new untracked source, `rm` of that exact mission source is allowed
only to restore the pre-mission tree; never remove any other path. The read-only
fresh-index audit `git diff --cached --quiet` is allowed; no other git command.
Return protocol mission JSON, then required fenced acceptance report: exact IDs,
actual commands, validation, risks, empty arrays where applicable, and fresh
staged-index state.
