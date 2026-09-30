---
name: bof3-macros
description: Discover, rank and resolve BOF3 macro opportunities during cleanup or source finalization. Requires pinned ranking, human-value assessment and reviewed transactions.
---

# BOF3 macros

Read [discovery and ranking](references/discovery-ranking.md) before selecting leads.
Before mutation read [application](references/application.md) and
[shared lifecycle](../bof3-types/references/transaction-lifecycle.md).
Existing suitable abstraction uses [unchanged assessment](references/existing-abstractions.md),
not manufactured extraction. Required knowledge lives in these references.

## Protocol

1. **Bind lead.** `bin/harness agent context cleanup macro-opportunity TARGET ID`.
   Preserve emitted target/ID and caller pool/ranking pins. Verify current membership
   with macro owner. Routing authorizes neither transaction nor wider search.
2. **Rank.** Obtain caller scope, explicit instruction floor, pool size and top-N.
   Retain machine pool pin before assessment. Inspect every selected member and
   existing definitions; record human-value decisions and rejection/defer reasons.
   No worthwhile abstraction is valid outcome.
3. **Pin.** Validate and externally pin ranked report before handing leads to reverse
   work. Ranking grants no semantic acceptance or durable attempt ledger.
4. **Resolve.** Authorized candidates only, through reviewed owner transactions.
   Preserve every consumer, independent-review, native-byte and rollback gate.
   Lifting/review also follows [BOF3 RE](../bof3-re/SKILL.md). No generated-file edits
   or frozen-proof rewrites.
5. **Account.** Separate selected/rejected/deferred/unresolved work from accepted
   applications. Suggestions and Python checks never count as source cleanup.

## Commands and authority

`bin/harness macros` or `sh .pi/skills/bof3-macros/scripts/audit.sh` accepts unchanged
arguments. `--help` lists discovery, ranking, inspection, transaction and acceptance
commands. Wrapper neither chooses targets nor grants writes.

Before native gates read [execution and authority](../bof3-re/references/native-execution.md).
Parent refreshes/reviews need no repeated conversational approval; sandbox and
original bounds still bind. Never refresh during pinned transaction.

Return tool count, wall time, method, utility/rejection decisions and validation.
Parent archives measurements; reusable directives belong in skill references.
