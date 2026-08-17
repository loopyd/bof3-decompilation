# Matching playbook

Symptom-to-lever reference after the [function-matching
loop](matching.md) set target, boundary, types, signatures, calls. Classify the
first live `bin/asm-diff` difference, one structural change, re-run; a
percentage alone is not a diagnosis.
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

Verify the profile before restructuring stubborn C: targets — sometimes
objects in one target — can differ in compiler version, optimization level,
`-G` value, signed-char setting.

```sh
bin/flag-search TARGET@0xADDRESS
```

Levers: compiler version/patch, optimization (`-O0`–`-O3`), `-G` value,
`-fsigned-char`/`-funsigned-char`, `-mno-split-addresses`,
`-fno-function-cse`, `-fno-schedule-insns`, `-fno-delayed-branch`, ASPSX
version, `--expand-div`, COMMON behavior (`-fcommon`, `--use-comm-section`).

When `flag-search` proves a non-canonical profile, record it in
`config/compiler/object-flags.cmake` so the object builds with it:

```cmake
set(BOF3_OBJFLAGS_emi_etc_game_01_func_801D0D5C_c -O1)
```

Key: source path relative to `src/`, non-alphanumerics as underscores; value:
a `flag-search` candidate replacing the canonical `-O` level (the
`-G0 -funsigned-char ...` base is kept). CMake and `compile_commands` read it,
keeping build and `flag-search` in sync; untouched sources keep canonical flags.
Add an entry only after an exact byte-match or a reviewed net partial
improvement (partial: record flag + live residual in
`@status`/`@match`/`@residual`; remove for no improvement or semantic/type
regression).

---

## Symbol representation

A declaration change can flip an address calculation into a load:

```c
extern Entity **g_entities;   /* lw of a pointer then offset */
extern Entity *g_entities[];  /* address of the array directly */
```

```c
g_state.currentMode = 4;        /* relocation to g_state plus offset */
extern s32 D_80123458;
D_80123458 = 4;                 /* standalone relocation */
```

Unexpected `lw` where the original calculates an address → try the
alternative declaration form.

### Extern rebinding: `addu` operand order and entry-copy theft

Replace a fixed-address macro with `extern Type D_XXXXXXXX[];` +
`WEAK_SYMBOL_AT`, indexed directly, when:

- **`addu` operand order.** C computing `table + index` makes GCC emit its own
  fixed-order `addu`; the extern-array form emits `lw table($idx)` and `as`
  expands `%hi`/`%lo` with canonical `lui $at; addu $at,$at,$idx`; remove any
  macro the rebinding strands.
- **Entry-copy theft.** A macro's constant address used twice can be CSEd
  into a callee-saved register, stealing an argument's prologue entry copy;
  the extern form emits `lui`/`lw` relocations, freeing the allocator.

---

## Control flow

### Branch direction

Inverting an `if/else` is the most common control-flow fix:

```c
if (condition) { HandleTrue(); } else { HandleFalse(); }
if (!condition) { HandleFalse(); } else { HandleTrue(); }
```

Swaps `beq` ↔ `bne`, changing delay-slot scheduling and fall-through order.
A chain inversion surviving every C spelling is gcc 2.7 `reorder_insns`
normalization; no flag disables it (`-fno-thread-jumps` is a proven no-op).
Escalate; do not churn spellings.

### Equal-valued branch arms

GCC may range-fold grouped cases or reverse/tail-merge an `if` chain. Before
changing shape, interpret every branch delay slot on both paths: a slot
constant may be the comparison value or selected result, making equal arms
semantically distinct. Try, one live diff each: ordered then nested/inverted
equality tests; separate ungrouped `case` bodies; duplicated arm
stores/returns; then shared locals or explicit labels. Keep the best semantic
shape; stop after three non-progressing variants (locals can alter the
register web). Proven on `emi/world00/area027/13@0x801F3650`: delay-slot
semantics made case 0 store 5, case 1 store 6, default nothing; separate cases
with an explicit empty default reproduced the shared-store tail exactly.

### Duplicate identical calls in `if/else` arms

When the original computes a value directly in `$a0` in each branch and
tail-merges into one `jal`, write `if (cond) f(x + A); else f(x + B);` — a
ternary argument computes before the call, breaking the merge.

### Early return vs result variable

```c
s32 result = 0; if (condition) { result = value; } return result;
if (condition) { return value; } return 0;  /* may coalesce into $v0 */
```

### Loop shape

GCC 2.7–2.8 rotates structured loops into bottom-tested forms; these may all
compile differently:

