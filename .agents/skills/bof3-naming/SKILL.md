---
name: bof3-naming
description: Discover BOF3 naming evidence or apply reviewed symbol, source-file, argument and local renames. Keep opportunity, audit and transaction modes separate; types and macros retain their owners.
---

# BOF3 naming

Bind the parent-pinned canonical mode and authorized mission inputs.
Missing inputs block; never infer them, switch modes or
treat evidence as edit permission. Preserve behavior, bytes, ABI, addresses,
boundaries, compiler settings, partial metadata and unrelated dirty work.

## Select one mode

Read only selected branch references before acting. Cross-mode links grant no permission.

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

1. Bind mission scope/budget; audit selects one target. Serialize report
   writers; held lock stops. Read [mission method](references/opportunities.md#mission-method)
   before delegation. Historical call counts are not hard caps.
2. Use `bin/harness agent context cleanup CANONICAL_REQUEST...`. Preserve exact
   target/ID/fingerprint. Opportunity check:
   `bin/harness naming describe-opportunity TARGET ID --expected-fingerprint PIN`.
   Missing caller pin permits scouting only.
3. Prove names through behavior, callers and consumers, never raw spelling.
   Report semantic/storage gaps instead of inventing names.
4. Follow the mode's evidence, application and rollback gates. Never replace
   frozen lead/report, expand queue, refresh index or reset budget silently.
5. Separate selected-row results, successful unfiltered full-report `complete:true`
   and independent terminal acceptance; none implies another.

Type spelling preserves representation. Argument/local types, layout and C
representation route to `$bof3-types`; extraction routes to `$bof3-macros`.

## Dispatch and close

`sh .agents/skills/bof3-naming/scripts/audit.sh ...` forwards unchanged args to
`bin/harness naming`; `scripts/evidence.sh` forwards to `bin/harness naming evidence`.
The harness owns parsing, evidence, transactions and acceptance.

Before proposing names read [style/origin rules](references/naming-style.md).
Before no-op/exhaustion acceptance read [terminal gate](references/terminal-acceptance.md).
Before owner execution or recovery read [lifecycle](../bof3-types/references/transaction-lifecycle.md).
Return tool count, wall time, method and accepted/blocked work. Parent archives
measurements; directives belong in skill references, not external docs.
