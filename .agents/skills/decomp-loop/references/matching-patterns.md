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
- Check the consumer after every load: the PSX R3000A exposes a load delay as
  well as branch/jump delay slots. A scheduler mismatch may therefore appear one
  instruction after the load rather than at the source expression.
- Distinguish tail calls from ordinary calls followed by a return.
- Remember that MIPS I has no integer condition-code or carry flag. Comparisons,
  overflow/carry tests, and multiword arithmetic are explicit instructions;
  source reconstructed around an implicit flag model is structurally wrong.
- Treat assembler pseudo-instructions as renderings, not one-instruction facts:
  `li`, `la`, `move`, and relational branches may expand or select different
  real instructions. Compare normalized emitted instructions and bytes.
- For `j`/`jal`, reconstruct the destination from the low 26-bit instruction
  field plus the caller PC region. Do not mistake that encoding for a complete
  absolute address or mask it as an ordinary immediate.

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

## Compiler scheduling limits ("hard tail")

Even when a function is structurally converged (correct CFG, calls, size,
types), the PsyQ/GCC scheduler can reorder argument evaluation, register moves,
and delay-slot fill relative to the original binary. This is a known gcc
register-allocation–bound class of mismatch observed in other PSX matching
decompilations:

- **NFSHS-PSX-decomp** (Caesar0007, splat + maspsx + real PsyQ cc1 2.8.0)
  explicitly documents *"5 near-misses left, all gcc register-allocation /
  induction-variable bound — the hard tail"* — functions that resist 100%
  match even with the original compiler.
  <https://github.com/Caesar0007/NFSHS-PSX-decomp> (README.md §Status,
  METHODOLOGY.md)
- **SOTN decomp** (Xeeynamo) uses per-function `//!` optimization-flag
  annotations and a custom permuter to close scheduling gaps.
  <https://github.com/Xeeynamo/sotn-decomp> (tools/builds/gen.py,
  permuter_settings.us.toml)

Common scheduling symptoms:
- `move $a0,$s0` placed before other argument loads instead of in a `jal`
  delay slot.
- Reordered independent loads (`lui`/`lw`/`lbu`) that are semantically
  identical but produce different instruction sequences.
- Argument-load interleaving that shifts instruction pairing relative to
  branch delay slots.

The available mitigations in ascending cost:
1. Permutation search for a source shape that nudges the scheduler.
2. Per-function compiler flags (e.g. `-O1` for one function).
3. Preserve a compiling factual near-match with its exact residual diff and
   classify it as a scheduler/register-allocation hard tail for later work.

Do not insert handwritten assembly or call a near-match exact. A hard-tail
classification is a progress record, not a relaxation of the acceptance gate.

## Escalation

1. Confirm target, compiler profile, function start, and size.
2. Compare canonical assembly and normalized asm-diff output.
3. Check whether mismatch is a structural issue (wrong CFG, missing call,
   incorrect type/width) or a scheduling-only residue (see above).
4. Use Ghidra, Rizin, or m2c only to test a concrete hypothesis.
5. Resolve symbols and local structures that obscure access shape.
6. Use decomp-permuter only for a structurally converged function.

Reject a source permutation that improves the score by depending on undefined
behavior, false types, misleading names, or unexplained magic addresses.

## Architecture references

- [psx-spx CPU specifications](https://psx-spx.consoledev.net/cpuspecifications/)
  documents the PSX R3000A/MIPS I execution model and delay behavior.
- [MIPS R3000 architecture manual](https://archive.org/details/bitsavers_mipsR3000R_6835608)
  is the instruction-encoding authority; verify pseudo-instruction expansions
  against the assembler actually used by this project.