```c
while (condition) { ... }
do { ... } while (condition);
for (;;) { if (!condition) break; ... }
loop: if (!condition) goto exit; ... goto loop; exit:  /* top-tested CFG */
```

**Do not clean up a matched `goto` into a loop**; the form that produces
identical instruction bytes is correct.

A `do { ... } while` wrapper anchors loop-body scheduling when the
`for`/`while` rotation drifts. When the original preheader materializes
invariants late (`la` chains plus a `move` copying an entry pointer), assign
them *inside* the loop body; `loop.c` hoists them as late-created pseudos,
reproducing the preheader order and entry copy — hoisting before the loop
flips `addu` order and drops the copy.

---

## Volatility

`volatile` is a scheduling constraint, not documentation; add it only with
asynchronous/hardware-mutation evidence (lessons.md). On plain RAM globals it
is the most common partial-lift root cause. Levers:

- A volatile store never moves into a jump delay slot; if the original sinks
  a store there and yours sits after a `nop`, drop the unjustified qualifier.
- A volatile pointee on a narrow signed global can force `lbu` + manual
  sign-extension where the original has one `lb`; a local non-volatile view
  `*(s8*)&x` restores it.
- A volatile pointer *cell* (`Type * volatile`) forces a per-evaluation
  reload without constraining the pointee — the sanctioned form when the
  original reloads a shared cursor.
- Evidenced exception: `*((volatile T*)SYM + n)` can pin an original-proven
  store-before-volatile-store order.
---

## `MATCHING_AID` comments

Every artificial matching aid is adjacent to the aid and says: what it
controls, the `asm-diff`-observed original/current instruction or register
placement, the exhausted ladder rung, and what future evidence removes it.
Retained `CLOBBER_*`/`REGISTER_PIN` aids must also say the immediately following
live `bin/byte-match` was exact. Never retain an aid on a percentage
improvement.

```c
/* MATCHING_AID: produces the li $t2,2 feeding the next comparison; both
 * branches are identical; the condition keeps $t2 live and $a3 holding count.
 * Remove when $t2 allocation is understood. */
if (count == 2) { flags |= 0x20; } else { flags |= 0x20; }
```

