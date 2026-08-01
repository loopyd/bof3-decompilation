# Sharing non-matches

Review non-exact escalation sharing policy for one selector. This policy does
not authorize publication; the parent owns `bin/scratchpad share`.

A restored partial is shareable when all are true:

1. it begins at a reviewed Splat `c` or `asm` `func_XXXXXXXX` boundary;
2. its restored authored `src/<target>/func_XXXXXXXX.c` exists; and
3. the payload passes local `bin/scratchpad preview SELECTOR` checks.

It is **not shareable** only when a boundary is data-leading/non-function,
unreviewed, mismatched, or its source is absent. Missing ABI, call ownership,
analyzer confidence, Rizin evidence, or a clean-C solution does not make an
otherwise qualifying function unshareable. These are exactly why it can be a
useful public scratch.

For a claimed share failure, verify the exact reason and distinguish payload
context defects from eligibility. A generated source that references a typedef,
extern, or type absent from its preview context is a tooling finding; identify
the missing declaration and require a focused regression test before accepting
the fix. Never change target map/Splat facts merely to make a payload shareable.

Record one of: scratch URL, `not shareable: <layout/source reason>`, or
`publication failure: <error>`. A scratch is public escalation evidence, never
lift acceptance, and a prior URL never replaces a new mission or live exact
check.
