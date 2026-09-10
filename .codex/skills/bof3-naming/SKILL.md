---
name: bof3-naming
description: Discover BOF3 symbol naming opportunities, gather and audit naming evidence, and apply explicitly reviewed identity transactions. Keep opportunity, audit and editing modes separate; exclude semantic lift rewrites and type representation changes.
---

# BOF3 naming

One skill owns the naming lifecycle; the caller's canonical mode determines
authority. Never infer missing inputs, switch modes or treat evidence as edit
permission. Preserve behavior, bytes, ABI, addresses, boundaries, compiler
settings, partial-lift metadata and unrelated dirty work.

| Mode | Contract |
|---|---|
| `naming-opportunity TARGET ID` | Read-only [opportunity assessment](../../../docs/agents/tool-usage.md#symbol-naming-opportunities); no source, map or report edits |
| `audit-target TARGET` | [Naming audit v3](references/NAMING_AUDIT_V3.md): target/report binding, bounded evidence collection and full-report closure; no identity edits |
| `symbol`, `type`, `repair`, `retained-lift` | [Identity transactions](references/IDENTITY_TRANSACTIONS.md): separately approved scope, validation and rollback |
| `retained-lift` cosmetics | Also [byte-safe cosmetics](references/BYTE_SAFE_COSMETICS.md); no semantic lift rewrites |
| `relocate-batch` | [Source relocation](references/SOURCE_RELOCATION.md): explicitly approved atomic ownership/build-graph updates |

Use `bin/agent-context cleanup CANONICAL_REQUEST...` for route-specific prefills.
Load only the selected mode's references; links to other modes are handoffs, not
authority. Opportunity assessment retains the caller's exact target/ID/fingerprint
and verifies it with `bin/naming-audit describe-opportunity TARGET ID
--expected-fingerprint PIN`. Without a caller pin, inspect only for scouting.
Raw spellings are leads, not semantic or storage proof. Justify proposals from
behavior, callers and consumers; prefer explicit gaps to invented names.

Never silently replace a frozen lead/report, expand a queue, refresh an index or
reset budgets. A selected-row result does not replace successful unfiltered
full-report `complete:true` validation or independent terminal acceptance.
Type-spelling identity preserves representation; layout and C representation
decisions belong to `$bof3-types`. Macro extraction belongs to `$bof3-macros`.

`sh .codex/skills/bof3-naming/scripts/audit.sh ...` dispatches unchanged arguments
to `bin/naming-audit`; `scripts/evidence.sh` dispatches to `bin/naming-evidence-run`.
The harness owners retain parsing, evidence, transactions and acceptance policy.
