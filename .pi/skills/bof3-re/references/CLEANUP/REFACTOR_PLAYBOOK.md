# Refactor playbook — byte-match-safe cleanup ladder

Cleanup edits are cosmetic and evidence-preserving only. Any edit that
touches a lift body requires a post-cleanup live `bin/byte-match
TARGET@0xADDRESS` before handoff; on failure revert, never fix forward.
Apply the ladder top down; stop at the first rung that resolves the drift.

## Safe ladder (no re-validation beyond diff hygiene)

1. **Comment/doc text** — wording, stale claims, dead links. Never delete a
   `MATCHING_AID`, `INFERRED:`, or evidence comment; never drop
   `@behavior`/`@source` metadata — correct a stale tag in place (preflight
   fails the lift without both).
2. **Whitespace/format** that `git diff --check` accepts — no line
   joins/splits inside declarations or macros.
3. **Dead code removal at file scope only when grep proves zero users** —
   macros and extern declarations. Never a `WEAK_SYMBOL_AT` binding or map
   entry.

## Guarded ladder (each rung needs live `bin/asm-diff TARGET@0xADDRESS --detail normal` with no first-difference, then `bin/byte-match TARGET@0xADDRESS` exit 0, per affected selector)

4. **Spelling transactions** — rename a target-local symbol/field with
   unchanged address, width, signedness, volatility, and layout; update map,
   declaration, binding, and same-target references together (RULES.md
   evidence gate). A `func_` file/function rename additionally requires the
   file's `@behavior`/`@source` metadata and the Splat label in the same
   transaction.
5. **Declaration consolidation** — merge a duplicate declaration into the
   owning `internal.h` only when the forms are token-identical. A qualifier
   difference is behavior, not drift.
6. **Macro hygiene** — remove a macro only after grep proves zero users;
   never "simplify" a surviving macro body.
## Never safe as cleanup

- Loop/branch/statement rewrites, type "modernization", extracting shared
  bodies, reordering declarations or initializers, changing `volatile` or
  signedness "for consistency".
- Renaming `func_XXXXXXXX` files or functions outside a rung-4 spelling
  transaction, moving files/targets, touching
  Splat boundaries, SDK maps, shared headers, compiler flags.
- Any edit justified by style alone. Style drift in a byte-matched lift is
  matching evidence, not a defect.

## Naming standards (proven renames only)

Types `s8/u8/s16/u16/s32/u32/f32` (`include/base/types.h`); locals/struct
fields camelCase; types PascalCase; macros SCREAMING_SNAKE_CASE.
Address-canonical names stay until the evidence gate passes. Naming style +
data `@source`/`@kind` tag rules: docs/agents/lessons.md (binding standard).
Addresses/offsets in hex; timers/sizes decimal. Always `{}`; functions in
assembly order (Splat-owned).

## Validation

Naming: `bin/symbols normalize TARGET --write`, `bin/symbols check TARGET`,
`bin/splat TARGET`, `bin/build TARGET`, `git diff --check`,
`git diff --cached --quiet`. Lift bodies: live `bin/asm-diff TARGET@0xADDRESS
--detail normal` (no first-difference) plus `bin/byte-match TARGET@0xADDRESS`
per edited selector. Docs: resolve changed links, grep the stale claim,
focused docs tests. Audit: no mutation.