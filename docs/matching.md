# Matching one function

Work on one `TARGET@0xADDRESS`; equal addresses in different targets are
unrelated until proven otherwise.

## Loop

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
3. Recover control flow, signedness, access widths, calls, and delay slots in
   readable C89.
4. Update the function C, adjacent `internal.h`, target-local symbol map, or
   Splat boundary when the evidence requires it; rerun `bin/symbols check` and
   `bin/splat TARGET` after configuration changes.
5. Use `bin/asm-diff` for instructions and `bin/byte-match` for raw equality.
6. If semantics are credible but source shape differs, run one bounded
   `bin/permute TARGET@0xADDRESS --time-limit 300 -j N` coordinator.

Permuter scores rank candidates; they do not accept a match. Do not run two
coordinators for one function.

## Reuse exact duplicate groups

Use `bin/rev-query duplicates TARGET@0xADDRESS --json` to inspect the complete
exact-byte candidate group. Match one deterministic representative, then
validate each reviewed member in its owning target.

Use this promotion sequence:

1. Verify every candidate range has the same reviewed size and bytes.
2. Choose one representative and iterate until it byte-matches. A partial lift,
   even with a good percentage, is only a candidate source shape.
3. Copy that shape to one other member, adapting only target-local symbols and
   declarations, and make it byte-match independently.
4. Extract a shared body only after both members match with the same C shape.
5. Add remaining members one at a time, retaining independent byte checks.

If the representative is still far from matching, skip the group unless its
size and expected reuse justify the decompilation effort. Never multiply a
partial implementation across the group merely because the original bytes are
identical.

Normalize names from evidence before sharing code:

- Use one semantic role for the group and the same names for equivalent
  parameters, locals, structs, and fields.
- Name unknown struct fields by offset (`unk_00`, `unk_04`) until behavior
  supports a semantic name.
- Keep addresses and raw function symbols target-local. Identical bytes do not
  make one module's extern address valid in another module.
- Keep constants as template parameters only when group members genuinely
  differ; exact members should normally use the same readable constants.

After two cross-target members independently match, a worthwhile common body
may live in `src/shared/<domain>/<role>.inc`. Each `func_XXXXXXXX.c` remains as a small
address-owned wrapper that defines the raw function macro and any explicit
parameters before including the template. Do not use a wrapper call or one
linked extern function: either can change instructions or cross independently
loaded binary ownership.

Every promoted member still requires its own source declaration, target map,
Splat `c` boundary, `bin/asm-diff`, and `bin/byte-match` result.

### Engine promotion

- Identical code embedded in multiple EMIs remains target-owned. Reuse its C
  body at compile time only when that reduces maintenance.
- A runtime engine service must have one implementation in `SLUS_004.22` and
  EMI callsite evidence to that address. Keep its C under
  `src/exe/slus_004_22/`; promote only its stable contract to
  `include/bof3/core.h`.
- `src/shared/` owns embedded implementation templates, never standalone
  runtime objects. Do not add generic `src/engine/` ownership until a real link
  target exists for it.

## Validate a candidate

```sh
bin/promote TARGET@0xADDRESS src/<target>/func_XXXXXXXX.c
```

After manually installing the candidate in its canonical source file,
`bin/promote` requires format-clean source, then compiles, links, and compares
it. It never changes reviewed source, Splat, or maps.

## Audit lifts

```sh
bin/decomp-status [TARGET...]
bin/decomp-status exe/logo --json -o out/status.json
```

Results are `exact`, `partial`, or `invalid`. Valid partial lifts exit `0`;
invalid metadata, compilation, linking, or comparison exits `2`. Rizin-index
coverage is supplementary and may be unavailable without invalidating the live
lift audit.

## Unmatched functions (INCLUDE_ASM)

When a function cannot yet be represented in clean matching C, use INCLUDE_ASM
as a first-class fallback. This preserves section selection, alignment, symbol
metadata, and separate read-only data inclusion — enabling incremental progress
without producing low-quality fake matches.

A clean unmatched function is better than unreadable C filled with arbitrary
hacks.

### Usage

Replace the function body in `func_XXXXXXXX.c`:

