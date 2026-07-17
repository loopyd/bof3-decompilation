# Matching decompilation and build/diff workflow

## Separate semantic and matching milestones

Use these states:

```text
unexplored
boundary-confirmed
identified
prototype-draft
semantically-decompiled
runtime-validated
nonmatching
matching
```

A function may be runtime-validated yet nonmatching. Matching bytes do not independently prove names/types.

## Common PS1 decompilation stack

- **splat**: split PSX binaries, sections, code/data, and emit project configuration
- **spimdisasm**: MIPS disassembly/symbol-aware analysis used by splat and standalone workflows
- **maspsx**: emulate PsyQ assembler behavior for matching builds
- **mips2c/decompiler tools**: produce an initial C hypothesis
- **asm-differ**: instruction-level comparison during iteration
- **objdiff**: object/function comparison and progress tracking
- **decomp-permuter**: explore equivalent C transformations
- **decomp.me**: isolated scratch/matching experiments

Real PS1 projects commonly combine these rather than relying on one decompiler.

## Compiler identification

Before tuning C, determine:

- PsyQ/runtime library version
- GCC/CC1 version and flags
- assembler/preprocessor behavior
- optimization level
- small-data/GP options
- signed-char and ABI details
- source language extensions
- linker ordering and alignment

Evidence comes from library signatures, startup code, generated idioms, object metadata, debug strings, known project/toolchain history, and controlled compilation experiments.

## Repository structure

A practical matching project separates:

```text
config/             splat/build configuration
include/            recovered headers/types
src/                decompiled C
a sm/               nonmatching or hand assembly
assets/             extracted non-code assets (not redistributed improperly)
build/              generated objects/binaries
expected/           local hashes or original slices, not public proprietary data
tools/              pinned helper scripts
```

## Function iteration

1. Extract exact original function bytes/instructions.
2. Confirm boundaries and relocations.
3. Draft semantic C.
4. Compile with pinned toolchain.
5. Compare instructions and relocations.
6. Diagnose control-flow, expression ordering, register allocation, stack layout, and delay-slot differences.
7. Apply one intentional change at a time.
8. Preserve readability unless a matching idiom is proven necessary.
9. Record score and first differing instruction.
10. Run runtime regression when integration is possible.

## Common causes of mismatches

- incorrect function signature or signedness
- source expression order
- temporary variable lifetime
- `switch` lowering form
- loop shape (`for`, `while`, `do`)
- constant type/width
- structure field type/alignment
- inlining/macros
- compiler version or flags
- assembler macro expansion
- section/order/relocation mismatch
- data symbol alignment

## `bin/build-diff`

The wrapper reads environment variables rather than imposing a build system:

```bash
PSX_BUILD_CMD='ninja -C build' \
PSX_EXPECTED=expected/SLUS.bin \
PSX_ACTUAL=build/SLUS.bin \
bin/build-diff
```

Optional:

```bash
PSX_DIFF_CMD='objdiff-cli diff ...' bin/build-diff
```

The wrapper records SHA-256 and byte differences. A repository-specific script should supersede it when one already exists.

## Reproducibility

Pin tool versions in lockfiles/containers where possible. Retain:

- compiler/assembler hashes
- command lines
- environment variables
- generated linker scripts/maps
- original and rebuilt object hashes
- diff reports

Do not publish copyrighted original binaries or large binary slices merely to make CI convenient.
