# Sharing non-matches

Non-exact escalation sharing policy for one selector. This policy does not
authorize publication; the parent owns `bin/scratchpad share`.

A restored partial is shareable when all hold:

1. it begins at a reviewed Splat `c` or `asm` function boundary;
2. its restored authored metadata-resolved lift source exists;
3. the payload passes local `bin/scratchpad preview SELECTOR` checks.

**Not shareable** only when a boundary is data-leading/non-function,
unreviewed, mismatched, or its source is absent. Missing ABI, call ownership,
analyzer confidence, Rizin evidence, or a clean-C solution does not make an
otherwise qualifying function unshareable — these are exactly why it can be a
useful public scratch.

For a claimed share failure, verify the exact reason; distinguish payload
context defects from eligibility. A generated source referencing a typedef,
extern, or type absent from its preview context is a tooling finding: identify
the missing declaration, require a focused regression test before accepting
the fix. Never change target map/Splat facts to make a payload shareable.

Record one of: scratch URL, `not shareable: <layout/source reason>`, or
`publication failure: <error>`. A scratch is public escalation evidence, never
lift acceptance; a prior URL never replaces a new mission or live exact check.
