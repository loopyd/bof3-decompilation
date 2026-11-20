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

If only the branch
polarity differs and its target offset matches, retain the donor's arms and invert
the condition. That opcode difference is not a relocation delta, so the twin is a
shape seed, not clone-eligible.

For genuinely absent C, require parent admission for the selected source. Pin original
bytes/load, reviewed boundary, caller/callee evidence, PRE absence, owned source/
header/map/layout paths and attempts. Create evidenced C89 with `@source`/`@behavior`,
manifest claim, declarations/bindings and `c` boundary. For a fresh member of a family
that already holds an exact lift, take that C89 from the sibling's body
([reverse protocol](reverse/mission-protocol.md)): where the twin differed only in
relocation words it measured first-gate exact, owed no sweep variant and left no
prototype or cell to declare, so `m2c`/`m2ctx` is not the next rung. Proven regions can be whole record arms or single statements.
Combining an exact donor's gate test with another's masked update measured exact
at the first gate. No donor has to cover the whole span. A donor may have a different size. Reuse only its proven matching regions and write
the rest from this span's bytes. Among siblings
that carry the identical span, transcribe the one whose own gate measured exact: a
span-equal sibling whose lift recorded a partial carries a spelling that reproduces
its residual, so keep it a shape donor rather than the seed. That residual need not
show as a score gap: a partial donor's spelling can reach the original's own byte
count while dropping instructions, first difference a callee-saved register shift,
so size parity never makes a partial seed the right one. A partial donor also reproduces its residual exactly: a seed taken from a 120/127 donor measured 120/127 at another target with the same block's slots differing, so a seed that lands on the donor's own score names the donor's spelling as the residual, not this body. A twin that is not
byte-identical still seeds once every remaining differing word shifts by the span's
own address delta, which is what an internal jump target does; a twin carrying one
differing word more than that delta explains differs in a carried constant, so it is
no clone source, yet it still seeds transcription: keep its statement shape and write
every constant from this span's own bytes, which measured exact on the first gate for
siblings differing only in a cell address, in a pool base together with its bound, and
in a set of literal ids. That twin may live in another target: the exact lift there
transcribed into this target on the first gate once the table and cell words came from
the receiving span's own bytes. The cell-access spelling carries the same way: a
pointer cell written by name and a paraphrase of it into a fixed-address constant
address compile differently, so a seed that swaps one for the other opens a rung-1
base-materialisation miss ([scratchpad cells](matching-knowledge.md#scratchpad-cells-and-record-views)), and
that miss is this transcription rather than a wrong donor. For an extra base-building
`addiu` before a fixed-address halfword access, test the pointer-local form in
[halfword read-modify-write](matching-knowledge.md#halfword-read-modify-write)
before rejecting the donor's statement shape. Obtain a live
[native diff](native-execution.md) before another edit. Missing-source errors supply
no baseline; seeds grant no score or acceptance. Unresolved
ownership/ABI or failed native gates stop matching. Require final byte match and
independent review. Diagnosis requires existing C.

Exact metadata: `@status exact`, `@match 100.00`, `@residual none`. Notes on separate
lines, never semicolon suffix in tag. A name refused after the body matched is a
name-only repair: keep the matched body and its proof, change the name, re-gate in the
same batch. Never re-lift a matched body to satisfy a name, weaken parser or repair
unrelated rows to admit selected function.

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
| 1. Representation/access | Prove ABI, widths, qualifiers, pointer cells and original reloads; inspect callees before declaring allocation-only. Measure caller and callee separately: a partial callee does not prevent its caller from matching the original call instructions. For an unlifted callee the declared parameter form is representation too: by-value pair/aggregate parameters expand at this call site and can justify a preheader register or a size deficit the scalar spelling never produces. Its declared return type decides the same way: with the callee declared `void` a literal the original keeps in `$v1` materialised in `$v0` and the body held at 11 of 13, while declaring the return the original's call result implies kept `$v0` for that result and matched. That decision belongs to call sites: an address-taken publish emits only the callee's address, so one exact installer stores both `void` and `s32` callees through its slots and the published return type moves no instruction there. Keep the callee's real return type and cast to the slot's type at the assignment; look for the residual at the calls, not at the publish. That width also follows the consumption: an answer tested against `-1` is the signed byte return, one compared with byte constants the unsigned byte return, and one exact body declares both widths at its two calls. An answer tested only against zero is narrowed at the test, not in the declaration: a zero test reads the same at either sign, so the callee keeps its full returned word and the test carries the cast (`(u8)callee() == 0`), and where the callee's own row is still `asm` that word comes from the register its body returns rather than from this test. A pool allocator's answer that the span never compares against its no-free-entry code takes no failure arm: keep the full returned word on the declaration and cast the index at the assignment (`slot = (u8)alloc(kind, mode)`), because a `==` guard on that code would emit a compare and a branch the original does not have | Original calls, conversions and memory events accounted for |
| 2. Control flow | Branch direction, loop/return/switch; test [boolean return expansion](matching-knowledge.md#boolean-return-expansion), branch-local calculations, the branch matrix and evidenced [explicit cursors](matching-knowledge.md#explicit-cursors-and-signed-bounds) | Branch/topology mismatch clears |
| 3. Value lifetimes | Reuse a consumed local for a later real result; split a call answer read both raw and signed into two locals at the call; diagnose [orphan stack slots](matching-knowledge.md#orphan-stack-slots); test block-local initialization, [operation scopes](matching-knowledge.md#operation-scopes-and-loop-note-dependencies) and arithmetic grouping | Predicted instruction effect measured; semantics and complete diff checked |
| 4. Compiler evidence | Find the first divergent pass in owning sources and before/after RTL, including return expansion before CSE/allocation; follow the relevant [compiler lever](matching-knowledge.md#compiler-evidence) | Source-backed mechanism predicts a new probe; diagnostics grant no match |
| 5. Runtime evidence | For unresolved behavior, use bounded original/candidate observations through `psx-emulator` | Inputs, observations, stop and provenance checked; host edits and coverage limits explicit |
| 6. Permuter | Opt-in, shape already right; `bin/harness lift permute TARGET@0xADDRESS -j N`; keep owning profile fixed | Attempt completes, residual class recorded |
| 7. Unresolved | Preserve best coherent clean C and truthful partial metadata; report missing evidence | Non-exact disposition separates retention, blocker and exhaustion |

Read [matching knowledge](matching-knowledge.md) for each evidence branch. Runtime
work answers a named behavior question; it is not mandatory for a proven register
permutation. An unauthorized permuter stays untried, not exhausted.

An opt-in permuter requires a cited residual and parent authorization. Verify its
generated `compile.sh` against the owning recipe before execution. Disable
forbidden transformations and inspect the remaining implementations: names and
randomization regions do not guarantee ABI or scope boundaries. Review the entire
candidate; reject artificial aliases or qualifiers, undefined uses and changed
memory or call behavior. Before expanding a saved expression, check intervening
writes to all its inputs. Permuter scores guide search; only the complete owning
native gate proves a match.

`--best-only` can suppress valid candidates after an invalid result lowers the
best score. In a bounded run, `--all-improvements --only-if-below N` permits
candidates with higher scores below N and no worse than the base. Duplicate
results are filtered; review each saved source.

A reported match count can rise when a new NOP restores later branch addresses
while an earlier missing NOP and allocation error remain. Inspect the added
instruction and changed delay slots before retaining the candidate. Check which
local residual improved; equal size and more matching rows alone do not prove it.

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
variant rather than reading it as a neutral result. A batch where every row is `-`,
including a variant that is the installed seed, measured nothing: that is the route,
not the shapes, so fix the route and take the retained candidate's own `asm-diff`
plus the gate as the measurement. A sweep row is that variant's own
measurement, not a reproducible one: variants with identical codegen scored
differently, and a shape the sweep reported a few instructions short at the original
byte size reached exact on the native gate, so when two rows disagree, or a retained
shape sits short at equal size, install that variant and gate it natively before
discarding it.

Before each sweep, state the semantic precondition and predicted instruction
change. Change one statement class per variant, as in the repair loop: a variant
that also moves the base-local count or the store order cannot attribute its score
gain, so measure those classes in separate variants. Attribution is not independence:
a store-order variant and an arm-polarity variant that each measured neutral on the
same residual measured exact as their union, and neither delta predicted it, so when
two variants are neutral on the same residual the composed spelling is the next probe.
Partial gains compose the same way: two order levers that each closed one instruction
and neither finished measured exact as their union, so a retained partial gain is
input to the next composed variant, not a lever to re-test alone. Three attributable
variants that each moved a different residual are the point to cross those axes in one
sweep instead of composing two at a time: the deciding axis can be two at once, and
four of ten rows in one grid measured exact, so gate a grid hit natively rather than
attributing it to a single class.
Compare the full result, not score alone. Restore neutral/regressive probes.
Retain a strict coherent partial improvement only after semantic review and truthful
metadata; retention is not exact acceptance or exhaustion. Near-exact allocation
differences trigger compiler evidence, not another spelling batch.

Reviewed exhaustion requires the separate exhaustion gate. A residual class,
negative batch or lack of a new idea does not prove it.

## Eligible-condition levers

Classify before first edit. Use eligible lever before clean-C attempts. Record
ineligible/declined reasons; revisit only with new evidence.
A frameless leaf can be a complete function. Verify its entry, control flow,
accesses and return within the reviewed range; a `jr $ra` word alone is not proof
that the range is code.

| Condition | Lever | Evidence and authority |
| --- | --- | --- |
| No function-shaped bytes: neither a prologue nor a `jr $ra` return path, pointer runs, printable blocks; or ASM-backlog triage | `bin/harness analysis data-candidates [TARGET...]` | Ranked byte evidence only. Promotion requires reviewed layout transaction, `source splat` and index refresh, not C lift. A target-wide run also settles one selector: a range with its own prologue and `jr $ra` return that is absent from that target's ranked list declines the data lever on measured evidence, with no per-range probe owed |
| Fragment/jump-table arm/shared epilogue with head elsewhere; code inside leading bare `[offset, bin, name]` segment or `bin` subsegment | `bin/harness analysis carve TARGET@0xADDRESS START:END` | Isolated two-split row/segment regroup probe preserving `vram`/text order. Never applies a layout change. A range that already starts at its own declared entry and ends at the next declared one has no fragment boundary to regroup, so carve is ineligible and owes no probe. Lane reports evidence; parent owns reviewed Splat placement, no whole-image oracle. A head that sits in a `textbin` row is ineligible: the probe regroups `bin` subsegments and bare `bin` segments only and refuses with `carve range is not inside a single bin subsegment or bare bin segment`, so that head row and the `asm` body become one reviewed `c` row with the old map rows replaced, and until they merge no body seed can match, because the diff opens after the head. The same merge covers a body split across several declared rows. A code row between the body's entry and the next declared entry is live code when the body's last access continues into it, so a `data-candidates` report of `kind=no_function_shape` / `jr_ra=False` on that fragment describes the fragment, not the bytes, and a trailing `asm` row whose only in-image reach is the body's own branch targets is that body's epilogue. Merge all of them into the one `c` row ending at the next declared entry before seeding, because the layout, not the C, is what the match was waiting on. The selector itself can name a prologue whose body starts two instructions earlier: the compiler hoists a fixed-address load above the frame setup, so that `asm` row opens on its own `addiu sp` and ends at the next declared start and passes every self-containment test on the late start. Read the payload instead: no branch word targets the address and no payload word equals it, while the only in-image reference is a pointer word to the row two instructions earlier, and another target's copy of the body carries the same split. Merge that row in and lift from the earlier address |
| Few clean-C shapes explain `first=` | `bin/harness lift sweep SELECTOR SOURCE variants...` | In-process comparison; retain each variant and outcome |
| A result needs a raw copy and a saved narrow value, or CSE changes a real copy | Trace [CSE width conversions, copy direction and value phases](matching-knowledge.md#compiler-evidence) | Prove narrowing at every consumer; trace real quantities, actual basic blocks, adjacent retargeting, equivalence propagation and flow deletion. Count surviving SETs and deaths separately before allocation, then require the complete native gate |
| A narrowing justified by source ranges adds a conversion before scaling | Trace [consumed bits and RTL known-bit facts](matching-knowledge.md#compiler-evidence) | Count the input bits that still affect the result; prove the compiler can discard the conversion before predicting an allocation repair. Require the complete native gate |
| An algebraically equivalent arithmetic sequence keeps extra operations | Trace [simplifier entry conditions and substitution guards](matching-knowledge.md#compiler-evidence) | Use consumed arithmetic phases; check each rule and intervening overwrites. Verify cleanup, counts, intervals and complete native output |
| Packing disappears but a frame remains without spill accesses | Trace [combine and reload](matching-knowledge.md#compiler-evidence) | Inspect surviving death-note `USE`s, register classes and reload stack homes. A silent `USE(MEM)` can retain a frame; check the complete output before retaining the representation |
| A preserved producer's captured result still forwards into its consumer | Trace [producer movement and result forwarding](matching-knowledge.md#compiler-evidence) | Use only consumed phases. Verify actual operand and result overwrites, the surviving handoff, deaths, register classes and lowword extraction. Require the complete native gate |
| A raw bound and its selected narrow copy interfere at the comparison | Trace [comparison results and live overlap](matching-knowledge.md#compiler-evidence) | Preserve the existing comparison and effects; verify the source death at the copy, interference and both selections in the complete native gate |
| A guarded increment loses the copy into its result | Trace [comparison capture and repeated field samples](matching-knowledge.md#compiler-evidence) | Keep the actual predicate before the copy, preserve each fresh sample, and check CSE boundaries, pointer phases, allocation and the complete native gate |
| A literal capture fixes load order but delays its publication or changes a pointer register | Trace [publication scheduling and literal intervals](matching-knowledge.md#compiler-evidence) | Group only real publications and inputs supported by the original order; preserve fresh reloads and check both schedulers, interval conflicts and the complete native gate |
| Lookup setup hoists into input arithmetic and changes a later value phase's register | Trace [address temporaries and hard-register conflicts](matching-knowledge.md#compiler-evidence) | Scope only the real arithmetic operation; verify the lookup dependency, removed overlap and complete register web before the native gate |
| Frame-base and cell address merge, or loop reduction creates the wrong carried pointer | Trace [CSE, local quantities and the first real consumer](matching-knowledge.md#compiler-evidence) | Preserve each memory event; check whether the first indexed store can precede the cell-address phase, then inspect the loop dump and complete native gate |
| Arithmetic is regrouped or its immediate changes in initial RTL | Trace [frontend folding, widths and shared constants](matching-knowledge.md#compiler-evidence) | Prove each consumer's representation and retain the final conversion. A real shared constant binding may defer folding to CSE; require initializer removal, direct result modes, preserved masks and the complete native gate |
| A widened local is narrowed at a real consumer | Trace [tree narrowing and multiword liveness](matching-knowledge.md#compiler-evidence) | Prove the introduced conversions and preserve accesses and ABI. Inspect initial RTL, partial definitions, high-word work and paired allocation; narrowing the arithmetic does not prove that the wide capture disappears. Require the complete native gate |
| A small body change exchanges saved loop registers, or a last-index zero test gains a literal | Trace [flow deletion, live length and consumed arithmetic results](matching-knowledge.md#compiler-evidence) | Preserve real initializations, counts and stores; inspect the first changed pass and all saved roles before the complete native gate |
| Store/readback becomes a mask, a later reload disappears after loop motion, or a table base gains an extra saved register | Trace [CSE forwarding and consumed cursor phases](matching-knowledge.md#compiler-evidence) | Preserve each access; trace the real next value that consumes the local, then inspect CSE before and after loop motion and require the complete native gate |
| A const record view lets a load cross mutable stores | Trace [readonly memory flags and capture order](matching-knowledge.md#compiler-evidence) | Prove storage writers and the access contract; inspect affected loads, both schedules and the complete native gate |
| A scope changes register classes, quantity ties, ready priority or a load-delay NOP | Trace [CSE regions and boundary receivers](matching-knowledge.md#operation-scopes-and-loop-note-dependencies) | Check classes and surviving quantities before counting references. Distinguish register-death notes from internal scope markers when tracing ready priority. Inspect receiver edge kinds, latency costs and emitted delay nops after reload; a real loop-test capture can move the receiver to its branch. Require the complete native gate |
| A halfword field copy sinks below a fixed scalar read, or a scope orders a store ahead of its copy reads | Trace [memory-dependence rules and the scope's first real load](matching-knowledge.md#compiler-evidence) | Preserve actual fields, access widths and reloads; inspect CSE and both scheduling passes, then require the complete native gate |
| A clamp loses its raw input or byte snapshot, or a pointer-cell load sinks past destination stores | Trace [raw/result quantities, byte promotions and pointer sampling](matching-knowledge.md#compiler-evidence) | Trace copy motion and allocation; for byte snapshots inspect CSE, combine and global priorities. Preserve the keep-value arm and distinguish cell sampling from pointee reads; prove cell independence, then require the complete native gate |
| A narrow snapshot disappears or changes an argument-register reservation | Trace [consumer conversions and argument preferences](matching-knowledge.md#compiler-evidence) | Compare the consumer RTL and combine result. Keep only real value phases, then inspect conflicts and the complete native gate |
| A pointer copy survives CSE but a store switches to the copied base | Trace [copy forwarding and both schedules](matching-knowledge.md#compiler-evidence) | Compare the first schedule, local allocation and scheduling after reload. A one-definition pointer live at block exit can keep the copy late until after forwarding; require the complete register web and native gate |
| A pure binding lands on the wrong side of tied initializations | Trace [ready queues and stable instruction order](matching-knowledge.md#compiler-evidence) | Distinguish initial readiness from dynamic promotion; move only an existing independent binding, then check both schedules, the complete register web and native gate |
| A narrow capture changes distant temporaries, or an update gains a call dependency after reload | Trace [destination forms, local conflicts and call dependencies](matching-knowledge.md#compiler-evidence) | Compare direct `REG` and `SUBREG` destinations, adjusted priorities, local intervals and global hard conflicts. Inspect the pre-reload pseudo and new call edges separately from memory flushing; a later stack store does not prove a first-schedule dependency. Preserve justified types and require the complete native gate |
| A comparison result uses a fresh register where the original overwrites its bound | Trace [local-allocation eligibility and operand/result tying](matching-knowledge.md#compiler-evidence) | Separate actual raw-input and bound quantities when reuse creates multiple deaths; check block membership, overlapping address temporaries and the complete native gate |
| A shared pointer-cell tail prevents delay-slot filling | Trace [tail merging and memory resources](matching-knowledge.md#compiler-evidence) | Inspect `.jump2` and `.dbr`; choose qualifiers from the access contract, preserve every original memory event and require the complete native gate |
| CSE deletes a narrow arithmetic local and the result chain excludes a cursor | Trace [surviving word quantities and cursor deaths](matching-knowledge.md#compiler-evidence) | Separate actual cursor samples and retain real word arithmetic quantities; verify CSE, allocation classes, memory widths and the complete native gate |
| A byte sample goes directly into a saved register before classification | Trace [guarded sample and narrow snapshot lifetimes](matching-knowledge.md#compiler-evidence) | Assign the real snapshot after rejecting guards; preserve byte loads and reloads after helpers, then check deaths, conflicts and the complete native gate |
| Signed division lacks the original trap-check instructions | Trace [assembler expansion](matching-knowledge.md#signed-division-expansion) | Compare compiler and linked assembly; route a profile defect through the pipeline owner with explicit authority and complete affected-function gates |
| Signed division has the right arithmetic but a different register web | Trace [division quantities and allocation](matching-knowledge.md#compiler-evidence) | Account for unused remainder outputs, single-death eligibility and global conflicts; split real sample/divisor/quotient phases and inspect in-place masks before the complete native gate |
| Narrow coordinate addition hoists, or its input ties to a saved result | Trace [first and second CSE around loop motion](matching-knowledge.md#compiler-evidence) | Capture only a real input; verify both definitions survive until loop optimization, later CSE removes the capture, and shared inputs do not become duplicate address computations. Require the complete native gate |
| A raw value outranks its narrow copy but loses the copy's argument register | Trace [global preference reservation](matching-knowledge.md#compiler-evidence) | Inspect both pseudos' preferences and conflicts, `prune_preferences` and `find_reg`. Preserve real consumers and require the complete native gate |
| A scope around a multiply moves its result copy ahead of loads | Trace [reload output copies and loop-note dependencies](matching-knowledge.md#operation-scopes-and-loop-note-dependencies) | Inspect the inserted `mflo` and the next instruction's dependencies in `.greg` and `.sched2`. Preserve the original placement of both the multiply and its result copy before the complete native gate |
| An output MFLO followed by a GPR copy appears coalescible | Trace [reload entry guards](matching-knowledge.md#compiler-evidence) | Check whether the hard-register move enters reload emission and whether input-reload retargeting applies. Follow the actual output destination; require the complete native gate |
| CSE does not retarget a stack-argument load into the first body copy | Trace [deferred parameter conversions](matching-knowledge.md#compiler-evidence) | Inspect initial RTL, canonical representatives and conversion SETs between the load and copy. Change a formal only with ABI and consumption evidence, consistent declarations and complete caller preservation |
| A computation-only scope fixes store order but exchanges locals | Compose [real computation and update groups](matching-knowledge.md#operation-scopes-and-loop-note-dependencies) | Preserve the outer boundary; verify both receiving instructions, quantities and final delay slots before the complete native gate |
| An approved permuter pass can change dependencies or return paths | Use [finite reviewed choices](matching-knowledge.md#permuter-input-safety) | Inspect installed implementations and disable unsafe passes. Preserve the owning recipe, review the complete candidate and require its native gate |
| Product accumulation changes multiply or load order while entry copies remain | Trace [real product phases and scheduled pointer deaths](matching-knowledge.md#compiler-evidence) | Check whether an actual shared multiplier starts a surviving projection quantity. Compare CSE, copy consumers, deaths, origin-load assignments and both schedules. Preserve arithmetic types and execution paths; a global return preference does not reserve a local register |
| Folding merges differently grouped arithmetic, or a local result changes distant scratch registers | Trace [real arithmetic phases and reload ranking](matching-knowledge.md#compiler-evidence) | Preserve only consumed intermediate definitions; verify both CSE passes and combine, then inspect local assignments, whole-function spill choices and the complete native gate |
| Real phase reuse preserves a copy but rotates surrounding registers | Trace [local hard conflicts and global preferences](matching-knowledge.md#compiler-evidence) | Check deaths and local assignments before global conflicts and preference reservations. A new global can lose the local hard conflict that displaced an input's argument preference. Add no consumer or lifetime; require the complete native gate |
| Three local quantities allocate in an unexpected order | Trace [local allocation ordering](matching-knowledge.md#compiler-evidence) | Reconstruct `qty_order`, tied references, intervals and hard-register suggestions; recheck after scopes change weights or scheduling |
| CSE forwards a seeded word that the original reloads | Trace [actual copy phases](matching-knowledge.md#compiler-evidence) | Prove the words are disjoint, compare existing read/store pairs, and verify both CSE passes and every original access before the native gate |
| Private scratch reload appears to need `volatile` after an aggregate copy | Trace the [aggregate copy RTL](matching-knowledge.md#compiler-evidence) | Check the backend's size/alignment path, both CSE passes and late copy split; require the ordinary reload and complete native result |
| A real operation scope or independent store order exchanges local registers | Trace [tied-quantity priorities and half-instruction intervals](matching-knowledge.md#compiler-evidence) | Trace SET counts through combine; calculate merged weighted references, power-of-two thresholds, scheduled births and deaths, and the priority margin. Keep real input and result phases distinct; reorder only proven independent stores. Inspect both CSE passes, allocation, and the lifetime and publication order in both schedules; use each pass's LUIDs before the complete native gate |
| A scope adds a copy at a narrow capture | Trace [word-producer deaths and scope edges](matching-knowledge.md#compiler-evidence) | Locate later consumers and inspect `REG_DEAD` and coalescing after scheduling. Keep real operations in the phase; verify both boundary receivers, register roles and the complete native image, including load delays |
| A sign-normalization cast creates a shift temporary that displaces a pointer | Trace [normalization phases and scope dependency costs](matching-knowledge.md#compiler-evidence) | Prove the target's conversion and shift semantics; express only the existing consumed phases. Check CSE/combine survival, load latency at the scope's first instruction, reloads and the complete native gate |
| Near-exact constant placement or a register exchange across separate value phases | Trace [compiler dependencies and interval conflicts](matching-knowledge.md#compiler-evidence) | Identify the real instruction receiving a scope dependency and the phase creating each hard-register conflict before another probe |
| Original shift/add or shift/subtract chain keeps one accumulator, or an absolute-value guard negates its copy in place | Trace [expression expansion and real value phases](matching-knowledge.md#compiler-evidence) | Prove the arithmetic domain. Trace actual accumulator definitions or conditional negation through CSE, local/global classes, preference propagation and scheduling, then require the complete native gate |
| Output initialization hoists across local arithmetic or a guarded return gains a copy | Trace [operation scopes and path liveness](matching-knowledge.md#compiler-evidence) | Scope only real producer work through its consumed predicate; verify initializer dependencies and result conflicts on each path before native comparison |
| Join-spanning boolean result or extra return copy | Compare expression return with [constant-return arms](matching-knowledge.md#boolean-return-expansion) | Predict the RTL change; preserve arithmetic, accesses and ABI; require the complete live gate |
| Overlay step dispatcher: scratchpad work-byte slot, scale by four, indirect call through a local handler table | `EMI_DEFINE_DISPATCHER` or `_THEN_RESET` in `include/bof3/emi/dispatchers.h`, with the matching `EMI_DISPATCH_WORK_BYTE_n` and shape variant | Emits the recorded 17-instruction, 68-byte body; keep the metadata block above the invocation. That body loads the record pointer before it creates the frame, so a hand-written variant keeps that order. Covers the bare body only: a dispatcher that keeps working after the call (second cell load, then a gate on the work bytes) is hand-written with one pointer local per original reload point and shares only the table extern. A hand-written bare body also reaches exact but duplicates the shared definition. Index width is part of the condition: the variants cover an unsigned byte slot only, and `n` is the record byte the original's `lbu` reads, not its table base, so a dispatcher that reads a wide slot of the same cell is not a macro gap. Cell identity is the same kind of condition: the macro hardcodes the scratchpad work slot, so a member whose record pointer comes from an in-image word is no macro gap either; declare both cells and call `TABLE[CELL[1]]()`, which reached the identical body. Write it by hand as `TABLE[SPAD_PTR_SLOT(u16, 0x44u)[slot]]()` with this function's own slot, which reaches exact on the first gate; do not add a variant or force the byte form. That typed base makes the subscript an element index, so a halfword at record byte 0x2C is `[0x16]`; spelling the byte offset loads the wrong halfword. A dispatcher whose whole handler table is two function addresses seeded into its own frame is neither shape: declare a local array of function pointers initialized with those addresses and index it with the work byte, because the macro indexes an extern table in place, never stores addresses into the frame and takes no index argument. How many of those words are seeded is not part of that condition: a two-word frame table with only word 0 seeded, called through the state byte with no index bound, matched on the first gate with the same local array and indexed call. An extern table copied into the frame is that same exclusion with an aggregate assignment in place of the initializer: `handlers = D_table; handlers.handlers[slot]();` emits the copied words and reads the call target back from them. An indexed body whose index arrives in the first argument register (`andi a0,0xFF` then `sll 2`, table loaded from its own immediate) is no macro gap either: declare the extern at the word the scaled index adds to and call `TABLE[index]()` in one statement Bind each dispatcher to the table base its own `%lo` names. Two same-target dispatchers that differ at that word need distinct table bindings and are shape siblings, not byte-identical clones. |
| Shared gate byte selects one of two constants into one shared status halfword | `WORLD_DEFINE_GATE_STATUS(function_name, gate_byte, status_halfword, when_nonzero, when_zero)` in `include/world/gate_status.h` | Matched on the first gate, and the macro already backs exact users in four overlays. The macro carries the branch and both arm constants, so a hand-written `if`/`else` duplicates the shared definition and re-opens an arm order that this shape is sensitive to. Declare the gate cell at the width and signedness its own load shows; per function only the two constants and the cell identities vary. The macro tests against zero. An original that compares the byte to a nonzero literal needs a plain `if`/`else` with that literal. This form matched with the else constant in the branch delay slot. Two blocks that advance one field under separate gate conditions select no constant, so keep them ordinary `if`s rather than this macro. A gate that only decides whether a constant lands in a different cell, while the shared halfword takes one constant on both paths, selects no arm either: write the plain `if` and then the unconditional store. When only two gate values write constants to another cell, preserve the no-store path for all other values. A case-per-value `switch` with `default: break;` matched that shape; a plain `if`/`else` would change behavior. The mirror order is a shape too: when the original's `sh` to the shared halfword precedes the guard's branch while the arm writes several byte cells, write that store first and put every arm store inside one `if`, which matched on the first gate with the arm's first constant filling the branch delay slot. Read that order off the `sh` position, not the arm spelling. A non-zero arm that stores a second cell after the status halfword is no macro shape either, since the macro emits one store per arm; write that pair as the plain `if`/`else` with the extra store following the status store, which matched on the first gate |
| Shared status byte takes the constant 2 and a `u16` counter moves by 0x14 | `COUNTER_ADVANCE(function_name, counter, flag)` or `COUNTER_RETREAT(...)` in `include/game/counter_step.h` | Emits the recorded body: read the counter into a local, store 2 into the status byte, then store counter ± 0x14, so the read stays ahead of both stores. Exact users in several overlays share it, and a hand-written body duplicates the shared definition. Eligibility is that trio: mode 2, movement 0x14, `u16` counter extern; a body with another amount, another mode constant or extra cells is no macro gap, and an ineligible amount still owes this recorded order, since writing it in place as `counter = (u16)(counter - K)` around the status store measured 7 of 11 instructions and four bytes over the original while the load-first order matched, so take the order and not the macro |
| Work-byte-`0xB` handler with three paths. Step 1 polls readiness, requests the resource in work byte 8 and stores step 2. Other nonzero steps update and finish when the state byte is zero. Step 0 finishes when ready. Finishing clears work byte `0x2B`, then published-record byte `0x12B` | `PLCHAR_DEFINE_WORK_BYTE_B_STEP(function_name, work_cell, record_cell, state_byte, ready, request, update)` in `include/bof3/plchar/work_byte_b_step.h` | Pass pointer-cell symbols, not loaded pointer locals. The macro preserves the original reload sites and owns `u8` step and done locals. Keep function metadata above the invocation. Reuse it only when all paths and stores agree with the original. Different paths, cleared cells or extra stores require a hand-written body, not a new macro variant |
| Bytes identical to already-exact function | `bin/harness lift clone TARGET@0xADDRESS --from REPRESENTATIVE` | Dry-run emits self-contained raw `func_`/`D_` clone, no foreign header. Cross-target only: the adapter refuses a representative from the clone target's own binary (`clone target is the representative's own target`), so a byte-identical sibling inside one target is copied-body shape evidence, not a clone. A declared replica payload with its own target id, the representative's sha256 and its load address is that same binary, so the adapter refuses it too; the lift already covers the replica's span at that address, and a separate registry row for the replica is a parent decision. A twin the manifest already lists as a replica with no target directory has no source to clone either, so record that decline and transcribe the body from the sibling's shape instead of spending an adapter call. `--apply` wires and retains only on `lift gate` PASS, otherwise restores. Still needs own exact gate/required review; `--apply` runs that gate itself, so a lane reporting one gate batch takes the dry-run body and performs the same splat/map/manifest wiring by hand. That hand copy is also the route when this target already binds an evidenced name for a symbol the body touches, such as a shared scratchpad cell every sibling names: `--apply` writes the dry-run's raw spelling, while the copy keeps the evidenced name at the same bytes. The same choice is owed for the body itself: `--name` carries the evidenced name into the emitted body and, on `--apply`, into the splat row, the map row and the filename, so a body whose own evidence names it needs no rename afterwards, and the hand copy is owed only when the body carries more than the label — this target's own `@behavior` reference facts, or a symbol it binds by an evidenced name, since `--apply` writes referenced symbols in their raw spelling. An empty `rewrites` list does not validate donor comments. Replace donor-specific
claims with this span's evidence, since byte gates do not check prose. A clone rewrite can misclassify a called function as `D_<ADDRESS>`. Check this target's map and generated `undefined_funcs_auto.txt`; reject a data rewrite for a proven callee and transcribe the body with the function binding. Check the composed `config/targets/shared/symbols.txt` too. A dry run replaced an established shared name with `D_<ADDRESS>` even though neither the target map nor `undefined_funcs_auto.txt` named that address. Transcribing the body with the shared name retained the exact match without taking `--apply`. Declined target-local include/unresolved external is recorded, never bypassed. The clone adapter has refused a proven-equal donor using `SPAD_PTR_SLOT` with `unresolved external 'SPAD_PTR_SLOT': not declared, not in the target map`. When the same limitation is already established for the donor, skip the adapter call, record the known limitation and transcribe the byte-proven body. Do not report an unissued call as a new refusal. A macro-defined representative can fail with `representative source requires one unambiguous function definition for 0xADDRESS` even when its bytes match. Record the refusal and invoke the same shared macro with this span's own bindings instead. Prove identity yourself with one `cmp` of the two payload spans at their own `runtime address - load address` offsets, which is the evidence the dry run would have reported. For a complete span containing no direct `j` or `jal` word, masking
those immediates changes nothing, so a second scan with only that mask is redundant. A payload scan that
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
relocation, not proof that no twin exists. Masking a data immediate finds shape candidates, not byte-identity proof.
Compare each candidate's original words before claiming a twin; the masked literal
may distinguish otherwise matching bodies. A raw `func_<ADDRESS>` row carrying the same label at the same address in a sibling target is not that evidence: sibling targets holding one such label at one address held unrelated bytes there, so the twin is a span match, never a name match. Nor is a pointer word: a sibling that loads at the same base can carry this address only because it names its own same-offset function, so that word is a self-reference, not a copy of this body. A family whose members each encode their own table base, callee address, passed constant or carried bound (a loop count, a length) in the instruction words can never be byte-identical to one another, so clone is ineligible across it and the family's shared body is the shape lever. Narrow that set before scanning: when every call target and referenced symbol of the span resolves in the shared main executable, only those carried constants can differ, so a member that differs from its sibling in one bound alone declines clone and takes the sibling as shape donor. A bound is a data immediate, so that miss is conclusive and owes no masked re-scan. Read that lever as a transcription of statement order, guard nesting and local shapes, and re-read each member's own table base, callee address, passed constant and bound from its bytes: those are exactly the words that differ between members, so a donor constant copied across is the one defect the shape never shows. A duplicate group whose `promotion_blockers` read `["no exact member"]` declines for lack of a representative, not for lack of shape; the first member lifted to exact becomes the representative its byte-identical peers clone from. A twin that differs in how many calls it makes is not one of those peers at any byte count, so count call sites before comparing sizes. Scan every extracted payload under `out/binaries`: twins cross area directories and archive indices, so a search confined to the current family or area directory is incomplete evidence. The one hit at the span's own offset in its own
payload is that scan's self-match, so a one-hit scan is no twin however many words are
masked. A byte miss still leaves a shape donor: one more pass that compares every
word's opcode and register fields finds a member of the family whose immediates alone
differ, and a miss at that granularity is the evidence that no sibling seeds these
statements, so write them from this span's bytes and spend no variant on donor search Check whether another byte hit is a whole function or an interior block of a larger body. An identical interior block supplies shape evidence, not a whole-function clone representative. Hits in unlifted `asm` rows or an unlifted executable copy provide no authored representative either. Record the reason and use a proven exact sibling's shape where available. |

Search the applicable levers before exhaustion. Byte identity proves no shared ownership.

For a grouped representative, use `--from TARGET@0xADDRESS` to select one attached
function. Clone retains common declarations and includes and rejects target-local
`*_internal.h` headers. If one proven-equal donor is refused for that reason, try a
sibling with file-local externs before transcribing the body by hand. A dry run
checks clone eligibility without a native gate. Clone compares both complete
reviewed ranges using validated PS-X payload placement. Address rewriting changes only the selected source tag;
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
