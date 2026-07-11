---
name: decomp-loop
description: "Lift and match one function from PSX MIPS assembly to C89. Use when the user asks to decompile, reverse, asm-diff, or match a function. Also use for improving match percentage on an already-lifted function."
---

# Decomp Loop

Use `bin/rebof3` as the only workflow entry point. Original bytes and canonical
Splat assembly outrank decompiler output.

## Function loop

```bash
bin/rebof3 inspect <target>
bin/rebof3 next [target]
bin/rebof3 lift <target@address>
bin/rebof3 diff <source>
```

Before editing C, verify the payload, load address, function range, and Splat
configuration reported by `inspect`.

## Reading asm-diff output

- `summary.json` records the instruction match percentage and first mismatch.
- `diff.patch` → `-` = original, `+` = compiled; common mismatches:
  - register / offset / instruction choice
  - `li` vs `lui+ori`, `move` vs `addiu $zero,`
  - delay-slot NOP placement
  - branch-target label shifts

## When stuck

| Blocked by | Action |
|---|---|
| Unsupported instruction | Read canonical Splat assembly; use Rizin or Ghidra as an optional hint |
| Match 80–95% | Trace the first meaningful mismatch in `out/asm-diff/` |
| Stuck on calling convention | Check the target Splat config and `capcom97-bof3` compiler profile |
| Compiler-inserted NOP | Verify delay slots in original vs compiled |
| Unresolved struct/global | Add `extern` to `internal.h` + `SYMBOL_AT` in `symbols.c` |
| Match % won't budge | Use decomp-permuter only after size, CFG, and calls converge |

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
