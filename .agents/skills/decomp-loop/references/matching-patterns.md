# PSX/PsyQ Exact-Match Patterns

Use this reference after validating the target and function boundary. Change
one source property at a time and compare the first meaningful mismatch.

## Boundary and control flow

- Treat direct call targets, branches, jump tables, and reviewed function-index
  ranges as evidence. A prologue or `jr ra` alone is not a boundary.
- Preserve early returns, fallthrough, loop direction, and branch inversion.
  Semantically equivalent control flow often schedules and allocates registers
  differently.
- Check the instruction in every branch and call delay slot. A moved assignment
  can explain a distant-looking mismatch.
- Distinguish tail calls from ordinary calls followed by a return.

## Types and memory

- Match `lb/lbu/lh/lhu/lw` with signedness and exact-width types before tuning
  expressions. Account for C integer promotion explicitly when needed.
- Preserve `volatile` at the accessed object, not as a blanket workaround.
- Recover struct field offsets only when repeated accesses support the layout;
  otherwise keep narrow target-local declarations.
- Check pointer arithmetic scaling. Byte offsets expressed through the wrong
  pointer type change both constants and instruction selection.

## Source shape and registers

- Declaration order, temporary lifetime, assignment order, and nested versus
  flattened expressions affect the PsyQ compiler's register allocation.
- Try direct expressions, named temporaries, compound assignments, and reordered
  independent statements only when each version remains readable and factual.
- Match constant construction: signed immediates, unsigned masks, address
  materialization, and `enum`/macro types can change `li`, `lui`, and `ori`.
- Preserve argument evaluation and promotion at call sites, especially `s8`,
  `u8`, `s16`, and variadic arguments.

## Data, macros, and library calls

- Separate code from strings, jump tables, lookup tables, alignment, and
  padding in Splat before attempting whole-binary matching.
- Use verified PsyQ declarations and SDK macros. Do not lift library code.
- Inspect macro expansion when GPU/GTE helpers emit unexpected stores or
  register use; prefer the period-correct SDK form when it matches behavior.
- Keep opaque data binary-backed until its type and ownership are evidenced.

## Escalation

1. Confirm target, compiler profile, function start, and size.
2. Compare canonical assembly and normalized asm-diff output.
3. Use Ghidra, Rizin, or m2c only to test a concrete hypothesis.
4. Resolve symbols and local structures that obscure access shape.
5. Use decomp-permuter only for a structurally converged function.

Reject a source permutation that improves the score by depending on undefined
behavior, false types, misleading names, or unexplained magic addresses.
