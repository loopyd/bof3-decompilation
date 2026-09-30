# bof3-macros observations

Per-skill half of the central observation folder ([index](INDEX.md)). The skill carries
**directives**; this file carries the **measurements** behind them. Read it before designing a
mission for this skill, and after each mission add one audit row and converge the finding into a
directive in `.pi/skills/bof3-macros/` (or its agent instructions) - prose alone is not convergence.

## Measured performance

The 4,171-run metadata snapshot contains **zero explicit selections** of
`bof3-macros`. This is a selection-field result, not zero historical macro work;
generic workers and parent operations still require attribution. The 14 findings
below are historical code-generation cases, not 14 skill missions or accepted
macro transactions. Their source-level reconciliation remains pending.

The [editorial audit](bof3-docs.md#macro-and-docs-editorial-child-review) fully
reviews both rewrites of this ledger: five calls/241.177 summed lifecycle seconds,
not macro missions. It recovered a dropped [transaction-test timing baseline](bof3-test.md#recovered-transaction-test-timing):
the two macro cases reported 159.84 and 154.59 seconds. These measurements justify
profiling shared-owner/revalidation costs; they do not identify a bottleneck cause
or establish current transaction latency.

For an attributed discovery cohort, measure candidates inspected, ranked and
rejected by reason, then reviewed transactions accepted / attempted. For each
transaction record writer/reviewer calls and time, affected functions, PRE/POST
byte checks, rollback outcome and reuse sites. Separate useful source reduction
from compile success and byte identity. Compare extraction missions of similar
scope; code-generation experiments alone do not measure extraction productivity.

Cross-skill supporting evidence from the 2026-09-28 lift cohort is not a
`bof3-macros` mission: review found unused macros in a three-function change, a
separate cleanup removed the stale definitions in **108.141 seconds / seven
calls**, and follow-up review confirmed the three exact byte matches remained.
Use this as a candidate reference improvement: check changed macros for remaining
users before review dispatch, then measure review-fix loops on matched cohorts.
The [body-free receipts](../../tmp/observation-ingest/cycle6-k-window700-20260928/agent-run-metrics.json)
keep this observation separate from macro-skill runtime and accepted transactions.

## Address-domain lead recovered during an RE review

Parent `01a0c062` record **1242**, reviewer `0275f718`, flags pre-existing
`BATTLE_CURRENT_QUEUED_PTR_4B20` passing main-RAM address `0x801EB4E0` as a
scratchpad offset, alongside duplicate absolute-field accessors. This is supporting
RE evidence, not a macro-resolution mission or verified fix. The report proposed
re-auditing partial siblings and retaining exact siblings as regression witnesses.

The report's numeric expansion **`0xA11EB4E0` is wrong**. Its retained transcript
record **252** captures `SPAD_BASE = 0x1F800000u` and
`SPAD_ADDRESS(offset) = SPAD_BASE + (u32)offset`; adding `0x801EB4E0` gives
**`0x9F9EB4E0`**. The captured output was fully read, but ends before the remaining
29 header lines; the full `SPAD_PTR_SLOT` expansion chain and sibling impact remain
pending. The address-domain concern survives this arithmetic correction, while its
claimed effect on the sibling's 59.62% score is only a hypothesis. Source/hash pin:
`tmp/observation-ingest/older-battle03-field-lift-metrics.json`.

Proposed **reference/tooling** improvement: distinguish absolute addresses from
scratchpad offsets and calculate expansions before promoting macro diagnoses.
Measure confirmed misuse sites and native improvements only after full expansion
and scoped sibling checks; do not count this reported opportunity as resolved.

## Macro spelling versus bound-symbol representation

Parent **1281–1286**, `battle/03@0x801E2170`, reports **32/69, 260 bytes** with
literal table macros and **69/69, 276 bytes** after macros were rebound to externs
and target-local map symbols, with the C body unchanged. Review reproduced the
literal form's smaller frame and first difference at **+0x000C**, but did not
reproduce its normalized score. This is supporting RE evidence, not a macro
extraction mission; its **82-call/375.744-second** review is shared cost.

Proposed **reference/tooling** improvement: inspect expanded address representation
and relocation behavior before rewriting control flow. Measure representation-only
repairs and unique affected consumers; preserve the compiler/profile and distinguish
mechanism corroboration from full score reproduction. The case supports this tested
spelling, not a universal ban on literal macros or new semantic ownership of table
symbols still tagged `unknown`. Full raw child review remains pending. Pins:
`tmp/observation-ingest/older-battle03-symbol-lift-metrics.json`.

## Iteration audit log

| Iteration | Mission | Tools | Duration | Method verdict | Converged into |
| --- | --- | --- | --- | --- | --- |
| - | - | - | - | - | - |

Add one row per mission. A row that changes nothing is itself a finding.

## Converged directives

None recorded yet. Directives for this skill live in `.pi/skills/bof3-macros/SKILL.md` and its references;
record the pointer here when one is added.

## Bundled harness improvements

No new harness change is established by this audit. Candidate improvements need
the [measurement contract](INDEX.md#performance-measurement-contract): macro
ranking guidance should be evaluated by reviewed utility and rejection reasons;
transaction tooling by measured validation/rework cost with unchanged byte and
ownership gates. The legacy match percentages below cannot supply those baselines.

## Important observations

14 measured observations, semantically compacted from Pi history. Byte pairs below are produced/expected.

- emi/battle/battle/03@0x801D64C4: local u8 idx = (u8)arg0 with indexed extern arrays D_80145E90[]/D_801EB630[] (unk_128/unk_104), replacing PSX_PTR forms, improved matching from 47.62% to 100% in one edit; live byte-match and symbols checks passed without map, Splat or header edits.
- emi/etc/shop/00@0x801DAB90: replacing the D_8014598C macro with plain extern plus WEAK_SYMBOL_AT regressed matching from 226/339 (66.67%) to 182/339 (53.69%). The macro had the correct 0x78 frame and first mismatch +0x0004; the replacement produced 1340/1356 bytes and diverged at the prologue.
- D_1F800044[3] = 0x0E: the named-symbol seed matched 57/59, first mismatch +0x00b8; li v0,14 occupied the load-delay slot instead of the target branch-delay slot. Seven initializer, pointer-local and body-assignment variants retained the same result.
- BATTLE_CURRENT_ENEMY_PTR at 0x801EB4E8: the fixed-address macro matched 33/43 (76.74%), 172/160 bytes, first mismatch +0x0004. Address reuse through s0 added save/restore instructions instead of a fresh lui+lw for the second read; the recorded fix was the named symbol battleCurrentEnemyWorkState.
- BATTLE_CURRENT_ENEMY_PTR and flags at 0x801E62E8: the volatile macro-local seed matched 34/49 (69.39%), first mismatch +0x0000. Of six variants, struct-member access plus FIELD_REF at offset 0x96 and u16-pointer division-assignment improved the best result to 42/47 (89.36%), 188/188 bytes, first mismatch +0x0010; the seed materialized constants, added redundant andi and folded bases per access.
- Volatile slot macros at 0x801EC339 matched 64/129 (49.61%), 512/516 bytes, first mismatch +0x004c. They materialized the full constant base instead of symbol-relative lui/addu/lbu and added a move around the count store.
- activeRecordBytes[index].active/.kind: volatile struct-field access improved matching from 35/61 to 48/62 over byte-pointer casts. A named queue symbol, pick = rand() & 0x3F and index rebasing at 0x3C reached 60/60 by correcting register allocation and delay-slot filling.
- WORLD00_AREA030_SCRATCH_PTR: the macro seed with an s0-held cursor and u8-cast conditionals matched 40/52 (76.92%), first mismatch +0x0004; address common-subexpression elimination introduced a frame and s0 spill.
- D_80010000 matched 147/149; remaining assembler-expansion differences were an extra addiu v0,v0,0 from the zero low-part in la and addu at,at,v0 instead of addu at,v0,at for indexed lhu.
- D_801E5DFC[g_PanelTaskRoot->state]() followed by func_801E2F04(g_PanelTaskRoot), with NO_SIBLING_CALLS, matched 15/24 (62.50%), first mismatch +0x0000. Unlike exact sibling func_801E27BC, it hoisted pointer-cell address 0x80148648 into saved s0 and reused it for both loads.
- dispatchScratchMode matched 17/17 with the bare-constant scratchpad macro: constant-address spelling did not uniformly cause mismatches.
- Literal-address PSX_REF and BATTLE_CURRENT_ENEMY_PTR matched 32/69 (46.38%), 260/276 bytes, first mismatch +0x000c. GCC used lui/ori rather than symbol-relative or absolute forms and commoned both accesses to 0x801EB4E8 into s1.
- WORLD00_AREA016_SCRATCH_PTR (PSX_REF volatile pointer-cell view at 0x1F800044) matched 25/41 (60.98%), first mismatch +0x0010. It emitted lui/ori/lw instead of lui/lw with offset 68, reused the cell address and rescheduled the unk_40 read-modify-write.
- dispatchWorkByte1Handler: D_801FC980[D_1F800044[1]]() matched 17/17 (100%) on the first seed, 68/68 bytes, empty diff. Indexed symbolic lw expanded to lui at,0x8020; addu at,at,v0; lw v0,-13952(at).
