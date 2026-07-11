---
name: decomp-loop
description: "Discover, lift, and exactly match PSX MIPS functions or complete overlays in C89. Use for decompilation, reversing, asm-diff work, improving match percentage, recovering function boundaries, or finishing a promoted binary target."
---

# Decomp Loop

Use `bin/rebof3` as the workflow entry point. Original bytes and canonical Splat
assembly outrank function indexes, decompilers, and source guesses.

For exact-match tactics and compiler pitfalls, read
[`references/matching-patterns.md`](references/matching-patterns.md). Read it
before changing source when control flow already looks correct but the diff is
still nonmatching.

## Function loop

```bash
bin/rebof3 inspect <target>
bin/rebof3 next [target]
bin/rebof3 lift <target@address>
bin/rebof3 diff <source>
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
6. Use permutation search only after size, CFG, calls, and memory accesses agree.

Do not trade readable, factual C for a percentage increase until the target,
boundary, compiler profile, and diff normalization are proven correct.

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
|---|---|
| Wrong size or shifted labels | Recheck function boundaries before editing C |
| Unsupported instruction | Read canonical Splat assembly; use Rizin or Ghidra as an optional hint |
| Match 80–95% | Trace the first meaningful mismatch in `out/asm-diff/` |
| Stuck on calling convention | Check the target Splat config and `capcom97-bof3` compiler profile |
| Compiler-inserted NOP | Verify delay slots in original vs compiled |
| Unresolved struct/global | Add `extern` to `internal.h` + `SYMBOL_AT` in `symbols.c` |
| Match % won't budge | Use decomp-permuter only after size, CFG, and calls converge |

## Module completion

A module is complete only when every reviewed code function is C, every data or
rodata range has explicit ownership, and the rebuilt payload matches the
original bytes. Report function matches separately from whole-binary matching.

## Tool chain

| Tool | Role |
|---|---|
| bin/maspsx-cc | PsyQ-compiler with maspsx flag translation |
| Splat/spimdisasm | Canonical binary segmentation and assembly |
| m2c | Optional matching-oriented C seed |
| asm-differ | Interactive instruction comparison |
| Rizin/Ghidra | Optional analysis hints |
| decomp-permuter | Optional late-stage source search |

## Coding conventions

`.agents/rules/decomp.md` (C89, REG32, DAT_xxx, internal.h, SYMBOL_AT)  
`.agents/rules/build.md` (module registration, sources.cmake, targets)  

Do not duplicate convention rules here.
