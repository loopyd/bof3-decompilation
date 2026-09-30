# Observation folder

Central, per-skill self-improvement mechanism for the BOF3 skills. **One file per skill.** Skills carry
**directives**; this folder carries the **measurements** behind them. Nothing here belongs inside a
skill: a skill that accumulates a ledger stops reading as a contract.

## The loop

After each mission, for the skill that owned it:

1. **Audit performance** from run metadata (`agent`, `status`, `toolCount`, `durationMs`) and task
   results. Record completed/accepted work units and their denominator, then calculate throughput per
   wall minute or hour; separate active time from waits when they matter. For unfinished work, estimate
   remaining time from the observed rate and remaining units, naming the sample, content mix and
   uncertainty. Keep outcome quality beside speed; a faster rate alone is not an optimization.
2. **Add one row** to that skill's *Iteration audit log*: mission, tools, duration, method verdict, and
   where the finding converged.
3. **Converge it** into a directive in `.pi/skills/<skill>/` (its `SKILL.md`, a reference, or its agent
   instructions). A row that changes nothing is itself a finding — record it as one.
4. **Harness fixes or additions** bundle an **execution directive** (what to change, where) with an
   **acceptance criterion** (how it is proven). Without both it is an observation, not a directive.

## What each ledger holds

Per skill, in one file: measured performance, iteration audits, durable lessons,
and evidence-backed improvement proposals pointing to the owning directive,
reference or harness component. Historical measurements are not current acceptance.

## Full-history ingestion status

**In progress; full semantic coverage is not yet established.** The earlier harvest
selected measured paragraphs, truncated them before filtering, and omitted the
legacy `.pi-subagents` tree. Its counts cannot establish complete ingestion or
prove that qualitative lessons were absent. Preserve its curated findings while
reconciling them against complete source records.

The current inventory covers `.pi/sessions`, `.pi-subagents`, and
`~/.pi/agent/sessions/--mnt-PROJECTS-repos-rebof3-simple--`: parent and child
JSONL, final output reports and run metadata. `sessions` is an alias of
`.pi/sessions`, not another population. A header audit of the other 1,926 global
JSONL files found no additional `rebof3-simple` working directories. Foreign
projects are excluded by provenance, not by incidental BOF3 vocabulary.

The 2026-09-26 inventory parsed **20,506 files**, with zero parse errors: the
initial 18,156 (9,814 JSONL, 4,171 output reports, 4,171 metadata files), plus
1,913 supplemental Markdown artifacts and 437 project-qualified runtime files.
The runtime files cover 123 child/standalone runs and 18 workflow parents under
`/tmp/pi-subagents-uid-1000/async-subagent-runs`. All 123 children already have
metadata identities; they add evidence, not runs. Metadata deduplication found
4,171 unique runs and no conflicting copies. An unchanged-input check protects
resumption. This snapshot does not establish complete semantic ingestion.

The resumable working inventory is `tmp/observation-ingest/coverage.sqlite`;
`inventory.py`, `backfill.py`, `metrics.py` and `transcript_metrics.py` in that
directory record source hashes, exact locations, extraction coverage and metric
denominators. It stores transcript pointers rather than transcript bodies.
Parsing and keyword candidate selection are not semantic review. Review every
relevant assistant text/reasoning block, tool result and child report; retain
concise conclusions with provenance, not raw reasoning or tool payloads.

The earlier 2026-09-26 snapshot covered **8,682/356,831 unique blocks (2.433%)**;
the changed source population and active tail mean each percentage must retain
its own denominator. At **2026-09-27 01:56:04 UTC**, semantic review covers
**10,760/357,163 unique blocks (3.012630%)**, with **346,403 pending**. Full
semantic ingestion is still in progress; this is not a completion claim.
At **2026-09-27 02:25:33 UTC**, after verifying ten pinned child-session
transcripts, coverage is **10,858/357,163 (3.040069%)**, with **346,305
pending**. This remains a partial semantic-review snapshot.
At **2026-09-27 02:50:42 UTC**, after reviewing 17 unique blocks from the pinned
`.pi-subagents/sessions/ten-wave/area030/run-0/session.jsonl` source (SHA-256
`19880f9d9c08829d28bafabf178d05e4e9d2f64b033697d086ad865d5079ab90`), coverage is
**10,875/357,163 (3.044828%)**, with **346,288 pending**. Those 56,793 characters
were untimed; full semantic review remains incomplete.

The active 2026-09-25 parent source
`.pi/sessions/2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4.jsonl`
grew append-only from a verified 10,974,605-byte prefix
(SHA-256 `b111ae4e5cc9320e27dbf3ac51ac078b44350720974f8d54f827b3f9b966fc78`).
The first 668,518-byte append contained 201 records and 255 new unique blocks /
367,102 characters, with 8 repeated block occurrences and zero parse errors;
its 0.211-second reconciliation measured 72,511.8 new blocks/minute. A second
verified append extended the intermediate 11,643,123-byte pin (SHA-256
`40e8500c61545a9d44898e0ccd7ff36d3447de218fcaa047fc5fa952b2d2b5d1`) by
244,961 bytes, 69 records, 77 new unique blocks / 100,532 characters and 7
repeated occurrences, also with zero parse errors; its 0.202-second
reconciliation measured 22,871.3 new blocks/minute. The current
source is 11,888,084 bytes (SHA-256
`b68b7d2145d116cd1da12a88c984ed271f72c1217e09355e1d8fb78b977b5b49`). Across
both append passes, 270 records and 332 new unique blocks / 467,634 characters
were indexed in 0.413 seconds: **48,232.4 new unique blocks/minute** and
**67,937,142.9 characters/minute**. Each duration includes prefix verification,
parsing/indexing, and a full-source hash; these are observed parser/index
reconciliation rates, not pure parser CPU rates or semantic-review speed. The
first append was 63.4% larger in bytes, yet its elapsed time was only 4.5% longer.
This makes per-phase timing of prefix/full hashing a useful optimization test,
while preserving the existing hash-verification contract. All **332 unique blocks**
introduced by the two verified appends have now been semantically reviewed; the
active source tail through record 2580 is fully checkpointed at this snapshot.

Cycle 3 completed **1,000/1,000**: 989 timed blocks / 1,848,450 characters in
4,476.201 seconds (**13.257 blocks/minute; 24,777 characters/minute**), with 11
untimed blocks. Cycle 4 completed **2,000/2,000**: 1,959 timed blocks /
3,292,144 characters in 7,681.236 seconds (**15.302 blocks/minute; 25,716
characters/minute**), with 41 untimed blocks. Cycle 4's descriptive rate was
15.4% higher in blocks/minute and 3.8% higher in characters/minute; the content
mix differed, so this is not a causal speedup claim. The previous 750-block cycle
had 21 untimed blocks; its 729 timed blocks / 1,261,023 characters took 2,556.81
seconds (**17.107 blocks/minute; 29,598.5 characters/minute**). The earlier
cycle's timed slice was 373 blocks / 782,630 characters in 1,514 seconds
(**14.8 blocks/minute; 31.0k characters/minute**); its first 128 blocks were
unmeasured.

The user-requested cycle 5 target is **5,000 blocks**, up from cycle 4's 2,000.
At **2026-09-27 01:56:04 UTC**, it is **2,078/5,000 complete (41.56%)**; 200
blocks were outside the cumulative timed segment. A separate measured window
reviewed **122/122 blocks and 11,047 characters in 48.476 seconds** (**151.003
blocks/minute; 13,673.2 characters/minute**). These were tool-output summaries,
averaging 90.5 characters per block. The high block rate with lower character
rate reflects short, repetitive units; it demonstrates that larger batches are
feasible, but is not comparable to prose- or reasoning-heavy samples. At this
single-cohort rate, 2,987 cycle blocks project to 19.8 minutes and the full
346,468-block backlog to 38.2 hours; keep these as content-mix sensitivity only.
At **02:25:33 UTC**, cycle 5 is **2,176/5,000 (43.52%)**, with 298 blocks
outside the cumulative timed segment.
At **02:50:42 UTC**, it is **2,193/5,000 (43.86%)**, with 315 untimed blocks and
2,807 remaining. The clean 29.235-block/minute sample projects **96.0 active
minutes** for this cycle remainder and **197.4 active hours** for the 346,288-block
backlog. The additional blocks are untimed and do not change either rate.

