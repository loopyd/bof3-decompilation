---
name: bof3-naming
description: Investigate BOF3 naming debt and edit evidence-backed symbols, source filenames, arguments and locals. Use for naming, identity and source organization; representation and extraction belong to types and macros.
---

# BOF3 naming

Resolve the explicit target and authorized scope. Read the
[target contract](../bof3-re/references/target-contract.md) and, before edits,
[editing and recovery](../bof3-re/references/editing-recovery.md).

1. Inspect `bin/harness naming list TARGET --json`. Select `--kind symbols`,
   `identifiers`, `references`, `files` or `all` as needed. Read
   [investigation](references/opportunities.md) before proposing a name.
2. Prove function behavior or data ownership from current original bytes,
   reviewed boundaries and callers/consumers. Read [naming style](references/naming-style.md).
   Raw names and lexical references are leads; unresolved roles retain their names.
3. For authorized edits, follow [direct editing](references/direct-editing.md).
   Argument/local edits also use [source identifiers](references/source-identifiers.md);
   path changes use [source relocation](references/source-relocation.md).
   Retained-lift cosmetics use [byte-safe cosmetics](references/byte-safe-cosmetics.md).
4. Run `bin/harness naming check TARGET [SELECTOR ...] --json`, inspect its actual
   consumer scope and resolve unavailable checks. Follow the shared native and
   independent-review gates before accepting edits.
5. Report changed identities, evidence, checked consumers and unresolved debt.
   A selected result closes only that scope. Full-target completion requires an
   untruncated inventory and a disposition for every row in the authorized campaign.

Edit source, declarations, maps and ownership files directly within scope. Keep
frozen campaign inputs and unfinished evidence/recovery duties. The harness lists
facts and checks consistency; the skill decides semantic names and applies edits.
Route representation/layout changes to `$bof3-types`, extraction to `$bof3-macros`,
and successful-match records to the metadata branch of `$bof3-re`.
