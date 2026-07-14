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
# Once the lift compiles and its boundaries/structure are credible:
bin/harness permute <source> -j <bounded-jobs>
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
6. Once the lift compiles and its boundaries and rough control flow are
   credible, run a bounded permutation pass. Use its best result as a candidate,
   manually correct factual types/control flow/readability, then permute again
   when expression shape, allocation, or scheduling remains.
7. Run permutation earlier for same-size or >=80% functions, but do not treat
   that threshold as a requirement when a bounded search may find the right
   instruction count or source shape.

The permuter does not guarantee compilable output. First verify that its
generated `base.c` compiles and that it evaluates real candidates. Reject runs
that fail the base compile, report zero compiled candidates, compare against a
current-object copy, or depend on invalid C/ABI guesses. Re-run
`bin/harness diff` on any adopted candidate; the canonical diff remains the
acceptance gate.

Before choosing `-j`, inspect logical cores and the current one-minute load.
Reserve at least `max(4, 25% of logical cores)` for interactive/system work.
Use only the remaining capacity after current load, divide it across all
concurrent permuter agents, and recheck load during long runs. Do not give every
agent the machine-wide worker count. Prefer one well-fed run over several
oversubscribed runs; reduce or defer permutation when the reserved headroom is
already consumed.

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
| Match >=80% or same size | Trace the first mismatch, then run bounded permutation |
| Calling convention | Check Splat config and the CMake compile command |
| Compiler-inserted NOP | Verify delay slots in original vs compiled |
| Unresolved symbol | Add a target-local declaration and address binding |
| Permuter base/candidates fail to compile | Fix the bundle/compiler context; continue manually |
| Score is stuck | Re-run bounded permutation after factual manual fixes |

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
