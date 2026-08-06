# Function matching

Work on one `TARGET@0xADDRESS`; equal addresses in different targets are
unrelated until proven otherwise.

## Required loop

```sh
bin/splat TARGET
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS -o out/candidate.c
# edit src/<target>/func_XXXXXXXX.c
bin/asm-diff TARGET@0xADDRESS
bin/byte-match TARGET@0xADDRESS
```

1. Verify the target manifest, Splat boundary, and map.
2. Treat m2c output as a C seed, never layout evidence.
3. Recover control flow, signedness, access widths, calls, delay slots in
   readable C89.
4. Update function C, adjacent `internal.h`, target-local map, or Splat
   boundary when evidence requires; rerun `bin/symbols check TARGET` and
   `bin/splat TARGET` after configuration changes.
5. `bin/asm-diff` for instructions; `bin/byte-match` for raw equality.
6. Credible semantics but wrong source shape: one bounded
   `bin/permute TARGET@0xADDRESS --time-limit 300 -j N` coordinator.

Permuter scores rank candidates; they never accept a match. Never run two
coordinators for one function.

## Exact duplicate groups

Use `bin/rev-query duplicates TARGET@0xADDRESS --json` for the complete
exact-byte candidate group. Match one deterministic representative, then
validate each reviewed member in its owning target. Promotion sequence:

1. Verify every candidate range has the same reviewed size and bytes.
2. Iterate one representative to a byte-match. A partial lift, even high
   percentage, is only a candidate source shape.
3. Copy the shape to one other member, adapting only target-local symbols and
   declarations; make it byte-match independently.
4. Extract a shared body only after both match with the same C shape.
5. Add remaining members one at a time, keeping independent byte checks.

Representative still far from matching: skip the group unless size and expected
reuse justify the effort. Never multiply a partial implementation across the
group because the original bytes are identical.

Normalize names from evidence before sharing code:

- One semantic role for the group; same names for equivalent parameters,
  locals, structs, fields.
- Unknown struct fields by offset (`unk_00`, `unk_04`) until behavior supports
  a semantic name.
- Addresses and raw function symbols stay target-local; identical bytes do not
  make one module's extern address valid in another.
- Constants as template parameters only when members genuinely differ; exact
  members normally share the same readable constants.

After two cross-target members independently match, a worthwhile common body
may live in `src/shared/<domain>/<role>.inc`. Each `func_XXXXXXXX.c` stays a
small address-owned wrapper defining the raw function macro and explicit
parameters before including the template. No wrapper call or linked extern
function: either can change instructions or cross independently loaded binary
ownership.

Every promoted member still requires its own source declaration, target map,
Splat `c` boundary, `bin/asm-diff`, and `bin/byte-match` result.

### Engine promotion

- Identical code embedded in multiple EMIs stays target-owned; reuse its C
  body at compile time only when that reduces maintenance.
- A runtime engine service has one implementation in `SLUS_004.22` plus EMI
  callsite evidence to that address. Keep its C under `src/exe/slus_004_22/`;
  promote only its stable contract to `include/bof3/core.h`.
- `src/shared/` owns embedded implementation templates, never standalone
  runtime objects. No generic `src/engine/` ownership until a real link target
  exists.

## Candidate validation

```sh
bin/promote TARGET@0xADDRESS src/<target>/func_XXXXXXXX.c
```

With the candidate installed in its canonical source file, `bin/promote`
requires format-clean source, then compiles, links, compares. It never changes
reviewed source, Splat, or maps.

## Lift audit

```sh
bin/decomp-status [TARGET...]
bin/decomp-status exe/logo --json -o out/status.json
```

Results: `exact`, `partial`, `invalid`. Valid partial lifts exit `0`;
invalid metadata/compilation/linking/comparison exits `2`. Rizin-index
coverage is supplementary; unavailability does not invalidate the live audit.

## Approved assembly fallback

`INCLUDE_ASM` is an explicit-user-approved fallback only. Without approval,
leave the function as its reviewed Splat `asm` segment and report the clean-C
residual; never add an assembly-backed source stub.

Approved, `INCLUDE_ASM` preserves section selection, alignment, symbol
metadata, and separate read-only data inclusion without low-quality fake
matches.

### Usage after explicit approval

Keep raw assembly under `asm/nonmatchings/`, included from the address-owned
C translation unit:

