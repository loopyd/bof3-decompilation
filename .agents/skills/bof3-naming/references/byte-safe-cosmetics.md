# Byte-safe cosmetics

Cosmetic, evidence-preserving edits only. Lift-body edit requires post-cleanup live
`bin/harness lift byte-match TARGET@0xADDRESS`; failure reverts, never fixes forward.
Apply ladder top-down; stop at first rung resolving drift.

## Metadata preflight and authority

Before any rung, require parsable function-level `@behavior` and address-authoritative `@source`; a missing tag fails preflight. Preserve `@behavior`, `@source`, `@kind`, `INFERRED:`, and evidence comments; correct stale tags in place, never infer identity from filename or directory ancestry. Naming evidence and eligibility come from [Naming audit v3](naming-audit-v3.md); atomic application, map/binding authority, and rollback come from [Identity transactions](identity-transactions.md); file moves come only from [Source relocation](source-relocation.md). Macro opportunity discovery, ranking, definition and extraction belong to [`$bof3-macros`](../../bof3-macros/SKILL.md); this ladder only removes a provably dead macro. Type layout and C-representation decisions belong to [`$bof3-types`](../../bof3-types/SKILL.md).

## Cosmetic ladder

[Metadata preflight](#metadata-preflight-and-authority) mandatory before every rung.
Rungs 1–3: diff hygiene. Rungs 4–6: every affected selector needs live
`bin/harness lift asm-diff TARGET@0xADDRESS --detail normal` with no first difference
and `bin/harness lift byte-match TARGET@0xADDRESS` exit 0, except bounded retained-partial
spelling case below.

1. Comment/documentation wording, stale claims, and dead links. Never delete `INFERRED:`, evidence, `@behavior`, or `@source`; correct stale tags in place.
2. Whitespace/format accepted by `git diff --check`; no line joins or splits inside declarations or macros.
3. File-scope dead macros/extern declarations only after grep proves zero users. Never remove a `WEAK_SYMBOL_AT` binding or map entry.

### Spelling transaction rung

4. Evidence-approved spelling transactions keep address, width, signedness, volatility, layout, ABI, boundary, compiler settings, and body unchanged. Update map, declaration, binding, and same-target references atomically. A `func_` function/file rename additionally requires existing `@behavior`/`@source` metadata and updates the Splat label in the same transaction. Retained partials preserve `@status partial`, `@match`, and `@residual` exactly.
5. Consolidate a duplicate declaration into the owning `internal.h` only when forms are token-identical; qualifier differences are behavior.
6. Remove a macro only after grep proves zero users; never simplify a surviving macro body.

## Never safe as cleanup

- Loop, branch, statement, declaration, or initializer rewrites; type modernization; shared-body extraction; volatile/signedness changes.
- Address-named function/file renames outside the [spelling transaction rung](#spelling-transaction-rung); target moves outside relocation-batch; Splat boundaries, SDK maps, shared headers, or compiler flags.
- Style-only edits. Style drift in a byte-matched lift is matching evidence.

## Naming and validation

Use types `s8/u8/s16/u16/s32/u32/f32` ([`include/base/types.h`](../../../../include/base/types.h)); camelCase locals/fields, PascalCase types, upper snake case macros; retain address-canonical names until evidence passes. Naming style and data `@source`/`@kind` tag rules follow [naming and origin rules](naming-style.md). Use hex addresses/offsets, decimal timers/sizes, braces, and functions in assembly order (Splat-owned).

Run `bin/harness source symbols normalize TARGET --write`, `bin/harness source symbols check TARGET`, `bin/harness source splat TARGET`, `bin/harness build TARGET`, `git diff --check`, and `git diff --cached --quiet` as applicable. Regenerate source-path build metadata through `bin/harness build`, never hand-edit `build/`; prove the old path absent and manifest source present.

- **Exact lift:** preflight exactness, then post-edit live `bin/harness lift asm-diff TARGET@0xADDRESS --detail normal` must have no first difference and `bin/harness lift byte-match TARGET@0xADDRESS` must exit 0.
- **Retained partial:** capture live pre-edit status, percentage, compared/matched byte sizes, first mismatch, `@status partial`, `@match`, `@residual`, and body. Pre-edit `byte-match` is expected to exit nonzero. Post-edit `asm-diff`/`byte-match` may remain nonzero but must report exactly unchanged metrics and first mismatch; body and all three metadata fields remain byte-for-byte preserved. Any improvement, regression, exact transition, body edit, or metadata canonicalization is not cosmetics and must be rejected from this transaction.

Docs resolve links and grep stale claims; audit never mutates. Retained partial
is bounded spelling-only exception, proved by unchanged PRE/POST rather than exit 0.
