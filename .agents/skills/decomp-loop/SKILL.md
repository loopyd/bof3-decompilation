---
name: decomp-loop
description: "Discover, lift, and exactly match PSX MIPS functions or complete overlays in C89. Use for decompilation, reversing, asm-diff work, improving match percentage, recovering function boundaries, or finishing a promoted binary target."
---

# Decomp Loop

Use `bin/harness` for workspace orchestration and `bin/asmdiff` for the focused
matching loop. Original bytes and canonical Splat assembly outrank function
indexes, decompilers, and source guesses.

For exact-match tactics and compiler pitfalls, read
[`references/matching-patterns.md`](references/matching-patterns.md). Read it
before changing source when control flow already looks correct but the diff is
still nonmatching.

Read [psx-mips-correctness.md](references/psx-mips-correctness.md) before
promoting control flow, unaligned access, GP-relative data, DMA/MMIO, cache, or
COP2/GTE behavior.

## Function loop

```bash
bin/harness targets <target>
bin/m2c <source>                       # automated C seed
bin/asmdiff <source>                   # validate
bin/permute <source> --prepare-only    # optional late-stage search
bin/permute <source> -j <bounded-jobs>
```

`bin/permute` is the only supported permuter path. It owns one function
workspace and forwards `-j` once to one upstream decomp-permuter coordinator.
Independent functions may run concurrently, but the wrapper rejects a second
coordinator for the same function workspace. Budget the sum of all active `-j`
worker counts against available capacity.

Use upstream `PERM_*` directives for focused interacting alternatives after
manual factual fixes. Run `--prepare-only`, edit the generated workspace
`base.c`, then run with `--prepared`; an ordinary run regenerates `base.c`.
Remember that any multi-choice directive disables automatic randomization unless
the intended region is wrapped in `PERM_RANDOMIZE`.

The generated `base.c` must be a compilable pruned translation unit: the target
function plus only declarations, types, and macros required by that function.
Generate it with the real target compiler's preprocessing and flags. Keep
`PERM_*` directives in the selected function while the context remains stable.

Before editing C, verify the payload, load address, function range, Splat
configuration, and that the proposed address is code rather than embedded data.
Do not infer a boundary from a lone prologue, `jr ra`, or decompiler label.
`bin/asmdiff` must build Make's per-source `.o` target and verify that an edited
source refreshed its object. Treat an empty successful build with a stale object
as a tooling failure, not a comparison result.

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
`bin/asmdiff` on any adopted candidate; the canonical diff remains the
acceptance gate.

Before choosing `-j`, inspect logical cores and the current one-minute load.
Reserve at least `max(4, 25% of logical cores)` for interactive/system work.
Use only the remaining capacity after current load, divided across all active
function coordinators. Recheck load during long runs. Reduce or defer
permutation when the reserved headroom is already consumed.

Omit `--seed` for independent exploration: `bin/permute` generates and reports a
system-random base seed, then derives per-worker seeds. Supply and record a
specific seed only to reproduce a useful candidate or diagnose a failed run;
an unexplained fixed default repeatedly explores the same search path.

When a newly lifted function reaches a canonical 100% instruction and byte
match, re-run `bin/asmdiff` and prepare a small focused change. Include
only its required Splat boundary, declaration, and address binding; exclude
unrelated cleanup and generated evidence. Commit or push only when explicitly
authorized.

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
| Calling convention | Check Splat config and the Make compile command |
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
| bin/m2c | Automated matching-oriented C seed from Splat assembly |
| bin/cc | Native-style PSX compiler driver with MASPSX translation |
| Splat/spimdisasm | Canonical binary segmentation and assembly |
| asm-differ | Interactive instruction comparison |
| Rizin/radare2 and Ghidra | Optional analysis hints |
| decomp-permuter | Optional bounded source-shape search |

## Coding conventions

`.agents/rules/decomp.md` (C89, REG32, D_xxx, internal.h, WEAK_SYMBOL_AT)
`.agents/rules/build.md` (module ownership, Make targets, matching checks)

Keep authored header guards short and path-scoped (`CORE_EMI_H`), without
a redundant repository-name prefix.

Do not duplicate convention rules here.
