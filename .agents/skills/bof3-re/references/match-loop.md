# Clean-C match loop

## Required loop

Before changing existing C, obtain a live normal asm diff. For absent C, follow
the [first-source route](#first-source). Diagnose `first=`, classify through the
[playbook](matching-knowledge.md#delay-slots-and-entry-copies), make one structural
fix and rerun normal diff. Revert percentage regressions immediately. Use full diff
only for first/ambiguous diagnosis; partial catalogs cannot replace live diagnosis.

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
| 1. Representation/access | Prove ABI, widths, qualifiers, pointer cells and original reloads; inspect callees before declaring allocation-only | Original calls, conversions and memory events accounted for |
| 2. Control flow | Branch direction, loop/return/switch; test branch-local calculations, the branch matrix and evidenced [explicit cursors](matching-knowledge.md#explicit-cursors-and-signed-bounds) | Branch/topology mismatch clears |
| 3. Value lifetimes | Reuse a consumed local for a later real result; diagnose [orphan stack slots](matching-knowledge.md#orphan-stack-slots); test block-local initialization, operation scopes and arithmetic grouping | Predicted instruction effect measured; semantics and complete diff checked |
| 4. Compiler evidence | At a stall, inspect matching compiler sources and before/after RTL; for displaced guard/product chains, trace [guard scheduling](matching-knowledge.md#guard-scheduling-and-value-reuse) | Source-backed mechanism predicts a new probe; diagnostics grant no match |
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
compare harness. In-process sweep resolves comparison context/canonical compile
command once, then compiles/compares each variant. Each variant rewrites/relinks.
Use `--subprocess` only for deliberate legacy reproduction.

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
| No function-shaped bytes: missing prologue/`jr $ra`, pointer runs, printable blocks; or ASM-backlog triage | `bin/harness analysis data-candidates [TARGET...]` | Ranked byte evidence only. Promotion requires reviewed layout transaction, `source splat` and index refresh, not C lift |
| Fragment/jump-table arm/shared epilogue with head elsewhere; code inside leading bare `[offset, bin, name]` segment or `bin` subsegment | `bin/harness analysis carve TARGET@0xADDRESS START:END` | Isolated two-split row/segment regroup probe preserving `vram`/text order. Never applies. Lane reports evidence; parent owns reviewed Splat placement, no whole-image oracle |
| Few clean-C shapes explain `first=` | `bin/harness lift sweep SELECTOR SOURCE variants...` | In-process comparison; retain each variant and outcome |
| Bytes identical to already-exact function | `bin/harness lift clone TARGET@0xADDRESS --from REPRESENTATIVE` | Dry-run emits self-contained raw `func_`/`D_` clone, no foreign header. `--apply` wires and retains only on `lift gate` PASS, otherwise restores. Still needs own exact gate/required review. Declined target-local include/unresolved external is recorded, never bypassed |

Search these four levers before exhaustion. Byte identity proves no shared ownership.

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
