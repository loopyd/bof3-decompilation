---
name: decomp-loop
description: "Lift and match one function from PSX MIPS assembly to C89. Use when the user asks to decompile, reverse, asm-diff, or match a function. Also use for improving match percentage on an already-lifted function."
---

# Decomp Loop

## Quick CLI path (no harness)

```bash
# 1. Get r2 decompilation at target address
r2 -q -c 'pdg @ 0x801ba678' out/binaries/ETC/GAME/0.bin

# 2. Lift via m2c
bin/harness lift func:ETC/GAME#0@0x801ba678
cat output/harness/workspaces/func:ETC:GAME#0@0x801ba678/func.m2c.c

# 3. Write/edit C89 source
#   src/modules/game/func_801ba678.c  +  #include "internal.h"

# 4. Match
bin/asm-diff-one src/modules/game/func_801ba678.c

# 5. Read results
cat out/asm-diff/func_801ba678/summary.json
cat out/asm-diff/func_801ba678/diff.patch
```

## Full harness path

Target selection, claiming, lifecycle, module verification → see `.agents/skills/harness/SKILL.md`.

## Reading asm-diff output

- `summary.json` → `"similarity": 0.95` = 95%
- `diff.patch` → `-` = original, `+` = compiled; common mismatches:
  - register / offset / instruction choice
  - `li` vs `lui+ori`, `move` vs `addiu $zero,`
  - delay-slot NOP placement
  - branch-target label shifts

## When stuck

| Blocked by | Action |
|---|---|
| Unknown instruction | `r2 -q -c 'pdga @ <addr>'` for decompiler asm with regs |
| Match 80–95% | Hand-trace with `r2 -q -c 'pD $(( <end> - <start> )) @ <start>'` |
| Stuck on calling convention | Check compiler flags in `config/splat/*.yaml` `$maspsx_extra` |
| Compiler-inserted NOP | Verify delay slots in original vs compiled |
| Unknown struct/global | Add `extern` to `internal.h` + `SYMBOL_AT` in `symbols.c` |
| Match % won't budge | Try decomp-permuter (Phase 2) for register/instruction variant search |

## Tool chain

| Tool | Role |
|---|---|
| r2 + r2ghidra 6.1.4 | MIPS decompiler (`pdg`/`pdga`) |
| bin/maspsx-cc | PsyQ-compiler with maspsx flag translation |
| splat (config/splat/) | Binary segmentation (6 yamls) |
| m2c | C decompiler (via `harness lift`) |
| bin/asm-diff-one | Compile + nbench + byte diff → summary.json + diff.patch |
| decomp-permuter | Last-mile register/instruction variant search (Phase 2) |
| bin/harness | Target selection, claiming, verified diff, reports |

## Coding conventions

`.agents/rules/decomp.md` (C89, REG32, DAT_xxx, internal.h, SYMBOL_AT)  
`.agents/rules/build.md` (module registration, sources.cmake, targets)  

Do not duplicate convention rules here.
