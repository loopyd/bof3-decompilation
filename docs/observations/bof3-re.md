# bof3-re observations

Self-improvement ledger for reverse and review missions. Read this file FIRST when a partial
needs a new experiment. Entries are selector-free: symptom, shape, measured
before/after, residual class. After every mission add one row (symptom, shape,
delta) plus any turn-cost lesson. Never put this prose in lifted source.

## Current protocol iteration audit

| Iteration | Evidence and outcome | Cost | Convergence |
| --- | --- | --- | --- |
| 2026-09-27, Batch 025 child `049c112a` | Complete byte-2 dispatcher, first seed 17/17 and 68 bytes exact; near-identical sibling differs in table immediate, not clone-eligible | 46 completed tool calls; workflow 360.895 s; one gate/no retry; prefill twice and unused m2c/m2ctx | Reverse protocol and skill now classify lever eligibility before editing, use gap-driven context/decompiler reads, cap applicable rungs rather than require attempts, and preserve cumulative repair/sweep consumption; trial exact but slower (60 writer + 27 redundant reviewer calls, 586.638 s); no speedup claim |

Full receipt, baseline and transaction diff: `out/lift-batches/batch-025/` and
`out/lift-batches/protocol-efficiency/`. Independent protocol review identified a
missing explicit cumulative repair call ceiling; correction passed re-review.
Trial prefill/decompiler compliance improved (one/zero), but nested lever statuses
misrouted an exact result to review. Explicit JSON membership/outcome routing now
passes eleven disposable replays and independent re-review (`18b12e02`); native
truth and cumulative budgets remain separate checks.
This observation does not imply semantic acceptance from byte identity alone.

## Measured mission performance

### 2026-09-28 linked K700 lift cohort

The pinned [run metrics](../../tmp/observation-ingest/cycle6-k-window700-20260928/agent-run-metrics.json) join native worker and reviewer metadata by target-qualified selectors. Twelve target-lift worker runs covered **26 functions**, with **20 reported exacts** and **six escalations**, using **765 calls / 17,657.692 summed seconds**. Eleven reviewer runs recorded **23 verdicts** (20 pass, three needs-fix) using **84 calls / 1,099.068 summed seconds**. Two pass selectors do not match worker targets; strict reconciliation therefore confirms **18 exacts**, or **3.670 confirmed exacts per summed worker-hour**. Including summed review cost gives **3.455 confirmed exacts per combined worker-and-reviewer hour**. These are delegated cohort rates, not direct `bof3-re` or parent skill runtime.

