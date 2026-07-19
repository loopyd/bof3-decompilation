# BOF3 Decompilation Foundation

> **Status.** This is the repository's adaptation of the external "PS1
> Matching-Decomp Foundation" standard. The verbatim 24-section standard was
> adopted as the design contract; its normative rules are summarized here and
> mapped onto this repository's actual layout. The concrete macro reference and
> worked examples live in [`docs/memory-api.md`](memory-api.md). The iteration
> loop and oracle workflow live in [`docs/matching.md`](matching.md); the
> symptom-to-lever catalog in [`docs/matching-playbook.md`](matching-playbook.md).
> Where this doc and the standard's illustrative layout differ, the
> [Repo deviations](#repo-deviations) section is authoritative.

## Core principle

> Generic headers describe PS1 memory.
> Subsystem headers describe program semantics.
> Source files reproduce original source shape.
> Build profiles reproduce compiler behavior.
> Unmatched assembly remains assembly until clean C can reproduce it.

Four outcomes drive every decision:

1. Decompiled C stays readable and resembles normal source code.
2. Low-level PS1 memory semantics stay explicit.
3. Byte-matching exceptions stay local and documented.
4. The build reproduces compiler, assembler, linker, section, and relocation
   behavior per object.

Do not build a large abstraction framework around every pointer qualifier or
matching trick. Small primitive helpers, accurate declarations, the original
compiler configs, semantic accessors, and local source-shaping changes are the
whole toolkit.

## Memory API (the permanent generic layer)

Implemented in `include/bof3/`. Full reference in [`docs/memory-api.md`](memory-api.md).

```c
PSX_PTR        /* typed pointer to a fixed address */
PSX_REF        /* lvalue at a fixed address */
FIELD_ADDR / FIELD_REF   /* transitional offset access on incomplete structs */
REG8 / REG16 / REG32     /* PS1 memory-mapped hardware registers */
SPAD_ADDRESS / SPAD_ADDR / SPAD_REF / SPAD_PTR_SLOT  /* scratchpad RAM */
WEAK_SYMBOL_AT /* bind a typed absolute symbol to an original-binary address */
```

Raw integer-to-pointer conversion exists only in `memory.h`. Generic headers
contain no game-specific globals; those belong in each target's `internal.h`
and target-local symbol files, per `AGENTS.md` ownership.

Qualifiers stay explicit on the `type` argument (never hidden in `vu8`-style
typedefs):

```c
volatile u8
const volatile u32
Entity *volatile
volatile Entity *volatile
```

## Semantic accessors

Low-level macros live in headers, not scattered through function bodies. Prefer
object-like macros for global pointer aliases and `static inline` accessors when
they match the original load pattern and improve semantics. Never expose raw
scratchpad offsets repeatedly across `.c` files. Per standard §7, match the
original load pattern — do not mechanically force one style.

## Matching workflow

Per [`docs/matching.md`](matching.md). Summary:

1. Establish the oracle: original assembly, relocations, jump-table data,
   referenced symbols, expected object, target compiler profile.
2. Reconstruct semantic source (m2c output is a hypothesis, not truth).
3. Match in order: signature → structs → globals → calls → control flow →
   loads/stores/sign-extension → stack → relocations/rodata → register
   allocation → scheduling/delay slots. Do not fight register allocation while
   the declaration or control flow is still wrong.
4. Compare continuously with objdiff; record the first real difference.
5. Run the permuter only after types, branches, loop topology, relocations, and
   stack frame are correct. It is not a substitute for a correct model.

A function is matched only when instruction bytes, relocations, local rodata,
stack frame, and function size all match (100% — no fuzzy percentages). See
[`docs/matching-playbook.md`](matching-playbook.md) for the symptom catalog
before altering a near-match.

## Local matching aids

Per standard §16, aids stay local to the function and carry a `MATCHING_AID`
comment. Acceptable: temporaries, pointer hoists, early returns, if/else
inversion, duplicated assignments, manual `goto` loops, reordered independent
statements. Avoid promoting these into generic macros (no `FORCE_REGISTER`,
`KEEP_TEMP`, etc. in common headers).

Register pinning and inline assembly are the last C-level resort, after
declarations, symbol representation, branch direction, loop shape, temporaries,
deref hoists, statement reordering, and the permuter are exhausted. Remove
speculative pins once a structural match is found.

## Data materialization

When a function byte-matches, materialize its owned data: zero-init owned BSS
globals, define owned initialized data with original bytes, keep other objects'
globals `extern`, and confirm following symbol addresses stay correct. Code
objects own their local statics, BSS, initialized globals, and private rodata.

## Implementation checklist (standard §23 adapted)

- [x] Generic headers normalized (`defines.h`, `memory.h`, `scratchpad.h`,
  `symbols.h` under `include/bof3/`); compatibility macros removed.
- [x] Unmatched-assembly pipeline (`INCLUDE_ASM`, `INCLUDE_RODATA`, `SKIP_ASM`,
  `NON_MATCHING`, `PERMUTER`, `M2CTX`) in `include/bof3/asm.h`.
- [x] Compiler profiles in `config/compiler-profiles/flag-catalog.json` +
  per-target config; inspect with `bin/flag-search TARGET@0xADDRESS`.
- [ ] Semantic subsystem accessors kept per-target in `internal.h`.
- [ ] Automated verification rejects regressions in matched functions
  (`bin/decomp-status`, `bin/asm-diff`, `bin/byte-match`).

## Definition of done

Generic foundation:

- Raw address casts exist only in `memory.h`.
- Scratchpad access is built on `PSX_PTR`/`PSX_REF`.
- Generic headers contain no program-specific globals.
- Compatibility macros are removed.
- Pointer qualifier semantics are explicit.

Function matched:

- Instructions byte-identical; relocations, stack frame, size, local rodata,
  and jump-table refs correct.
- Matching aids documented; speculative register pins removed where possible.
- Owned data materialized; the function is protected from regression.

Source quality: semantic C (`CURRENT_CONTEXT->state = 1;`) is the normal
interface; the low-level representation stays available but not sprinkled
through function bodies.

## Repo deviations

Deliberate differences from the standard's *illustrative* layout, recorded so
this doc stays usable without silent drift.

1. **No `include/subsystems/` or `include/psyq/` trees.** The standard shows
   these as example layout. This repo keeps per-subsystem semantic accessors in
   each target's `internal.h` and target-local symbol files
   (`config/symbols/<target>.txt`), per `AGENTS.md` ownership.
   `include/bof3/psyq.h` wraps the PsyQ SDK headers instead of `include/psyq/`.

2. **Repo-extension matching aids are retained.** Evidence-backed and not in the
   standard's banned set (§16): `FUNCTION_AT` (fixed-address function pointer,
   `include/bof3/memory.h`), `barrier()` and `CLOBBER_A0()/V0()/A1()`
   (MIPS delay-slot/access-ordering barriers, `include/bof3/defines.h`),
   `NO_SIBLING_CALLS` (`include/bof3/defines.h`). See `docs/memory-api.md` and
   `LESSONS.md` for worked examples.

