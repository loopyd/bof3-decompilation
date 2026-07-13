---
name: decomp-loop
description: "Discover, lift, and exactly match PSX MIPS functions or complete overlays in C89. Use for decompilation, reversing, asm-diff work, improving match percentage, recovering function boundaries, or finishing a promoted binary target."
---

# Decomp Loop

Use `bin/harness` as the workflow entry point. Original bytes and canonical Splat
assembly outrank function indexes, decompilers, and source guesses.

For exact-match tactics and compiler pitfalls, read
[`references/matching-patterns.md`](references/matching-patterns.md). Read it
before changing source when control flow already looks correct but the diff is
still nonmatching.

## Function loop

```bash
bin/harness target show <target>
bin/harness next [target]
bin/harness lift <target> <function>
bin/harness diff <source>
```

Before editing C, verify the payload, load address, function range, Splat
configuration, and that the proposed address is code rather than embedded data.
Do not infer a boundary from a lone prologue, `jr ra`, or decompiler label.
`diff` must build CMake's per-source `.obj` target and verify that an edited
source refreshed its object. Treat an empty successful build with a stale
object as a tooling failure, not a comparison result.

## Exact-match order

1. Establish reviewed code/data boundaries and the exact function size.
2. Recover calls, branches, delay slots, and return paths.
3. Match access widths, signedness, constants, and argument promotion.
4. Match expression shape and declaration order for register allocation.
5. Diagnose only the first meaningful mismatch after each compile.
6. Use bounded permutation only after the target, boundary, compiler command,
   and original-byte comparison are proven. It may search for the right size or
   source shape early; prioritize it for the scheduling hard tail.

When a newly lifted function reaches a canonical 100% instruction and byte
match, re-run `bin/harness diff` and commit it immediately in a small focused
commit. Include only its required Splat boundary, declaration, and address
binding; exclude unrelated cleanup and generated evidence. A commit does not
authorize a push.

Do not trade readable, factual C for a percentage increase until the target,
boundary, compiler command, and diff normalization are proven correct.

## Reading asm-diff output

- `summary.json` is the machine-readable result and artifact index.
- `original.s` and `current.s` are the canonical normalized inputs to the diff.
- `compiler.s` preserves the compiler's raw assembly for source-shape analysis.
- `diff.patch` → `-` = original, `+` = compiled; common mismatches:
  - register / offset / instruction choice
  - `li` vs `lui+ori`, `move` vs `addiu $zero,`
  - delay-slot NOP placement
  - branch-target label shifts

## When stuck

| Blocked by | Action |
| --- | --- |
| Wrong size or shifted labels | Recheck function boundaries before editing C |
| Unsupported instruction | Read Splat assembly; use an analyzer as a hint |
| Match 80–95% | Trace the first meaningful mismatch in `out/matching/` |
| Calling convention | Check Splat config and the CMake compile command |
| Compiler-inserted NOP | Verify delay slots in original vs compiled |
| Unresolved symbol | Add a target-local declaration and address binding |
| Score is stuck | Run a bounded permutation; reject false candidates |

## Module completion

A module is complete only when every reviewed code function is C, every data or
rodata range has explicit ownership, and the rebuilt payload matches the
original bytes. Report function matches separately from whole-binary matching.

## Tool chain

| Tool | Role |
| --- | --- |
| bin/cc | Native-style PSX compiler driver with MASPSX translation |
| Splat/spimdisasm | Canonical binary segmentation and assembly |
| m2c | Optional matching-oriented C seed |
| asm-differ | Interactive instruction comparison |
| Rizin/Ghidra | Optional analysis hints |
| decomp-permuter | Optional bounded source-shape search |

## Coding conventions

`.agents/rules/decomp.md` (C89, REG32, DAT_xxx, internal.h, WEAK_SYMBOL_AT)
`.agents/rules/build.md` (module registration, sources.cmake, targets)

Keep authored header guards short and path-scoped (`CORE_EMI_H`), without
a redundant repository-name prefix.

Do not duplicate convention rules here.
