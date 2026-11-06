# Matching knowledge

Follow the [match-loop](match-loop.md) ladder. Before address or
qualifier changes read [memory API](memory-api.md). Every lever is hypothesis until
owning live native comparison; keep best coherent PRE and full attempt ledger.

Branches: [first diff](#first-diff-decisions), [control flow](#control-flow),
[explicit cursors](#explicit-cursors-and-signed-bounds),
[operation scopes](#operation-scopes-and-loop-note-dependencies),
[delay slots](#delay-slots-and-entry-copies), [representation](#representation-and-volatility),
[allocation](#allocator-sensitive-functions), [orphan stack slots](#orphan-stack-slots),
[named scale products](#named-scale-products),
[repeated literals](#repeated-literals-through-one-narrow-value-local),
[compiler](#compiler-evidence), [boolean returns](#boolean-return-expansion),
[guard scheduling](#guard-scheduling-and-value-reuse),
[CSE and operand order](#cse-lifetimes-and-commutative-targets),
[narrow load sharing](#narrow-load-sharing-and-real-value-reuse),
[halfword read-modify-write](#halfword-read-modify-write),
[scratch cursor groups](#scratch-cursor-group-variables),
[shared and branch-local loads](#shared-and-branch-local-cursor-loads),
[runtime](#runtime-evidence), [storage](#storage-and-analysis-context),
[failure/capture](#failure-and-capture).

## First-diff decisions

| Symptom | Evidence and clean-C lever |
| --- | --- |
| Pointer load versus address calculation | Prove pointer versus array declaration |
| Wrong relocation/base folding | Standalone symbol versus reviewed struct field/array |
| `addu $at,index,$at` versus `addu $at,$at,index` | Named extern array can shift expansion to assembler; never pin scratch `$at` |
| Fixed-address constant steals entry copy | Named extern binding can prevent constant-address CSE; verify entire register web |
| Saved copy moves to entry or commutative operands stay swapped | Trace [CSE lifetimes and commutative targets](#cse-lifetimes-and-commutative-targets) |
| First mismatch stays at one entry-block offset through width, mask and cast probes | Reorder the loop-variable initializer statements (`i`, destination, source); declaration order does not move it. One measured order was exact, another moved the first mismatch later at a lower score |
| Extra sign extension or `andi` | Prove signedness/width/necessary mask; do not guess from decompiler. One measured source is a narrowed `volatile` cell read into a wider index, where reading the cell through a non-volatile byte view drops the mask |
| `lh` plus `lhu` replaces `lh` plus register copy | Trace [narrow load sharing](#narrow-load-sharing-and-real-value-reuse), known bits and later definitions |
| Byte-cell load hoisted above the record's word stores, extra live registers | Split the record view by access width: [record views](#scratchpad-cells-and-record-views) |
| Indirect call loads the selected pointer but the original fills frame slots with the whole table | Assign the named extern table to a local of its aggregate type, then index the local; indexing the extern indexes the table in place |
| Small block copy builds as per-word `lw`/`sw` or as a byte loop, original moves it with `lwl`/`lwr` plus `swl`/`swr` pairs | Give the moved bytes one byte-array struct and assign the whole block through it: an unaligned word-pair move is what GCC 2.7.2 emits for a block copy whose type has alignment 1, so a `u32` view of the same bytes stays on aligned loads |
| Frame/size mismatch | Correct calls/prototypes, address-taken locals, aggregate copies, lifetime and topology; trace unused frame bytes through [orphan stack slots](#orphan-stack-slots), then name repeated scale products (#named-scale-products) |
| Pointer read hoisted above a call, extra saved register, frame unchanged | Bind the call-derived scaled value to its own local before the pointer read; folding the mask/shift into the later address expression keeps the pointer live across the call |
| Call result stored inside the next call's delay slot | The original stores a `jal` result into its outgoing argument slot right after that call, before the next call's argument setup. Bind the result to a local; nesting the call in the later argument list moves that store into the following `jal` delay slot |
| `beq`/`bne`, return web or tail merge | Invert branches, early return versus result local, duplicate branch calls |
| Two bit tests on one word collapse into a single combined mask compare | `&&` on two distinct bits lets GCC merge them into `(x & (a\|b)) == a`; write nested `if`s to keep both tests. The merge needs one loaded word: two non-zero tests on different byte fields of a record stay two loads, so keep that conjunction as written |
| Both arms truncate the same value, then differ only in a constant, and the current build masks once before the branch | Repeat the truncation as its own statement inside each arm (`angle &= 0xFFFF;` then `angle += bias;`). One combined expression lets GCC share the mask across the arms and fill the branch delay slot, which came out two instructions short with identical memory events; the split arms keep a per-arm `andi` and the original's unfilled slot |
| Three-term sum stays one instruction long, or the first divergence sits at a later term | Sweep the three associations `a + (b + c)`, `(a + b) + c`, `(b + c) + a` before touching guards, casts or truthiness spellings of that sum; the mirrored nesting stalled one instruction short with the divergence pushed to the last term, and the re-spellings measured neutral |
| Boolean result misses the return register; extra return copy | Trace [boolean return expansion](#boolean-return-expansion) before allocation |
| Loop topology | Compare while/do/for/guarded loop/explicit labels; matched goto is not style debt. For displaced induction setup, inspect [explicit cursors](#explicit-cursors-and-signed-bounds) |
| Global/BSS offsets | Audit COMMON/section/alignment/order/padding and following symbols |
| Phantom read-only object | Local static constant versus existing shared extern ownership |
| Load uses wrong argument register | Verify callee argument consumption and caller setup before allocator probes |
| Missing read after a store or an intervening call | Recheck pointer reloads, source qualification and aliasing against original instructions. A call that writes shared state, a formatter writing a shared buffer for instance, does not license holding one loaded cell in a local across it: two loads in the original are two accesses, so write the later use through the same address again |
| Saved-register pair exchanged | Trace real value deaths, later reuse and merged allocator quantities |
| Guard load/test displaced around a product/store chain | Trace [guard scheduling and value reuse](#guard-scheduling-and-value-reuse) before changing arithmetic |

Named extern-array indexing can emit folded `lw table(index)` and assembler
`lui $at; addu $at,$at,index`; fixed-address arithmetic may emit compiler's opposite
operand order. Neither representation universally wins. Known RAM pointer-valued
global needs narrow evidenced local extern, not anonymous macro hiding ownership.

## Control flow

Interpret every delay slot on both paths before changing equal-valued arms; slot
constant may be comparison operand, not selected result. Bounded matrix: ordered
then nested/inverted equality; separate ungrouped cases; duplicated arm stores/
returns; shared locals or labels. Advance only when measured evidence rejects
eligible alternatives. Locals may change register web. GCC can normalize
chain inversion regardless of spelling; another spelling alone is not new evidence.

Branch calls `if (cond) f(x+A); else f(x+B);` can merge into one `jal` where ternary
argument does not. The same holds for identical argument lists: writing the call in
both arms can produce the single original call site through cross-jumping, and it
keeps the per-arm argument setup the original shows. Early returns versus initialized
result changes `$v0` lifetime. When the guard wraps calls and stores and each side
ends in a different constant return or constant store, put the work inside the `if`
with the constant arm after the block: the work arm goes out of line behind `beq` while
`j <epilogue>` keeps `move $v0,$zero` in its delay slot. Read the arm order off the
original as well: the arm the function falls into is the `if` body, and the arm the
branch jumps to is the `else` or the block after an early return. Reversed arms keep
frame and size and only invert the test and its target, so a `bnez` where the build
emits `beqz` is this ordering choice, not a later rung. A result local live across the
calls cost a saved register and frame bytes, and an early `return B;` before the block
relocated that return out of line instead - `.text` size stayed equal and only the
branch shape differed (`bne` to a trailing `move $v0,$zero`).
GCC loop rotation can move invariants: assigning inside loop body can permit late
hoisting/preheader copies; moving before loop can lose them. Keep only measured
semantically correct shape; record negative experiments before trying another shape.
When the original's entry branch carries the loop index's zeroing in its delay slot,
put the `index = 0;` statement before an explicit `if (count != 0)` guard and keep the
body as `do`/`while`: the count is an unrestricted byte, so the guard is required, and
the `do` supplies the latch compare the original shows.

A shared test after value-producing arms extends a local across the join. Test
per-arm checks branching to the existing common operation. For tail-merged stores,
express the original subtraction, saturation and capped paths explicitly; retain
sentinel exits and each path's memory effects. Duplicate no calls or volatile reads.

For indexed fields, match the original calculation's branch boundary. A partial
stride such as `four = slot << 2` before the branch, completed as
`(four + slot) << 6` inside each arm, can preserve folded field addressing where
hoisting the full stride does not. Prove unsigned range, shifts and equivalence to
`slot * 0x140`. Share only until an intervening store/call can change the index;
preserve original post-store rereads. Do not add volatility to force them.

For preheader or delay-slot order, test block-local initialization or evidenced
[operation-scope placement](#operation-scopes-and-loop-note-dependencies).

Write a two-sided range guard as the inclusion conjunction the original branches on,
`(x >= lo && x <= hi)`, not the exclusion disjunction `(x < lo || x > hi)`: the
exclusion form emits the inverse test and swaps the arms behind it while size stays
equal, so a wrong tail arm order there is a symptom of the range spelling rather
than a second defect. Masks and a re-typed narrow index on that compared value
measured neutral at the same residue.

A guarded dispatcher (`sltiu` bound, table base, one indexed load, `jr`) gives the
dense switch shape directly: one arm per table word, in order, and the arm an
out-of-range index reaches is the default, so that case carries the default's
statements instead of a duplicate block. When the first seed already builds to the
original `.text` size and differs only at address-bearing words, the source shape is
settled; finish the symbol bindings and the reviewed table placement before probing
more C.

### Operation scopes and loop-note dependencies

Use when real initialization has the required lifetime but adjacent call arguments
schedule incorrectly. Keep owning compiler/flags fixed; inspect both scheduling
passes before changing scope placement.

1. In GCC 2.7.2 `sched.c::sched_analyze_insn`, trace `LOOP_BEG`/`LOOP_END`
   handling. Loop notes can attach dependencies to the next instruction: pending
   uses/sets gain edges, `reg_pending_sets_all` marks every register set, and
   memory lists flush. An apparent output dependency between unrelated arguments
   may come from this boundary, not ABI or register overlap.
2. Test a single-execution scope around actual work, for example:

   ```c
   do {
     size = frame_bytes;
   } while (0);
   install_callback(callback);
   start_stream(buffer, mode);
   ```

   Move the intact operation scope across a call only when value independence,
   address escapes and every path permit it. Preserve call, volatile and memory
   effects. Initialization needs a real later consumer. Empty scopes, dummy work,
   extra accesses and barrier substitutes remain forbidden.
   A scope may hold one real store rather than an initializer: its `LOOP_END` note
   flushes the pending memory lists, which keeps a following call's reload of an
   earlier-read cell after that store; without the scope the bare store lets the
   reload drop into the first load's delay slot.
3. Compare plain initialization, PRE placement and proposed placement through
   `lift sweep`. Plain form may rematerialize a constant or drop saved registers;
   another placement may retain allocation while freeing argument order. Bind
   `.sched`/`.sched2` to source hashes and verify the dependency change.
4. Check the whole function, frame and readonly bytes with the live gate and
   independent semantic review; freeze during review. Scope necessity or one
   corrected pair alone proves neither admissibility nor exactness. Keep the
   mechanism profile-specific, not proof of original source spelling.

### Explicit cursors and signed bounds

Use when loop accesses match but induction setup, entry guard or bound compare
differs. Diagnose before changing statement order; keep compiler/flags fixed.

1. Bind RTL to source hashes. In GCC 2.7.2, trace `loop.c` strength reduction
   and invariant insertion, then `.sched`/`.sched2`. An index loop can acquire
   pointer setup after another initializer; scheduling may preserve that order.
   Keep every dump out of the repository: `cc1 -da`/`-dR` writes `<basename>.*`
   into the compiler's working directory. `-dumpbase` takes a bare base name, so a
   `/tmp/...` path there does not move the files — run the compile from `/tmp`, or
   move the dumps immediately. Four lanes have leaked dumps to the repo root this way.
2. Test an explicit cursor and endpoint. Use `do` only when original control
   flow and the complete input domain prove at least one iteration. Prove
   start, stride, count, final cursor and parallel indices, including exit and
   `continue` paths. Preserve conditional loads, byte truncation and aliases.
3. If the cursor form loses an original address copy, trace CSE and allocation.
   After deriving accumulator pointers, test reusing the consumed address scalar
   for a real kind value. Preserve every reload; add no dummy use or alias view.
4. Match compare semantics, not just loop length. Pointer `<` may emit `sltu`
   where original cursors use `slt`; `!=` may remove the compare. Test signed
   address casts only after proving target pointer width/conversion, ordered
   endpoints within one signed region, reachable stride progression and no
   wraparound. Re-prove these bounds for relocated copies; never cast solely
   to select an opcode. A wrap test belongs to the compare, not the declaration:
   when the original decrements a counter and branches on a signed below-zero
   result, write `(s16)field < 0` and leave a shared unsigned declaration alone,
   because an unsigned compare never reaches the wrap arm and a signed
   re-declaration re-types every sibling handler. A latch that zero-extends the
   counter with `andi` before an unsigned `sltiu` bound is the same placement. The
   compare reads the counter's low byte, so keep the counter wide and write
   `((u8)i < 10)` at the test.
5. Confirm predicted setup, entry test and compare through `lift sweep`, then
   full function/readonly-byte gate and independent review. Freeze during review.
   Correct setup alone proves neither semantics nor exactness; this lever does
   not establish original C or universal compiler behavior.

## Delay slots and entry copies

- Same-size branch/`jal`/`j` difference: identify exact instruction, live inputs/
  outputs, value use after transfer. Try branch inversion, early return, independent
  statement order, one local or pointer hoist.
- Original `move tN,aN`/`move vN,aN`: prove lifetime across call/branch or overlapping
  temporary, then local copy at that lifetime with bounded declaration/first-use order.
- Frame mismatch: solve prototype/width/call/aggregate/address-taking first.
- Relocation/load order: owner, symbol form, offset, pointer-cell volatility and
  cache/reload evidence first.

Preinitialization that fills one slot but damages downstream allocation is rejected.
Sole commutative `$at` operand-order residual may survive representation and
compiler-informed clean-C work. Assembler need not canonicalize operand encoding.
Preserve the unresolved result; opt-in work remains untried, not exhausted.

## Representation and volatility

Volatile store cannot move into jump delay slot. A volatile pointee also forces the
post-store test to reload, so a field the original re-tests from the register it just
stored - a saturation arm testing the sum it wrote - needs the plain view. Narrow
volatile pointee may cause `lbu` plus manual sign extension versus original `lb`.
Remove unjustified qualifier only with semantic evidence; reviewed plain view may
restore shape. Volatile pointer cell forces reload without qualifying pointee; use
API reference. Never add volatility as
arbitrary scheduler control. The shared scratchpad work cell is the one place this is
already decided: five byte-exact sources declare it `u8* volatile`, so when the original
keeps consecutive handoff stores that no call separates, declare that cell volatile rather
than inventing a barrier. Cite the siblings when you do, and leave any other cell alone.
Original-proven ordering remains explicit evidence. Where a store through the loaded
pointer already separates the reads, the plain spelling measured identical bytes on that
same cell, so the qualifier is not what carries the reload there.

Signedness belongs to the use site, not the cell: read it off that site's load. One
byte cell can need `*(s8*)&cell` where the original emits `lb` in a branch test and an
unsigned read where another function emits `lbu` on the same address, so a shared
header's `u8` scalar fixes the address without proving the signedness. Declare the
signed view locally at the use that shows `lb`.

Unaligned fields/padding/SDK stack types follow type-application reference. For
pooled member addresses, compare direct access through the existing member with
an initialized pointer local serving one operation. Direct member access can retain
folded offsets where a reused pointer materializes the cell address. Compare plain
and single-execution scopes; loop notes can change CSE/combine as well as scheduling.
Preserve cell/pointee types, qualifiers, evaluation count, aliases and reloads after
stores. Do not replace the owning aggregate, add cell aliases or qualify accesses
solely for matching. Check the whole function and frame, not the first fixed load.

Match local views to existing storage and consumers. A mutable source view over a
mutable byte table can change read-after-write code generation without adding
volatility. `const` does not imply non-aliasing. A pointer-cell reload after a store
may correct an earlier reconstruction rather than preserve PRE under every alias.
Prove original accesses and final linked bytes; make no universal alias guarantee.

Use established halfword/aggregate views when field widths and consumers support
them. Preserve public constness and ABI unless caller/callee evidence supports a
change. Bitfields require target layout and codegen proof, not guessed packing.
Regroup unsigned address sums through a row/base temporary when original addition
order differs; prove value equivalence and preserve all memory events.

Pin a reload where the original reads it with a typed value local. Reading a cell
straight into the store keeps the load next to that store, while assigning it to a
`u8` local at an earlier statement pins the load at that statement and leaves the
store later, which is the shape a record-arming block shows. A pointer local instead
defers the dereference, so choose the local's type by which side of the load the
original fixes.

Keep a narrow parameter narrow. Copying an `s16` argument into an `s32` local moves
the sign extension to the entry block, where the original converts at each use site.
Read the parameter directly and cast at the use instead. The formal's own width
decides this: a wide parameter cast to `s16` at each use is a different shape that
moved the entry block, frame and size, so declare the narrow formal and leave the
body's uses alone. The mirror case is a cast used twice: `(s8)value` in both the call
and the comparison lets one conversion serve both, while reassigning
`value = (s8)value;` between them duplicates it.

Mirror the width rule at the assignment: when a narrow callee return feeds a narrow
cell, keep the accumulator local at the cell's width. A wider local converts the
return where it is assigned and costs an `sll`/`sra` pair the original does not have,
while the narrow local leaves the truncation to the store.

### Halfword read-modify-write

Use when a scratch halfword is decremented or advanced and then compared, or when
adjacent fields of one record are updated in a row.

1. Address the field through a plain `s16*` and keep the arithmetic in place:
   `*half = (s16)(*half + delta);` then compare `*half`. The add stays on the
   zero-extended load, the store truncates, and the sign extension happens after
   the store. That reproduces the usual `lhu`, `addiu`, `sh`, `sll`/`sra`, `slti`
   sequence.
2. When the cell is volatile and the original does not reload it, keep the
   decremented value in an `s32` local and cast at both uses:
   `value = *(volatile u16*)(p + n) - delta; *(volatile s16*)(p + n) = (s16)value;`
   then `if ((s16)value < limit)`. The store then precedes the sign extension.
   Widening without the comparison cast, or storing the uncast value, moves the
   extension ahead of the store. An `s16` local is still wrong: GCC copies it with
   `move` and emits a register `slt` instead of `slti`.
3. When the original stalls on its first load, or keeps two cursor registers,
   read the cell again into a second pointer variable placed after the first
   store. See [scratch cursor group variables](#scratch-cursor-group-variables).
4. Update adjacent fields with one in-place read-modify-write each, in the
   original store order. Binding every loaded value to a local first measured a
   partial whose first divergence was the first add, because the separate locals
   let GCC hoist all the loads above the whole add chain; the all-in-place form
   matched on the next probe. Once every update is in place, the remaining store
   positions are free: interleaving the countdown store between two updates also
   matched.
5. Confirm with live asm-diff and the whole-function gate before retaining.

Byte cells take a `u8` accumulator instead:
`countdown = (u8)(p[n] - 1); p[n] = countdown; if (countdown == 0)` is the form
whose `sb` is followed by `andi 0xff` and `bnez` on that stored register, with no
second load of the cell. The exact countdown handlers across these overlays all
use it, so the truncation, not a reload, supplies the compared value. The test's
signedness picks that extension: `if ((s8)countdown == 8)` on the same `u8`
accumulator emits `sll`/`sra` of the stored register before the compare, so a test
against a signed-looking channel value takes a cast at the test and leaves the
accumulator `u8`. A cell whose test has its own load takes the `s8` view on the cell
expression at that test instead.

### Scratch cursor group variables

Use when a function re-reads or re-derives a base before each store or call group and
the current build keeps one register for all of them, whether that base is a pointer
cell load or a record address computed from a cursor.

1. Read the cell into its own variable at each original reload point. One variable
   reused across groups becomes a single global pseudo, and that pseudo conflicts
   with the registers the original used across its whole range. The same split
   applies to per-group record temporaries: a cursor shared by two store groups
   kept both in one register and left the block's tail mismatched.
2. Read the allocator dump before changing spelling. `.greg` reports the number of
   global pseudos, each conflict set and the register each pseudo receives.
3. Give the variables distinct names. GCC then keeps separate quantities, so each
   can take the register the original used. Renaming one variable in place does not
   split it.
4. Place each reload where the original has it. A second read after the first
   store also reproduces a load-delay `nop` that a merged read removes.
5. Confirm with live asm-diff. Register choice inside later groups can still
   differ; the dump explains which quantity moved.
6. Count the original's groups before naming variables. Splitting groups one at a
   time raised the score at that group's first mismatch and no further. Once every
   group has its own variable, a remaining tail mismatch is usually a value width
   on the indexed row, not another cursor.

This split also repairs a partial, not only a seed. Once the branch layout matches, a
residual that puts one call group's reload cursor in the neighbouring register while
byte count and frame already agree is this lever, not another arm spelling. Splitting
only the mismatching group measured the same bytes as splitting every group, so keep
the narrowest split and spend no variants on the rest.

### Shared and branch-local cursor loads

Use when the body and size match but a branch target is off by eight bytes, or when
the original reuses one cursor load across stores that the current build reloads.

1. When a byte store and the following word stores in the original share a single
   cursor load, write both through one local pointer. Reading the volatile pointer
   cell directly reloads it for every use, which adds a `lui`/`lw` pair per store.
   The mirror case is a cell the original reloads per use with an unfilled load
   delay: keep that access inline through the named cell, because a pointer local
   placed ahead of the neighbouring work raises the load, fills the slot the original
   leaves as `nop`, and moves the branch target earlier by one nop per group.
   One function can need both spellings: when the original re-materialises the cell
   for its leading statements and then shares one load for a later run, keep those
   leading statements on the named cell and open the single pointer local at the
   original's first shared load. The switch is a statement in the body, not a
   function-wide choice.
2. When a reload runs on one arm only, make it the last statement of that arm.
   Written after the `if`, both arms share it and the branch target lands eight
   bytes before the original target. Size stays equal because the instruction count
   does not change; only the target exposes the difference.
3. Judge argument width from the mask. `andi 0xffff` means a `u16` parameter; a
   cast to `s16` emits `sll`/`sra` instead. Widening a shared prototype is safe when
   the existing exact callers still pass whole-number constants.
4. For a run of consecutive word clears that shares one cursor load, take a `u32*`
   local and index by word: `words = (u32*)D_x; words[24] = 0u;`. A `u8*` local with
   byte offsets emits the same stores yet blocks the delay-slot fill of the call
   that follows the run, leaving a `nop` where the original has the last store.
5. Store a negative byte constant through an `s8*` pointer. `(u8)-100` folds to 156
   and emits `li ...,156` where the original has `li ...,-100`.

### Scratchpad cells and record views

A bare constant scratchpad address is CSE'd into one register: `lui` plus `ori` once,
then `lw 0(vN)` per access. When the original re-materialises the constant per access,
name the cell: declare an extern at its fixed address so each access emits its own
`lui 0x1F80` plus `lw 68(vN)`. Applies to any fixed-address data the original reloads
per use; the named symbol in the map restores that repetition.

Pick the access spelling from the original's hi/lo pair, not from symbol ownership:
`lui` plus `ori` is an integer-constant address, while a map symbol emits
`%hi`/`%lo`, folds into the load when the offset fits and otherwise builds its base
with `addiu`. A cell that owns a map row can still need a literal accessor at one use
site; the hi/lo pair and base register in that load separate the two forms before
another local arrangement is tried.

A shared header that maps a cell name to a macro over a constant address
(`#define D_X g_Cell`) folds the same way, once, because the macro hides the
symbol. `#undef` it in the source when the original reloads the cell per access and
read the target-local symbol instead; unconditional `#undef` at outer body level is
admissible for an ordinary function. Sibling sources on more than one target closed
a hoisted-base residual here, so reach for this before allocation work.

Stores follow the same rule. Each `sb` to a distinct fixed-address cell carries its own
`lui $at, %hi(sym)`, so declare one named extern per cell. That holds for one cell read
and written back: keep `cell = cell - 1` as the bare symbol expression, which folds the
read into `lui`+`lbu %lo` and re-materialises `$at` for the store, because a pointer
local gives that read and store one shared base instead. A build that holds one base
register across the run instead has a shared base or aggregate view the original
lacked. Read the store's base register to pick the spelling. A store that
re-materialises `$at` wants the bare symbol expression, while a store through the
base the load already left in a saved register wants one pointer local for both
accesses, even when the load sits in a guard test and the store in its arm. Inside
one block, stores keep source statement order, and a constant re-materialised into
one register between stores shows where its statement starts.
A run of stores to one pointer cell that parks a different record and restores the
saved value is real state: keep every publish and restore as a store to the cell and
re-read the cell after the restore, because holding the record in a local drops the run.

Reads follow it in the other direction. When the original materialises a fixed-address
symbol once into a saved register and re-reads one byte of it before each call,
declare a local `u8 *` to that symbol so one base serves every read. Passing the symbol
expression to each call instead emits its own `lui %hi`/`lbu %lo` pair and loses the
shared base. One read needs the same local when a store to another cell sits between
the base and the read: the pointer copy takes the base load early enough to fill the
preceding delay slot, while the inline symbol expression materialises the base after
that store and costs two instructions. A cell that is loaded, stored and re-armed
inside a guard wants it too: one base for all three uses is what lets the original
keep the re-arm constant in both branch delay slots. A local that only re-types a promoted value
(`u16 row = y;`) folds away and changes nothing.

Give a work record a word-typed view before chasing pointer roles. A typed pointer over
the same storage fixes which value the pointer holds and where the second cell load
lands; a byte view leaves both ambiguous, so the gap shows up as a displaced load rather
than a wrong one. Check the original access order before widening the view.

Split record views by access width, not only by type. When word-pair updates match but a
byte cell's load is hoisted above them, keep the word pairs on one word-typed pointer
and reach that byte through a `u8*` derived from it:
`words[13] += words[3]; ((u8 *)words)[9]++; words[14] += words[4];`. Correct store
order alone still hoisted the byte load and kept extra live registers; the split view
closed it, so this drift needs the wider representation change rather than another
reorder. The same accesses read through signed words emit identical bytes, so
signedness is not that lever.

### Signed table index feeding a scaled offset

When a signed table load feeds an index multiply, GCC 2.7.2 can allocate the offset and
the value to the opposite registers from the original (`v0`/`v1` exchanged at each
read-modify-write arm) while everything else matches. Three separate lanes reached this
residual from different starting shapes, each retrying lvalue forms and each landing on
the same exchange. Treat it as an allocation fact rather than a source-spelling problem:
record the exchange and the arm count, and do not spend further candidates on it. The
orphan frame that often accompanies it *is* source-shaped - reusing the computed offset
as the next record address, as the exact sibling does, removes it.

## Allocator-sensitive functions

Same-CFG near match plus narrow probe causing spills/frame/size/save-register/
prologue changes indicates allocation sensitivity. Follow the match-loop ledger
and exhaustion gate. Move intact statements across dependency-safe lifetime
boundaries before expression churn. Classify retained improvement, retained frontier,
reverted neutral/local regression/structural regression. Rank exact bytes, matched
instructions, later first mismatch, fewer hunks, unchanged size/count/frame, then
least source disturbance. Frontier alone is not completion; keep it separately and
combine with best strict improvement only with semantic evidence and live comparison.

Unexpected frame/size/spill/multi-block/prologue drift means restore, unless predicted
by original diff. Ledger retains source parent, moved statement/crossed lifetime,
score/first mismatch/hunks/size/count/frame, retained changes and interaction result.
Reopen rejected shape only with changed definition/last use, overlapping call,
interfering saved value/frame/residual block. Equivalent emitted forms share
one optimizer class; retry only when representation, lifetime or supported access
evidence changes. Chained assignments and local reuse change lifetimes.
Unsupported probes/compile failures never prove exhaustion.

Preserve original surviving calculations even if analyzer calls them dead. Caller
`andi v0,v0,0xFF` proves low-byte consumption, not narrow callee ABI; retain evidenced
full-width return and explicit caller cast, verify both. Boolean spelling can alter
narrow-type code. Local representation view may yield memory `lhu` instead of
register `andi`; no generic punning framework.

Trace each local from definition through its final consumer and address escapes.
After its value is consumed, test reusing it for a later real helper result or
calculation with a compatible type. Preserve call order, signed multiplication,
shifts and narrowing. The new result must have a genuine consumer. When the residual
is a tail register exchange, write that result back into the same local before its
consumer: a fresh local leaves both quantities live and the exchange survives, while
the self-assignment merges them. Mask, shift and operand-order spellings of that tail
stall at one hunk with the size already exact, so the residual belongs to this rung,
not to another shaping attempt. Never add dummy uses to inflate liveness. Compatible
locals can also serve mutually exclusive arms when each arm defines before use and no
value escapes the join. Split phase values when their interference differs; share a
consumed scalar only with a real later consumer. Review the whole allocator output,
not one register pair.

A head exchange is the same rung with the opposite fix. When size, frame, memory
events and every instruction agree except that two locals of one type hold each
other's adjacent registers from the first divergent instruction, treat it as a
reference-count ordering fact: read `.lreg` counts and `.greg` order, then give the
local that must take the earlier register a second genuine use - load the raw narrow
value into that same local and compute the widened result from it - so its count
rises and the allocation order reverses. One lane spent its whole shape sweep on
this exchange and closed it on the compiler-evidence rung alone with that one change.
Declaration-order swaps, a named limit constant and respelled clamps measured neutral
at the same score, so they are not candidates for it.

A narrowed induction temporary needs a complete reachable-range proof, including
the final endpoint. A higher score at the same size can be a retainable partial
while introducing a different residual. More matched instructions with missing
original bytes is not exactness; inspect conversion, mask, frame and flow changes.

### Orphan stack slots

Use this lever when the body matches but the frame differs, with no emitted body
access to the extra bytes. First exclude ABI, call, aggregate and address-escape
errors. In GCC 2.7.2, a folded address can leave an internal `USE` of a pseudo
whose definition disappeared. Reload may allocate a slot for that pseudo even
though the `USE` emits no instruction.

A bare prologue and epilogue with no saved register, every instruction outside them
already matching, and an original frame larger than this body's own locals is a
second cause: the reservation comes from a declaration the original translation
unit carried that emitted no code. One declared local of the missing size restores
the frame, and it takes no use, since a use would change liveness instead. The bytes
constrain only that size, so the name and type are a reconstruction choice rather
than recovered source: flag the declaration for review and record what removing it
leaves, which is the frame and offset residual with the body otherwise matching.

1. Bind before/after dumps to the source and unchanged compiler settings. Trace
   the address definition, its death note and surviving `USE` through combine,
   allocation and reload. Confirm a stack-slot `USE` and the extra frame bytes;
   a frame-size difference alone does not establish this cause.
2. Reuse one compatible scalar after each value's final consumer, for example
   store address, next store address, then a later real row index. Preserve each
   original reload, store and call. For address/index reuse through `u32`, prove
   the target's pointer width, lossless round-trip, alignment and unsigned offset
   arithmetic. Do not narrow an address or invent an aggregate to steer aliasing.
3. Check whether the later numeric consumer gives the local a hard register.
   An internal `USE` may remain; it must no longer require an orphan stack slot.
   Pointer-only reuse may reduce slots without removing the last one. Typed
   indexing can instead change alias analysis, reloads and the instruction body.
4. Measure through `lift sweep`, then the owning live gate and independent review.
   Require the original frame, complete function bytes and reviewed readonly data
   for exact acceptance. Preserve semantic and memory-event equivalence; smaller
   frames or fewer `USE`s alone are not acceptance evidence.

### Named scale products

Use this lever when a repeated index scale such as `span * 3` sits inline in a load
and an address, and the difference is outside the body you are shaping: a larger
frame, a saved register or hoisted base, or a shifted induction setup. The unnamed
product can force a temporary that widens the frame without any body access to the
extra bytes.

1. Bind both forms before and after to the same source and compiler settings.
   Confirm the mismatch is the prologue/epilogue, the base materialisation or the
   preheader, not the instruction body. One occurrence is enough and no loop is
   needed: an inline `((cell * 2) + 1) * 32` offset in a single load came out a few
   bytes short with two saved registers exchanged because GCC folded the `+0x20` into
   the load displacement, while naming that offset as an `s32` or `u32` local restored
   the separate base, and three algebraic respellings of the same inline constant
   measured identically.
2. Name the product once as a scalar local and index the same array symbol through
   it in both places: `offset = span * 3;` then `D_x[offset + 2]` and
   `&D_x[offset]`. Keep the symbol name on both uses; a pointer local used for both
   changes the base materialisation order and emits the base late.
3. An inline scale inside a fixed-address array subscript makes the whole symbol
   address loop-invariant. GCC hoists that base into a saved register before the
   loop and indexes from it, one instruction more than the original's per-iteration
   `lui $at,%hi(X)` + `addu $at,$at,offset` + `lbu %lo($at)` fold; the named offset
   restores the fold. A bare-index subscript such as `D_y[i]` already folds, so
   leave that one inline.
4. Two passes over one table with the same stride need one offset local per pass.
   A shared local ties the second pass' product to the first pass' quantity, so the
   scale stops being block-local; keep each pass' own index reset and multiply.
5. Measure. Require the original frame, the original base load position and the
   complete function bytes before accepting.

`func_801D41E4` (47/47, 188 B) needed this after four forms: branch polarity first,
array symbol for both row uses, then the named product. The named product also
shrank the frame from 32 to the original 24 bytes, which no body instruction had
shown.

### Repeated literals through one narrow value local

Use when a constant load occupies the register the original keeps for reloaded cell
pointers and the same value is stored at several sites. Write those stores through
one narrow value local - assign, store, reassign, store - so the literals become a
single multi-definition quantity. Read `.lreg`/`.greg`/`.cse2` first: an inline
constant can surface as its own quantity with several references, allocated over a
pointer reload, and routing the writes through one `s8` local merges them into the
earlier write's quantity and frees that register. Keep every store site, order and
value exact; one different literal or an extra store splits the quantity again.
Confirm with the complete live gate, and keep this as the matching form: restoring
inline literals re-opens the exchange.

## Compiler evidence

Before more allocator/scheduler probes, inspect the actual compiler build recipe,
relevant patches and source version. Bind diagnostic dumps to the source image,
effective compiler and unchanged codegen flags. Locate the first differing pass:
parameter expansion, combine, scheduling, local/global allocation or reload.
Debug dumps and assembly-only diagnostics are not linked acceptance evidence.

For GCC 2.7.2, inspect `local-alloc.c` quantity membership, births/deaths, classes,
call crossings and hard-register suggestions. Its `combine_regs` can tie a dying
input to a result and sum reference counts. Ordinary priority uses merged references,
register size and lifetime, with suggested-register allocation first. `flow.c`
weights references by loop depth; even single-execution loop notes can matter.
Individual pseudo counts or declaration order alone do not explain allocation.

Read `expr.c` for promoted-variable conversions and `combine.c`/`loop.c` for known
bits and induction rewrites. Predict which temporary survives and which instruction
changes. Verify that prediction in before/after RTL and a supported live comparison.
Do not infer exact allocator causality from matching scores, debug-register names
or source formulas without accounting for suggestions and interference. Keep flags
fixed; no automatic flag catalogs or compiler-variant batches.

### Boolean return expansion

For join-spanning booleans or extra return copies, inspect initial RTL before
blaming CSE or allocation.

1. Compare `return condition;` with explicit constant-return arms:

   ```c
   if (condition) {
       return 1;
   }
   return 0;
   ```

   Keep operand calculations and memory accesses in their original locations.
   Preserve short-circuit effects, widths, signedness and return ABI. This lever
   applies only when the required result is exactly zero or one.
2. Pin dumps to source hashes and the unchanged compiler profile. Trace expansion,
   jump optimization, CSE, combine and local/global allocation. GCC 2.7.2 can use
   a join pseudo for expression returns but hard return registers for constant
   arms. Jump optimization can merge the arms, CSE remove duplicate comparisons,
   and combine fold the result into the return register. Verify each claimed
   change in RTL; pseudo numbers alone do not identify values across forms.
3. Measure with `lift sweep`; require complete live byte equality and owning
   review. Restore neutral/regressive forms. Keep the conclusion profile-specific,
   not recovered source spelling or a universal CSE/allocation rule. Add no pins,
   dummy work or accesses.

### Guard scheduling and value reuse

Use when guard loads/tests and a product/store chain have correct operations but
wrong order. Keep compiler/flags fixed; prove the first divergent pass.

1. In GCC 2.7.2 `sched.c`, inspect `birthing_insn_p` and `adjust_priority`.
   Before reload, a live destination with `reg_n_sets[i] == 1` can receive
   `max_priority` when `n_deaths == 0`. Scheduling runs backwards: a boosted
   guard can land late in forward order. Check adjusted ready-list priorities,
   not raw priorities alone.
2. Test count-to-boolean reuse with two real consumers:

   ```c
   count_or_ready = stage[2];
   count_or_ready = i < count_or_ready;
   ```

   Use the result as the guard; repeat at the original latch. Preserve guard
   reads, paths and call/store order, including `continue` paths. Confirm both
   definitions survive optimization and remove the observed boost. Dummy
   assignments, duplicate reads and invented dependencies remain forbidden.
3. Separate scheduling from allocation. Correct order can still use wrong
   registers. Trace `.lreg`/`.greg`, block-local versus global allocation,
   suggestions and conflicts. After consumption, test a sample local reused for
   a real loop-scale operand; a shared sample-result local may instead force
   both results into one wrong register. Every value needs a genuine consumer.
4. Bind before/after dumps to source hashes. Verify predicted order and register
   changes, then full function/readonly bytes through `lift sweep`, live gate
   and independent review. Freeze the candidate during review. This lever is
   profile-specific evidence, not a universal spelling or proof of original C.

### CSE lifetimes and commutative targets

Use for an early saved-argument copy or a persistent commutative operand swap.
Keep owning settings fixed; inspect that compiler version, recipe and patches.
Nearby-version source supports a hypothesis, not owning-compiler proof.

1. For entry copies, trace `cse.c::make_regs_eqv` and `canon_reg`, then scheduler
   and allocator output. GCC 2.6.3 can prefer an equivalent pseudo whose lifetime
   extends further beyond the block. After the argument's final consumer, test
   reusing the formal for a later real index or control value, beyond the saved
   copy's last use. Preserve the saved value, ABI, signed indexing and conversions
   on every path. Confirm canonicalization and call-delay placement in RTL;
   source assignment order alone does not establish the cause.
2. For operand swaps, inspect `optabs.c::expand_binop`. GCC 2.6.3 can swap
   commutative operands when `target == op1`; reversing source operands alone
   can normalize back. Test a distinct genuine result before consuming it:

   ```c
   product = channel * fade;
   fade = product;
   ```

   Preserve signedness, product range, shifts and rounding. Keep both assignments
   only when the product and copied value have real consumers. Confirm RTL avoids
   the swap and allocation coalesces the copy without extra instructions. Byte
   equality alone does not justify an unsigned product or changed narrowing.
3. Measure each predicted effect and their interaction through `lift sweep`.
   Bind dumps to source hashes; inspect the complete register web and memory
   events. Require full function/readonly bytes, live gate and independent review;
   freeze during review. Treat success as profile-specific, not original-source
   recovery or permission to add dummy uses, aliases, pins or barriers.

### Narrow load sharing and real value reuse

Use for `lh` plus `lhu` from one address where original uses one signed load and
a register copy, often in a branch delay slot. Keep declarations and flags fixed.

1. Bind `.cse`/`.combine`/`.lreg`/`.greg` to source/compiler hashes. Trace narrow
   `HI` value, signed `SI` consumer and all later definitions. GCC 2.7.2
   `combine.c::try_combine` can split a narrow-load/sign-extension `PARALLEL` into
   signed load plus copy. Its `reg_nonzero_bits` guard rejects a narrow destination
   assumed zero-extended. Later definitions can change this global fact despite
   nonoverlapping runtime lifetimes.
2. Reuse the consumed narrow local for existing real work. Example: carry the
   complemented unsigned status byte through `s16` for an existing bit clear:

   ```c
   /* pending is s16; its previous value is dead on this path. */
   {
     u32 slot = *selected; /* selected already points to volatile u8. */
     pending = ~records[slot].status; /* status is u8. */
     records[slot].status = ~(pending | 0x10);
   }
   ```

   This replaces `records[*selected].status &= ~0x10`, not an extra operation.
   For unsigned `u8` on the 32-bit target, the promoted complement ranges from
   -256 through -1 and fits `s16`; `~(~status | mask)` equals `status & ~mask`.
   Re-prove widths, promotions and narrowing for other fields/masks. Evaluate the
   original index once; preserve status read/store and ordering. Require the old
   value dead on every incoming path and a definition before every later use.
3. Subtraction-result reuse can clear duplicate loads but tie output to the input
   register. Constants/booleans/masks may fold away without changing known bits.
   Word promotion can remove internal `USE`s and change frame size; trace
   [orphan slots](#orphan-stack-slots), never pad. Identical emitted forms share one
   optimizer class; retry only with changed compiler evidence.
4. Use `lift sweep`; require predicted load/copy change and inspect full frame,
   CFG, register web and memory events. Confirm current dumps before causal claims;
   a compiler rule plus exact bytes does not prove every intermediate fact. Finish
   with live whole-function byte gate and required semantic review. No dummy
   complements, discarded values, duplicate accesses, new volatility, pins or
   barriers. Treat as profile-specific hypothesis, not preferred spelling or
   recovered original source.

## Runtime evidence

Use `psx-rizin` for original call/access questions and `psx-emulator` for a bounded
behavior question. Define inputs, selected PCs or writes, stop and coverage before
execution. Natural observations and host-directed calls are different evidence.
For controlled calls, verify original overlay/helper residency, code identities,
register/stack setup and every host edit. Compare original and candidate from the
same supplied state. Keep proprietary captures local.

Check maintained receipts and current/staged input hashes. Version scripts before
changing them; an old receipt with a changed original script path is historical
staged-script evidence, not a current-input pass. Exec snapshots are pre-instruction
observations. Compare actual arguments, helper returns, memory events and outputs;
classify code, saved-return stack, timing and device differences separately.
A passed mission, sampled equality or narrower argument range cannot replace full
source-domain reasoning or live byte equality. Runtime can expose wrong behavior
labels; it does not prove an allocator explanation.

## Storage and analysis context

Pad/layout checks include real field offsets, size, BSS order, GP reach and following
symbols. Prefer owning explicit padding symbol, not universal macro. COMMON may
reorder by name. Prove the owning storage declaration and linker placement under
the attached profile; a layout discrepancy grants no flag changes. Shared
strings/tables need existing extern ownership, not phantom local static `.rodata`.

Separate Splat code/read-only jump-table objects can obscure m2c flow. Inspect label
resolution through owning generated-layout tooling; no handwritten assembly workaround
or unreviewed boundary change. m2ctx needs declarations/types/prototypes/macros/helpers,
not unrelated bodies/static data that shift rodata. GNU assembler GTE spelling may
use exact `.word` only in generated/SDK assembly, never C matching technique.

A reviewed manifest placement row is comparison-link evidence only: payload address
comes from the generated `linker.ld`, which can still group `.rodata` per segment, so
when a compiler-emitted table sits inside a `bin` span, check the generated placement
with the parent instead of reading payload placement from the row.

Read the effective address of a dense symbol load, not the printed base. A
`lui at,0x8015` with the displacement `0x930C` resolves to `0x8014930C` because the
displacement is sign-extended, while `0x8015930C` would need the base `0x8016`. The
same displacement therefore appears under two different bases, and a misnamed symbol
still builds and links; the map and binding addresses are what settle it. A dispatched
handler table follows that rule too: bind the symbol at the word its scaled index is
added to, which can be an entry inside a longer in-image pointer run rather than the
run's first word. Two exact dispatchers of one overlay differed only in that table
immediate, so a run base is not automatically the table symbol.

A Splat `asm` span is not automatically code. Disassemble the span against the target
binary before treating it as a lift target:
`bin/objdump -D -b binary -m mips:isa32 -EL --adjust-vma=<segment va>
--start-address=<lo> --stop-address=<hi> out/binaries/<target>.bin`. Text and table
spans disassemble as `.word` runs or implausible instructions and belong to a data or
rodata segment change instead. Measure span sizes from the segment list rather than
assuming a symbol name marks a function start: one `asm` span can hold several
functions, and its first entry can begin mid-function.

An address whose only in-image references are words of a code-pointer run has no
`jal` caller; that is the dispatch shape, not missing evidence. Read its prototype
off the code that consumes the run - the argument registers it fills and whether it
reads the result - because the body can ignore arguments its caller still passes.

## Failure and capture

Compile failure is not diff or exhaustion. Compare known same-target function to
separate candidate from workspace/toolchain failure. Preserve last verified diff;
regenerate compile database when selected source settings are missing or stale.
Known sandbox i386 failure uses native-execution route, never C/flag churn.

Record reviewed reusable residual class, decisive clean-C shape, before/after and
eligibility in this skill's references through authorized writer. Function-specific
selectors/scores stay mission evidence; source retains required metadata and rationale
for opaque clean-C shapes, not campaign journal. Partial evidence is not universal
rule. Reviewer proposes wording; approval precedes durable rule promotion.
