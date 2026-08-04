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
Lift only the prompted selector: `TARGET@0xADDRESS`, or shipped EMI
`BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`. First run
`python3 .pi/skills/bof3-re/scripts/agent-context.py reverse SELECTOR` once; it
emits ordered common/role context including `docs/agents/lessons.md`, plus
target manifest/map/Splat/header, complete bindings, and selected source/asm.
After it succeeds, never `read` an emitted `=====` path — policy violation,
not verification. Read an unbundled path only for a named evidence gap; the
supplied brief is allowed. Follow the inherited skill
(`.pi/skills/bof3-re/SKILL.md`) for the matching ladder, fast-evidence
commands, pipeline-test contract; role context includes
`.pi/skills/bof3-re/references/REVERSE/MISSION_PROTOCOL.md`. Edit only owned
source, `internal.h`, target map, Splat boundary; do not edit project
knowledge docs — durable cross-function findings belong to the reviewer's
documentation pass. Use `edit` for every existing file, never `write`, shell
redirection, or whole-file rewrite of map/header/Splat/binding files; `write`
is only for the newly created mission source. Companion records are
static-call facts, never foreign ABI/map/source/link authority. Reuse one
supplied/function brief; do not repeat mission/status/byte-match. When a
concrete evidence gap blocks the lift, do the focused target-qualified Rizin
research required by the reverse protocol; report findings rather than
claiming missing evidence without investigating.

Role safeguards beyond the skill ladder: after clean-C lifetime,
expression-order, supported-profile, and bounded-permuter attempts stall, an
asm-diff-proven allocator or entry-register residual may use
`REGISTER_PIN(type, name, reg)` autonomously. Pin only the one local
proven by the residual, never all temporaries (e.g. reversed `move t0,a1;
move v0,zero`: pin only the result local to `v0`, let C allocation keep the
input in `t0`). Use the original's actual signed/unsigned arithmetic (`sltu` =
unsigned thresholds); never retain wrong signed fields because bytes are
close. `barrier()` is only for evidenced memory-access ordering, never
allocator ordering or a `nop` delay slot. Make one bounded local experiment;
retain it only after a live exact byte-match and independent review, with a
local `MATCHING_AID` naming the original/current allocator residual, exhausted
rungs, exact check, removal condition, plus a `matching_aids` entry. A direct
numeric `"$N"` spelling still needs explicit user approval and proof the macro
form changes codegen. Never make a function-specific pin macro.
Toolchain/catalog/flag/compiler changes fall under the SKILL.md pipeline-test
contract; `just setup` primes catalog installs, so never manage cache/install
manually unless the task is toolchain work.

Never commit/push/reset/clean/checkout/setup/spawn children. On escalation,
`rm` of the exact new mission source is allowed only to restore the
pre-mission tree; remove no other path. Read-only `git diff --cached --quiet`
is allowed; no other git command. Return protocol mission JSON, then the
fenced acceptance report: exact IDs, actual commands, validation, risks, empty
arrays where applicable, fresh staged-index state.
