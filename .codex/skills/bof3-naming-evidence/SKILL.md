---
name: bof3-naming-evidence
description: Prepares and validates BOF3 naming-audit/v3 readiness, evidence, receipts, corroboration, and closure. Use for only an explicit audit-target naming-evidence request; exclude ordinary lift, match, review, relocation, cosmetic editing, and general analysis tasks.
---

# BOF3 naming evidence

Read-only except caller-authorized disposable reports; never edit names, annotations, docs, locations, or lifts.

Invoke `bin/naming-audit` or `sh .codex/skills/bof3-naming-evidence/scripts/audit.sh`
with identical arguments. `scripts/evidence.sh` similarly dispatches to
`bin/naming-evidence-run`; it never chooses or substitutes the frozen target/report.

## Run contract

Each `audit-target TARGET` run owns exactly one target report and refreshes it in place; it never batches multiple targets. Run `bin/naming-evidence-run TARGET REPORT` with both tokens copied exactly from the frozen cleanup request. Never construct, guess, search for, substitute, or accept a caller-supplied report path. Missing/noncanonical report state blocks with `bin/naming-audit init-all out/reviews/plan-audit-naming`; never create or choose another report. Convert an initializer row only to a receipt-backed `exhausted` or `proposed` conclusion. Return proposed identities to the parent for separate identity-maintenance application; this audit emits evidence and readiness, not edits.

One invocation processes at most 10 rows within a 600-second shard wall and
120-second native-operation deadline. Resume with the same canonical command;
completed checkpoint rows must not replay. Keep tool output at most 8 KiB and
cite report/receipt paths with SHA-256 rather than pasting report bodies. A shard
or query success is not target completion: run `bin/naming-audit validate TARGET
REPORT` without a transaction filter and require successful canonical full-report
validation with `complete:true`. When collection/import used an explicit evidence
root, pass the same canonical absolute `--evidence-root PATH` to validation;
never substitute a root or rewrite receipts to bypass provenance checks.
Incomplete or blocked rows remain explicit gaps;
never mark the audit complete merely because its runner exited successfully.

- Evidence/schema/inventory: [Naming audit v3](references/NAMING_AUDIT_V3.md). Preserve order; stale/incomplete/uncorroborated blocks.
- Application owner: [Identity transactions](../bof3-identity-maintenance/references/IDENTITY_TRANSACTIONS.md).
- Exact/partial spelling semantics: [Byte-safe cosmetics](../bof3-identity-maintenance/references/BYTE_SAFE_COSMETICS.md).

For an existing canonical report missing newly introduced inventory rows, preview
`bin/naming-audit reconcile TARGET` (with the same `--evidence-root PATH` if
retained receipts use an explicit root). With explicit reconciliation authority, use
`--apply` to append only missing blocked rows after full validation. Existing rows
and other reports are retained; duplicate/extra rows or invalid evidence block.
This does not complete the audit or authorize identity application. Never use
`init-all` as reconciliation: it replaces the entire report set.

Links define handoffs, not edit permission.

## Terminal accounting

Structural `validate`/capability exhaustion is not independent semantic acceptance.
Autonomous current no-op/exhaustion must pass the read-only parent-pinned
[`terminal-verify` contract](../../../docs/usage.md#naming-terminal-acceptance-read-only)
with distinct preparation/reviewer/parent runs, exact current binding and retained
ceiling evidence, no unresolved executable leads. It neither rewrites historical
reports nor authorizes identities. Selected-row acceptance does not replace the
separate successful full-report `complete:true` gate above.
