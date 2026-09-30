---
name: bof3-naming
description: Discover naming evidence or apply reviewed BOF3 symbol, source-file, argument and local-variable renames. Separate opportunity, audit and transaction modes; type representation and macro extraction belong to their owning skills.
---

# BOF3 naming

Select automatically for naming work. Parent chooses canonical mode and pins inputs
within authorized mission. Missing inputs block; never infer them, switch modes or
treat evidence as edit permission. Preserve behavior, bytes, ABI, addresses,
boundaries, compiler settings, partial metadata and unrelated dirty work.

## Select one mode

Read only selected branch references before acting. Cross-mode links are handoffs,
not permission.

| Mode | Required contract |
| --- | --- |
| `naming-opportunity TARGET ID` | Read-only [opportunity assessment](references/opportunities.md); no source/map/report edits |
| `source-identifiers TARGET`, `describe-source-identifier TARGET ID` | Read-only [argument/local leads](references/source-identifiers.md#discovery) |
| `prepare/apply/verify/rollback-source-transaction` | [Source identifier transaction](references/source-identifiers.md#transaction), one argument/local in one function, PRE-bound native byte gate |
| `audit-target TARGET` | [Naming audit v3](references/naming-audit-v3.md), bounded evidence/full-report closure, no identities |
| `symbol`, `type`, `repair`, `retained-lift` | [Identity transactions](references/identity-transactions.md), separately approved scope/validation/rollback |
| `retained-lift` cosmetics | Also [byte-safe cosmetics](references/byte-safe-cosmetics.md); no semantic lift rewrite |
| `relocate-batch` | [Source relocation](references/source-relocation.md), approved atomic ownership/build-graph updates |

## Protocol

1. Bind actual mission scope/budget; audit selects one target. Serialize report
   writers; held lock stops. Read [mission method](references/opportunities.md#mission-method)
   before delegation. Historical call/time heuristics are not hard caps.
2. Use `bin/harness agent context cleanup CANONICAL_REQUEST...`. Preserve exact
   target/ID/fingerprint. Opportunity check:
   `bin/harness naming describe-opportunity TARGET ID --expected-fingerprint PIN`.
   Missing caller pin permits scouting only.
3. Prove names through behavior, callers and consumers. Raw spelling supplies no
   semantic/storage proof. Report gaps instead of inventing names.
4. Follow selected mode's evidence, application and rollback gates. Never replace
   frozen lead/report, expand queue, refresh index or reset budget silently.
5. Report selected-row result separately from successful unfiltered full-report
   `complete:true` and independent terminal acceptance. Neither implies the other.

Type spelling preserves representation. Argument/local types, layout and C
representation route to `$bof3-types`; extraction routes to `$bof3-macros`.

## Dispatch and close

`sh .pi/skills/bof3-naming/scripts/audit.sh ...` forwards unchanged args to
`bin/harness naming`; `scripts/evidence.sh` forwards to `bin/harness naming evidence`.
Harness owns parsing, evidence, transactions and acceptance.

Before proposing names read [style/origin rules](references/naming-style.md).
Before no-op/exhaustion acceptance read [terminal gate](references/terminal-acceptance.md).
Before owner deadlines/recovery read [lifecycle](../bof3-types/references/transaction-lifecycle.md).
Return tool count, wall time, method and accepted/blocked work. Parent archives
measurements; reusable directives belong in skill references, not external docs.
