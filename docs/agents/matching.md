# Function matching

Work one `TARGET@0xADDRESS`; equal addresses in different targets are
unrelated until proven otherwise.

## First source and required loop

For an existing claimed source, obtain a live `asm-diff` before editing C.
For a genuinely unlifted function, the parent first admits one bounded seed:
pin original bytes, load address, reviewed boundary, caller/callee evidence,
PRE source absence, exact owned source/header/map/layout paths and attempt limits.
Create only the evidence-backed C89 seed with `@source`/`@behavior`, its manifest
claim and required declarations/bindings and `c` boundary. Then obtain the first
live `asm-diff` through the [native execution route](codex.md#native-compiler-execution)
before any further C edit. A missing-source error is not a native baseline; seed
creation grants no score, acceptance, budget renewal or compiler-guard bypass.
Unresolved ownership/ABI or a failed native gate stops matching until resolved
within the original bounds. Final live byte matching and independent review remain
mandatory. `agent-run diagnose` still requires an existing claimed source.

Native equality is not progress-metadata validity. An exact record uses
`@status exact`, `@match 100.00`, and `@residual none`; put explanatory notes on
separate comment lines, not after a semicolon in a tag value. At naming handoff,
`bin/naming-audit prepare TARGET` reports metadata debt. Its explicit repair mode
requires fresh exact proof and preserves notes; never weaken the parser or repair
unrelated rows to make a selected function ready.

```sh
bin/splat TARGET
bin/m2ctx TARGET@0xADDRESS
bin/m2c TARGET@0xADDRESS -o out/candidate.c
# edit the metadata-resolved lift under src/bof3/<subsystem>/
bin/asm-diff TARGET@0xADDRESS
bin/byte-match TARGET@0xADDRESS
```

`bin/splat` reads tracked target configuration under `config/targets/` and shared
source-protection/full-disassembly options from [`config/splat.yaml`](../../config/splat.yaml).
Target YAML owns generated symbol-list, asset and cache paths under `out/splat/<target>/`;
bootstrapping propagates these settings. Generated lists are disposable evidence,
not reviewed maps: promote only verified target-local facts into `symbols.txt`.

1. Verify the target manifest, Splat boundary, and map.
2. Treat m2c output as a C seed, never layout evidence.
3. Recover control flow, signedness, access widths, calls, delay slots in
   readable C89.
4. Update function C, adjacent `internal.h`, target-local map, or Splat
   boundary when evidence requires; rerun `bin/symbols check TARGET` and
   `bin/splat TARGET` after configuration changes.
5. `bin/asm-diff` for instructions; `bin/byte-match` for raw equality.
6. Credible semantics but wrong source shape: one bounded
   `bin/permute TARGET@0xADDRESS --time-limit 60 -j N` coordinator (60s cap;
   `--allow-long-run` is interactive-only).

Permuter scores rank candidates, never accept a match. Never run two
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

Representative still far from matching: skip unless size and expected reuse
justify effort. Never multiply a partial implementation across the group
because the original bytes are identical.

Normalize names from evidence before sharing code (AGENTS.md §Exact
duplicates). Constants as template parameters only when members genuinely
differ; exact members normally share the same readable constants.

After two cross-target members independently match, a worthwhile common body
may live in `src/shared/<domain>/<role>.inc`. Each member keeps a small
metadata-tagged target-local wrapper defining the compiled function macro and
explicit parameters before including the template; the wrapper filename may be
semantic. No wrapper call or linked extern function: either can change
instructions or cross independently loaded binary ownership.

Every promoted member still requires its own source declaration, target map,
Splat `c` boundary, `bin/asm-diff`, and `bin/byte-match` result.

### Engine promotion

- Identical code embedded in multiple EMIs stays target-owned; reuse its C
body at compile time only when that reduces maintenance.
- A runtime engine service has one implementation owned by `SLUS_004.22` plus
EMI callsite evidence to that address. Keep its C under the narrowest semantic
`src/bof3/<class>/` path, ownership carried by explicit manifest claims and
target metadata; promote only its stable contract to `include/bof3/core.h`.
- `src/shared/` owns embedded implementation templates, never standalone
runtime objects; no generic `src/engine/` ownership until a real link target
exists.

## Candidate validation

```sh
bin/promote TARGET@0xADDRESS src/bof3/<subsystem>/<metadata-resolved-source>.c
```

In its canonical source file, `bin/promote` requires
format-clean source, compiles, links, compares; never changes reviewed source,
Splat, or maps.

## Lift audit

```sh
bin/decomp-status [TARGET...]
bin/decomp-status exe/logo --json -o out/status.json
```

Results: `exact`, `partial`, `invalid`. An improved, reviewed coherent
partial stays tracked with function-level `@status partial`, `@match NN.NN`,
and `@residual ...`; an improving per-object flag/profile may stay with the
same evidence. Progress evidence, never exact acceptance. Revert only when a
completed bounded pass has no net improvement or review rejects
semantics/types. Valid partial lifts exit `0`; invalid
metadata/compilation/linking/comparison exits `2`. Rizin-index coverage is
supplementary; its unavailability does not invalidate the audit.

Aid-removal requeues use `@status partial`, `@match unavailable`, and a residual
stating that fresh clean-C matching and independent review are pending. Never
carry an aided score forward as a measurement of the cleaned source. These
unmeasured requeues are not accepted partial or exact results.

## Approved assembly fallback

`INCLUDE_ASM` is an explicit-user-approved fallback only (policy:
[playbook §Approved `INCLUDE_ASM` fallback](matching-playbook.md#approved-include_asm-fallback)).
Approved, it preserves section selection, alignment, symbol metadata, and
separate read-only data inclusion.

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
`.text` in the same `.s` file. `INCLUDE_RODATA(FOLDER, NAME)` only when an
explicitly approved layout requires a separate data fragment.

### Promotion path

On reconstruction success:

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

The [source contract](../INDEX.md#source-and-duplicate-rules) bans `REGISTER_PIN`,
direct asm register bindings, `CLOBBER_*`, `barrier()` and artificial empty asm.
Prior approvals, exact aided bytes and allocator regressions do not permit
retention or restoration as matching candidates. Remove the aids without a
declaration/no-op shim, preserve historical evidence, and requeue each consumer
for clean-C native matching and independent review. Exhausting clean-C levers
leaves a documented residual; it does not reopen the old exceptions.

## Owned-data materialization

On byte-match, materialize owned data: zero-init owned BSS globals, define
owned initialized data with original bytes, keep other objects' globals
`extern`, confirm following symbol addresses stay correct.

## `internal.h` order

Order: include guard (`#ifndef`/`#define`); `#include` lines; types
(`typedef`, `struct`, `enum`); external variables (`extern ...;`); external
function prototypes; `#define` macros and `static inline` helpers at the
bottom.

## Naming

| Element | Style | Example |
| --- | --- | --- |
| Structs / typedefs | PascalCase | `PanelTask`, `AbilityObject` |
| Struct members | snake_case | `targeting_flags`, `item_type` |
| Function aliases | PascalCase, abbreviated domain prefix | `GpuAppendPrim`, `CbSchedTick` |
| Constants / macros | SCREAMING_SNAKE_CASE | `EMI_SECTOR_SIZE`, `EQUIP_RYU` |
| Single-bit flags | `(1 << N)` shift form | `#define ELEM_FIRE (1 << 0)` |
| Globals (fixed-address) | `g_` + PascalCase | `g_PrimCursor`, `g_GameState` |
| Source filenames | Flexible semantic name; mandatory function-level `@source`/`@behavior` own identity | `dispatchStateHandler.c` |

Values:

| Kind | Representation | Example |
| --- | --- | --- |
| Addresses, bitmasks, struct offsets/sizes | Hexadecimal | `0x8014598Cu`, `0x140` |
| Human quantities (pixels, counts, loop bounds) | Decimal | `32`, `228`, `92` |
| Encoded values, sentinels | Hexadecimal | `0xFF63`, `0x7f` |
| Sequential ordinals, state codes | Decimal in enum | `SKILL_CLASS_HEALING = 0` |