```c
#include "internal.h"
#include "bof3/asm.h"

/* Explicitly approved fallback for func_8014AEE0. */
INCLUDE_ASM("asm/nonmatchings/<target>", func_8014AEE0);
```

The macro textual-includes `asm/nonmatchings/<target>/func_8014AEE0.s`;
never also compile it standalone. The C file remains the target's tracked
source/boundary owner.

### Assembly file format

```asm
.set noreorder
.set noat

.section .text.func_8014AEE0, "ax", @progbits
.align 2
.globl func_8014AEE0
.ent   func_8014AEE0
func_8014AEE0:
    # raw disassembly here — preserve original instruction order
.end   func_8014AEE0
.set reorder
```

The section name (`.text.func_XXXXXXXX`) must match the Splat boundary for
correct placement. Flags: `"ax"` code, `"aw" @nobits` BSS, `"a" @progbits`
read-only data.

### Adjacent .rodata

Adjacent `.rodata` (jump tables, string literals): place its section before
`.text` in the same included `.s` file. `INCLUDE_RODATA(FOLDER, NAME)` only
when an explicitly approved layout requires a separate data fragment.

### Promotion path

When reconstruction succeeds:

1. Replace `INCLUDE_ASM("FOLDER", func_XXXXXXXX);` with matching C body.
2. Remove its included `FOLDER/func_XXXXXXXX.s` file.
3. Update the Splat boundary from `"a"` (asm) to `"c"` (C).
4. Run `bin/byte-match TARGET@0xADDRESS` to validate.

The macro owns its assembly inclusion; no CMake source-list change is needed.

## Local matching aids and pins

Aids stay local to the function and carry a `MATCHING_AID` comment.
Acceptable: temporaries, pointer hoists, early returns, if/else inversion,
duplicated assignments, manual `goto` loops, reordered independent statements.
Never promote function-specific aids into generic macros.

`REGISTER_PIN(type, name, reg)` is the shared spelling for a local allocator
constraint — only after declarations, symbol representation, branch direction,
loop shape, temporaries, deref hoists, statement reordering, and the permuter
are exhausted, as one bounded local experiment on an asm-diff-proven allocator
or entry-register residual. Retain only with adjacent `MATCHING_AID` rationale,
independent review, live byte match. Never a generic matching macro. A legacy
direct numeric `"$N"` spelling still requires explicit user approval and proof
the macro form changes codegen. Remove speculative pins once a structural
match is found.

## Owned-data materialization

On byte-match, materialize owned data: zero-init owned BSS globals, define
owned initialized data with original bytes, keep other objects' globals
`extern`, confirm following symbol addresses stay correct.

## `internal.h` order

Every target `internal.h` is a structured barrel, ordered:

1. Include guard (`#ifndef`/`#define`).
2. `#include` lines.
3. Types: `typedef`, `struct`, `enum` definitions.
4. External variables: every `extern ...;`.
5. External functions: free function prototypes.
6. `#define` macros and `static inline` helpers at the bottom.

## Naming

| Element | Style | Example |
| --- | --- | --- |
| Structs / typedefs | PascalCase | `PanelTask`, `AbilityObject` |
| Struct members | snake_case | `targeting_flags`, `item_type` |
| Function aliases | PascalCase, abbreviated domain prefix | `GpuAppendPrim`, `CbSchedTick` |
| Constants / macros | SCREAMING_SNAKE_CASE | `EMI_SECTOR_SIZE`, `EQUIP_RYU` |
| Single-bit flags | `(1 << N)` shift form | `#define ELEM_FIRE (1 << 0)` |
| Globals (fixed-address) | `g_` + PascalCase | `g_PrimCursor`, `g_GameState` |
| Filenames | `func_XXXXXXXX.c` until the evidence gate passes; renamed files carry `@source` | `func_8014E5A0.c` |

Values:

| Kind | Representation | Example |
| --- | --- | --- |
| Addresses, bitmasks, struct offsets/sizes | Hexadecimal | `0x8014598Cu`, `0x140` |
| Human quantities (pixels, counts, loop bounds) | Decimal | `32`, `228`, `92` |
| Encoded values, sentinels | Hexadecimal | `0xFF63`, `0x7f` |
| Sequential ordinals, state codes | Decimal in enum | `SKILL_CLASS_HEALING = 0` |