```c
#include "internal.h"
#include "bof3/asm.h"

// func_8014AEE0 is not yet matched — see adjacent .s file.
INCLUDE_ASM(func_8014AEE0);
```

The macro marks the call site with a global symbol declaration. The actual
implementation lives in an adjacent assembly file compiled into the same target:

```text
src/exe/<target>/func_XXXXXXXX.c    (declarations + INCLUDE_ASM marker)
src/exe/<target>/func_XXXXXXXX.s    (raw disassembly)
```

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

The section name (`.text.func_XXXXXXXX`) must match the Splat boundary to ensure
correct placement in the linked binary. Use `"ax"` flags for code, `"aw" @nobits`
for BSS, and `"a" @progbits` for read-only data.

### Adjacent .rodata

For functions with adjacent `.rodata` (jump tables, string literals):

- Include the `.rodata` section before `.text` in the same `.s` file, or
- Place it in a companion `func_XXXXXXXX.rodata.s` file and include both from
  the build system.

### Promotion path

When reconstruction succeeds:

1. Replace `INCLUDE_ASM(func_XXXXXXXX);` with matching C body.
2. Remove `func_XXXXXXXX.s`.
3. Update the Splat boundary from `"a"` (asm) to `"c"` (C).
4. Run `bin/byte-match TARGET@0xADDRESS` to validate.

### Build integration

The build system must compile adjacent `.s` files alongside their `.c`
counterparts for each target directory containing INCLUDE_ASM markers.

## Local matching aids

Aids stay local to the function and carry a `MATCHING_AID` comment. Acceptable:
temporaries, pointer hoists, early returns, if/else inversion, duplicated
assignments, manual `goto` loops, reordered independent statements. Never
promote function-specific matching aids into generic macros.

`REGISTER_PIN(type, name, reg)` is the shared spelling for an approved
register constraint. A pin is still local to its function: use it only after
declarations, symbol representation, branch direction, loop shape, temporaries,
deref hoists, statement reordering, and the permuter are exhausted; retain it
only with function-specific user approval, an adjacent `MATCHING_AID` rationale,
and a live byte match. A legacy direct numeric `"$N"` spelling may remain only
when the macro form demonstrably changes codegen. Remove speculative pins once
a structural match is found.

## Data materialization

When a function byte-matches, materialize its owned data: zero-init owned BSS
globals, define owned initialized data with original bytes, keep other objects'
globals `extern`, and confirm following symbol addresses stay correct.

## Header barrel convention (`internal.h`)

Every target `internal.h` is a structured barrel, ordered:

1. Include guard (`#ifndef`/`#define`).
2. `#include` lines.
3. Types: `typedef`, `struct`, `enum` definitions.
4. External variables: every `extern ...;`.
5. External functions: free function prototypes.
6. `#define` macros and `static inline` helpers at the bottom.

## Naming convention (PSX-era Capcom style)

| Element | Style | Example |
| --- | --- | --- |
| Structs / typedefs | PascalCase | `PanelTask`, `AbilityObject` |
| Struct members | snake_case | `targeting_flags`, `item_type` |
| Function aliases | PascalCase, abbreviated domain prefix | `GpuAppendPrim`, `CbSchedTick` |
| Constants / macros | SCREAMING_SNAKE_CASE | `EMI_SECTOR_SIZE`, `EQUIP_RYU` |
| Single-bit flags | `(1 << N)` shift form | `#define ELEM_FIRE (1 << 0)` |
| Globals (fixed-address) | `g_` + PascalCase | `g_PrimCursor`, `g_GameState` |
| Filenames | `func_XXXXXXXX.c` (address-based) | `func_8014E5A0.c` |

Values:

| Kind | Representation | Example |
| --- | --- | --- |
| Addresses, bitmasks, struct offsets/sizes | Hexadecimal | `0x8014598Cu`, `0x140` |
| Human quantities (pixels, counts, loop bounds) | Decimal | `32`, `228`, `92` |
| Encoded values, sentinels | Hexadecimal | `0xFF63`, `0x7f` |
| Sequential ordinals, state codes | Decimal in enum | `SKILL_CLASS_HEALING = 0` |
