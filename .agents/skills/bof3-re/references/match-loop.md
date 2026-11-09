# Clean-C match loop

## Required loop

Before changing existing C, obtain a live normal asm diff. For absent C, follow
the [first-source route](#first-source). A gate that stops before comparison yields
no `first=`: `undefined reference` at link is a binding gap, so add the row for the
address it names to this target's own map and re-gate the same source instead of
editing C. Rows are per target, so a cell or callee already exact in a sibling target
still needs its own row here; add those rows before the first gate, where the link
error names the missing symbols and, for a cell, the offsets of its uses.
Diagnose `first=`, classify through the
[playbook](matching-knowledge.md#delay-slots-and-entry-copies), make one structural
fix and rerun normal diff. Revert percentage regressions immediately. Use full diff
only for first/ambiguous diagnosis; partial catalogs cannot replace live diagnosis.

## Comparison views

Three views read the same native diff. Pick one before editing.

| Command | Shows |
| --- | --- |
| `bin/harness lift asm-diff TARGET@0xADDRESS` | summary plus the first hunk |
| `bin/harness lift asm-diff TARGET@0xADDRESS --detail full` | the complete patch |
| `bin/harness lift asm-compare TARGET@0xADDRESS` | every original and current instruction as aligned, numbered rows |

In `asm-compare` the left column is the original and the right column is the
current build. A differing row ends with `<<<`; `--` marks a side that has no
instruction at that point. `--context N` keeps differing rows plus N equal rows
on each side. Exit 0 means byte match, 1 means differences, 2 means the request
failed.

Reach for `asm-compare` when the score or a patch hunk does not explain the
difference: saved-register, insertion and scheduling drift read as a shifted
block in a patch, while the aligned rows show which side gained the instruction
and what stayed equal around it. Discovery of further actions: `bin/harness lift
--help`.

## First source

For genuinely absent C, require parent admission for the selected source. Pin original
bytes/load, reviewed boundary, caller/callee evidence, PRE absence, owned source/
header/map/layout paths and attempts. Create evidenced C89 with `@source`/`@behavior`,
manifest claim, declarations/bindings and `c` boundary. For a fresh member of a family
that already holds an exact lift, take that C89 from the sibling's body
([reverse protocol](reverse/mission-protocol.md)): where the twin differed only in
relocation words it measured first-gate exact, owed no sweep variant and left no
prototype or cell to declare, so `m2c`/`m2ctx` is not the next rung. Among siblings
that carry the identical span, transcribe the one whose own gate measured exact: a
span-equal sibling whose lift recorded a partial carries a spelling that reproduces
its residual, so keep it a shape donor rather than the seed. That residual need not
show as a score gap: a partial donor's spelling can reach the original's own byte
count while dropping instructions, first difference a callee-saved register shift,
so size parity never makes a partial seed the right one. A partial donor also reproduces its residual exactly: a seed taken from a 120/127 donor measured 120/127 at another target with the same block's slots differing, so a seed that lands on the donor's own score names the donor's spelling as the residual, not this body. A twin that is not
byte-identical still seeds once every remaining differing word shifts by the span's
own address delta, which is what an internal jump target does; a twin carrying one
differing word more than that delta explains differs in a carried constant, so read
it as a different variant of the body and not as the donor. Obtain a live
[native diff](native-execution.md) before another edit. Missing-source errors supply
no baseline; seeds grant no score or acceptance. Unresolved
ownership/ABI or failed native gates stop matching. Require final byte match and
independent review. Diagnosis requires existing C.

Exact metadata: `@status exact`, `@match 100.00`, `@residual none`. Notes on separate
lines, never semicolon suffix in tag. Naming repair needs fresh exact proof;
never weaken parser or repair unrelated rows to admit selected function.

## Terminal ladder

Use the earliest applicable rung and record inapplicability. Carry the full ordered
attempt ledger through repair, including missing history. Select experiments from
original/live evidence; repeat a tested lever only with changed evidence explaining
its expected effect. Restore regressions. Advance when evidence closes a rung or
proves its eligible alternatives exhausted, never after a fixed attempt count.
End at first byte match. Keep the owning effective compiler and flags fixed,
including reviewed attached settings. Flag catalogs and compiler-variant batches
are not ladder rungs or exhaustion requirements. Route actual pipeline defects
through the tooling owner; matching work grants no profile-change authority.

| Rung | Work | Gate |
| --- | --- | --- |
| 1. Representation/access | Prove ABI, widths, qualifiers, pointer cells and original reloads; inspect callees before declaring allocation-only. For an unlifted callee the declared parameter form is representation too: by-value pair/aggregate parameters expand at this call site and can justify a preheader register or a size deficit the scalar spelling never produces. Its declared return type decides the same way: with the callee declared `void` a literal the original keeps in `$v1` materialised in `$v0` and the body held at 11 of 13, while declaring the return the original's call result implies kept `$v0` for that result and matched. That decision belongs to call sites: an address-taken publish emits only the callee's address, so one exact installer stores both `void` and `s32` callees through its slots and the published return type moves no instruction there. Keep the callee's real return type and cast to the slot's type at the assignment; look for the residual at the calls, not at the publish. That width also follows the consumption: an answer tested against `-1` is the signed byte return, one compared with byte constants the unsigned byte return, and one exact body declares both widths at its two calls | Original calls, conversions and memory events accounted for |
| 2. Control flow | Branch direction, loop/return/switch; test [boolean return expansion](matching-knowledge.md#boolean-return-expansion), branch-local calculations, the branch matrix and evidenced [explicit cursors](matching-knowledge.md#explicit-cursors-and-signed-bounds) | Branch/topology mismatch clears |
| 3. Value lifetimes | Reuse a consumed local for a later real result; split a call answer read both raw and signed into two locals at the call; diagnose [orphan stack slots](matching-knowledge.md#orphan-stack-slots); test block-local initialization, [operation scopes](matching-knowledge.md#operation-scopes-and-loop-note-dependencies) and arithmetic grouping | Predicted instruction effect measured; semantics and complete diff checked |
| 4. Compiler evidence | Find the first divergent pass in owning sources and before/after RTL, including return expansion before CSE/allocation; follow the relevant [compiler lever](matching-knowledge.md#compiler-evidence) | Source-backed mechanism predicts a new probe; diagnostics grant no match |
| 5. Runtime evidence | For unresolved behavior, use bounded original/candidate observations through `psx-emulator` | Inputs, observations, stop and provenance checked; host edits and coverage limits explicit |
| 6. Permuter | Opt-in, shape already right; `bin/harness lift permute TARGET@0xADDRESS -j N`; keep owning profile fixed | Attempt completes, residual class recorded |
| 7. Unresolved | Preserve best coherent clean C and truthful partial metadata; report missing evidence | Non-exact disposition separates retention, blocker and exhaustion |

Read [matching knowledge](matching-knowledge.md) for each evidence branch. Runtime
work answers a named behavior question; it is not mandatory for a proven register
permutation. An unauthorized permuter stays untried, not exhausted.

Frame/size: types/calls, address-taken locals, aggregate copies and flow first.
Relocation/load order: symbol representation and original reload evidence first.
Entry copies: representation and live ranges before scheduler claims. For a lone
delay slot, prove branch operands and liveness; never introduce a clobber.

## Probe method

Use `lift sweep` for eligible clean-C alternatives without a call or variant quota.
No per-variant `asm-diff`/`byte-match` loop or local Python/shell compile/
compare harness. Reading one comparison needs no script: `lift asm-compare`
renders the aligned rows. A spelling question an exact sibling already answers owes no
variant: the sibling's compiled body at `out/asm-diff/<target>/<function>/current.s`
shows which access re-materialises a cell and which register holds a base, so read it
and write that form. In-process sweep resolves comparison context/canonical compile
command once, then compiles/compares each variant. Each variant rewrites/relinks.
Use `--subprocess` only for deliberate legacy reproduction. Give every variant the
measured `@source`/`@behavior` metadata: one without them is refused before
comparing and the sweeper prints `-`, while a plain `asm-diff` on that variant
prints no diff line at all, so record that as a non-measurement and fix the
variant rather than reading it as a neutral result.

Before each sweep, state the semantic precondition and predicted instruction
change. Change one statement class per variant, as in the repair loop: a variant
that also moves the base-local count or the store order cannot attribute its score
gain, so measure those classes in separate variants. Attribution is not independence:
a store-order variant and an arm-polarity variant that each measured neutral on the
same residual measured exact as their union, and neither delta predicted it, so when
two variants are neutral on the same residual the composed spelling is the next probe.
Partial gains compose the same way: two order levers that each closed one instruction
and neither finished measured exact as their union, so a retained partial gain is
input to the next composed variant, not a lever to re-test alone.
Compare the full result, not score alone. Restore neutral/regressive probes.
Retain a strict coherent partial improvement only after semantic review and truthful
metadata; retention is not exact acceptance or exhaustion. Near-exact allocation
differences trigger compiler evidence, not another spelling batch.

Reviewed exhaustion requires the separate exhaustion gate. A residual class,
negative batch or lack of a new idea does not prove it.

## Eligible-condition levers

Classify before first edit. Use eligible lever before clean-C attempts. Record
ineligible/declined reasons; revisit only with new evidence.

| Condition | Lever | Evidence and authority |
| --- | --- | --- |
| No function-shaped bytes: neither a prologue nor a `jr $ra` return path, pointer runs, printable blocks; or ASM-backlog triage | `bin/harness analysis data-candidates [TARGET...]` | Ranked byte evidence only. Promotion requires reviewed layout transaction, `source splat` and index refresh, not C lift. A target-wide run also settles one selector: a range with its own prologue and `jr $ra` return that is absent from that target's ranked list declines the data lever on measured evidence, with no per-range probe owed |
| Fragment/jump-table arm/shared epilogue with head elsewhere; code inside leading bare `[offset, bin, name]` segment or `bin` subsegment | `bin/harness analysis carve TARGET@0xADDRESS START:END` | Isolated two-split row/segment regroup probe preserving `vram`/text order. Never applies a layout change. A range that already starts at its own declared entry and ends at the next declared one has no fragment boundary to regroup, so carve is ineligible and owes no probe. Lane reports evidence; parent owns reviewed Splat placement, no whole-image oracle. A head that sits in a `textbin` row is ineligible: the probe regroups `bin` subsegments and bare `bin` segments only and refuses with `carve range is not inside a single bin subsegment or bare bin segment`, so that head row and the `asm` body become one reviewed `c` row with the old map rows replaced, and until they merge no body seed can match, because the diff opens after the head. The same merge covers a body split across several declared rows. A code row between the body's entry and the next declared entry is live code when the body's last access continues into it, so a `data-candidates` report of `kind=no_function_shape` / `jr_ra=False` on that fragment describes the fragment, not the bytes, and a trailing `asm` row whose only in-image reach is the body's own branch targets is that body's epilogue. Merge all of them into the one `c` row ending at the next declared entry before seeding, because the layout, not the C, is what the match was waiting on |
| Few clean-C shapes explain `first=` | `bin/harness lift sweep SELECTOR SOURCE variants...` | In-process comparison; retain each variant and outcome |
| Join-spanning boolean result or extra return copy | Compare expression return with [constant-return arms](matching-knowledge.md#boolean-return-expansion) | Predict the RTL change; preserve arithmetic, accesses and ABI; require the complete live gate |
| Overlay step dispatcher: scratchpad work-byte slot, scale by four, indirect call through a local handler table | `EMI_DEFINE_DISPATCHER` or `_THEN_RESET` in `include/bof3/emi/dispatchers.h`, with the matching `EMI_DISPATCH_WORK_BYTE_n` and shape variant | Emits the recorded 17-instruction, 68-byte body; keep the metadata block above the invocation. That body loads the record pointer before it creates the frame, so a hand-written variant keeps that order. Covers the bare body only: a dispatcher that keeps working after the call (second cell load, then a gate on the work bytes) is hand-written with one pointer local per original reload point and shares only the table extern. A hand-written bare body also reaches exact but duplicates the shared definition. Index width is part of the condition: the variants cover an unsigned byte slot only, and `n` is the record byte the original's `lbu` reads, not its table base, so a dispatcher that reads a wide slot of the same cell is not a macro gap. Cell identity is the same kind of condition: the macro hardcodes the scratchpad work slot, so a member whose record pointer comes from an in-image word is no macro gap either; declare both cells and call `TABLE[CELL[1]]()`, which reached the identical body. Write it by hand as `TABLE[SPAD_PTR_SLOT(u16, 0x44u)[slot]]()` with this function's own slot, which reaches exact on the first gate; do not add a variant or force the byte form. That typed base makes the subscript an element index, so a halfword at record byte 0x2C is `[0x16]`; spelling the byte offset loads the wrong halfword. A dispatcher whose whole handler table is two function addresses seeded into its own frame is neither shape: declare a local array of function pointers initialized with those addresses and index it with the work byte, because the macro indexes an extern table in place, never stores addresses into the frame and takes no index argument. How many of those words are seeded is not part of that condition: a two-word frame table with only word 0 seeded, called through the state byte with no index bound, matched on the first gate with the same local array and indexed call. An extern table copied into the frame is that same exclusion with an aggregate assignment in place of the initializer: `handlers = D_table; handlers.handlers[slot]();` emits the copied words and reads the call target back from them. An indexed body whose index arrives in the first argument register (`andi a0,0xFF` then `sll 2`, table loaded from its own immediate) is no macro gap either: declare the extern at the word the scaled index adds to and call `TABLE[index]()` in one statement |
| Shared gate byte selects one of two constants into one shared status halfword | `WORLD_DEFINE_GATE_STATUS(function_name, gate_byte, status_halfword, when_nonzero, when_zero)` in `include/world/gate_status.h` | Matched on the first gate, and the macro already backs exact users in four overlays. The macro carries the branch and both arm constants, so a hand-written `if`/`else` duplicates the shared definition and re-opens an arm order that this shape is sensitive to. Declare the gate cell at the width and signedness its own load shows; per function only the two constants and the cell identities vary. Two blocks that advance one field under separate gate conditions select no constant, so keep them ordinary `if`s rather than this macro. A gate that only decides whether a constant lands in a different cell, while the shared halfword takes one constant on both paths, selects no arm either: write the plain `if` and then the unconditional store. The mirror order is a shape too: when the original's `sh` to the shared halfword precedes the guard's branch while the arm writes several byte cells, write that store first and put every arm store inside one `if`, which matched on the first gate with the arm's first constant filling the branch delay slot. Read that order off the `sh` position, not the arm spelling. A non-zero arm that stores a second cell after the status halfword is no macro shape either, since the macro emits one store per arm; write that pair as the plain `if`/`else` with the extra store following the status store, which matched on the first gate |
| Shared status byte takes the constant 2 and a `u16` counter moves by 0x14 | `COUNTER_ADVANCE(function_name, counter, flag)` or `COUNTER_RETREAT(...)` in `include/game/counter_step.h` | Emits the recorded body: read the counter into a local, store 2 into the status byte, then store counter ± 0x14, so the read stays ahead of both stores. Exact users in several overlays share it, and a hand-written body duplicates the shared definition. Eligibility is that trio: mode 2, movement 0x14, `u16` counter extern; a body with another amount, another mode constant or extra cells is no macro gap, and an ineligible amount still owes this recorded order, since writing it in place as `counter = (u16)(counter - K)` around the status store measured 7 of 11 instructions and four bytes over the original while the load-first order matched, so take the order and not the macro |
| Bytes identical to already-exact function | `bin/harness lift clone TARGET@0xADDRESS --from REPRESENTATIVE` | Dry-run emits self-contained raw `func_`/`D_` clone, no foreign header. Cross-target only: the adapter refuses a representative from the clone target's own binary (`clone target is the representative's own target`), so a byte-identical sibling inside one target is copied-body shape evidence, not a clone. A declared replica payload with its own target id, the representative's sha256 and its load address is that same binary, so the adapter refuses it too; the lift already covers the replica's span at that address, and a separate registry row for the replica is a parent decision. `--apply` wires and retains only on `lift gate` PASS, otherwise restores. Still needs own exact gate/required review; `--apply` runs that gate itself, so a lane reporting one gate batch takes the dry-run body and performs the same splat/map/manifest wiring by hand. That hand copy is also the route when this target already binds an evidenced name for a symbol the body touches, such as a shared scratchpad cell every sibling names: `--apply` writes the dry-run's raw spelling, while the copy keeps the evidenced name at the same bytes. The same choice is owed for the body itself: `--name` carries the evidenced name into the emitted body and, on `--apply`, into the splat row, the map row and the filename, so a body whose own evidence names it needs no rename afterwards, and the hand copy is owed only when the body carries more than the label — this target's own `@behavior` reference facts, or a symbol it binds by an evidenced name, since `--apply` writes referenced symbols in their raw spelling. That spelling can also carry the wrong kind, because it writes what the representative's own bodies reached: a callee that representative addressed as data comes back as `D_<ADDRESS>`, so decline `--apply` when this target binds that address as a function and copy the body instead. Declined target-local include/unresolved external is recorded, never bypassed. A representative whose body reaches a shared-header macro the clone target does not declare fails the dry run with `unresolved external 'SPAD_PTR_SLOT': not declared, not in the target map`, even when the two ranges are byte-identical; those bytes still lift as a shape copy of that sibling, so record the refusal and seed from it rather than retrying the adapter. Prove identity yourself with one `cmp` of the two payload spans at their own `runtime address - load address` offsets, which is the evidence the dry run would have reported. A payload scan that
finds no twin proves nothing until it is repeated with each `jal` immediate word
masked: a relocation-different sibling differs exactly in those words, so the
unmasked miss is inconclusive and a masked hit is that shape donor. Decode those
words in the payload's own byte order: a big-endian pass reported a clean miss on a
span whose six masked siblings sit in the corpus, so a zero-hit masked scan is
inconclusive until it repeats little-endian. Masking covers
call-target words only, so a miss that stands on a table base, an installed pointer or
other data immediate is conclusive and owes no masked re-scan: that word belongs to
this body, and no other payload holds it. A branch or jump target inside the span is
not that class: it names the span's own code and relocates with the span, and one
member measured against six siblings differed in exactly its two internal jump words,
so mask those words too. A masked miss that stands only on one of them is that
relocation, not proof that no twin exists. A raw `func_<ADDRESS>` row carrying the same label at the same address in a sibling target is not that evidence: sibling targets holding one such label at one address held unrelated bytes there, so the twin is a span match, never a name match. Nor is a pointer word: a sibling that loads at the same base can carry this address only because it names its own same-offset function, so that word is a self-reference, not a copy of this body. A family whose members each encode their own table base, callee address, passed constant or carried bound (a loop count, a length) in the instruction words can never be byte-identical to one another, so clone is ineligible across it and the family's shared body is the shape lever. Narrow that set before scanning: when every call target and referenced symbol of the span resolves in the shared main executable, only those carried constants can differ, so a member that differs from its sibling in one bound alone declines clone and takes the sibling as shape donor. A bound is a data immediate, so that miss is conclusive and owes no masked re-scan. Read that lever as a transcription of statement order, guard nesting and local shapes, and re-read each member's own table base, callee address, passed constant and bound from its bytes: those are exactly the words that differ between members, so a donor constant copied across is the one defect the shape never shows. A duplicate group whose `promotion_blockers` read `["no exact member"]` declines for lack of a representative, not for lack of shape; the first member lifted to exact becomes the representative its byte-identical peers clone from. A twin that differs in how many calls it makes is not one of those peers at any byte count, so count call sites before comparing sizes. Scan every extracted payload under `out/binaries`: twins cross area directories and archive indices, so a search confined to the current family or area directory is incomplete evidence |

Search the applicable levers before exhaustion. Byte identity proves no shared ownership.

For a grouped representative, use `--from TARGET@0xADDRESS` to select one attached
function. Clone retains common declarations and includes and rejects target-local
`*_internal.h` headers. It compares both complete reviewed ranges using validated
PS-X payload placement. Address rewriting changes only the selected source tag;
body literals stay unchanged.

## Review and stop

Non-exact review proposes up to three supported untried experiments with expected
instruction effects. If none is justified, name the evidence gap; do not invent a
probe or claim exhaustion. Caller owns retries/stopping. Retain coherent improvement with atomic
partial metadata; revert no-progress/semantic defects. Partial-to-exact review
identifies decisive experiment; parent records reusable playbook/lesson proposal.

Percentage alone proves no success. Do not use matching-hack macros or historical
aid exemptions. Report target, first difference, attempts and next untried/blocked
evidence on exhaustion. Require final live byte-match exit 0 and independent review,
subject to the canonical lane's explicit exact-PASS reviewer exception.
