# Clean-C match loop

## Required loop

Before changing existing C, obtain a live normal asm diff. For absent C, follow
the [first-source route](#first-source). Diagnose `first=`, classify through the
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
manifest claim, declarations/bindings and `c` boundary. Obtain a live
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
| 1. Representation/access | Prove ABI, widths, qualifiers, pointer cells and original reloads; inspect callees before declaring allocation-only. For an unlifted callee the declared parameter form is representation too: by-value pair/aggregate parameters expand at this call site and can justify a preheader register or a size deficit the scalar spelling never produces | Original calls, conversions and memory events accounted for |
| 2. Control flow | Branch direction, loop/return/switch; test [boolean return expansion](matching-knowledge.md#boolean-return-expansion), branch-local calculations, the branch matrix and evidenced [explicit cursors](matching-knowledge.md#explicit-cursors-and-signed-bounds) | Branch/topology mismatch clears |
| 3. Value lifetimes | Reuse a consumed local for a later real result; diagnose [orphan stack slots](matching-knowledge.md#orphan-stack-slots); test block-local initialization, [operation scopes](matching-knowledge.md#operation-scopes-and-loop-note-dependencies) and arithmetic grouping | Predicted instruction effect measured; semantics and complete diff checked |
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
renders the aligned rows. In-process sweep resolves comparison context/canonical compile
command once, then compiles/compares each variant. Each variant rewrites/relinks.
Use `--subprocess` only for deliberate legacy reproduction. Give every variant the
measured `@source`/`@behavior` metadata: one without them is refused before
comparing and the sweeper prints `-`, so record that as a non-measurement and fix
the variant rather than reading it as a neutral result.

Before each sweep, state the semantic precondition and predicted instruction
change. Compare the full result, not score alone. Restore neutral/regressive probes.
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
| No function-shaped bytes: neither a prologue nor a `jr $ra` return path, pointer runs, printable blocks; or ASM-backlog triage | `bin/harness analysis data-candidates [TARGET...]` | Ranked byte evidence only. Promotion requires reviewed layout transaction, `source splat` and index refresh, not C lift |
| Fragment/jump-table arm/shared epilogue with head elsewhere; code inside leading bare `[offset, bin, name]` segment or `bin` subsegment | `bin/harness analysis carve TARGET@0xADDRESS START:END` | Isolated two-split row/segment regroup probe preserving `vram`/text order. Never applies. Lane reports evidence; parent owns reviewed Splat placement, no whole-image oracle |
| Few clean-C shapes explain `first=` | `bin/harness lift sweep SELECTOR SOURCE variants...` | In-process comparison; retain each variant and outcome |
| Join-spanning boolean result or extra return copy | Compare expression return with [constant-return arms](matching-knowledge.md#boolean-return-expansion) | Predict the RTL change; preserve arithmetic, accesses and ABI; require the complete live gate |
| Overlay step dispatcher: scratchpad work-byte slot, scale by four, indirect call through a local handler table | `EMI_DEFINE_DISPATCHER` or `_THEN_RESET` in `include/bof3/emi/dispatchers.h`, with the matching `EMI_DISPATCH_WORK_BYTE_n` and shape variant | Emits the recorded 17-instruction, 68-byte body; keep the metadata block above the invocation. That body loads the record pointer before it creates the frame, so a hand-written variant keeps that order. Covers the bare body only: a dispatcher that keeps working after the call (second cell load, then a gate on the work bytes) is hand-written with one pointer local per original reload point and shares only the table extern. A hand-written bare body also reaches exact but duplicates the shared definition. Index width is part of the condition: the variants cover an unsigned byte slot only, and `n` is the record byte the original's `lbu` reads, not its table base, so a dispatcher that reads a wide slot of the same cell is not a macro gap. Write it by hand as `TABLE[SPAD_PTR_SLOT(u16, 0x44u)[slot]]()` with this function's own slot, which reaches exact on the first gate; do not add a variant or force the byte form. A dispatcher whose whole handler table is two function addresses seeded into its own frame is neither shape: declare a local array of function pointers initialized with those addresses and index it with the work byte, because the macro indexes an extern table in place, never stores addresses into the frame and takes no index argument. An indexed body whose index arrives in the first argument register (`andi a0,0xFF` then `sll 2`, table loaded from its own immediate) is no macro gap either: declare the extern at the word the scaled index adds to and call `TABLE[index]()` in one statement |
| Shared gate byte selects one of two constants into one shared status halfword | `WORLD_DEFINE_GATE_STATUS(function_name, gate_byte, status_halfword, when_nonzero, when_zero)` in `include/world/gate_status.h` | Matched on the first gate, and the macro already backs exact users in four overlays. The macro carries the branch and both arm constants, so a hand-written `if`/`else` duplicates the shared definition and re-opens an arm order that this shape is sensitive to. Declare the gate cell at the width and signedness its own load shows; per function only the two constants and the cell identities vary. Two blocks that advance one field under separate gate conditions select no constant, so keep them ordinary `if`s rather than this macro |
| Bytes identical to already-exact function | `bin/harness lift clone TARGET@0xADDRESS --from REPRESENTATIVE` | Dry-run emits self-contained raw `func_`/`D_` clone, no foreign header. Cross-target only: the adapter refuses a representative from the clone target's own binary (`clone target is the representative's own target`), so a byte-identical sibling inside one target is copied-body shape evidence, not a clone. `--apply` wires and retains only on `lift gate` PASS, otherwise restores. Still needs own exact gate/required review; `--apply` runs that gate itself, so a lane reporting one gate batch takes the dry-run body and performs the same splat/map/manifest wiring by hand. That hand copy is also the route when this target already binds an evidenced name for a symbol the body touches, such as a shared scratchpad cell every sibling names: `--apply` writes the dry-run's raw spelling, while the copy keeps the evidenced name at the same bytes. Declined target-local include/unresolved external is recorded, never bypassed. A representative whose body reaches a shared-header macro the clone target does not declare fails the dry run with `unresolved external 'SPAD_PTR_SLOT': not declared, not in the target map`, even when the two ranges are byte-identical; those bytes still lift as a shape copy of that sibling, so record the refusal and seed from it rather than retrying the adapter. Prove identity yourself with one `cmp` of the two payload spans at their own `runtime address - load address` offsets, which is the evidence the dry run would have reported. A family whose members each encode their own table base or callee address in the instruction words can never be byte-identical to one another, so clone is ineligible across it and the family's shared body is the shape lever |

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
