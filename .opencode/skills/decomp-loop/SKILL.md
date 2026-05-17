---
name: decomp-loop
description: Lift and match one function from PSX MIPS assembly to C89. Use when the user wants to decompile, reverse, asm-diff, or match a function. Also use for improving match percentage on an already-lifted function.
---

## Loop
1. Pick a function to lift or improve.
2. If unlifted: `bin/harness lift <target-id>` to get m2c draft at `out/harness/workspaces/<target>/func.m2c.c`.
3. Write/edit a C89 source at `bof3/src/modules/<mod>/func_XXXXXXXX.c` with `#include "internal.h"`.
4. Run `bin/asm-diff-one bof3/src/modules/<mod>/func_XXXXXXXX.c` to compile and diff.
5. Read `out/asm-diff/func_XXXXXXXX/summary.json` for match%.
6. Read `out/asm-diff/func_XXXXXXXX/diff.patch` for instruction-level mismatches.
7. Iterate until readable match. Add structs/typedefs to `internal.h` as you go.

## Style
- C89: `/* */` comments, declare vars at top.
- `REG32()`, `REG16()`, `REG8()` from `bof3/include/bof3/defines.h` for hardware registers.
- `DEFINE_FUNC_AT()` in `bof3/include/bof3/context.h` only for cross-module calls not yet lifted.
- Readable semantic names > forcing 100% match when too complex.
- Add function to the module's `BOF3_MODULE_*_SOURCES` in `bof3/cmake/sources.cmake`.

## Verify
```bash
bin/asm-diff-one <source>   # per-function
make build                   # full
```