Do not mark obvious workarounds (evidenced `barrier()` ordering is in
[Volatility](#volatility)); reserve `MATCHING_AID` for shape decisions opaque
to readers without the matching diff.

---

## Temporaries and allocation

### Pointer hoist

```c
value = (*table)[index];          other = (*table)[otherIndex];
Entry *entries = *table;          value = entries[index]; other = entries[otherIndex];
```

Hoisting can free the allocator to place unrelated values in the required
register.

### Expression splitting

```c
result = base[index].value + offset;
Entry *entry = &base[index]; s32 value = entry->value; result = value + offset;
```

Collapse instead if the original keeps everything in one register.

### Named constant reuse

```c
one = 1; if (value == 1) {
one = 1; if (value == one) {
```

GCC may reuse the register already holding `one`.

### Per-evaluation reload via fresh locals

Re-read a global cursor per case (`case A: f(*table);`) when the original
reloads per region — fresh reads reproduce its per-region reload registers.

### Force global read/store order with a local

m2c reorders independent global accesses (`flag = 2; counter += 0x14;` may
emit `sb` before `lhu`). Pin the original order with a local:
`count = counter; flag = 2; counter = (u16)(count + 0x14);`.

### Induction variable

```c
for (entry = entries; entry < end; entry++) { ... }
for (i = 0; i < count; i++) { entry = &entries[i]; ... }
```

These may allocate differently; compare the induction-variable structure in
the original assembly.

### Allocator-sensitive complex functions

Classify a same-CFG near match as allocator-sensitive when separate probes
(one narrow temporary, one split chained assignment, one removed constraint)
cause spills, frame/size changes, saved-register role changes, a
prologue-first mismatch, or a broad score collapse. Probes diagnose the search
neighborhood but do not authorize retaining a non-exact allocator aid.

For a classified function:

1. Work the first mismatch hunk first: move existing statements across
   adjacent dependency-safe lifetime boundaries before changing expressions.
   Run live `asm-diff` after every variant; continue while the first-mismatch
   frontier advances.
2. Classify as `retained-improvement`, `retained-frontier` (same score,
   later first mismatch), `reverted-neutral`, `reverted-local-regression`, or
   `reverted-structural-regression`. Rank: exact bytes, matching
   instructions, later first mismatch, fewer residual hunks, unchanged
   size/instruction count/frame, then smaller source disturbance. Keep a
   frontier candidate separately; test once with the best strict improvement;
   frontier movement alone is not completion evidence.
3. Abort and restore a nominally local variant when it unexpectedly changes
   frame size, function size/instruction count, spills, multi-block
   saved-register roles, or the prologue; do not tune that structural shape
   unless the original diff predicted the change.
4. Track `source_parent`, moved statement, crossed lifetime boundary, score,
   first mismatch, residual-hunk count, size, instruction count, frame size,
   retained changes, and interaction result. Requeue a reverted structural
   experiment only when a retained change affects its definition/last use,
   overlapping call, interfering saved value, frame, compiler profile, or
   owning residual block; an unrelated reorder is not invalidation.
5. Group source forms proven to emit identical instructions into an optimizer
   equivalence class (constant identities, commutative reversal, equivalent
   casts, normalized store order); retry within it only when profile, type,
   lifetime, volatility, or expression equivalence changes. When splitting a
   chained assignment broadly changes allocation, move the intact chain only as
   a unit; reusing a dead-looking local for an unrelated role is also a
   lifetime-changing experiment, not free storage.
6. After an existing `REGISTER_PIN` removal probe, record score/size/frame and
   allocator effects; a severe regression prevents redundant removal retries
   but does not waive the exact-match retention rule in
   [Allocation ladder](#allocation-ladder).

Queue at least two independent experiments plus one combination with the best
strict/frontier candidate when evidence supports a safe combination; restore
the best coherent state after each rejected variant. After an improvement,
allow three reviewed non-improving queues at the new frontier, then advance to
static allocation evidence, profile search, the permuter, or a recorded
compiler ceiling instead of broad spelling churn. A non-compiling C89 variant is not comparison evidence; repair declaration
placement within the same attempt, record only the compiled result.

---

## Surviving dead code

Original source sometimes keeps unused calculations that survive partial
optimization; removing them changes register lifetimes and the instruction
stream.

```c
/* MATCHING_AID: the distance calculation and following dead branch survive
 * partial optimization in the original; removing them shifts allocation. */
distance = SquareRoot0((x * x) + (y * y) + (z * z));
if ((distance * 2) == 0) { distance = 2; } else { distance *= 2; }
```

Do not remove such code because a static analyzer calls it useless.

---

## Signedness

The default `char` signedness is set by the compiler profile; mismatches
produce `lb` vs `lbu`, wrong sign-extension, and different constant folding.

Prefer explicit types (`s8 signedValue; u8 flags;`) and reproduce the
translation unit's default `char` signedness when the original uses plain
`char`. Extra `andi` before a shift → check whether your reconstructed field
has a wider unsigned type than the original.

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

Boolean spelling is a matching lever, not a style choice.

---

## Representation views

An explicit representation view can beat bitwise operators:

```c
low = *(u16 *)&word;   /* emits lhu from memory */
low = word & 0xFFFF;   /* keeps value in register, emits andi */
```

Use locally; do not create a generic punning framework.

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
translation unit over a universal padding macro.

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
static const char format[] = "%d";  /* local .rodata — wrong for a shared string */
extern const char D_80051234[];     /* correct for a shared string */
```

Localizing shared read-only data creates phantom `.rodata` and changes the
object — critical for format strings, lookup/jump/animation tables, shared
vectors, SDK constants.

---

## Jump tables

When Splat separates code and `.rodata`, jump-table entries may reference
labels inside a function object, so the function can look structurally wrong
in m2c. Fixes: make jump-table labels global so cross-object references
resolve; or prepend jump-table data manually to the function assembly:

```asm
.set noat
.set noreorder

.section .rodata
/* jump table */

.section .text
/* function */
```

---

## Analysis context

m2c context: typedefs, structs, enums, prototypes and `extern` declarations,
required macros and `static inline` functions; usually **not** unrelated
function definitions, local static data, or unrelated `.rodata` (inclusion can
shift target rodata offsets). See `bin/m2ctx`.

---

## Generated-assembly spelling

GNU `as` rejects some GTE compute mnemonics from disassembly pipelines; use
exact `.word` encodings in generated assembly:

```asm
.word 0xXXXXXX
```

`.word` only in generated assembly or low-level SDK code, never as a C
matching technique.

---

## Allocation ladder

Direct MIPS register pinning (`register type name asm("$N")`) and
`INCLUDE_ASM` are **banned unless the user explicitly approves them** for a
specific function. After this ladder, the shared `REGISTER_PIN(type, name,
reg)` macro may be tried once as a bounded local experiment for an
asm-diff-proven allocator or entry-register residual. A bare numeric spelling
needs proof the macro form alters codegen plus explicit user approval;
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
variables — one pin can create new mismatches elsewhere.

---

## Delay slots and entry copies

Classify the **first** live diff before changing source; the table below is
the durable first lever. The ignored `out/non-exact-lifts.json` audit is
disposable priority state.

| Observed first-diff shape | Diagnose first | First clean-C levers | Escalation boundary |
| --- | --- | --- | --- |
| Same byte size; one/few differences around `jal`, branch, or `j` delay slots | The exact original/current instruction, its live input/output register, and whether its value is needed after the transfer | Invert the branch, use early return vs result variable, reorder independent statements, introduce/remove one local, or adjust a pointer hoist | `CLOBBER_CALLER_REG` only if the evidence names a caller-clobbered register and placement; it must retain C-generated work, never select an opcode |
| Original begins `move tN,aN` or `move vN,aN`; current uses the argument directly | Whether the copied argument remains live across a call/branch or overlaps another temporary | Name one local copy at the original source lifetime; vary its declaration/first use and surrounding independent statement order | After all clean-C, profile, and one permuter attempt, one local `REGISTER_PIN(type, name, "tN"/"vN")` experiment is allowed only for this asm-diff-proven entry allocator residual |
| Frame/size differs at or before the first call | Exact prologue/epilogue, calls made, address-taken locals, aggregate assignment, and values live across calls | Correct prototypes and widths; remove accidental address-taking; split/collapse aggregate copies; choose early return/loop shape; shorten/extend a temporary lifetime | A pin never substitutes for an unmatched frame or changed control-flow shape |
| Same size; relocated address, `lui`/`addiu`, or load order differs | Symbol owner/declaration form, field offset, pointer-cell volatility, and whether a pointer is cached or reloaded | Pointer versus array; standalone symbol versus field; `PSX_REF`/`SPAD_PTR_SLOT` qualifiers; hoist or unhoist one dereference | `CLOBBER_*` only after the precise caller-clobbered reload ordering is proven |

Preinitializing a result can fill a desired branch delay slot but lengthens
that value's lifetime; reject it when downstream allocation changes (even if
the slot now matches).

A sole commutative operand-order difference involving assembler scratch
`$at` can survive typed-record arrays, byte views, pointer arithmetic, and
extern-array indexing; GNU `as` preserves compiler-emitted operand order and
does not canonicalize this encoding. After representation, profile, and
permuter rungs are exhausted, record a compiler-order ceiling. Never pin or
clobber `$at` to control it.

Partial lift: keep a residual note only when specific and durable —
command/target, first differing instruction(s), no-progress attempts, next
untried rung. No speculative TODOs.
## Approved `INCLUDE_ASM` fallback

Use `INCLUDE_ASM` only after explicit user approval for that function.
Without approval, leave its reviewed Splat segment as `asm` and report the
clean-C residual; never add an assembly-backed source stub.

---

## Permuter

Use `bin/permute TARGET@0xADDRESS --time-limit 60 -j N` for source-shape
search (60s hard cap; `--allow-long-run` is interactive-only). Lessons: fix
structure/declarations first (a lower-percentage clean source often beats a
heavily pinned near-match); register pins constrain the search space; it
handles scheduling/allocation changes, not wrong control-flow shape;
pointer-hoist mutations can unlock inaccessible allocations; mark
permuter-found aids with `MATCHING_AID`.



---

## Historical GCC catalog

`config/compiler/variants.json` + `bin/compiler-variants` manage four
SHA-256-verified candidates (`gcc-2.6.3-psx`, `gcc-2.8.0-psx`,
`gcc-2.8.1-psx`, `gcc-2.95.2-psx`). One object selects `gcc-2.6.3-psx`:
`src/bof3/audio/dispatchSoundCue.c`, byte-exact at
`exe/slus_004_22@0x8015DF18` (671/671, 2684 bytes).

Add a candidate catalog-first, probe second: add full provenance to
`config/compiler/variants.json` (`bin/compiler-variants list` validates);
`install <id>` (SHA-256 gate) then `verify <id>` (binary + `--version`);
probe with `bin/flag-search TARGET@0xADDRESS --compiler <id>` (exit 0 +
non-empty `exact_matches` = exact; exit 1 + empty = negative; exit 2/invalid
payload = probe failure); retain only with verified provenance plus a
recorded exact-or-negative probe. Selection is separate: a fresh exact match
plus a reviewed `BOF3_OBJCOMPILER_`/`BOF3_OBJFLAGS_` entry in
`config/compiler/object-flags.cmake`; procedure in
`docs/specs/runtime/compiler-variants.md`.