Two one-hour worker timeouts consumed **7,200.288 seconds / 218 calls** across three functions, no accepted exacts and **40.777%** of target-lift worker time; parent rollback restored the best interim candidates. The measurements support selector-stable result/review joins and bounded no-progress checkpoints, but do not prove a shorter deadline would improve yield. The transcript has one explicit parent `bof3-lift-loop` invocation with no parent duration or native call count. See the [screening and mission analysis](bof3-lift-loop.md#2026-09-28-observation-screening-throughput-and-lift-loop-mission-metrics) and its [body-free receipts](../../tmp/observation-ingest/cycle6-k-window700-20260928/agent-run-metrics.json).

### 2026-09-28 linked K800 reverse/review cohort

The disjoint [K800 run metrics](../../tmp/observation-ingest/cycle6-k-window800-20260928/agent-run-metrics.json) record ten reverse workers over ten functions, with six exact claims and four escalations in **15,019.854 summed worker-seconds / 504 calls**. Five exact selectors match a reviewer pass (**1.438 worker claims per summed worker-hour; 1.159 reviewer-confirmed exacts per combined worker-and-reviewer hour**); the sixth claim is unjoined at the selected-source cutoff. Seven reviewer runs produced seven pass verdicts in **510.542 seconds / 33 calls**, covering five unique selectors because two cleanup edits required re-review. One read-only pass report incorrectly exited as failed after **62.192 seconds / three calls**; the project reviewer completion guard was disabled and later review runs returned normal status. Keep report verdict and process exit separate, and keep direct parent skill runtime unmeasured.

Three one-hour timeouts consumed **10,800.086 seconds / 269 calls**, or **71.905%** of reverse-worker time, with no exacts; parent rollback restored all candidate edits, though one out-of-scope compiler-flags residue needed a separate restore. Five cleanup runs took **427.142 seconds / 20 calls** and produced two edits plus three no-change results. These measures support joining current partial queues to prior outcomes, tracking per-target progress before hard deadlines, checking the full write set after rollback, and separating cleanup yield from lift yield. They do not prove that extending or shortening the deadline would improve exact-match throughput.

The 2026-09-26 metadata audit finds **260 explicit skill selections**, dated
2026-07-23 through 2026-08-01 UTC. Recorded cost exists for 188/260 (72 missing):
**5,011 calls** and **32,248.7 summed seconds**, with calls median/p90/max
**23 / 44 / 128** and recorded seconds **138.1 / 332.1 / 837.7**. Process exit
was zero for 220/260 (84.6%); acceptance was not-required 142, checked 68,
rejected 35 and attested 15. Nineteen records have report-parser failures.
These are task-selection and transport metrics, not accepted-lift throughput.

### One mission with measured run and intake cost

The explicit `bof3-re` run `f5f39958` targeted
`emi/battle/battle/15@0x800A3F28`: **53 calls, 219.793 seconds, exit 1, rejected**
for missing structured acceptance. Its final report was partial at **12/69
(17.39%)**, down from an earlier 22/69 (31.88%); an intended rollback has no later
diff/hash proof. This one run yielded **zero accepted exact lifts** in 3.66 minutes,
not a stable skill-throughput estimate. Directives/references should pin candidate
source, score, byte count and first mismatch, then verify rollback after regression.
Harness receipts should bind the final candidate to the structured report and
acceptance reason; measure verified exact/valid-partial outcomes per call and
elapsed minute, not tool activity alone.

Review indexed **65 blocks / 141,027 characters** from the child transcript; eight
displays showed 63 blocks / 134,753 safe-rendered characters and withheld two
config-like outputs. Review plus final hash check took at most **140 seconds**:
about **27 displayed blocks/minute** and **57,751 displayed characters/minute**.
This is ingest, not skill runtime, and earns no coverage absent a checkpoint. The
192-block allowance did not bind; the 22,000-character ceiling limited batches to
29 blocks. Test a larger character budget on matched input, requiring full hash
review without more clipping, rereads or lost checkpoints.

### 2026-09-28 audit of the 2026-07-23 area030/04 cohort: accepted throughput

Five explicit bof3-re runs worked on
emi/world00/area030/04@0x801E0C80. The first writer reached **83/83 instructions
and 332/332 bytes**, but review found an unnormalized target map and then a local
primitive-append prototype that conflicted with the shared GPU API. The repair
included gpu/prim.h, removed the duplicate prototype and used the shared API;
the final review passed with the exact match preserved.

| Stage | Calls | Summed worker seconds | Result |
| --- | ---: | ---: | --- |
| Writer 37ff8077 | 54 | 197.363 | Exact bytes; first candidate |
| Reviewer c8af5231 | 21 | 89.387 | Needs fix |
| Reviewer ffdb8730 | 19 | 69.175 | Needs fix |
| Repair writer 63eae735 | 21 | 75.775 | Shared API correction |
| Final reviewer a8376300 | 21 | 43.780 | Pass |

The accepted outcome is **one exact target after five runs, 136 calls and 475.480
summed worker seconds**: **7.57 accepted exact targets per summed worker-hour**
or **136 calls per accepted target**. Review and repair beyond the first writer
accounted for **278.117 seconds**, 141% of the initial writer cost; full acceptance
took 2.41 times that cost. Both initial review stages found issues before the final
pass. The repository-wide symbol check remained blocked by unrelated source/map
drift and is separate from the target result. This is one small 83-instruction
target, so the rate is descriptive, not a stable skill estimate.

Transcript screening is a separate throughput measure. The five sources contain
**176 source-local block occurrences** and **554,212 characters**; deduplication
leaves **154 unique hashes / 473,954 characters** (15 repeated hashes, 22 extra
occurrences). Of these, five hashes were reviewed earlier; among the 149 newly
screened hashes, 143 were displayed and six config-like outputs were withheld.
One timed interval covered **64 blocks / 224,579 characters in 242 seconds**:
**15.87 blocks/minute** and **55,681 characters/minute**. This short sample
includes two large repetitive rg outputs and depends on content mix; it measures
transcript screening, not lift execution. The [hash-pinned receipt](../../tmp/observation-ingest/bof3-re-area030-20260723-five-run-screening-receipt.jsonl)
retains per-hash disposition and source origins. It earns zero coverage until
checkpointed.

Use the result to prioritize and measure three changes:

- Skill directive: preflight target-map normalization and shared declaration
  ownership before treating a byte-exact candidate as review-ready. Track first
  review passes and repairs separately from accepted exacts.
- Reference material: show how to find and use the shared GPU declaration rather
  than adding a conflicting local prototype, with the map-normalization failure
  as a separate setup example.
- Harness: check map normalization and duplicate/conflicting declarations before
  reviewer dispatch; report target-local gates separately from repository-wide
  drift. On matched targets, compare first-review pass rate, calls and summed
  worker time per accepted exact lift while preserving all native and review gates.

Same-source Cycle 6 Cohort Y windows 3–22 accepted **200 blocks / 280,893
characters in 9,217.786 seconds**: **1.302 blocks/minute**, **1,828.4
characters/minute**, and **8/20 zero-yield windows**. The 788-block remainder
projected to **605.2 operational minutes** at the pooled rate; positive-window
projections spanned **35.6–2,973.6 minutes** (not a confidence interval). W22
screened 135 blocks at **10.533 blocks/minute** but accepted zero; all were
quarantined. This intake rate is not skill runtime: optimize yield by comparing
content-boundary isolation or smaller quarantine batches on matched input, using
accepted blocks/characters per minute, zero-yield frequency, quarantine size and
checkpoint completeness. Cycle status was **5,212/6,000**; W22 earned no coverage.

The broader screening pool contains **3,119 candidate runs**, including 2,859
selected only by agent name. It is **not a verified per-skill denominator**:
`bof3-reviewer` run `a6291437-2d16-45ca-ab5a-ed5c2e0bf586` reviewed a naming
no-op, as its complete output and parent workflow prove. The tables below retain
historical role costs for investigation while actual mission ownership is
reconciled. The two agent generations are not a controlled comparison.

| Cohort | Runs | Recorded tools/duration available | Tool calls median / p90 / max | Recorded seconds median / p90 / max |
| --- | --- | --- | --- | --- |
| `bof3-reverse` (July–August) | 1,272 | 1,154 | 21 / 39 / 220 | 156.7 / 435.6 / 7,200.1 |
| `bof3-review` (July–August) | 1,062 | 1,041 | 10 / 20 / 36 | 68.3 / 135.1 / 333.9 |
| `bof3-lifter` (September) | 678 | 0 | unavailable | unavailable |
| `bof3-reviewer` (September) | 90 | 1 | 78 / 78 / 78 | 315.0 / 315.0 / 315.0 |

Seventeen other explicitly selected missions contribute to the screening pool but
are excluded from those role rows. Across all 3,119 candidates, 2,211 carry
both recorded fields (908 missing): **40,223 tool calls** and **406,869.4 recorded
seconds**. This is summed run time, not elapsed campaign wall time.

Transcript tool-start IDs independently recover September call counts: lifters
**69 / 121 / 205** (median/p90/max, 678/678 runs); reviewers **46.5 / 98 / 121**
(90/90). Their observed first-to-last-event spans are respectively
**597.4 / 1,652.5 / 5,399.3 seconds** and **351.0 / 763.1 / 2,081.5 seconds**.
Observed spans exclude unrecorded launch/cleanup time and do not fill missing
`durationMs`. No accepted-lift throughput is claimed until child results are
linked to independent reviews and final gates.

**Screening-pool outcomes are separate measures.** Process exit was zero for 2,965/3,119 runs
(95.1%); 154 were nonzero. Metadata acceptance was checked 1,079, attested 866,
not-required 650, rejected 452, verified 48 and review-required 24. None of those
labels alone proves an exact, semantically accepted lift. Report parsing failed
in **75/3,119 (2.4%)**: 68 missing reports, four unsupported-field reports and
three malformed JSON reports.

Provenance: `_meta.json` and matching `_transcript.jsonl` under
`.pi-subagents/artifacts` and `.pi/sessions/subagent-artifacts`; per-run paths,
hashes and cohort attribution are in `tmp/observation-ingest/runs.jsonl`, with
aggregates in `metrics.json` and independent event counts in
`transcript-metrics.jsonl` in the same directory. Quantiles use nearest-rank p90.
Forty corpus-wide records with metadata zero have 1–2 requested calls but no
tool-start/end events: 22 output reports record `ENOENT` session-path failure and
18 record `EEXIST`. Sixteen belong to `bof3-reverse`; the other 24 are lane
orchestration. Requested calls are not executed calls; keep these launch failures
separate from skill execution cost.

### Measured improvement candidates

| Measured problem | Proposed owner/change | Acceptance measurement |
| --- | --- | --- |
| 19 parser failures among 260 explicit selections; 75 in the broader candidate pool | Reverse/review mission references: provide the supported report structure and a concise completion checklist; investigate a project-owned format check without modifying installed Pi extensions | In a comparable attributed cohort, missing/malformed/unsupported reports are 0/N; independent review and native gates remain required |
| September lifter p90 is 121 calls, versus 69 median | Mission protocol and harness sweep/gate call sites: attribute the tail to probe count, repeated context, races and required validation before choosing a change | Compare like-size/like-residual missions; lower calls and duration per independently accepted or valid partial result without increasing regressions or skipped gates |
| 72/260 explicit selections lack recorded cost; agent-name routing includes a naming review | Observation measurement: bind task ownership and retain recorded, requested and started counts separately | Every included run has attributable scope and measured or explicitly missing cost; no role-based contamination or silent zero fill |
| 9/15 selected exact reports required measured shape changes; six were first-seed exact | RE references: surface same-target exact siblings and confirmed representation/order evidence before broad sweeps | For matched candidates, report first-seed exacts, measured variants, live diffs and independent reviews per candidate-size band |
| A partial source/manifest publication blocked tools and 6/10 lanes; stale selectors and a same-target duplicate writer also consumed capacity | Lift-loop preflight and `next-lift`: publish source/claim atomically, filter current worktree claims and enforce one active writer per selector | Zero cross-target failures from in-flight claims, stale picks or overlapping writers; preserve strict manifest checks and every native gate |
| Wait-window ceilings varied 13.8–27.6 exact reports/hour but omit dispatch and review | Harness telemetry: bind dispatch, child start, retry, final gate, reviewer and terminal events | Report dispatch-to-last-terminal and independently reviewed exacts/hour, with retries by failure class and matched task mix |
| Batch 2 produced 29 exact reports plus one reverted partial, but only seven reviewer outcomes were reported accepted and the full review denominator is missing | RE review reference: bind each exact result to its reviewer outcome and retain unsupported-name corrections as a separate concern | All exact reports have an explicit review state; accepted exacts/hour uses only independently accepted selectors and complete dispatch-to-terminal timing |
| Batch 3 classified 3/24 candidates as data/rodata while 21/24 were exact C lifts; batch 4 had 5 failures among 9 observed async runs, 4 resumable | RE/lift-loop decision guidance: separate code-lift eligibility from boundary/data-layout work and resume persisted sessions before replacement | Every selector has one terminal or resumable disposition; classification rows never enter the failed-lift denominator; retries and review outcomes reconcile |
| `coverage` returned rc=2 while producing usable JSON in batches 2 and 3 | Harness receipt: retain exit code, artifact existence, schema validation and consumer decision as distinct fields | A nonzero CLI exit with an artifact is explicitly classified; no coverage/gate success is inferred without validated output and source-count reconciliation |

These are proposals, not changes to current skill or harness contracts. A
historical pin, volatile trick or compiler override below is not authorization
to reuse it; current reverse-engineering evidence gates take precedence.

## Recent cohort and harness optimization evidence

The older parent `01a0c062`, records **1907–1983**, reports ten exact lift results
in one ten-lifter wait and nine exact results in a later ten-lane cohort that still
had one active run after 50 minutes. The pinned transcript is
`.pi/sessions/2026-09-20T19-54-21-668Z_01a0c062-4864-703b-a873-798c0b9fcbb0.jsonl`
(SHA-256 `cb3a63e57b9374ff832c0ec2ef522a2852e5ee54f0dd2d2d5dbc6250c42e6751`).
These are parent-reported native results; this slice does not provide a linked
review receipt or per-child cost for each function.

The ten-result batch added one reported exact each at `game/00@0x801C6FD4`,
`battle/15@0x800A3060`, `shop/00@0x801E3B7C`, `battle/03@0x801DB844`,
`world00/area030/04@0x801DA3F4`, `etc/commu00/00@0x801F1444`,
`etc/sisyou/00@0x801D12FC`, `world00/area008/13@0x801F357C`,
`world00/area016/13@0x801F3060` and `world00/area024/14@0x801F2D5C`.
The parent totals moved exact **860→870**, lifted **1,010→1,020**, partial stayed
150 and arithmetic unlifted moved **1,337→1,327** out of 2,347 indexed. A later
`lifts: valid=1,021` receipt disagreed with the 1,020 lifted total; keep that
reconciliation open. The 22m37s wait for ten completions gives only an optimistic
ceiling of **26.5 native-exact reports/hour**. Full dispatch-to-last-result time,
per-child calls and independent-review acceptance are not established, so this is
not accepted-lift throughput.

The parent supplied a separate estimate of **100–120 agent calls per match**;
that denominator is not reproduced by this cohort. The broader September lifter
sample above has 69 median and 121 p90 tool starts, a different screened run
population. Do not substitute either cohort for the other.

The optimization trial added a combined gate command, stale-index ranking support
and a shared-CMake configure lock. A stale ranking worked. The combined gate needed
repairs for missing `hashlib` and `Path` imports before an exact candidate passed
and a partial candidate failed as intended. The first touched-module test run
reported 9 failures/452 passes because the new `allow_stale` keyword broke a
one-argument test stub; after the compatibility correction, the full touched suite
reported **461 passes/44.90s**. A stale index rebuild became green after target
analysis was refreshed, and scoped `just check` passed in 54s. No post-lock
concurrency cohort was measured, so the proposed 19-minute CMake contention cause
and any throughput gain remain unproven.

The estimated call savings are conditional. Combining five separate agent-level
gates into one wrapper saves four orchestration calls when all five were otherwise
issued; it does not remove the underlying checks. The stale-ranking fallback was
estimated to cost another five to eight calls on affected missions. Together that
is **9–12 calls (about 7.5–12% of 100–120)** only for missions hitting both costs.
A useful follow-up records agent calls and wall time independently, with exact,
partial, blocked, review-repair and gate-failure outcomes. Also record lock wait,
configure attempts, build races and retry time; compare matched candidate sizes
and concurrency before claiming saved time or higher accepted throughput.

### Post-change cohort outcomes and rate ceiling

Records **1984–2009** contain ten completed async runs and eleven function
reports: eight exact and three partial, because one lane returned two function
results. The 28m46s wait yields an optimistic ceiling of **16.7 reported exacts
per hour**. Parent totals moved exact 880→888, lifted 1,030→1,041 and partial
150→153; `lifts valid=1,042` differed from the reported lifted total. The complete
dispatch interval and independent reviews are not available.

The next ten-lane cohort, records **2010–2034**, completed ten runs after a
21m33s wait and reported:

| Reported outcome | Count | Selectors |
| --- | ---: | --- |
| Exact, live byte-match reported | 8 | `game/00@0x801A8CB4`, `shop/00@0x801D46E0`, `battle/03@0x801E320C`, `area030/04@0x801E09B4`, `commu00/00@0x801F1538`, `sisyou/00@0x801D282C`, `area008/13@0x801F4424`, `area024/14@0x801F2CB0` |
| Partial | 2 | `battle/15@0x8009CCBC` 98.87%; `area016/13@0x801F30EC` 58.97% |

Eight exact reports over the wait window give at most **22.3 reported exacts/hour**;
the ten terminal runs give at most 27.8 reported function results/hour. The
dispatch-to-last-terminal duration is longer or unknown, and no independent review
is recorded, so **accepted exact throughput is unmeasured**. The two cohorts have
different tasks and only wait-window bounds; their rates cannot establish that the
gate wrapper, stale-ranking fallback or configure lock improved throughput.

Parent batch totals moved exact 888→896, lifted 1,041→1,051, partial 153→155 and
unlifted 1,306→1,296. The same report listed `lifts valid=1,052`. `bin/index`
returned rc=2 for a conflicting function/global symbol,
`emi/world00/area016/13:D_801F4D7C`; a later printed rc=0 followed a `tail` and did
not prove the index command succeeded. The scoped check separately returned rc=0
in 9s. The conflict and one-record discrepancy remain open; do not convert these
outputs to a clean batch-acceptance count. At least four of the ten lane reports
also describe transient build/configure or cross-lane source-claim races. Their
mixed causes and missing lock telemetry leave the configure lock's effect
unmeasured.

### Records 2386-2638: throughput and wasted work

Five later ten-lane batches report wait-window ceilings between **13.8 and 27.6
native exact reports/hour**. These are optimistic report rates, not dispatch-to-last
completion or independently accepted exact throughput. Batch 32 is internally
inconsistent: its parent summarized 10 exact results, but the child reports show
9 exact and one partial (`commu00@0x801F205C`, 93.10%). Batch 28 redirected two
SDK-scope selectors; keep scope correction separate from EMI lift productivity.

| Batch | Child-report outcome | Wait | Exact-report ceiling |
| --- | --- | ---: | ---: |
| 28 | 10 exact; 2 SDK-scope redirects | 25m53s | 23.2/hour |
| 29 | 10 exact | 23m34s | 25.5/hour |
| 30 | 9 exact, 1 partial | 23m05s | 23.4/hour |
| 31 | 9 exact, 1 partial | 19m36s | 27.6/hour |
| 32 | 9 exact, 1 partial (parent said 10 exact) | 39m11s | 13.8/hour |

The subsequent child reports in records 2581–2638 include 15 distinct exact
mission reports, six exact on the first live seed and nine requiring at least one
measured shape change. One more exact selector was verified by a standby lane that
made no edits after a concurrent duplicate writer finished it. These reports span
overlapping fan-outs: they cannot be treated as 15 net newly covered selectors or
as a complete cohort denominator. Parent exact-count movement is lower than this
selected child-report count. For performance tracking, classify `first-seed exact`,
`adjusted exact`, `partial`, `scope redirect`, `stale pick`, `standby verification`,
`blocked`, and `independently reviewed accepted` separately.

An advertised-but-absent source in `emi/etc/commu00/00/target.toml` (records
2490–2517) disabled
manifest-dependent tools across the shared repository for about 10–12 minutes;
six of ten batch-33 lanes reported failure. The claim was being exposed before the
source file landed. Keep the tested strict manifest invariant; evaluate atomic
source/claim publication and pre-dispatch claim-set checks instead of tolerating a
missing claim target. Separately, stale `next-lift` picks returned already claimed
addresses, and one battle15 selector had two active writers. The corrected live
claim filter returned three verified-unlifted selectors and passed 155 tests, but
there is no matched pre/post throughput sample. Several reports still show
shared-build/configure races and retry cost.

The evidence does not establish which proposed change raises accepted throughput.
Use it to prioritize telemetry and controlled comparisons, not to claim the stale
filter, claim publication or concurrency controls already improved speed.

### 2026-09-26 throughput measurements and optimization priorities

The completed observation-harvest cycle processed **1,000/1,000 blocks** from
the pinned 2026-09-20 parent transcript cited above. Its 2026-09-26 corpus snapshot
was **6,682/356,571 unique blocks
(1.874%)**, with 349,889 pending; this is parsed-block coverage, not reviewed
skill coverage. Its 989 timed blocks contained 1,848,450 characters and took
4,476.201 seconds: **13.257 blocks/minute** and **24,777 characters/minute**.
Eleven blocks in the cycle were untimed. The previous completed sample measured
729 blocks / 1,261,023 characters in 2,556.81 seconds (**17.107 blocks/minute**,
**29,598.5 characters/minute**). The samples have different content mixes, so
the lower current rate is a signal to investigate, not a causal regression claim.
At that historical semantic-review rate, 349,889 remaining blocks projected to
about **440 active review hours** (roughly 18.3 continuous days), excluding setup,
checkpoints and reconciliation; corpus mix may change that estimate. The 1,000
block cycle itself corresponded to about 75 minutes at that rate. The
contemporaneous recommendation was to keep the 1,000 target while collecting
more timed cycles; the later user-directed cycle 5 target is 5,000, as recorded
in the [current ingestion status](INDEX.md#full-history-ingestion-status).
Character-volume sizing still matters: this cycle averaged about 1,870
characters/block, and large responses already approach the tool output limit.
The corpus denominator grew by 270 from the earlier 356,301 snapshot; its cause
was unresolved at that point, so retain snapshot time and denominator with every
coverage percentage.

For BOF3 missions, the recent pinned parent transcript reports these completed
mission rates over wait windows:

| Lanes completed | Wait window | Reported completions/hour | Additional outcome evidence |
| ---: | ---: | ---: | --- |
| 5 | 18m37s | 16.1 | Outcomes were mixed; exact and partial totals are not a reliable accepted-rate numerator |
| 10 | 24m27s | 24.5 | Five new exact dispositions and one new partial; other updates/escalations overlap |
| 10 | 21m40s | 27.6 | Exact/partial numerator not reconciled |
| 10 | 38m40s | 15.5 | Independent review count absent |
| 10 | 24m58s | 24.0 | Nine reported exacts and one partial: 21.6 exact reports/hour |

These are optimistic wait-window rates, not dispatch-to-terminal rates; waves
have different task mixes and terminal reports arrive asynchronously. The 24m58s
exact numerator is a submission-report count, not independent acceptance.
**Accepted exacts/hour remains unmeasured.** Do not infer a speedup from the
15.5–27.6 mission/hour range or combine overlapping dispositions into a rate.

Use these measurements to drive the next instrumentation and optimization work:

| Opportunity | Evidence and proposed change | Acceptance measure |
| --- | --- | --- |
| Estimate harvest work and cycle sizing | Store timed blocks, characters, elapsed time, untimed count, corpus snapshot/hash and completion percentage per cycle; recalculate ETA from rolling rates | Report block- and character-based rates plus an ETA range after multiple cycles; keep cycle size bounded by actual response size and wall time |
| Separate lane throughput from lift yield | Emit dispatch, start, first seed, variant compile/measurement, gate, retry, review and terminal timestamps with a unique selector/run ID | Report dispatch-to-terminal median/p90, missions/hour, exact reports/hour and independently reviewed exacts/hour separately, with task-mix strata |
| Avoid repeat compiler work | Hash emitted objects for variants; area008 had 11 clean-C variants without progress and multiple source-order forms compiled to identical objects | Skip already-seen object hashes and reduce measured compile/gate count without lowering exact or partial outcomes on a matched cohort |
| Reduce shared-tree retry cost | Record claim conflicts, manifest loss, configure/build races and retry wall time; evaluate serialized or atomic claim publication under a controlled cohort | Fewer race retries and lower terminal time at the same fan-out, with strict manifest and native gates unchanged |
| Keep rate denominators valid | Check that mutually exclusive exact, partial and in-scope-unlifted counts reconcile to the indexed denominator; prior reports violated this while `just check` returned rc=0 | Refuse to publish coverage or yield rates when conservation fails; keep SDK attribution separate unless linker inclusion is proven |

These are measurement-backed proposals, not evidence that the proposed harness
changes already improve performance. The timed 1,000-block sample measures
semantic-review throughput; wait-window mission rates measure BOF3 dispatch;
neither substitutes for independently accepted lift throughput.

### 2026-09-27 parent batches 2-4: outcomes and throughput

The reviewed parent slice is records **2326–2410** in
`.pi/sessions/2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4.jsonl`
(full-source SHA-256
`40e8500c61545a9d44898e0ccd7ff36d3447de218fcaa047fc5fa952b2d2b5d1`). The
outcome counts are useful for sizing skill and reference changes, but the waits
and review receipts do not support an accepted-throughput rate:

| Batch | Selector/result denominator | Native gates and parent accounting | Timing and independent review | Valid performance measure |
| --- | --- | --- | --- | --- |
| 2 | 10 target lanes; 29 exact lift reports and 1 partial report reverted | Splat 10/10; scoped build 10/10; index recovery + index rc=0; source validation 2,128 valid / 0 invalid (+29); indexed coverage 1,883→1,912 (+29), 18.00%→18.28% | 30m43s wait covered four async runs, not full dispatch-to-last-terminal. Seven reviewer outcomes were reported accepted; the denominator and coverage of all 29 reports are unknown. Coverage command returned rc=2 while producing the JSON used. | Preserve counts and gates; exact-report/hour and independently accepted exacts/hour are unmeasured. |
| 3 | 24 selector candidates: 21 exact C-lift reports (87.5%) and 3 data/rodata boundary reclassifications (12.5%); 2 lanes still needed broader data-layout work | Splat 10/10; scoped build 10/10; index rc=0; source validation 2,149 valid / 0 invalid (+21); coverage 1,912→1,933 (+21), 18.25%→18.45% | 27m40s wait covered nine async runs; 10 lifter and 5 reviewer children were reported. Coverage command returned rc=2 with usable JSON. Complete reviewer and dispatch intervals are absent. | The 21/24 exact share is a candidate disposition, not accepted throughput. The three data cases are not failed code lifts. |
| 4 | Nine async statuses observed: 4 complete, 5 failed; 4/5 failed children retained resumable sessions (80%). Remaining lane/terminal accounting is unresolved. | Several children produced repeated needs-attention alerts after bash remained open over 240s. | 45m06s wait; no complete dispatch interval or reconciled independent review. | No exact-yield or accepted-throughput rate; preserve incomplete and resumable states. |

Batch 2's reviewer found an evidence-quality defect even though the lift bodies
were exact: the name “BankByte4” and “effect-bank” role were unsupported, and the
explanatory wording overclaimed semantics. The writer changed only names and
wording, left bodies and match proofs unchanged, passed refreshed gates, and
received a second-review acceptance. Measure naming/evidence corrections
separately from byte exactness and review acceptance.

These data support a concrete RE workflow improvement: report unique selector
outcomes as exact, retained/reverted partial, data-boundary reclassification,
broader data-layout block, failed, resumable, and independently accepted. Keep
the denominator at selector level and require a source witness for semantic
names. References should give examples of supportable role labels and identify
the evidence required to promote asm to data/rodata. The parent timing and
deduplicated attention recommendations are in the [lift-loop analysis](bof3-lift-loop.md#2026-09-25-parent-records-2326-2410-lift-yield-and-supervision).
Do not claim that these changes improve speed until a matched cohort records
dispatch-to-last-terminal time, review completion, first-pass dispositions,
corrections, retries and gate outcomes.

### 2026-09-27 `bof3-re` transcript: screened throughput and zero accepted lifts

Transcript `.pi-subagents/artifacts/40d637e1_bof3-reverse_0_transcript.jsonl`
has SHA-256
`248548ae54805f181a9704a08ac9e2089248b8afd6b9cc5cd35dc39fa0261e5e`, 493
lines, 232 unique blocks and 398 origin pointers. All 231 pending unique blocks
were accounted for; one had already been reviewed. Thirteen source-order batches
contained 1–47 blocks (mean 17.8) and 296,590 pending source characters. From
17:25:19.684 to review completion at about 17:37:16 UTC, the timed pass covered
231 blocks in 716 seconds: **19.35 unique blocks/minute** and **24,843 source
characters/minute**. This includes provenance reconciliation, excludes manifest
preparation and later ledger work, and is a screening rate rather than lift yield.
Two roughly 50 KB build-output blocks were isolated and bounded-profiled because
their referenced full capture was unavailable; they count as accounted, not as
complete original tool logs.

The child task itself covered one selector for **837.702 seconds (13.96 minutes),
128 tool calls and zero accepted exact lifts**: accepted-exact throughput was
**0 per task-hour**. The best displayed candidate reached **191/339 (56.34%)**,
but no final verification or structured acceptance report exists; the child ended
with exit 143. Metadata says 104 turns while a captured terminal note says the
turn budget stopped at 21 assistant turns, so turn rate is not reliable. The run
requested three unavailable context tools despite the strict allowlist, lacked
the target's Splat output, and initially used the wrong Rizin architecture. Its
task explicitly forbade setup, yet the transcript records setup and ad-hoc
dependency-install attempts. Treat this as a readiness/scope failure, not a
representative estimate of steady-state lifting speed.

The 231 reviews reduce the explicit `bof3-re` pending backlog from 9,037 to
8,806 (**2.56%**); global block coverage is 19,141/379,196 (**5.05%**, 360,055
pending). These denominators answer corpus progress, not skill productivity.
Packing the same blocks at a 32 KB source-character cap projects 13 batches to
10 (23% fewer), keeping the two oversized blocks isolated; this is a packing
simulation, not a measured time saving. Trial that cap only with a response-token
guard and compare blocks/minute, characters/minute, clipped batches and rereads.

| Improvement destination | Evidence-based change | Acceptance measure |
| --- | --- | --- |
| Skill directive | Preflight every required tool and target artifact; stop when a task forbids setup or required tools are unavailable | Out-of-scope setup/tool failures and accepted exact lifts per selector-hour, by blocker class |
| Reference material | Add the missing-Splat/wrong-architecture recovery path and warn when an exact duplicate's register pins are outside task authority | Independently reproduced recoveries and accepted lifts after preflight |
| Harness tooling | Validate child allowlists before dispatch; retain complete tool output with hash/offset pagination and reconcile turn, call, and duration units | Zero allowlist surprises; fewer clipped rereads; comparable dispatch-to-terminal and accepted-exact rates |

### 2026-09-27 M13 continuation: compiler ceiling with no exact lift

Transcript `.pi-subagents/sessions/lift-loop/m13-emi-world00-area008-13-0x801f3d88-continuation/run-0/session.jsonl` is pinned by SHA-256 `f2181f1b608eb53decd8cc1da3a1203f80a4a0f6e7ae57fc60ddc6bf7fc9d83b` (1,316 lines). The child used **633 tool calls** over an **84m36.836s event span** (**7.48 calls/minute**); recorded `durationMs` is absent, so this is not active execution time. The one target remained **326/339 instructions (96.17%)**, with **zero score gain, zero retained variants and 0/1 accepted exact lifts** through attempt **50/50**.

The compiler ceiling was reported after attempt 9 with parent review required. The transcript continues through attempts 10–50 (41 further attempts) without changing the score. Because the source includes intervening parent prompts but no independent parent workflow record, treat this as a handoff-boundary failure signal, not proof of an isolated child disobedience. Three historical compilers scored below baseline; the bounded 60-second permuter ended at 1,924 iterations, 1,388 compile errors and an infinite score. One flag search lacked `compile_commands.json`; a hexadecimal seed spelling was rejected before retrying in decimal. Full screening and attempt-level details are in the [pinned metrics](../../tmp/observation-ingest/m13-area008-continuation-full-metrics.json).

| Improvement destination | Evidence-based change | Acceptance measure |
| --- | --- | --- |
| Skill directive | Preserve the existing review-pending stop; require each continuation handoff to state the parent's decision and cumulative attempt count | On comparable missions, zero substantive attempts occur after the stop state without a parent decision; exact/partial yield and required checks do not regress |
| Reference material | Show compiler-ceiling exit criteria, remaining evidence, compile-database readiness and accepted seed syntax | Missing-prerequisite and invalid-seed starts per eligible search; valid comparisons / attempted comparisons |
| Harness tooling | Prevent queue launch after a parent-review-required result until a parent disposition and cumulative attempt count are recorded | Zero post-gate launches without disposition; every attempt joins to one selector, baseline score, final score and terminal outcome |

### 2026-09-27 `49745aa6` child transcript: throughput baseline and partial compiler result

Transcript `.pi-subagents/artifacts/49745aa6_bof3-reverse_0_transcript.jsonl`
is pinned by SHA-256
`226100e7e8104eacf5c3b9f784ffbfb742f11fc78f70d967a1e0f900e9fd1304` (722
lines, 1,252,942 bytes). It has 370 unique blocks: 369 pending blocks / 466,008
characters were screened and checkpointed; one block was already reviewed.
Across the four windows, content comprised 140 reasoning, 151 tool-output and 78
assistant-text blocks. Fifty bounded displays and four checkpoint transactions
covered the pending blocks; every source occurrence was hash-revalidated. One
32,678-character block needed three bounded slices, and one clipped aggregate
display was recovered. No other window clipped or required rereads.

| Window | Target / completed | Characters | Timed seconds and scope | Blocks/min; chars/min | Displays; mean / max blocks |
| --- | ---: | ---: | --- | --- | --- |
| 1 | 64 / 64 | 141,608 | 491.500; review plus origin reconciliation to precommit | 7.813; 17,286.8 | 13; 4.92 / 18 |
| 2 | 64 / 64 | 101,508 | 316.511; review plus origin reconciliation to precommit | 12.132; 19,242.6 | 12; 5.33 / 14 |
| 3 | 128 / 128 | 124,340 | 203.199; review plus origin reconciliation to precommit | 37.795; 36,714.8 | 15; 8.53 / 16 |
| 4 | 113 / 113 | 98,552 | 155.698 active review; 0.013 source reconciliation; 219.492 precommit total | 43.546; 37,978.2 | 10; 11.30 / 21 |

Windows 1–3 lack a review-complete marker and combine screening with source
reconciliation, so their rates are descriptive only and excluded from optimization
recommendations. Window 4 isolates review through an explicit completion marker;
its 63.781 seconds between that marker and the checkpoint command are unattributed,
not review time. This is one clean 113-block sample, not proof that cycle size
caused the higher rate. The 128-block window had no clipping, and average display
packing rose from 4.92–5.33 blocks in the two 64-block windows to 8.53, but source
mix and timing differ.

Global coverage after the checkpoint is **19,890/379,196 (5.245308%)**, with
**359,306 blocks pending**; the reopened database and progress file agree, across
21,102 inventoried sources and zero parse errors. At the isolated 43.546-blocks/min
sample, the backlog scales to **137.5 active hours** if all remaining content behaves
similarly. This is a one-window sensitivity scenario, not a planning ETA. At that
rate, a 128-block cycle is about **2.94 active minutes**, before reconciliation,
checkpoint, ledger and validation time. Run a clean 128-block control, then a
192-block trial under the same display-character cap and content mix; capture
review-complete and reconciliation boundaries separately before increasing again.

The historical child task was one target, `emi/world00/area008/13@0x801F3D88`.
Metadata records 180 tool calls and 7,200,045 ms; the best candidate reached
**310/339 instructions (91.45%)** at unchanged 1,356-byte size, but no exact lift
was accepted (**0/1**). These are lift outcome measures, separate from transcript
screening throughput.

| Improvement destination | Evidence-based proposal | Acceptance measurement |
| --- | --- | --- |
| Skill directive | Keep score gain, byte size and accepted exactness as separate outcomes; vary one compiler-sensitive lever at a time and preserve the best verified candidate | First-pass exact/partial/failed outcomes per selector-hour; score regressions and changes reverted; no acceptance inferred from a high instruction percentage |
| Reference material | Document target-specific pin, scheduling and operand-order evidence with the observed asm diff; warn that promising local pins do not authorize generalization | Comparable targets reproduce the claimed residual change with the same compiler/profile and unchanged source scope |
| Harness tooling | Add explicit review-end and reconciliation timestamps; isolate candidate/seed workspaces; validate heuristic rankings against native `asm-diff` and bind each result to source/object hashes | Matched 128/192-block windows report active blocks/min, chars/min, display density, clips/rereads and reconciliation time; heuristic/native rank disagreement and workspace collisions are counted; all source hashes reconcile |

### 2026-09-27 `0309437d` transcript: throughput baseline and pending checkpoint

The pinned child transcript `.pi-subagents/artifacts/0309437d_bof3-reverse_0_transcript.jsonl`
(SHA-256 `72bc45054bff23d919bb7939d633511f5be2933ead23fb47ee04cd3ab6c87988`)
and its linked parent launch/result records cover one 251-instruction, 1,004-byte
target. The writer used **56 tool calls / 295.970 seconds**, reported **152/251
(60.56%)**, and escalated with **0/1 exact lifts**. Its first compiled candidate
matched 37.45%; a broad field/accessor macro experiment regressed to **41/304
(13.49%)** and **1,216 bytes** (+212 versus the original). The draft returned to
60.56% / 956 bytes, with the first residual at **+0x0004**. The initial compile
also exposed missing type/constant declarations; the final symbol check was
blocked by repository-wide source/map drift. These are writer-reported outcomes,
not independent acceptance.

The transcript-screening cohort contained **114 unique blocks / 163,075 characters**.
Active screening took **263.358 seconds** (**25.972 blocks/minute; 37,152.9
characters/minute**). Tool output was **90/114 blocks** and **161,009/163,075
characters (98.73%)**. Block sizes were strongly skewed: median **70**, mean
**1,430.5**, p95 **8,712**, maximum **19,873** characters. Revalidation covered
**4,399 origin pointers across 673 pinned source files**. Source/origin verification
took 6.0 seconds; the manifest-start to precommit interval was 749.760 seconds,
including 98.921 seconds before screening and other un-attributed work.

The final display segment moved from a 20-block/10,000-character cap to
40-block/11,000-character cap. The two observed calls returned 20 blocks / 5,977
characters and 35 / 7,098 respectively. Because both caps changed and block sizes
vary sharply, this is not a matched speed comparison. The earlier display-call
count was not persisted, so full-interval calls/display and per-call latency are
unknown. Keep the 40-block setting as a trial; compare it with 20 at the same
character ceiling on matched content, logging blocks, characters, latency,
clipping, rereads and redactions for each display.

Applying this one-run rate to the **359,302-block / 1,214,455,686-character**
backlog gives separate sensitivities of **230.6 active hours** by blocks and
**544.8 hours** by characters. The cohort averaged 1,430.5 characters/block,
while the backlog averages 3,380.0; neither projection is a calendar ETA or a
matched forecast. Use the divergence as evidence to size cycles by both blocks
and characters and stratify throughput by content mix.

All screened bodies and origins were verified, but two SQLite commit attempts
returned `database is locked` on the `fuseblk`-backed coverage database. The 114
blocks remain pending and receive **zero coverage credit**: global coverage stays
**19,894/379,196 (5.246363%)**, with 359,302 pending. The hash-verified screening
receipt is retained at
`../../tmp/observation-ingest/bof3-re-0309437d-screened-not-checkpointed.jsonl`;
resume its checkpoint after the lock clears.

| Improvement destination | Evidence-based proposal | Acceptance measurement |
| --- | --- | --- |
| Skill directive | Preflight declarations used by the candidate before its first compile; test fixed-base field access one change at a time and retain residual/size regressions | Missing-declaration starts and regressive experiments per comparable mission; exact/partial outcomes and required gates unchanged |
| Reference material | Add this fixed-base versus broad-accessor result as a caution, not a universal representation rule | Reproduce the register/size effect on another eligible target before generalizing |
| Harness tooling | Persist per-display block/character counts and latency; separate screened from checkpointed counts and expose DB-lock failures | Full call telemetry; 100% screened-body and origin reconciliation; zero coverage advancement until commit is verified |

### 2026-09-27 `d120b984` child mission: 91.15% partial, no accepted exact

The historical target was `emi/world00/area008/13@0x801F3D88`. Its pinned
child session, transcript, parent and final output are listed in the
[cohort manifest](../../tmp/observation-ingest/d120b984-pinned-cohort.json);
the [screening receipt](../../tmp/observation-ingest/d120b984-screening-review-receipt.jsonl)
records each reviewed block and source origin. The run record reports **220
started calls**, **5,020.186 seconds**, exit **1**, and rejected acceptance; the
transcript has 220 call-starts over **4,942.007 seconds**, including 157 Bash
and 63 edit calls, 55 `asm-diff` and two `byte-match` calls. Ten error-end events
are not classified. The only final output is 53 characters and lacks the
required structured acceptance report.

The best reported candidate reached **309/339 instructions (91.15%)**, stayed
at **1,356 bytes**, and moved the first mismatch to **+0x00D8**. This is a
non-exact candidate, not an accepted lift: accepted exact throughput is **0/1
targets**. A permuter run spent 280 seconds and reached 22,934 iterations,
282 errors and 898 failures; its best score was **2460** against base **3245**.
Its only source mutation wrapped two stores in `do { ... } while (0)`. Applying
that wrapper raised the asm-diff score from 87.94% to 91.15% at equal byte
size, but did not close the residual. A different 2540-score candidate moved
`left_inner_x` below its uses and is invalid; permuter score is not semantic
validity or byte-match acceptance.

Several negative controls narrowed, but did not explain, the scheduler issue:
reordering the prim3 y0 store emitted identical code; removing all pins still
hoisted the prim1 bottom chain; the no-sched1 diagnostic left sched2 active and
changed allocation, while disabling both schedulers also changed allocation.
Moving `left_inner_x` earlier regressed to 75.58%; reordering prim4 updates
regressed to 86.47%; unpinning `u_width` regressed to 61.06%. Reordering
palette materialization after cursor setup reached 88.53%, still partial. A
stale assembly file also survived source restoration once, so comparisons need
candidate/build identity. These probes do not establish pins or any one compiler
pass as the cause, and historical pin use is not authorization under current
contracts.

The transcript-screening cohort is now fully reviewed: **440/440 unique blocks**
from 782 entries across three pinned manifests, **669,766 indexed characters**,
with 83,884 verified repeated characters avoided and 585,882 novel characters.
The last 125 blocks contained 58,859 characters and took 370.115 seconds:
**20.264 blocks/minute; 9,541.7 characters/minute**. They were compact (101/125
tool-output blocks, median 65 characters, maximum 8,211) and arrived in nine
bounded displays without clipping or rereads. The preceding 60-block window
contained 59,755 characters in 929.918 seconds (**3.871 blocks/minute; 3,855.5
characters/minute**), but included a full index/hash integrity reread and
receipt reconciliation. A prior 50-block window measured 11.6 blocks/minute
and 15.4k characters/minute on 66,345 characters. Different block sizes and
timing scopes explain why block and character rates diverge; these are archival
screening rates, not BOF3 mission speed or a causal batch-size comparison.

All 440 source blocks are screened, but the checkpoint remains pending and
coverage credit is **zero**. The latest global snapshot is still **19,894/379,196
(5.246363%)**, with 359,302 pending; completing this source cohort does not
complete repository-wide semantic ingestion. The latest snapshot contains
1,214,455,686 pending unique characters (20:53:45 UTC). Extrapolating the last
125-block cycle gives **12.3 active-work days by block rate** but **88.4 by
character rate**; the 7.2x gap follows from 471 characters per sampled block
versus 3,380 per pending block. These are continuous-work estimates from one
compact sample, not calendar forecasts. Track both rates and content mix; use a
dual block/character cycle cap so short blocks can be batched without letting
large blocks inflate cycle cost. The 60-block re-read followed a suspected
index-label mismatch; full isolated hash reconciliation confirmed the mapping.
Validate range-to-hash mapping before semantic review to avoid repeating that
cost.

| Improvement destination | Evidence-based proposal | Acceptance measurement |
| --- | --- | --- |
| Skill directive | Keep permuter score, asm-diff numerator/denominator, byte size, first mismatch, semantic validity and byte-match result as separate fields; stop only with a complete terminal report | Every attempt joins to candidate/source hash and compiler profile; invalid use-before-definition candidates never advance; exact status requires live byte-match and required independent review |
| Reference material | Record the `do-while(0)` grouping as a scheduling experiment, not a general rule; keep scheduler cause unresolved until matched controls reproduce it | Reproduce on comparable GCC 2.7 scheduling-sensitive blocks with unchanged semantics, pinned inputs and compiler flags; report score, size, residual and byte identity |
| Harness tooling | Bind diff artifacts to source/build/object hashes and flags; validate the terminal-report schema; emit display ranges with block indices/hashes and refuse receipt writes on any mismatch | Stale outputs are rejected; malformed/missing reports fail visibly; shuffled, clipped or repeated ranges cannot create false review credit, with complete source-origin reconciliation |

### 2026-09-27 `area026` cohort: throughput and target reconciliation

The [screening receipt](../../tmp/observation-ingest/bof3-re-area026-four-run-screening-receipt.jsonl)
and [cohort manifest](../../tmp/observation-ingest/bof3-re-explicit-20260728-pair-cohort.json)
cover four pinned writer/reviewer transcripts: **198 unique blocks / 159,418
characters**, with **421 origin checks across 13 files**. The reports contain two
writer `exact` results and two reviewer `pass` results, but target identity joins
only one pair: `0x801F3174`. Writer `0x801F319C` has no selected review, and the
review for `0x801F3338` has no selected writer. Only **one of two writer targets
(50%)** had a selected review. The prepared manifest's two-pair claim was corrected; these
report counts do not mean two accepted pairs.

Pinned `tool_start`/`tool_end` events yield **81 calls** and **277.078 summed
first-tool-to-last-tool seconds** across the four runs: writers used 51 calls /
191.562 seconds, reviewers 30 / 85.516. Three raw error flags do not map one-to-one
to domain failures. The sole matched pair used 48 calls over **197.782 seconds**,
or **18.20 reported exact/pass pairs per first-tool-span hour** (n=1). Dispatch
and setup are excluded; this is not a population rate or accepted-lift throughput.
Metadata sidecars
were excluded after hash instability. Stale Rizin snapshots and invalid
`rev-query` options limited some live duplicate/call evidence.

Body screening ran for **621 seconds** (**19.13 blocks/minute; 15,402.7
characters/minute**), including display recovery and excluding post-screen
reconciliation, ledger work and checkpointing; active-review time is unknown.
The size distribution was median **74**, p90 **3,052**, p95 **4,231**, maximum
**11,898** characters. Sixteen display calls covered 201 block instances, with
three repeats. A compact **97-block / 14,845-character** batch fit in one call;
**16 / 20,933** also fit, while **11 / 21,822** clipped and needed two bounded
range reads. Four Markdown-reader format errors were recovered. This supports
raising batch counts for compact bodies while sizing each call by remaining
context/output budget, not a fixed block or character cap. Record blocks and
characters per display, clipping, rereads and active versus operational time.

Coverage remains **19,894/379,196 (5.246363%)**, with **359,302 blocks** and
**1,214,455,686 characters** pending. At this sample's operational rate, simple
homogeneous projections are **313.04 hours / 13.04 days by blocks** and
**1,314.12 hours / 54.75 days by characters**. The sample averages 805 characters
per block versus 3,380 in the pending corpus (4.2× larger), so neither is a
corpus ETA; use both rates and content mix for planning. This cohort remains
screened but uncheckpointed and receives **zero coverage credit** because the
same database already has a persistent `database is locked` failure. Future
handoffs should join reports by exact target and measure paired-review coverage
plus orphan writer/reviewer counts before deriving accepted exacts/hour. These
observations suggest batching and pairing improvements; they do not establish a
skill speedup or change its directives.

### 2026-08-11 area01613 child workflow: non-exact checkpoint and ABI ownership gap

The parent session `.pi-subagents/sessions/r5d/area01613/run-0/session.jsonl`
(SHA-256 `bd5fec95cf5c8f5e28fe293bc564069123ced13d32a66ba374bd06be42649994`,
session `019ff27e-c691-7586-841a-f6b5074356b4`) reports work on
`emi/world00/area016/13@0x801F3B00`. Its baseline was **190/243 instructions
(78.19%)**, **972→968 bytes**, first mismatch **+0x28**. The best reported
candidate reached **193/243 (79.42%)**, **972→964 bytes**, with the same first
mismatch; subsequent probes regressed or failed to improve it, and the best
checkpoint was restored. Final status remained escalated/review-pending with
`parent_restore_required=true`; exact acceptance was **0/1 target**. A green
symbol-map check did not establish ownership for the changed
`func_80166CB0` return ABI; review still found that evidence missing.

Two tool-friction classes were recorded for this target: stale reverse
snapshot/index blocked Rizin evidence, and the first flag-search attempt lacked
`compile_commands.json`; a repository-native configure later generated it.
These are blocked probes, not source-level failures. The parent result contains
122 unique tool-output blocks reviewed, but those are evidence units rather
than executed tool calls. Child-level call counts and dispatch-to-terminal time
remain unreconciled, so mission cost and accepted throughput are unmeasured.
Candidate improvement: require fresh target-qualified callee/caller ownership
evidence before an ABI declaration change, and expose stale-index and missing
compile-database preflight states separately. Measure declaration changes
without ownership proof, blocked probes by cause, retained exact/partial outcomes,
and review acceptance before changing guidance or harness behavior.

### 2026-08-11 nested cleanup and review: writer claim blocked independently

The nested writer session
`.pi-subagents/sessions/r5d/area01613/run-0/session/043e9261/run-0/session.jsonl`
(SHA-256 `87e1598f4131298b49a7317051bb70e4bb76343d32e4ed47fa7408d6352221`,
session `019ff28c-231a-7810-abf8-3222430e8d2c`) renamed
`func_801F3B00` → `drawArea016MarkerPanel`. Its report claimed spelling-only
acceptance **1/1** and showed the live partial at **193/243 instructions
(79.42%)**, **972→964 bytes**, first mismatch **+0x28**. The target build, symbol
and Splat checks, live `asm-diff`, checkpoint capture, whitespace check and
affected-target snapshot check passed. The receipt listed **8/10** command
outcomes as passed; byte-match was an expected non-exact result, while
repository-wide index status was blocked by stale snapshots for two unrelated
battle targets. The affected target snapshot was fresh.

Independent review in
`.pi-subagents/sessions/r5d/area01613/run-0/session/e489b3e2/run-0/session.jsonl`
(SHA-256 `b0a2e2388c1c94e9ad5cfa8bba370fea330a4e38b98a2c5d535e7a82ad411864`,
session `019ff28d-bded-7ad2-a877-3a81283f8043`) returned **block**,
`repairable=true`, and found two blockers: the candidate added `s32 offset` and
rewrote both marker loops, so it was not a body-preserving rename; and
`func_80166CB0` changed from `s8` to `s32` without target-local ownership or ABI
evidence. It also found stale `@match 78.19` metadata against the live **79.42%**
result. The old spelling in `config/symbol-naming-baseline.json` was classified
as debt; target map, Splat, manifest, declaration, caller, definition and path
were otherwise integrated consistently. Overall cleanup acceptance is **0/1**
pending repair, while the reviewer completed its blocker-detection task **1/1**.
The lift remained non-exact; no accepted exact lift resulted.

Writer cost was **16 Bash calls / 104.264 seconds** of first-to-last event span
(5/16 tool results flagged errors); reviewer cost was **19 calls** (12 Bash,
7 grep) / **103.888 seconds** (2/19 flagged errors from expected non-exact
comparison checks). Together this is **35 calls** over **208.152 seconds of
summed event spans** (**209.059 seconds** first-to-last wall span) for one writer
and one reviewer, with zero accepted rename transactions. The false acceptance
report despite passing target gates is a measured improvement lead: compare
before/after source bodies and external ABI declarations directly, and refresh
`@match` from the live result. For a matched cleanup cohort, measure body/ABI
deviations accepted, stale match metadata, review blocks, calls and event span;
do not divide by accepted transactions while the accepted denominator is zero.
Keep target-local gates separate from unrelated global-index status and raw
tool-result error flags.

### 2026-08-11 nested compiler-rung follow-up: no profile improvement

Child transcript
.pi-subagents/sessions/r5d/area01613/run-0/session/ba881954/run-0/session.jsonl
(SHA-256 81240beca526c8a91a58a8afeb150b6fa00a40e1500c1bc9235caf9793b9cc06)
continued emi/world00/area016/13@0x801F3B00. Its final report preserved the
inherited partial at **193/243 instructions (79.42%), 964 versus 972 bytes,
first mismatch +0x28** and returned escalated with
parent_restore_required=true; exact acceptance remains **0/1**. No source
change was made in this continuation.

The canonical and four installed historical compiler matrices each tested 52
variants: **260 candidate results, 246 different, 14 compile errors, zero exact**.
Best scores were canonical **79.42%**, GCC 2.6.3 **78.60%**, GCC 2.8.0/2.8.1
**75.72%** each, and GCC 2.95.2 **48.56%**. The canonical matrix took **3.662s**;
the four historical searches took **15.265s** combined. Across the run, 23
started tool calls (16 Bash, 3 grep, 2 read, 1 ls, 1 find) spanned **156.736s**;
paired command execution summed to **34.208s**. Four Bash error flags were
expected non-exact results from asm-diff, flag-search and byte-match, not
infrastructure failures. The transcript has no recorded toolCount or durationMs,
so these are event-derived costs. Zero accepted units means accepted-throughput
is undefined; do not infer exhaustion of clean-C search or make profile search
mandatory from this one continuation.

One setup command passed CMAKE_EXPORT_COMPILE_COMMANDS=ON, which CMake reported
unused; the repository-native compile-database generator then succeeded. This
repeats the missing-database friction already seen in the parent case, with a
specific wasted setup path. Candidate harness improvement: have flag-search
preflight the repository-native database generator or return that exact remedy.
Measure missing-database starts and recovery calls/time per run, with zero
change to the compiler profile or source and the same live byte-match gate.

### 2026-08-11 nested area01613 review and cleanup cohort

Ten pinned child transcripts continued the same
`emi/world00/area016/13@0x801F3B00` partial. They cover **one distinct target
function**, not ten completed lifts. The best remained **193/243 instructions
(79.42%), 964/972 bytes (-8), first mismatch +0x28**; exact acceptance is
**0/1**. Child report verdicts were **5 pass, 3 needs-fix, 1 escalated and 1
block**. Three later checkpoint attempts reported no change from the 79.42%
best. A bounded permuter rerun reached its 60-second limit at iteration 2,571
with best score 1,290, 110 compiler errors and 18 permuter failures; it did not
improve the candidate. The prior compiler matrix still records 260 variants,
zero exact results.

The source transcripts contain no run-metadata sidecars, so these costs are
event-derived. The ten runs used **83 tool calls** (67 Bash, 13 grep, 2 edit,
1 read); median calls/run **7.5**, nearest-rank p90 **14**, maximum **16**.
Summed child event spans were **685.563 seconds** (median **50.248s**, p90
**98.752s**, maximum **172.662s**). Thirteen of 83 tool results carried a raw
`isError` flag; this is not a failure rate because expected non-exact comparison
exits and genuine preview/prerequisite blockers are mixed. Event usage fields
summed to 1,645,816 total tokens (463,436 input, 1,157,632 cache-read, 24,748
output); these totals describe this small repeated-review cohort, not a general
per-lift cost.

| Child source (JSONL SHA-256) | Calls / event span | Reported result |
| --- | ---: | --- |
| [09edd13d](../../.pi-subagents/sessions/r5d/area01613/run-0/session/09edd13d/run-0/session.jsonl) `872c9ffc68776e6674c0bafd7d8ed6598267fd8b24883052b5492d8af686eb30` | 14 / 172.662s | Escalated; 60s permuter rerun did not improve the 79.42% best. |
| [2a403b0f](../../.pi-subagents/sessions/r5d/area01613/run-0/session/2a403b0f/run-0/session.jsonl) `37e846d6e9cea976cf3ccacdd9328ebd4062bc91517527e3ac0fca119c39953` | 4 / 42.947s | Attempt 2 tied the best; improvement gate refused to record it. |
| [2b936229](../../.pi-subagents/sessions/r5d/area01613/run-0/session/2b936229/run-0/session.jsonl) `07579895369fce1a7b0769e76b742bd8e6e48235e21c46c122d4b5c3e2c750d5` | 6 / 47.195s | Attempt 1 checkpoint captured; candidate remained non-exact. |
| [4bbb8c76](../../.pi-subagents/sessions/r5d/area01613/run-0/session/4bbb8c76/run-0/session.jsonl) `9e71c78aee1a64f7b20a2debd4d23790db77adc8785b61c300382f387c81418d` | 7 / 53.354s | Attempt 3 verified the retained best; no source change. |
| [7d037ffc](../../.pi-subagents/sessions/r5d/area01613/run-0/session/7d037ffc/run-0/session.jsonl) `cde0094f9b66d59340395002210082129c8e71709dc4fac54e1684aceb42ce4e` | 8 / 50.745s | Attempt 1 restore validated; partial still had the 8-byte residual. |
| [c3cd9546](../../.pi-subagents/sessions/r5d/area01613/run-0/session/c3cd9546/run-0/session.jsonl) `343900bcb996c765196c84d81626b750676d77749a518b2deefe2eedc334bd94` | 4 / 39.745s | Restore returned 0 after fatal path diagnostics; worktree path set changed. |
| [c4b9c92f](../../.pi-subagents/sessions/r5d/area01613/run-0/session/c4b9c92f/run-0/session.jsonl) `0db38c4120aed64bba5bcff1617dc5027bb90b55c986cf020234683581726f40` | 16 / 98.752s | Needs-fix; required compiler-profile rung lacked `compile_commands.json`. |
| [e5831fe1](../../.pi-subagents/sessions/r5d/area01613/run-0/session/e5831fe1/run-0/session.jsonl) `07936f82ff0317a3ca958e268f1b1e4718e9873554b58bcfcc9448a01ba77539` | 3 / 44.188s | Attempt 4 tied the best; source `@match` remained stale at 78.19. |
| [e91c9844](../../.pi-subagents/sessions/r5d/area01613/run-0/session/e91c9844/run-0/session.jsonl) `3d0f70765d226e14b0bd5a3c296acd8bd7c9251c3ba3cdc74049bf9d2abeb67e` | 9 / 49.750s | Compiler ladder marked exhausted; scratch preview rejected ignored PsyQ declarations. |
| [f2225726](../../.pi-subagents/sessions/r5d/area01613/run-0/session/f2225726/run-0/session.jsonl) `5afb1f246a9f0ac091b601c780179b7ea689db227d77e258e9ec863fd96589f0` | 12 / 86.225s | Cleanup blocked: changed `func_80166CB0` ABI lacked target-local map/ownership evidence. |

The corresponding 98-block ingest window reviewed **825,660 serialized
characters in 806.711 active seconds**: **7.289 blocks/minute** and **61,409.3
serialized characters/minute**. Fourteen oversized outputs held **716,827
characters**. Exact alignment against the reviewed parent and earlier child
outputs found at least **638,361 repeated characters** in those large blocks;
because the matcher can miss additional repeats, the **78,466 unmatched
characters** are an upper bound, not a count of semantic novelty. Including all
108,833 characters from the smaller blocks gives an alignment-assisted review
volume upper bound of **187,299 characters** (**13,930.6/minute**). Three
same-record truncation bodies were exact prefixes of their enclosing outputs,
repeating **153,477 characters** in total. This cohort is a context-heavy
sensitivity, not a replacement for the longer clean planning rate.

Measured improvement proposals:

- Ingestion: detect exact same-record truncation prefixes and index their body
  once while retaining both source pointers, hashes and clipping trailers. This
  window contains three such pairs / 153,477 repeated characters. Acceptance:
  preserve all 98 source-block identities and truncation metadata, report
  serialized versus unique-body characters, and show zero omitted or changed
  source bytes.
- Reader harness: emit numbered, redacted chunks with hashes and continuation
  offsets for blocks above the 26,000-character display limit instead of
  rejecting a whole-batch read. Measure safe-display rejections and repeated
  reader calls per oversized block; acceptance is zero rejected batches and
  complete contiguous coverage of every block.
- Restore harness: preflight dirty paths against the selected checkpoint's
  recorded paths, refuse to touch unowned changes, and return nonzero when a
  requested checkout path fails. In `c3cd9546`, the transcript shows several
  modified/deleted/untracked paths before restore, fatal checkout diagnostics,
  exit 0, and only the target source modified afterward; ownership of every
  removed path is not established by this transcript. Acceptance is zero
  collateral path changes and before/after status verification.
- Retry orchestration: compare source hash, selector and live metric before
  repeating producer-side capture. Attempts 2–4 were unchanged across three
  children (14 calls / 140.489 summed event-span seconds); keep independent
  review and restore gates. Acceptance is fewer unchanged capture calls with
  checkpoint identity and review coverage unchanged.
- Preflight: make missing compile-database and ignored PsyQ declaration closure
  explicit before flag-search or scratch preview. This cohort records the
  missing `compile_commands.json` prerequisite and preview rejection of
  `DR_MODE`/`SetDrawMode`. Measure blocked starts and recovery calls/time while
  preserving source/profile and live byte-match gates.

## Measured shapes that flipped a partial to exact

| Symptom (first-difference class) | Shape that fixed it | Measured delta |
| --- | --- | --- |
| Scratchpad cell `0x1F800044` CSEd into a callee-saved register (`lui+ori`, `lw 0(s0)`) instead of a fresh folded `lui+lw` per access | target-local named symbol (`extern u8* D_1F800044;` + map row + `WEAK_SYMBOL_AT`), read directly; raw-constant/`SPAD_PTR_SLOT`/table/volatile-cast forms all defeat it | 72.97→100 (sce10eff), 61.90→100 (area028), 81.82→100 (area024), 55.88→100 (area028) |
| Extra `andi` after a narrow load + merged/relabelled arms | read the byte through a non-volatile view; keep per-arm stores instead of one hoisted label local | 47.62→100 (commu00) |
| Repeated `table[i].field` emits `%lo(sym)(at)` per store instead of reusing the published address | one function-scope pointer local (`record = &table[i]; record->f = v;`) | 91.30→100 (game/00), 86.67→100 (area027) |
| Frame/prologue collapsed, address not kept in `$s0` | take the address once into a local pointer (`u8 *state = &D_...;`) and read/store through it | 52.17→100 (scena00) |
| `move a1,zero` emitted before `li a0,K` | declaration initializer (`s32 count = 0;`) instead of a body assignment | 96.30→100 (area030/04) |
| Scaled result path in the wrong arm (`bnez` vs `beqz`) | invert to an early return so the scaled path is the fall-through | 88.46→100 (area030/04) |
| Divergence at a `div` site; `break 7`/`break 6` absent from the object | object profile `set(BOF3_OBJFLAGS_<sanitized> -O2 -Wa,--expand-div)` in `config/compiler/object-flags.cmake` (opt-in rung) | Same final C: 35 instructions/140 B → exact 44/176 B; the earlier 30/44 seed also needed a control-flow change ([controlled comparison](#division-profile-proof-separated-from-control-flow)) |
| Sole commutative `addu dst,dst,sub` vs `addu dst,sub,dst` after a base/index split | derive the destination twice (`base+row`, then `base+sub+row`) with an adjacent rationale comment | 97.73→100 (game/00) |
| Call-argument masks `andi 0xFFFF` / sign-extension `sll;sra` missing or extra | match the callee/parameter width to the original (`u16`/`s16` params, `u8` fifth arg read by `lbu`) | enables exact (area030/04, area032) |
| Polling loop preheader lacks the duplicated constant and backward-branch load | bottom-tested `do { } while (call() == 0);` | first-seed exact (exe/logo) |
| Wrapper republishes a record from a `u8[]` table whose entries are 0x118 bytes | reuse the exact sibling shape: `base + ((index * 36u - index) * 8u)` on the `u8` param, cast the table to the record type | first-seed 100 (battle03, 25/25, 100/100) |
| Residual `+0x0000` frame/noise from a volatile byte read widened for arithmetic | hoist each table offset into a `u32` local before the record store; express the store as a declared volatile access (`((volatile s16*)BASE)[2]`); keep the then-arm reset offset inline | 69.49→100 (battle/15) |
| `andi`/frame noise when a volatile byte feeds a subtraction | non-volatile local pointer for the arithmetic, volatile access for the store | 82.22→100 (exe/logo), 75.76→100 (area016) |
| Head/allocator permutation + load-delay `nop`, wrong branch-delay slot | single-expression `abs()` for the delta; express the gate as the success path (`range >= |delta|`) with `return 0;` last | 35→86→100 (game/00) |
| Final-call argument emits `lhu`+`sll`+`sra` around the `jal` instead of a direct signed `lh` | read the argument through a block-local non-volatile pointer (sibling idiom) | 91.30→100 (area016) |
| Single 0xCC/constant store duplicated because two arms each stored it | one disjunction (`(A&&B)\|\|(C&&D)`) with exactly one shared store statement | 79.66→100 (commu00) |
| Per-arm constant folded by the compiler (`sltiu` range test) | read the compared byte per arm into a word local; keep the compared constant in a `u8` local | 60/79→79/79 (battle/03) |
| Extra `andi 0xFFFF` zero-extension after the `lhu` of a plain-RAM `u16` global | declare it `extern u16` (no unjustified `volatile`); the volatile load stays HImode and forces manual promotion | 66.67→68.75 (area016) |
| Three consecutive stores through a pointer local re-assigned from the published cell reuse the still-live read and hoist a duplicate `li` | write the store block directly through the published symbol (`D_...->field = v;`) | 68.75→100, 192/192 (area016) |
| Volatile intermediate store of a shared work pair dead-store-eliminated | wrap ONLY that store: `*(volatile u8*)&pair = x;`; both volatile leaves a `nop` delay slot | 97.73→100 (battle03@801E347C) |
| Split-symbol table bytes; one array with a `+1` addend forces an 8-byte frame | bind the two bytes as two symbols → restores `vars=0` and the 0x18 frame | 64.44→100 (battle03@801E1BA8) |
| Cell read then re-read: constant-address macro materializes the base into a callee-saved register and CSEs | use the existing named symbol → fresh folded `lui/lw` per access | 76.74→100 (battle03@801E320C), 62.50→100 (shop@801E2EB0) |
| Store cannot be stuffed into a `j` delay slot through a volatile pointee view | `scratch = D_1F800044; scratch->mode = 1;` (non-volatile reloaded pointer local) | 88.10→100 (area016@801F3360) |
| Constant differs from a sibling but the body is identical | constant-store sibling mirror: replace `x = task->field` with the literal, keep the sibling's statement order | first-seed exact (commu00 0x801F1538/15B8/15F8) |
| Hoisting an explicit offset variable changes the suffix register | keep `(s16)(x + K + i * stride)` inline | first-seed 100 (area030@801D7784) |
| Symmetric loop-carried values of equal ref count | put the value defined last in RTL order first; use declaration initializers, not body assignments | 76.36→100 (area024@14@801F362C) |
| Array-typed extern forces base-register addressing (`lui+addiu`, offset 0) | scalar-typed extern at the same address folds per access (`lui/lb %lo`); a second target-map name at a mapped address is a collision, so use an address-derived raw alias | 70→100 (commu00@801F1824) |
| Struct padding length wrong | widen padding to `target_offset - next_free_offset`, not `target_offset - last_named_field` | 93.10→100 (area008@801F31D0) |
| Arms differ in delay-slot store constraint | non-volatile reloaded pointer local for the arm sinking into `j`; volatile published view for the fall-through arm | 85.86→100 (area016@801F2EEC) |
| Cursor must be read before the countdown store | read the cursor first; lets the scheduler fill the `lhu` load-delay slot and keep the record in the argument register | 67.74→100 (area032/13@801F378C) |
| Two adjacent array externs make gcc hoist `lui+addiu` bases per array | model the record as ONE struct array (0x98 stride) with one extern → folded `lui/addu/lbu %lo` | 60→100 (battle15@800AC7D0) |
| Scaled inline index inside a loop hoists the symbol address (`la`) | bind the scaled byte offset to a local first → `lui/addu/lbu %lo(sym)(at)` | 89.71→100 (sisyou@801D3148) |
| Masked equality pair plus an unmasked range test | two separate plain ifs; a combined expression if-converts | first-seed exact (game00@801B5EBC) |
| Multiset-identical instruction stream except the original's `nop`s | residual is entry-block load-delay filling/allocation; `volatile` tricks "fix" order only by degrading load forms — not a valid candidate | documented partial (area016@801F42F8) |
| Call constants set up inside each arm while a live address pointer sits in a caller-saved arg register | duplicate the identical call in BOTH arms; gcc tail-merges it (one shared call re-allocates the pointer to `$a0`) | first-seed (battle03@801E6420), 60→100 (area008@801F3BB4) |
| Hoisted invariant loop bound merges with the guard address as `move t0,v1` | re-derive the loop bound address into a body pointer local; `loop_count = count;` is propagated away | 86.67→100 (battle15@800AD5C8) |
| `/n` element count emits `mult/mfhi` | `(T*)a - (T*)b` emits gcc `EXACT_DIV_EXPR` (magic multiply, no `mult`) | first-seed exact (area032/13@801F3C18) |
| Single `sll` + two `lui/addu/lbu %at` pairs + early `mult/mflo` with the store in the `jal` delay slot | scaled index in a narrow local + one left-to-right address expression | first-seed exact (area016@801F4610) |
| Value live range too short so the symbol address and value swap registers | in-place local + deferred store: `m = SYM[0]; m -= K; SYM[0] = m;` | 74.14→100 (game00@801990D0) |
| Reusing one `u8` local for the pre-call broadcast byte and the post-call incremented byte forces the pointer into `$v0` | give the incremented byte its own temporary | 67.57→100 (battle03@801E69C0) |
| Two clamp stores need one shared constant register | re-read the just-stored clamp value through the pointer | game00@801990D0 (2nd lever) |
| Four-statement copied-word group re-reads the index per statement | one cursor local for the copy group, keeping per-statement index reads | first-seed exact (area032/13@801F3A78, 104/104) |
| Loop bound loaded late (sample lands in `$v0`), two `sched2` placements left | word-typed temporary: `s32 ref = table[0x0F];` instead of the inline byte expression | 95→100 (area030/04@801E0680) |
| Shared tail block reached by two arms with the same (compare, return) semantics | branch polarity / return order: `if (equal) return value; return constant;` (not the negated form) | 80→90→100 (game/00@801BA278); scena00@801F7350 |
| Volatile store ordering displaces a call-argument constant into the sub-step load-delay slot | plain non-volatile view of the volatile store (`u8* p = (u8*)&volatile_var; *p = v;`) keeps the constant adjacent to the closing `jal` | 89→100 (shop/00@801E2084) |
| Three published-cell loads become local-pointer copies instead of fresh folded `lui+lw` | direct global-cell spelling `*(s32 *)(D_1F800044 + 0x34) = 0xB0000;` lets gcc-2.7 CSE the three loads exactly | 57→100 (scena00@801F7484) |
| Extra `andi` argument conversion + record-view mismatch | non-volatile record view `work = (Battle03LocalWork*)(void*)D_801EB4E0;` (ordinary C cast dropping the pointee volatile) | 89→100 (battle/03@801E6D28) |
| Three descending stores of one constant emit three address computations | chained assignment (`a = b = c = 0;`) computes all LHS addresses before any store, so one pointer load feeds the descending sequence | first-seed exact (scena00@801F7264) |
| Cursor-word copies re-emit loads | copy through the reviewed struct fields (`Scena00WorkCell`/`Scena00Cursor`), not raw `s32` casts | 82→100 (scena00@801F6E48) |
| Same helper called in two passes with swapped argument order | read each call's argument order from its live arg image; keep one call per arm | 43→100 (scena00@801F7670) |
| 0x1C-byte slot-record stride | `(idx << 3) - idx; << 2` on a `u8` index | first-seed exact (game/00@8019A1D4) |
| Byte stores invalidate the cached cell while word/halfword stores do not | per-statement published-symbol writes emit a fresh `lui+lw` after every byte store; also the tint channels are signed (`= -1`, `li v0,-1`) not `u8 0xFF` (`li 255`) | 44→100 (scena00@801F7A5C) |
| Loop-control flag materialised as `move v0,s0; bnez v0` | 16-bit control flag (hint from the emitted `move`) — but then the copy schedules between the tail stores | 93/97 (scena00@801F74EC partial) |
| Countdown advance shifts the branch targets | advance via a separate re-read of the pointer cell after the call, with a promoted local for the compare (a compound assignment reloads the cell) | first-seed exact (scena00@801F7B0C) |
| Call-site `andi` vs `sra` for a callee's parameter widths | file-local 16-bit callee prototype declaration (the byte-width form provably breaks the call site) | first-seed exact (game/00@801A0348) |
| Loop head lands in the wrong place | redundant pre-branch store (`interp.y = cur.y + 0x8000;` before the `if`) moves the whole loop head | 49 mismatches → 93/97 (scena00@801F74EC) |
| 0x38 constant-wrapper family member | sibling mirror with one literal (`0x66/0x65/0x62/0x55/0x5D/0x57/0x60/0x68` at fixed offsets) | first-seed exact (battle/15 ×3) |
| Narrow signed local needed for `move v0,vX; addiu v0,v0,-N` plus an extra 8-byte frame; an `s32` local gives one `addiu` and a 24-byte frame, cascading into an `a0`/`a1` swap | declare the narrow signed local (`s8 fade;`) and mirror the in-family sibling | 72.31→100 (scena00@801F882C) |
| Raw-constant slot macros (`0x801ec339u + idx*0x78`) materialise the base (`lui`+`ori`) and index it | spell the slot through the named struct array field (`D_801EC330[idx].flag_09`) for the symbol-relative `%hi/%lo` fold; `((s8)u8_field) << 9` emits `lb; sll 9` where a volatile-u8 view emits `lbu; sll 24; sra 15` | 49.61→100 (battle/03@801E3638) |
| Halfword store between four leading cell accesses allocates the published pointer to `$a0` instead of `$v1` | spell `D_1F800044` directly at each leading access (gcc-2.7 CSE keeps one load in `$v1`); byte stores still invalidate, so a later fade arm keeps one fresh load per statement | 88.41→100 (scena00@801F8AB8) |
| `sltiu` emitted where the original has `slti` | make the compared word signed (`s32`) instead of `u32` | 97.67→100 (scena00@801F82B4) |
| Handler seeds a scratch record then accumulates an origin delta; `u8* work = D_1F800044;` binds the pointer to the wrong register | keep the `u8* work` local for the copy group, spell the accumulate tail through the published cell | first-seed exact (scena00@801F89F0) |

## Residual classes (ranked by frequency)

| # | Class | Usual cause |
| --- | --- | --- |
| 1 | Register allocation/permutation (`a0`/`a1` swap) | often a ceiling; record it |
| 2 | Address materialization/CSE | fixed by named symbols or pointer locals |
| 3 | Delay-slot fill / scheduling | branch-local return, per-arm stores, statement order |
| 4 | Commutative operand order | duplicated derivation |
| 5 | Frame/spill differences | consequence of 1–2 |
| 6 | Clamp/mask width | declared parameter and field widths |

## Turn-efficiency policy and fan-out rules

The turn-efficiency and fan-out rules this file used to own now live with the mission protocol that
enforces them: [bof3-lift-loop's ledger](bof3-lift-loop.md). Lift-specific selection policy stays
below.

## Ranked selection when the index is stale

`bin/harness analysis query quick-wins` refuses with `stale Rizin snapshot recipe` while unrelated dirty target configs change the replay hash (the parent owns the refresh): do not stop and do not rebuild analysis. Rank from tracked evidence — the target's Splat `asm` boundaries minus every address already claimed by an `@source` tag, ordered by (resolved calls, decisions, instructions), with `div`/`break` byte screening. `bin/harness lift asm-diff`, `bin/harness lift byte-match`, `bin/harness source symbols check` and `bin/harness source splat` all keep working while the snapshot is stale.

## Expensive rungs are opt-in (never a mandatory ladder step)

OPTIONAL — `bin/harness lift flag-search`, `bin/harness build variants`, the permuter and any
per-object compiler profile run only when the user or the parent mission explicitly
authorizes them **for that selector**. The default ladder stops at clean-C shapes
(types/widths, control flow, expression order, temporaries,
volatility/representation views) and, when exhausted, reports a documented partial
with the smallest missing evidence.

## Recording rule

Measurements land in this file; directives land in the skill and its references. After a mission add
one audit row under [the central folder's rule](INDEX.md) — audit tool calls, wall time and method,
then converge the finding into a directive. Never put this prose in lifted source; source carries only
the mandatory metadata tags. Absorbed from the legacy `OBSERVATIONS.md` recording rule, which
duplicated this one per skill.

## Retained report recovered through a parent search

Parent record 1593 reprinted parts of child `415bf77b-d6e2-4bca-b991-4185d65d0004`
(transcript records 272–288), including its complete final report. For
`emi/battle/battle/15@0x800AD4D4`, the writer reported **49/61→61/61 instructions**
(80.33%→100%, 244 bytes) after giving the loop's per-iteration scratchpad-base
reload a separate local. Of four reported frontier variants, one reached exact
and three stayed at 80.33%; the successful variant was retained. This is an
experiment result from one writer report, not independently accepted throughput;
the complete child transcript and review remain pending.

The same report records one bare-target context usage error before the qualified
selector succeeded. Its selection also depended on an unresolved interpretation
of the parent's “4-mod-8 jump-table” exclusion; reaching an exact match did not
settle that scope question. Proposed mission reference: give the fully qualified
selector and an executable or explicit exclusion predicate before dispatch.
Measure invalid prefill calls and selector disputes per comparable mission while
retaining the native identity gate and independent review. The writer correctly
returned its reusable lesson to the parent instead of editing out-of-scope docs.

## First campaign writer partial report

Parent `01a0c062`, record **1026**, retained the complete report from writer
`c8df397d-a0df-4d4a-a31f-c4934a3bdfed`, explicitly dispatched to `$bof3-re` at 990
for `emi/etc/game/00@0x801C3154`. This verifies mission attribution and the delivered
proposal, not the complete child reasoning/tool history or independent acceptance.
The child's artifact transcript is
`.pi/sessions/subagent-artifacts/c8df397d-a0df-4d4a-a31f-c4934a3bdfed_bof3-lifter_transcript.jsonl`;
its full review remains pending. Parent costs belong to the
[supervision cohort](bof3-lift-loop.md#first-lift-dispatch-and-asynchronous-supervision).

| Writer-reported experiment | Outcome | Evidence limit |
| --- | --- | --- |
| Initial named-local seed | 35/44 instructions, 79.55% | Proposed candidate, no independent check here |
| Direct `D_80146250[0x12C]` head | 38/44, 86.36% | Changed head improved measured report score |
| Split address statements | 36/44, 81.82% | Reported desired order but mirrored v0/v1 allocation |
| `base+sub+row` with named locals | **43/44, 97.73%**, 176→176 bytes | Best retained candidate, first difference `+0x0050` |
| Further spellings/order/type variants | Same residual or 36/44 | No reported exact result |
| Permuter | No improvement/result candidate emitted | Time-cap propagation and worker cleanup are report claims pending raw verification |

The report correctly made **no exact claim**: `byte-match` reportedly exited 1.
Equal byte length and 97.73% instruction matching are not byte identity. Its
“two-instruction scheduling residual” refers to an `andi`/`sll` pair moved around
an `addu`; it is not a byte-error count or proof of semantic equivalence by score.
The load model was explicit: `0x801C3154−0x80195800=0x2D954` (186,708), a 176-byte
payload range reportedly matching original bytes. Four paths changed: a new UI
source and target-local Splat, symbols and manifest; the internal header remained
untouched. Reported metadata included partial/match/residual tags and a requeue
request for RTL evidence or a genuinely capped permuter run.

The report supplied eight ordered attempt rows against a mission allowance of
three attempts/two repairs, without a consumption mapping. It also reported that
`--time-limit 60 -j 4` was not forwarded to the underlying permuter, which ran
about 16 minutes before four workers were terminated. Neither its no-process-left
claim nor its scope/policy claims have been independently established by this
parent excerpt. A filtered dirty-status listing contained two other battle
changes; without a captured baseline those cannot be attributed to this writer.

At 1027 the parent requested a distinct read-only reviewer to reproduce the gates,
check metadata/load/scope and assess the residual; reviewer launch completed at
1028. The queue was labeled partial at 1029 while review remained pending.
Proposed **mission reference**: define attempts and repairs in terms of recorded
experiments, distinguish score/length/byte identity, and list required manifest
writes before dispatch. Proposed **tooling investigation**: trace time limits to
the actual worker process and verify descendant termination. Acceptance requires
reproduced measurements, an accounted original budget and independent review;
this observation makes no new acceptance or tooling-defect confirmation claim.
The [subsequent repair and review](#first-campaign-review-repair-and-exact-acceptance)
supersede the partial outcome and challenge the missing-timeout diagnosis.

## First campaign review, repair and exact acceptance

Parent `01a0c062`, records **1030–1047**, closes the preceding proposal for
`emi/etc/game/00@0x801C3154`. Complete embedded reports were reviewed at 1032,
1037 and 1044, together with parent commands/results. Full child reasoning and
tool-result review remains pending. Pinned source paths, hashes, event counts and
metadata fields are in `tmp/observation-ingest/older-first-lift-acceptance-metrics.json`.

| Stage / child run | Started tool calls | Observed event span, s | Domain result |
| --- | --- | --- | --- |
| Initial writer `c8df397d` | 80 | 1,810.925 | 43/44 partial proposed |
| First review `7ab017d6` | 93 | 526.799 | Repair required: two findings |
| Repair writer `703fe2be` | 59 | 537.925 | 44/44 exact proposed |
| Re-review `a1f16485` | 45 | 310.677 | Pass; parent subsequently recorded accepted-exact |

These four linked runs used **277 calls / 3,186.326 summed observed seconds**:
writers 139 calls/2,348.850 seconds; reviewers 138/837.476. Recorded `toolCount`
and `durationMs` are missing in all four; event-derived values do not fill those
fields. All use recorded model `ninerouter/ds-combo:high`. Their 81/78/48/36
reported turns differ from call counts. Seven error-end events require raw-result
classification; they are not seven domain failures. This is **one function**,
with **0/1 first-pass review acceptance**, one repair mission and eventual parent
accepted-exact status—not four completed functions or a campaign success rate.

The first reviewer reproduced the honest 43/44, 176-byte partial and independently
reported the disc→EMI→payload identity chain, but found:

- Fixed RAM used raw casts instead of `PSX_PTR`; the measured correction was
  byte-neutral.
- A split-base candidate preserved 43/44 and 176 bytes while moving the first
  mismatch **+0x50→+0x5C**. Placement was repaired; only commutative `addu` operand
  order remained. The retained candidate therefore was not the best frontier,
  and its claimed clean-C exhaustion was refuted. Score alone missed this gain.

The repair adopted both findings. Its report describes five neutral probes, then
an exact form: derive destination as `base+row`, followed by `base+sub+row`.
Compiler reuse preserves the first add's placement and emits the required outer
operand order. The documented duplicated-assignment `MATCHING_AID` was judged
allowed under the historical contract; its name alone did not establish that.
The permuter was not needed. Parent gates at 1039 independently returned
**44/44, 176→176 bytes, first=-**, with both diff and byte-match exit zero.
Re-review reported identical linked bytes and original-media window, hash
`6d21cb7985e149f3711bb7942e00a9a63d57c4d3f14ac3ec8bf4716290bdcc1a`.
The repaired source hash was `d185c857…`; three target-config hashes matched the
prior review. This supports source-only repair; mtime alone would not.

Three report qualifications matter for future measurements. The re-review
acknowledged a failed contiguous-sector extraction, then corrected MODE2/2352
sector-stride handling; this was a probe error, not original-pin drift. The
behavior parser retained only the leading clause of wrapped prose, while the
full sentence survived in Splat; nonempty/parsable metadata did not establish
complete behavior extraction. Finally, repair reported `ladder_exhausted:false`
because exactness ended the search, but final review JSON changed it to `true`
without exhaustion evidence. Exactness and search exhaustion are separate fields.

**Timeout diagnosis corrected:** repair's read-only inspection reported that the
cap reached `subprocess.run(timeout=…)`; its absence from downstream argv did not
prove missing enforcement. It instead identified possible orphaned worker
processes and exit zero on timeout. Neither concern was dynamically reproduced
in that mission. Preserve the earlier long-run report as a lead, but do not
promote its original diagnosis into a confirmed tooling defect or implement a
forwarding fix from that claim alone.

**Acceptance layers disagree:** both repair and re-review metadata say
`rejected`, despite process exit zero and the domain pass. Runtime checks report
staged files; re-review also records `evidence:no-staged-files` failure, although
its structured report explicitly says `noStagedFiles:false`. Reports identify
122 pre-existing staged entries and no new staging. Full baseline/child review
remains pending, so these are qualified attribution findings, not authorization
to bypass checks. Preserve native result, reviewer verdict, runtime acceptance
and parent disposition separately; no all-layer acceptance is established here.

| Improvement destination | Proposed change grounded in this case | Acceptance measurement |
| --- | --- | --- |
| Skill directive | Retain the best candidate using score, first mismatch, residual shape and size; verify fixed-RAM form before review | Comparable missions retain the measured best frontier; contract-only review repairs/N and first-pass outcomes reported |
| Matching references | Explain the split-base/operand-order case and its local comment evidence; distinguish exact closure from ladder exhaustion | Reproduced gates on eligible examples; zero unsupported exhaustion flags, with original bounds retained |
| Harness tooling | Trace timeout from wrapper through worker descendants and preserve timeout status; provide reusable sector-aware original-byte extraction | No orphan descendants after a measured cutoff, distinct timeout outcome, original-byte hashes preserved; baseline failures reproduced first |
| Report/acceptance tooling | Preserve false versus missing fields, baseline versus new staging, and all acceptance layers | Every conflicting outcome remains visible with run and snapshot identity; no success inferred from exit zero or prose alone |

These are proposals. Native checks remain necessary; this single repaired case
does not establish how much review or repair cost any proposal would save.

## Next three campaign lifts: exactness and unresolved review findings

Parent `01a0c062`, records **1048–1082**, contains complete writer/reviewer
deliveries for queue entries 2–4. The attribution is `$bof3-re` through the parent
missions, despite empty skill-selection metadata. Raw child semantic review
remains pending. `tmp/observation-ingest/older-next-lifts-metrics.json` pins each
report's parent location and each child's full run ID, source hash and metrics.

| Selector in `emi/etc/game/00` | Writer / reviewer | Started calls, writer + reviewer | Observed seconds, writer + reviewer | Reported native result |
| --- | --- | --- | --- | --- |
| `0x801C3598` | `246bf438` / `1e769790` | 59 + 75 = 134 | 394.050 + 396.474 = 790.524 | 45/45, 180 B, byte-match exit 0 |
| `0x8019AA44` | `217d99e4` / `6f4a5bfc` | 99 + 77 = 176 | 632.942 + 366.221 = 999.163 | 46/46, 184 B, byte-match exit 0 |
| `0x801C2710` | `974d244d` / `f19c5423` | 87 + 113 = 200 | 1,765.268 + 507.903 = 2,273.171 | 52/52, 208 B, byte-match exit 0 |

Six children total **510 calls / 4,062.858 summed event-span seconds**; writers
245/2,792.260, reviewers 265/1,270.598. All six lack recorded tool-count/duration
fields, use `ninerouter/ds-combo:high`, and exited zero. Fourteen error-end events
await raw-result classification. **3/3 first review reports passed**, with no
repair dispatch in this slice, but all three reviewer metadata records are
`rejected` for staged-file checks; entry 3 also has a missing-evidence check.
Writers are `not-required`. Review pass and parent queue acceptance therefore
must not become an unqualified all-contract success rate. These three differently
sized missions and their different evidence work do not establish a speedup.

**Entry 2 — successful expression change and a disproved stale-output claim.**
The writer reported two of three attempts used: local work-pointer reassignment
gave **41/45 (91.11%), first +0x54**; indexing the global directly at both reload
points gave **45/45**, with no matching aid. It first inserted a name-ordered
symbol row, received `unnormalized map`, and repaired address order. The optional
promotion command could not run because `clang-format` was absent; no install
was attempted. Reviewer forced object rebuilds for this entry and entry 1,
reported stable object hashes, and invoked the canonical source-policy check.
Metadata validation's **923 valid/0 invalid** was correctly distinguished from
native comparison; `uncompared` was not a native failure.

The writer called generated assembly a stale stub. Reviewer deleted/regenerated
the two outputs, which reappeared at **2,763/2,655 bytes**, and confirmed the C
object remained linked and exact. This refuted staleness for this boundary;
older unrelated leftovers were a separate observation. Reviewer also reported
513 wrapped behavior tags in HEAD sources, but prevalence cannot establish that
the parser retains the full behavior sentence. Its concern about entry 1's
allowed aid was referred back to the already recorded scoped decision.

**Entry 3 — exact bytes did not resolve source-contract findings.** The short
writer delivery listed five changed paths and exactness, but omitted the
requested command/outcome detail and consumed-attempt counts. Reviewer reproduced
46/46 after clearing disposable objects/output, reported six new externs in
17 header lines and **154 includers within one target**, and rechecked entries
1–2. Its provenance stopped at extracted archive-header metadata and payload;
it disclosed not opening the raw archive. Mtimes and a truncated dirty-status
inspection do not independently prove all changes were attributable to this writer.

Three findings were retained as non-blocking, then copied into queue acceptance:

- The aid comment claimed losing two instructions while quoting 45/46. No
  independent ablation reproduced that claim; score and instruction-count change
  are different quantities, so the complete candidate is needed to resolve it.
- Strict progress parsing rejected the actual `@residual none; …` text.
  Canonicalization in memory yielded valid tags; it did not repair the source.
  The reported **714 matching files** and **46/60 sampled failures** do not excuse
  a new file's invalid metadata. Sampling method, duplicates and denominator
  coverage remain unverified; do not extrapolate that sample into a failure rate.
- Six new addresses lacked `WEAK_SYMBOL_AT` despite the cited three-part binding
  contract. A green check enforcing binding→map does not establish the reverse
  obligation, and peer omissions do not override that contract. Full contract
  acceptance remains unresolved despite the review's pass.

The reviewer proposed untried aid-removal experiments while setting
`ladder_exhausted:true`, again conflating exactness with exhaustion. The parent
then allowed “canonicalizer-normalized” metadata in the next review prompt. That
is observable handoff relaxation, not evidence that invalid source metadata was
fixed or that current policy changed.

**Entry 4 — independent ablation, layout checks and another unverified overrun.**
Original boundaries proved **208 bytes/52 instructions**, correcting the frozen
queue's 51. The seven-row writer attempt ledger progressed **46/52→49/52→52/52**;
an address-taken facing pointer was decisive. Three-attempt/two-repair limits
were not reconciled with those seven rows. The writer reported `m2ctx`, `m2c`
and duplicate-query refusals for stale analysis, then reconstructed from Splat
assembly and target C. It also reported a permuter exceeding its 60-second cap;
only a shape hint was retained and re-derived manually. Raw execution and worker
cleanup remain unverified; a second report is not a reproduced timeout cause.

Review's scratch-copy ablation reproduced **49/52, first +0xA8** without the
pointer. In contrast, adding a `scale` local to the final shape remained exact;
combining it with direct facing access remained 49/52 at +0xA8. Thus the source
comment's earlier 46/52 at +0x68 could not be reproduced from the retained form.
Intermediate-state claims need candidate identity, not just a prose lever name.
The reviewer reported three rebuilds, while the displayed digest evidence labels
pre-state and two deletions; the exact executed count awaits raw child review.

The `pad_14[4]`→`s32 unk_14` header change had scratch compile assertions for
size **0x98** and ten offsets, one field user and **155 same-target includers**;
new table `D_80181BD4` had extern, map and support binding. These are stronger
layout/ownership evidence than naming convention alone. The original word load
proves width, not signedness by itself; the claim that this was the only clean-C
representation is not established by that load. Reported disc extraction and
byte checks support payload identity. Source-comment and transient queue-state
notes remained open after the parent recorded acceptance.

| Improvement destination | Proposed change | Acceptance measurement |
| --- | --- | --- |
| Skill directives | Keep contract findings open until corrected or explicitly resolved by the owner; compare exact candidate states before declaring a lever exhausted | First-pass review outcomes reported beside unresolved contract findings/N; zero waivers inferred from peer prevalence |
| References | Show strict tag spelling, map address ordering, three-part data bindings and reproducible aid ablations | Invalid-tag/map/binding repairs and unrepeatable comment claims/N decline without dropping native or ownership checks |
| Harness tooling | Validate all required binding directions and actual on-disk tags; distinguish regenerated assembly from stale leftovers; preserve queue estimate versus measured boundary count | Each known mismatch is detected with source/target identity; valid generated outputs remain eligible; corrected counts cannot silently disagree |
| Campaign handoffs | Refresh analysis at an authorized stable boundary and account for all attempts across searches | Stale-refusal calls/time and unaccounted attempts/N measured; refresh preserves frozen evidence and does not race review |

These proposals preserve the verified byte results while retaining unresolved
scope, contract and provenance obligations. Source-comment repairs remain with
the source owner; this documentation audit does not authorize editing lifted C.

## Last frozen entry and five-function batch accounting

Parent `01a0c062`, records **1083–1092**, retains the complete writer/reviewer
reports for `emi/etc/game/00@0x801B6418`. Writer `428c9e06` used **73 started
calls / 379.286 observed seconds**; reviewer `b56f5765` used **84 / 365.312**.
Both lack recorded tool-count/duration fields, use `ninerouter/ds-combo:high`,
and exited zero. Writer metadata is `not-required`; reviewer metadata is
`rejected` for staged-file and missing-evidence checks. Eight error-end events
need raw-result classification. Attribution comes from the explicit RE mission;
full child semantic review remains pending. Pins and joins are in
`tmp/observation-ingest/older-batch-verification-metrics.json`.

An indexed four-byte record of two `u16` fields produced **22/22, 88 bytes**
on the first eligible C-shape comparison, without an aid. It was not the first
successful tool invocation: an omitted function-map row first prevented symbol
resolution, then a misordered data row failed map normalization. The writer
explicitly accounted for **one shape attempt and two configuration repairs**;
its four-row ledger also included final metadata/checking. This distinguishes
shape yield from transaction readiness more clearly than counting every row as
an attempt. Six paths changed, including header typedef/extern and support binding
outside the initial named source/map/Splat scope; later review included them.

Review reported two forced rebuilds with stable object hash, linked `.text`
equal to the original 88-byte window, strict metadata/source-policy success,
and rebuilt non-regression checks for entries 1–4. The original loop supports
**ten reads at four-byte stride**, not a proven complete table extent. The next
known symbol is **0x8C (140) bytes** after the table start; after the consumed
0x28-byte range, the remaining gap is **0x64 (100) bytes**, not the 140 bytes
claimed for that remainder in the suggested probe. Treat loop bounds, declared
array extent and independently delimited data extent as separate evidence.
Semantic field names relied on prior documented context, not the stride alone.

The reviewer found **17 new naming-debt rows** and correctly distinguished the
passing target check from the failing repository-wide check. However, its prompt
already asked it to confirm that debt was not a match/scope defect; an independent
review should retain room to refute the proposed classification. Other report
inconsistencies remain: entry 1's source was called entry 3, prior-entry forced
rebuilds were both reported completed and proposed again, and `ladder_exhausted`
was true after first-shape exactness. None changes the reported native match,
but each weakens unqualified completion/exhaustion claims. The writer also
repeated a stale-stub diagnosis without the delete/regenerate evidence that had
refuted it for entry 2; do not transfer that diagnosis between boundaries.

Across this five-function batch, the linked cohort comprises **six writer and
six reviewer runs**, including entry 1's repair cycle: **944 started calls /
7,993.782 summed observed seconds**. Writers account for 457 calls/5,520.396
seconds; reviewers 487/2,473.386. First review reports passed for **4/5 functions**,
one repair mission followed, and the parent eventually labeled **5/5** accepted
exact. All 12 processes exited zero; metadata acceptance is six `not-required`
and six `rejected`. Twenty-nine error-end events remain unclassified. These are
separate outcome dimensions, and the earlier source-contract findings remain
open despite the parent labels. No population throughput or all-contract pass
rate is established from this batch.

Proposed **mission/reference** improvement: supply the complete identity/config
transaction before the first native comparison; label shape attempts, repairs
and final checks separately; describe only the data extent actually evidenced.
Proposed **report tooling**: reconcile selector/entry identity, completed versus
remaining checks, and exactness versus exhaustion. Measure configuration-blocked
comparisons, avoidable map repairs and inconsistent report fields per comparable
mission, preserving native and independent-review requirements.

## First battle partial: residual correction and placement obligation

Parent `01a0c062`, records **1108–1142**, retains writer `1fb4fd01` and reviewer
`06af8056` for `emi/battle/battle/15@0x800A82F8`. Their extracted costs are
**109 calls/1,910.386 observed seconds** and **79/479.463**, totaling **188 calls /
2,389.849 summed event-span seconds**. Both lack recorded tool-count/duration
fields, use `ninerouter/ds-combo:high`, and exited zero; metadata is writer
`not-required`, reviewer `rejected` for staged-file/evidence checks. Eight
error-end events await raw-result classification. Complete embedded reports and
parent blocks were reviewed; full child history remains pending. Pins and joins:
`tmp/observation-ingest/older-battle-partial-recovery-metrics.json`.

The writer retained **21/27 (77.78%), 104→108 bytes, first +0x0004**, byte-match
exit **1**. The original has 26 instructions; the displayed score uses 27 from
the candidate, not 26 original instructions. Its reported sequence had seven
initial shapes and eight further permutations, plus two diagnostic variants,
without reconciling them to three attempts/two repairs. The no-store diagnostic
is not a semantically eligible candidate. A permuter hint remained unmeasured;
the writer reported stopping runaway coordinators after steering, not exactness.
Six source/header/support/config paths changed; stale analysis still blocked
function-brief/m2c. These are a partial proposal and execution costs, not an
accepted exact lift or evidence that a small instruction count guarantees cheap work.

Independent review reproduced the score and byte difference but required repair
of two residual clauses. Linked instructions showed **the nop before the branch**,
the branch delay slot containing `move a1,v0`, and the allegedly absent count
copy `move a2,v1` present. Scratch assembler probes reportedly isolated the
mechanism: a macro-expanded count load immediately followed by a branch using
its result received an injected nop; count-first ordering with intervening
pointer/store work removed it. This supports a specific scheduling experiment,
not a claim that the remaining source shape was already measured exact.

The parent applied the specified comment correction at 1124–1125, **33.315
seconds after review delivery**. A subsequent diff retained 21/27, 104→108,
first +0x0004. No fresh byte-match result or independent re-review of the repaired
state appears in this slice; the queue records `partial-documented` and retains
the prior needs-fix verdict in its evidence text. The parent later called the
partial accepted after aggregate checks, which is a separate claim.

**Whole-target placement remains unresolved.** The generated 108-byte body exceeds
its 104-byte boundary by four bytes, reaching the next function's address
`0x800A8360`. Review reported that the neighbor's own object/diff was unchanged,
but also identified overlap rejection in placement validation. Individual object
checks, symbol/Splat success and a later green full test recipe do not prove the
combined target accepts this oversize partial. No placement result discharged
that obligation before continuation. Review's proposed blanket rollback of five
modified files likewise is not proof of safe rollback in the dirty tree; restore
the mission baseline, not unrelated earlier work.

Proposed **matching directives/references**: derive residual mechanism clauses
from the linked instruction sequence; distinguish assembler-injected nops from
branch delay slots and preserve candidate identity for each claim. Proposed
**harness acceptance support**: carry oversize/overlap obligations separately from
per-function score and metadata status. Measure corrected mechanism claims/N,
unaccounted variants/N and oversize partials with verified placement/N. A review
that catches a wrong explanation has value even when its match score is unchanged.

## Second battle partial: repeated residual errors after stronger guidance

Parent `01a0c062`, records **1143–1163**, retains the complete reports for
`emi/battle/battle/15@0x800AF6B8`. Writer `93f4ac11` used **125 calls/1,729.808
observed seconds**; reviewer `b6084cdf` used **30/222.975**: **155 calls/1,952.783
summed event-span seconds**. Both exited zero, used `ninerouter/ds-combo:high`,
and lack recorded tool-count/duration fields. Metadata is writer `not-required`,
reviewer `rejected` for no staged files; six writer error-end events remain
unclassified. Full child history is pending. Source/hash/event joins:
`tmp/observation-ingest/older-battle-second-partial-metrics.json`.

The retained candidate is **7/27 (25.93%), 104→108 bytes, first +0x0004**;
byte-match exits **1**. Five paths changed, including the manifest omitted from
the writer's explicit allowed-path list. The report lists seven rungs, including
**84 statement orders**, then **84×3=252 order/tail combinations**, numerous
return shapes and six profile flags. These are reported search batches, not 336
proven unique candidates; their relation to the three-attempt cap is unstated.
The permuter reportedly used `timeout 150`, `--time-limit 60`, eight workers,
and required orphan cleanup, despite the handoff's `timeout 120` instruction.
Its saved score 410 yielded only 7.41% and was rejected. Raw execution and
descendant cleanup still need verification; shorter total duration alone cannot
prove the revised bound worked.

Review reproduced the score, decoded all 26 original words at payload offset
`0x18EB8`, and checked the binary file SHA-1 against Splat. It found **two false
residual statements**: `7/26` contradicted the candidate denominator, and the
claimed result in `t2`/required `move v0,t2` referred to neither current listing
nor actual return behavior. The current listing contains no `t2`; the original
plane value used it, while the current plane value uses `a2`. The observed extra
instruction is a jump over a separate zero-return block, with zero initialization
sunk from entry. A single trailing return was proposed as an experiment, not
demonstrated to reproduce the original. The writer already reported related
return-shape searches; the reviewer did not reconcile that overlap, and its
"not yet tried" profile/permuter claim contradicts the writer's report. Proposed
acceptance by a `byte-match` percentage also mismatches the displayed binary
command result; use `asm-diff` for score and byte-match exit for exactness.

The parent repaired the comment **16.443 seconds after review delivery** and
explicitly left byte match and independent review pending. No new native check
or independent review of that repaired entry appears before the next launch.
The later full recipe passed but does not discharge those obligations. This
review did independently recheck entry 1's corrected residual and unchanged
21/27 score, extending the previous episode's evidence; whole-target placement
remained untested. For entry 2, the reported next boundary is `0x800AF720`:
108 bytes from `0x800AF6B8` ends at `0x800AF724`, another four-byte excess by
arithmetic, not a successful combined placement result.

Across these **two consecutive battle partials**, **2/2 first reviews required
residual repair**, **0/2 produced exact lifts**, and the four child runs used
**343 calls/4,342.632 summed event-span seconds**. This bounded cohort supports
a recurring explanation-quality finding, not a campaign-wide failure rate or
causal speed comparison. Proposed **directive/reference** change: tie every
residual clause and experiment to the retained candidate and prior-attempt ID;
reject stale register explanations and already-tried rungs presented as new.
Proposed **harness** support: emit original/current/matching counts and candidate
identity together, record actual search cardinality against mission budgets,
and retain unresolved placement/review obligations through aggregate checks.
Measure residual repair frequency, duplicate experiments and bound violations
on subsequent comparable missions before claiming improvement.

## Jump-table alignment: reproducible scratch proof, canonical partial

Parent `01a0c062`, records **1164–1188**, retains writer `a58166a8` and reviewer
`a0caa538` for `emi/battle/battle/15@0x8009CF80`: **105 calls/860.811 observed
seconds** and **95/576.319**, totaling **200 calls/1,437.130 summed event-span
seconds**. Both exited zero with missing recorded tool-count/duration fields;
writer metadata is `not-required`, reviewer `rejected` for staged-file/evidence
checks. Three error-end events remain unclassified. Full parent blocks, supervisor
receipt and embedded reports were reviewed; full raw child review remains pending.
Pins and joins: `tmp/observation-ingest/older-battle-alignment-metrics.json`.

The canonical result remains **25/27 (92.59%), 108→108 bytes, first +0x0010**,
byte-match exit **1**. Both differing instructions materialize the jump-table
address: original `0x80096954`, default linked table `0x8009CFF0`. Review checked
the target binary hash, 108 code bytes at payload `0x6780`, and the 32-byte table
at payload `0x154`, whose eight pointers target this function's case labels.
The writer classified the first seed as relocation/placement, reported four
rungs, and kept four source/config paths changed with no placement declared.
That rung count is not reconciled to the three-attempt mission cap.

| Route | Reported, independently reproduced result | Acceptance meaning |
| --- | --- | --- |
| Unmodified object, no placement | Eight-byte-aligned `.rodata`; 25/27, DIFFER | Canonical partial |
| Explicit true placement at `0x80096954`, size `0x20` | Four leading pad bytes; section `0x24`, table at `0x80096958`; harness rejects bytes/size | The original address alone is insufficient |
| Scratch object alignment reduced to four, same link address | 108 `.text` bytes and 32 `.rodata` bytes identical | Evidence for a scoped pipeline capability, not an accepted canonical exact build |
| Existing `game/00@0x801ACEBC` with four-aligned input | `.rodata` at `0x80195ED4` (4 mod 8), size `0x14`; byte-match MATCH | Counterexample to a blanket inability to represent 4-mod-8 placements |

The reviewer reproduced the placement exception through a compare override
without changing the manifest. It also narrowed compiler evidence: **2.6.3
failed this translation unit on `__attribute__`**, so its probe did not establish
output alignment; **2.95.2 was installed but omitted** from the writer's list.
The four successful versions reported by review all emitted eight-byte alignment,
as did the four optimization levels tested. These findings support this input's
alignment mismatch; they do not prove every possible C/profile route exhausted.
The proposed `SUBALIGN(4)` alternative remained untested. Matching blocks in an
instruction diff are also not interchangeable with control-flow basic blocks.

Review returned **accepted-partial**, preserving the live mismatch and scratch
proof. The parent deferred a linker change to a separate pipeline-validated
transaction and recorded the capability gap. No whole-target build established
that leaving generated `.rodata` unplaced was safe for combined layout; earlier
partials' unchanged per-function metrics and a green aggregate recipe do not
provide that proof. Review noted two other dirty metadata edits, but earlier
reports already identified them as pre-existing; without mission-baseline diffs,
their presence does not attribute an out-of-scope edit to this writer. Likewise,
a naming gate requiring baseline membership does not alone authorize admission.

Across the **first three battle candidates**, six child runs used **543 calls /
5,779.762 summed event-span seconds**: zero canonical exact results, two first
reviews requiring residual repair, and one first-review accepted partial.
Scratch exactness is retained as a separate evidence outcome. Proposed
**directives/references**: classify relocation/alignment mismatches before more
C search; separate failed compilers from measured profiles; seek a counterexample
before generalizing a tooling ceiling. Proposed **harness** work: an explicit,
reviewed per-placement alignment route using a scratch copy, preserving input
objects and checking both code and data bytes plus existing placement behavior.
Measure canonical exact conversions per verified alignment case, retained
placement obligations and duplicated profile probes; this audit implements none
of that pipeline change.

## Fourth battle partial: score gain and repeated evidence repair

Parent `01a0c062`, records **1189–1220**, retains writer `7852c78a` and two
reviews of `emi/battle/battle/15@0x800AF5FC`. Extracted costs:

| Run / role | Calls | Observed event-span seconds | Report outcome |
| --- | --- | --- | --- |
| `7852c78a`, writer | 103 | 1,676.993 | Deferred partial, 5/28 |
| `990e8338`, first review | 91 | 1,104.421 | Repair; higher-scoring candidate found |
| `8251857c`, repaired-state review | 76 | 569.005 | Body accepted-partial; residual still needs repair |

Total **270 calls/3,350.419 summed seconds**; reviews alone account for **167
calls/1,673.426 seconds**. All three exited zero and lack recorded tool-count/
duration fields; writer metadata is `not-required`, both reviewers `rejected`
for staged-file/evidence checks. Nine error-end events await classification.
Complete parent blocks/arguments and embedded reports were reviewed; raw child
review remains pending. Pins: `tmp/observation-ingest/older-battle-frontier-repair-metrics.json`.

The writer reported six rungs, including 52 catalogue flag candidates, compiler
variants and `timeout -k 5 110 … --time-limit 60 -j 4` permuting. It also reported
killing workers after an earlier wrapper-only timeout; "one bounded run" is not
proof of one process attempt or descendant termination. These search units remain
unreconciled to three attempts/two repairs. The retained five-path proposal scored
**5/28 (17.86%), 112→112 bytes, first +0x0004**, byte-match DIFFER.

The first reviewer reproduced the documented pair and found a third candidate:

| Candidate | Matching count | Positional matches | First difference | Residual hunks | Positional opcode kinds |
| --- | --- | --- | --- | --- | --- |
| Retained named shifts | 5/28 | 5 | +0x0004 | 4 | 20/28 |
| Plane first, inlined shifts | 4/28 | 4 | +0x0000 | 5 | 27/28 |
| Plane last, inlined shifts | 9/28 | 5 | +0x0004 | 7 | 15/28 |

All three have 112 bytes/28 instructions. The parent applied the third shape and
reproduced **9/28 (32.14%)** after **33.266 seconds from review delivery**; the
second reviewer independently confirmed it. The gain follows the ranking's first
score key while positional agreement worsens. Review identifies the harness metric
as `difflib.SequenceMatcher`; the first report's "LCS" label does not establish
a literal longest-common-subsequence metric. The final source still says **"9/28
instructions in place"**, contradicting the reported five positional matches.
The plane-first alternative's "exact 28-opcode sequence" was also corrected to
27/28, because its return delay slot contains a move rather than the original nop.

Two reviews exposed candidate-provenance failures. Profile numbers from an earlier
body did not reproduce; the first review measured canonical 17.86% versus catalogue
best 19.35%, 2.8.x 17.86%/19.35%, and 2.95.2 39.29%/46.43%. On the new body,
the second review measured 2.8.x **7/28** and 2.95.2 **10/28**. The parent's first
rewrite correctly scoped profiles to an earlier body but copied its register
description: retained work is in `$a2` and plane ends in `$a0`, not old `$a1`/`$a2`.
Only the result temporary's `$t1` versus original `$v0` claim survived review.

Wrapper evidence also failed reproduction. The first reviewer reported eight
matches at 120 bytes/30 instructions and ten for a whole-body wrapper; the second
obtained **7/30**, or **8/28, 9/28 and 11/28** for different reconstructions.
The retained-order wrapper was byte-identical to the unwrapped candidate; its
result clear was not sunk. Exact source identities were missing, so these are
unresolved variant differences, not proof either reviewer measured the same code
incorrectly. The writer called the wrapper a banned artificial control; reviewers
treated it as a shape experiment. Eligibility needs the owning rule and evidence,
not a score-driven assumption. An unpromoted 11/28 experiment does not establish
an eligible better retained candidate.

The parent made one body change and two comment rewrites. The last dropped
unreproducible counts and stale registers, but retained a generic structural-
regression rejection and the inaccurate "in place" label. Its final native read
still showed 9/28; no independent review of that final comment occurred before
continuation. Across the **first four battle candidates**, nine child runs used
**813 calls/9,130.181 summed seconds**: **3/4 first reviews required repair**, one
accepted a partial, and zero canonical exact lifts resulted. This cohort supports
measuring residual accuracy and review rework alongside match percentage.

Proposed **directive/reference** improvement: bind every alternative, profile and
register claim to exact candidate/flags identities; compare score, positional
matches, hunks and size separately. Proposed **harness** support: durable candidate
receipts with source/flags/object hashes and named metric definitions, invalidated
when the body changes. Measure unreproduced claims, repeated comment repairs and
review calls per verified disposition; removing disputed numbers alone does not
verify their remaining explanation.

## Battle batch closure: one exact lift and four partials

Parent `01a0c062`, records **1221–1234**, retains writer `b739779c` and reviewer
`737e9cfc` for `emi/battle/battle/15@0x800AD930`: **67 calls/550.587 observed
seconds** and **60/264.528**, totaling **127 calls/815.115 summed seconds**.
Both exited zero; writer metadata is `not-required`, reviewer `rejected` for
staged files. Recorded tool-count/duration fields are missing; two error-end events
await raw classification. Complete parent blocks/arguments and reports were read;
full child semantic review remains pending. Pins and batch joins:
`tmp/observation-ingest/older-battle-batch-completion-metrics.json`.

The writer reported **three attempts, zero repairs, no permuter**: seed **18/32,
124→128 bytes**, a qualifier change removing the extra byte mask, then direct
compound scratchpad stores fixing the second block's pointer allocation. Final
**31/31, 124→124 bytes, byte-match MATCH** was independently reproduced; the
reviewer reported two forced object rebuilds. Source extent `0x17130..0x171AC`
and payload bytes matched, while the other qualifier consumer remained **14/14,
56 bytes, MATCH** and all four partials retained their scores. Six paths changed,
including support/manifest paths outside the handoff's explicit list. The new
`D_80148624` binding was derived from this target's `lui/lbu` address calculation;
the same address in the executable map is a separate identity, not evidence of
cross-target copying. The qualifier lesson is [scoped separately](bof3-types.md#byte-global-qualifier-experiment-and-its-limits).

Review's overall verdict was **repair**, with the lift accepted and three new
global naming-debt rows still blocking the repo-wide gate. Parent admission and
aggregate checks subsequently passed; no separate post-admission native recheck
or review receipt appears. Thus the exact function evidence is strong, while
"review accepted everything" would overstate the recorded verdict.

The frozen five-entry battle batch now has **1/5 canonical exact lifts (20%)**
and **4/5 documented partials (80%)**. Five writers and six reviewers used:

| Role | Runs measured | Calls / summed observed seconds | Calls median / p90 / max | Seconds median / p90 / max |
| --- | --- | --- | --- | --- |
| Writers | 5/5 | 509 / 6,728.585 | 105 / 125 / 125 | 1,676.993 / 1,910.386 / 1,910.386 |
| Reviewers | 6/6 | 431 / 3,216.711 | 77.5 / 95 / 95 | 524.234 / 1,104.421 / 1,104.421 |

Total **940 calls/9,945.296 summed event-span seconds**; p90 is nearest-rank
and equals maximum in these small role cohorts. All 11 runs lack recorded
tool-count/duration fields; these are extracted event measurements. All exited
zero, but six reviewer metadata results were rejected and five writer results
were not required; 28 error-end events remain unclassified. First reviews were
**three residual repairs, one naming repair with lift accepted, and one accepted
partial**. Including the second review of entry 4 yields five repair verdicts
out of six reviews. Parent repairs are additional work, not extra child runs.

The preceding game00 batch used 944 calls/7,993.782 summed seconds for five
parent-accepted exact lifts. Different functions, blockers and repair paths make
this a descriptive contrast, not a controlled efficiency regression. Proposed
**selection/reference** improvement: report outcomes by residual class and review
stage instead of small instruction count alone. Proposed **harness** reporting:
retain canonical exact, scratch exact, body accepted, report repaired, naming
admitted and combined placement verified as distinct fields. Evaluate future
changes against comparable task cohorts, including review and parent-repair cost.

## First battle03 lift: struct-field recovery and invalid lifecycle metadata

Parent `01a0c062`, records **1235–1246**, retains writer `8bd43905` and reviewer
`0275f718` for `battle/03@0x801E71EC` (`countdownScratchByte1ByGroupFlags`):
**89 calls/661.768 observed seconds** and **68/349.388**, totaling **157 calls/
1,011.156 summed seconds**. Both exited zero; writer metadata is `not-required`,
reviewer `rejected` for staged files. Both lack recorded tool-count/duration fields;
four error-end events remain unclassified. Full parent blocks/arguments and reports
were read; raw child review remains pending. Pins and joins:
`tmp/observation-ingest/older-battle03-field-lift-metrics.json`.

The seed scored **25/47 (53.19%), 188→180 bytes, first +0x0010** using byte-offset
casts and a temporary `D_801EB6B2` symbol. Existing target types and exact siblings
supported `D_80145E90[idx].unk_80` (stride `0x140`) and
`D_801EB630[idx-3].unk_82` (stride `0x118`). Replacing the casts with these fields
produced **47/47, 188→188 bytes, MATCH**, with no permuter or aid. The temporary
header/support/map additions were removed; review compared content to HEAD and
checked index flags, rather than treating modified timestamps as retained edits.
Four source/config paths remained changed. The four numbered report rows include
measurement/finalization, not four distinct source candidates.

Independent review reported identical object and linked-object hashes after a
forced rebuild, payload equality for all 188 bytes at `0x165EC`, and exact field
address/stride correspondence. The initial offset-arithmetic error was reportedly
corrected before edits. The writer's extra relocation proof is internally
inconsistent: 47 words minus ten claimed relocation slots cannot leave 45 other
words. Keep the independent linked-byte proof separate from that unresolved count.
The review's "no partial-to-exact decisive experiment" lesson is also contradicted
by the writer's measured 25→47 transition; first-time source creation does not
erase the earlier nonmatching candidate.

**Native exactness did not establish valid lifecycle metadata.** Review reproduced
`parse_progress_tags` rejecting `@residual none; live audit …`, requiring exactly
`none`; lifecycle/index status was therefore `invalid` while `validate_sources`
reported **932 valid/0 invalid**. It called this a non-blocking convention because
**730/939 sources** reportedly shared the phrasing. That prevalence is a reviewer
scan claim, not a separately reviewed corpus measurement or acceptance authority.
The newly authored source introduced another instance, even if its spelling copied
siblings. Parent and reviewer retained it and recorded accepted-exact. A successful
index refresh does not repair invalid metadata; the actual refreshed lifecycle
row was not displayed.

Other retained qualifications: m2c reportedly failed on `ABILITY_OBJECTS`, so the
seed was hand-authored; a stale asm file was reported inert because the regenerated
linker script did not reference it; and discarding a pointee `volatile` qualifier
was described as a sibling convention, not separately justified semantics. Semantic
filename registration passed without raw-name admission, but no full naming audit
was performed. Proposed **directive/reference** improvement: retain measured
representation transitions and distinguish native, metadata and semantic evidence.
Proposed **harness** support: make lifecycle and aggregate validators expose their
disagreements, and attach candidate IDs to relocation-count claims. Measure exact
lifts with invalid lifecycle metadata, successful type-representation repairs and
missed lessons separately; prevalence does not make a parser failure harmless.

## Marker reuse: exact bytes and an independent counterfactual

Parent `01a0c062`, records **1247–1260**, covers `battle/03@0x801DD29C`
(`consumePendingGroupQueue`). Writer `e5385ef0` used **128 calls/1,273.396
observed seconds**; reviewer `0aac314e` used **97/482.810**: **225 calls/1,756.206
summed seconds**. Both exited zero and lack recorded count/duration fields;
metadata is respectively `not-required` and `rejected` for staged files. Four
reviewer error-end events remain unclassified. Complete parent reports were read;
raw child semantic review remains pending. Pins and joins:
`tmp/observation-ingest/older-battle03-marker-lift-metrics.json`.

The writer reported nine variants under a three-attempt/two-repair mission,
without reconciling those units. Its seed was **16/48**; named marker locals
(`0xFF`, `1`, `3`) retained preheader constant loads and reached **45/45,
180→180 bytes, byte-match exit 0**. A final cursor simplification preserved that
result. Independent review replaced the locals with literals: **27/45 (60%),
43 current instructions, 180→172 bytes, first difference +0x0014**; its control
reproduced the shipped 45/45 result. This supports this specific ordinary-C aid,
not a general rule that named constants improve matching. No pinned registers,
inline assembly or clobbers were reported. The adjacent aid note explained the
controlled placement and removal condition.

Review also reproduced payload equality over offsets `0xC69C:0xC750`, forced
function regeneration, and purged **657 artifacts for 219 target sources** before
a successful target build with **zero errors/31 reported pre-existing warnings**.
A subsequent native check stayed exact. Compilation of all objects does not prove
all target bytes or combined placement. The layout checks and naming correction
are supporting evidence in their owning ledgers, not additional RE completions.

The known `@residual none; …` issue recurred and was treated as a convention;
this report did not display a fresh strict-parser result. Aggregate validation
again reported zero invalid sources, so the earlier parser disagreement remains
unresolved. The review's `ladder_exhausted=true` and “no partial-to-exact experiment”
lesson also fail to describe the nine-variant measured trajectory: exactness can
justify stopping without proving every rung exhausted. Six paths changed, including
manifest/support files omitted from the writer's explicit scope list.

Proposed **directives/references**: preserve candidate-bound counterfactuals and
use an exact-result stop reason separately from exhaustion; account for variant,
attempt and repair units. Proposed **harness** support: record control/counterfactual
hashes, score algorithm, byte sizes and layout/build coverage in one review receipt.
Measure aids independently reproduced / aids reviewed, variants per accepted lift,
scope exceptions and unresolved lifecycle findings alongside exactness and cost.

## Loop-local bases: reproduced allocation and mutability evidence

Parent `01a0c062`, records **1261–1276**, covers `battle/03@0x801DCFD0`
(`copyLocalWorkToTemplates`). Writer `86f6ebb6` used **62 calls/647.519 observed
seconds**; reviewer `fb2b7c92` **110/521.279**: **172 calls/1,168.798 summed
seconds**. Both exited zero and lack recorded count/duration; metadata is
`not-required`/`rejected` (staged-file checks), with eight unclassified error-end
events. Complete parent reports were read; full raw child review remains pending.
Pins: `tmp/observation-ingest/older-battle03-copy-lift-metrics.json`.

Inline base expressions produced **43/47 (91.49%), 188→188 bytes, first +0x0018**,
with template/work registers swapped. Naming both bases inside the loop, templates
first, using an exact inverse-copy sibling's idiom, produced **47/47, 188 bytes,
byte-match exit 0**. Independent review compiled the inline counterfactual under
the canonical profile and reproduced 43/47 and the same first difference. Four
writer report rows comprise a seed, decisive shape, type correction and finalization;
they are not four distinct matching attempts. No matching aid was declared.

Review forced object regeneration and independently applied ten reported HI16/LO16
relocations, obtaining the same 188 bytes as the payload. A fresh disc extraction,
EMI slice at `0x43000` of length `0x1CDD0`, configured binary hashes and function
slice `0xC3D0:0xC48C` corroborated original-byte provenance. A target build reported
219/219 objects; the report separately counted 220 registered sources. Preserve
those denominators instead of calling the build a 220-function byte comparison.
The [type evidence](bof3-types.md#store-evidence-corrected-a-const-declaration)
explains why removing `const` was justified. Five paths changed; a modification-time
window supports scope inspection but cannot alone prove unrelated bytes untouched.

Review again reproduced strict progress parsing failure and lifecycle `invalid`,
yet accepted it as convention. It reported **175/372 cached target rows invalid**,
named three invalid siblings while calling them “two,” and claimed **718 sources**
shared the syntax. These are report-level scans pending raw verification, not proof
that the new defect was pre-existing. Aggregate validation still reported **934
valid/0 invalid**. Restricting a byte sweep to three index-`exact` rows cannot cover
all byte-exact sources when this parser misclassifies others; the review separately
checked two prior lifts and the two table consumers, with one overlap in the sweep.

Proposed **directives/references**: preserve independently reproduced declaration-
placement experiments, qualifier evidence and explicit stop reasons. Proposed
**harness** support: expose native versus lifecycle populations before selecting a
regression set, and report unique selectors across overlapping checks. Measure
counterfactual reproduction, semantic corrections and validation-population omissions
separately from native exactness and raw tool count.

## Symbol-bound tables: exactness with qualified review findings

Parent `01a0c062`, records **1277–1296**, covers the fourth battle03 lift and the
next writer's return. Writer `4007a764` used **113 calls/825.989 observed seconds**;
reviewer `3a2e4192` **82/375.744**: **195 calls/1,201.733 summed seconds** for
`battle/03@0x801E2170` (`dispatchEnemyModeHandlers`). Both exited zero; metadata
is `not-required`/`rejected` for staged-file checks, with two unclassified reviewer
error-end events and missing recorded count/duration fields. Full parent reports
were read; raw child review remains pending. Pins:
`tmp/observation-ingest/older-battle03-symbol-lift-metrics.json`.

Two reported attempts kept the C body unchanged: literal-address macros scored
**32/69 (46.38%), 276→260 bytes, first +0x000C**; target-local extern/map bindings
and table-macro rebinding produced **69/69, 276 bytes, byte-match exit 0**. Review
reproduced the literal variant's frame-size change and first difference, explicitly
not the normalized 32/69 score. This is mechanism corroboration, not complete score
reproduction or a universal proof that literals cannot produce symbol-like sequences.
Four transaction objects were forced to rebuild by removing twelve artifacts; hashes
remained stable across subsequent comparisons. Neighbors stayed **78/78** and
**20/20**, table consumers **48/48** and **28/28**, and the three prior lifts stayed
exact. The writer's target build reported 223 objects; review did no full native sweep.

Three review findings need qualification. First, its “omitted support file” finding
attributes entry-2 `groupQueueTable/groupQueueCursor` bindings to this writer;
the earlier complete report already lists that change, and the review's own 05:47
mtime predates this transaction's 06:43–06:50 edits. A starting diff is necessary
before charging inherited hunks as omissions. Second, its next-boundary claim names
`0x1167C`; **`0x11570 + 276 = 0x11684`**. The earlier offset is the current
function's final eight bytes, not the next function at `0x801E2284`. Third, it calls
metadata verified while retaining the known `@residual none; …` syntax, without a
fresh strict-parser pass. Exact bytes do not close those reporting/parser obligations.

`bin/promote` failed because `clang-format` was absent; formatting remained manually
checked, despite the opening “all gates green” claim. No installation was authorized.
Review's `ladder_exhausted=true` is unnecessary for a two-attempt exact stop. Proposed
**directives/references**: retain candidate-specific representation evidence and
separate exact stopping, formatter availability, scope attribution and parser validity.
Proposed **harness** support: compare review paths against starting hashes, compute
boundaries from identity facts, and label counterfactual mechanism versus score
reproduction. Measure false-positive findings and unresolved gate prerequisites.

The next writer, `c7f4044c` (`battle/03@0x801DE43C`), used **46 calls/481.128
observed seconds**, not its claimed approximately 25 minutes. It reported two
attempts/one repair: **65/74, 292→296 bytes, first +0x0080** with a volatile view;
**70/73, 292 bytes, +0x0098** with a plain view; **73/73** after commuting mask
operands. It preserved the declaration's qualifier, documented the local view as an
aid, normalized `@residual none` onto its own line and reran native checks. Six changed
paths were enumerated. Exit zero/`not-required` metadata and no error-end events do
not self-accept the lift; reviewer `0cb930cc` was only launched at this checkpoint.

## Battle03 batch closure: factor separation and acceptance metrics

Parent **1297–1312** completes the fifth review (`0cb930cc`): **89 calls/347.152
observed seconds**, exit zero, metadata `rejected` for staged files, five unclassified
error-end events, recorded count/duration missing. Full parent report and arguments
were read; raw child review remains pending. Pins, cohort joins and distributions:
`tmp/observation-ingest/older-battle03-completion-metrics.json`.

The review reproduced **73/73, 292 bytes** after forced object removal and used a
four-variant comparison to isolate two causes. Adding `volatile` to the local slot
pointer adds an `andi` and makes **74 instructions**, but leaves mask-load order
unchanged. Reversing only the mask-test operands keeps **73 instructions** and
changes load order; combining the changes produces both effects. The writer's aid
comment conflated them and omitted the exhausted rung. Review accepted with notes;
the parent corrected both claims. The edit and asm/full-check calls shared a message,
and their result timestamps differ by only 1 ms at the end of the 353-second gate;
those receipt times do not prove edit completion preceded the native comparison.
An asm receipt says exact; no fresh standalone byte-match or independent review of
the final comment is shown. The body itself was unchanged.

This source's strict metadata parser returned **`('exact', 100.0, 'none')`**, a
verified distinction from the preceding sources' invalid syntax. Review also found
three new raw-name baseline omissions while the target-local symbol gate passed;
the parent then ran admission hygiene and the global gate passed. Exact admission
count/diff was not emitted, so the three findings are not automatically a measured
three-row mutation. No raw name was semantically resolved by that baseline operation.

For the **five-function battle03 batch**, ten child runs used **884 calls/5,966.173
summed observed seconds**; first dispatch to final full green was **7,965.540
seconds**. All five received first-review pass verdicts and parent native-exact
acceptance, but those passes included notes, aid-comment repair and unresolved
lifecycle validity. Do not label this five clean all-gate first passes.

| Role | Runs | Calls | Summed seconds | Calls median / p90 / max | Seconds median / p90 / max |
| --- | ---: | ---: | ---: | --- | --- |
| Writer | 5 | 438 | 3,889.800 | 89 / 128 / 128 | 661.768 / 1,273.396 / 1,273.396 |
| Reviewer | 5 | 446 | 2,076.373 | 89 / 110 / 110 | 375.744 / 521.279 / 521.279 |

Nearest-rank p90 equals the maximum for these five-run groups. All ten exited zero;
metadata labels split five `not-required`/five `rejected`; all ten lack recorded
count/duration, and 23 error-end events remain unclassified. **Six full checks:
four passed/two failed, 2,131 wall seconds and 2,092.75 reported pytest seconds**.
Those totals include the overlapping writer/gate episode, so do not add all costs
as serial elapsed time. Differences from the previous battle15 batch are descriptive,
not evidence of a directive's causal speedup across different functions.

Proposed **directives/references**: use one-factor changes or small factorial probes
when an aid explanation attributes multiple effects; retain finding-level repairs
inside accepted verdicts. Proposed **harness** support: distinguish native, lifecycle,
review-note and full-gate states in batch summaries. Measure reproduced causal claims,
corrected claims and clean acceptance separately from exact-result throughput.

## Scratchpad cell volatility: a reproduced local workaround

Parent `01a0c062`, records **1313–1330**, covers `world00/area030/04@0x801DDEEC`.
Writer `cbcafe20` used **102 calls/1,129.695 observed seconds**; reviewer `fe64f3e5`
**81/466.546**: **183 calls/1,596.241 summed seconds**. Both exited zero; metadata
is `not-required`/`rejected` for staged files. Both lack recorded count/duration;
nine error-end events remain unclassified. Full parent reports and arguments were
read; raw child review remains pending. Pins:
`tmp/observation-ingest/older-area030-cell-lift-metrics.json`.

The volatile pointer-cell seed scored **30/37 (81.08%), 144→148 bytes, first
+0x0010**. Statement reordering produced identical output; about twenty scratch
shapes were reported without a per-shape ledger or reconciliation to the three-attempt
mission limit. Using `SPAD_PTR_SLOT(u8, 0x44u)` instead of the volatile cell
produced **36/36, 144 bytes, byte-match exit 0**. Independent review reproduced
both results, including the volatile variant's extra delay-slot `nop`, and found
marker-local and statement-order probes identical to the volatile control. The aid
comment included all four required elements. Forced rebuilding reproduced six
comparison-artifact hashes and the object hash. Canonical progress metadata parsed
exact; the target build passed.

The opening writer JSON mistyped the load address as `0x801DDE00??`; its body and
review correctly established **`0x801DDEEC − 0x801D0C00 = 0xD2EC`**, a 144-byte
payload slice. Review distinguished primary load from companion overlay load
`0x800F5000`. Its suggested future selector `func_801DDFFEC` has nine hex digits
and needs correction before use. The stale analyzer duplicate query was replaced
by an exact-byte scan with no hit; that excludes identical scanned byte strings,
not relocated or semantic duplicates.

The [type review](bof3-types.md#pointer-cell-qualification-and-measured-consumer-scope)
identified 36 readers of the shared volatile declaration, but only one sibling was
experimentally shown insensitive to a qualifier change. The local code-generation
result does not justify applying that change to all readers. Proposed
**directives/references**: distinguish cell qualification from pointee qualification,
retain full counterfactual measurements and quantify shared-declaration scope.
Proposed **harness** support: emit validated selector/load facts and per-consumer
sensitivity receipts, and distinguish duplicate-query failures from negative results.
Measure fully reproduced counterfactuals, measured consumers / inventoried consumers,
and unresolved experiment-budget accounting alongside native exactness.

## Declaration initialization and a naming-only repair

Parent **1331–1340**, `world00/area030/04@0x801E1C10`, records writer `4ebf8b66`
(**57 calls/340.023 observed seconds**) and reviewer `095fda0e` (**90/375.204**):
**147 calls/715.227 summed seconds**. Both exited zero, with metadata
`not-required`/`rejected` for staged files, missing recorded count/duration and seven
unclassified error-end events. Full parent reports and arguments were read; raw
child review remains pending. Pins:
`tmp/observation-ingest/older-area030-initializer-lift-metrics.json`.

The seed matched **26/27 (96.30%), 108 bytes, first +0x0000**, with the counter
zeroing preceding the loop-index constant instead of following it. Moving both
`i = 0x38` and `count = 0` into declarations produced **27/27, 108 bytes** without
aids. The report calls the count initializer decisive, but both initializations moved;
count-only causality was not isolated. Independent review verified exactness, forced
both this and the prior lift's objects to rebuild (reported 3.9 seconds), obtained
the same object hash and confirmed payload/linked-text equality. It did not repeat
the initialization counterfactual. Its “no decisive partial-to-exact experiment”
lesson contradicts the writer's measured transition; exact first-source completion
does not erase the seed experiment.

The two array symbols' addresses and byte accesses were payload-supported. Removing
their explicit map rows reportedly left 27/27 because address-suffix resolution and
Splat auto-symbols supplied addresses. Review corroborated those resolution paths;
this explains native success without making ownership documentation optional. The
labels “slot tables,” “value/companion” and “sentinel” remained disclosed semantic
inferences, unlike numeric loop bounds and comparison constants.

Review requested a **four-row naming-baseline repair**, not a re-lift; parent
hygiene restored the global gate and full checks passed. Two other review claims
were mistaken: counting predecessor `D_80145AA4` as this writer's third extern
ignores the explicitly preserved starting state, and treating its absent target-map
row as debt overlooks the shared map/binding established in the preceding review.
The [naming observation](bof3-naming.md#four-baseline-findings-and-inherited-change-attribution)
separates those claims. Formatting remained visually checked because `clang-format`
was unavailable. Proposed **directives/references**: preserve measured seed changes,
identify multi-variable experiments and compare edits to the starting tree.
Proposed **harness** support: record symbol-resolution provenance and finding ownership;
measure reviewer false positives and missed experiment lessons beside valid repairs.

## Division profile proof separated from control flow

Parent `01a0c062`, records **1373–1403**, retains the completed writer and review
for `world00/area030/04@0x801E0ABC`. The parent mission establishes RE ownership;
explicit skill arrays are empty. Full parent blocks/arguments and both reports
were read, while full raw child review remains pending. Pins and event joins:
`tmp/observation-ingest/older-docs-routing-metrics.json`.

| Role / run | Started calls | Observed event-span seconds | Process / metadata acceptance |
| --- | ---: | ---: | --- |
| Writer `882c0714` | 104 | 746.692 | Exit 0 / not-required |
| Reviewer `97258d0a` | 102 | 361.823 | Exit 0 / rejected |

Together: **206 calls/1,108.515 summed seconds**, with nine unclassified error-end
events. Recorded count/duration fields are missing in both; both models are
`ninerouter/ds-combo:high`. Review metadata fails missing no-staged-files evidence
and whole-tree staging checks, separately from its domain verdict **accepted**.
This is one first-review native-exact result with low findings, not two accomplished
functions or an all-layer pass. The intervening documentation request delayed
writer-delivery to review-dispatch by **248.891 seconds**; this is elapsed priority
switching, not measured wasted RE execution.

The writer reported two attempts. Its first **30/44 (68.18%), 176→144 bytes,
first +0x0024** combined wrong branch polarity with omitted division traps. It
then inverted the early return and added the per-object `-O2 -Wa,--expand-div`
profile, reaching **44/44, 176 bytes, byte-match exit 0**. That combined change
cannot establish the flag alone caused the full score gain. Independent review
compiled the **same final source** under the canonical profile: **35 instructions/
140 bytes**, missing the original's division checks. The configured result was
44 instructions/176 bytes and exact; original `break 7` and `break 6` were at
+64 and +88. This controlled comparison corrects the earlier shape table's
single-cause attribution without inventing a canonical 35/44 match score.

Review forced three function objects to regenerate, retained their hashes and
reproduced both prior entries' **36/36** and **27/27** byte matches. Packed linked
instructions equalled the original 176-byte slice at **0xFEBC**, consistent with
`0x801E0ABC−0x801D0C00`, Splat and payload hash. Strict metadata parsed exact,
100 and canonical `none`, with one function record; aggregate validation reported
939 valid/zero invalid. These are stronger evidence than equal length or a prose
PASS. The writer's repeated post-exact optimization probes are reported separately
from its two claimed attempts; complete candidate/budget accounting needs raw review.

The profile mechanism resolved the exact source key, left the two prior sources
canonical, and appeared on **four build edges: three predecessors plus this one**.
Review ran **16 compiler-configuration, five compiler-wrapper and four assembly-link
tests**, all passing. Its reference paths were stale after test-unit relocation,
and the compiler-variant table already listed only two of three earlier users.
These observations support scoped profile application and specific reference drift;
they do not justify a universal profile default. The writer changed seven paths,
including compiler configuration omitted from its explicit write scope; later
review's technical acceptance does not itself resolve that dispatch discrepancy.

The new 36-byte `Area030ScaleRecord` has measured byte/halfword offsets **0x1F/
0x22**, corroborated by sibling stride/access evidence. Names `limit`/`value`
remain descriptive and unaudited. Explicit casts preserved the existing s32
prototype instead of changing all callers. The stale root compile database made
`flag-search` fail with **zero commands where one was required**; disposable
canonical compilation supplied the comparison evidence. No parent `just build`
receipt proves compile-database repair in this slice, despite later analysis/index
refresh and `ready: true`.

Parent record 1396 explicitly admitted two raw-name baseline rows. This establishes
mutation provenance more directly than review's timestamp inference; it does not
resolve those names. Record 1403 later gives actual scoped green **rc=0/9 seconds**,
22.275 seconds after review delivery. Individual analyze/index exits were suppressed.
The review's “no partial-to-exact experiment” lesson contradicts both the writer's
seed and its own controlled compile; `ladder_exhausted:true` also conflates exact
stopping with exhaustive search.

Proposed **directive/reference** improvement: retain separate control-flow and
profile candidate identities, preserve decisive experiments even for first-source
lifts, and enumerate authorized configuration writes before dispatch. Proposed
**harness** support: expose the selected object profile and compile-database freshness
in comparison receipts, with exact original/current counts and independent statuses.
Baseline: one confounded summary corrected, one missing compile-command entry,
and two stale reference inventories. Compare profile-required missions by reproduced
single-factor evidence, scope exceptions, stale-discovery calls/time and complete
pipeline checks; lower search cost must preserve native and independent-review gates.

## Separate pointer lifetimes and a qualified exact review

Parent **1408–1439**, `world00/area030/04@0x801DE4AC`, retains writer `89f782b5`
(**68 calls/999.313 observed seconds**) and reviewer `29f6629f` (**88/382.253**):
**156 calls/1,381.566 summed seconds**. Both exited zero; recorded count/duration
fields are absent. Metadata is `not-required`/`rejected` for staged-file/evidence
checks, with eight unclassified error-end events. Full parent reports/arguments
were read; raw child review remains pending. Pins and joins:
`tmp/observation-ingest/older-naming-recorder-metrics.json`.

The writer's measured sequence was **45/80, 320→316 bytes, first +0x000C** with
a numeric scratchpad-cell macro; **49/80** with the existing bound volatile cell;
**66/80, 320 bytes** after both signed-byte level and struct indexing changes;
then **76/80** with a separate mirror cursor. Giving the pre-call record its own
local finally produced **80/80, 320 bytes, byte-match exit 0**. The 66/80 transition
changes two factors and cannot isolate either alone. An inline-address alternative
regressed to **64/82, 328 bytes** and was reverted. Seven report rows include several
spellings and twelve measured diff cycles, without reconciliation to the mission's
three-attempt/two-repair budget. Reported `timeout 130 … --time-limit 60 -j 8`
permuting differed from the brief's external 120-second bound, emitted no candidate
and was not used; raw timeout/descendant-cleanup verification remains pending.

Review reproduced exactness under canonical flags, two forced rebuilds with equal
object hash, linked-text/payload equality at **0xD8AC..0xD9EC**, strict exact/100/none
metadata and the three preceding byte matches. It did not reproduce the 76→80
pointer-local counterfactual. Its domain verdict was accepted with notes; ordinary
readable temporaries were not labeled matching aids. Preserve that verified final
result separately from the writer's causal explanation and budget accounting.

Payload evidence supported **0x98 stride, thirty records**, a signed halfword load
at **0x3E**, and byte fields at **0/1/0x5D/0x5E/0x5F**. A sibling's unsigned counter
was a layout lead, not a reason to copy its signedness. The signed load and division
support this target's s16 view without proving it the only possible clean-C
representation. Table `D_801E2314` received a binding/map row, with no entry-boundary
or cross-target ownership claim. Review correctly attributed inherited binding
`D_801E2AB8` to the prior lift. Its `function_801DE4AC.c`-equivalent wording is an
inaccurate path spelling; actual source/manifest identity is `func_801DE4AC.c`.

The volatile cell produced exact output here while entry 1 needed a plain local
view. That is evidence against blanket representation advice, not a proof that
removing the declaration's qualifier would break this entry; no such ablation
was reported. The parent deferred a target-wide consumer assessment. The behavior
phrase “handler byte 0x01” also needed its offset-versus-value meaning kept clear.

The [naming-recorder episode](bof3-naming.md#baseline-recorder-recovery-and-review-boundary)
accounts for the intervening 131.742 seconds before review dispatch and four raw-row
admissions; these are not RE execution cost or resolved names. After review, the
parent's scoped gate passed **rc=0/8 seconds, 940 valid/zero invalid**; `ready:true`
was displayed, while individual analysis/index exits were suppressed. Coverage
was recorded as target **62 lifted/55 exact/7 partial/112 unlifted**, global
**940/797/143/1,407**; those are parent arithmetic records, not a fresh full native
status census.

Proposed **directive/reference** improvement: preserve target-specific representation
counterexamples, separate simultaneous changes and keep actual experiment units
against the mission budget. Proposed **harness** support: candidate-bound ablation
receipts and explicit timeout/process outcomes. Measure independently reproduced
causal claims, unaccounted variants, bound deviations and representation-sensitive
consumers alongside exactness; a successful final byte check cannot fill these gaps.

## Guard-return partial and the cost of repeated search

Parent `01a0c062`, records **1446–1494**, retains writer `e235c69e` and reviewer
`c0938fc8` for `world00/area030/04@0x801E0770`. Complete embedded reports and parent
arguments/results are reviewed; full child semantic review remains pending. Pins:
`tmp/observation-ingest/older-annotation-retirement-metrics.json`.

| Role / run | Started calls | Observed event-span seconds | Process / metadata acceptance |
| --- | ---: | ---: | --- |
| Writer `e235c69e` | 107 | 1,784.537 | Exit 0 / not-required |
| Reviewer `c0938fc8` | 59 | 2,081.457 | Exit 0 / rejected |

Together: **166 calls/3,865.994 summed seconds**, with eight unclassified error-end
events and both recorded count/duration fields missing. Review metadata fails
staged-file/evidence checks; its domain verdict is separately **repair**. The
reviewer's 2,081.457 seconds is about 35 minutes, not the parent's claimed 14-minute
runtime: **14m32s was only the final wait**. The two parent waits total 2,072.264
seconds and overlap child execution. Writer delivery to review dispatch took
193.230 seconds during the [annotation-retirement request](bof3-docs.md#annotation-retirement-without-transfer-evidence);
review dispatch to delivery took 2,091.711 seconds. A finalize steer preceded the
report by 882.075 seconds, without a phase trace establishing where that time went.

**The retained result is a partial.** A conjunction seed reportedly scored **6/56,
220→224 bytes**; separate early `return 0` guards and final `return dz < 0x400`
reached **12/55 (21.82%), 220→220 bytes, first +0x0000**. Review reproduced those
metrics and byte-match **exit 1**. Equal original/current counts of 55 do not mean
55 matching instructions. Original payload bytes at **0xFB70:0xFC4C** (220 bytes)
were independently decoded and hashed. The original reserves `$v0=0` in the first
guard's delay slot for later failure returns; the candidate's per-path zeroing
changes the register allocation. This supports the observed mechanism, not proof
that every eligible C form or compiler profile is exhausted.

The writer reported roughly thirty offline C probes, including single-return,
goto, nested, order and magnitude spellings; six classes survived in its residual.
Those are different search units, not thirty uniquely reconstructed experiments.
Review explicitly could not remeasure the six source alternatives under its
read-only constraint. O1/O3 and boolean-only diagnostics must remain separate from
eligible semantic candidates; a changed-behavior score cannot improve the frontier.

**Search history did not survive handoff accurately.** The writer reported a
negative flag search after regenerating the root compile database, plus a
`--time-limit 60 -j 6` permuter attempt with no improvement and terminated workers.
Review found those rungs missing from the source residual, repeated the 52-candidate
catalogue and four installed historical compiler searches, and reported no exact
match. Its best 30.51% no-delayed-branch result was rejected for the wrong structure.
Review's own permuter attempt returned no useful result and was terminated. Its
final `ps` grep supports that inspection, not a process-tree ownership proof.
Calling the next permuter rung **untried**, as the review and parent repair did,
contradicts both reported attempts. The accurate state is attempted with incomplete
useful-result evidence. Missing receipts and an unattempted experiment need different
follow-up actions; repeated searches are not automatically unnecessary when their
candidate identities or evidence are absent.

Five externs and a padding split exposing `s32` fields at **0x34/0x38** remained
target-local; review's offset arithmetic preserved **0x98** size. Two consumers
were identified, with no other consumer changes reported. Four prior functions
remained exact (**36/36, 27/27, 44/44, 80/80**), and target symbol/Splat gates passed.
The writer listed seven authored paths plus the generated root compile database;
review's “six paths” wording was inaccurate. Its **55 baseline additions** were an
aggregate dirty diff, not this entry's six newly recorded raw names. Existing
profile/header additions were correctly attributed to earlier entries. A scoped
whitespace check passed while the whole-tree check found seven unrelated EOF issues.

The historical user asked whether a full `just check` caused the slow review.
The parent's diagnostic counted **38 text mentions**, printed only twelve, and
then declared none were executions. That mixed-event substring scan cannot prove
absence from tool arguments or attribute duration by command family. The report
confirms expensive searches occurred, but supplies no exclusive phase timings.
The parent nevertheless edited two directives to distinguish scoped/full gates
and cap exceptional reviewer searches at approximately 120 seconds. This is a
recorded policy response, not demonstrated enforcement or measured time saved.
The next mission inherited the cap; its outcome requires separate evaluation.

One parent source-comment repair recorded the negative profiles and correctly
qualified the six C alternatives as writer-attested, but retained the inaccurate
“untried” rung. No subsequent independent review or native recheck of that repaired
state appears before continuation. The parent recorded `partial-documented`, then
scoped green **rc=0/8 seconds, 941 valid/zero invalid**, 37.454 seconds after the
review delivery. Analysis/index exits were suppressed and readiness was not shown.
Target **63 lifted/55 exact/8 partial/111 unlifted**, global **941/797/144/1,406**
were manually maintained coverage counts, not a fresh native census.

The five-entry area030 batch comprises **four native-exact entries and one documented
partial**, not a completed target. Ten children used **858 calls/8,667.543 summed
seconds**: writers 438/5,000.260, reviewers 420/3,667.283. All ten lack recorded
count/duration fields; 41 error-end events remain unclassified. The fifth review
required residual repair, and entry 2 required naming repair despite accepted
native bytes. These qualifications prevent calling all four exact results clean
first-pass acceptance or using this different-function batch as a causal comparison.

| Improvement destination | Concrete proposal | Acceptance measurement |
| --- | --- | --- |
| Skill directive | Before repeating search, reconcile candidate-bound receipts and classify each rung as unattempted, attempted without usable output, measured negative or exact; bound one unanswered review question | This case repeated profile searches and mislabeled two reported permuter attempts. Measure repeated equivalent probes, unresolved receipts and review repairs per comparable partial, retaining independent verification. |
| Reference material | Add a guard-return residual example with original/current placement, eligible alternatives, source/profile identities and explicit limits of a read-only review | Six C classes were writer-attested only. Report independently reproduced claims / applicable claims; preserve uncertainty rather than claiming exhaustion or inventing an untried rung. |
| Harness tooling | Extract executed tool arguments and phase timings by call ID; retain external timeout, descendant termination and source/profile hashes in search receipts | The 38-mention diagnostic supplied no execution denominator or phase attribution. Require complete executed-call coverage, distinct timeout/no-result/negative statuses and reconciled lifecycle versus wait durations. |

Compare later reviews of similar partials for cost, reproduced claims and repair
frequency before crediting the 120-second guidance with improvement. Native score,
review verdict, final-comment accuracy and process completion remain separate metrics.

## First logo pair and a duplicate review caused by ID confusion

Parent `01a0c062`, records **1499–1527**, contains two new exact results and the
next writer's launch/wait. Complete parent blocks, arguments and embedded reports
are reviewed; raw child semantic review remains pending. Reproduction:
`tmp/observation-ingest/older-logo-first-pair-metrics.json`. The enclosing parent
slice used **13 calls/1,583.783 observed seconds**, including 1,485.277 seconds of
waits, eleven reasoning blocks/8,223 characters and zero tool-error flags. Those
costs include the next entry's wait and are not a pure two-function completion time.

| Entry / role | Run | Calls | Observed seconds | Domain report |
| --- | --- | ---: | ---: | --- |
| `exe/logo@0x801CE7F4`, writer | `f8e3e5d8` | 85 | 495.507 | Exact 58/58, 232 B |
| Entry 1, reviewer | `4e186deb` | 55 | 357.890 | Accepted with notes |
| `exe/logo@0x801CEDA4`, writer | `1523c675` | 45 | 243.736 | First-seed exact 22/22, 88 B |
| Entry 2, first reviewer | `c2ff8fa1` | 53 | 256.131 | Accepted |
| Entry 2, redundant reviewer | `8f89942c` | 58 | 278.468 | Accepted |

Five runs total **296 calls/1,631.732 summed seconds**, not serial elapsed time.
All exited zero; both writers' metadata says not-required and all three reviews
say rejected for staging checks, with the first entry-2 review also missing the
no-staged-files evidence field. Recorded count/duration fields are absent in all
five; eight error-end events remain unclassified. Both functions received first
review domain passes, but notes and metadata rejection preclude an all-layer
success claim. These small exact functions are not a controlled comparison with
the preceding allocation partial or proof that the revised search cap saved time.

**Entry 1: exact bytes and corrected dispatch assumptions.** The writer resolved
“entry 1 (entry index 1)” to the first queue element, index zero, using prior
campaign conventions. The parent confirmed that intent and made the next prompt's
ordinal, array index and selector explicit. Review reproduced 58/58 and byte-match
exit zero, forced function/support objects to rebuild twice with stable hashes,
and checked payload **0x7F4:0x8DC** under load address **0x801CE000**. Two display
environments use **0x14 stride** and `isrgb24` at **+0x11**. The report's first
field address `0x801EB411` is an arithmetic typo: base `0x801EB480 + 0x11` is
**0x801EB491**; the second reported address `0x801EB4A5` is consistent. Preserve
the native proof separately from that explanatory error.

The review correctly found the header/support files were extended, but wrongly
charged the “new” wording to the writer. The retained writer delivery labels only
the source new; the **parent's review prompt** calls both shared files new.
Likewise, the parent said this was the target's first lift with no sibling regression
expected, although four existing byte-exact siblings were independently checked.
This is the batch's first entry, not the target's first source. Finding attribution
must retain which report or prompt actually introduced a claim.

Review counted **58 added baseline lines**, four logo-related and 54 across other
targets, then attributed the whole delta to this writer's global recorder. The
preceding reviews already describe aggregate baseline additions; without a
transaction-start diff, this does not prove the writer added those 54 lines.
Even one of the four logo entries, `D_801EB470`, was pre-existing debt. Global
recorder scope is real; this dirty-diff count is not a measured per-mission write
set. Passing symbol checks does not establish accepted ownership of every admitted
row. Non-EMI companion-check returned exit 2 for this and an existing executable
sibling; SDK-only callees made that check inapplicable here, not a native failure.
Parent scoped green was **rc=0/9 seconds, 942 valid**, 21.478 seconds after review.

**Entry 2: first-seed sibling reuse, with qualified provenance.** The writer
mirrored `rampCdMixLevel` with a signed countdown from `0x80` by two, yielding
`bgez` instead of the sibling's increasing-loop comparison. Both reviews reproduced
**22/22, 88 bytes, byte-match exit zero**, forced stable object/link rebuilds and
compared linked bytes with payload **0xDA4:0xDFC**. One review explicitly noted
that first-seed history remained writer-attested. Absence of retained search
artifacts cannot prove no earlier experiment occurred. The second review also
showed canonical strict metadata **exact/100/none**, one function record and
unchanged entry-1 native bytes. No new map row was required because it already
existed. Four changed paths were reported, including one raw-function-file admission.

Scope claims based on modification-time windows do not prove all unrelated bytes
were untouched. The first review's “22 instructions to `0x801CEDE4`” contradicts
its own 88-byte extent: the last word is **`0x801CEDF8`**. Its all-23-stale-snapshots
claim explains a reported coverage condition, not the cause of each stale state.
The cheap metadata gate's `uncompared` status was correctly kept separate from
successful native comparison. Both reviews nevertheless set `ladder_exhausted:true`
after exactness without an exhaustion experiment. Parent scoped green was
**rc=0/9 seconds, 943 valid**, 20.105 seconds after the second delivery; suppressed
analyze/index exits and no displayed readiness leave freshness separately unproved.

**The duplicate review was not a vanished worker.** Launch returned run
`c2ff8fa1` and mission `3cf0a6c4`. The parent passed the **mission ID** to `bg_wait`,
then searched archives/artifacts under that same wrong ID. “No active run matched”
therefore did not establish the actual child's termination or disappearance.
After **27.036 seconds from the first launch**, it dispatched `8f89942c` on an
unsupported infrastructure-failure diagnosis. Both complete reports subsequently
arrived, yet the queue cited only the replacement and the diagnosis was not corrected.

The redundant run incurred **58 calls/278.468 observed seconds**; this measures
extra execution, not the counterfactual reduction in end-to-end latency. The two
review lifecycles overlapped for **229.089 seconds**. Both reported deleting and
regenerating the same disposable object paths, exposing shared-build interference
risk even though authored files were read-only. No observed mismatch establishes
that a race actually corrupted these results. The first launch's real handle was
available throughout the erroneous recovery sequence.

| Improvement destination | Concrete proposal | Acceptance measurement |
| --- | --- | --- |
| Skill directive / orchestration handoff | Preserve run ID separately from mission ID, inspect the returned run before restart, and correct failed diagnoses when delayed results arrive | One wrong-ID wait, one unsupported replacement, two reviews of one entry. Comparable workflows must resolve every launched run and have zero duplicate launches caused by identity confusion. |
| Reference material | Give explicit selector/ordinal/index and a starting change inventory; attribute findings to the actual writer, parent or inherited state | Baselines: one ambiguous selector brief, two parent claims blamed on or projected onto the writer, and aggregate baseline deltas treated as mission changes. Measure corrected finding attribution and fully reconciled mutation sets. |
| Harness tooling | Make ID kinds explicit in launch/control receipts, distinguish wrong-kind lookup from terminal state, and expose overlapping disposable-build access | Preserve the actual handle on failed lookup; report duplicate-review calls/time and shared-artifact overlap. Verify no replacement begins solely because a different identifier has no match. |

These proposals preserve both exact results while separating search history,
metadata acceptance, source attribution and avoidable orchestration duplication.
No installed extension or harness change is made by this audit.

## Curated findings from mission reports

The [initial harvest audit](bof3-docs.md#initial-history-harvest) qualifies this
material's provenance. Parent records 1606–1632 selected 47 truncated excerpts
from 790 report files using phrase/numeric filters; this was not complete review
or 47 independently accepted experiments. Per-row source IDs were not retained
by that script, and shortening could discard the claimed measurement or target.
The later editorial findings below remain subject to full-source reconciliation.

Its 790 metadata dispositions included **678 lifters: 332 attested, 293 rejected,
53 not-required**; **90 reviewers: 65 not-required, 25 rejected**; and thirteen
namers, six workers, two cleaners and one generic reviewer. These are roles and
acceptance labels, not an RE task-outcome cohort. In particular, 332/678 is not a
majority, and the script did not establish why the rejected runs failed. Its
duration threshold could not measure lifter timeouts because all 678 lacked
recorded duration. Use the attributed cohorts and independent transcript costs
above; a larger metadata sample does not repair missing outcome evidence.

The [editorial-pass audit](bof3-docs.md#selective-filtering-and-semantic-summary-claims)
found 21 RE findings, not the claimed 18, synthesized from shortened previews.
Retain them as case-specific leads pending complete source reconciliation; they
do not establish comparative technique yields or new acceptance rules.

### Migration strategy

1. **Mirroring an exact sibling produced selected first-seed matches.** Seeds authored from an exact
   sibling reached first-seed exact with no variants (10/10, 37/37, 51/51, 100.00%); `func_801D1430`
   differed from its clone in one constant (`addiu $v0,$zero,0x10` vs `0x20`), and `func_801F1064` reached
   43/43 via the exact mirror sibling `func_801F0E1C`. Mirror the already-exact sibling file verbatim and
   append only the evidenced delta.
2. **Take the field-store order from the object's store offsets**, not from the prose, for one-shot wrappers.
3. **Re-decode base addresses, strides and bounds from the original payload** rather than inferring them.

### Branch, delay-slot and arm-order residuals

4. **Arm order in the source decides branch polarity and delay-slot placement.** `if (flag == 0) { A } else
   { B }` (fall-through = the original's *first* arm) matched where a nested-if did not; a flat
   `(kind == arg0 && flags != 0)` with `count = 0;` before it matched where the nested form failed.
5. **Check whether the gate comparison is already correct before reshaping it** — one gate byte was a
   zero-extended `u8`, so `sltiu` was right and the whole residual was arm orientation.
6. **A store sunk into the following jump's delay slot is defeated by a volatile view** (`69/74, 292->296,
   first=+0x008c`).
7. **Duplicating a call in both arms can be the fix**: repeating the identical effect-stop call per arm made
   gcc emit per-arm argument setup and tail-merge them into one `jal` → 100.00% (30/30 insns).

### Locals, typing and allocator residuals

8. **Local declaration shape drives frame layout and the register web.** `s32` return + `s32 result = 0` +
   `u32 flags` + a single `offset` local → 92/92 first-`-`; four separate `s32` saved-locals with direct cell
   spelling → 68.27% with depth/flag at `0x58`/`0x5C` and locals in `$s3`-`$s6`.
9. **A named width local can hoist its spill and inflate the frame** (the 339-instruction
   allocator/scheduler-sensitive block is the measured instance).
10. **A loop-invariant narrow-parameter promotion kept live in a register costs 8 frame bytes**; reading the
    parameter home through a volatile view fixed it.
11. **Deferred-decrement beats in-place decrement for a count that is also loaded** (`next = count - 1;` with
    the loaded byte preserved: 64/66 vs 54/66); simplification probes regressed (73/86, 79/88 — both reverted).

### Fixed RAM, scratchpad and symbol spelling

12. **A byte counter's load+store needs one materialized base** — `PSX_REF(u8,(u32)&SYM) += 1;` beside the
    sibling exact-gate idiom (first-seed exact).
13. **Take the base once through an explicitly non-volatile byte view** (`u8 *timer = (u8*)&phaseTimer;`);
    `volatile` defeats the delay-slot sink in finding 6.
14. **A leading published-cell read must be spelled directly** (`D_1F800044[6]`) with a `u8* work` local kept
    only for the post-call block.
15. **Re-materializing the symbol per access is not a defect to remove** — `lui a0,%hi(workCursor); lw
    a0,%lo(workCursor)(a0)` reproduces the original ("named symbol beats constant-address macro").
16. **A sibling's pointer-local idiom can be invalid elsewhere** (27/43) when an intervening call keeps the
    pointer live — check the call graph before reusing a sibling idiom.

### Dispatch, switch and boundary classification (harness-facing)

17. **gcc-2.x sparse switches still emit the full table** (11 labels spanning 0..50 → the 51-entry table;
    identical case bodies cross-jumped to the later body); a case-4 argument constant can be hoisted into the
    dispatch branch delay slot as the last residual.
18. **Boundary rules need precision/recall, not plausibility.** Against the marker proxy (502 code slots) the
    current rule selected 513 → 502 TP / 11 FP (precision 0.979); signal-only selected 508 → 502 TP / 6 FP.
    Ten `AREA176–185` payloads have 29 `jr $ra` **and** 29 analyzer starts inside `[4,0x6E0)`; all 12
    unconfigured payloads lacked `target.toml`, so the rule was applied to 35 payloads excluding every
    payload its own pilot validated. Splat layout: 365/365 splats start at offset 0, 193 use code start 4.

### Process and gates

19. **A shared-tree race is a build race, not a C error** (`watched input entry or ancestor changed`) — retry
    once, then report; never edit C to chase it.
20. **A scheduling-only residual is a legitimate stopping point** when the retained shape is the minimum
    change reaching identity and every rejected variant is recorded with its delta.
21. **Do not pipe the prefill to `head`** — it discards the target-evidence tail; re-run unpiped once.

## Editorial provenance and sweep baseline

The second RE rewrite used **three calls/121.153 lifecycle seconds** to compact
49 bullets to 27. Its native input and complete transcript are reviewed in the
[docs ledger](bof3-docs.md#second-loop-and-re-editorial-review); this is editorial
cost, not another reverse-engineering mission. Three merged rows retained their
witnesses, while nineteen rows were discarded, including two operational hazards.

The [loop ledger](bof3-lift-loop.md#important-observations) now preserves those
hazards: the claimed 30-run race cohort and a sweep that selected 84/90 over an
excluded 86/89 baseline. Original execution remains pending. For RE, the durable
acceptance question is whether the retained source matches the best eligible,
coherent candidate including the pre-sweep baseline. Record its source hash,
original/candidate sizes, instruction denominator, first mismatch and byte gate;
never count failed compilation as a valid negative experiment or successful
tool completion as improved retained code. This is a proposed measurement/gate
clarification, not evidence that current sweep tooling has been fixed.

### Recovered measurement and screening leads

The [first RE editorial review](bof3-docs.md#first-re-editorial-review-and-cohort-closure)
recovered the following from native session
`.pi/sessions/2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4/2dc53811-1df1-4511-80da-c30105d245a6/run-0/session.jsonl`.
Record/line locations below refer to native result bodies. These are historical
input claims; original executions and relevant implementation versions remain pending.

- Record 13, line 123 reports **41/44 positional versus 43/44 “multiset”/97.73%**
  for the same 176-byte candidate. Other fragments alternately describe positional,
  LCS and multiset scoring. Do not resolve that contradiction from labels: retain
  metric implementation/version, numerator/denominator, source/object hashes and
  byte sizes before comparing scores. Local diagnostics do not replace the native gate.
- Record 11, line 133 describes assembly left by the last sweep variant rather
  than the retained best candidate. Proposed sweep evidence criterion: every
  diagnostic artifact must identify its candidate hash; verify the retained source
  and object together before interpreting a residual or claiming improvement.
- Record 13, line 26 reports a screening false negative for
  `emi/scenario/scena00/00@0x801FB204`: word **0x34420180** at **0x801FB2C0**
  was decoded as `ori v0,v0,0x180`, despite the candidate being rejected by the
  div/break screen. Preserve this as a reproducer lead; measure screening false
  positives/negatives against pinned original bytes and an independently checked
  instruction classification, not candidate count alone.
- Record 13, line 60 observes that `EXIT=$?` after `tail` reported the pipeline
  tail's status, not the matching command's status. The reported **23/23, 92-byte
  MATCH** body and process exit are distinct evidence. Proposed reporting check:
  capture the owning command's exit directly and retain its complete result.

These proposals target matching references and sweep/screening/reporting owners;
none claims a tooling fix or authorizes historical matching aids under current rules.

## Important observations

53 summarized observations.

- emi/world00/area030/05@0x800F50B4: first-pass lift matched 22/22 instructions and bytes.
- battle03@0x801E6C20: a block-local u16 pointer for halfword read-modify-write eliminated duplicate address materialization and recovered the call delay slot; 76.92% → 100%, unchanged 100 bytes.
- battle/03@0x801E50C0: inverting the two-arm test to != 0 made the large arm fall through; 84.51% → 100%, unchanged 284 bytes.
- sce10eff@0x801D165C: reading the narrow parameter home through a volatile s16 view inside each arm removed a loop-invariant promotion's saved-register lifetime; 367/449 → 382/447 instructions, frame 0x78 → 0x70. Eight further clean-C variants did not improve the retained 382/447 (85.46%).
- src/bof3/world/drawTexturedFrame.c, drawTexturedFrame@0x801F3D88: sibling local/register shape improved 200/339 → 272/339 (59.00% → 80.24%), restored the 0x78 frame from 0x70, and moved the first mismatch from +0x0000 to +0x0074 (1356→1348 bytes). Moving the width-local u16 conversion beside the sub-step call reached 278/339 (82.01%) at the same size and first mismatch. Separately, emi/world00/area008/13@0x801F3D88 statement reordering improved 317/339 → 326/339 (96.17%), 1356/1356 bytes, first mismatch +0x154; still partial.
- scena00, high-halfword alias D_80143F86: reading the just-stored word's high halfword through an address-derived byte view of the same symbol prevented hoisting above the store; 261/263 → 263/263, unchanged 1052 bytes. A separate fixed-address alias D_80143F86 did not preserve the ordering.
- emi/etc/sisyou/00@0x801D2A4C: mirroring exact sibling func_801D28B0 with only the evidenced equality test, mode index 4, and action +9 produced first-seed 70/70 instructions and 280/280 bytes.
- exe/logo@0x801CEBFC: a bottom-tested polling loop preserved the duplicated constant and backward-branch load; 35/35 instructions and 140/140 bytes.
- emi/etc/shop/00@0x801E2D5C: a separate signed_next temporary preserved the signed 16-bit conversion used by slt; 20/22 (90.91%) → 22/22 instructions, unchanged 88 bytes, with no matching aid.
- Recorded register pins resolved allocator residuals: emi/etc/game/00@0x801ADC98 result in a0, 20/23 with 92→88 bytes → 23/23 with 92/92 bytes; emi/battle/battle/03@0x801E60DC slot pointer in v1, 8/30 → 30/30 at 120 bytes after lifetime-order attempts failed; emi/battle/battle/15@0x800AF66C result in v0 recovered the entry register web, 19/19 at 76 bytes, byte-exact without a flag override.
- exe/slus_004_22@0x80162B08: the documented 66/67 residual was a jump delay slot containing li v0,1 instead of nop; a later live check found 67/67 instructions and 268 bytes exact, superseding the stale partial claim.
- exe/slus_004_22@0x80162500: a same-size 55/57 (96.49%), 228-byte partial differed only in the loop scheduling of addiu a1,a1,-1 and bnez; no allocation or control-flow change was observed.
- emi/battle/battle/15@0x800A8360: 36/39 instructions, 152→156 bytes, first mismatch +0x08; the residual combined reversed 1/8 constant materialization with an extra absolute-address lui for the late field_6C store.
- emitRadialTranslucentQuads@0x801D240C: the recorded partial reached 146/147 (99.32%) at unchanged 588 bytes; the sole residual at +0x74 scheduled a1 initialization after, rather than before, s2/scratchpad-radius setup.
- emi/scenario/scena16/00@0x801F83B0: a live spelling-only baseline remained 21/43 (48.84%), unchanged 172 bytes, first mismatch +0x14; byte comparison confirmed non-exactness.
- emi/battle/battle/15@0x800A41D8: declaring func_800A3A10 with an 8-bit return omitted the original post-call andi 0xFF; the result was 22/24 (91.67%), 96→92 bytes, first mismatch +0x2C rather than exact.
- emi/scenario/sce10eff/00@0x801D218C: the unchanged live partial was 154/160 (96.25%), 640/640 bytes, first mismatch +0x60; symbol and Splat checks passed while byte comparison failed.
- emi/world00/area030/04@0x801E0044: the post-collision baseline was 15/40 (37.50%), 156→160 bytes, first mismatch +0x10; the best clean-C candidate reached 37/39 (94.87%), 156/156 bytes, first mismatch +0x5C.
- emi/world00/area028/13@0x801F2D3C: 115/136 (84.56%), 544→472 bytes, first mismatch +0x24; only two missing sign-division trap sequences and three shifted branch targets differed, while the body, loop, and tail otherwise matched.
- emi/scenario/scena00/00@0x801FA7D8: a 485-instruction text region still failed the reviewed read-only-data placement gate because jump-table entries 5 and 9 differed; the case-4/5 and case-8/9 boundaries were each displaced by one instruction.
- emi/etc/sisyou/00@0x801D0DD4: replacing an empty stub improved 2/182 (1.10%), 728→8 bytes, to 180/182 (98.90%), 728/728 bytes. Thirteen measured clean-C control-flow shapes left the mirrored branch/jump pair unresolved at +0xB0.
- emi/battle/battle/03@0x801DA7D4: the retained partial measured 109/196 (55.61%), 784→776 bytes; both GCC 2.7.2 and 2.6.3 removed the original's surviving dead accumulator.
- emi/etc/game/00@0x801C3154: splitting the destination base moved a 43/44 (97.73%), 176-byte residual from +0x50 to +0x5C, leaving only commutative addu operand order; duplicated destination derivation then reached 44/44 and live byte-exactness.
- emi/battle/battle/15@0x800A0A40: hoisting table offsets into word locals while retaining the declared volatile halfword store changed 69.49%, 236→232 bytes, to 59/59 instructions and 236/236 bytes.
- emi/etc/game/00@0x801C3598: indexing the global directly at the reload instead of reassigning the local corrected the cross-block work-pointer allocation from a0 to v1; 41/45 (91.11%), first mismatch +0x54, → 45/45 and byte-exactness.
- emi/scenario/scena00/00@0x801FC3A0: the recorded baseline was 183/269 (68.03%), 1072→1076 bytes, first mismatch +0x0000; move a3,zero was hoisted above the preheader loads rather than filling the blez delay slot.
- emi/battle/battle/15@0x8009CF80: 25/27 (92.59%), unchanged 108 bytes, first mismatch +0x10; the sole residual was a jump-table base pair using 0x80096954 instead of 0x8009CFF0.
- emi/world00/area008/13@0x801F3D88 improved from 317/339 (93.51%) to 326/339 (96.17%) over two 20-attempt passes, with equal 1,356-byte size and the first mismatch moving from +0x00DC to +0x0154. Earlier entry-copy diagnosis was superseded by three scheduler-placement clusters. At this frontier, an earlier u3 store preserved 326/339 but moved the first mismatch backward to +0x0150; prefix or discarded-postfix y2/v2 decrements regressed to 324/339, reordered positive adjustments to 319/339, and reused dead geometry locals to 318/339.
- emi/etc/game/00@0x801C3154 had two clean-C candidates at 43/44 (97.73%); the split-base form moved the first mismatch from +0x50 to +0x5C. Independent review rejected exhaustion wording that omitted this better frontier.
- emi/battle/battle/15@0x800AF5FC improved from 5/28 (17.86%) to 9/28 (32.14%) at the same 112-byte size and +0x0004 first mismatch by computing the plane bound after both axis bounds and inlining both shifts. Compiler-profile results for the replaced shape did not establish results for the improved shape.
- emi/world00/area030/04@0x801DB620 improved from 68/70 (97.14%) to 70/70 (100%) by expressing the byte gate as byte > 1 rather than byte < 2 and swapping the arm constants, restoring branch polarity and the delay-slot constant.
- emi/battle/battle/15@0x8009B160 improved from 35/43 (81.40%) to 36/43 (83.72%), at 172 bytes, by correcting D_801485BC from u16 to s16; the -170 constant then matched. A diagnostic one-to-a0 pin reached 95.35% but remained non-exact and was removed.
- emi/battle/battle/15@0x800AC704 remained a clean-C partial at 19/20 (95%), 80/80 bytes, with one commutative addu operand-order mismatch at +0x1C; checkpoint attempt 3 made no observable change and retained attempt 1 as best.
- emi/world00/area027/13@0x801F3650 improved from 10/16 (62.50%), 60 bytes, to 12/16 (75%), 64 bytes, with an explicit equality chain. GCC still reversed the first equality branch and tail-merged the value-5 assignment; the flag matrix, four historical compilers and corrected-context permuter produced no valid improvement.
- emi/world00/area027/13@0x801F36D0 reached 16/16 instructions and 64/64 bytes with a clean-C s32 local and switch, without matching aids.
- emi/world00/area024/14@0x801F3250 improved from 33.33% to 48.48%, at the original 132-byte size, by separating initialWork, timerWork and clearWork lifetimes. A direct-only variant regressed to 24.24% and 128 bytes and was reverted.
- emi/world00/area030/04@0x801DAE3C retained an equal-size 15/18 (83.33%), 72-byte candidate with a gate/constant register and store-scheduling residual. A comma-expression initializer emitted identical assembly; clean-C, profile, permuter and one diagnostic-pin probes remained non-exact, and the pin was removed.
- emi/etc/game/00@0x801968BC matched 70/159 (44.03%), with 612 original bytes versus 636 generated bytes. Its original jump table had five entries (0x14 bytes) versus six (0x18 bytes) in the candidate; the first relocation mismatch did not account for the broader CFG and size divergence.
- emi/etc/game/00@0x8019625C improved from 48.85% to 49.43% after signed loop-bound correction, reaching 86/174 instructions, 704 original bytes versus 668 generated. The first mismatch was ori versus addiu address materialization at +0x0014; broad CFG/size divergence remained, rather than an isolated allocator residual.
- emi/etc/game/00@0x80196B9C improved from 26/60 (43.33%), 240 bytes, to 58.33%, 232 bytes. Expanded-division comparison reached 62.30% but generated 244 bytes; an a1 pin regressed to 56.67% and 228 bytes and was removed. The retained candidate still had index allocation and an unwanted andi at +0xC.
- emi/etc/game/00@0x8019EB38 retained 32/58 (55.17%), 232/232 bytes, first mismatch +0x20. Route-index temporaries regressed to 36.21%/228 bytes, coordinate-result temporaries to 31.03%/220 bytes, and a counter move to 43.10%; all were reverted. GCC 2.95.2 reached 65% but was non-exact.
- emi/etc/shop/00@0x801DAB90 retained 182/339 (53.69%) with a 0x70 frame versus the original 0x78. Historical compiler results were 58.11% for GCC 2.6.3, 51.62% for GCC 2.8.0/2.8.1 and 35.40% for GCC 2.95.2; the bounded permuter also remained non-exact.
- emi/battle/battle/03@0x801E62BC reached 60/65 (92.31%), with 260 original bytes versus 256 generated, but retained a signed-load/allocator-lifetime residual after clean-C, flag-search and bounded-permuter probes.
- emi/scenario/scena16/00@0x801F6D90 improved from 16/40 (40.00%) to 17/40 (42.50%) with a signed int view that changed sltiu to slti; the 160-original/152-generated-byte CFG/frame mismatch remained.
- exe/slus_004_22@0x8014E0FC attempts 2 and 3 both measured 42/82 (51.22%), 328 generated bytes versus 304 original, with the same first mismatch at +0x0004. Review blocked the claim that attempt 3 improved the checkpoint.
- emi/battle/battle/15@0x800A3638 required only canonicalizing the exact residual metadata: post-repair checks still measured 20/20 instructions and 80 exact bytes. A broader review reportedly found noncanonical residual-none metadata in 730/939 sources; its identity and relationship to this target review remain unverified.
- exe/logo@0x801CE760 measured 37/37 instructions and 148 bytes despite documentation claiming unresolved status.
- exe/logo@0x801CEA98 reached a clean-C first-source exact match at 89/89 instructions and exact bytes.
- Ability-selectability at 0x801AF5B0 remained partial: 68/148 matching instructions (45.33%), 592 original versus 600 current bytes.
- Level-growth at 0x801ADDD4 remained partial: 108/272 matching instructions (39.71%), 1,088 original versus 1,080 current bytes.
- emi/etc/game/00@0x801A7C2C matched 32/32 instructions and 128/128 bytes in live checks.
- emi/world00/area008/13@0x801F3D88 reorderings improved 317/339 to 323/339 matching instructions at 1,356 bytes; UV-local, frame and pin-removal alternatives regressed. A later receipt measured 326/339 (96.17%), 1,356 bytes, first mismatch +0x154; review found this selector-specific receipt misplaced in durable guidance.
- exe/slus_004_22@0x8015DF18 (dispatchSoundCue) selected gcc-2.6.3-psx and matched 671/671 instructions and 2,684 bytes, contradicting documentation claiming no object selected a compiler variant.
