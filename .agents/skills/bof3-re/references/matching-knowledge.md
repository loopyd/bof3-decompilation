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
| `addu $at,index,$at` versus `addu $at,$at,index` | Named extern array can shift expansion to assembler; never pin scratch `$at`. For a scaled byte subscript on a named table, four symbol-based spellings (named-array subscript, `(u32)sym + off`, pointer add, offset local) all kept the compiler's operand order and stayed one instruction short, and only a fixed-address lvalue carrying the scale inside its constant (`PSX_REF(u8, base + i*STRIDE)`) produced the original `addu $at,index,$at` |
| Fixed-address constant steals entry copy | Named extern binding can prevent constant-address CSE; verify entire register web. A fixed-address scalar read and written back later steals it too: bare `g += 1` splits the symbol into `lui` plus displacement at each access, while one pointer local to it (`p = &g; *p = *p + 1;`) keeps the original's single base and clears that entry-block difference with it |
| Saved copy moves to entry or commutative operands stay swapped | Trace [CSE lifetimes and commutative targets](#cse-lifetimes-and-commutative-targets) |
| First mismatch stays at one entry-block offset through width, mask and cast probes | Reorder the loop-variable initializer statements (`i`, destination, source); declaration order does not move it. One measured order was exact, another moved the first mismatch later at a lower score. When that block also materialises a fixed-address base, order each zeroing against that materialisation separately: hoisting both above the base measured 96/98, the reversed pair 90/98, and the split matched. The later zeroing's home, plain statement or `for` initializer, measured the same, so the split is the lever |
| Extra sign extension or `andi` | Prove signedness/width/necessary mask; do not guess from decompiler. One measured source is a narrowed `volatile` cell read into a wider index, where reading the cell through a non-volatile byte view drops the mask. A redundant `andi 0xFF` on a byte the original's own load already zero-extended is a widening conversion written at the use, so cast there, and keep such a byte-validity guard as its two tests (`!((u32)mode < 2u || mode == 2)`) because folding them into `mode <= 2` emits one compare where the original has `sltiu` plus `addiu`/`bne`. A mask on a call argument the original does not carry is the callee's declared parameter width, not the argument expression: a `u16` parameter emitted that mask ahead of the `jal` with eight extra bytes, dropping the cast at the call site measured the same bytes, and declaring the parameter `u32` removed it. Take the width off the original's argument register, since existing sources on other targets may declare that callee narrower |
| One byte of a named byte group, addressed by a constant subscript or by a stride the body computes | `G[1]` computes the element address in its own register (`lui v1,%hi`, `addiu v1,v1,%lo+1`, then `lbu 0(v1)`), so the first difference is that base register, while the original folds `lui` plus `%lo(X)`. Declare that byte as its own scalar extern at its absolute address: the load folds and each store re-materializes `$at`, which cleared the residual on the next gate. The split is per byte, not per subscript kind: byte fields of one table record walked with a runtime stride need one extern each for the same reason, since every load then folds its own `%hi`/`%lo` |
| `lh` plus `lhu` replaces `lh` plus register copy | Trace [narrow load sharing](#narrow-load-sharing-and-real-value-reuse), known bits and later definitions. Mirror case: one record halfword the original reloads twice needs its own signed word temporary at the second read, since a reused local or an inline cast at the second use folds the pair into one zero-extending load and lands one or two instructions short with the first difference at that load |
| Byte-cell load hoisted above the record's word stores, extra live registers | Split the record view by access width: [record views](#scratchpad-cells-and-record-views) |
| Indirect call loads the selected pointer but the original fills frame slots with the whole table | Assign the named extern table to a local of its aggregate type, then index the local; indexing the extern indexes the table in place. When the original copies only a leading prefix of a longer pointer run, size that extern's type to the copied words: the aggregate assignment emits one word load and store per declared element and nothing past it, while a type covering the whole run copies the neighbours |
| Small block copy builds as per-word `lw`/`sw` or as a byte loop, original moves it with `lwl`/`lwr` plus `swl`/`swr` pairs | Give the moved bytes one byte-array struct and assign the whole block through it: an unaligned word-pair move is what GCC 2.7.2 emits for a block copy whose type has alignment 1, so a `u32` view of the same bytes stays on aligned loads. Fields copied one at a time instead of as one block take the same type per field: assign each moved four-byte field through an alignment-1 typedef, since that record's own `u32` field keeps the aligned load |
| Frame/size mismatch | Correct calls/prototypes, address-taken locals, aggregate copies, lifetime and topology; trace unused frame bytes through [orphan stack slots](#orphan-stack-slots), then name repeated scale products (#named-scale-products) |
| Frame grows with callee-saved registers and a cell store the original commits before a call ends up after it in the build | Keep that store at its original place, as an in-place update on the record (`p[n]++`). Carrying the incremented value in a local and storing it after the call keeps the value live across the call: one measured body cost two callee-saved registers and an `0x20`-byte frame (22 of 32 instructions, 144 bytes against 128), while the in-place form let the following argument load fill the load-delay gap and the store fill the call's delay slot. This is the mirror of a `jal` result the original stores right after the call, which does take its own local |
| Pointer read hoisted above a call, extra saved register, frame unchanged | Bind the call-derived scaled value to its own local before the pointer read; folding the mask/shift into the later address expression keeps the pointer live across the call |
| Saved-register group differs at entry while one stack aggregate feeds repeated calls and some of its fields keep a literal | Assign every field at each update, including the fields whose value does not change. Each assignment is that field's definition, so a skipped field drops its literal's liveness, which moves the literal to a caller-saved register and removes its store (one measured partial 61/67 at 244 bytes, first difference at the entry group). Naming those literals as locals measured identical, so the lever is field coverage, not spelling |
| Strided row stores whose scale the original re-reads from a published byte before the later rows | Keep the computed index in a local for the uses before that reload, and address each later row through the published byte (`D_row[(u32)*scratch * stride] = ...`). Read both ends off the original: the store that consumes the register without reloading it, and the `lbu` that precedes the first row which does reload |
| Call result stored inside the next call's delay slot | The original stores a `jal` result into its outgoing argument slot right after that call, before the next call's argument setup. Bind the result to a local; nesting the call in the later argument list moves that store into the following `jal` delay slot |
| A call result that also fills the stored cell builds the test one polarity off (`beqz` where the original `bnez`) and hoists the later store's constant above the first store | Bind the call result to that cell's own local, compare the local, and let both arm constants assign back to it (`s = wait(); if (s == 0) s = 0x0A; else s = 0x14;`). Testing the call inline in the `if` condition leaves the value in a register the call did not return it in. Arm order alone inverts a test without moving a constant, which separates the two causes. The same arm pair selected by an already-loaded byte instead of a call result needs no value local: read the byte into a local, then write one conditional expression straight into the cell (`cell = (gate != 0) ? 0x14 : 0xA;`), which matched on the first gate because the load's value and the selected constant keep their own registers. That local's own width is not part of the shape: a compare-and-store local carrying no arithmetic emitted the same `lbu` as `u8` and as `s32` across two exact bodies of the shape, so take the sibling's width and spend no variant on it |
| Set-once flag assigned a literal in several arms and returned as the result; body short of the original with the first difference inside the dispatch | Declare the flag and the return at the width the original stores it (`u8` for a byte store). As `s32` the pair measured four bytes short, because a wide value reuses a constant the dispatch already holds and drops the flag's own `addiu` and store; the byte pair restored both at the original size. That width insensitivity belongs to a compared-and-stored local, not to a returned literal. A flag set inside a loop and returned after it takes the same width decision at a larger cost: as `s32` its assignment folds into the switch's compare constant, one arm takes the compare delay slot and cross-jumps into another arm's tail, which measured 14 instructions and 12 bytes short at the original size, while the `u8` local matched unchanged |
| Arm tests a loaded value and passes it as a call argument; the build reloads the cell for the argument | Give that load its own local, test the local and pass the local. Read the arm's argument register: when it is the register the `sltiu`/compare tested, one load feeds both uses, so the same cell written in the argument list is a second load |
| `beq`/`bne`, return web or tail merge | Invert branches, early return versus result local, duplicate branch calls |
| One hunk differing only in the register order of a single `beq`/`bne`, with size, count and frame already equal | Reverse the written operands of that compare so the held value is on the left (`value != table[index]`); comparison operands follow the written order, so this swap works where an arithmetic commutative respelling normalizes back |
| First difference is one `addu` building an indexed entry address with its operand roles swapped | Give that entry address its own pointer local and compare through the dereference (`entry = records + index + 2; if (g < *entry)`). This is a lifetime edit, not a representation one: casts on the index and a reversed comparison each re-folded the address and stayed short, while the pointer local matched |
| Two bit tests on one word collapse into a single combined mask compare | `&&` on two distinct bits lets GCC merge them into `(x & (a\|b)) == a`; write nested `if`s to keep both tests. The merge needs one loaded word: two non-zero tests on different byte fields of a record stay two loads, so keep that conjunction as written |
| Both arms truncate the same value, then differ only in a constant, and the current build masks once before the branch | Repeat the truncation as its own statement inside each arm (`angle &= 0xFFFF;` then `angle += bias;`). One combined expression lets GCC share the mask across the arms and fill the branch delay slot, which came out two instructions short with identical memory events; the split arms keep a per-arm `andi` and the original's unfilled slot |
| Three-term sum stays one instruction long, or the first divergence sits at a later term | Sweep the three associations `a + (b + c)`, `(a + b) + c`, `(b + c) + a` before touching guards, casts or truthiness spellings of that sum; the mirrored nesting stalled one instruction short with the divergence pushed to the last term, and the re-spellings measured neutral. When one term is a product minus a constant and several stores reuse it, name that bias as one `s32` local per store group instead of keeping the sum inline: the parenthesized form let GCC fold the constant into an earlier argument's `addiu` and drop the `move` that saved it, eight bytes short, and the unparenthesized form lost nine instructions at the original size. Grouping also places the multiply's `mflo`: a scaled sum of two byte-table addends kept left-associative measured a partial with `mflo` ahead of the last addend's add, and parenthesizing each addend's own load-plus-offset fixed that order at unchanged size. A constant bias used once in the sum is part of that choice: `(a >> 9) - 0x4000 + p` keeps the `addiu` on the addend the original bills, while the algebraically equal `(a >> 9) + (p - 0x4000)` folds it into the product group and measured ten instructions short of a matched body of identical size, so a size-equal count deficit that survives corrected statement order is this association, not another ordering probe |
| `mult` plus `bgez`/`addiu (N-1)`/`sra log2(N)` around a scaled product | That triple is truncating signed division, not a shift: write `product / N` on a signed type. Shifting the product emits the bare `sra` and loses the sign-correction pair, so the divide spelling is what reproduces `mult` and the correction. An unsigned divide of the same kind of value instead shows one `mult`/`multu` with a large magic and no correction, so cast that dividend to an unsigned type at that use; one body can hold both spellings, since division signedness belongs to the use like a load's. A `u8` dividend promotes to `int`, so that cast, or a `u32` local holding the byte, is what selects `multu`, not the cell's own type. The mask applies to the divided value, not to the dividend: the original masks after the `multu`/`srl` pair, so write the byte cast where each result is used (`(u8)(count % 5) == 0`, `func((u8)(count / 5) + 4)`, `(u8)(channel / 17) << 5`), while masking the dividend instead moves that `andi` ahead of the divide and leaves the result bare. A record-slot index is that same dividend from the other side: when the original derives the index from the distance between a published record and its table base, its `multu` by a large magic plus a logical shift is that difference divided by the stride, so write the difference through byte pointers with a `u32` cast (`((u32)((u8*)p - (u8*)base)) / STRIDE`) - a typed record-pointer difference is signed, and that dividend brings back the sign-correction pair the original does not carry |
| `subu`, `bgez` and `negu` chain in one guard | That chain is an absolute difference. A magnitude consumed only by its own comparison stays inline: `abs(a - K) > L` through the `<stdlib.h>` `abs` matched a band guard of three such tests, each keeping its transient chain beside one limit constant the whole guard shares. When the magnitude is compared again or used after the test, write `d = a - b; if (d < 0) d = -d;` on an `s32` local and compare that magnitude against the bound. Keep each halfword's `<< 16` cell-start term inline in its own subtraction, since the original recomputes it per guard. When the magnitude's bound compare is followed by more arm work, write `d = d >= 0 ? d : -d;`: the mirrored `d = d < 0 ? -d : d;` and the `if (d < 0) d = -d;` form leave the negation outside the arm, so a later constant or copy fills the sign branch's delay slot and the body lands one instruction short at equal size. If the copy survives that, `d = __builtin_abs(d);` emits the backend's in-place `bgez`/`nop`/`negu` and shields the branch from the later constant, closing the residual on the compiler-evidence rung after the ternary alone stalled at the same size |
| `andi 1`, `sll 1` and `addiu -1` ahead of one `mult` | The sign is arithmetic, not a select: write `s = (gate & 1) * 2 - 1;` on an `s32` and multiply the scaled value with it (`(u16)(s * (count >> 1))`), so a value the product consumes carries it instead of a branch on the bit |
| Boolean result misses the return register; extra return copy | Trace [boolean return expansion](#boolean-return-expansion) before allocation |
| Loop topology | Compare while/do/for/guarded loop/explicit labels; matched goto is not style debt. For displaced induction setup, inspect [explicit cursors](#explicit-cursors-and-signed-bounds) |
| Global/BSS offsets | Audit COMMON/section/alignment/order/padding and following symbols |
| Phantom read-only object | Local static constant versus existing shared extern ownership |
| Load uses wrong argument register | Verify callee argument consumption and caller setup before allocator probes |
| Original stores the same constant to one cell twice | Write both stores in the original order. The repeat is real bytes: one store leaves the build short and puts the first difference in that cell's block |
| Exactly 4 bytes over an exact twin whose bodies differ only in the constants they store | A store of zero reads `$zero` and needs no materialisation, while a store of a non-zero constant costs the `addiu` that produces it, so the twin that stores zero is the shorter one by that instruction and the difference is not a shape fault. Read both stored constants before reshaping, and do not recover the bytes by respelling the store |
| Missing read after a store or an intervening call | Recheck pointer reloads, source qualification and aliasing against original instructions. A call that writes shared state, a formatter writing a shared buffer for instance, does not license holding one loaded cell in a local across it: two loads in the original are two accesses, so write the later use through the same address again. A store through a pointer loaded from a second symbol is the harder case: one symbol read twice stays live across it because GCC proves the two objects distinct, so give one of the two reads an indirect form of that same address, a fixed-address dereference or accessor, or give each read its own local: two locals loaded from the one cell stay two `lui`/`lw` pairs where a shared local folds them into one and lands a pair short, with the first difference at the second read. That serves the pair inside a single arm too, where the store between the reads writes another symbol and so forces nothing. Either read may carry it, since what forces the reload is that the pair differ; casting the storing record or spelling its base as a `u32` measured neutral |
| Second read of one byte missing with no store between the two | That pair needs a volatile pointee on the pointer reaching the byte, keeping the record's word fields on plain typed views; see [representation](#representation-and-volatility) |
| Second read of one table element scheduled above an intervening store | Write that read as a scalar dereference through its own pointer local (`p = &row[i]; ... return *p;`): a repeated `row[i]` compiles as an array-element reference, which the pre-reload scheduler lifts above the store, moving the store's constant into another register, while the scalar load stays at its statement. One pointer for both uses measured a 48/49 partial with the `addu` operands exchanged, and one value local folded the pair into a single load four bytes short; width, cast and guard respellings held at 46/49 at the original size, so the split of uses is the lever, not another local's width |
| One extern array covers a run of same-size tables and a later table is reached as `base[i + N]` | Bind that table at its own symbolic address and index it with its own element type (`extern s16 D_A[16][2]; extern s16 D_B[16][2];` then `D_B[i][0]`). The offset form gives both tables one base pseudo and one index quantity: one measured seed spelled the second table as `D_A[i + 16][0]` and held 23 of 46 instructions with a swapped index-register pair (`move a0,v0` against `move a1,v0`), while the two symbolic rows matched on the next gate. Both addresses are the payload's own immediates, so this is a binding split, not a layout change |
| Fixed run of identical calls whose arguments differ only in a constant | Write each call out with its own literal arguments rather than looping over an argument table: the original's run has no latch, and a loop adds the induction, the counter register and the frame bytes the original never had |
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
returns; shared locals or labels. Two equality tests on one call result that select
the same value take one disjunction with a single assignment
(`if (f() == A || f() == B) v = C; else v = D;`): the original reuses one assignment
block for both hits and puts `li C` in the branch delay slot, while an `else if`
chain repeating the assignment emits the merged compare form instead. When every arm
re-tests one byte and stores, keep those cases ungrouped: arm stores stop the byte
staying live, so separate `if` statements re-materialise the base and reload it per
arm, while a `switch` or `if`/`else if` chain evaluates it once and drops that
reload. Advance only when measured evidence rejects eligible alternatives. Locals may
change register web. GCC can normalize chain inversion regardless of spelling; another
spelling alone is not new evidence.

Branch calls `if (cond) f(x+A); else f(x+B);` can merge into one `jal` where ternary
argument does not. The same holds for identical argument lists: writing the call in
both arms can produce the single original call site through cross-jumping, and it
keeps the per-arm argument setup the original shows. Arms that each pass a
different table base never cross-jump, so a dispatch whose arms differ only in that
base keeps one call site per arm, each with its own setup. Arm stores merge the
same way: two stores into one cell whose constants share a high half cross-jump
into the one join store with that half's `lui` filling the branch delay slot, so keep
the store in each arm rather than selecting into a result local, which holds the
value in a general register and one more register live. Keeping the store in both
arms is not enough on its own: a constant stored in one arm and a read-modify-write
of the same cell in the other merge into one join store with a selected value even
when the constants share no high half, which dropped that arm's load and store and
left 28 of 39 instructions at the original size, while a separate pointer base for
the increment arm restored the per-arm store. Read the store count off the
original before choosing that: the cross-jump applies where the original stores once
per arm, while an original that stores once after the branch keeps the store there and
selects in the arms. A guard that picks the sign of one delta on a shared halfword is
that second case: assign one pointer and one value local in each arm and store once
after the branch, so each arm keeps its own base materialisation and load while the
join holds the single store. That merge also needs each arm
to read the cell's base itself: a pointer loaded once above three arms that write one
byte cell kept all three stores, with the first difference at the extra store, while a
base read inside each arm folded them into the original's single post-branch store.
That merge also costs instructions when one constant reaches one cell from two
nesting depths: an outer arm and an arm inside its `else` chain are identical blocks
to the late jump passes, which merge them, drop the outer arm's own store path and
land short, two instructions in one measured case; the first difference then sits on
that outer arm's own branch, which jumps to the merged block with an unfilled delay
slot instead of reaching the store that carries the constant. Give the inner arm its
own result local at the store width and hand it over with one assignment after the
chain, since that copy coalesces in registration.
The accessor is not the discriminator: the constant slot form and the named cell
measured the same while that read stayed above the arms. Do not route that argument
through one shared constant local: with one call it hoists the fall-through arm's
constant above the test and fills the branch delay slot with `sw $ra`, three
instructions short of the original. A signed pair such as `-1` and `0` fails one step
worse: selecting it into a local drops the branch for a compare-and-negate, so the
duplicated call is the spelling that keeps it. Early returns versus initialized
result changes `$v0` lifetime. Two constants that share no high half still
cross-jump, so per-arm stores do not save that select: every function-level
spelling, counting a result local, a conditional expression written into the cell, a
store in each arm and either polarity, came out short of the original.
Nesting the select inside the positive guard block that carries the function's own
`return 1` splits the arms again; in that block the arm order is the last variable,
since the correct order leaves the branch delay slot unfilled and the mirrored
nesting stays short at the same size. When the guard wraps calls and stores and each side
ends in a different constant return or constant store, put the work inside the `if`
with the constant arm after the block: the work arm goes out of line behind `beq` while
`j <epilogue>` keeps `move $v0,$zero` in its delay slot. Read the arm order off the
original as well: the arm the function falls into is the `if` body, and the arm the
branch jumps to is the `else` or the block after an early return. Reversed arms that
each end in their own return keep frame and size and only invert the test and its
target, so a `bnez` where the build emits `beqz` is this ordering choice, not a later
rung. Arms that both end in the same constant return are that case with the shared
constant as the only tell: choose the written order from the arm the branch enters,
because ordering by which arm does more work instead inverts the test and hoists that
constant above the other arm's stores at unchanged size and count. When two arms
rejoin a common tail instead, the inverse order is not size-neutral: skipping the arm
written first needs its own `j`, which cost one instruction and four bytes at an
unchanged frame. A result local live across
the calls cost a saved register and frame bytes, and an early `return B;` before the
block relocated that return out of line instead - `.text` size stayed equal and only the
branch shape differed (`bne` to a trailing `move $v0,$zero`). A guard that returns the
same constant a loop exit holds is that case from the other side: the early-return
spelling needs `bne` plus `j` to reach the shared block, while wrapping the body in the
guard's complement, looping with `for` and keeping the constant arm inside the guard,
matched where the early return and the same wrap under a do-while both stayed short
(43/52 against 51/51, the short forms four bytes over the original at an unchanged
frame).

Per-arm cleanup is the arm merge in the other direction: when every case of a
`switch` ends with the same block and the original's failure edge lands past it,
write that block inside each case before its `break`. One copy placed after the
switch gives the default edge a block the original never enters, and the first
difference is that branch's target, one instruction away, with a re-arm constant
then hoisted into the case's delay slot. The identical per-arm tails cross-jump
into the one site the original shows, so the cleanup still emits as a single
block.

GCC loop rotation can move invariants: assigning inside loop body can permit late
hoisting/preheader copies; moving before loop can lose them. Keep only measured
semantically correct shape; record negative experiments before trying another shape.
Hoisting also reorders whatever sits beside it: a mask the body re-derives from an
argument each iteration belongs at that use, because assigning it once above the loop
moved the neighbouring entry-block masks and rotated the temporary pool at the same
size.
When the original's entry branch carries the loop index's zeroing in its delay slot,
put the `index = 0;` statement before an explicit `if (count != 0)` guard and keep the
body as `do`/`while`: the count is an unrestricted byte, so the guard is required, and
the `do` supplies the latch compare the original shows. That zeroing also matters when
the counter feeds a scale before the loop: folding `index = 0` into the `for`
initializer moved the first difference to instruction 0 at unchanged size, while `for`,
`do`/`while` and `continue`-per-bound-test spellings of the loop measured identical, so
spend no variant on the loop keyword.

A shared test after value-producing arms extends a local across the join. Test
per-arm checks branching to the existing common operation. When that common statement
sits behind a further guard that only some paths reach, write it once at a `goto` label
inside that guard and jump there from each arm's own test, so both arms share the one
store the original branches to. For tail-merged stores, express the original
subtraction, saturation and capped paths explicitly; retain sentinel exits and each
path's memory effects. Duplicate no calls or volatile reads.

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
measured neutral at the same residue. A conjunction also folds rather than swaps
arms when its low bound is `>= 0`, because that pair is exactly the unsigned bound:
GCC emits one `sltiu` and lands two instructions short of the original's separate
sign and magnitude tests, so write each test as its own early return. A bounded range
whose lower bound the original subtracts before a single `sltiu` is that wrapped
unsigned compare, not the two-test conjunction: write `(u32)(x - lo) >= N`, casting
at the compare so a value below `lo` wraps into the reject range. Two signed compares
cannot emit the `addiu`/`sltiu` pair, and the failure arm stays the early return.

Unrelated reject tests around a block that itself calls are the opposite case: write
them as one conjunction with a single `return 0` after that block. A ladder of early
`return 0`s merges its returns into one block at the first reject position, so that
block takes the slot the accepted block's call needs and the accept arms swap;
measured at unchanged size with the first difference inside the reject region. Where
the accepted block calls nothing further, the early-return ladder is what matches.

A guarded dispatcher (`sltiu` bound, table base, one indexed load, `jr`) gives the
dense switch shape directly: one arm per table word, in order, and the arm an
out-of-range index reaches is the default, so that case carries the default's
statements instead of a duplicate block. Count the labels off that bound: a sparse
`case 0:`/`default:` set builds a comparison chain and emits no table, which surfaces
as a reviewed-`.rodata` placement refusal with no instruction count, so a `sltiu N`
with no MIN subtraction and an N-entry table owes labels 0 through N-1. When the
first seed already builds to the original `.text` size and differs only at
address-bearing words, the source shape is settled; finish the symbol bindings and
the reviewed table placement before probing more C. A table whose entries differ only
in which block each case value reaches is part of that step, because the reviewed
`.rodata` compares entry contents: when reshaping the arms leaves one differing table
word at matching `.text` size, move the case labels and leave the bodies alone.
The mirror carries no bound at all: a body that loads a state byte, scales it by
four and adds it to the table base with no `sltiu` in front is one indexed call on
an extern bound at that word, whatever the entry count, so write no clamp, no
bound check and no arms.

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
   reload drop into the first load's delay slot. A load group with no store of its
   own takes the same scope for the mirror residual: when the build is one
   instruction short and the first difference is a filled load delay slot,
   single-definition live loads are being raised by the
   [guard-scheduling](#guard-scheduling-and-value-reuse) boost and sunk past a later
   store. Wrap the real loads in the scope; its `LOOP_END` stays between them and
   that store and holds them ahead of it. Containment is exact, because `LOOP_BEG`
   is a boundary as well: a load the original issues before the prologue save group
   stays outside, with only the group that must clear the later store inside.
   Wrapping that first load with the rest moved the saves ahead of it with `nop` in
   its delay slot and one differing hunk at offset 0, the unscoped body fell five
   instructions further, and the split matched.
   The same note pins a constant rather than a reload: when block order and size
   already match and one branch delay slot holds `nop` where the original loads a
   literal, wrap that literal's real initialization in the scope. Its `LOOP_END`
   then stays ahead of the following cursor reload, so the first scheduling pass
   cannot move the constant above that load and the delay-slot pass fills the
   empty slot with it. A volatile view over the same cursors recovered the block
   order and still left that slot `nop`, and a baseline `.sched2` kept both
   constants in the body while only the delay-slot pass hoisted one, so name the
   pass from its dump before choosing this lever. A loop index reset is the same
   real initialization: when it directly follows a call whose delay slot the
   original's zeroing fills and the loop's bound read comes next, write
   `do { index = 0; } while (0);` ahead of that read, so `LOOP_END` keeps the reset
   below the call. That boundary is per call, so one body takes one scope per such
   loop, while a reset that follows an ordinary statement stays plain.
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
   `((u8)i < 10)` at the test. Declare that wide value unsigned: a `u8` local or a
   named masked local folds the `andi` away, and a signed wide local emits `slti`
   for the bound where the original has `sltiu`, one instruction off.
5. Confirm predicted setup, entry test and compare through `lift sweep`, then
   full function/readonly-byte gate and independent review. Freeze during review.
   Correct setup alone proves neither semantics nor exactness; this lever does
   not establish original C or universal compiler behavior.

Spell the cursor as one source pointer and let the compiler derive an interior
anchor. A second pointer declared at the record's interior offset added a third
address anchor, an extra literal materialisation and sixteen bytes, while the exact
body keeps the base pointer for the head accesses and reaches the tail group through
the anchor the compiler derives, whose stores then read as negative displacements.
The increment list is order-bearing in the same way: of two updates, the one that
advances the anchor comes first, and the reversed list lost one instruction at the
advance block with size unchanged. The loss can land at the loop head instead, with
the reversed step rotated in there and dropped after the last store, and moving the
same two steps to trailing `while` statements reproduces it: the order is the lever
and the loop spelling is not.

## Delay slots and entry copies

- Same-size branch/`jal`/`j` difference: identify exact instruction, live inputs/
  outputs, value use after transfer. Try branch inversion, early return, independent
  statement order, one local or pointer hoist.
- Original `move tN,aN`/`move vN,aN`: prove lifetime across call/branch or overlapping
  temporary, then local copy at that lifetime with bounded declaration/first-use order.
  A `move` that copies a freshly loaded base into a saved register after a store group
  marks a second local's live-range start: one local serving both the store group and
  the loop base keeps that saved register live from the first store on and drops the
  copy, so load the loop base into its own local after the group.
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
stored - a saturation arm testing the sum it wrote - needs the plain view. Without a
store to separate them the mirror holds: a byte the original re-reads for its own
decrement after a test already loaded it takes a volatile pointee on the pointer that
reaches it, while that record's word fields stay plain through typed views whose cast
drops the qualifier, so the scheduler still interleaves them. On the shared
scratchpad cell this pointee qualifier sits beside the `u8* volatile` spelling below,
which still governs reloads of the cell itself. Narrow
volatile pointee may cause `lbu` plus manual sign extension versus original `lb`.
Remove unjustified qualifier only with semantic evidence; reviewed plain view may
restore shape. Volatile pointer cell forces reload without qualifying pointee; use
API reference. Never add volatility as
arbitrary scheduler control. The shared scratchpad work cell is the one place this is
already decided: five byte-exact sources declare it `u8* volatile`, so when the original
keeps consecutive handoff stores that no call separates, declare that cell volatile rather
than inventing a barrier. Cite the siblings when you do, and leave any other cell alone.
Reading or writing that cell from another target still owes this target its own
`@source`-tagged extern and map row: the no-copy rule bans borrowing a binding, not
re-declaring a cell this target's own bytes show.
Original-proven ordering remains explicit evidence. Where a store through the loaded
pointer already separates the reads, the plain spelling measured identical bytes on that
same cell, so the qualifier is not what carries the reload there: a store through the
pointer that cell just supplied may alias the cell, so every access on the plain named
cell re-reads it and the per-access reload comes out unflagged. That alias covers any
pointer cell, not only the scratchpad one: exact bodies reload an in-image cell before
each byte store of a clear run, with every access on the plain named cell and no
pointer local. That alias needs a store through the pointer the cell supplied. Where
the original loads one fixed-address pointer cell again before each of several copied
fields and only stores through another pointer separate those loads, the plain cell
keeps one loaded value and the body comes out short: one local per read does not
restore the loads, and `u8* volatile` on the cell does. Pick a sibling's cell spelling
by its handoff pattern - consecutive stores that no call separates take the volatile
cell, a reload per store group takes the plain one - and leave a cell that already has
exact siblings on both counts alone.
The qualifier is not neutral once the reloads are already forced by an intervening
store or call: the plain cell also leaves a re-arm constant free to fill the delay
slot of the test that follows it, while the volatile spelling keeps that slot `nop`,
so a body one instruction short at a filled branch delay slot on a re-armed cell is
this declaration, not a scheduling rung.

Signedness belongs to the use site, not the cell: read it off that site's load. One
byte cell can need `*(s8*)&cell` where the original emits `lb` in a branch test and an
unsigned read where another function emits `lbu` on the same address, so a shared
header's `u8` scalar fixes the address without proving the signedness. Declare the
signed view locally at the use that shows `lb`. Width is per target use in the same
way: an address sibling targets declare and access as a `u16` takes a `u32` extern in
the target whose own original compares the whole word, because the declaration picks
the load that compare reads.

Unaligned fields/padding/SDK stack types follow type-application reference. For
pooled member addresses, compare direct access through the existing member with
an initialized pointer local serving one operation. Direct member access can retain
folded offsets where a reused pointer materializes the cell address. An array of rows
is that same comparison at a wider scale: a per-column `u8*` row-pointer local kept
the narrowed column in a caller-saved register across the row's draw call and cost two
instructions and eight bytes, while declaring the table with its own rank and indexing
`[col][row]` matched, and re-narrowing the column into an `s16` local lost nineteen
instructions. Let the declaration carry the rank. Compare plain and single-execution
scopes; loop notes can change CSE/combine as well as scheduling.
Preserve cell/pointee types, qualifiers, evaluation count, aliases and reloads after
stores. Do not replace the owning aggregate, add cell aliases or qualify accesses
solely for matching. Check the whole function and frame, not the first fixed load.

Match local views to existing storage and consumers. A mutable source view over a
mutable byte table can change read-after-write code generation without adding
volatility. `const` does not imply non-aliasing. A `const volatile` view is a
separate declaration from a mutable `volatile` one: when a partial's view already
re-reads its live fields and a tail of load and store order differences survives,
dropping `const` from both the declaration and the cast leaves every access and store
unchanged and closed that residual at the original size, while qualifier probes on
the individual reads measured neutral. A pointer-cell reload after a store may
correct an earlier reconstruction rather than preserve PRE under every alias.
Prove original accesses and final linked bytes; make no universal alias guarantee.

Use established halfword/aggregate views when field widths and consumers support
them. Preserve public constness and ABI unless caller/callee evidence supports a
change. Bitfields require target layout and codegen proof, not guessed packing.
Regroup unsigned address sums through a row/base temporary when original addition
order differs; prove value equivalence and preserve all memory events. Declare that
temporary at the narrower unsigned width when two uses narrow it in opposite
directions. A `u16` row that feeds one unsigned and one signed cast keeps the sum
un-narrowed until each use and matched; the inline spelling stayed a couple of
instructions short, an `s16` temporary that narrows at the assignment measured one
instruction worse again, and an `s32` temporary lost the addition chain.

Pin a reload where the original reads it with a typed value local. Reading a cell
straight into the store keeps the load next to that store, while assigning it to a
`u8` local at an earlier statement pins the load at that statement and leaves the
store later, which is the shape a record-arming block shows. That earlier position is
also what lets the store fill a following branch or return delay slot; loading at the
store instead puts the load and its delay `nop` next to it and leaves the slot empty.
A pointer local instead defers the dereference, so choose the local's type by which
side of the load the original fixes. The same pin works for a scalar read from another
symbol. When the original loads one shared halfword ahead of an unrelated store and
consumes it after a pointer-cell reload, bind it to a `u16` local at that earlier
statement. Spelling that consuming store inline measured eight bytes longer with the
first difference in the entry block, because the compiler then reloads the pointer
cell again for the later stores. Two independent loads keep that order too: a build
four bytes over size whose first difference is the record load ahead of the
pointer-cell reload is a statement-order residual, and issuing the reload first
recovered the size. A local for the record base measured the same bytes either way,
while the pinning value local stayed load-bearing: inlining it back cost two
instructions at the original size.

Keep a narrow parameter narrow. Copying an `s16` argument into an `s32` local moves
the sign extension to the entry block, where the original converts at each use site.
Read the parameter directly and cast at the use instead. The formal's own width
decides this: a wide parameter cast to `s16` at each use is a different shape that
moved the entry block, frame and size, so declare the narrow formal and leave the
body's uses alone. The mirror case is a cast used twice: `(s8)value` in both the call
and the comparison lets one conversion serve both, while reassigning
`value = (s8)value;` between them duplicates it.

Where the mask sits, not the parameter's name, decides that width. An original that
masks a byte argument once at entry (`andi $a0,$a0,0xff`) and leaves it alone after
has the wide formal with the mask written into the body: the `u8` formal made GCC mask
per use and left the body 68 bytes over at 39 of 60 instructions, while `s32`
parameters with in-place `& 0xFF` matched on the next gate. The mirror is the call
site. A truncation this build emits in a `jal` delay slot that the original lacks
means the prototype the body declares for that callee is narrower than the original's,
so widen the declared parameter and re-gate. That declaration belongs to the caller,
because the conversion is the caller's: two exact callers of one helper declared it
`u8` and `s32` and each matched its own call-site bytes, so unifying them into one
declaration owes both call sites a fresh gate.

The formal's width decides the exit the same way. When the original's tail
sign-extends the return value with `sll`/`sra` of `$v0`, that conversion comes from a
narrow declared return type: an `s32` local returned as `return (s16)local;` folds the
cast away and leaves `j $31` plus `move $2,$4`, four bytes short, and widening the
return type to `s32` reproduces the same loss. Declare the narrow return and return
the narrow local plainly.

Mirror the width rule at the assignment: when a narrow callee return feeds a narrow
cell, keep the accumulator local at the cell's width. A clamp result stored into
several byte fields takes those fields' width too, not the `s16` it was computed from:
the wide local measured two instructions and eight bytes over with the first difference
inside the second clamp group, and the `u8` local left one `andi 0xff` per group.
A wider local converts the return where it is assigned and costs an `sll`/`sra` pair
the original does not have,
while the narrow local leaves the truncation to the store. The accumulator's width
also picks the load: scaling a signed byte element by a wider value through an `s8`
local made GCC load it `lbu` and widen with `sll`/`sra` before the multiply, which
overshot that body by eight bytes, while a word-wide `s32` local restored the
original's single `lb` at the original size. Read an `lbu`-plus-widen pair as that
accumulator width rather than the cell's signedness, and set the width by where the
original converts: truncation at the store takes the cell's width, widening at the
multiply takes the wider local. A loop-carried accumulator takes the narrow width too,
with the pack written as one truncating assignment (`packed = (u16)((packed << 5) |
component);`) so the copy back into it stays a real `move`. A wide accumulator emits
the same body but lets the CSE pass after loop optimisation substitute the product
word for its later uses, which rotates the temporary pool at equal size; read that
rotation as this width rather than an allocation residual, and keep the width on the
accumulator, since a second narrow product local measured worse.

A narrow value built from a difference wants that split one step further: assign the
base constant, subtract in place, then truncate with a self-assignment
(`w = 0x40; w -= limit; w = (u8)w;`). The one-expression `(u8)(0x40 - limit)` and the
`& 0xFF` respelling both let GCC fold the constant and the mask and stayed two
instructions short at the original size, and an arm-local narrow temporary copied
into the wide local measured the same. A store to another cell written ahead of that
staircase closed the remaining pair, while swapping declarations did not move it.

One argument word that carries a signed cell in its high half and a fraction in its
low half wants that split as well: read the cell out into a narrow local and mask off
the fraction in its own statement, both ahead of the dependent index and the repack.
Respelling that single expression - a cast at each use, a shift-pair cast, narrower
step locals, a by-value struct parameter - stayed short of the original's size or
kept the wrong order, and the split matched at its order and size.

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
   matched, but that freedom sits between adjacent in-place updates. Where the
   block instead copies a descriptor, advances a field, decrements a counter and
   then decrements-and-tests another, the plain decrement keeps the place the
   original's store sequence gives it, ahead of that test: moving it after the
   test dropped nine instructions at the original size with the first difference
   inside the block, and moving the test earlier cost more.
   Locals win instead when the original groups its own loads above its adds:
   bind both new values and order the two stores so the one the slot carries is
   written last before the test, because a store sitting immediately before a
   test is that slot's own candidate. Both splits are measured: a slot holding
   the tested field's own store writes the other field first, while a slot
   holding the other field's store writes the tested field first and leaves the
   test on its local. Where the original's adds sit tells which side applies.
5. Confirm with live asm-diff and the whole-function gate before retaining.

Byte cells take a `u8` accumulator instead:
`countdown = (u8)(p[n] - 1); p[n] = countdown; if (countdown == 0)` is the form
whose `sb` is followed by `andi 0xff` and `bnez` on that stored register, with no
second load of the cell. The exact countdown handlers across these overlays keep
that shape, so the truncation, not a reload, supplies the compared value. Where
that mask sits picks the form: a byte update whose mask shapes the stored value
stays in place (`p[n] = (p[n] - 1) & 0x1F;` emits `lbu`, `addiu -1`, `andi 0x1F`,
`sb` on one loaded base), while the `sb` an `andi 0xff` follows is this
accumulator, because only the test consumes that mask. The local
itself is not load-bearing: `if (--p[n] == 0)` emits the same `lbu`, `addiu -1`,
`sb`, `andi 0xff` and `bnez` on the stored register. Both spellings reached exact
on that class, so take the one the sibling shows and spend no probe on the other -
unless the same loaded pointer also carries a word update: there the named
accumulator measured a partial at unchanged size with the pointer and that word
holding each other's registers from the first instruction, while the in-place
decrement matched, and rebinding the word's statements, deriving a word view or
swapping the cell spelling all measured that same partial, so remove the
accumulator instead.
The test's signedness picks that extension: `if ((s8)countdown == 8)` on the same
`u8` accumulator emits `sll`/`sra` of the stored register before the compare, so a
test against a signed-looking channel value takes a cast at the test and leaves the
accumulator `u8`. That cast carries the conversion only against a nonzero constant.
A zero test is sign-agnostic, so `if ((s8)accumulator == 0)` folds back to the `u8`
build and keeps the same one-instruction residual at unchanged size; declare the
accumulator `s8` and store and test it plainly. A cell whose test has its own load
takes the `s8` view on the cell expression at that test instead. An add-then-test
cell takes its spelling from that test: one that re-reads the cell stays in place
(`p[n] += 4; if (p[n] == limit)`), while an unsigned bound on the value just stored
takes the `u8` accumulator. That bound admits the in-place result too:
`if ((p[n] += 0x10) == 0x80)` and `if (++p[n] >= 0x81)` each reached exact on the
first gate, so a body seeded from a sibling that compares its own update owes no
accumulator probe. A guard on the value before the store is neither: when
the original's `andi` tests the loaded byte while the incremented byte's `sb` fills
the branch delay slot, keep the old value (`old = p[n]++; if ((old & 3) == 0)`) rather
than testing the stored byte. Two exact sources on one overlay differ in nothing else,
so the cell's test decides before either form is probed.

### Scratch cursor group variables

Use when a function re-reads or re-derives a base before each store or call group and
the current build keeps one register for all of them, whether that base is a pointer
cell load or a record address computed from a cursor.

1. Read the cell into its own variable at each original reload point. One variable
   reused across groups becomes a single global pseudo, and that pseudo conflicts
   with the registers the original used across its whole range. The same split
   applies to per-group record temporaries: a cursor shared by two store groups
   kept both in one register and left the block's tail mismatched.
   Mutually exclusive arms split the same way: a record pointer declared once and
   used by both arms of an `if`/`else` stays a single quantity, so declare it
   inside each arm. That split made the already-close arm exact at unchanged byte
   count, while sharing the pointer kept that arm mismatched.
   Only the reuse is the defect, not the missing variable. A group read once before
   the next reload point measured the same bytes reached through the named cell as
   through its own variable, so split where two groups share one, not every access:
   routing the leading group through the later group's variable merges them into one
   quantity and hands the leading group the later group's register (measured `v1`
   against `a0` at unchanged size). Two reads of one cell inside a single statement
   are separate reload points as well: when the original's two loads land in
   different registers, one local serving both arguments of that one call is a
   single allocno and gives them one register (measured nine instructions short),
   while the inline named-cell spelling and two distinct locals both matched.
2. Read the allocator dump before changing spelling. `.greg` reports the number of
   global pseudos, each conflict set and the register each pseudo receives.
3. Give the variables distinct names. GCC then keeps separate quantities, so each
   can take the register the original used. Renaming one variable in place does not
   split it.
4. Place each reload where the original has it. A second read after the first
   store also reproduces a load-delay `nop` that a merged read removes. Ordering
   against an unrelated store is part of that placement: with the cell read written
   after a halfword store the pointer took the register that store writes, its slot
   stayed empty and the load dropped below the store, eight bytes long at an
   unchanged frame; reading the cell pointer ahead of the store and deriving the
   offset after it restored the original register and both filled slots. Moving the
   index byte ahead instead of the pointer kept the same eight-byte loss, so the
   value moved early is the cell pointer.
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

A reload per iteration is not a group. When the original interleaves a work-pointer
reload between copies of one record's fields, keep those reads inline on the named
cell and give a value local only to the copy whose store the original places later. A
pointer local for that block measured a partial at unchanged size and instruction
order: its reload attached to the preceding field load, and the byte-masked slot index
then took the neighbouring register. Step 1 applies where a reload opens a store or
call group, not between fields of one record. Split by the register the original gives
each reload, not by adjacency: reloads it lands in one register stay one local even
with a whole block between them, and reloads it lands in different registers take one
local each, so count groups from those registers rather than from statement distance.

The walk state itself can be that cell: when the original advances a published cursor
by the record stride and re-materialises the cell's load after each call or iteration,
keep the loop on the cell - advance the cell in place and address the body through it,
or read one fresh local from it per iteration - instead of carrying one pointer local
across the loop, which keeps a single base and drops those reloads.

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
   function-wide choice. The inverse split needs no local at all: when a guard read
   and the arm's first store share one loaded base while later stores re-read the
   cell, write every access on the named cell and the store's own possible aliasing
   of the cell supplies those later reloads.
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
   and emits `li ...,156` where the original has `li ...,-100`. An addend needs the same
   reading: `value - 1` emits `addiu ...,-1` where the original's bytes show
   `addiu ...,0xFF`, and `value + 0xFF` stores the same truncated byte with the
   original immediate.

### Scratchpad cells and record views

A bare constant scratchpad address is CSE'd into one register: `lui` plus `ori` once,
then `lw 0(vN)` per access. When the original re-materialises the constant per access,
name the cell: declare an extern at its fixed address so each access emits its own
`lui 0x1F80` plus `lw 68(vN)`. Applies to any fixed-address data the original reloads
per use; the named symbol in the map restores that repetition. The CSE also holds that
register across an intervening `jal`, so a cell read on both sides of a call costs a
callee-saved register, its save/restore pair and frame bytes: a built `0x20` frame
against the original's `0x10` or `0x18`, first mismatch the prologue's
`addiu sp,sp,-32`. Shorten the live range instead - a single-use store written inline
at its site lets the cell load land after the call, and a cell the original re-reads
around a call takes the pointer-cell extern (`extern u8 *D_<ADDRESS>`) reloaded at
every access.

Pick the access spelling from the original's hi/lo pair, not from symbol ownership:
`lui` plus `ori` is an integer-constant address, while a map symbol emits
`%hi`/`%lo`, folds into the load when the offset fits and otherwise builds its base
with `addiu`. A cell that owns a map row can still need a literal accessor at one use
site; the hi/lo pair and base register in that load separate the two forms before
another local arrangement is tried. The pair also says when to un-fold: when the
original builds the cell's base with `addiu` ahead of the access, a direct access
through the named cell folds that address into the load's 16-bit displacement
(`lbu v0,OFFSET(v0)` where the original has `addiu v0,v0,OFFSET`). Take the cell's
address into a pointer local and access through it (`u8 *flags = &D_<ADDRESS>;
*flags |= 1;`) so the address gets its own register. A byte read-modify-write residual
closed on the next gate with that spelling.

The form is per body, not per family or per cell, so count the original's cell loads
before copying a sibling's spelling: a member that reads the cell once and reuses that
base for its guard and its store stays exact through the constant-address accessor,
while a member whose original reloads the cell after a store through the record needs
the named extern. Copy a sibling's statement order and operand forms, not its cell
access spelling. Once the reload count agrees the spelling is free: a displacement
that folds makes a named extern and the `SPAD_PTR_TABLE` accessor at that cell's own
word emit the same `lui`/`lw` pair at each reload point, so neither spelling is owed
a probe and the seeded sibling's form stands. One body can mix both: keep a
read-modify-write pair on one pointer local and re-spell the named cell for each
later access, because a store through that local may alias the cell and the
original re-reads it there. Read each access's own `lui`/`lw` pair; running one
local down the body shares that base and drops the later reloads. When every access
re-reads the cell, a per-width `u16 *`/`u8 *` local or one pointer per access group
still shares a base within that view. Write each access as a cast/index expression on
the named cell instead.

A shared header that maps a cell name to a macro over a constant address
(`#define D_X g_Cell`) folds the same way, once, because the macro hides the
symbol. `#undef` it in the source when the original reloads the cell per access and
read the target-local symbol instead; unconditional `#undef` at outer body level is
admissible for an ordinary function. Sibling sources on more than one target closed
a hoisted-base residual here, so reach for this before allocation work. The
fixed-address accessors of the [memory API](memory-api.md) are that same macro, and
their fold can surface outside the body: a frame eight bytes larger with one extra
saved register and no differing body access is the shared base, not an orphan slot.
The accessor folds only while its address is not shared: a cell read again after a
call keeps the constant live across it, so the base hoists into a callee-saved
register and the frame grows with it, while the named map cell re-materialises at
each reload point, since reads that no call and no aliasing store separate still
share one base. Two accesses to one cell then trade one instruction for one: the
shared base makes the first access `lui`+`ori`+`lw 0(base)` and the second a bare
`lw 0(base)`. So a body can keep the original byte count and still lose matched
instructions with the first difference at instruction 0 - that pair is the accessor,
not an allocation residual. The cost scales with how many accesses cross the call: a
body whose many cell accesses all follow one `jal` measured 35 of 66 instructions,
first difference in the prologue (`sw s0,16(sp)` where the original materialises the
cell), and both local layouts, each access inline and one local per store group,
measured identically there, so only the cell spelling moves that residual and no
local sweep is owed. A body with no map row for the cell has a non-volatile
spelling: build the accessor's base from a non-constant offset (`u32 off = 0x44u;`
then `PSX_REF(u8 *, SPAD_BASE + off)` at each reload point), which measured 28 of 28
instructions where the same accessor with a literal offset hoisted the cell address
into `$s0` and measured 18 of 28.

Stores follow the same rule. Each `sb` to a distinct fixed-address cell carries its own
`lui $at, %hi(sym)`, so declare one named extern per cell. That holds for one cell read
and written back: keep `cell = cell - 1` as the bare symbol expression, which folds the
read into `lui`+`lbu %lo` and re-materialises `$at` for the store, because a pointer
local gives that read and store one shared base instead. A build that holds one base
register across the run instead has a shared base or aggregate view the original
lacked. Read the store's base register to pick the spelling. A store that
re-materialises `$at` wants the bare symbol expression, while a store through the
base the load already left in a saved register wants one pointer local for both
accesses, even when the load sits in a guard test and the store in its arm. A byte
that is one field of a wider cell reaches that shared base by keeping the wider
declaration and casting the one access (`*(u8*)&cell += 1`), which reproduced the
original address computation, while a byte-scalar extern at the same address folded
the load into `$at` and re-materialised `$at` for the store. Inside
one block, stores keep source statement order, and a constant re-materialised into
one register between stores shows where its statement starts. That order holds
within one width: a `sh` pair can land above byte stores written before it, so a
mixed-width run's source order is the `sb` sequence and the constant
materialisations, not the emitted store order. Two adjacent compound stores on one
record that differ in width behave the same way, seen from the loads: their stores
can issue in one order while the operands each one needs load in the other. Source
order sets those loads, so order the two statements by the original's load order,
not by its store order.
A run of stores to one pointer cell that parks a different record and restores the
saved value is real state: keep every publish and restore as a store to the cell and
re-read the cell after the restore, because holding the record in a local drops the run.
The re-read begins at the next call, not at the publish: a field store written between
the publishing store and that call keeps the local that holds the published pointer,
because the original consumes that value in the call's own delay slot, while an access
after the call addresses the named cell, since the callee may republish it and the
original's store then reaches the callee's record. Two exact bodies of one family
differ in nothing but that position, so read where the field store sits before
choosing.

An unrolled repeat over one strided table splits by field, not by record. The
original keeps one base for the field it advances by the stride and re-materialises
`$at` for every other word, so give that field a pointer local advanced between
records and write each remaining word as its own fixed-address dereference
(`p = D_t; *p = K; *PSX_PTR(s32, ADDR) = K2; p += 8;`). Keep the repeat spelled
out, since the original has no latch, and leave out fields it never writes.
Walking a whole record through the one pointer gives those stores the shared base
the original re-materialises.

Reads follow it in the other direction. When the original materialises a fixed-address
symbol once into a saved register and re-reads one byte of it before each call,
declare a local `u8 *` to that symbol so one base serves every read. Passing the symbol
expression to each call instead emits its own `lui %hi`/`lbu %lo` pair and loses the
shared base. A cell that holds a pointer takes the address of the cell: `T **cell =
&sym;` and `*cell` at each use. The value local (`T *p = sym;`) caches the pointee,
so it drops the original's later reload along with the shared base. A post-call reload
whose only difference is the register that receives its base (`$v1` where the original
builds `$v0`) is that local's lifetime, not allocation: keep the local for the accesses
ahead of the call and write the post-call test on the named cell, which cleared the
one-register residual. One read needs the same local when a store to another cell sits
between the base and the read: the pointer copy takes the base load early enough to
fill the preceding delay slot, while the inline symbol expression materialises the base
after that store and costs two instructions. A cell that is loaded, stored and
re-armed inside a guard wants it too: one base for all three uses is what lets the
original keep the re-arm constant in both branch delay slots. A fixed address the
original hands to a callee as its destination is this same local from the other side:
take the address once (`screen = &cell;`), pass the local, and read the result back
through it, while a cell of the same pair the body only reads keeps its own symbol
expression. A local that only re-types a promoted value (`u16 row = y;`) folds away
and changes nothing.

Give a work record a word-typed view before chasing pointer roles. A typed pointer over
the same storage fixes which value the pointer holds and where the second cell load
lands; a byte view leaves both ambiguous, so the gap shows up as a displaced load rather
than a wrong one. Check the original access order before widening the view.

A displaced store/load pair that keeps its score and its first difference through
reorder variants is the representation, not an ordering residual: give each accessed
field of the record a typed member over its evidenced offset with the unknown gaps
padded, and keep the separately loaded fixed cell on its own local. A member access does not enter the
`sched.c` `true_dependence` test against a standalone fixed cell, so the pair orders
against it where two pointer expressions cannot. Measured
negatives: reading that cell earlier, binding its value to a local, moving its
read-modify-write above the record's pointer stores, and one pointer local serving
both bases each kept the first difference or lost instructions.

Split record views by access width, not only by type. When word-pair updates match but a
byte cell's load is hoisted above them, keep the word pairs on one word-typed pointer
and reach that byte through a `u8*` derived from it:
`words[13] += words[3]; ((u8 *)words)[9]++; words[14] += words[4];`. Correct store
order alone still hoisted the byte load and kept extra live registers; the split view
closed it, so this drift needs the wider representation change rather than another
reorder. The same accesses read through signed words emit identical bytes, so
signedness is not that lever. The split can also sit inside one store run: a byte
store the original makes through the copy group's base stays on the derived view, and
the next store to that record re-reads the named cell, because a store through the
derived pointer may alias the cell. Two exact bodies reached a byte cell through the
derived view at the end of the word copy and the following byte store of that same
record through the named cell; writing both on the local gives both stores one base and
drops that reload.

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
boundaries before expression churn. A store group is one of those statements: when one
register tie survives declaration order, scope and narrowing variants with the same
`first=`, the residual is program order. Move that group to the point the original's
stores show, after the statement that produces the later value; the move alone flipped
the allocated pair and dropped the scheduling drift that followed the tie. Classify
retained improvement, retained frontier, reverted neutral/local regression/structural
regression. Rank exact bytes, matched
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
the self-assignment merges them. Where the later result is a product of a
separately loaded value, assign the product into the dead local and leave that
load in its own scalar. Mask, shift and operand-order spellings of that tail
stall at one hunk with the size already exact, so the residual belongs to this rung,
not to another shaping attempt. A copy is not that rung: copying a consumed operand
into fresh locals tied at every placement attempted, while defining the value a later
consumer needs into that same formal closed a size-and-saved-register residual,
because the original keeps two live phases of the operand and only a later real
definition keeps them apart. Reusing the dead formal for an unrelated quantity can
restore that pair and still leave a residual. Read a copy-only sweep that ties
everywhere as the operand being a formal, not a missing placement. Never add dummy
uses to inflate liveness. Compatible locals can also serve mutually exclusive arms
when each arm defines before use and no
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
at the same score, so they are not candidates for it. The same count lever reaches a
call result: when a masked call result is compared against a loaded table byte and
the two trade registers mid-body at equal size and frame, write the mask as its own
statement on the same local (`v = draw(); v &= MASK;`). The folded
`v = draw() & MASK` put the result in `$a0` and the byte in `$v1`, the reverse of the
original's `.greg` order, and the split statement restored the pair. Width, cast and
declaration-order spellings of that local measured identical there, so the statement
count is the lever and the type is not.

A narrowed induction temporary needs a complete reachable-range proof, including
the final endpoint. A higher score at the same size can be a retainable partial
while introducing a different residual. More matched instructions with missing
original bytes is not exactness; inspect conversion, mask, frame and flow changes.

### Orphan stack slots

Use this lever when the body matches but the frame differs, with no emitted body
access to the extra bytes. First exclude ABI, call, aggregate and address-escape
errors. An indexed access to a strided record type is no aggregate error and can
reserve that frame on its own, so try the record spelling before declaring a
padding local. It cuts the other way on a frame-less leaf over a strided table: a
record-typed view of three byte fields reserved a 16-byte frame the original does
not have, while the same statements as byte arrays with a computed byte offset
matched on the first gate, so read the original's frame before choosing the view.
In GCC 2.7.2, a folded address can leave an internal `USE` of a
pseudo whose definition disappeared. Reload may allocate a slot for that pseudo
even though the `USE` emits no instruction.

A bare prologue and epilogue with no saved register, every instruction outside them
already matching, and an original frame larger than this body's own locals is a
second cause: the reservation comes from a declaration the original translation
unit carried that emitted no code. One declared local of the missing size restores
the frame, and it takes no use, since a use would change liveness instead. The bytes
constrain only that size, so the name and type are a reconstruction choice rather
than recovered source: flag the declaration for review and record what removing it
leaves, which is the frame and offset residual with the body otherwise matching.
An empty body is that cause at its limit: original bytes of only a prologue,
`jr $ra` and a `nop` still take one declared local of the frame's size, so a frame
on a function that reads and writes no state is this lever, not a missing call or
a wrong boundary.

A local the body already uses bounds the frame the same way. When an aggregate it
passes to a callee sets the frame and the frame still falls short by a fixed gap,
widen that aggregate's declared extent rather than adding an unused local beside it:
every access and offset stays as written and the gap closes. Only the bytes the
callee reads evidence that type, so the wider extent is shape evidence, not a
recovered declaration.

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
   changes the base materialisation order and emits the base late. A subscript
   that carries a real row stride needs the named offset and a byte-typed base:
   naming the product alone left the base hoisted as `lui`/`addiu` plus
   `lw|sw 0(reg)`, and re-typing the base to `u8 *` with the byte sum still inline
   measured the same, each 20 bytes short with the first divergence at the base
   materialisation. Write the byte offset as its own local against the byte view
   (`offset = slot * 152; ((u8 *)D_x)[offset] += ...;`) so every access re-derives
   the base. Five respellings of the word-scaled subscript, including a named
   `slot * 38` and a separate load/store value local, measured identical.
3. An inline scale inside a fixed-address array subscript makes the whole symbol
   address loop-invariant. GCC hoists that base into a saved register before the
   loop and indexes from it, one instruction more than the original's per-iteration
   `lui $at,%hi(X)` + `addu $at,$at,offset` + `lbu %lo($at)` fold; the named offset
   restores the fold. A bare-index subscript such as `D_y[i]` already folds, so
   leave that one inline. That hoist is not generic: with the stride in its own
   register - a separate counting index feeding the bound compare and the stride's
   `addiu` in the latch delay slot - an inline `D_x[index << 3]` keeps the per-access
   fold and takes no saved base, so read the original's base register before naming
   an offset. Naming it sets the hoisting depth as well, because the declaration site
   bounds how far the base moves: on a row-and-column block the declared offset kept
   the per-access fold for the two byte-table lookups it served, while the destination
   address folded into the preheader and the preheader copies reordered at unchanged
   size, and moving that same declaration into the inner body let GCC hoist it one
   level and cleared both. When body size is already exact and the first divergence is
   the base materialisation, sweep the declaration site before another spelling.
4. Two passes over one table with the same stride need one offset local per pass.
   A shared local ties the second pass' product to the first pass' quantity, so the
   scale stops being block-local; keep each pass' own index reset and multiply.
5. Measure. Require the original frame, the original base load position and the
   complete function bytes before accepting.

`func_801D41E4` (47/47, 188 B) needed this after four forms: branch polarity first,
array symbol for both row uses, then the named product. The named product also
shrank the frame from 32 to the original 24 bytes, which no body instruction had
shown.

The mirror is not a lever: a sum the original recomputes at each use stays inline.
One local both edge stores read measured a hunk short at a wider frame, rewriting
the parameter in place measured eight bytes over with an extra saved register, and
writing the sum out at each store matched, because GCC still copies the parameter
and adds into that copy per site. Before naming, find the original's own separate
materialisation; a name serving only two existing uses widens a live range without
adding an instruction the original has.

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
inline literals re-opens the exchange. The arm-and-merged-tail form is worth trying
before any dump: when one constant is stored inside a switch arm and again at the
tail after the switch, assign it in each arm to one arm-local of the store width and
store that local at both sites. Without the local the compiler loads the constant
twice, and the first mismatch then sits at the case compare's target because the
storing arm grew by that load, not at either store, so read that signature as this
lever rather than as an arm-order defect.

## Compiler evidence

Before more allocator/scheduler probes, inspect the actual compiler build recipe,
relevant patches and source version. Bind diagnostic dumps to the source image,
effective compiler and unchanged codegen flags. Locate the first differing pass:
parameter expansion, combine, scheduling, jump optimization and delay-slot filling
(`reorg.c`), local/global allocation or reload. Debug dumps and assembly-only
diagnostics are not linked acceptance evidence.

A conditional branch whose delay slot holds one arm's constant while the `j` over
the other arm is gone, with the build short by that `j` and its `nop`, is `reorg.c`:
`fill_slots_from_thread` steals the branch's own fall-through arm constant, then
`relax_delay_slots` inverts the branch onto the join and deletes the `j`. That fill
tests the candidate with `insn_sets_resource_p` against what stays live on the
opposite thread, and a bare arm constant writes only the return register, so every
arm spelling passes the test. No spelling of the arms refuses the steal: put the
select inside a positive guard that owns the `return` so the arms have no shared
join behind one branch, per [control flow](#control-flow).

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
with the parent instead of reading payload placement from the row. A switch jump table
the original placed in leading table data is the case that row is for: write
`[[matching.section_placements]]` with the function, the section, the original's own
address and the table size, and the comparator lowers only that section's alignment to
the largest power of two dividing that address before it compares the emitted table
against the payload words. With the row the gate read every case word in its original
slot, including the slots no case owns.

Read the effective address of a dense symbol load, not the printed base. A
`lui at,0x8015` with the displacement `0x930C` resolves to `0x8014930C` because the
displacement is sign-extended, while `0x8015930C` would need the base `0x8016`. The
same displacement therefore appears under two different bases, and a misnamed symbol
still builds and links; the map and binding addresses are what settle it. A dispatched
handler table follows that rule too: bind the symbol at the word its scaled index is
added to, which can be an entry inside a longer in-image pointer run rather than the
run's first word. Two exact dispatchers of one overlay differed only in that table
immediate, so a run base is not automatically the table symbol. A run can also cross
a reviewed data-row boundary; that is no reason to split or extend a row, so declare
the extern at the indexed word and leave both rows as reviewed. A word the body
stores as a value binds the same way: take the address of that interior word and
leave the row whole, since a published address needs no row boundary.

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
The store that publishes a code address reads the same way from the other side: bind
the slot at the type its indirect call consumes and cast each stored callee whose own
prototype differs; a slot whose call passes an index or discards a result needs no
wrapper and no per-callee slot type.
An entry with no in-image reference at all is ordinary in an overlay, which is
entered from outside its payload: its prologue and return paths prove the boundary,
so a missing `jal` or table word is no reason to hold the lift.

When several dispatchers share one run, the word order names the caller. Number the
entries from the run base, then compare each candidate dispatcher's base entry with
the entry holding the function's own address: only a base at or below that entry can
reach it, because a dispatcher indexes forward from its base. One surviving candidate
is the caller, so no `jal` search is owed.

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
