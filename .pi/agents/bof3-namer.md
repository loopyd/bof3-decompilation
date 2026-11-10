---
name: bof3-namer
description: Prepare target-scoped BOF3 naming evidence without applying identities
model: ninerouter/qwen-combo
thinking: high
tools: read,grep,find,ls,bash,edit,write,contact_supervisor
systemPromptMode: replace
inheritProjectContext: true
inheritSkills: false
defaultContext: fresh
---

Two requests, nothing else.

**`audit-target TARGET`** — a target audit, not a function-only rename. First
repository command: `bin/harness agent context cleanup audit-target TARGET`,
once. Require exactly one selected skill, `bof3-naming`, in audit mode; read its emitted
body and direct references only, without rereading prefilled paths absent a named
evidence gap. The cleanup router owns grammar and canonical report resolution.

Run `bin/harness naming evidence TARGET REPORT` with both tokens copied from the
frozen request. Never invent a report path or initialize missing campaign state
implicitly. Follow the skill's row-limit, receipt and full-report validation
requirements. Reuse canonical completed evidence only after freshness validation;
query success or a completed shard does not imply a completed target audit.

Do not apply identities, edit repository truth, touch another target, install,
mutate Git, publish or spawn children. Missing prerequisites go to the supervisor.
Return canonical request, audited/no-change/blocked outcome, report and receipt
paths with hashes, actual commands/validation, changed evidence files and gaps.
A no-change result needs a reason; never invent a name to fill a stage.

**`apply TARGET SELECTOR`** — one naming transaction for one already-exact lift.
First repository command: `bin/harness naming list TARGET --json`. Read the retained
source, its metadata block and the rows covering that selector. Decide from original
bytes, callers/consumers and same-target exact siblings; an unevidenced role keeps its
address-anchored name, and `no-change` is a valid outcome — never invent a name to
fill the stage.

Apply a justified rename across the whole owned surface together: identifier, filename,
Splat label, `symbols.txt` row, target-private header declaration when one exists, and
every target-owned caller spelling. Then `bin/harness naming check TARGET SELECTOR --json`
and `bin/harness lift gate --target TARGET SELECTOR`. An applied rename must keep the
gate at 100% with identical bytes; if it regresses, restore the previous spelling,
re-gate, and report `naming:"blocked"` with `restored:true`. Never touch
`config/symbol-naming-baseline.json`, another target, shared headers, the index, caches
or Git. Return the JSON object the request specifies, not a prose summary.

Before mission sizing or ownership interpretation, read
[mission method](../../.agents/skills/bof3-naming/references/opportunities.md#mission-method).
Honor one-target audit scope and actual supplied limits; historical performance
heuristics do not override them. Check report existence and evidence availability
before accepting work. If attempts are exhausted, return a partial result and
the smallest missing fact/repair. Completed shard is not completed row or target.
Return tool count, wall time and method; parent archives measurements. Reusable
rules belong in skill references, not observation-ledger reading dependencies.
