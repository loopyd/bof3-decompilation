---
name: bof3-macros
description: Discover, assess and resolve BOF3 preprocessor macro opportunities during cleanup or source finalization, with pinned ranking and reviewed transactions.
---

# BOF3 macros

Read [MACROS.md](../../../docs/specs/MACROS.md), the sole macro-resolution
specification. Use its relevant command and evidence sections, not a separate
skill-local schema or policy copy.

Invoke `bin/macro-audit` or `sh .codex/skills/bof3-macros/scripts/audit.sh` with
the same arguments. `--help` lists discovery, ranking, inspection, transaction
and acceptance commands; the wrapper does not choose targets or grant writes.

For ranking, obtain the caller's target scope, explicit instruction floor, pool
size and top-N bound. Retain the machine pool pin before assessment; inspect every
selected member and existing definitions, then record honest human-value decisions.
Keep rejection/defer reasons. No worthwhile abstraction is a valid outcome.
Validate and externally pin the ranked report before handing selected leads to
reverse work. Ranking is not semantic acceptance or a durable attempt ledger.

Resolve only authorized candidates through their reviewed owner transactions.
Preserve all consumer, independent-review, native-byte and rollback gates in the
specification. Source lifting/review still follows [BOF3 RE](../bof3-re/SKILL.md).
Do not edit generated files, rebuild the index silently, or rewrite frozen proofs.
Report selected, rejected, deferred and unresolved work separately from accepted
applications; never count a suggestion or passing Python check as source cleanup.
