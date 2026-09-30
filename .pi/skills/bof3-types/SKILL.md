---
name: bof3-types
description: Inspect BOF3 declarations, uses and representation opportunities; prepare and apply reviewed type transactions for cleanup or finalization.
---

# BOF3 types

## Protocol

1. Read [discovery](references/type-application.md#discovery) for inspection.
   Before transaction preparation, writes, native gates, review, revalidation or
   recovery, read selected [application branch](references/type-application.md)
   and [transaction lifecycle](references/transaction-lifecycle.md).
   Representation, naming and macro extraction remain separate domains.
2. For `bin/harness agent context cleanup type-opportunity TARGET ID`, retain emitted
   target/opaque ID. Verify current membership with type owner before preparation.
   Routing supplies neither evidence nor application approval.
3. Inspect target headers, shared declarations and PsyQ definitions before new
   alias/aggregate/field/prototype. Reuse established types. Prove widths, signedness,
   pointer depth, alignment, offsets and ABI from original accesses plus callers/
   consumers. Indexed candidates, single casts and analyzer guesses are leads only.
4. Apply only explicitly authorized, reviewed candidates through owning transaction.
   Preserve exact ownership, consumer scope, adopted dirty baseline, independent
   native receipts, rollback and parent acceptance. Shared promotion requires
   independently accepted private proofs; one compile cannot qualify.
5. Report application evidence and unresolved gaps separately. Never edit generated
   bindings or silently recover index. `type TARGET OLD -> NEW` remains identity
   maintenance through [naming transaction](../bof3-naming/references/identity-transactions.md),
   not representation authority.

## Commands and references

Use `bin/harness types` or `sh .pi/skills/bof3-types/scripts/audit.sh` with unchanged
arguments. Wrapper adds no policy. Before account, prepare/run, review or revalidation,
read corresponding branch in [type application](references/type-application.md)
and command `--help`. Before native gates read
[execution contract](../bof3-re/references/native-execution.md).

Return tool count, wall time, method and acceptance gaps. Parent archives
measurements; reusable directives stay in skill references, not external docs.
