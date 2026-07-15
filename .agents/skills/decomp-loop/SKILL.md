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

## Function discovery and lifting workflow

The correct workflow for new functions discovered by rizin:

```bash
# 1. Discover candidates across all targets
bin/harness reverse --all --strategy leaf --functions 10

# 2. Analyze a single target to classify all functions
bin/harness analyze --target emi/etc/game/00

# 3. Bulk-populate Splat: add all missing functions as asm subsegments
#    Compute file offsets: file_offset = code_start + (vram - vram_base)
#    Insert into config/splat/...yaml in ascending offset order

# 4. Regenerate Splat assembly
bin/splat split config/splat/<target>.yaml

# 5. For each function — the controlled loop:
#    m2c seed → manual cleanup → asmdiff → permute → fix → asmdiff
```

**Do NOT skip Splat.** The correct path for a new function:

1. Rizin/radare2 identifies a candidate address and size
2. Add the function to the Splat config (`config/splat/...yaml`) as a code subsegment
3. Run `bin/harness split <target>` to regenerate Splat assembly
4. Run `bin/m2c` on the new assembly to get a C seed
5. Refine with `bin/asmdiff` as the acceptance gate

Functions detected by rizin but not in Splat are **false positives** if they
fall outside code ranges. The `analyze` and `reverse --all` commands filter
these automatically using `_get_code_ranges()`.

## Function loop

The controlled lifting loop — one function at a time, no heavy automation:

```bash
# 1. Get the m2c seed from Splat assembly
bin/m2c <source>                       # generates C from ASM

# 2. Clean up the seed manually (C89, project conventions, internal.h types)
#    - Replace M2C_FIELD with struct member access
#    - Use SCRATCH_WORK, GLOBAL_WORK_PTR macros where applicable
#    - Declare vars at top of function
#    - Add @behavior / @source trace comment

# 3. Check the match
bin/asmdiff <source>                   # validate — this is the acceptance gate

# 4. If match is incomplete, run permuter with safe defaults
bin/permute <source> -j <bounded-jobs> # bounded source-shape search

# 5. Adopt the best candidate, fix factual issues, repeat from step 3
```

**Do not use `bin/harness reverse <target>@<addr> --run` for bulk lifting.**
The `--run` flag launches a full AI mission — too slow and heavy for routine
work. Use it only for targeted exploration when manual analysis is stuck.

**Do not skip Splat.** The correct path for a new function:

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
| bin/asmdiff | Validate match against canonical assembly (acceptance gate) |
| bin/permute | Bounded source-shape search (decomp-permuter wrapper) |
| Splat/spimdisasm | Canonical binary segmentation and assembly |
| bin/harness reverse | Discover and rank function candidates across targets |
| bin/harness analyze | Mass-analyze all targets, classify functions, write report |
| bin/harness normalize | Materialize binaries for all promoted targets |
| Rizin/radare2 and Ghidra | Optional analysis hints (not for bulk lifting) |

## Coding conventions

`.agents/rules/decomp.md` (C89, REG32, D_xxx, internal.h, WEAK_SYMBOL_AT)
`.agents/rules/build.md` (module ownership, Make targets, matching checks)

Keep authored header guards short and path-scoped (`CORE_EMI_H`), without
a redundant repository-name prefix.

Do not duplicate convention rules here.
