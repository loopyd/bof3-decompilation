# Matching playbook

A symptom-to-lever reference for resolving `bin/asm-diff` mismatches in BOF3
function lifts. Use after [matching workflow](matching.md) fundamentals are
confirmed (types, signatures, calls).

## Symptom-to-lever table

| Diff symptom | First things to test | Section |
|---|---|---|
| `lw` instead of address calculation | Pointer vs array declaration | [§2](#2-pointer-vs-array-declaration) |
| Wrong relocation symbol | Standalone symbol vs struct field | [§2](#2-pointer-vs-array-declaration) |
| Folded base register | Array/table access, semantic symbol | [§2](#2-pointer-vs-array-declaration) |
| `beq` vs `bne` | Invert `if/else` | [§3](#3-control-flow) |
| Wrong loop topology | `while`, `do`, `for`, guarded infinite loop, `goto` | [§3](#3-control-flow) |
| Wrong `$v0` return web | Early returns vs result variable | [§3](#3-control-flow) |
| Register swap or unexpected spill | Temp lifetime, statement order, pointer hoist | [§5](#5-temporaries-and-register-allocation) |
| Extra `andi` before shift | Verify reconstructed mask is necessary | [§7](#7-signedness) |
| Unexpected block-copy loop | Structure assignment | [§2](#2-pointer-vs-array-declaration) |
| Phantom `.rodata` | Local `static const` vs shared `extern` | [§12](#12-shared-read-only-data) |
| `lb` vs `lbu` | Field type and default `char` signedness | [§7](#7-signedness) |
| Wrong global/BSS offsets | COMMON, section, alignment, ordering, padding | [§11](#11-commonbss-and-symbol-ordering) |
| GNU assembler rejects GTE op | Exact `.word` in generated assembly | [§15](#15-gnu-as-instruction-spelling) |
| Proven incoming `a*` copied to `t*`/`v*` at entry | Preserve a local value's lifetime; after the ladder, one local `REGISTER_PIN` experiment | [§16](#16-register-pinning-ladder) |
| Same-sized near match with a lone delay-slot difference | Inspect the exact branch/jump and live operands; use clean-C ordering, then an evidenced caller-register clobber | [§17](#17-delay-slots-and-entry-register-copies) |
| Code size or stack frame differs | Check calls, address-taken locals, aggregate copies, temporary lifetime, and branch topology before allocator aids | [§5](#5-temporaries-and-register-allocation) |
| No clean C solution after levers | Record the exhausted evidence; do not add inline asm or `INCLUDE_ASM` without explicit approval | [§16](#16-register-pinning-ladder) |

---

## 1. Compiler profile verification

Before restructuring C for a stubborn function, verify the compiler profile.
Different targets — and sometimes different objects within one target — can use
different compiler versions, optimization levels, `-G` values, and signed-char
settings.

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

When `flag-search` proves an original was compiled at a non-canonical profile,
record it in `config/compiler/object-flags.cmake` so the object actually builds
with that profile instead of bridging to canonical `-O2` with C aids:

```cmake
set(BOF3_OBJFLAGS_emi_etc_game_01_func_801D0D5C_c -O1)
```

The key is the source path relative to `src/` with non-alphanumerics replaced by
underscores; the value is a `flag-search` candidate that replaces the canonical
`-O` level (the `-G0 -funsigned-char ...` base is kept). CMake and the
`compile_commands` generator both read this file, so the build and `flag-search`
stay in sync. Sources without an entry keep the canonical flags. Only add an
entry after `flag-search` reports an exact byte-match, then re-confirm with
`bin/byte-match`.

---

## 2. Pointer vs array declaration

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

---

## 3. Control flow

### Branch direction

Inverting an `if/else` is the most common control-flow fix:

```c
// Instead of:
if (condition) { HandleTrue(); } else { HandleFalse(); }

// Try:
if (!condition) { HandleFalse(); } else { HandleTrue(); }
```

This swaps `beq` ↔ `bne` and changes delay-slot scheduling and fall-through
reachability.

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

---

## 4. `MATCHING_AID` comment convention

Every artificial matching aid must be adjacent to the aid and say exactly what
it controls, the `asm-diff`-observed original/current instruction or register
placement, the ladder rung already exhausted, and what future evidence would
remove it. Retained `CLOBBER_*` and `REGISTER_PIN` aids must additionally say
that the immediately following live `bin/byte-match` was exact. Do not retain
an aid on a percentage improvement.

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

```c
/*
 * MATCHING_AID:
 * Hoisting the slot pointer keeps the index temporary in $a1.
 * Without this local, GCC allocates $v1 for the index and the store
 * at +0x34 uses a different base.
 */
slots = *slotTable;
```

Do not mark obvious workarounds (e.g., `barrier()` already has its own
convention in agents/lessons.md). Reserve `MATCHING_AID` for shape decisions that
are opaque to a reader without the matching diff.

---

## 5. Temporaries and register allocation

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

## 6. Preserve unused or partially optimized code

Original source sometimes contains calculations whose results are unused in the
final behavior but survive partial optimization. Removing them changes
register lifetimes and instruction streams.

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
/* scaledX and scaledY are dead in the final behavior */
scaledX = (x * scale) / distance;
scaledY = (y * scale) / distance;
```

Do not remove such code because a static analyzer calls it useless.

---

## 7. Signedness

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

## 8. Boolean spelling

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

## 9. Type punning

An explicit representation view can be more accurate than bitwise operators:

```c
/* Emits lhu from memory. */
low = *(u16 *)&word;

/* Keeps value in register, emits andi. */
low = word & 0xFFFF;
```

Use these locally. Do not create a generic punning framework.

---

## 10. Padding and alignment

Padding can affect field offsets, BSS order, `$gp` reachability, section size,
and following symbol placement.

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

## 11. COMMON/BSS and symbol ordering

The compiler's COMMON section and MASPSX handling of uninitialized globals can
produce different variable ordering. The linker may reorder COMMON symbols by
name.

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

## 12. Shared read-only data

```c
/* Creates local .rodata — wrong if the original references a shared string */
static const char format[] = "%d";

/* Correct for a shared string: */
extern const char D_80051234[];
```

Making shared read-only data local creates phantom `.rodata` and changes the
object. This is especially important for format strings, lookup tables, jump
tables, animation tables, shared vectors, and SDK constants.

---

## 13. Jump tables

When Splat separates code and `.rodata`, jump-table entries may reference
labels inside a function object. Symptoms include functions that look
structurally wrong in m2c.

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

A function may look structurally wrong in m2c simply because its jump table
was omitted from the analysis context.

---

## 14. Analysis context

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

## 15. GNU `as` instruction spelling

GNU `as` does not accept some GTE compute mnemonics emitted by disassembly
pipelines. Use exact `.word` encodings in generated assembly rather than
relying on the assembler to accept a non-standard mnemonic:

```asm
.word 0xXXXXXX
```

This is safer than depending on the assembler to encode the correct function
field, reorder, or interpret aliases. Use `.word` only in generated assembly
or low-level SDK code — not as a normal C matching technique.

---

## 16. Register allocation ladder

Direct MIPS register pinning (`register type name asm("$N")`) and `INCLUDE_ASM`
are **banned unless the user explicitly approves them** for a specific function.
After this ladder is exhausted, the shared `REGISTER_PIN(type, name, reg)` macro
may be tried once as a bounded local experiment for an asm-diff-proven allocator
or entry-register residual. A bare numeric spelling still needs proof that the macro form alters
codegen and explicit user approval. Retention also requires independent review.
Pins change the register web globally and
can mask the real cause. Escalate through this ladder first:

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

A pinned local may remain live across the whole function and displace unrelated
variables — pinning one register can create several new mismatches elsewhere.
Each retained `REGISTER_PIN` needs an adjacent `MATCHING_AID` rationale,
independent review, and a live exact byte match.

---

## 17. Delay slots and entry-register copies

Classify the **first** live diff before changing source. A raw percentage does
not identify a cause. The categories in
ignored `out/non-exact-lifts.json` audit is disposable priority state;
re-run the target's `asm-diff` and use the table below as the durable choice of
first lever.

| Observed first-diff shape | Diagnose first | First clean-C levers | Escalation boundary |
| --- | --- | --- | --- |
| Same byte size; one/few differences around `jal`, branch, or `j` delay slots | The exact original/current instruction, its live input/output register, and whether its value is needed after the transfer | Invert the branch, use early return vs result variable, reorder independent statements, introduce/remove one local, or adjust a pointer hoist | `CLOBBER_CALLER_REG` only if the evidence names a caller-clobbered register and placement; it must retain C-generated work, never select an opcode |
| Original begins `move tN,aN` or `move vN,aN`; current uses the argument directly | Whether the copied argument remains live across a call/branch or overlaps another temporary | Name one local copy at the original source lifetime; vary its declaration/first use and surrounding independent statement order | After all clean-C, profile, and one permuter attempt, one local `REGISTER_PIN(type, name, "tN"/"vN")` experiment is allowed only for this asm-diff-proven entry allocator residual |
| Frame/size differs at or before the first call | Exact prologue/epilogue, calls made, address-taken locals, aggregate assignment, and values live across calls | Correct prototypes and widths; remove accidental address-taking; split/collapse aggregate copies; choose early return/loop shape; shorten/extend a temporary lifetime | A pin never substitutes for an unmatched frame or changed control-flow shape |
| Same size; relocated address, `lui`/`addiu`, or load order differs | Symbol owner/declaration form, field offset, pointer-cell volatility, and whether a pointer is cached or reloaded | Pointer versus array; standalone symbol versus field; `PSX_REF`/`SPAD_PTR_SLOT` qualifiers; hoist or unhoist one dereference | `CLOBBER_*` only after the precise caller-clobbered reload ordering is proven |

For a partial lift, keep a short residual note in the function only when it is
specific and durable: command/target, first differing instruction(s), attempts
that changed no result, and the next untried rung. Do not fill sources with
speculative TODOs. At a rung's third non-progressing diagnosed attempt, restore
the best clean-C state and move to the next rung; once the ladder is exhausted,
report that evidence rather than looping.

## 18. `INCLUDE_ASM` fallback

Use `INCLUDE_ASM` only after explicit user approval for that function. Without
approval, leave its reviewed Splat segment as `asm` and report the clean-C
residual rather than adding an assembly-backed source stub. An approved clean
unmatched function allows:

- Incremental reconstruction
- Fully linkable intermediate builds
- Per-function progress tracking
- Avoiding low-quality fake matches
- Keeping hard hand-written assembly intact

See [matching workflow](matching.md) for iteration procedure.

---

## 19. Permuter gotchas

Use `bin/permute TARGET@0xADDRESS --time-limit 300 -j N` for source-shape
search. Key lessons from practice:

- Fix structure and declarations before running the permuter.
- A lower-percentage but structurally clean source is often a better seed than
  a heavily pinned near-match.
- Register pins constrain the search space.
- The permuter is good at scheduling and some allocation changes, but cannot
  reliably repair the wrong fundamental control-flow shape.
- Pointer-hoist mutations can unlock otherwise inaccessible register
  allocations.
- Matching aids found by the permuter should be marked with `MATCHING_AID`.

Recommended workflow:

```text
1. Reach semantic equivalence.
2. Match function boundaries and calls.
3. Match branches and loop structure.
4. Match loads, stores and relocations.
5. Match stack frame.
6. Match register allocation approximately.
7. Run permuter.
8. Inspect winning mutations.
9. Simplify the winning source.
10. Re-run to remove unnecessary hacks.
```

See `third_party/decomp-permuter/` for upstream documentation.

---

## 20. Historical GCC variant catalog

The framework (`config/compiler/variants.json`, `bin/compiler-variants`)
manages historical GCC compiler candidates. Four provenance-pinned candidates
exist — `gcc-2.6.3-psx`, `gcc-2.8.0-psx`, `gcc-2.8.1-psx`, and
`gcc-2.95.2-psx` (SHA-256-verified) — but no object selects any: bounded
probes were non-exact and none is claimed to match. Research and the negative
records are documented in `docs/specs/runtime/compiler-variants.md`.

When adding a candidate:

1. Verify SHA-256 of downloaded archive matches entry.
2. Test against one BOF3 function via `bin/flag-search` + `bin/byte-match`.
3. Confirm byte-match before adding to production catalog.
4. Update `toolchains/README.md §20` with provenance evidence.
