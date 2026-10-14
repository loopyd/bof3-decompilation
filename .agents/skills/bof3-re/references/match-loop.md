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
End at first byte match.

| Rung | Work | Gate |
| --- | --- | --- |
| 1. Types/declarations | Width, signedness, pointers, fields, prototypes | `bin/harness lift byte-match TARGET@0xADDRESS` clears type-shape mismatch |
| 2. Control flow | Branch direction, loop/return/switch; equal-valued arms use bounded playbook branch matrix before escalation | Branch/topology mismatch clears |
| 3. Expression/register order | Ordinary temporaries, hoists, statements; no artificial allocator/scheduler controls | Allocation/scheduling residual changes |
| 4. Compiler profile | Selector-specific parent/user authorization only; `bin/harness lift flag-search TARGET@0xADDRESS`; clean-C exact profiles only | Exit 0 with non-empty `exact_matches` |
| 5. Permuter | Opt-in, shape already right; `bin/harness lift permute TARGET@0xADDRESS -j N` | Attempt completes, residual class recorded |
| 6. Unresolved | Best coherent clean-C candidate, documented allocation/scheduling; no forced registers/asm | `byte-match` remains non-exact; residual documented |

Unauthorized profile/permuter rungs remain untried, not exhausted.

Frame/size: types/calls, address-taken locals, aggregate copies, flow first.
Same-size relocation/load-order: symbol representation and evidenced pointer-cell
volatility. Entry copies: lifetime, clean-C ordering, profile/permuter diagnosis.
Lone delay slot: exact branch/jump operands and liveness, never clobber.

## Probe method

Use `lift sweep` for eligible clean-C alternatives without a call or variant quota.
No per-variant `asm-diff`/`byte-match` loop or local Python/shell compile/
compare harness. In-process sweep resolves comparison context/canonical compile
command once, then compiles/compares each variant. Each variant rewrites/relinks.
Use `--subprocess` only for deliberate legacy reproduction.

Reviewed allocation/scheduling exhaustion: retain best coherent candidate and
truthful `@status partial`/`@match`/`@residual`, then stop. Never chase spelling
coincidences. A residual class alone does not prove exhaustion.

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

Non-exact review proposes 1–3 ranked untried experiments with expected instruction
effects. Caller owns retries/stopping. Retain coherent improvement with atomic
partial metadata; revert no-progress/semantic defects. Partial-to-exact review
identifies decisive experiment; parent records reusable playbook/lesson proposal.

Percentage alone proves no success. Do not use matching-hack macros or historical
aid exemptions. Report target, first difference, attempts and next untried/blocked
evidence on exhaustion. Require final live byte-match exit 0 and independent review,
subject to the canonical lane's explicit exact-PASS reviewer exception.
