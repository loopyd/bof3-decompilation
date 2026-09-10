---
name: bof3-types
description: Inspect BOF3 type declarations, uses and representation opportunities, then prepare evidence-backed type transactions for cleanup and finalization.
---

# BOF3 types

Start at [tool usage](../../../docs/agents/tool-usage.md#3b-target-analysis-freshness--rebuild--query)
for type queries and [parent acceptance](../../../docs/agents/tool-usage.md#type-parent-review-and-final-verification)
for the application lifecycle. [Harness ownership](../../../docs/agents/harness.md)
separates type representation from symbol naming and macro extraction.

`bin/agent-context cleanup type-opportunity TARGET ID` routes one supplied lead.
Keep the emitted target and opaque ID; verify current target membership through
the type owner before preparing work. Routing supplies neither candidate evidence
nor application approval. `type TARGET OLD -> NEW` remains identity maintenance.

Inspect existing target headers, shared declarations and PsyQ definitions before
proposing a new alias, aggregate, field or prototype. Prefer established types;
justify widths, signedness, pointer depth, alignment, offsets and ABI from original
accesses and corroborating callers/consumers. An indexed candidate is only a lead.
Do not infer layouts or names from one cast or an analyzer guess.

Invoke `bin/type-audit` or `sh .codex/skills/bof3-types/scripts/audit.sh` with
the same arguments. Use `--help` and the owning usage sections for account,
prepare/run, review and revalidation commands. The script adds no execution policy.

Apply only explicitly authorized, reviewed candidates. Keep exact target ownership,
affected-consumer scope, adopted dirty baseline, independent native receipts,
rollback and parent acceptance. Shared promotion needs independently accepted
private proofs; one successful compile is insufficient. Never hand-edit generated
bindings or recover the index silently. Report unresolved evidence as a gap.
Symbol identity changes remain a separate explicitly routed
[naming identity transaction](../bof3-naming/references/IDENTITY_TRANSACTIONS.md).
