# Clean-C match loop

## Required loop

Before changing existing C, obtain live normal asm diff. Absent source uses
[bounded first-source route](#first-source);
seed still needs native gates and required independent review. Diagnose `first=`,
classify via [playbook](matching-knowledge.md#delay-slots-and-entry-copies),
make one structural fix, rerun normal diff. Revert percentage regressions immediately.
Full diff only for first/ambiguous diagnosis. Partial catalog is parent audit data,
not live diagnosis.

## First source

For genuinely absent C, parent admits one bounded seed. Pin original bytes/load,
reviewed boundary, caller/callee evidence, PRE absence, exact owned source/header/
map/layout paths and attempt limits. Create evidenced C89 with `@source`/`@behavior`,
manifest claim, required declarations/bindings and `c` boundary. Obtain first live
diff through [native execution](native-execution.md) before another C edit.
Missing-source error is not baseline. Seed grants no score, acceptance or budget
renewal. Unresolved ownership/ABI or failed native gate stops matching. Final byte
match and required independent review remain. Diagnosis command requires existing C.

Exact metadata: `@status exact`, `@match 100.00`, `@residual none`. Notes on separate
lines, never semicolon suffix in tag. Naming repair needs fresh exact proof;
never weaken parser or repair unrelated rows to admit selected function.

## Terminal ladder

Single home of rungs. Earliest applicable rung first; record inapplicability evidence
instead of spending attempts. At most three non-progressing attempts per rung,
within smaller mission/probe budget; no minimum quota. Carry full ordered ledger
and consumption through repair. First byte match ends ladder.

| Rung | Work | Gate |
| --- | --- | --- |
| 1. Types/declarations | Width, signedness, pointers, fields, prototypes | `bin/harness lift byte-match TARGET@0xADDRESS` clears type-shape mismatch |
| 2. Control flow | Branch direction, loop/return/switch; equal-valued arms use bounded playbook branch matrix before escalation | Branch/topology mismatch clears |
| 3. Expression/register order | Ordinary temporaries, hoists, statements; no artificial allocator/scheduler controls | Allocation/scheduling residual changes |
| 4. Compiler profile | Selector-specific parent/user authorization only; `bin/harness lift flag-search TARGET@0xADDRESS`; clean-C exact profiles only | Exit 0 with non-empty `exact_matches` |
| 5. Permuter | Opt-in, shape already right; one `bin/harness lift permute TARGET@0xADDRESS --time-limit 60 -j N` | Attempt completes, residual class recorded |
| 6. Unresolved | Best coherent clean-C candidate, documented allocation/scheduling; no forced registers/asm | `byte-match` remains non-exact; residual documented |

Unauthorized profile/permuter rungs remain untried, not exhausted.

Frame/size: types/calls, address-taken locals, aggregate copies, flow first.
Same-size relocation/load-order: symbol representation and evidenced pointer-cell
volatility. Entry copies: lifetime, clean-C ordering, profile/permuter diagnosis.
Lone delay slot: exact branch/jump operands and liveness, never clobber.

## Probe ceiling

At most ONE `lift sweep`, four variants total across lane/mission and all repair
children. No per-variant `asm-diff`/`byte-match` loop or local Python/shell compile/
compare harness. In-process sweep resolves comparison context/canonical compile
command once, then compiles/compares each variant. Measured warm ~3-4s versus legacy
~60s under fan-out; each variant still rewrites/relinks. `--subprocess` only for
deliberate legacy reproduction.

Allocation/scheduling residual: retain best coherent candidate and truthful
`@status partial`/`@match`/`@residual`, then stop. Never chase spelling coincidences.

## Eligible-condition levers

Classify before first edit. Use eligible lever before clean-C attempts. Record
ineligible/declined reasons; revisit only with new evidence.

| Condition | Lever | Evidence and authority |
| --- | --- | --- |
| No function-shaped bytes: missing prologue/`jr $ra`, pointer runs, printable blocks; or ASM-backlog triage | `bin/harness analysis data-candidates [TARGET...]` | Ranked byte evidence only. Promotion requires reviewed layout transaction, `source splat` and index refresh, not C lift |
| Fragment/jump-table arm/shared epilogue with head elsewhere; code inside leading bare `[offset, bin, name]` segment or `bin` subsegment | `bin/harness analysis carve TARGET@0xADDRESS START:END` | Isolated two-split row/segment regroup probe preserving `vram`/text order. Never applies. Lane reports evidence; parent owns reviewed Splat placement, no whole-image oracle |
| Few clean-C shapes explain `first=` | `bin/harness lift sweep SELECTOR SOURCE variants...` | One call, four variants maximum, in-process; cumulative ceiling above |
| Bytes identical to already-exact function | `bin/harness lift clone TARGET@0xADDRESS --from REPRESENTATIVE` | Dry-run emits self-contained raw `func_`/`D_` clone, no foreign header. `--apply` wires and retains only on `lift gate` PASS, otherwise restores. Still needs own exact gate/required review. Declined target-local include/unresolved external is recorded, never bypassed |

These four form safe-untried-candidate search before exhaustion. Byte identity
never establishes shared ownership.

## Review and stop

Non-exact review proposes 1–3 ranked untried experiments with expected instruction
effects. Caller owns retries/stopping. Retain coherent improvement with atomic
partial metadata; revert no-progress/semantic defects. Partial-to-exact review
identifies decisive experiment; parent records reusable playbook/lesson proposal.

Percentage alone never means success. No matching-hack macros or historical aid
exemptions. Third non-progressing attempt: restore best clean-C state, advance rung.
Exhaustion report names target, first difference, attempts, next untried/blocked
evidence. Acceptance requires final live byte-match exit 0 and required independent
review. Canonical lift lanes retain their explicit exact-PASS reviewer exception.
