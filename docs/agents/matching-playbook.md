# Matching playbook

Use this symptom-to-lever reference only after the [function-matching
loop](matching.md) has established the target, boundary, types, signatures, and
calls. Classify the first live `bin/asm-diff` difference, make one structural
change, and re-run the diff; a percentage alone is not a diagnosis.

## Symptom-to-lever table

| Diff symptom | First things to test | Section |
|---|---|---|
| `lw` instead of address calculation | Pointer vs array declaration | [Symbol representation](#symbol-representation) |
| Wrong relocation symbol | Standalone symbol vs struct field | [Symbol representation](#symbol-representation) |
| Folded base register | Array/table access, semantic symbol | [Symbol representation](#symbol-representation) |
| `lui $at` then `addu $at,$reg,$at`; original `addu $at,$at,$reg` | Extern-array rebinding so `as` folds base+index | [Symbol representation](#symbol-representation) |
| Address constant CSEd into a callee-saved register, stealing an entry copy | Extern symbol instead of fixed-address macro | [Symbol representation](#symbol-representation) |
| Original `lb`; current `lbu` plus `sll`/`sra` | Volatile pointee on a narrow signed global | [Volatility](#volatility) |
| Store missing from a jump delay slot; extra `nop` | Volatile store on plain RAM | [Volatility](#volatility) |
| `beq` vs `bne` | Invert `if/else` | [Control flow](#control-flow) |
| Wrong loop topology | `while`, `do`, `for`, guarded infinite loop, `goto` | [Control flow](#control-flow) |
| Wrong `$v0` return web | Early returns vs result variable | [Control flow](#control-flow) |
| Register swap or unexpected spill | Temp lifetime, statement order, pointer hoist | [Temporaries and allocation](#temporaries-and-allocation) |
| Extra `andi` before shift | Verify reconstructed mask is necessary | [Signedness](#signedness) |
| Unexpected block-copy loop | Structure assignment | [Symbol representation](#symbol-representation) |
| Phantom `.rodata` | Local `static const` vs shared `extern` | [Shared read-only data](#shared-read-only-data) |
| `lb` vs `lbu` | Field type and default `char` signedness | [Signedness](#signedness) |
| Wrong global/BSS offsets | COMMON, section, alignment, ordering, padding | [COMMON, BSS, and symbol order](#common-bss-and-symbol-order) |
| GNU assembler rejects GTE op | Exact `.word` in generated assembly | [Generated-assembly spelling](#generated-assembly-spelling) |
| Proven incoming `a*` copied to `t*`/`v*` at entry | Preserve a local value's lifetime; after the ladder, one local `REGISTER_PIN` experiment | [Allocation ladder](#allocation-ladder) |
| Same-sized near match with a lone delay-slot difference | Inspect the exact branch/jump and live operands; use clean-C ordering, then an evidenced caller-register clobber | [Delay slots and entry copies](#delay-slots-and-entry-copies) |
| Sole difference is commutative `addu` operand order using `$at` | Exhaust source representation forms, then record a compiler-order ceiling; never pin assembler scratch `$at` | [Allocation ladder](#allocation-ladder) |
| Preinitialization fills the desired delay slot but changes downstream registers | Reject it unless the longer value lifetime preserves the complete register web; placement alone is not progress | [Delay slots and entry copies](#delay-slots-and-entry-copies) |
| Code size or stack frame differs | Check calls, address-taken locals, aggregate copies, temporary lifetime, and branch topology before allocator aids | [Temporaries and allocation](#temporaries-and-allocation) |
| No clean C solution after levers | Record the exhausted evidence; do not add inline asm or `INCLUDE_ASM` without explicit approval | [Allocation ladder](#allocation-ladder) |

---

## Compiler profile

Before restructuring C for a stubborn function, verify the compiler profile:
different targets — sometimes different objects in one target — can differ in
compiler version, optimization level, `-G` value, signed-char setting.

To test a profile:

```sh
bin/flag-search TARGET@0xADDRESS
```

Profile levers that affect matching:

- Compiler version and patch level
- Optimization level (`-O0` through `-O3`)
- `-G` value (small-data limit)
- `-fsigned-char` / `-funsigned-char`
- `-mno-split-addresses`
- `-fno-function-cse`, `-fno-schedule-insns`, `-fno-delayed-branch`
- ASPSX version
- `--expand-div` (division hardening)
- COMMON-section behavior (`-fcommon`, `--use-comm-section`)

When `flag-search` proves a non-canonical profile, record it in
`config/compiler/object-flags.cmake` so the object builds with it instead of
bridging to canonical `-O2` with C aids:

```cmake
set(BOF3_OBJFLAGS_emi_etc_game_01_func_801D0D5C_c -O1)
```

Key: source path relative to `src/`, non-alphanumerics as underscores; value:
a `flag-search` candidate replacing the canonical `-O` level (the
`-G0 -funsigned-char ...` base is kept). CMake and the `compile_commands`
generator both read it, so build and `flag-search` stay in sync. Sources without
an entry keep canonical flags. Add an entry after `flag-search` reports an exact byte-match or a reviewed, semantically coherent net partial improvement. For a partial, record the selected flag/profile and live residual in `@status`/`@match`/`@residual`; remove it for no net improvement or semantic/type regression. Re-confirm exact results with `bin/byte-match` and partial results with live `bin/asm-diff`.

---

## Symbol representation

A declaration change can flip an address calculation into a load.

```c
/* Generates lw of a pointer then offset. */
extern Entity **g_entities;

/* Generates the address of the array directly. */
extern Entity *g_entities[];
```

```c
/* Generates relocation to g_state plus an offset. */
g_state.currentMode = 4;

/* Generates standalone relocation. */
extern s32 D_80123458;
D_80123458 = 4;
```

**Matching lever**: when m2c or your struct produces an unexpected `lw` for what
should be an address calculation, try the alternative declaration form.

### Extern rebinding: `addu` operand order and entry-copy theft

Replace a fixed-address macro with `extern Type D_XXXXXXXX[];` plus
`WEAK_SYMBOL_AT`, indexed directly, when:

- **`addu` operand order.** C computing `table + index` makes GCC emit its
  own fixed-order `addu`; through the extern-array form GCC emits
  `lw table($idx)` and `as` expands `%hi`/`%lo` with its canonical
  `lui $at; addu $at,$at,$idx` order. Remove any macro the rebinding strands.
- **Entry-copy theft.** A macro's constant address used twice can be CSEd
  into a callee-saved register, stealing an argument's prologue entry copy;
  the extern form emits `lui`/`lw` relocations and frees the allocator.

---

## Control flow

### Branch direction

Inverting an `if/else` is the most common control-flow fix:

```c
// Instead of:
if (condition) { HandleTrue(); } else { HandleFalse(); }

// Try:
if (!condition) { HandleFalse(); } else { HandleTrue(); }
```

This swaps `beq` ↔ `bne` and changes delay-slot scheduling and fall-through
order. A chain inversion surviving every C spelling is gcc 2.7
`reorder_insns` normalization: no flag disables it (`-fno-thread-jumps` is a
proven no-op); escalate, do not churn spellings.

### Equal-valued branch arms

GCC may range-fold grouped cases or reverse/tail-merge an `if` chain. Before
changing shape, interpret every MIPS branch delay slot on both taken and
fall-through paths; a delay-slot constant may be the branch comparison value or
the selected result, making apparently equal arms semantically distinct. Try,
one live diff each: ordered then nested/inverted equality tests; separate
ungrouped `case` bodies with varied case/default order; duplicated arm
stores/returns; then shared locals or explicit labels. Keep the best semantic
shape; stop after three non-progressing variants. Locals can alter the register
web. Proven on `emi/world00/area027/13@0x801F3650`: correct delay-slot semantics
made case 0 store 5, case 1 store 6, and default perform no store; separate
cases with an explicit empty default reproduced the compiler's shared-store
tail and byte-matched exactly.

### Duplicate identical calls in `if/else` arms

When the original computes a value directly in `$a0` in each branch and
tail-merges into one `jal`, write `if (cond) f(x + A); else f(x + B);` — a
ternary argument computes before the call, changing allocation and breaking
the merge.

### Early return vs result variable

```c
// Instead of:
s32 result = 0;
if (condition) { result = value; }
return result;

// Try:
if (condition) { return value; }
return 0;
```

The second form may allow values to coalesce directly into `$v0`.

### Loop shape

GCC 2.7–2.8 rotates structured loops into bottom-tested forms. These may all
compile differently:

```c
while (condition) { ... }

do { ... } while (condition);

for (;;) { if (!condition) break; ... }

/* goto wrapper preserves a genuine top-tested CFG. */
loop:
    if (!condition) goto exit;
    ...
    goto loop;
exit:
```

**Do not automatically clean up a matched `goto` into a loop.** The loop
form that produces identical instruction bytes is the correct form, regardless
of modern taste.

A `do { ... } while` wrapper also anchors loop-body scheduling when the
`for`/`while` rotation drifts. When the original preheader materializes
invariants late (`la` chains plus a `move` copying an entry pointer), assign
them *inside* the loop body: `loop.c` hoists them as late-created pseudos,
reproducing the preheader order and the entry copy; hoisting in source before
the loop flips `addu` operand order and drops the copy.

---

## Volatility

`volatile` is a scheduling constraint, not free documentation. Add it only
with asynchronous/hardware-mutation evidence (lessons.md); on plain RAM
globals it is the most common partial-lift root cause. Levers:

- A volatile store never moves into a jump delay slot; if the original sinks
  a store there and yours sits after a `nop`, drop the unjustified qualifier.
- A volatile pointee on a narrow signed global can force `lbu` + manual
  sign-extension where the original has one `lb`; a local non-volatile view
  `*(s8*)&x` restores it. Check declared type width/signedness first.
- A volatile pointer *cell* (`Type * volatile`) forces a per-evaluation
  reload without constraining the pointee — the sanctioned form when the
  original reloads a shared cursor.
- Evidenced exception: a view `*((volatile T*)SYM + n)` can pin an
  original-proven store-before-volatile-store order.

---

## `MATCHING_AID` comments

Every artificial matching aid is adjacent to the aid and says: what it
controls, the `asm-diff`-observed original/current instruction or register
placement, the exhausted ladder rung, and what future evidence removes it.
Retained `CLOBBER_*`/`REGISTER_PIN` aids must also say the immediately following
live `bin/byte-match` was exact. Never retain an aid on a percentage
improvement.

```c
/*
 * MATCHING_AID:
 * This guard produces the li $t2,2 that feeds the next comparison.
 * Both branches are identical; the condition must remain to keep $t2 live
 * and $a3 holding count. Remove when $t2 allocation is understood.
 */
if (count == 2) {
    flags |= 0x20;
} else {
    flags |= 0x20;
}
```

Do not mark obvious workarounds (evidenced `barrier()` ordering is
covered by [Volatility](#volatility)). Reserve `MATCHING_AID` for shape
decisions opaque to a reader without the matching diff.

---

## Temporaries and allocation

### Pointer hoist

```c
/* Instead of: */
value = (*table)[index];
other = (*table)[otherIndex];

/* Try: */
Entry *entries = *table;
value = entries[index];
other = entries[otherIndex];
```

Hoisting can free the allocator to place unrelated values in the required
register.

### Expression splitting

```c
/* Instead of: */
result = base[index].value + offset;

/* Try: */
Entry *entry = &base[index];
s32 value = entry->value;
result = value + offset;
```

Or collapse if the original keeps everything in one register.

### Named constant reuse

```c
/* Instead of: */
one = 1;
if (value == 1) {

/* Try: */
one = 1;
if (value == one) {
```

GCC may reuse the register already holding `one`.

### Per-evaluation reload via fresh locals

Re-read a global cursor per case (`case A: f(*table);`) instead of caching it
when the original reloads per region — fresh reads reproduce the original's
per-region reload registers (`a0` in one case, `v0` in another).

### Force global read/store order with a local

m2c reorders independent global accesses (`flag = 2; counter += 0x14;` may
emit the `sb` before the `lhu`). Pin the original order with a local:
`count = counter; flag = 2; counter = (u16)(count + 0x14);`. Keep the
narrow-width cast so the store width matches.

### Induction variable

```c
/* These may allocate differently: */
for (entry = entries; entry < end; entry++) { ... }

for (i = 0; i < count; i++) {
    entry = &entries[i];
    ...
}
```

Compare the induction-variable structure in the original assembly.

---

## Surviving dead code

Original source sometimes keeps calculations whose results are unused but
survive partial optimization; removing them changes register lifetimes and the
instruction stream.

```c
/*
 * MATCHING_AID:
 * The distance calculation and following dead branch survive partial
 * optimization in the original. Removing them shifts register allocation.
 */
distance = SquareRoot0((x * x) + (y * y) + (z * z));
if ((distance * 2) == 0) {
    distance = 2;
} else {
    distance *= 2;
}
```

Do not remove such code because a static analyzer calls it useless.

---

## Signedness

The default `char` signedness is set by the compiler profile. Mismatches
produce `lb` vs `lbu`, wrong sign-extension, and different constant folding.

Prefer explicit types in reconstructed structures:

```c
s8 signedValue;
u8 flags;
```

But also reproduce the translation unit's default `char` signedness, because
original code may use plain `char` in declarations.

When a function produces extra `andi` before a shift, check whether your
reconstructed field has a wider unsigned type than the original.

---

## Boolean spelling

These may compile differently, especially with narrow types:

```c
if (flag)
if (flag != 0)
if (flag == 1)
if (!flag)
if (flag == false)
```

Treat boolean spelling as a matching lever, not just a style choice.

---

## Representation views

An explicit representation view can be more accurate than bitwise operators:

```c
/* Emits lhu from memory. */
low = *(u16 *)&word;

/* Keeps value in register, emits andi. */
low = word & 0xFFFF;
```

Use these locally. Do not create a generic punning framework.

---

## Padding and alignment

Padding affects field offsets, BSS order, `$gp` reachability, section size,
following symbol placement.

```c
typedef struct Context {
    u8  state_00;
    u8  pad_01[3];
    u32 value_04;
} Context;

ASSERT_OFFSET(Context, value_04, 0x04);
ASSERT_SIZE(Context, 0x08);
```

For global-section padding, prefer an explicit symbol owned by the relevant
translation unit rather than a universal padding macro.

---

## COMMON, BSS, and symbol order

COMMON-section and MASPSX handling of uninitialized globals can reorder
variables; the linker may reorder COMMON symbols by name.

Typical symptoms:

- Code matches but global addresses do not.
- `$gp` offsets differ.
- BSS symbols are in a different order.
- A variable moves between `.bss`, `.sbss`, or COMMON.
- Renaming an unknown global changes the binary.

Controls include:

```text
-fcommon / -fno-common
-G0 / -G4 / -G8
--use-comm-section
```

Exact `.bss` / `.sbss` / `.data` / `.sdata` / COMMON control may also be
needed via the linker script or Splat layout.

---

## Shared read-only data

```c
/* Creates local .rodata — wrong if the original references a shared string */
static const char format[] = "%d";

/* Correct for a shared string: */
extern const char D_80051234[];
```

Making shared read-only data local creates phantom `.rodata` and changes the
object — critical for format strings, lookup/jump/animation tables, shared
vectors, SDK constants.

---

## Jump tables

When Splat separates code and `.rodata`, jump-table entries may reference
labels inside a function object; functions can look structurally wrong in m2c.

Solutions:

- Make jump-table labels global so cross-object references resolve.
- Prepend jump-table data manually to the function assembly:

```asm
.set noat
.set noreorder

.section .rodata
/* jump table */

.section .text
/* function */
```

A function can look wrong in m2c simply because its jump table was omitted
from the analysis context.

---

## Analysis context

m2c context should contain:

- Typedefs, structs, enums
- Prototypes and `extern` declarations
- Required macros and `static inline` functions

m2c context should usually **not** contain:

- Unrelated function definitions
- Unrelated local static data
- Unrelated `.rodata` (their inclusion can shift target rodata offsets)

See `bin/m2ctx` for the context generation command.

---

## Generated-assembly spelling

GNU `as` rejects some GTE compute mnemonics from disassembly pipelines. Use
exact `.word` encodings in generated assembly:

```asm
.word 0xXXXXXX
```

Use `.word` only in generated assembly or low-level SDK code, never as a C
matching technique.

---

## Allocation ladder

Direct MIPS register pinning (`register type name asm("$N")`) and
`INCLUDE_ASM` are **banned unless the user explicitly approves them** for a
specific function. After this ladder, the shared `REGISTER_PIN(type, name,
reg)` macro may be tried once as a bounded local experiment for an
asm-diff-proven allocator or entry-register residual. A bare numeric spelling
still needs proof the macro form alters codegen plus explicit user approval;
retention also requires independent review. Pins change the register web
globally and mask real causes. Ladder:

1. Correct types and declarations
2. Correct control-flow structure
3. Reorder declarations and statements
4. Introduce or remove temporaries
5. Hoist pointer dereferences
6. Try separate loop counter vs pointer induction variable
7. Use `barrier()` / `CLOBBER_*` for ordering and delay-slot placement
8. Check the compiler profile (`bin/flag-search`); if a non-canonical profile
   byte-matches clean C, record it in `config/compiler/object-flags.cmake`
   (per-object override) rather than pinning
9. Bind fixed-address symbols with `WEAK_SYMBOL_AT` in `symbols.c`, not
   `extern X asm("NAME")` renames
10. For an asm-diff-proven allocator or entry-register residual, make one
    bounded local `REGISTER_PIN` experiment; otherwise report the residual.
    `INCLUDE_ASM` still requires user approval.

A pinned local can stay live across the whole function and displace unrelated
variables — one pin can create new mismatches elsewhere. Each retained
`REGISTER_PIN` needs an adjacent `MATCHING_AID` rationale, independent review,
and a live exact byte match.

---

## Delay slots and entry copies

Classify the **first** live diff before changing source; a raw percentage
identifies no cause. The ignored `out/non-exact-lifts.json` audit is disposable
priority state — re-run the target's `asm-diff` and use the table below as the
durable choice of first lever.

| Observed first-diff shape | Diagnose first | First clean-C levers | Escalation boundary |
| --- | --- | --- | --- |
| Same byte size; one/few differences around `jal`, branch, or `j` delay slots | The exact original/current instruction, its live input/output register, and whether its value is needed after the transfer | Invert the branch, use early return vs result variable, reorder independent statements, introduce/remove one local, or adjust a pointer hoist | `CLOBBER_CALLER_REG` only if the evidence names a caller-clobbered register and placement; it must retain C-generated work, never select an opcode |
| Original begins `move tN,aN` or `move vN,aN`; current uses the argument directly | Whether the copied argument remains live across a call/branch or overlaps another temporary | Name one local copy at the original source lifetime; vary its declaration/first use and surrounding independent statement order | After all clean-C, profile, and one permuter attempt, one local `REGISTER_PIN(type, name, "tN"/"vN")` experiment is allowed only for this asm-diff-proven entry allocator residual |
| Frame/size differs at or before the first call | Exact prologue/epilogue, calls made, address-taken locals, aggregate assignment, and values live across calls | Correct prototypes and widths; remove accidental address-taking; split/collapse aggregate copies; choose early return/loop shape; shorten/extend a temporary lifetime | A pin never substitutes for an unmatched frame or changed control-flow shape |
| Same size; relocated address, `lui`/`addiu`, or load order differs | Symbol owner/declaration form, field offset, pointer-cell volatility, and whether a pointer is cached or reloaded | Pointer versus array; standalone symbol versus field; `PSX_REF`/`SPAD_PTR_SLOT` qualifiers; hoist or unhoist one dereference | `CLOBBER_*` only after the precise caller-clobbered reload ordering is proven |

Preinitializing a result can make GCC fill a desired branch delay slot, but it also lengthens that value's lifetime. Reject the variant when downstream allocation changes even if the slot now matches: placement alone is not a net improvement.

A sole commutative operand-order difference involving assembler scratch `$at` can survive typed-record arrays, byte views, pointer arithmetic, and extern-array indexing. GNU `as` preserves the compiler-emitted operand order; it does not canonicalize this encoding. After representation, profile, and permuter rungs are exhausted, record a compiler-order ceiling. Never pin or clobber `$at` to control it.

Partial lift: keep a residual note only when specific and durable —
command/target, first differing instruction(s), no-progress attempts, next
untried rung. No speculative TODOs. Third non-progressing diagnosed attempt:
restore best clean-C state, move on. Ladder exhausted: report that evidence
rather than looping.

## Approved `INCLUDE_ASM` fallback

Use `INCLUDE_ASM` only after explicit user approval for that function.
Without approval, leave its reviewed Splat segment as `asm` and report the
clean-C residual; never add an assembly-backed source stub.

See the [function-matching loop](matching.md) for the required iteration procedure.

---

## Permuter

Use `bin/permute TARGET@0xADDRESS --time-limit 60 -j N` for source-shape (60s hard cap per run;
search. Lessons:

- Fix structure and declarations before running it.
- A lower-percentage structurally clean source is often a better seed than a
  heavily pinned near-match.
- Register pins constrain the search space.
- It handles scheduling and some allocation changes, but cannot repair a wrong
  fundamental control-flow shape.
- Pointer-hoist mutations can unlock otherwise inaccessible allocations.
- Mark permuter-found aids with `MATCHING_AID`.

Recommended workflow:

```text
1. Reach semantic equivalence.
2. Match function boundaries and calls.
3. Match branches and loop structure.
4. Match loads, stores and relocations.
5. Match stack frame.
6. Match register allocation approximately.
7. Run permuter (`bin/permute` caps runs at 30s by default).
8. Inspect winning mutations.
9. Simplify the winning source.
10. Re-run to remove unnecessary hacks.
```

See `third_party/decomp-permuter/` for upstream documentation.

---

## Historical GCC catalog

The framework (`config/compiler/variants.json`, `bin/compiler-variants`)
manages historical GCC candidates. Four provenance-pinned candidates exist —
`gcc-2.6.3-psx`, `gcc-2.8.0-psx`, `gcc-2.8.1-psx`, `gcc-2.95.2-psx`
(SHA-256-verified) — but no object selects any: bounded probes were non-exact.
Research and negative records: `docs/specs/runtime/compiler-variants.md`.

When adding a candidate:

1. Verify SHA-256 of downloaded archive matches entry.
2. Test against one BOF3 function via `bin/flag-search` + `bin/byte-match`.
3. Confirm byte-match before adding to production catalog.
4. Update `toolchains/README.md §20` with provenance evidence.