3. **`INCLUDE_ASM`/`INCLUDE_RODATA` follow standard §8.** `include/bof3/asm.h`
   (pulled in for every TU via `include/bof3/bof3.h`) is the single canonical
   definition: 2-arg `INCLUDE_ASM(FOLDER, NAME)` /
   `INCLUDE_RODATA(FOLDER, NAME)` that `.include` the adjacent `.s` file, plus
   `SKIP_ASM`, `NON_MATCHING`, `PERMUTER`, and `M2CTX` support. The former
   `include/include_asm.h` is now a thin wrapper over `bof3/asm.h`.

4. **Compiler profiles** live in `config/compiler-profiles/flag-catalog.json`
   plus per-target config under `config/targets/` (standard §9 intent).

5. **Scratchpad pointer cell is non-volatile by design.** `SPAD_PTR_SLOT`
   expands to `PSX_REF(type *, addr)` (non-volatile cell) to match
   constant-address codegen; use `SPAD_PTR_SLOT(volatile T, off)` for a volatile
   *target*. This matches the standard's §4/`PSX_REF` qualifier rules.

## Header barrel convention (`internal.h`)

Every target `internal.h` is a structured barrel, ordered:

1. Include guard (`#ifndef`/`#define`).
2. `#include` lines.
3. Types: `typedef`, `struct`, `enum` definitions.
4. External variables: every `extern ...;`.
5. External functions: free function prototypes (keep `@behavior`/`@source`
   doc comments attached).
6. `#define` macros and `static inline` helpers at the bottom (a `static inline`
   that references a `#define` follows that macro; a `typedef` used by an
   `extern` precedes it in step 3).

This ordering is enforced by convention only (no CI lint yet). See the
bof3-re skill for the lift workflow that produces these headers.
