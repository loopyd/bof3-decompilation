# Decomp Workflow

This is the repeatable loop for small, focused decomp work in `rebof3-simple`.
The fast path is function parity: lift or revise one function, build that
function, diff it against the original bytes, and repeat. Whole-`.bin` parity is
a later module-level gate after enough functions have been migrated.

## Bootstrap

Run these once after setup inputs are present:

```bash
make doctor
make extract
make inventory
make ghidra
make configure
make build
make doctor
```

`make doctor` checks the tools used by this loop, including the PSn00b MIPS
objdump and nm binaries used by asm diff.

## One Function Loop

Pick one C source file under `bof3/src/` and run:

```bash
bin/asm-diff-one bof3/src/core/emi/func_80162178.c
```

The harness equivalent records an event in `out/harness/harness.sqlite3` while
using the same asm-diff implementation:

```bash
bin/harness verify function bof3/src/core/emi/func_80162178.c
```

After `bin/harness analyze`, existing lifted source files are also available as
function queue targets. For those targets, `bin/harness diff <target-id>` runs
the same asm-diff loop and records the result on the target.

The command:

1. builds only that CMake object target,
2. infers the original address from `@source` or `func_XXXXXXXX`,
3. infers the byte size from the next sibling source file,
4. extracts the original bytes from `build/extracted/SLUS_004.22` for core
   sources or `build/extracted/LOGO/LOGO.EXE` for logo sources,
5. writes extracted original asm, generated compiler asm, normalized asm, and a
   unified diff under `out/asm-diff/<function>/`.

Important outputs:

- `out/asm-diff/<function>/summary.json`
- `out/asm-diff/<function>/diff.patch`
- `out/asm-diff/<function>/original.objdump.s`
- `out/asm-diff/<function>/current.compiler.s`
- `out/asm-diff/<function>/current.objdump.s`
- `out/asm-diff/<function>/original.normalized.s`
- `out/asm-diff/<function>/current.normalized.s`

`bof3/src/...` is the authored decomp source tree. `bof3/asm/...` mirrors it
for reviewed original assembly baselines or fallback assembly. Generated
comparison artifacts stay in `out/asm-diff/...`.

If size inference is not possible, pass the size explicitly:

```bash
bin/asm-diff-one bof3/src/core/emi/func_80162178.c --size 0x70
```

Check `original_size` in `summary.json`. If the next sibling source is not the
true next original function, pass `--size` explicitly.

Overlay sources need an explicit source binary and load address until the
overlay map is connected directly to asm diff:

```bash
bin/asm-diff-one bof3/src/modules/battle/03/func_801dece0.c \
  --binary path/to/overlay.bin \
  --load-address 0x801d0000 \
  --size 0x74
```

## Working Rules

- Work one function at a time.
- Keep constants and original addresses visible in source or headers.
- Prefer small named headers over linker-script symbol patches.
- Use `bof3/include/bof3/original_symbols.h` only for not-yet-lifted original
  calls, and replace those macros as functions are lifted.
- After each meaningful change, rerun `bin/harness verify function` or
  `bin/asm-diff-one` for the function.
- Before handing off, run `bin/build`, `bin/doctor --strict`, and the relevant
  Python tests.
