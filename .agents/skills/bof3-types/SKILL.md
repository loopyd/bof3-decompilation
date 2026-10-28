---
name: bof3-types
description: Identify and refine BOF3 types, fields, layouts and function prototypes from original accesses and consumers. Inspect current declarations and uses, then apply evidence-backed representation edits directly within the authorized scope.
---

# BOF3 types

1. Resolve the explicit target and selected functions. Read the
   [target contract](../bof3-re/references/target-contract.md), then inspect
   `bin/harness types list TARGET --json`. Use `--kind uses`, `storage` or `all`
   for other facts. Missing or stale evidence blocks the affected conclusion.
2. Before proposing a representation, read [type refinement](references/type-application.md).
   Reuse target, shared and PsyQ declarations. Prove layout and ABI from original
   instructions and all consumers; indexed storage leads and casts are insufficient.
3. For authorized edits, read [editing and recovery](../bof3-re/references/editing-recovery.md).
   Capture owned PRE, current exact/partial measurements and complete consumers.
   Edit declarations and affected C directly; serialize shared writes.
4. Run `bin/harness types check TARGET [SELECTOR ...] --json`. Inspect actual
   consumer coverage, layout conflicts, unknowns and native results. Check shared
   declarations under every owning target and obtain independent semantic review.
5. Report accepted refinements, measured preservation, unresolved layouts and
   unfinished campaign scope. Selected success does not complete a target.

Spelling-only changes use `$bof3-naming`; macro extraction uses `$bof3-macros`.
Record successful matches through `$bof3-re` after the current live byte gate.
