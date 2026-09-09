# Sharing non-matches

Non-exact escalation sharing policy for one selector. This does not authorize publication; the parent owns `bin/scratchpad share`.

| Decision | Conditions |
|---|---|
| Shareable restored partial | Reviewed Splat `c` or `asm` function boundary; restored authored metadata-resolved lift source exists; payload passes local `bin/scratchpad preview SELECTOR` checks. |
| Not shareable | Boundary is data-leading/non-function, unreviewed, or mismatched; or source is absent. |

Missing ABI, call ownership, analyzer confidence, Rizin evidence, or a clean-C solution does not make an otherwise qualifying function unshareable; a public scratch explores those gaps.

For a claimed failure, verify the exact reason; distinguish payload context defects from eligibility. Generated source referencing a typedef, extern, or type absent from preview context is a tooling finding: identify the missing declaration and require a focused regression test before accepting the fix. Never change target map/Splat facts to make a payload shareable.

Record one: scratch URL; `not shareable: <layout/source reason>`; `publication failure: <error>`. A scratch is public escalation evidence, never lift acceptance; a prior URL never replaces a new mission or live exact check.
