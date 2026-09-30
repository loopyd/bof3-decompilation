# bof3-types observations

Per-skill half of the central observation folder ([index](INDEX.md)). The skill carries
**directives**; this file carries the **measurements** behind them. Read it before designing a
mission for this skill, and after each mission add one audit row and converge the finding into a
directive in `.pi/skills/bof3-types/` (or its agent instructions) - prose alone is not convergence.

## Measured performance

The 4,171-run metadata snapshot contains **zero explicit selections** of
`bof3-types`; generic workers and parent operations remain unattributed. This
does not establish absence of type work. The 26 historical representation cases
below are not 26 invocations or accepted type transactions, and their original
source records still need reconciliation.

Supporting [transaction-test timing evidence](bof3-test.md#recovered-transaction-test-timing)
records two type cases at 152.00 and 150.96 seconds in a failing historical suite.
Their 302.96-second sum is a profiling baseline, not type-skill mission cost or
proof of a timeout. Preserve fixture/evidence distinctions before proposing shared
validation changes; the complete optimization episode remains under review.

Measure opportunities inspected, representations proposed and independently
accepted transactions / attempts, separating layout, ownership and code-generation
rejections. Record calls/time to establish offsets and consumers, affected targets,
PRE/POST byte checks, review rework and restoration. Match improvement is a useful
experiment result, but type acceptance also requires evidence for layout and use.
Stratify local representation changes versus shared declarations before comparing
cost; an edit affecting many consumers has a different validation burden.

## Declaration occurrence repair

Older parent `01a0c062…`, records **413–431**, supplies supporting harness
maintenance evidence, not an explicit `bof3-types` invocation or accepted type
transaction. The enclosing [test episode](bof3-test.md#cmake-ownership-assertions-and-declaration-checks)
owns the shared metrics and source pin. Diagnosis/edit/first suite used six
calls/46.551 observed seconds; subsequent checks used three calls/143.330 seconds.

The failing resolver selected an unsupported `DIRENTRY` occurrence from
`extern struct DIRENTRY * firstfile(char *, struct DIRENTRY *);` alongside its
actual SDK definition. The parser diagnosed prototype tag scope, while the builder
emitted an aggregate tag occurrence. The repair suppressed aggregate occurrence
creation whenever all declarators had the `function` relationship. This moved the
scratchpad suite to **18 passes/2 failures**, now concerning unsupported `g0`;
**180 broader passes** supplied separate evidence. Neither result established
complete declaration resolution or all relevant C declaration semantics.

The guard lacked a completeness, definition or visible-binding check; its comment
claimed more than the observed failing prototype established. The operator also
reasoned that ordinary `firstfile` received an alias target, although the inspected
builder assigned that field only for typedefs. Distinguish the parser diagnostic,
occurrence identity and resolver selection before choosing a broader filter.
The edit returned **20 printed Pyright diagnostics plus six omitted**; a repeated
native delivery duplicated that block. The operator labeled them preexisting
without a before-edit comparison. Ruff success does not answer that attribution.
Retained output is fully reviewed; the six omitted diagnostics remain unavailable.

Proposed directive/reference improvement: require the smallest declaration witness,
its scope/binding evidence and the affected occurrence/resolution path before
promoting a classification rule. Proposed harness improvement: expose occurrence
provenance and rejection reasons so a later unsupported name is distinguishable
from recurrence of the original defect. Measure failure identities before/after,
resolved cases/attempted cases, diagnostic attribution and calls/time to a scoped
resolution. Acceptance must retain unsupported-case refusal and distinguish tested
semantics from uncovered declarations; a broad green suite alone is insufficient.
These are improvement proposals, not implemented rules or expanded test coverage.

### Bitfield witness and partial parser acceptance

Records **432–485** continue the declaration investigation in a shared
27-call/220.619-second test, naming and planning episode. The suspected
`Battle03SpritePrimitive` parsed with no diagnostic. A wider source-header scan
returned `hits 0`, but silently skipped read/parse exceptions, used substring
matching and did not recreate the supplemental input; it could not prove absence
of the defect. The useful discriminator was the actual resolver entity's origin,
ordinal and canonical declaration, not another search for a field token.

Diagnostic instrumentation initially read occurrence attributes from an entity,
introducing two Pyright attribute errors. Access through `representative` repaired
them **10.155 observed seconds** after that result. The resulting witness was
`supplemental:261`, `P_TAG`, with `unsigned addr: 24`, `unsigned len: 8` and
`u_char r0,g0,b0,code`. The diagnostic edit reached that witness in **17.562
observed seconds**. The earlier header hypothesis was disproved; this was a
supplemental unsupported declaration, not evidence that the named header's fields
leaked into top-level scope.

The repair accepted an optional `: ...` suffix in the shared declarator regex.
It moved the same selected two-file run from **47 passes/3 failures in 6.62 seconds**
to **49 passes/1 failure in 6.68 seconds**; the remaining failure was the permute
lookup bound. The later narration incorrectly called all 49 passes scratchpad
passes. From parser-edit result to suite result was **11.752 observed seconds**.
This demonstrates recovery of the two payload cases, not full bitfield support:
the regex did not restrict the suffix to field context or validate its width, and
the structured field view lacked bit-width representation even though canonical
text retained it. No new semantic coverage was demonstrated. The temporary error
message was actually reverted at record 484, **70.497 observed seconds** after
its first edit result; proposed reversion alone would not establish restoration.

Proposed type references and tooling: preserve declaration provenance in bounded
diagnostics, distinguish entity from occurrence APIs, and explicitly classify
lossy structured representations. Measure diagnostic edits/rework, time to a
reproducing witness and resolved cases/selected cases; require evidence for each
newly accepted declaration form while retaining unsupported-form refusals. Pins
and joins: `tmp/observation-ingest/older-baseline-bitfield-metrics.json`.

## Editorial provenance and recovered witness

The [editorial audit](bof3-docs.md#rizin-and-type-editorial-child-review) covers
both rewrites of this ledger: six calls/246.454 summed lifecycle seconds. The
first worker received the full 120-bullet ledger; its exported transcript was
clipped, and the missing tail is now recovered from its retained native session.
That artifact defect does not prove the worker read an incomplete document.

The second worker removed a passage mentioning the **sibling** `func_801D6668`
as a duplicate of that function's own result. Original lifter
`11ddeb3f-7393-475e-b79a-c96327d2b19d`, final record 276, instead names
`emi/etc/shop/00@0x801DE3C8`: the writer reports a first-seed **17/17 instructions,
68/68 bytes**, using `PSX_REF(u8,(u32)&D_80148652) += 1` within the sibling gate
idiom. It reports five changed paths, a retried gate/build, a re-applied manifest
claim and pending independent review. The complete earlier transcript and those
claims still require review; no accepted-result rate is inferred from the report.

This is a second witness for the shared-address pattern, not a second copy of
the sibling's experiment. Keep the general lesson compact while retaining both
target/run identities. Measure deduplication by preserved distinct witnesses and
obligations, not bullet reduction; source references to siblings are not mission
identities. Reproduction/source pins: `tmp/observation-ingest/editorial-native-results.json`.

## Append-only field evidence in a battle lift

Parent `01a0c062` records **1145–1152** provide supporting type evidence from
an RE writer/reviewer, not a separately invoked types mission. The new
`BattleWork` fields are two padding bytes at `0x3C` and `u16 unk_3E`; the
reviewer decoded `lhu 62(t0)` from original payload bytes. It reported one field
user, target-local declaration/uses, pointer-only use, and no `sizeof` or array
element use across `src/` and `include/`. Append-only fields preserve the prior
member offsets; that is narrower than proving unchanged struct size or all
target behavior. Two edited functions retained their individual diff results;
no whole-target build was run. The report also identified an unchanged guard
ending before roughly eleven extern declarations, retained as separate debt.

The shared RE review cost **30 calls/222.975 observed seconds** and covered
many other obligations; no exclusive type-validation time is available. Complete
report reviewed, raw child validation pending; source pins:
`tmp/observation-ingest/older-battle-second-partial-metrics.json`. Proposed
**reference/tooling** improvement: distinguish access-width/alignment proof,
existing-offset preservation, changed aggregate size and pointer/array/sizeof
consumers. Measure how many representation claims have each necessary witness;
do not count a successful local compile as blanket layout neutrality.

The next field extension, `BattleRange` at parent **1191–1200**, reused the
same `u8[2]`/`u16` append pattern. Review reported seven named source users, all
owned by battle15, and original `lw 0x34`, `lw 0x38`, `lhu 0x3E` evidence.
This adds a second representation witness, not a second standalone types mission;
existing-offset preservation remains narrower than unchanged aggregate size.
Pins: `tmp/observation-ingest/older-battle-frontier-repair-metrics.json`.

## Byte-global qualifier experiment and its limits

Parent `01a0c062`, records **1223–1228**, supplies supporting RE evidence for
`D_801462E6`, a target-local one-byte extern. In the canonical compiler/flags and
tested comparison, `volatile u8` generated an extra `andi` plus swapped compare
registers; plain `u8` matched the original opening. Together with a separate
expression-shape change, the function reached **31/31 instructions, 124 bytes**.
Review reproduced the qualifier probe and reported two forced exact rebuilds;
the only other known consumer, a store, stayed **14/14, 56 bytes, exact**. This
qualifier-only edit preserves object size/alignment, unlike a claim about all
observable memory-access semantics. Shared RE review cost was **60 calls/264.528
seconds**, not exclusive type-validation time. Raw child review remains pending;
pins: `tmp/observation-ingest/older-battle-batch-completion-metrics.json`.

The reviewer proposed a universal rule that a byte global loaded with bare
`lbu`/`sb` must be non-volatile. The evidence supports the tested code-generation
choice, not that general rule or proof the original declaration had this qualifier.
The writer itself measured a non-volatile local view while retaining the volatile
declaration: it also produced 31 instructions and the matching opening, although
full final exactness for that alternative was not established. Access instructions
alone do not resolve asynchronous/device semantics or all legal source forms.

Proposed **directives/references**: bind qualifier corrections to target identity,
the exact compiler/body probe, relevant memory semantics and the affected-use
inventory. Keep byte width, ABI neutrality, code-generation necessity and semantic
necessity separate. Proposed **tooling** support: retain both probe sources/flags
and all consumer results. Measure qualified claims with alternative-source and
consumer evidence; do not promote a local successful experiment into a blanket
qualifier-removal rule.

## Existing field representation recovered a matching address form

Parent **1237–1242**, `battle/03@0x801E71EC`, supplies a concrete representation
transition: byte-offset casts and a temporary absolute symbol scored **25/47**;
existing `D_80145E90[idx].unk_80` / `D_801EB630[idx-3].unk_82` fields scored
**47/47, 188 bytes, MATCH**. Review checked effective addresses `0x80145F10`
and `0x801EB6B2`, observed strides `0x140`/`0x118`, unsigned halfword loads, existing
declarations and sibling use. Header/support additions were reverted; exact field
reuse did not require inventing a new shared type or retaining the synthetic symbol.
The shared RE review cost **68 calls/349.388 seconds**; exclusive typing cost is
unmeasured. Full raw child review remains pending. Pins:
`tmp/observation-ingest/older-battle03-field-lift-metrics.json`.

The source also discarded the existing pointer's pointee `volatile` qualifier,
which the writer called stylistic because an exact sibling did the same. Matching
bytes and precedent do not independently resolve qualifier semantics. Proposed
**reference/tooling** improvement: surface existing field/base/stride witnesses
before creating synthetic symbols, preserve temporary-declaration rollback evidence,
and track qualifier warnings separately from layout/byte proofs. Measure successful
field reuse and avoided retained declarations, not just new type count.

## Padding split with measured layout preservation

Parent **1251–1256**, `battle/03@0x801DD29C`, replaced `pad_a5[0x74]` with
`pad_a5[0x73]` plus byte field `unk_118`, retaining `unk_119`. Independent review
reported **four compile-time assertions**: offsets `0xA4`, `0x118`, `0x119` and
`sizeof=0x140`; it found no other `pad_a5` use. Native evidence supported stores
of `3` and `1` into the two adjacent bytes. Two exact neighbors and the prior lift
were rechecked, and a forced 219-source target build passed. These are distinct
layout, consumer-compilation and selected byte-identity proofs; they do not establish
whole-target byte identity. The **97-call/482.810-second** review is shared with
RE, not exclusive typing cost; full raw child review remains pending.

Proposed **reference/tooling** improvement: pair padding splits with explicit old/new
size and offset assertions, use-site coverage and selected native checks. Measure
asserted layout properties and checked consumers against their known denominators,
rather than treating “append-only” or a successful build as complete representation
proof. Source pins: `tmp/observation-ingest/older-battle03-marker-lift-metrics.json`.

## Store evidence corrected a const declaration

Parent **1263–1270**, `battle/03@0x801DCFD0`, proves writes through `D_80144968[]`:
`0xA0` bytes in 16-byte chunks plus a four-byte tail fill one `0xA4` record.
Removing `const` therefore corrects the declaration's mutability claim. Both other
reported consumers stayed exact: **48/48, 192 bytes** and **28/28, 112 bytes**.
The target-local header was reportedly included by **217/220 target sources** and
no other target; a 219-object build passed. Header inclusion is potential scope,
not a count of actual symbol consumers or full byte comparisons. This is stronger
qualifier evidence than sibling convention alone.

A new byte extern `D_80146254` was backed by two payload `lbu` sites and an existing
shared-map row already in the target's configured map list. No duplicate target-map
row was needed. Proposed **reference/tooling** improvement: capture store witnesses,
actual consumers, header fan-out and map ownership separately for qualifier changes.
Measure proven mutability repairs / qualifier edits and unchanged byte results /
identified consumers. The **110-call/521.279-second** review is shared with RE;
exclusive type cost and full raw child review remain unmeasured/pending. Pins:
`tmp/observation-ingest/older-battle03-copy-lift-metrics.json`.

## Qualifier and operand order were separate causes

Parent **1299**, `battle/03@0x801DE43C`, supplies a four-variant review: a volatile
local slot view changes **73→74 instructions** by adding an `andi`; mask-test
operand reversal independently changes load order while retaining 73 instructions.
Combining them produces both effects. This refutes the writer's claim that the
qualifier caused both changes. A scratch header removing the declaration's qualifier
also left the only other reported consumer at **21 identical instructions and
identical relocation records**. No declaration change was applied.

The review calls removal a safe durable fix based on plain RAM and a C writer.
That is useful candidate evidence, but RAM location, known writers and byte identity
alone do not prove absence of asynchronous mutation or all volatile semantics. Keep
the missing semantic evidence explicit before a real transaction. Proposed
**reference/tooling** improvement: factor qualifier, expression-order and representation
changes independently; record native and semantic evidence separately. Measure
confounded diagnoses corrected and consumer equivalence / identified consumers.
The **89-call/347.152-second** review is shared RE cost, not exclusive type time;
full raw child review remains pending. Pins:
`tmp/observation-ingest/older-battle03-completion-metrics.json`.

## Pointer-cell qualification and measured consumer scope

Parent **1315–1320**, `world00/area030/04@0x801DDEEC`, concerns
`extern u8* volatile D_1F800044`: the **pointer cell** is volatile, unlike a pointer
to volatile bytes. The local non-volatile cell accessor changed **30/37, 148 bytes**
to exact **36/36, 144 bytes**, independently reproduced. An address-equivalent
non-volatile declaration also reproduced exactness. Review inventoried **36 target
readers**, spot-checked two existing exact siblings, and changed the qualifier for
only one sibling in a scratch experiment: **21/21, 84 bytes**, still exact. That is
one measured sibling's sensitivity result, not all 36 consumers verified under a
new declaration. One read per block is code-generation evidence, not by itself proof
of all asynchronous mutation semantics. No shared qualifier edit was performed.

The added `extern u16 D_80145AA4` matched the shared declaration's type; the shared
header was absent from the TU's measured include closure, and shared map/binding
ownership already existed. This supported a local declaration without importing the
entire core header or duplicating a binding. Proposed **reference/tooling** improvement:
record which object is qualified, typed declaration compatibility, include closure,
existing ownership and changed-qualifier consumer checks separately. Measure avoided
unnecessary imports/bindings and measured consumer sensitivity with denominators.
The **81-call/466.546-second** review is shared RE cost; exclusive type cost is
unmeasured and raw child review pending. Pins:
`tmp/observation-ingest/older-area030-cell-lift-metrics.json`.

## Iteration audit log

| Iteration | Mission | Tools | Duration | Method verdict | Converged into |
| --- | --- | --- | --- | --- | --- |
| 2026-09-20 parent, records 413–431 | Supporting declaration resolver repair | 9 shared calls | 46.551 + 143.330 observed s | Scratchpad 18/20 passed; broader 180 passed; general rule and diagnostic baseline unproved | Witness, occurrence provenance and failure-identity proposals above |
| 2026-09-20 parent, records 470–480 | Supporting bitfield diagnostic and parser repair | 5 shared calls | 40.008 observed s | Two payload failures resolved in 50-case subset; bitfield semantic completeness unproved | Provenance diagnostics and representation-loss proposals above |

Add one row per mission. A row that changes nothing is itself a finding.

## Converged directives

None recorded yet. Directives for this skill live in `.pi/skills/bof3-types/SKILL.md` and its references;
record the pointer here when one is added.

## Representation and ownership gates (absorbed from the legacy `LESSONS.md`)

Cherry-picked from `LESSONS.md` Level 3, minus what `docs/agents/lessons.md` already owns
(`PSX_PTR`/`PSX_REF` reach, fixed-RAM pointer ownership, argument-register pins as an allocator
residual).

- **C alignment silently relocates a misdeclared field** — a `u32` at an unaligned offset lands on the
  next boundary and shifts every later field. Model unaligned words as `u8[N]`, and audit offsets
  against the real C layout rather than against comments.
- **Model stack locals with official PsyQ SDK types** (`MATRIX`) when the evidence fits; ad-hoc `u32`
  arrays round to 8-byte slots and inflate the frame. Adjacent source blobs copied into adjacent
  stack ranges are **separate** locals — one aggregate duplicates storage and inflates the frame.
- **`volatile` needs asynchronous or hardware-mutation evidence.** A `volatile` slot cell forces
  `lui + ori + lw`; use `PSX_REF(Entity * volatile, SPAD_ADDRESS(off))` when a per-evaluation reload
  is wanted, and a named typed global when a match depends on it (gate: `func_800B2218` matched only
  once `D_80148648` became a named `PanelTask*`).
- **Recover stable field offsets into a target-local struct before permuting.** Addresses, masks and
  encoded values stay hex; human quantities (32-pixel step, 320-pixel clamp) stay decimal.

## Bundled harness improvements

No new harness change is established by this audit. Under the
[measurement contract](INDEX.md#performance-measurement-contract), evaluate
layout references by preventable offset/rework incidents, and consumer-discovery
tooling by time/calls with complete target coverage. The retained alignment and
padding cases motivate investigation, but do not yet establish incident rates or
expected savings. Preserve ownership and byte checks in any proposed comparison.

## Important observations

26 measured observations, semantically deduplicated from Pi history.

- At D_801EC330, the struct layout reproduces flag_09 at +0x09, words at +0x34/+0x38/+0x3C, and ptr_74 at +0x74; sll 4, subu, sll 3 gives the 0x78-byte record stride. The retained profile was -G4, without pins or assembly.
- emi/battle/battle/03@0x801DDE7C remained at 21/23 instructions (91.30%) and 92 bytes: both mismatches were addu at,at,t0 versus addu at,t0,at, an operand-order residual rather than a size difference.
- emi/etc/shop/00@0x801E2724 remained at 21/39 instructions (53.85%), 152 -> 156 bytes, after correcting the D_80148378 array representation and volatility; the first difference was omission of the opening base materialization.
- Copying the named three-entry Battle03DispatchTable from D_801D0F38, dispatching through battleWork[1], and guarding func_8014D4E0 with battleWork[0] & 1 produced 34/34 instructions on the first seed, without matching aids.
- In battle15@0x800AC7D0, a single struct-array representation improved the match from 60% to 100%; in battle15@0x800AD5C8, re-deriving the loop-bound address into a body pointer local improved 86.67% to 100%.
- In game/00, a plain extern struct GameWorkArea* g_game_work field read followed by a store and trailing call reproduced the scratchpad pointer load above the prologue on the first seed: 13/13 instructions and 52/52 bytes.
- For func_801F85A8, saving and restoring the four origin words at +0x64/+0x68/+0x6C/+0x70 with scalar s32 locals scored 71/104 instructions (68.27%); struct copies in both directions reached 104/104 and 416 -> 416 bytes. Field saves with a struct restore reached only 90/104.
- For the byte-counter gate-call shape of func_801D6668, PSX_REF(u8,(u32)&SYM) += 1 materialized one shared address for the load and store and produced 17/17 instructions, 68/68 bytes. The volatile-symbol increment seed had scored 12/19 (63.16%).
- A volatile record-struct read with a shared label local and shared tail store scored 20/42 instructions (47.62%), 168 -> 128 bytes: GCC emitted an extra andi 0xff after lbu and merged the branch arms. The seed was reverted.
- When extending a struct to +0x118, padding computed from the last named field instead of the next free byte shifted lbu from offset 280 to 281. The correct padding was 0x118 - 0x0C = 0x10C.
- Assigning g_game_work from recordTable[record_index] and then dispatching through D_801C7C70[recordTable[record_index].unk_05] scored 42/46 instructions (91.30%), 176 -> 184 bytes; at +0x50 the candidate recomputed the address instead of reusing it for lbu at +5.
- Initializing both scan sentinels in the declaration block scored 72/79 instructions (91.14%), with the first difference at +0x0C. Moving lowest = 0xFFFFu and selected = 0u between the two loops reached 79/79 and byte identity.
- Replacing three s16 angle scalars with one s16 angle[3] memory object improved 57/78 instructions (73.08%) to 72/76 (94.74%). The scalar seed used a 0x48-byte frame instead of 0x40 and kept angles in saved registers; the array forced per-call spills and reloads.
- A seed based on sibling func_801E4E10 scored 41/52 instructions (78.85%) with eight extra bytes. At +0x38, accesses to halfword 0x801462E8 used separate folded load/store addresses instead of a shared lui/addiu base and zero-offset lhu/sh.
- Reading the rand() & 0x3F seed at D_1F800008 through a non-volatile s32 view let the scheduler sink its store before func_801782FC and hoist the volatile D_1F800004 argument read. The match improved from 152/160 to 154/160 instructions at an unchanged 640 bytes.
- For func_801F4158, volatile struct slots with plain reads scored 7/30 instructions, 128 -> 100 bytes; volatile input views with s16 temporaries improved to 8/30, 128 -> 108 bytes. Using int temporaries produced 11/43 instructions and 172 bytes and was rejected as a structural regression.
- A named-symbol seed with plain u8* locals scored 37/53 instructions (69.81%); a struct form with volatile locals reached 96.30%. Struct with plain locals scored 41/53, while named-symbol with volatile locals scored 36/55, so neither change alone explained the improvement.
- For func_801F2E3C, a local pointer to a struct member scored 41/55 instructions (74.55%), 208 -> 220 bytes, adding a saved-register entry copy and increasing the frame from 72 to 80 bytes. Direct member indexing kept +0x18 in the address mode and reached 52/52, 208 -> 208 bytes.
- A fixed-address seed using BATTLE_CURRENT_ENEMY_PTR and table macros scored 32/69 instructions (46.38%), 276 -> 260 bytes, with the first difference at +0x0C; constant-address lui/ori materialization and common-subexpression reuse in s1 led to reverting that representation.
- For the sequence calling func_801E0F78, incrementing D_80148651, and clearing D_801E6260, direct volatile globals scored 9/15 instructions (60%), 56 -> 60 bytes: the load and store used separate folded addresses. A later volatile pointer-local form reached 12/15 (80%) but retained an epilogue scheduling residual at +0x28.
- In battle15@0x800A99AC, splitting a struct-array padding region to add a four-byte field, with padding measured from the next free offset, preserved folded %lo addressing and reached 54/54 instructions on the first seed.
- For an X - K struct-array index in the func_801E97A8 style, inlining the subtraction reached 103/103 instructions versus 101/103 (98.06%) with a temporary: the inline form loaded and subtracted in the index register instead of emitting a separate addiu into another register.
- At 0x800AE014, all tested aggregate-initializer forms reached 22/22 instructions, including struct-typed table and index-local variants. The retained form was the plain initializer with BATTLE_SCRATCHPAD_PTR and no attribute.
- For D_800E4050[i].id, the declared-symbol baseline scored 19/20 instructions because MASPSX expanded the address with addu at,at,index. A numeric 32-bit displacement fixed the operand order in the C body, producing the required addu at,index,at.
- exe/slus_004_22@0x8014B3C4 (displayBootExceptionDump) scored 182/188 instructions (96.81%), 752 -> 744 bytes. At +0x70, the candidate placed sw t2,208(sp) in the call delay slot rather than preserving the original intervening nop and store before the next load.
- A cursor-store candidate scored 17/19 instructions at equal size because li v0,1 followed the folded cursor load instead of occupying the bnez delay slot. Using the fixed-address view for the first constant store, while retaining named-symbol access for the other cells, reached 19/19 and a successful live byte-match.
