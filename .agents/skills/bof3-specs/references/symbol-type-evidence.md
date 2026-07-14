# BOF3 symbol and type evidence

## Contents

- [Result model](#result-model)
- [Establish the exact target](#1-establish-the-exact-target)
- [Inventory semantic evidence](#2-inventory-semantic-evidence)
- [Recover callers, callees, and data shape](#3-recover-callers-callees-and-data-shape-in-rizin)
- [Correlate signatures and PsyQ](#4-correlate-a-function-signature)
- [Recover and apply types](#6-recover-structures-and-values-safely)
- [Aliases and replay](#7-add-semantic-aliases-while-retaining-traceability)
- [Duplicates, promotion, and validation](#9-handle-duplicate-code-across-binaries-and-emi-entries)
- [Stop conditions](#stop-conditions)

This workflow maps semantic evidence from any PSX executable, overlay, raw code
or data blob, PsyQ header/library, string, and repeated binary back to original
`func_XXXXXXXX` and `DAT_XXXXXXXX` symbols. EMI entries are one application,
not the boundary of the method.
The address name remains the stable identity used by Splat, asm-diff, m2c,
permuter, analyzer replay, and cross-target comparison. A semantic name is an
evidence-backed alias, not a replacement for that identity.

## Result model

Keep four facts separate:

| Fact | Example | Owner |
| --- | --- | --- |
| Binary identity | `exe/slus_004_22`, `0x80161fdc` | target/layout metadata |
| Address symbol | `func_80161fdc`, `DAT_8014677c` | Splat symbols and compiled declarations |
| Semantic interpretation | `emi_stream_init_slot` | target/shared header alias or analyzer comment |
| Type/signature | `void (u32)`, `EmiTocEntry *` | owning `internal.h` or reviewed shared header |

Do not encode all four facts in one analyzer rename. A useful analyzer display
name must not make the original address or target disappear.

## 1. Establish the exact target

Before correlating anything, record:

- executable or promoted EMI target identity;
- input SHA-256 and runtime load address;
- function/data boundary and size;
- architecture `mips`, 32 bits, little-endian;
- whether the input is a normalized executable image or one extracted raw EMI
  entry, never the `.EMI` container.

Prefer the repository adapter:

```sh
bin/harness target show exe/slus_004_22
bin/harness analysis doctor
bin/harness analysis init exe/slus_004_22
bin/harness analysis query exe/slus_004_22 functions
```

For native inspection, use the verified load address:

```sh
rizin -q0 -a mips -b 32 -e cfg.bigendian=false -m 0xLOAD RAW.bin
```

Then verify bytes and instructions before running broad analysis:

```text
px 32 @ 0xADDRESS
pd 12 @ 0xADDRESS
af @ 0xADDRESS
pdf @ 0xADDRESS
```

If an address is absent, first recheck the PS-X EXE `t_addr`, target manifest,
and raw-image mapping. Do not reinterpret zero padding as evidence.

## 2. Inventory semantic evidence

Search source and reviewed declarations before inventing a name:

```sh
rg -n 'emi_|Emi|func_80161fdc|DAT_8014677c' include src config docs/specs
rg -n 'typedef .*\(\*|struct |enum ' include/bof3 src/emi src/exe
rg -n 'WEAK_SYMBOL_AT\(|#define .*func_' src include/bof3
```

Useful evidence, strongest first:

1. direct calls, loads/stores, instruction widths, constants, and boundaries;
2. multiple xrefs showing a consistent role;
3. exact repeated implementation plus compatible relocations and state use;
4. official PsyQ header prototype, enum, macro, or structure layout;
5. archive-member/library provenance or a recognized library signature;
6. strings and neighboring state transitions;
7. decompiler-proposed names or types.

Named code in another target is evidence, not authority. Recover its
original address symbol with `rg`, the target's Splat symbols, or analyzer xrefs,
then compare call shape and referenced state in both targets.

## 3. Recover callers, callees, and data shape in Rizin

Start bounded and create the reviewed address function if needed:

```text
aa
af func_80161fdc 0x80161fdc
afu 0xFUNCTION_END @ 0x80161fdc
pdf @ 0x80161fdc
axt 0x80161fdc
axf 0x80161fdc
agf @ 0x80161fdc
```

Machine-readable queries are useful for comparison, but probe their schema on
the installed version:

```text
aflj
axj
agfj @ 0x80161fdc
aoj 24 @ 0x80161fdc
```

For data, inspect every access rather than trusting an inferred type:

```text
axt 0x8014677c
px 64 @ 0x8014677c
pd 16 @ <each-xref-function>
```

Record for each field:

- byte offset and observed stride;
- `lb/lbu`, `lh/lhu`, or `lw` access width and signedness;
- read, write, or address-taken use;
- array bound or terminal condition;
- pointer arithmetic and alignment;
- functions receiving the value in `a0` through `a3` or returning it in `v0`.

An `lw` does not prove a pointer; a value may be an integer, address, bitset, or
packed data. A Ghidra cast does not resolve that ambiguity.

## 4. Correlate a function signature

Build the prototype from MIPS evidence before naming it:

- inspect `a0`-`a3` setup at several callers;
- inspect stack-passed arguments where applicable;
- determine whether `v0`/`v1` is consumed and at what width;
- inspect sign/zero extension at both caller and callee;
- identify indirect calls (`jalr`) and the table or field supplying the target;
- compare the candidate prototype with every known callsite.

For an indirect callback, preserve both the address function and a reusable
function-pointer type:

```c
typedef void (*EmiLoaderCallback)(void);

extern void func_80162d18(EmiLoaderCallback callback);
#define emi_loader_set_callback func_80162d18
```

Use a function-pointer typedef only when it names a repeated contract or a
stored callback field. A one-off direct call normally needs only a prototype.
Do not hide unknown parameters with a variadic prototype or an old-style empty
parameter list merely to satisfy compilation.

Rizin can display and propagate reviewed types, but source remains canonical:

```text
td "typedef void (*EmiLoaderCallback)(void);"
afc @ 0x80162d18
pdg @ 0x80162d18
```

Probe `afc?`, `afvt?`, and the installed decompiler commands before applying
function types; their exact mutation commands vary between engine versions.
After applying a candidate, re-read callers and raw disassembly. Type
propagation is a hypothesis generator, not proof.

## 5. Correlate PsyQ identities

Treat these as four independent facts:

1. official SDK header and version containing the declaration;
2. library/archive and object-member provenance, if known;
3. observed runtime implementation/call shape;
4. address in this exact executable or overlay.

Search official staged headers first:

```sh
rg -n '^[[:space:]]*(extern[[:space:]]+)?[^;]*CdControl[[:space:]]*\(' \
  toolchains/psyq include
rg -n 'CdControl|libcd|LIBCD' toolchains config docs src
```

In Rizin, inspect all callers and the candidate body:

```text
axt 0xPSYQ_ADDRESS
pdf @ 0xPSYQ_ADDRESS
axf 0xPSYQ_ADDRESS
/ string-associated-with-api
```

Check argument count, return use, imported SDK types, side effects, and known
wrapper behavior against the official declaration. A recognizable call shape
alone is insufficient. A library-signature match strengthens identity but does
not prove that another binary uses the same runtime address.

When verified, use the official SDK declaration and bind its target-local
address in `symbols.c` or a shallow target `symbols/*.c` unit:

```c
/* PsyQ addresses are local to this target. SDK declarations remain canonical. */
WEAK_SYMBOL_AT(CdInit, 0x801cfc30);
```

Do not create a target-local PsyQ header, redeclare a conflicting prototype, or
lift verified library code. Record SDK version and archive/object provenance in
reviewed comments or a spec when known.

## 6. Recover structures and values safely

Start with offsets, not semantic field names:

```c
typedef struct EmiTransferSlot {
  u32 unknown_00;
  u32 byte_count;
  u8  unknown_08[8];
} EmiTransferSlot;
```

Promote a field name only when multiple accesses establish its role. Preserve
unknown padding explicitly so offsets remain visible. For an analysis-only
draft, use fixed-width C declarations in `config/analysis/bof3_objects.h` and
apply them to the exact address:

```text
to config/analysis/bof3_objects.h
ts EmiTransferSlot
tp EmiTransferSlot 0x8014677c
avga DAT_8014677c EmiTransferSlot @ 0x8014677c
avg
```

Modern Rizin uses typed globals such as `avga`; repository adapters may retain
version-specific `tl` commands. Do not copy a native command into tracked
replay until it is tested with the selected engine.

Before moving a type into compiled headers, verify:

- PSX pointer size and alignment;
- `sizeof` and important offsets under the project compiler;
- all observed signed/unsigned accesses;
- array length/stride across more than one xref;
- that the type is shared semantically, not merely byte-identical.

Record layout confidence independently from source-match confidence. A reviewed
`lb`/`lbu`, `lh`/`lhu`, or `lw` access can prove a field offset, width, and
signedness even while the current C reconstruction is non-exact. It does not by
itself prove a semantic field name. Keep unknown names/padding, cite the raw
consumer evidence, and report the C match status separately.

Keep a target-specific type in its `internal.h`. Move it to `include/bof3/`
only when multiple compiled targets use the same reviewed contract. Keep
analysis-only cross-target catalogs in `config/analysis/`; they do not override
compiled declarations.

## 7. Add semantic aliases while retaining traceability

For compiled functions, the preferred pattern is:

```c
extern void func_80161fdc(u32 slot_id);
#define emi_stream_init_slot func_80161fdc
```

For compiled data, retain the address object and introduce a semantic macro
only after its type and role are reviewed:

```c
extern EmiTocEntry DAT_8014677c[];
#define emi_slot_table DAT_8014677c
```

The binding remains address-based:

```c
WEAK_SYMBOL_AT(DAT_8014677c, 0x8014677c);
```

This preserves searchable source, linker bindings, matching-tool inputs, and a
readable semantic callsite. Do not bind both alias names to the same address,
and do not use a typedef as a substitute for a symbol alias: typedefs name
types, macros name the proven source-level role of an address symbol.

For evidence-backed but unproven meanings, keep only the address symbol and a
concise comment:

```c
/* INFERRED: slot table from 0x10-byte indexed accesses; verify all writers. */
extern u8 DAT_8014677c[];
```

Do not promote an inferred name into a public shared header.

## 8. Represent the mapping in analyzer replay

Keep address identity as the function/flag name and put semantic evidence in a
comment unless the selected export preserves aliases reliably:

```text
af func_80161fdc 0x80161fdc
CC "Reviewed alias: emi_stream_init_slot; u32 slot_id" @ 0x80161fdc
f DAT_8014677c 0xSIZE @ 0x8014677c
CC "Reviewed type: EmiTocEntry[]; target-local address" @ 0x8014677c
```

If a semantic analyzer flag is useful, place it in a distinct reviewed
flagspace and retain the canonical address flag. Test export/replay first;
flag-name collisions and namespaces differ between Rizin/radare2 versions.

Store reviewed commands in `config/analysis/<target-path>.r2`, grouped in stable
order: functions, data, type import/placement, comments. Generate projects and
exports under `out/analysis/`:

```sh
bin/harness analysis init exe/slus_004_22
bin/harness analysis export exe/slus_004_22
bin/harness analysis graph exe/slus_004_22
```

Reinitialize from clean input and replay, then compare deterministic exports.
A native `.rzdb` or radare2 project is a cache, not the sole copy of evidence.

## 9. Handle duplicate code across binaries and EMI entries

Exact bytes prove only bounded byte identity. For each target independently,
compare:

- verified load address and relocation delta;
- direct call targets and global references;
- data layouts and state ownership;
- entry convention and overlay lifetime;
- compiler-visible prototype and return use;
- PsyQ/library bindings.

Useful repository and analyzer checks:

```sh
bin/harness analysis graph
rg -n 'func_XXXXXXXX|semantic_alias' src config include
```

```text
pdf @ 0xADDRESS_A
axf 0xADDRESS_A
axt 0xADDRESS_A
```

Repeat the native commands in the other target's separately mapped project.
Never seek to target B's address inside target A's project and infer absence or
identity from the result.

If two targets implement the same behavior at different addresses, keep two
`func_XXXXXXXX` symbols and target-local bindings. They may share a typedef,
enum, or semantic alias name in separate target headers. Promote a shared
compiled declaration only when the ABI and ownership are independently proven.

## 10. Promotion sequence

Promote the smallest fact to the narrowest owner:

1. Correct boundary/load information in tracked target layout.
2. Add reviewed analyzer function/data/comment/type placement to
   `config/analysis/<target-path>.r2`.
3. Add unresolved target-local absolute binding to `symbols.c` or its shallow
   `symbols/functions.c` / `symbols/variables.c` units.
4. Add the address-based declaration to the target symbol barrel or
   `internal.h`.
5. Add a semantic macro alias and a recovered typedef in `internal.h`.
6. Promote only genuinely shared ABI/types to `include/bof3/`.
7. Record stable runtime/layout contracts in `docs/specs/`.

The repository already demonstrates this split:

- `include/bof3/core/emi.h` declares `func_80161fdc` and exposes
  `emi_stream_init_slot` as an alias;
- `src/exe/slus_004_22/symbols/functions.h` retains loader address symbols and
  semantic aliases;
- `src/exe/logo/symbols.c` binds verified PsyQ names at LOGO.EXE-local
  addresses while official SDK declarations remain canonical;
- `config/analysis/bof3_objects.h` contains analysis-only recovered layouts.

Do not duplicate declarations across `internal.h` and focused symbol headers.
Keep the include path singular: `internal.h` includes `symbols/symbols.h`, which
barrels `functions.h`, `variables.h`, and `files.h` where that target needs the
split.

## 11. Validate every promotion

Before editing compiled declarations, capture a canonical baseline:

```sh
bin/harness diff src/path/func_XXXXXXXX.c </dev/null
```

After the type, prototype, or alias change, run the same diff and compare
instruction count, byte size, score, and first mismatch. A semantic alias should
not change output. A type correction may change output; accept it only when the
new instructions agree with original evidence.

Then run the smallest target build and repository checks appropriate to the
change:

```sh
just build
just check
bin/harness doctor --strict
```

For analyzer state, rebuild the project from exact input plus tracked replay and
compare the deterministic export. For a new PsyQ identity, also verify every
known callsite against the official prototype.

## Stop conditions

Do not promote when any of these remain unresolved:

- target, load address, or function/data boundary is uncertain;
- callers disagree on argument count, width, or return use;
- a semantic name rests on one string, one callsite, or decompiler output;
- a structure requires overlapping fields without evidence for a union;
- official PsyQ prototype and observed call shape conflict;
- duplicate bytes have unreviewed relocations or target-local global accesses;
- analyzer replay cannot reproduce the proposed name/type placement;
- the compiled source no longer builds or its canonical diff regresses without
  binary evidence explaining the change.

Keep the address symbol and an `INFERRED:` or `UNKNOWN:` note instead. That is a
useful, reversible intermediate result.
