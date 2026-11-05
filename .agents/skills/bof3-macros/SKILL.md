---
name: bof3-macros
description: Find and apply useful BOF3 macro extractions, review existing abstractions, or resolve cleanup leads with direct edits and complete native consumer checks.
---

# BOF3 macros

1. Bind the caller's target, functions, paths and mission bounds. Inspect current
   `bin/harness macros list TARGET` facts and existing definitions. Read
   [selection](references/selection.md) before choosing an extraction.
2. Explain the useful operation, meaningful parameters and clearer call sites.
   Inspect every selected member. Retain rejection, deferral and failed-attempt
   reasons in the owning task. A justified no-op is a valid result.
3. Before edits, read [application](references/application.md) and the shared
   [editing and recovery](../bof3-re/references/editing-recovery.md) rules.
   Edit authorized source, headers and manifests directly. The harness only
   lists facts and checks mechanical results.
4. Review parameter mapping, every use site and all semantic hazards. Run
   `bin/harness macros check TARGET SELECTOR...` for the mechanical facts:
   definition diagnostics, parameter shape, consumer identity and which other
   targets consume a definition this one owns. It deliberately compiles nothing,
   so byte evidence comes from one `bin/harness lift gate` per affected target
   covering every affected selector. Obtain independent review. Missing coverage
   blocks acceptance.
5. Report selected scope, edits or no-op rationale, actual checks, review and
   remaining obligations. Preserve frozen campaign membership and attempt limits.
   A bounded extraction does not close a campaign.

Use [existing abstractions](references/existing-abstractions.md) when the useful
macro already exists. Route source grouping to its owning skill and lifting to
[BOF3 RE](../bof3-re/SKILL.md). Before native execution, read
[native execution](../bof3-re/references/native-execution.md).