A second separate window reviewed **24 blocks / 128,478 characters in 290.270
seconds** (**4.961 blocks/minute; 26,557 characters/minute**). Two large outputs
were near-duplicates: one 51,113-character tool output was a prefix of a
51,211-character output after credential redaction. This makes 51,113 characters
(39.8% of this window's text) overlapping content; the remaining visible text
was 77,365 characters (**15,991.7 characters/minute**). The window includes
documentation, C source, assembly and repeated command output, so its low block
rate and character rate are a content-mix sensitivity, not a general review
baseline. At this rate, 2,963 cycle blocks project to **597.3 minutes** and the
346,444-block backlog to **1,163.9 hours**.

A third separate window reviewed **22 blocks / 71,313 characters in 164.549
seconds** (**8.022 blocks/minute; 26,003.1 characters/minute**). Its largest
context output was aligned against a previously reviewed 51,211-character
output; 13 changed regions comprised 16,003 characters. The shared material
included RE guidance and source/assembly context, so this alignment-assisted
sample is also content-specific. At its rate, 2,941 cycle blocks project to
**366.6 minutes** and the 346,422-block backlog to **719.7 hours**.

The last clean long-window sample, at 00:46:11, covered 1,716 blocks / 2,146,026
characters in 3,521.767 seconds (**29.235 blocks/minute; 36,561.6
characters/minute**). Batches 8–17 add a separate 133-block / 159,730-character
window in about 199 seconds (**40.1 blocks/minute; 48,142 characters/minute**),
derived from rounded cumulative checkpoints. That short window is heavy in
repeated notices and one large workflow report, so it is not a comparable
steady-state sample.

The cumulative tracker rate of 26.886 blocks/minute includes an interval when already
read batches were checkpointed alongside documentation work; preserve the raw rate,
but exclude it from active-review recommendations. Using the longer clean sample,
the remaining 2,824 cycle blocks project to **96.6 active minutes** and the
346,305-block backlog to **197.4 active review hours**. The new 98-block
context-heavy window projects **387.4 minutes / 791.8 hours** at its raw indexed
block rate; earlier large-output and alignment-assisted windows project
**569.2 minutes / 1,163.0 hours** and **351.9 minutes / 719.2 hours** for this
same remaining workload. These are content-mix sensitivity scenarios, not
confidence limits. Use the longer clean sample for planning until comparable
cohorts are available. All estimates exclude source reconciliation,
documentation, validation and future tail growth.
Keep the 5,000 target through cycle completion, then size the next cycle from a
completed, comparable sample.

A fourth separate window reviewed **19 indexed blocks / 125,411 serialized
characters in 369.753 seconds** (**3.083 indexed blocks/minute; 20,350.5
serialized characters/minute**). One 50,987-character truncation body exactly
matched the prefix of its enclosing 51,085-character tool output; only a
98-character clipping trailer differed. After deduplication, this was **18
content-distinct blocks / 74,424 characters** (**2.921 blocks/minute; 12,076.8
distinct characters/minute**). The repeated body is **40.66%** of serialized
characters. This context-heavy window projects 2,922 remaining cycle blocks to
**947.7 minutes** and the 346,403-block backlog to **1,872.6 hours**; treat
those as a high-cost content-mix sensitivity, not the planning ETA. Count indexed
blocks and distinct content separately. The measured overlap makes a concrete
parser optimization candidate: when a truncation body is an exact prefix of the
enclosing tool output, retain truncation metadata but avoid indexing its body
twice. Acceptance should prove byte-for-byte source coverage and zero loss of
truncation status while reporting unique bodies and transferred characters.

A fifth separate window reviewed **98 blocks / 825,660 serialized characters in
806.711 seconds** (**7.289 blocks/minute; 61,409.3 serialized characters/minute**).
Fourteen oversized outputs contributed 716,827 characters. Exact alignment
against already-reviewed text found at least **638,361 repeated characters**;
the unmatched 78,466 large-output characters are a conservative upper bound
because the matcher may miss additional repeats. Including all 108,833
characters from smaller blocks gives an alignment-assisted review-volume upper
bound of **187,299 characters** (**13,930.6/minute**). Three nested truncation
prefixes account for 153,477 of the repeated characters. This window is a
repeated compiler/checkpoint/cleanup review of one function, not a steady-state
planning cohort. Ten child runs used 83 event-derived tool calls over 685.563
summed seconds; five reports passed, three needed fixes, one escalated and one
blocked, while exact-lift acceptance remained 0/1. The contrast between 7.289
raw blocks/minute and the longer clean 29.235-block/minute sample shows why
throughput estimates need content mix and outcome denominators. The measured
repeat volume supports two specific experiments: collapse verified nested
truncation bodies while preserving both pointers, and stream large blocks in
bounded redacted chunks instead of rejecting an oversized display request.
For that 98-block cohort, omitting at least 638,361 repeated characters would
remove 77.3% of serialized volume. At the measured 61,409.3 characters/minute,
this is at most **10.4 active minutes** of payload-proportional effort per
comparable cohort (about 13.45 minutes observed versus 3.05 minutes modeled for
the 187,299-character distinct-content upper bound). This is a prioritization
ceiling, not a measured speedup: all source occurrences still need attribution,
and review time is not purely character-proportional. A paired same-cohort test
should compare active seconds, reviewed blocks, distinct characters and outcome
quality while preserving every source hash, pointer and clipping marker.

At **2026-09-27 05:51:36 UTC**, cycle 5 reached **5,056/5,000 blocks
(101.12%)**, 56 blocks over target. Global semantic-review coverage is
**13,738/372,248 (3.69055%)**, with **358,510 pending**; full coverage remains
incomplete. The latest pinned parent cohort
`.pi/sessions/2026-09-23T00-57-03-517Z_01a0cbc4-211d-7757-881d-33ed2fcd42d7.jsonl`
(SHA-256 `fd34915b19d843ee3a29611c98d53e24938f5e4baa9344c6c7e866c8010aa2bc`,
records 1764–2115) reviewed **450 unique blocks / 698,641 characters** in
**394.827 active seconds**: **68.384 blocks/minute** and **106,169.2
characters/minute**. The 32 display groups were tool-output heavy: 171 tool-output
blocks / 261,004 characters, 154 reasoning blocks / 419,054 characters, and 125
assistant-text blocks / 18,583 characters. All 32 checkpoint records reconcile to
the cohort totals and the reopened inventory verifies the commit. One duplicate
checkpoint retry was rejected before it could create a second commit.

From the timer start at 05:40:08 to the verified commit at 05:51:36, the broad
window was **688 seconds** (**39.244 blocks/minute; 60,928.0 characters/minute**).
The **293.173-second** difference from active review is unallocated workflow time,
not measured checkpoint cost. The earlier 400-block cohort ran at 73.5
blocks/minute and 121.4k characters/minute; this 450-block sample is about 7.0%
lower in block rate and 12.5% lower in character rate, but the content mixes differ,
so batch-size causality is unknown. At this sample's rates, the 358,510-block
backlog projects to **87.4 active hours** (active-review rate) or **152.3 hours**
(broad rate); these are content-specific sensitivities, not planning ETAs. The
longer clean 29.235-block/minute sample remains the planning baseline, implying
**204.4 active hours** for the current pending count before future tail growth.

Historical quality accompanies these speed measures: the reviewed mission verified
**59,105/59,105** indexed instances and four parity queries, while the source also
records a bank-relative offset defect and two report-count/label mismatches that
were corrected before final verification. For the next throughput comparison,
record each group's semantic-review and checkpoint start/end times separately,
then compare matched 400/450/500-block cohorts with block/character mix, duplicate
work, integrity and repair counts. Do not promote the observed rate difference to
a batch-size optimization until that comparison reproduces it.

At **2026-09-27 16:30:58 UTC**, the reopened coverage database contained **21,102
source files** (zero parse errors) and **379,196 unique blocks**. Its block table
shows **18,910 reviewed (4.986867%)** and **360,286 pending**. The companion
`progress.json` instead reports 18,950 reviewed (**4.997416%**) with the same
pending count, a 40-block inconsistency. Until reconciled, report global progress
as **about 4.99%** and retain both exact counts; do not claim a single reconciled
numerator. Cycle 6 is a separate work target at **5,212/6,000 (86.8667%)**.

A separate run-metadata refresh generated at **2026-09-27 17:01:35 UTC** scanned
**4,527 `_meta.json` files** dated through 16:56 UTC: 4,527 deduplicated runs,
zero metadata parse errors and zero conflicting copies. Attribution is **261
explicit skill selections**, **3,228 historical-role candidates** (unverified),
and **1,038 unattributed runs**. The explicit selections are 260 `bof3-re` and
one `bof3-docs`; the other 20 skills have no explicit metadata selection in this
scan. This does not rule out parent-level skill work or prove absence from a
skill's history. Tool-call and duration fields are each present for 3,186 runs;
**218 child-report parses failed**.

For `bof3-re`, 188/260 explicit runs have recorded tool counts and durations:
5,011 calls total (median 23, p90 44, max 128) and 32,248.660 summed seconds
(median 138.050, p90 332.056, max 837.702). Run metadata labels 68 checked,
35 rejected, 15 attested and 142 not-required; these are not accepted task units.
The single explicit `bof3-docs` run exited 1 after four failed generations and
recorded zero tool calls, so it cannot establish a normal performance rate. For
skill optimization, repair the missing run fields and report parser errors, then
pair run cost with task-unit outcomes; the current metadata alone cannot support
accepted-task throughput or a skill-completion ETA.

The same-source Cohort Y pool for windows 3–22 recorded **200 accepted blocks in
9,217.786 operational seconds** (**1.302 accepted blocks/minute**). At that pooled
rate, the **788 blocks remaining to the Cycle 6 target** project to **605.2 minutes
(10.1 hours)** of operational time. Positive-yield windows ranged from **0.265 to
22.126 accepted blocks/minute**, projecting **35.6–2,973.6 minutes**; this is a
descriptive range, not a confidence interval, and eight windows yielded zero
accepted blocks. It is a conditional cycle estimate, not a full-corpus ETA.

Window 22 screened **135 blocks / 230,077 indexed characters in 768.989 seconds**
(**10.533 screened blocks/minute; 17,951.6 indexed characters/minute**) across
14 display groups for 35 batches, a **60% reduction** from one display per batch.
It accepted **zero blocks** because all 135 were quarantined. One 66,979-character
block was initially limited to a 15,000-character prefix; a later hash-verified
continuation read its remaining 51,979 characters in four slices, outside W22's
timed interval. This is ingestion throughput, not a skill-runtime benchmark. The
call reduction did not improve accepted-work throughput, so the next optimization
comparison should measure accepted blocks and characters per operational minute,
quarantine yield and display calls together. A smaller, content-isolated review
unit is a candidate to test if it can preserve quarantine and provenance rules;
no speedup is established yet.

At **2026-09-27 17:49:15 UTC**, the coverage database has **19,141/379,196
reviewed unique blocks (5.047785%)** and **360,055 pending**. Before refresh,
`progress.json` still had 18,950 reviewed and 360,286 pending. At 16:30, its
reviewed count was already **40 above** the database's 18,910; by refresh, the
database had advanced another 231 blocks, leaving the file **191 behind on
reviewed** and **231 high on pending**. The old values, database snapshots and
hashes are preserved in `coverage_reconciliation_history`; current progress
counters now match the database, though this manual refresh does not establish
automatic synchronization. In the `bof3-re` transcript review, 231 unique blocks / 296,590 characters took about
**716 seconds** across 13 parsing cycles (mean 17.8, maximum 47 blocks): **19.35
blocks/minute** and **24,843 characters/minute**. A 32 KB source-character
packing simulation reduces 13 cycles to 10 (23%), but has no measured time
effect. The linked child lift used 837.702 seconds and produced **zero accepted
exact lifts**, so review throughput and accepted-lift throughput remain separate.
See the [RE observation](bof3-re.md#2026-09-27-bof3-re-transcript-screened-throughput-and-zero-accepted-lifts).

Parser/indexing throughput and semantic-review throughput describe different
work. Use the parser rate to evaluate source scanning, hashing and database
indexing; use the timed semantic-review rate for coverage ETA and review-cycle
sizing. Neither rate substitutes for lift-mission throughput. Qualitative lessons
do not require a numerical measurement to survive.

Remaining work: reconcile embedded child deliveries and older artifact formats;
complete semantic review and skill attribution; fill each ledger's coverage gaps;
deduplicate lessons and link proposals to measured failure/cost populations.
The current catalogue contains **22 skills: 10 BOF3/domain skills and 12
`openspec-*` skills**, each now represented below. The prior count of 13 OpenSpec
skills was incorrect. File presence does not imply completed ingestion: the
OpenSpec onboarding ledger contains one fully reviewed episode; the proposal
ledger contains a 52-call program with four deliveries, a nine-call naming proposal,
and implicit guard-fix, registry-repair and evidence-refresh proposals. The apply
ledger covers seven parent cases through the approved naming scope split and the
eighth through evidence refresh, the first campaign's stop, its pilot and narrowed-scope
verification closeout. A ninth contextual apply case reaches the successor's
first-row capability pause; a tenth covers the capability follow-on's launch and
first workflow's blocked acceptance, a subsequent canonical-proof reproduction,
and owner-body launch, decoder corrections, capture diagnosis, timeout, recovery
and handshake supervision, the writer's blocked canonical return, independent
review and task-closure corrections after an unsupported proposed range change.
The archive ledger covers seven manual archives and the
subsequent native CLI archive. The new-change ledger covers successor scaffolding;
the continue ledger covers proposal, spec revisions, design and tasks, with
validator cost, guidance ordering and artifact-consistency measurements, plus the
follow-on's four-artifact plan and feasibility gaps. The update ledger covers
two approved four-artifact rescopings,
including guidance-read coverage, changed task denominators and unsupported claims
promoted into acceptance text. The naming ledger also traces its original observation
file's creation, migration and unsupported metric-to-policy promotion. Other OpenSpec ledgers identify supporting evidence
and unresolved coverage.

Campaign launch/control and pilot records 911–1075 are consolidated in the naming
ledger; records 1076–1097 cover the subsequent rescope in the update ledger.
Records 1098–1119 cover verification closeout in the apply ledger, and 1120–1138
cover archive, successor scaffolding and its first draft; records 1139–1182 cover
the remaining planning artifacts. Records 1183–1199 cover the first successor
implementation attempt and its capability pause; records 1200–1226 cover its
rescope, the capability-follow-on scaffold and initial naming observation ledger.
Records 1227–1263 cover capability planning, launch and the first scope/gate
corrections, with acknowledgment latency and duplicate-notice inspection costs.
Records 1264–1300 cover caller-discovery scope, watchdog inspection, reviewer
tool mismatch, parent-run checks and rejected canonical proof. The historical
mtime-based attribution lesson is corrected against the reviewer and writer reports.
Records 1301–1327 cover the corrective proof workflow, its reported independent
reproduction, provisional task checkbox and loss of an older rule during lesson editing.
Records 1328–1363 cover acceptance recording, lesson-edit recovery, owner-body
launch and its first two scope decisions. Records 1364–1398 cover bounded scouting,
decoder-shape corrections, an unproven timeout diagnosis and a failed fix; rollback
is authorized but not verified at that boundary. Records 1399–1455 cover the
standalone measurement, reported trace, incompatible transport condition, worker
timeout, inequivalent probes and resumed diagnosis. Records 1456–1508 cover its
return, handshake implementation/fixture scope, canonical refusal and a premature
boundary remedy, including a corrected end-address calculation. The owner proof
remains open. Records 1509–1537 cover observation migration, mixed-population
performance claims promoted into limits, the writer's return and independent
review dispatch. Records 1538–1564 cover the central folder's initial eight-ledger
rollout, its six empty templates, propagated metric errors and the limits of its
link/router checks. Records 1565–1579 cover the emulator ledger's opt-in, one
placeholder repair and repeated same-scope verification after a dependent edit/check
batch. Records 1580–1598 cover legacy cherry-picking/removal, preservation claims
based on partial reads and recovery from a transcript-inclusive reference search.
Its retained 213 MB grep expansion is separately reconciled in
`tmp/observation-ingest/grep-log-reconciliation.json`: exact duplicate records
retain their original review obligations; thirteen other lines and five controls
were read. This auxiliary expansion is outside the 20,506-file primary snapshot.
Records 1599–1605 cover the independent review's return, canonical refusal,
two ledger corrections and a task closure that dropped a caller-impact obligation.
The transport fix has independent evidence; complete capability acceptance and
the full child transcript review remain open.
Records 1606–1632 cover the initial history harvest: 13 calls/139.177 seconds,
790 reports filtered into 47 excerpts, one lost heading and four damaged prose
lines subsequently repaired. The docs ledger distinguishes scanned files from
reviewed content and records source-fidelity and recovery measurements; the RE
ledger qualifies the selected excerpts and metadata dispositions.
Records 1633–1642 cover the first chunked continuation: four calls/60.389 seconds,
forty appended rows discarded, 32 replacement rows and eight source IDs removed
during cleanup. The changed-filter replay did not establish duplicate safety;
the docs ledger records the provenance and replay acceptance measures.
Records 1643–1673 cover the harvest loop's launch through its first foreground
slice: 15 calls/742.013 seconds, two RPC timeouts, duplicated input paths,
unverified process attribution and cursor/coverage defects. The docs ledger
separates reported scan speed from semantic-review throughput; the loop's later
records 1674–1690 add eight calls/902.769 seconds through cursor 4,900, another
RPC timeout and missing terminal evidence. Its 1,656 tagged excerpts are an
unreviewed candidate backlog, not established skill ownership or complete ingestion.
Records 1691–1723 cover shared-cursor recovery, lossy clustering, incomplete
restoration evidence and link neutralization: 17 calls/1,049.809 seconds. The
docs ledger distinguishes the observed orphan from the unproved writer attribution,
reconciles misleading receipt/cluster totals, and records preservation criteria.
Records 1724–1739 cover selective filtering and editorial synthesis: eight calls/
95.446 seconds; 1,350 fragments became 125 filter passes, 81 representatives and
25 written findings. The docs ledger corrects the reported 94% noise, 81 passes
and 23 findings, and records the limits of preview-based semantic claims.
Records 1740–1773 cover JSON decoding, global-history traversal and two refinement
passes: 16 calls/326.262 seconds. The docs ledger separates 4,685 candidates,
235 scored rows and 84 grammar-selected rows from complete semantic coverage,
and records unresolved provenance, error accounting and quality-check defects.
Records 1774–1793 cover receipt merging, lossy archive caps, a 21-finding RE
restoration and consolidation to one file per skill: six calls/75.389 seconds
and three/34.880 seconds across two requests. The docs ledger distinguishes the
achieved file layout from the unproved preservation claim and counts recovery cost.
Records 1794–1803 cover the report follow-up: four calls/49.412 seconds and twelve
unlocated RE additions. The 107 global reports belonged to the separately encoded
`rebof3` directory; their skill relevance and relationship to this repository's
history remain a distinct provenance-review obligation outside the primary snapshot.
The docs ledger corrects the final bullet denominator and same-project claims.
Records 1804–1822 cover the nine-worker editorial rewrite: three parent calls/
227.028 seconds, 36 child calls and 175.322 seconds of workflow time. The docs
ledger distinguishes 1,295→185 observation bullets from acceptance: all nine
processes completed but metadata acceptance was rejected. Child selection review
remains pending; structural checks did not establish full preservation.
Records 1823–1839 cover the expanded harvest and second rewrite launch: seven
calls/296.510 seconds, a false source-order diagnosis, omitted nested reports,
unproved contamination attribution and an old-version first slice. The docs ledger
separates 98 row additions from demonstrated new stream coverage; full child
review remains pending.
Records 1840–1858 finish that parent session: four return/closure calls/42.393
seconds, 29 child calls and 145.348 seconds of workflow time. The second rewrite
reduced 283 section bullets to 162, then moved 26 heuristically between skills.
The docs ledger records the rejected metadata acceptance, stale index, omitted
delivery and remaining attribution/preservation gaps. That status covered the
then-reviewed prefix. The same parent transcript is now reviewed sequentially
through record 2638. Records 1859–1923 include ten reported exact results after a
22m37s wait; records 1924–1983 include nine exact reports with one run still active
after a 50-minute wait. The harness trial then repaired the combined gate and
stale-ranking tests, and added a configure lock. Records 1984–2009 cover ten
completed async runs yielding eleven function reports (eight exact, three partial)
after a 28m46s wait; `lifted` and `lifts valid` differed by one. Records 2010–2040
cover ten completed lanes (eight exact, two partial) after a 21m33s wait, an
optimistic ceiling of **22.3 reported exacts/hour**. These are not independently
reviewed accepted-throughput rates, and the incomplete dispatch intervals and
different task mix prevent a controlled before/after comparison. Build/configure
and cross-lane claim races still appear in reports. Index validation also reports
a function/global conflict at `area016/13:D_801F4D7C`; the count discrepancy and
index failure remain open. Records 2386–2478 report five later ten-lane wait
windows with **13.8–27.6 reported exacts/hour ceilings**; one batch's child reports
reconcile to 9 exact plus 1 partial, contrary to the parent's 10-exact summary.
Records 2581–2638 contain 15 distinct exact mission reports, including six first-seed
matches, plus one exact standby verification; the overlapping batch boundaries and
aggregate-count mismatch prevent treating these as net coverage growth. A missing
source claim blocked manifest-based tools across the repo and six of ten lanes in
one cohort. Current-tree candidate filtering passed 155 tests and returned three
unclaimed selectors in its smoke check, but no controlled throughput gain is shown.
See the [lift-loop](bof3-lift-loop.md#concurrent-lift-throughput-and-harness-cost-center-trial)
and [RE](bof3-re.md#records-2386-2638-throughput-and-wasted-work) ledgers. The
[current throughput update](bof3-re.md#2026-09-26-throughput-measurements-and-optimization-priorities)
adds harvest rates, mission wait-window rates and measured optimization proposals.
Raw child transcripts and the wider history remain pending. The broad extractor
has now been reconciled for this parent: its startup error is classified separately,
and eight repeated LSP summary occurrences are duplicate diagnostic evidence.
All eighteen editorial children are fully reviewed in the
[docs ledger](bof3-docs.md#first-re-editorial-review-and-cohort-closure); older domain
children and the wider corpus remain pending. Fifty-five of their 65 exported
result bodies exactly match native records; all ten clipped exports are recovered.
All native read continuations are reviewed. Operational leads
lost during the second loop/RE compaction are restored with original-run verification
still pending; editorial input is not independent proof of a historical run.
A discarded type witness is restored from lifter `11ddeb3f` record 276;
that lifter's earlier transcript remains pending. The extractor
also matches `model.thinking` configuration strings: twelve occurrences in
records 911–973 are metadata, excluded from reasoning-text measurements. Preserve
their pointers for review, and do not treat raw extractor-kind totals as verified
reasoning coverage without this classification.

The macro rewrite's dropped timing lead is now recovered from parent `01a0c062`,
records 314–321, including all assistant/reasoning/results and arguments. The
[test ledger](bof3-test.md#recovered-transaction-test-timing) records a 2,209.23-second
suite and its four slowest cases, while separating five diagnostic calls from
test execution. Records 1–95 of that parent now have complete retained-content
review through [goal drafting](plans.md#skill-scope-goal-drafting) and its
[confirmed revision](plans.md#ownership-edits-and-confirmed-scope-revision):
24 calls/175.564 observed seconds, then 25 calls/87.095 seconds. Two original full-output logs are missing;
the retained truncated results are reviewed with that coverage gap explicit.
Records 96–154 now cover the [initial source-identifier implementation](bof3-naming.md#initial-source-identifier-implementation):
32 calls/198.452 observed seconds, a corrected discovery false positive, one failed
test run followed by two passing runs, and unproved native transaction acceptance.
Records 155–217 cover guidance publication, two repaired links, premature task
closure and the approved performance revision: 28 shared calls/1,682.922 observed
seconds including aborted checks. The docs, plans and test ledgers distinguish
file-link checks from operational acceptance, and censored phase timings from
profiled bottlenecks. Records 218–256 add component profiling and build-refusal
diagnosis: 22 calls/995.541 observed seconds, 21 failed target builds and a
600-second censored baseline. The test ledger records sample denominators,
instrumentation errors and the still-unproved gate-preserving remedy.
Records 257–273 cover the metadata-only implementation and failed full-suite
verification: 10 calls/2,185.447 observed seconds. The 19-second standalone result
removed native checks; the full recipe failed with eleven tests before its later
gates, including a newly attributed naming CLI size regression. Records 274–313
add its repair and duration-run supervision: 20 calls/2,621.862 observed seconds,
two failed edit batches, eleven focused passes, and a false-live process report
beside a terminal suite summary. Records 322–354 add baseline comparison, four
unavailable status-tool attempts and the final failed recipe: 18 shared calls/
2,481.943 observed seconds. Compiler-process instrumentation identified one lookup
over the existing 64 MiB limit; broader build-failure causality remains unproved.
Records 355–402 add 25 shared calls/217.870 observed seconds: PATH trimming did
not remove the observed failure, four more control calls were unavailable, two
selected repairs passed, and two CMake fixture tests still failed after setup
changes. Records 403–431 add 14 shared calls/247.221 observed seconds: ownership
assertion repairs passed 27 manifest tests; declaration repair left two scratchpad
failures despite 180 broader passes. Records 432–485 add 27 calls/220.619 observed
seconds: a baseline admission changed the naming gate, and a bitfield parser
repair moved a 50-case selection from three failures to one. Lookup causality
remained unresolved and its byte limit unchanged at this boundary. The older
parent's records 486–521 add 17 calls/2,529.223 observed seconds: doubled lookup
limits passed 30 tests, a 2,241-second full recipe failed at lint, and its rerun
was aborted before the user repeated the performance request. The older parent
records 522–531 add four calls/752.074 observed seconds: compiler and test profiling
measured waiting and hashing costs, but a narrow subprocess probe supported an
unjustified broad exclusion and no fix was implemented. The older parent is now
reviewed through record 1527. Records 532–555 add twelve calls/2,160.327 observed
seconds: 3,895 Git-query invocations explained the aggregate wait cost, a cache
reduced one test from 152.69 to 83.24 seconds, and the full recipe passed in
1,385 seconds. Cache freshness/cutoff coverage and unchanged-gate acceptance remain
unproved. Records 556–580 add thirteen calls/2,031.864 observed seconds: a revised
entire-suite ≤5-minute target led to a 147-second default that excluded 302 tests,
followed by three unavailable completion calls. The target remained unmet;
records 581–599 add six calls/68.680 observed seconds through an abort: four more
unavailable completion calls and one interrupted selected smoke check, with no
new passing verification. Later focus clearing and a new user-requested draft
do not establish completion. Records 600–635 add sixteen calls/2,029.990 observed
seconds: an explicitly confirmed full-suite goal restored the requested scope;
the unfiltered baseline took 1,394 seconds with one failure. Two production-cache
edits then crossed the test-only boundary, and a two-case pass plus a parallel
probe with discarded statuses did not establish the suite target. Records
636–658 add twelve calls/1,483.286 observed seconds: oversized-file content hashes
were replaced with metadata hashes; full pytest improved to 1,021 seconds but
remained above target. A complete fixture read contradicted the assumed compiler
dependency, and later profiling attributed 1,455 runner calls/41.517 seconds to
repository queries. Records 659–681 add eleven calls/948.559 observed seconds:
fresh-capture attribution corrected the same-snapshot premise, then a production
workspace cache reduced full pytest to 566 wall seconds with unchanged pass/skip
counts. Its metadata-only signature left invalidation gaps; fixture organization
and the whole-recipe target remained unmet. Records 682–715 add seventeen
calls/1,167.120 observed seconds: SQLite batching and backup caching reduced full
pytest to 394 then 356 seconds. The fixture task was marked complete without
fixture-method or behavior-equivalence evidence; the full target remained unmet,
and a name inventory only began the redundancy audit. Records 716–743 add fourteen calls/898.216 observed seconds: include profiling
and two further caches yielded actual green full-recipe timings of 355 then 375
seconds. Both exceeded the target; one run per version did not establish the
string cache's effect, and filesystem-cache invalidation remained unproved.
Records 744–781 add nineteen calls/517.995 observed seconds: the string cache
was reverted; a redundancy audit advanced to 4/5 despite unproved equivalence.
An isolated three-file run passed 130 tests, but a full parallel rerun took 72
seconds with three failures. No tests were deleted; the full target remained
unmet. Records 782–801 add nine calls/1,551.469 observed seconds: a repository-local
parallel trial failed 43 tests, and a new cache was reverted after a failed
444-second recipe. Rollback passed at 443 seconds; removing temporary trees,
including a 336M repository directory, preceded a 377-second pass. The target
remained 77 seconds away; causal timing attribution remains qualified. Records
802–837 add sixteen calls/1,279.038 observed seconds: user-directed ignore pruning
failed 26 cases with 46 errors, then rollback restored 98 selected passes and a
382-second green recipe. A blocking request returned untried fixture alternatives;
a native-build memo passed full pytest in 372.64 seconds but established no
meaningful gain. Records 838–863 add seven calls/503.337 observed seconds including user interaction:
a 31-second scoped smoke added no folder moves, and the old performance goal was
blocked. The user then explicitly retired the 300-second bar, approved only the
two-case candidate-1 deletion and confirmed a six-task folder/unit/skill goal.
The new contract was created, with implementation still pending at this boundary.
Records 864–899 add eighteen calls/861.535 observed seconds: all 77 files moved
into 13 units; configuration repair restored 84 silently excluded cases, then
23-file path repair cleared a 92-failure recipe. The full gate passed at 376
seconds with 1,921 passes/two skips, and task 1/6 persisted complete. These are
1,923 collected cases; the approved deletion had not occurred. The next scoped
recipe edit was still unverified. Records 900–953 add 28 calls/1,868.039 observed
seconds including the completion audit: approved deletion reduced collection to
1,921; map validation recovered from three false orphans to 77/77; the skill,
wrapper and routing were delivered. Application and full-gate failures recovered
on retry, ending at 1,919 passes/two skips and goal approval. The deletion
precedent and flaky-failure labels still lack the claimed proof. Records 954–989
add fourteen calls/891.028 observed seconds: the user confirmed a whole-game
campaign allowing documented partials, original coverage reported an unknown
function denominator, and a 498-second status run returned 782 exact/139 partial.
A five-entry queue was frozen and baseline task 1/5 persisted, but derived
remaining counts and pin completeness remained qualified. No lift was accepted
in this preparation slice. Records 990–1029 add seventeen parent calls/1,825.571
observed seconds: the first writer returned a 43/44 partial, after three waits
and repeated attention/status handling. The parent prepared 18 more queues and
launched independent review; the entry was labeled partial before a review
verdict. The embedded report raises time-cap and attempt-accounting questions
that require its raw child transcript. Records 1030–1047 add eight parent calls/
1,424.289 observed seconds: two review findings led to a repair, parent-confirmed
44/44 exactness, independent pass and queue acceptance. Four linked children used
277 started calls/3,186.326 summed event-span seconds; raw child review remains
pending. Repair/re-review runtime metadata rejected staged-file checks, distinct
from the domain pass. The timeout-forwarding claim was challenged by repair
inspection; worker cleanup concerns remain unverified. The next writer launched
with improved fixed-RAM guidance but unresolved scope/budget ambiguity. Records
1048–1082 add fifteen parent calls/4,129.833 observed seconds for entries 2–4
and the next dispatch. Six linked children used 510 calls/4,062.858 summed
event-span seconds. Three review passes and exact byte results coexist with
runtime rejections and unresolved source-contract findings. New evidence corrects
a stale-stub claim and a 51-instruction queue estimate, reproduces an aid ablation,
and exposes invalid-tag/binding exceptions, incomplete parent receipts, repeated
stale-analysis refusals and another reported permuter overrun. Records 1083–1107
add twelve parent calls/1,189.375 observed seconds: the last lift reached 22/22
after two configuration repairs; the batch totals twelve children/944 calls/
7,993.782 summed event-span seconds. Five parent acceptances coexist with six
runtime metadata rejections and earlier unresolved contract findings. The full
check passed after 17 naming-debt admissions, including repair of 16 malformed
rows; no names were resolved. Coverage arithmetic was corrected after mixing
target/global counts, but its claimed command origin and old index pin were not
refreshed measurements. Analyzer freshness remains unverified after index recovery.
Records 1108–1142 add sixteen parent calls/3,199.992 observed seconds: the battle
writer/reviewer used 188 calls/2,389.849 summed event-span seconds and retained a
21/27 partial. Review corrected two residual mechanisms and identified unresolved
whole-target placement. Parent baseline repair repeated the newline defect; a
failed full check recovered 396.827 seconds later through snapshot/index refresh.
The next prompt added bounded permuter and linked-object evidence requirements;
their effectiveness required subsequent measurement. Records **1143–1163** add
nine parent calls/2,371.801 observed seconds and a second battle partial: 7/27,
155 child calls/1,952.783 summed event-span seconds. Both consecutive partials
required residual repair (2/2); the reported permuter bound differed from the
handoff. Preflight snapshot/index refresh preceded a first-run green full check,
but repaired-state review and combined placement remained unverified. Type-field
evidence was narrower than blanket layout neutrality. The next compaction
summary omitted the active campaign and live child. Records **1164–1188** add
ten parent calls/1,813.643 observed seconds and 200 child calls/1,437.130 summed
event-span seconds. Review reproduced scratch code/data exactness after changing
object alignment, while canonical gates retained a 25/27 partial; an existing
four-aligned input narrowed the alleged general placement ceiling. A failed
compiler probe was not alignment evidence. The parent repeated an acknowledged
decision after a stale attention event; full checks passed, combined placement
remained unproved, and a separate tooling gap was recorded. Records **1189–1220**
add fourteen parent calls/3,833.429 observed seconds and three child runs totaling
270 calls/3,350.419 summed seconds. A 5→9/28 score gain was independently confirmed,
but positional matches stayed at five and residual hunks grew. Two reviews required
evidence repairs; the parent copied old register claims, removed unreproduced
measurements, and overstated final-comment review in the queue. A stale steer
failed after an explicit terminal result. The full recipe passed, while the final
comment's "in place" label remained unsupported. Records **1221–1234** add six
parent calls/1,196.383 observed seconds and the batch's sole exact lift, 31/31;
review required three naming admissions. The qualifier experiment supports this
compiler/body result, not a universal non-volatile rule. Across five battle
entries, eleven child runs used **940 calls/9,945.296 summed event-span seconds**:
one exact/four partials, with four of five first reviews requiring some repair.
Full checks passed, but the parent recorded success before observing it and the
next-target lookup hit a guessed-path error. Records **1235–1246** add five parent
calls/1,404.505 observed seconds and 157 child calls/1,011.156 summed seconds:
existing struct fields improved a new battle03 source from 25/47 to exact 47/47.
Review accepted it despite a reproduced lifecycle-parser failure, which aggregate
validation did not catch. A supplemental retained macro definition corrected the
reviewer's scratchpad address arithmetic; the full misuse chain remains unverified.
Records **1247–1260** add six parent calls/2,152.025 observed seconds and 225 child
calls/1,756.206 summed seconds. The second battle03 lift reached 45/45; an independent
literal-marker counterfactual fell to 27/45. Four layout assertions and a forced
target build strengthened proof, but attempt accounting and lifecycle validity
remain separate obligations. Semantic-name baseline advice was corrected by review.
Records **1261–1276** add seven parent calls/1,562.834 observed seconds and 172
child calls/1,168.798 summed seconds: a loop-local-base experiment reproduced
43→47/47, and stores justified a const correction. Review still accepted a strict
metadata failure; an index-exact-only sweep omitted misclassified native matches.
Records **1277–1296** add nine parent calls/1,722.023 observed seconds and 241
child calls/1,682.861 summed seconds. Symbol rebinding produced 69/69; review's
scope/boundary claims needed correction. Raw timestamps establish writer/gate
overlap, but an isolated test pass does not prove the cause of the failed full check.
Records **1297–1312** add eight parent calls/1,120.156 observed seconds and an
89-call/347.152-second review. Four variants separate qualifier and operand-order
effects; aid-comment repair led to stale index fingerprints and a second full failure,
then explicit refresh and full green. The five-function batch used ten child runs,
884 calls/5,966.173 summed seconds; all five native results were exact, with unresolved
review/lifecycle qualifications. Records **1313–1330** add eight parent calls/
2,357.950 observed seconds and 183 child calls/1,596.241 summed seconds. A scratchpad
cell-volatility counterfactual reproduced 30/37 versus exact 36/36; only one sibling
was tested under a shared qualifier change despite 36 inventoried readers. Serialized
full validation failed once and recovered, with cause unverified. Records **1331–1340**
add four parent calls/1,096.251 observed seconds and 147 child calls/715.227 summed
seconds: initialization placement improved 26/27 to 27/27; four baseline findings
were followed by a green gate. Review also misattributed an inherited extern and
missed the measured experiment in its lesson. Records **1341–1368** add the user's
explicit gate-scoping correction: fourteen correction calls, a 9-second no-pytest
check, and 88 focused tests in 15.95 seconds. Full/unit wrapper execution was not
shown. This changes gate scope rather than speeding up the same suite. The enclosing
sixteen-call slice includes a resumed lift's 728.914-second wait. Records **1369–1406**
add twenty calls/1,631.504 observed seconds. The [documentation migration](bof3-docs.md#routing-migration-after-provider-failure)
used fourteen calls/245.774 seconds: eight sections moved and eighteen incoming
references rewritten, but three outgoing links acquired incorrect resolutions.
The failed explicit child is the previously inventoried zero-tool provider failure.
An eight-line context smoke did not prove owner delivery; scoped green was claimed
early and later observed at rc=0/9 seconds. The [accompanying lift](bof3-re.md#division-profile-proof-separated-from-control-flow)
used 206 child calls/1,108.515 summed seconds; independent same-source compilation
separated a 35→44-instruction profile effect from the writer's combined 30/44→44/44
change. Strict metadata and scoped checks passed; compile-database recovery and
configuration-scope reconciliation remained unproved. Records **1407–1442** add nineteen calls/2,313.491 observed seconds. The
[naming-recorder correction](bof3-naming.md#baseline-recorder-recovery-and-review-boundary)
used fourteen calls/153.997 seconds: 1,770 diagnostic false positives were corrected,
four raw rows admitted and existing suites passed, but global admission was not
bound to selected reviewed changes. The [next lift](bof3-re.md#separate-pointer-lifetimes-and-a-qualified-exact-review)
used 156 child calls/1,381.566 summed seconds and independently matched 80/80;
representation causality and search-budget accounting remain qualified. Records
**1443–1471** add the [annotation-retirement case](bof3-docs.md#annotation-retirement-without-transfer-evidence):
15 parent calls/184.019 seconds, twenty removed comment blocks and six changed
reference files. Complete knowledge transfer remained unproved; native checks
covered six of twenty affected sources, while strict and aggregate validators
disagreed. Records **1472–1498** complete the [partial's review](bof3-re.md#guard-return-partial-and-the-cost-of-repeated-search)
and next dispatch. Two children used 166 calls/3,865.994 summed seconds; the
12/55 partial needed residual repair. A 14-minute wait was misreported as whole
review runtime, attempted searches became “untried,” and a text-mention scan did
not establish executed-command costs. The area030 batch totals ten children,
858 calls/8,667.543 summed seconds, four exact results and one documented partial.
Records **1499–1527** add [two logo entries and duplicate review](bof3-re.md#first-logo-pair-and-a-duplicate-review-caused-by-id-confusion):
13 parent calls/1,583.783 seconds, including the next entry's wait; five children
used 296 calls/1,631.732 summed seconds. Native results were 58/58 and 22/22.
A mission ID mistaken for a run ID caused an unsupported “vanished” diagnosis
and a redundant 58-call review, overlapping the original by 229.089 seconds.
Finding attribution and aggregate baseline-diff claims are qualified against the
actual parent/writer reports. Records 1528 onward, full child review and raw
auditor verification remain pending.
These measurements establish progress, not
completion of the full historical optimization or this ingestion.

A candidate scan found 56 files mentioning OpenSpec/opsx. Nineteen expanded
request records occur in one parent session (two onboarding prompt records for
one episode, two proposal, eight apply and seven archive records). These are
request records, not independent completions. Parent workflows are invisible to
child-only skill-selection metrics; conversely, catalogue mentions do not prove
invocation. Candidate pointers live in `tmp/observation-ingest/openspec-requests.json`.

At **2026-09-27 18:49:45 UTC**, the reopened inventory has **21,102 source files**, zero parse errors and **19,521/379,196 reviewed unique blocks (5.147997%)**, with **359,675 pending**. This reconciles the prior progress snapshot; earlier counters and hashes remain in `coverage_reconciliation_history`. The selected M13 continuation transcript (`.pi-subagents/sessions/lift-loop/m13-emi-world00-area008-13-0x801f3d88-continuation/run-0/session.jsonl`, SHA-256 `f2181f1b608eb53decd8cc1da3a1203f80a4a0f6e7ae57fc60ddc6bf7fc9d83b`) is **380/380 pending blocks complete**, with 511,507 indexed characters and 742 origin pointers revalidated. Its 32.963-minute first-read-to-checkpoint window screened **11.528 blocks/minute** and **15,517.6 indexed characters/minute**; after verified prefix reuse, novel semantic volume was **10,864.6 characters/minute**. The 15-cycle 32 KB plan completed with a 75-block/31,938-character one-call cycle. A 40 KB plan reduced projected cycles to 13 from 15, but its 65-block/38,783-character display clipped and required bounded recovery, so it was not adopted. These are archival screening rates, not BOF3 runtime; the child mission used 633 calls over an 84m36.836s event span and ended partial at 326/339 with **0/1 exact lifts**. Cycle 6 remains a separate target at **5,212/6,000 (86.8667%)**. Full-history semantic coverage is still incomplete. See the [M13 RE outcome](bof3-re.md#2026-09-27-m13-continuation-compiler-ceiling-with-no-exact-lift) and [lift-loop ingest measurement](bof3-lift-loop.md#iteration-audit-log).

At **2026-09-27 19:34:20 UTC**, the read-only reopened database and `progress.json` agree: **21,102 source files**, zero parse errors, and **19,890/379,196 reviewed unique blocks (5.245308%)**, with **359,306 pending**. The pinned `49745aa6` child transcript is **369/369 pending blocks complete** (466,008 characters; 644 source-origin pointer checks across four windows). Its first three windows combined semantic screening with origin reconciliation over **1,011.210 seconds**; the final 113-block window isolated active review at **43.546 blocks/minute / 37,978.2 characters/minute**, with origin reconciliation timed separately. A 128-block window had no clipping and averaged 8.53 blocks/display versus 4.92–5.33 in the earlier 64-block windows, but the timing boundary and content mix differ, so batch-size causality remains unproven. At the isolated rate, 359,306 remaining blocks correspond to a **137.5-active-hour scale scenario**; this single 113-block sample is not a planning ETA. A 192-block K trial has since completed on a different source window; it is not a matched control and does not establish a batch-size speedup. The historical lift reached 310/339 (91.45%) at equal size but remained partial, with **0/1 accepted exact lifts**. See the [49745aa6 RE observation](bof3-re.md#2026-09-27-49745aa6-child-transcript-throughput-baseline-and-partial-compiler-result). Overall ingestion remains incomplete.

The latest recorded read-only coverage audit in `tmp/observation-ingest/progress.json` is dated **2026-09-27 20:53:45 UTC**: **19,894/379,196 blocks (5.246363%)**, with **359,302 pending**. The inventory has not been re-queried during this window, so treat this as the latest reconciled checkpoint, not a live count. At that audit, Cycle 6 targeted 6,000 blocks and stood at **5,212/6,000 (86.8667%)**; the target was later raised to 7,000, as recorded below.

The preceding Cohort K sample, `cycle6-cohort-k-window192-20260928`, reviewed **192 unique blocks / 255,656 indexed characters** from `.pi/sessions/2026-08-03T20-37-30-546Z_019fc958-8932-7a72-a358-0f5d33895319.jsonl` (SHA-256 `29420b39507759c1ae447a0e28bd298869032b2682ab895388b03978f1d0d78c`). Two readers used 12 display groups, averaging 16 blocks / 21,304.7 characters; the largest was 23,912 under the 24,000-character cap. Clipping, rereads, omissions and hash mismatches were zero; five sensitive candidates were withheld. Its approximate 688-second operational interval gives **16.744 screened blocks/minute** and **22,295.6 indexed characters/minute**, not active-only review or skill-execution rates. Two source-backed ledger entries were recorded, but total observation yield remains unresolved. No checkpoint was attempted; it receives **0/192 coverage credit**. See its [pending receipt](../../tmp/observation-ingest/cycle6-k-window192-20260928-screened-pending.jsonl).

Following the user's direction, the Cohort K window increased from 192 to **256 unique blocks (+33.3%)**, totaling **290,080 indexed characters** from the same pinned source after line 775 (SHA-256 `29420b39507759c1ae447a0e28bd298869032b2682ab895388b03978f1d0d78c`). The [selection and manifest record](../../tmp/observation-ingest/cycle6-k-window256-20260928/selection.json) contains two disjoint 128-block manifests; each display stayed under **24,000 characters / 64 blocks**, with three sensitive candidates withheld before selection. Review completed **256/256 blocks** across 14 displays: indexed and rendered counts both equal 290,080; all 256 block hashes and source pins were verified; clipping, rereads and omissions were zero. Block mix was 133 tool outputs, 77 reasoning blocks, 44 assistant-text blocks and 2 runtime-error blocks; block sizes ranged from 0 to 16,086 characters (median 392, p90 2,779, p95 5,591). The 14 displays averaged 18.3 blocks / 20,720 characters, with a 23,875-character maximum. The 226-second interval from the first reader start to the last finish screened **67.965 blocks/minute** and **77,012.4 indexed characters/minute**; summed active reviewer time was 333 seconds, or **46.126 blocks** and **52,266.7 indexed characters per cumulative reviewer-minute**. These rates exclude selection/manifest preparation, ledger synthesis, checkpoint and validation. Applying this single-window sample to the next **320-block** target projects about **362,600 characters, 18 display calls, 4.7 minutes of reader-window wall time and 6.9 cumulative reviewer-minutes**. This is a sizing forecast from one unmatched cohort, not a corpus ETA or measured speedup; compare it with the next run before changing display limits or recommending harness optimization. The cohorts are not content-matched, so the 192-block rate is descriptive only. The next target is **320 blocks (+25%)** under the same display caps; treat it as another trial because this window has no checkpoint credit and its per-block finding yield remains uncounted. Two source-backed supporting entries are recorded in [`bof3-test`](bof3-test.md#bundled-harness-improvements) and [`plans`](plans.md#module-cleanup-follow-up-and-incomplete-adjacent-gates); neither has skill-mission attribution or measures skill execution. No checkpoint was attempted; this window receives **0/256 coverage credit**. See its [pending receipt](../../tmp/observation-ingest/cycle6-k-window256-20260928-screened-pending.jsonl).

The corrected 320-block Cohort K trial completed on **2026-09-28 04:01:07 UTC**: **320/320 unique blocks / 553,478 indexed characters**, split into 126/278,892 and 194/274,586 characters (1.6% payload imbalance). Two readers used 26 display groups and 28 calls, including three segments for one oversized block. All 320 source/block hashes and finding links validate; there was no reader clipping or whole-block omission, zero rereads inside the corrected manifests, and four blocks replayed from the invalidated attempt with no prior semantic disposition. The selector withheld 41 sensitive candidates (131,892 characters); one selected block was redacted, while partial-redaction volume is unknown. Eight candidate findings link to 34 blocks; 286 blocks have no durable finding. No explicit skill mission was attributable, so this sample measures observation-screening throughput only.

The parallel review interval was **745 seconds** (**25.772 blocks/minute; 44,575.4 indexed characters/minute**); summed reviewer time was **1,181 seconds** (**16.257 blocks and 28,119.1 characters per reviewer-minute**). The two parts had nearly equal character volume but took 439 versus 742 seconds: part 2 contained 44 reasoning blocks versus one in part 1 and linked 30 finding blocks versus four. The 256-block sample forecast 7.2 parallel minutes and 10.6 reviewer-minutes by characters; actuals were 12.4 and 19.7. This unmatched sample shows block count and raw characters alone underpredict reasoning-heavy review. Keep the 320-block window and display caps for the next sample; add content mix and finding yield to sizing, then compare a matched cohort before increasing the window again. The invalid first selection overlapped 232 prior-screened hashes and produced 38 blocks with rendered content (37 complete, one partial) in eight successful and two failed display calls. The corrected census-backed selection had zero prior-screen overlap. No checkpoint was attempted, so coverage credit remains **0/320**. See the [selection and review manifests](../../tmp/observation-ingest/cycle6-k-window320-20260928/selection.json), [pending block receipt](../../tmp/observation-ingest/cycle6-k-window320-20260928-screened-pending.jsonl), and [part 1 receipt](../../tmp/observation-ingest/cycle6-k-window320-20260928/part1-review.json) (SHA-256 `1907810dcf73e6da2f1b4f5caedd7ede3d26c31c477431278770de8d99300216`) and [part 2 receipt](../../tmp/observation-ingest/cycle6-k-window320-20260928/part2-review.json) (SHA-256 `cfc4b4282d6be83c5c9cf229cd66c5f84b38292f5463ac0db3fbf511f8238e75`).

The next disjoint 320-block Cohort K window, `cycle6-cohort-k-window320b-20260928`, screened **320/320 unique blocks / 305,045 indexed characters** from the same pinned source. Its mix was 179 tool-output, 78 reasoning and 63 assistant-text blocks; 41 sensitive candidates were withheld before selection. Two readers completed 15 display groups/calls; all 320 hashes matched, with zero oversized blocks, clipping, omissions or rereads. Six findings link to 30 blocks; 290 blocks have no durable finding. One display group reported a **+7 rendered-versus-indexed character delta**; its cause remains unresolved, so indexed characters are the denominator. The [selection](../../tmp/observation-ingest/cycle6-k-window320b-20260928/selection.json), [pending receipt](../../tmp/observation-ingest/cycle6-k-window320b-20260928-screened-pending.jsonl) (SHA-256 `45954bbdad65b5040e967e8e15d26887fda1b5e27532e7b6fe8f701f0f200596`), [part 1 receipt](../../tmp/observation-ingest/cycle6-k-window320b-20260928/part1-review.json) (SHA-256 `7e3f50529831ff09d6747a3e8501e71c254216f55509b2d78d3127c41a05d50e`) and [part 2 receipt](../../tmp/observation-ingest/cycle6-k-window320b-20260928/part2-review.json) (SHA-256 `a222f1748861da3e2ff0fcc49ecae8cde47bdedf2e90093b3830866eb698788f`) retain the source evidence.

The first-to-last reader interval was **529 seconds** (**36.295 blocks/minute; 34,598.7 indexed characters/minute**); summed reviewer time was **1,006.18 seconds** (**19.082 blocks and 18,190.3 indexed characters per reviewer-minute**). Compared with the preceding 320-block window, block throughput rose 40.8% while indexed-character throughput fell 22.4%; average block size fell from 1,729.6 to 953.3 characters. Across these two unmatched windows, the pooled rates are **30.141 blocks and 40,432.8 indexed characters per wall minute**, or **17.557 blocks and 23,551.5 characters per summed reviewer-minute**. These differences show why sizing should pair block and character rates with content mix and finding yield; they do not establish a causal speedup. One explicit `bof3-lift-loop` mission appears in this cohort, with **1 of 5 queued candidates independently accepted and committed by cutoff**; native tool count and skill duration are unavailable, so no skill-execution throughput is inferred. See the [skill-specific observation](bof3-lift-loop.md#2026-09-28-observation-screening-throughput-and-lift-loop-mission-metrics).

The completed 400-block Cohort K screen reviewed **400 unique blocks / 159,277 indexed characters** from the pinned source (SHA-256 `29420b39507759c1ae447a0e28bd298869032b2682ab895388b03978f1d0d78c`). The selection SHA-256 is `e72e2916da46a1eaa270382573174f84861fde172b6569704e46308c2ae0f020`; its mix was 297 tool-output, 52 reasoning, 48 assistant-text, one summary and two runtime-error blocks, with 41 sensitive candidates withheld. Eight display calls/groups covered all blocks, with no oversized groups, clipping, omissions or rereads. Nine findings linked to 68 blocks; 332 had no durable finding. One part rendered five more characters than the indexed denominator; the cause remains unresolved. The [pending receipt](../../tmp/observation-ingest/cycle6-k-window400-20260928-screened-pending.jsonl) (SHA-256 `998fa6c4e7efc8d0dfea9c2b2cdd8a33f67d1a0b2192dc9e671397f17dd9c15d`), [part 1 receipt](../../tmp/observation-ingest/cycle6-k-window400-20260928/part1-review.json) (SHA-256 `0215c825cff727c6be275460b6133bc49ec5e2bf1169f4cb34b32cacd7204a72`), and [part 2 receipt](../../tmp/observation-ingest/cycle6-k-window400-20260928/part2-review.json) (SHA-256 `16e36ca80f39abe2c782841e517b94de2232c2795660f89b79e8f98db1779149`) retain the screen evidence.

The 400-block interval was **502 seconds** (**47.809 blocks/minute; 19,037.1 indexed characters/minute**); summed reviewer time was **742.357 seconds** (**32.329 blocks and 12,873.4 indexed characters per reviewer-minute**). Across the four completed 320/320/400/500 screens, pooled rates are **38.308 blocks and 28,512.7 indexed characters per wall minute**, or **25.242 blocks and 18,787.6 characters per summed reviewer-minute**. The cohorts vary in block size and content mix, so the rates guide bounded planning and optimization comparisons without proving a causal batch-size gain.

The 500-block Cohort K screen completed with **500/500 unique blocks / 128,409 indexed characters**. Its mix was 447 tool-output, 26 reasoning and 27 assistant-text blocks; 41 sensitive candidates were withheld. Nine groups/calls covered the full selection, with zero oversized groups, clipping, omissions or rereads, and all 500 block hashes reconciled. Six findings link to 34 blocks; 466 have no durable finding. Part 1 recorded four findings over 272 blocks; part 2 recorded two findings over 228 blocks. The [selection](../../tmp/observation-ingest/cycle6-k-window500-20260928/selection.json) SHA-256 is `19c41b4e55bb88dcd73f628a26189ed27700c50c6cf55ba59a8a6aa3194ceaca`; [part 1](../../tmp/observation-ingest/cycle6-k-window500-20260928/part1-review.json) is `ade8da78abffa2ad06e3ae6fa7dde6729dff8b18d88ac7fc245787679d1de654`, [part 2](../../tmp/observation-ingest/cycle6-k-window500-20260928/part2-review.json) is `689d763af42b09e14594b74a222d26e3af3919b1971a3cce93f5792749d425b2`, and the body-free [pending receipt](../../tmp/observation-ingest/cycle6-k-window500-20260928-screened-pending.jsonl) is `f03fb98d87177a73703955e22bd54ffebc838bfeb2e488ed4e305fb4bb4caa88`. Candidate findings await synthesis and checkpoint; no coverage credit is claimed.

The full first-to-last reader interval was **636 seconds** (**47.170 blocks/minute; 12,114.1 indexed characters/minute**); summed reviewer time was **731 seconds** (**41.040 blocks and 10,539.7 indexed characters per reviewer-minute**). Part 1 reviewed 272 blocks in 151 seconds (**108.079 blocks / 25,596.2 characters per minute**); part 2 reviewed 228 in 580 seconds (**23.586 / 6,619.9 per minute**). The pooled block rate is close to the 400-block window's 47.809, while character throughput fell with smaller average blocks and a changed mix. A sixfold gap between the block-scaled and character-scaled 500-window scenarios and the 10.6-minute actual show that neither denominator alone gives a reliable ETA; use both plus content mix, findings and part-to-part spread.

The next disjoint window completed semantic screening of **600/600 blocks / 175,839 indexed characters** from the same pinned source. Its mix was 538 tool-output, 33 reasoning and 29 assistant-text blocks; 41 newly detected sensitive candidates were withheld, with eight earlier sensitive hashes also excluded. Eleven capped display groups/calls covered all 600 blocks. All hashes matched; indexed and rendered characters matched; clipping, omissions, rereads and oversized groups were zero. Five candidate findings link to 50 blocks; 550 have no durable finding. Part 1 reviewed 259 blocks / 87,284 characters in 250 seconds (**62.160 blocks / 20,948.2 characters per reviewer-minute**); part 2 reviewed 341 / 88,555 in 563 seconds (**36.341 / 9,437.5 per reviewer-minute**). The first-to-last interval was **563 seconds** (**63.943 blocks / 18,739.5 indexed characters per wall minute**); summed reviewer time was **813 seconds** (**44.280 blocks / 12,977.0 characters per reviewer-minute**). These are screening rates, not skill execution rates. The [selection](../../tmp/observation-ingest/cycle6-k-window600-20260928/selection.json), [part 1 receipt](../../tmp/observation-ingest/cycle6-k-window600-20260928/part1-review.json), [part 2 receipt](../../tmp/observation-ingest/cycle6-k-window600-20260928/part2-review.json), and body-free [pending receipt](../../tmp/observation-ingest/cycle6-k-window600-20260928-screened-pending.jsonl) (SHA-256 `abe6914c16c32743e2ee33abb37d5d6e407dcbff8f37b358966792175aa3527e`) retain the evidence. Finding-ledger synthesis and checkpoint remain pending; the window receives **0/600 coverage credit**.

Across the five completed 320/320/400/500/600 screens, pooled throughput is **43.160 blocks and 26,663.2 indexed characters per wall minute** over 2,975 seconds, or **28.702 blocks and 17,731.6 indexed characters per summed reviewer-minute** over 4,473.537 seconds. The 600-block screen raises the descriptive block rate while its character rate sits below the earlier four-window pool; block size and content mix differ. Keep both units and both time bases when sizing work, and treat all pooled rates as planning evidence rather than a batch-size causal claim.

No explicit `bof3-lift-loop` mission appears in the 500- or 600-block selections, so neither adds a skill execution rate. The 600-block cutoff again shows session-level progress at 35/161 before the transcript ends during investigation; it has no terminal mission verdict. The two attributed earlier skill snapshots remain **1/5** in `window320b` and **3/161** in `window400`; neither has native tool count or mission duration, so accepted skill throughput remains unavailable. See the [skill-specific observation and instrumentation proposal](bof3-lift-loop.md#2026-09-28-observation-screening-throughput-and-lift-loop-mission-metrics).

The **700-block** selection has now been screened under the same **24,000-character / 64-block** caps. Its [manifest](../../tmp/observation-ingest/cycle6-k-window700-20260928/selection.json) contains **222,172 indexed characters** (627 tool-output, 39 reasoning, 33 assistant-text and one summary block), split 384/316 blocks and 110,654/111,518 characters across 13 complete display groups. All 700 block hashes reconciled. One outer display response clipped and was reread; reader-level clipping and omissions were zero. The timed portion covered **636 blocks / 200,177 characters in 323 seconds**: **118.142 blocks and 37,184.582 indexed characters per wall minute**. The first 64 blocks / 21,995 characters were outside the timer, so these rates describe only the timed remainder.

The body-free [screening metrics](../../tmp/observation-ingest/cycle6-k-window700-20260928/screening-metrics.json), [delegated run metrics](../../tmp/observation-ingest/cycle6-k-window700-20260928/agent-run-metrics.json), [part 1 receipt](../../tmp/observation-ingest/cycle6-k-window700-20260928/part1-review.json), [part 2 receipt](../../tmp/observation-ingest/cycle6-k-window700-20260928/part2-review.json) and [pending block receipt](../../tmp/observation-ingest/cycle6-k-window700-20260928-screened-pending.jsonl) preserve the evidence. They record one explicit `bof3-lift-loop` invocation but no parent duration or native tool count. Delegated worker/reviewer costs are separate from direct skill throughput. A validated selection disjoint from earlier K windows contained 800 hashes / **391,160 characters** (489 per block versus 317 in K700) across 20 capped groups; screening it took **347 seconds**. The next paragraph reports its screened and newly covered rates separately.

The hash-pinned [K800 selection](../../tmp/observation-ingest/cycle6-k-window800-20260928/selection.json) screened **800 unique hashes / 391,160 indexed characters in 347 seconds**: **138.329 blocks / 67,635.735 characters per minute**. The global ledger found six hashes already reviewed, so **794 blocks / 390,483 characters** earned new coverage: **137.291 new blocks / 67,518.674 new characters per minute**, a **99.25%** yield. The six duplicates show that local K-window disjointness does not guarantee global novelty; join candidate hashes against the global reviewed ledger before display, then compare duplicate-screen rate and net-new throughput on the next cohort. This sample does not measure saved time.

At the gross screening rate, a same-mix 900-block window (about 440,055 indexed characters) projects to **6.51 minutes**; 900 newly covered blocks project to **6.56 minutes** at the observed net-new rate. Both exclude selection, synthesis and checkpoint time and assume similar content density. Relative to K700's timed remainder, K800's gross block rate was 17.1% higher and character rate 81.9% higher, but K800 was denser and fully timed while K700's first 64 blocks were not; this is descriptive, not a causal batch-size gain. The projection informs a cautious next-window size increase after reconciling older pending receipts. Twenty capped display groups completed in 22 invocations; one outer response clipped and two groups were rerendered, with zero reader-level clipping or omissions. Forty-one new sensitive candidates and eight previously withheld hashes were excluded. The [screening metrics](../../tmp/observation-ingest/cycle6-k-window800-20260928/screening-metrics.json), [agent run metrics](../../tmp/observation-ingest/cycle6-k-window800-20260928/agent-run-metrics.json), [part receipts](../../tmp/observation-ingest/cycle6-k-window800-20260928/part1-review.json) / [part 2](../../tmp/observation-ingest/cycle6-k-window800-20260928/part2-review.json), body-free [block receipt](../../tmp/observation-ingest/cycle6-k-window800-20260928-screened-pending.jsonl) and [checkpoint sidecar](../../tmp/observation-ingest/cycle6-k-window800-checkpoints.jsonl) retain all 800 hash outcomes and five findings.

The delegated cohort has **10 reverse missions / 10 functions**, with **six exact claims and four escalations** in **15,019.854 summed worker-seconds / 504 calls**. Five claims join a reviewer pass; one remains unjoined at cutoff. This is **1.438 reported exacts per summed worker-hour** or **1.159 reviewer-confirmed exacts per combined worker/reviewer hour**. Seven reviewer runs recorded seven passes in **510.542 seconds / 33 calls** (including two post-cleanup reviews); five cleanup runs recorded two edits and three no-change results in **427.142 seconds / 20 calls**. Three one-hour timeouts consumed **10,800.086 seconds / 269 calls** (71.905% of worker time) with no exact. One reviewer pass had a false failed process status; the project agent flag fixed it. Direct parent skill duration and native call count remain unavailable.

K800 is checkpointed: **794 new blocks** advanced global coverage to **20,688/379,196 (5.455754%)**, with **358,508 pending**, and Cycle 6 to **6,006/7,000 (85.8%)**, with **994 blocks** to target. Six of the 800 screened hashes were already covered and received no duplicate credit. **Eleven earlier receipts covering 3,922 screened block occurrences** remain checkpoint-pending; globally reconcile and deduplicate them before dispatching the next K900 window. Full-history ingestion remains incomplete. Join partial candidates to prior journal outcomes, record score checkpoints before long deadlines, verify the full write set on rollback, and count reviewer verdicts separately from process status. Retain gross screening throughput, net-new coverage throughput and same-mix remaining-window estimates with content mix, included phases and uncertainty so they can size intake windows and quantify candidate harness improvements without confusing screening speed with skill execution.

For each window, continue reporting gross screened and net-new checkpointed blocks and characters per minute, their yield, same-mix remaining-work projections, content mix, display calls, repeated work, sensitive quarantines, hash coverage, omissions, clipping, rereads and separate review, coordination, checkpoint, ledger-edit and validation time. Keep skill-execution throughput separate from observation-screening rates. Cycle 6 advances only as eligible blocks are reviewed and checkpointed.

## Performance measurement contract

- Count unique runs, identify the date range, skill attribution and source cohort,
  and separate writers, reviewers and orchestration. A generic agent name alone
  does not establish a skill owner. Keep ambiguous runs in an explicit remainder.
- Give a denominator for every rate and distribution: total runs, measured runs,
  missing fields, median, nearest-rank p90 and maximum tool calls/duration.
  Transcript event counts and observed timestamp spans remain distinct from
  recorded `toolCount` and `durationMs`; missing values are never zero-filled.
- Separate process exit, timeout, report acceptance and independently verified
  task outcomes. Exit zero, attestation or a prose PASS cannot establish a correct
  lift or successful transaction. Report exact reports per wall hour separately
  from independently reviewed accepted exacts per hour; only the latter is accepted
  throughput. If only a wait window is known, report an optimistic ceiling and mark
  the full interval missing.
- Measure repeated context/gate calls, retries, lock contention, failed probes,
  regressions and restoration when the source supports them. Repetition alone
  does not prove waste; inspect its cause and necessary validation before proposing
  removal. Compare like tasks and record historical tool/skill changes.
- Each improvement names the measured problem, proposed directive/reference/tool
  change, acceptance measurement and remaining uncertainty. Proposed work is not
  implemented policy; current owners and evidence gates still govern execution.

### Measures that support decisions

For each comparable cohort retain run/parent IDs, source locations/hashes, dates,
skill-selection evidence, mission kind, target scope, model, tool/skill revision
when available, and missing-field counts. Separate attempts from distinct tasks;
link resumed attempts so retries do not inflate accomplished work. Keep raw counts
beside rates. An unknown denominator means **unmeasured**, not zero.

| Measure | Definition and use |
| --- | --- |
| Task outcome | Accepted task units / eligible attempted units, with pending, blocked, rejected and unknown outcomes separately counted. Name the unit: function, naming row, transaction, document or runtime question. No-op acceptance is its own outcome. |
| First-pass completion | Tasks accepted without corrective retry / tasks with a known complete attempt history. Report review rejection and rework separately. |
| Cost | Calls, elapsed seconds and recorded tokens per attempt; median, p90, maximum and measured N. Report writer, reviewer and orchestration costs separately. Aggregate cost per accepted unit includes failed/retried attempts; undefined when no units are accepted. |
| Delay and recovery | Time to first actionable evidence, blocked time, deadline failures / terminal attempts, resumed checkpoints / eligible interrupted attempts, and restoration results. Do not infer timeout from a long duration alone. |
| Tool friction | Invalid invocations, stale evidence, lock waits, parser failures and repeated unchanged probes / observed relevant calls or attempts. Record cause and attributable time; necessary validation is not waste. |
| Evidence quality | Required checks passed / applicable checks, with failed, skipped, unavailable and unreached counts. Record independent review coverage, subsequent regressions and claim corrections. |
| Reference effectiveness | Missing or incorrect guidance incidents / reviewed missions, discovery calls/time, and whether the operator read the relevant version. Missing guidance, missed guidance and unclear guidance imply different changes. |
| Ingestion coverage | Reviewed relevant blocks / inventoried relevant blocks, plus unattributed and unreviewed counts. File parsing, metadata extraction and semantic review are separate stages. |
| Parser/indexing throughput | New unique blocks, records and characters processed per reconciliation wall minute, with appended bytes, pinned prefix/full hashes, repeated occurrences, parse errors, cache/index state and included hash/database phases. Use it to tune ingestion tools; do not use it as semantic-review speed. |
| Semantic-review throughput | Fully reviewed unique blocks and characters per active review minute, with untimed blocks, cycle target/completion, content mix and checkpoint scope. Pause or exclude source reconciliation, ledger editing and validation; if a checkpoint interval combines them, mark the rate contaminated and exclude it from optimization recommendations. Use comparable samples for ETA and cycle sizing. |
| Mission throughput | Report native exact/partial reports and independently reviewed accepted exact/partial units per dispatch-to-last-terminal wall hour as separate measures. For an unfinished mission or queue, estimate time from a comparable accepted-work rate and remaining eligible units; state sample size, task mix, active versus wall time and uncertainty. Keep concurrent summed child time separate; a wait-window rate is only an optimistic ceiling. |

Use three improvement destinations: **skill directive** for a repeated decision or
ordering error; **reference material** for missing evidence, examples or discovery
guidance; **harness tooling** for measured execution friction. Each proposal needs
a source-linked baseline, causal hypothesis, concrete owner/change, comparison
cohort and acceptance criterion. Prioritize by affected missions and recoverable
cost, with evidence confidence visible. When a comparable throughput rate and
affected work volume exist, estimate the candidate's active-time effect and label
it measured, modeled or bounded; name a paired acceptance test. A volume-derived
ceiling is not an observed speedup, and call counts alone do not establish savings.

Evaluate changes on comparable task types and evidence requirements. Retain
failures and unfinished work in the cohort; report changed models, caches,
concurrency and task difficulty. Small case studies can motivate an experiment,
but cannot establish a population speedup. A reduced cost must retain required
checks and accepted outcomes; no single score substitutes for those measures.

## Ledgers

| Skill | File | Curated content |
| --- | --- | --- |
| `bof3-docs` | [bof3-docs.md](bof3-docs.md) | documentation workflow and coverage integrity |
| `bof3-lift-loop` | [bof3-lift-loop.md](bof3-lift-loop.md) | mission economics, fan-out and serialized writers |
| `bof3-macros` | [bof3-macros.md](bof3-macros.md) | macro workflow |
| `bof3-naming` | [bof3-naming.md](bof3-naming.md) | capability ceiling, evidence and transaction cost |
| `bof3-re` | [bof3-re.md](bof3-re.md) | measured shapes, residual classes and writer/reviewer performance |
| `bof3-test` | [bof3-test.md](bof3-test.md) | test-unit ownership |
| `bof3-types` | [bof3-types.md](bof3-types.md) | representation and ownership gates |
| `openspec-apply-change` | [openspec-apply-change.md](openspec-apply-change.md) | ten parent cases through independent review return and task closure; later work/full child review pending |
| `openspec-archive-change` | [openspec-archive-change.md](openspec-archive-change.md) | eight archives; manual/native costs, sync preservation and evidence limits |
| `openspec-bulk-archive-change` | [openspec-bulk-archive-change.md](openspec-bulk-archive-change.md) | coverage gap and batch outcome measures |
| `openspec-continue-change` | [openspec-continue-change.md](openspec-continue-change.md) | four contextual continuations; 30 calls, spec rework, guidance order and feasibility gaps |
| `openspec-explore` | [openspec-explore.md](openspec-explore.md) | coverage gap and evidence-to-decision measures |
| `openspec-ff-change` | [openspec-ff-change.md](openspec-ff-change.md) | coverage gap and artifact-set measures |
| `openspec-new-change` | [openspec-new-change.md](openspec-new-change.md) | two scaffolds; shared costs, instruction coverage and obligation-transfer limits |
| `openspec-onboard` | [openspec-onboard.md](openspec-onboard.md) | complete 75-call episode; configuration and reporting defects |
| `openspec-propose` | [openspec-propose.md](openspec-propose.md) | two explicit programs plus three implicit proposals; metrics, work-unit identity and recovery lessons |
| `openspec-sync-specs` | [openspec-sync-specs.md](openspec-sync-specs.md) | inline archive sync; full-body preservation versus heading equality |
| `openspec-update-change` | [openspec-update-change.md](openspec-update-change.md) | two approved rescopings; shared costs, artifact reconciliation and evidence-backed checkmarks |
| `openspec-verify-change` | [openspec-verify-change.md](openspec-verify-change.md) | coverage gap and semantic acceptance measures |
| `plans` | [plans.md](plans.md) | consolidation, execution evidence and incomplete obligations |
| `psx-emulator` | [psx-emulator.md](psx-emulator.md) | guidance restructuring evidence; runtime-performance coverage pending |
| `psx-rizin` | [psx-rizin.md](psx-rizin.md) | analyzer evidence |

## Ownership

`docs/` owns this folder; each skill owns the directives its ledger converges into. Register a new ledger
here and link it from the skill's self-improvement clause. Measured shape data belongs in the ledger;
contracts and gates stay with the skill.
