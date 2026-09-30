# bof3-docs observations

Per-skill half of the central observation folder ([index](INDEX.md)). The skill carries
**directives**; this file carries the **measurements** behind them. Read it before designing a
mission for this skill, and after each mission add one audit row and converge the finding into a
directive in `.pi/skills/bof3-docs/` (or its agent instructions) - prose alone is not convergence.

## Measured performance

The initial 4,171-run metadata audit found one mission explicitly selecting this
skill: `b3c6b526-e859-4bf1-a12d-d1f3c1133128` (2026-09-21). Its transcript has
**zero tool calls**, four failed assistant-generation events and a **20.338-second
observed span**; metadata exits 1 and omits `toolCount`/`durationMs`. The provider
reported a usage limit on all four attempts. This is availability failure before
documentation work, not evidence of a fast or ineffective documentation method.

Source: matching `*_worker_meta.json` and `*_worker_transcript.jsonl` under
`.pi/sessions/subagent-artifacts`. Generic document workers and parent-session
work remain to be attributed; a one-run explicit-selection cohort cannot establish
this skill's success rate or normal cost distribution.

A separate harness-consistency-cleanup plan provides cross-cutting context-payload
evidence, not a timed `bof3-docs` mission. The reviewer artifact measured one
representative `agent-context` output at exactly **260,161 bytes** (37,866 words,
6,112 lines; source block 0, hash
`140c61bc3ae36ce9344dd791f21df0aee211983703d71ad894a00ab1c910b165`, line 533,
`/message/content/0/text`). The later plan reports that omitting the unconditional
specs preload reduced the default output to **about 79–80 KB**, and that its test
asserts specs are absent and output stays under **100 KB** (block 15, hash
`080123c6424034f378bb877b07762e40771a05f942f98bf079b9992efaf8b4db`, line 550,
`/message/content/1/text`). The reviewer output separately reports five registered
script-test cases passing; the plan cites **284 `just check` passes**. The after
size is rounded and no context-generation time or byte-exact paired output is
available. Treat this as a substantial payload-volume reduction and bounded-output
check, not a measured speedup or `bof3-docs` throughput rate. The source is pinned
at `.pi/sessions/2026-08-03T20-37-30-546Z_019fc958-8932-7a72-a358-0f5d33895319.jsonl`
(SHA-256 `29420b39507759c1ae447a0e28bd298869032b2682ab895388b03978f1d0d78c`).

## Iteration audit log

| Iteration | Mission | Tools | Duration | Method verdict | Converged into |
| --- | --- | --- | --- | --- | --- |
| 2026-09-26 | Full-history coverage and metric audit | Not measured as a Pi mission | Not measured as a Pi mission | Recovered omitted source tree and separated parsing, review and performance denominators | [Measurement contract](INDEX.md#performance-measurement-contract); no skill edit |
| 2026-09-27 | Observation-ledger update: ingestion and skill performance | Pi `toolCount` unavailable; append reconciliation helper used | 0.211 s + 0.202 s for two source reconciliations; documentation/edit/validation duration not captured | Kept parser/indexing throughput separate from semantic-review throughput; added denominators and scope to the skill cohort observations | [Current coverage and throughput](INDEX.md#full-history-ingestion-status); no skill contract changed |
| 2026-09-27 | Full-history and `bof3-re` throughput update (`40d637e1`) | No Pi `toolCount`; local API calls not reconciled | Documentation write/check time not captured; transcript review was 716 s / 231 blocks | Recorded 19.35 blocks/min and 24.8k chars/min separately from 0 accepted exact lifts per task-hour; retained the 191-block progress-file discrepancy and unmeasured batch-size projection | [Current coverage](INDEX.md#full-history-ingestion-status), [RE performance](bof3-re.md#2026-09-27-bof3-re-transcript-screened-throughput-and-zero-accepted-lifts); no skill contract changed |
| 2026-09-27 | Full-history and `bof3-re` throughput update (`49745aa6`) | Pi total unavailable; 50 bounded transcript displays and 4 checkpoint transactions | First three combined review/reconciliation windows: 1,011.210 s; final isolated review: 155.698 s; ledger/edit/validation time not captured | Screened and hash-reconciled 369 blocks / 466,008 chars; final window 43.546 blocks/min and 37,978.2 chars/min, with a one-sample conditional backlog scenario; child outcome remained 0/1 accepted exact | [Current coverage](INDEX.md#full-history-ingestion-status), [RE performance](bof3-re.md#2026-09-27-49745aa6-child-transcript-throughput-baseline-and-partial-compiler-result); proposed, not causal, cycle-size trial |
| 2026-09-28 | Cohort K cross-skill history screening; no explicit docs-skill invocation | 12 bounded display groups; 2 readers; Pi `toolCount` unavailable | ~688 s approximate operational wall; active review and ledger-edit time not isolated | 192/192 unique blocks and 255,656 indexed chars; 16.744 blocks/min and 22,295.6 chars/min operationally; zero reader clipping/rereads; two source-backed candidate findings recorded so far, total yield unresolved; zero coverage credit | [Cohort K window](INDEX.md#full-history-ingestion-status), [context-payload evidence](#measured-performance); not a `bof3-docs` runtime rate or speedup claim |
| 2026-09-28 | Cohort K 256-block cross-skill history screening; no explicit docs-skill invocation | 14 bounded displays; 2 readers; Pi `toolCount` unavailable | 226 s reader-window wall; 333 s summed active-reviewer time; docs-edit duration not isolated | Screened 256/256 unique blocks / 290,080 indexed chars; 67.965 blocks/min and 77,012.4 chars/min by window wall, 46.126 blocks/min and 52,266.7 chars/min by summed reviewer time; hashes/pins verified, zero clipping/rereads/omissions; 0/256 coverage credit; two supporting evidence entries recorded in `plans`/`bof3-test`, neither skill-attributed; per-block finding yield unresolved. One-window sizing forecast for the next 320 blocks: ~362,600 characters, 18 displays, 4.7 reader-window wall minutes / 6.9 cumulative reviewer-minutes; compare forecast with outcome before tuning display limits. These are ingestion rates, not docs-skill throughput; unmatched content/timing scopes prevent causal comparison with the 192-block window | [Cohort K window](INDEX.md#full-history-ingestion-status); no skill contract change |
| 2026-09-26 historical parent, records 1538–1561 | Central observation rollout; supporting case, no explicit skill invocation | 18 | 71.430 s observed span | Eight ledgers wired, six empty; checks narrower than the closing claims | [Rollout findings](#central-observation-rollout); proposed reference improvements |
| 2026-09-26 historical parent, records 1606–1632 | Initial history harvest; supporting case, no explicit skill invocation | 13 | 139.177 s observed span | Filtered 790 reports into 47 excerpts; two editing defects required repair; full review not established | [Harvest findings](#initial-history-harvest); reference and ingestion-tool proposals |
| 2026-09-26 historical editorial cohorts | Eighteen ledger rewrite children; supporting cases, no explicit skill selections | 65; median 3, p90 6, max 8 | 2,203.014 s summed lifecycles; not elapsed workflow time | All process exits zero; one tool failure; metadata rejected all; no independent verification | [Cohort closure](#first-re-editorial-review-and-cohort-closure); evidence-retention and native-result recovery proposals |

| 2026-09-21 parent `01a0c062`, records 1369–1395 | TOC/directive relocation; failed explicit child followed by parent execution | 14 parent; 0 child | 245.774 s parent span; 20.338 s child event span, overlapping | Eight sections moved; incoming rewrites missed three outgoing relative links; context/gate claims exceeded checks | [Routing migration](#routing-migration-after-provider-failure); proposed reference and context-reporting improvements |

| 2026-09-21 parent `01a0c062`, records 1443–1471 | Annotation retirement and reference-capture directive; supporting parent case | 15 | 184.019 s observed span | Twenty comment blocks removed; preservation mapping absent; six native spot checks cannot establish twenty-file coverage | [Annotation retirement](#annotation-retirement-without-transfer-evidence); proposed preservation and scope metrics |

Add one row per mission. A row that changes nothing is itself a finding.

## Converged directives

None recorded yet. Directives for this skill live in `.pi/skills/bof3-docs/SKILL.md` and its references;
record the pointer here when one is added.

## Bundled harness improvements

| Evidence | Proposed change | Acceptance measurement |
| --- | --- | --- |
| Four generation errors before any tool call in the explicit docs mission | Operational guidance: classify provider availability separately from documentation quality; investigate retry timing in its owner without changing installed extensions | Comparable runs report startup failures/N and documentation outcomes/N separately; no misleading zero-cost successful mission |
| Previous harvest omitted `.pi-subagents` and filtered/truncated before interpretation | Ingestion workflow: hash inventory, unfiltered textual coverage and source-linked semantic dispositions | Every in-scope source/block has a recorded disposition; qualitative lessons survive; no coverage claim rests solely on parsing or bullet counts |
| Two verified append scans processed 270 records / 332 new blocks / 467,634 unique-block characters in 0.413 s; the smaller second suffix (−63.4% bytes) took 0.202 s versus 0.211 s for the first. One semantic sample reviewed 1,667 blocks in 3,418.940 active seconds | Ingestion telemetry: time prefix verification, decode/extraction, block deduplication, database writes and final hashing separately; retain source hashes, cache/index state and semantic-review timing | Comparable append cohorts report records, new unique blocks, characters, bytes, repeats, parse errors and per-phase wall time; semantic blocks/minute remains separate; all hashes and inventory counts reconcile |
| Four transcript windows screened 369 blocks / 466,008 characters; the first three combined review/reconciliation for 1,011.210 s, while the final 113-block interval isolated review at 43.546 blocks/min and 37,978.2 chars/min. A 128-block interval was faster but combined review and source reconciliation; no cycle-size effect is established | Add explicit review-complete and reconciliation boundaries, and report both block and character throughput with cycle target, display density, content mix, clipping/recovery, and checkpoint cost. Use rates to size a bounded next-cycle experiment and show a conditional backlog scenario separately from a planning ETA | Compare clean 128- and 192-block windows with similar content mix and display cap; report at least three matched windows before attributing a change to cycle size, while hashes, reviewed dispositions and recovery counts reconcile |

The first and second append scans measured **72,511.8** and **22,871.3 new
unique blocks/minute**; together they measured **48,232.4 blocks/minute** and
**67,937,142.9 characters/minute** across two passes. The semantic-review sample
measured **29.255 blocks/minute** and **36,457.7 characters/minute**. These work
units and timing scopes differ. The near-flat append wall time as suffix size
fell makes prefix/full-hash phase timing a worthwhile optimization experiment,
not proof that hashing is the bottleneck. The current full-history coverage and
backlog ETA are maintained in the [central index](INDEX.md#full-history-ingestion-status).

## Important observations

The [older skill-scope discovery](plans.md#skill-scope-goal-drafting) exposes two
searches whose broad roots mixed live references with session/frozen artifacts,
causing truncated output before a narrower third search. Their full logs are no
longer retained. Proposed reference/tooling measure: separately count live-owner
hits, historical hits, clipped results and recovery calls; use an explicit live
source inventory when checking current references. Never interpret capped search
output as complete absence, and preserve the original source gap in later audits.

The [reviewed ladder-compaction attempt](openspec-apply-change.md#ladder-compaction-initial-attempt-through-pause)
is supporting documentation-method evidence owned by an OpenSpec apply invocation,
not another explicit `bof3-docs` run: 47 calls in 273.268 observed seconds, ending
at 5/12 tasks. Its 31-line reduction included a six-line reduction that added one
character. Exact-line scans were promoted to claims of no semantic duplication,
and retained keywords/row labels were promoted to preservation proof. Proposed
documentation reference improvement: allow justified no-ops, compare complete
obligations and owner links, and pair size metrics with semantic review and
follow-up correction counts. Do not add these costs to the explicit docs cohort.

The [fully reviewed transport retirement](openspec-apply-change.md#documentation-transport-retirement)
is another OpenSpec-owned supporting case: 40 calls/502.347 observed seconds for
harness, skill and owner-document work. Its native-tool migration removed five
modules and the wrapper while retaining the drift/path library. These are delivery
and scope measures, not a measured documentation-performance improvement.

- The actual document read/write bodies changed **170→101 lines, 10,622→6,136
  characters**; the reported 159-line baseline was wrong. The skill changed
  45→42 lines, 2,862→2,374 characters. Size metrics must come from pinned bodies,
  not remembered counts or heading offsets.
- The operator invoked the deleted checker for its own rewrite, then recovered
  with one seven-link inspection across three files. That replacement handled
  simple inline links and ATX slugs, not the full reference syntax retained by
  the documentation contract. Later edited owners were outside that seven-link
  scope. Report parser coverage and inspected paths alongside zero-error counts.
- Nine phrase checks were called proof that all obligations survived. The old
  stale-hash stop procedure was replaced by fresh reads/general preservation
  language. Retiring a transport does not itself prove that this evidence change
  preserves concurrency protection. Map every obligation to its surviving
  procedure before claiming equivalent safeguards.

Proposed documentation-reference improvements: a scoped native inspection procedure
with explicit unsupported forms, baseline-relative content checks for concurrent
work, and before/after obligation mapping. Acceptance measures are supported link
forms/required forms, checked references/known applicable references, preserved
obligations/applicable obligations, and recovery calls per comparable migration.
These proposals authorize no tool restoration and add no runtime skill invocation.

The [observation migration audit](bof3-naming.md#metric-promotion-and-observation-migration)
adds supporting documentation-method evidence: **11 parent calls / 91.222 s**
for records 1509–1526, with no explicit docs-skill invocation. Four write/edit
calls moved a 127-line in-skill ledger into a 109-line external document and
updated navigation/directives before deleting the old file. No fresh complete
baseline read or obligation comparison established preservation. A router load,
two existence checks and whitespace checks established only their narrow results.
The context batch returned 23,111 characters of repeated search excerpts; its
132-line/7.0 KB command-output summary was not the size of the full returned body.
Six reasoning blocks totalled 39,104 characters. These are observed costs, not
measured savings from context compression.

Proposed **migration reference**: pair size and discovery-cost measurements with
an obligation map, corrected-claim propagation and checked-reference coverage.
Acceptance requires every retained obligation to resolve to its new owner and
every superseded remedy to be withdrawn before handoff. Compare correction calls
and lost-obligation incidents across equivalent migrations; do not duplicate this
shared parent cost in the explicit docs cohort.

### Central observation rollout

Parent records **1538–1564** in the pinned `01a0dad5` session contain **19 calls /
86.710 observed seconds**. The structural rollout used 18 calls/71.430 s; the
remaining call checked the still-running reviewer `360af7c5`. This is a parent
documentation case, not a new explicit `bof3-docs` mission or a reviewer result.
Recorded usage is 4,478,414 tokens: 42,427 input, 4,419,840 cache-read and 16,147
output, including 9,342 reasoning tokens. Seven reasoning blocks contain 33,990
characters. The batch's 118-line/7.8 KB command-output summary differs from its
18,238-character returned search body; neither demonstrates context savings.

**Deliverables were structural.** Two existing ledgers were moved, six empty
templates created and eight skill pointers updated. The templates accurately
said metrics were not captured; their existence proved no historical coverage.
`plans`, OpenSpec and `psx-emulator` were explicitly excluded from this rollout.
The later request for every skill requires the broader current 22-skill mapping.
The index copied the already flawed 120-directory aggregate and its unsupported
70-call/five-minute rule; centralization amplified that claim without new evidence.
See the [metric-promotion audit](bof3-naming.md#metric-promotion-and-observation-migration).

**Verification had narrower scope than the report.** The scan found four additional
references after the moves; thirteen edit calls repaired pointers and navigation.
Its **197 links across 36 files** checked simple inline destinations for existence,
discarded fragments and skipped fragment-only links. One phrase grep found the
expensive-rung heading; it was not an anchor audit. Eight exact `name:` matches
did not parse frontmatter or descriptions. Only naming's router was exercised
(exit 0, 17,217 output bytes), and whitespace checks passed. These results support
those checks, not “all references/frontmatter intact.” No full document body was
returned for semantic review; reconnaissance read eight observation lines and
twelve lesson lines. Moving content preserves its text but can change relative
link meaning and does not establish policy consistency. The mission protocol
still linked an expensive-rung **policy** in the external ledger, despite the
new claim that contracts stayed inside skills.

Proposed **documentation reference**: inventory incoming/outgoing links and policy
owners before a move, distinguish scaffold coverage from reviewed-history coverage,
and report supported link syntax, checked anchors and exercised routes separately.
Baseline: four late-discovered references, six empty ledgers and one propagated
unsupported budget claim. Acceptance measures are reconciled references/required
references, reviewed relevant blocks/inventoried blocks, and corrected claims per
comparable migration. Preserve the provenance of moved evidence; moving a file
must not promote historical heuristics into current policy.

The [emulator observation opt-in](psx-emulator.md#observation-mechanism-opt-in)
adds a **7-call/32.670-second** documentation case, not a runtime mission. An
edit/check batch returned the old placeholder; sequential verification changed
the result from one broken link to zero across the same 204 destinations. The
historical mitigation repeated every same-batch check. Prefer completing dependent
edits before validation, then rerunning only when changed inputs or unresolved
results justify it. Track check reruns/edit, detected defects and checked-reference
coverage; a script that merely prints failures cannot supply a reliable exit gate.

### Legacy consolidation and search-scope recovery

Parent records **1580–1598** contain **12 calls / 118.262 observed seconds**:
ten calls/82.666 s for source inspection and cherry-picking, followed by two
calls/28.923 s for search recovery, removal and reporting. Six reasoning blocks
contain 38,866 characters. Recorded usage is 4,066,308 tokens, including 4,013,184
cache-read and 17,673 output tokens; 10,437 reasoning tokens are included in output.
This is documentation work, not five new domain-skill missions.

The parent read the 52-line legacy lessons file completely, but inspected only
headings, keyword matches clipped to 120 characters, and a 41-line slice of the
129-line observation file. It then treated heading/topic agreement with the
164-line destination lessons document as proof that Levels 1, 2 and 4 were fully
duplicated. Five ledger writes/edits relocated turn-cost guidance and added type,
naming and Rizin lessons; the legacy lessons file was removed. The parent explicitly
relied on a sharing heading instead of reading its second-member and shared-body
obligations. This establishes insufficient preservation evidence, not proven loss.
The closing “all nine ledgers populated” claim was false: docs, macros, test and
emulator still had templates. Canonical owner proof also remained blocked; the
outstanding reviewer was not the only remaining acceptance obligation.

**One broad search created substantial output without answering the live-reference
question.** Searching `.pi/` included transcripts. Its retained log contains
**213,090,721 bytes / 13,622 lines**: 4,269 `LESSONS.md` matches and 9,348
`OBSERVATIONS.md` matches, plus five control lines. The displayed 49,825-character
tail was only the last nine lines with a truncation notice. The final comparison
of “9,348 versus seven” mixed the observation and lesson searches; the correctly
scoped counts were seven lesson mentions plus two observation mentions.

The audit reconciled the full log by exact content: **13,259 JSONL records and
345 other source lines** match the existing inventory. Their original semantic
review obligations remain pending where not already reviewed. The other thirteen
lines and five controls were read directly. The source hash and offsets are in
`tmp/observation-ingest/grep-log-reconciliation.json`; matching source content is
deduplication evidence, not a claim to have semantically reviewed 213 MB anew.

The historical **209 links / 41 files** check tested file existence, not anchors
or obligations; naming's router and whitespace checks passed. Proposed **reference
inspection procedure**: separate authored links from transcript provenance before
searching, inspect complete obligations before deleting a duplicate, and preserve
source-to-destination mappings. Baseline: one oversized search/recovery call, four
empty ledgers misreported as populated and one unproved complete-preservation
claim. Acceptance: every retired obligation has an inspected surviving owner;
report searched bytes, recovery calls, unresolved mappings and actual populated
coverage separately. No migration speedup follows from fewer files alone.

### Initial history harvest

Parent records **1606–1632** in the pinned `01a0dad5` session contain **13 calls /
139.177 observed seconds**. Inventory/filtering used four calls/49.090 s;
append and initial repair used five/25.364 s; later cleanup, recovery and checks
used four/39.194 s. These phase spans exclude gaps between phases. Thirteen
reasoning blocks contain 45,316 characters. Recorded usage is 9,456,109 tokens:
54,632 input, 9,380,224 cache-read and 21,253 output, including 11,382 reasoning.
Reproduction: `tmp/observation-ingest/initial-harvest-metrics.json`. This parent
case adds no explicit docs-skill invocation or domain-skill completion.

**Selection was not full-history review.** A filename inventory counted 9,821
files/8,822.8 MB under `.pi/sessions`; scripts scanned 790 reports and 790 metadata
files. They omitted `.pi-subagents` and did not interpret the parent transcripts.
The reported 246 lifter lesson-line hits, 133 distinct flagged candidates, later
60 candidates and final 47 appended rows used different filters, deduplication
prefixes and caps. They are not a recall funnel. Numeric slash patterns can match
target paths; zero reviewer/naming rows under these filters does not mean those
missions had no lessons. Worker statements that parent capture was needed do not
prove the parent never captured them. Transcripts contain reasoning, failed probes
and corrections absent from final reports; the claim that they merely repeat
reports was unsupported.

**Formatting introduced two measured defects.** Removing flag prefixes from
thirteen rows dropped the `partition()` separator and therefore one section
heading. Its repair was checked in the same edit/check batch; a subsequent
sequential read confirmed the heading. This establishes ordering uncertainty,
not the asserted fifth cache-staleness incident. Truncation to 300 characters,
then further trimming 33 rows toward 260, could remove measurements, qualifiers
or function identity. Appending delimiters did not restore that content. A later
line-local balancing pass edited seven lines, corrupting four pre-existing lines
whose parentheses correctly spanned line breaks; four exact replacements restored
them. The parent acknowledged both defects but still called every excerpt complete
and bounded to 260 characters, although added delimiters could exceed the cap.

Final checks reported 196 existing link destinations, one successful schema
validation and clean whitespace. Those checks did not validate excerpt fidelity,
anchors or semantic preservation. Recent-mtime counts and a baseline hash prefix
also did not prove no unrelated writes. The 47-row result was a candidate list,
not 47 independently verified or newly discovered lessons.

Proposed **documentation reference**: compact complete semantic units with source
locations; review changed paragraphs, including wrapped prose, before accepting
formatting changes. Proposed **ingestion tooling**: preserve run/record/block IDs,
full candidate spans, selection reasons and review dispositions separately from
display limits. Keep missing metadata explicit and report acceptance status apart
from task outcome. Baseline: one heading loss, four original prose lines damaged
and repaired, and no demonstrated complete semantic review of the 790 reports.
For comparable harvests measure source-linked candidates/retained candidates,
reviewed blocks/eligible blocks, lost qualifiers, repair calls and elapsed cost.
Acceptance requires traceable, complete retained lessons and zero unresolved
preservation defects; smaller files and balanced punctuation are insufficient.

### Chunked ingestion and duplicate control

Parent records **1633–1642** contain **four calls / 60.389 observed seconds**,
five reasoning blocks/17,307 characters, and 3,743,580 recorded tokens
(13,930 input, 3,720,704 cache-read, 8,946 output including 4,437 reasoning).
The historical request explicitly required chunking without duplicate observations.
This is supporting documentation-method evidence, not an explicit skill mission;
reproduction is in `tmp/observation-ingest/chunk-one-metrics.json`.

The first pass scanned 150 lifter reports, found 72 candidates and appended forty
before inspecting five short samples. It then removed that entire block because
the samples included attempt logs. The replacement scanned all 678 lifter reports,
selected 39 candidates and appended 32, leaving seven capped candidates. Its
regex accepted several alternatives to the promised symptom/shape pair; neither
filter's precision or recall was measured against a reviewed source population.
Useful failed-probe evidence should be interpreted, not discarded merely because
it appears in a log. The final 79 bullets were not 79 distinct durable lessons.

**The duplicate barrier was not validated.** Admission compared normalized
90-character prefixes within the scan and 70-character prefixes against the
ledger. Source preambles remained during comparison, while earlier ledger entries
had those preambles removed. Returned samples therefore repeated already visible
lessons about narrow-parameter promotion, enemy-index rematerialization, sparse
switches and byte-counter addressing. A subsequent cleanup removed preambles
from eight new rows **and their eight-character source IDs**. Rewriting presentation
changed the very keys used for deduplication and weakened provenance.

The final check reported 95 unmatched line hits, but changed the source population
from lifters to all reports, changed the regex, omitted log/JSON exclusions and
did not deduplicate hits. No identical-filter replay occurred. The closing claim
that every admitted item was verified against the barrier was unsupported; 95
is neither the remaining unique-lesson count nor evidence that replay is safe.
The reported 109 remaining reviewer/namer/worker/cleaner reports also misadded
90+13+6+2 (**111**), excluding one generic reviewer. Recursive session-file counts
overlap subagent artifacts and cannot be summed as disjoint populations. Links
and whitespace passed; the closing schema-success claim reused an earlier run.

Proposed **ingestion tooling**: retain immutable source/block identities and
separate semantic duplicate decisions from rendered prefixes; persist capped
items with explicit pending status. Before advancing a cursor, replay the same
pinned inputs/filter and verify that admitted identities add zero duplicates,
pending identities remain discoverable, and source links survive compaction.
Baseline: forty appended rows discarded, eight IDs removed, zero equivalent
replay checks and unquantified semantic duplicates. Measure duplicate admissions,
false merges, retained provenance/N and recovery cost on reviewed comparable
chunks. These checks concern ingestion correctness, not an assertion that prefix
matching can establish semantic equivalence.

### Harvest loop launch and recovery

Parent records **1643–1673** contain **15 calls / 742.013 observed seconds**,
through the first foreground slice; the loop continues beyond this boundary.
Output filtering and two RPC failures used four calls/176.710 s, background
launch/diagnosis nine/532.972 s, and the first foreground slice two/14.887 s.
Thirteen reasoning blocks contain 34,153 characters. Recorded usage is 9,977,750
tokens: 34,565 input, 9,923,456 cache-read and 19,729 output, including 9,106
reasoning. Reproduction: `tmp/observation-ingest/loop-harvest-start-metrics.json`.
These shared parent costs are not additional domain-skill missions.

The output pass scanned 790 reports and appended 26 candidates. Its zero
unadmitted candidates meant only that the filtered list fit its forty-row cap;
the report incorrectly called the output history fully ingested. Two subsequent
JSON traversal calls returned RPC timeouts after **62.112 and 62.263 observed
seconds**. No terminal handle or output comparison established the claimed
“wrote nothing.” The programs could write after traversal; a response timeout
does not prove execution stopped. Their per-file limits also examined at most
4,002 and 1,201 records respectively, so even normal completion would omit tails.
The second program advanced its file cursor before appending and counted a
partially read file as done.

**Background recovery did not establish its claimed cause.** The first script
duplicated 791 transcript paths in its **7,011-entry list of 6,220 distinct
paths**. After two launches and a regex rewrite, its log reached file 50 in two
seconds, then remained there at the 5:37 process-age check. These observations
show a stalled checkpoint, not a measured quadratic-complexity diagnosis or
whole-corpus speedup. The two RPC failures predated the blamed window regex.
A direct probe checked three files/926 lines with rounded times 0.1/0.0/0.0 s;
it did not profile the stalled worker or its other inputs.

One broad `pkill` returned 143 after matching the caller. Subsequent PID selection
used the first substring match, although earlier listings contained both shell
wrapper and Python child. The sampled PID's 0% CPU, sleeping state and 2,188 KB
RSS therefore did not prove the miner's identity or root cause. Killing that PID
and observing it gone established only that PID's disappearance. No command-line
and start-time binding established that all potential writers were terminal
before another writer began. The ledger's tagged-row count also rose 50→137
while the new background log explained only sixty RE additions; the extra
27 were not reconciled to a writer in this interval.

**The foreground result changed the work being measured.** It reported 500 files
scanned in fifteen rounded seconds and appended 38 rows, all to lift-loop. It
skipped lines over 120,000 characters, searched raw JSON instead of decoded
content, and retained only windows near the first cue. Unrecognized filenames
defaulted to lift-loop; no token-based routing existed in this version despite
the report claiming it did. File traversal, selected-window extraction and skill
attribution were therefore three different unresolved coverage questions.

The scripts also exposed recovery hazards independent of speed: a sixty-row
per-ledger cap silently discarded later candidates while advancing the cursor;
the background cursor omitted its final partial batch; the foreground script
set its cursor to the planned slice end even on an early deadline. The first
foreground slice did finish all 500 files, so deadline-induced skipping was a
code-path risk, not an observed loss in that slice. Neither version pinned the
source list or recorded skipped spans and read failures as pending work.

Proposed **ingestion reference/tooling**: separate observed timeout from verified
termination; bind worker identity before restart; commit source dispositions and
ledger writes together; retain unprocessed spans and uncategorized material.
Measure response timeouts/calls, verified terminal handles/restarts, duplicate
paths/inventory, skipped bytes/eligible bytes, attributed blocks/reviewed blocks,
and durable cursor coverage after interruption. Baselines here are two response
timeouts, 791 duplicate paths, one self-matching kill and one stalled checkpoint.
Acceptance requires no lost pending work or unowned concurrent writer, and equal
semantic coverage before comparing throughput. A fifteen-second filtered scan
cannot establish the cost of complete history interpretation.

### Foreground iteration and missing terminal evidence

Records **1674–1690** add **eight calls / 902.769 observed seconds** through
cursor 4,900/6,220. Eight reasoning blocks contain 8,121 characters. Recorded
usage is 6,288,646 tokens (19,758 input, 6,260,864 cache-read, 8,024 output,
including 2,355 reasoning). Reproduction: `foreground-iteration-metrics.json`
under `tmp/observation-ingest`. Three completed slice reports provide distinct
scan measurements: **1,600 files/24 s/120 rows**, **700/77 s/84**, and
**900/176 s/46**. Times are rounded script measurements; these different inputs
do not form a controlled comparison or predict whole-corpus completion time.

The next large RPC call timed out after 62.025 observed seconds; the following
read showed cursor 3,000. Again, no terminal-process evidence supported “killed”
or “nothing lost.” The reusable runner improved final cursor assignment to
`cur + done`, but still counted read failures as done, skipped large lines,
and checked its deadline only between files. Its output/cursor writes were
separate operations. The shell's reported `exit=0` was the final `sed` status,
not the miner or outer `timeout` status. The final two launches returned no
completion output; the last cursor remained 4,900. Empty piped output alone
did not establish whether a timeout, buffering or another failure caused it.

Token routing was now present, but it chose the first matching skill term, with
filename roles overriding content and lift-loop as the default. Row growth across
seven ledgers did not verify ownership. The last snapshot contained **1,656 tagged
excerpts**: docs 68, loop 639, macros 48, naming 110, RE 649, types 97 and Rizin 45;
test and emulator still had none. Loop and RE had grown to 475,409 and 468,807
bytes. These sizes/counts measure an unreviewed candidate backlog, not useful
consolidation. Eight-character filename tags also collapse timestamp-named parent
sessions to month prefixes, preventing unique source identification.

Proposed **runner reference/tooling**: preserve miner exit status and unbuffered
progress separately from presentation pipelines; record the last completed source
and all skipped spans; route ambiguous material to explicit pending attribution.
Measure terminal receipts/launched attempts, skipped or failed sources/N, unique
source identities/retained excerpts, and verified owners/attributed excerpts.
Baseline: one further RPC timeout, two launches without completion output, and
1,656 candidates without a demonstrated semantic-review denominator. Acceptance
requires source-complete receipts and reviewed ownership before row totals or
file growth can support a completion claim.

### Tail recovery and mechanical consolidation

Records **1691–1723** contain **17 calls / 1,049.809 observed seconds**: tail
recovery four/909.864 s, consolidation and splitting eight/70.779 s, and link
repair/reporting five/47.353 s. Sixteen reasoning blocks contain 45,244 characters.
Recorded usage is 12,920,321 tokens (41,402 input, 12,853,120 cache-read, 25,799
output including 11,758 reasoning). Reproduction:
`tmp/observation-ingest/mechanical-consolidation-metrics.json`. This supporting
parent case adds no explicit docs-skill invocation or accepted domain result.

**Shared state was demonstrably unsafe, but causation was misattributed.** The
cursor changed from `cursor:4900` to `transcript_cursor:57`. A subsequent process
listing identified Python miner PID 296905, age 26:09; it was killed, then the
cursor was reset and reached 4,950. This confirms an overlooked worker, not that
this worker wrote the old schema: the inspected daemon used `cursor`, while the
earlier timed-out inline traversal used `transcript_cursor`. The asserted
`KeyError` was not returned as a traceback. Exact writer identity remained
unresolved; restoring the number alone did not reconcile possible concurrent
ledger writes. Two more capped runs left the cursor at 4,950.

The parent reduced large-file reads to three-million-character head/tail slices,
then one million each, while seeking by byte size in a text stream. This both
omitted the middle and mixed byte/character units without recording skipped spans.
No I/O profile established the claimed slow-mount cause or that the omitted
content was redundant. Deferring 1,270 paths and calling reports/transcripts
fully ingested did not satisfy the historical request to finish the complete
history. The 6,220-path list already included the 791 transcripts; treating them
as additional completed files double-counted coverage.

**The first consolidation destroyed unselected content.** It rebuilt RE's
recovered section from `- [word_tag]` rows: 717 previously tagged rows became
137 retained rows, a difference of 580, plus excluded untagged bullets. Calling
all 580 “untagged” was incorrect: `\w+` excludes the hyphens and periods present
in timestamp/filename prefixes. Re-scanning outputs added 59 candidates, without
a lost-row identity map, so complete restoration remained unproved. The rebuilt
disposition section also retained those 59 new bullets while the raw section
included them again; the next pass counted **255 rather than 196 receipts**.
Thus repetition introduced by the recovery itself contributed to later clusters.

Initial signatures produced 137→136 and 196→195 clusters. Jaccard at 0.45 then
produced 255→184 for RE and **1,095→988 across six other ledgers**; the latter
was reported as the seven-ledger total. These are heuristic group counts, not
distinct findings or independent corroborations. The algorithm removed words
including “not,” “never” and “must,” chose representatives by percentage-count
and length, then truncated them to 230 characters. It did not preserve semantic
polarity or establish equivalent obligations. Two further passes still skipped
the other six ledgers; one appended the missing section heading after its rows,
then found nothing below it. The later “findings” totals also counted unrelated
existing tables.

Seven receipt files were created. Their claims of exact traceability were weakened
by nonunique source tags and subsequent link conversion. The first destination
check found **79 broken occurrences/291 checked**. A bulk conversion changed 112
links, including valid navigation links, and sometimes discarded the target when
its basename appeared in the label. Two attempted fixes changed nothing; one
also emitted three invalid-grep errors. Four literal replacements finally produced
**202 checked/zero broken**, with a changed path scope. This was removal of active
link syntax, not restoration of source-relative references or proof that receipts
remained exact. Link existence and whitespace checks did not inspect malformed
table cells, semantic preservation, clustering quality or complete recovery.

Proposed **compaction reference/tooling**: snapshot the complete selected content,
bind every original row to a retained or explicitly reviewed merged disposition,
and compare that map after the write. Keep cluster counts separate from verified
findings and independent support. Retain source-relative link targets with their
source context even when displaying them as text. Baseline: 580 tagged rows
excluded, unverified 59-row recovery, 59 duplicated receipts and 79 broken links.
Acceptance requires every input identity reconciled, zero unreviewed losses or
false semantic merges, and preserved provenance/navigation. Report recovery calls
and time alongside compression; size reduction alone is not success.

### Selective filtering and semantic-summary claims

Records **1724–1739** contain **eight calls / 95.446 observed seconds**, seven
reasoning blocks/29,299 characters and 5,880,318 recorded tokens (28,378 input,
5,836,032 cache-read, 15,908 output including 7,642 reasoning). Reproduction:
`tmp/observation-ingest/selective-summary-metrics.json`. This supporting parent
episode concerns documentation performance; no explicit docs-skill invocation or
new independently accepted domain outcome is established.

The user requested semantic reduction of the large ledgers. The parent first
sampled **27/700 loop and 22/255 RE rows**, limited to 175 characters each. It
then applied a measurement/rule-cue/prose filter to 1,350 extracted fragments:

| Assigned ledger | Input fragments | Filter passed | Cluster representatives |
| --- | ---: | ---: | ---: |
| docs | 71 | 0 | 0 |
| loop | 700 | 3 | 2 |
| macros | 55 | 0 | 0 |
| naming | 117 | 1 | 1 |
| RE | 255 | 120 | 77 |
| types | 107 | 0 | 0 |
| Rizin | 45 | 1 | 1 |
| Total | 1,350 | 125 | 81 |

The reported **94% noise** conflated **1,225 filter rejections (90.74%)** with
44 additional clustering removals. Neither establishes semantic precision or
recall. The filter rejected escaped newlines and transcript vocabulary, required
numerical/rule cues, and removed negation from clustering tokens; content formatting
could therefore decide whether a lesson survived. These are fragment counts from
an already selective harvest, not full-history findings or verified skill ownership.

The editorial pass read 40 RE previews at 190 characters, then all 81 representatives
at 165 characters. Its purported sentence trim actually removed leading text up
to an uppercase letter. No full-source reconciliation established that all rejected
content was debris or that only final reports contained lessons. Seven receipt
files were rewritten to keep representatives, losing the other passed fragments
and their equivalence evidence. Claiming these were all passing receipts was false.
The rewrite produced **21 RE plus four other findings = 25**, despite claiming
18 RE and 23 total; even the claimed category counts sum to 22.

The synthesis also introduced stronger conclusions than the previews supported:
selected mirror successes became “highest-yield,” and 29 analyzer starts/returns
became 29 false boundaries without reviewed labels. The latter false-boundary
inference is absent from the retained RE ledger; comparative mirror yield remains
unmeasured. Instruction-match ratios are not counts of successful missions.
The reported 222 existing links, clean whitespace, unchanged baseline prefix and
zero recent source mtimes did not validate these semantic claims or complete
mutation preservation. Stopping at cursor 4,950 left the earlier tail unresolved.

Proposed **summary directive/reference**: require full candidate context before
editorial acceptance, preserve qualitative findings, and distinguish filter passes,
clusters, reviewed findings and source-backed outcomes. Proposed **harness tooling**:
retain source IDs and an input-to-disposition map through decoding, clustering and
rewrites; preserve rejected/merged evidence until reviewed. Baseline is 1,350
fragments with no complete semantic disposition, not 94% proven noise. Acceptance:
every relevant input reviewed or explicitly pending; zero unsupported merges or
strengthened claims; count totals agree; measured false exclusions/merges on a
source-reviewed cohort. Report review cost and provenance coverage alongside size.

### JSON decoding, global scope and grammar filtering

Records **1740–1773** contain **16 calls / 326.262 observed seconds**: seven/
212.065 s for the decoded repository pass and nine/114.190 s for global discovery,
ingestion and refinement. Thirteen reasoning blocks contain 30,454 characters;
recorded usage is 11,275,126 tokens (37,003 input, 11,219,072 cache-read, 19,051
output including 7,879 reasoning). Reproduction and per-ledger stage counts:
`tmp/observation-ingest/decoded-harvest-metrics.json`. Supporting parent case;
explicit docs-skill invocation and independently accepted domain outcomes remain
unestablished.

JSON decoding addressed escaped transcript fragments, but the prototype's three
transcripts yielded 461 text blocks and 124 measured paragraphs, including 14
debris-regex matches. Its four short previews repeated mission prompts and ledger
tables. The production walker skipped tool-call arguments but did not restrict
roles, collected only `text`/`thinking` fields of at least 100 characters, skipped
short/malformed records silently, and required 130–900-character measured prose
with selected cues. Plain string `content` and child-summary fields were omitted.
It still could not establish complete assistant/reasoning/tool-result coverage.

A destructive section-replacement bug was noticed and an append-only correction
requested before execution. The dependent edit/run shared a batch, without a
separate exact-source assertion. Subsequent previews found zero rows under the
expected heading; the parent dismissed this as a stale read without demonstrating
the cause, then claimed growth proved quality. Later checks disproved that claim.
The initial “transcripts first” run actually selected from the sorted repository
JSONL list. Rounded slice timings are scan measurements, not semantic throughput.

The cursor eventually reached **6,220 repository + 1,958 global = 8,178 paths**.
Read and parse failures were not counted, and the cursor advanced past failed
opens, so “all files parsed” was unsupported. The global gate used paragraph
vocabulary and a broad `0x80…` address pattern, not session working-directory
provenance. It could admit foreign-project text and omit relevant paragraphs with
no marker; exclusion of foreign sessions was unproved. Rebuilding a sorted list
without source pins also left index-based resumption vulnerable to changed inputs.

| Stage | Observed count | What it establishes |
| --- | ---: | --- |
| Harvest rows across nine ledgers | 4,685 | Filtered candidates, rounded to 4,700 in the report; not all measured paragraphs |
| Scored selection | 235 | Keyword score, Jaccard 0.5 and cap 40/ledger; no measured semantic precision |
| Debris-regex matches in that selection | 26/235 | The claimed clean-prose result failed its own later check |
| Grammar selection from the harvest | 84 | Format-selected candidates; narration still appeared in the displayed samples |
| Final debris scan | 0/176 bullets | Narrower regex over a different population, excluding archives/receipts |

The final grammar pass lifted the 40-row cap: loop retained 64, naming eight, RE
six, types four, docs/macros one each, and test/emulator/Rizin zero. These are
heuristic routing results, not skill lesson yields: agent-name matching and the
first matching keyword could assign unrelated work, with loop as the fallback.
Requiring RE-style symptom/shape language biased other skills' results. Neither
zero candidates nor 84 survivors measured semantic absence, recall or acceptance.
The historical closing claim that reusable content totaled 84 was therefore false
as a coverage assertion.

Nine harvest sidecars preserved the selected paragraphs, but lacked source-record
identities; earlier receipts remained, producing 26 files rather than one per skill.
Eight unresolved link occurrences were neutralized in three archives: **239 checked/
eight broken → 231/zero**. This improved rendered link validity, not source-relative
traceability. No identical-input replay demonstrated the claimed idempotence;
70-character normalized prefixes were not semantic identity. Whitespace, baseline
prefix and recent-mtime checks did not establish semantic fidelity or full mutation
preservation.

Proposed **ingestion tooling/reference**: decode by supported record schema, retain
role, source/record identity and project provenance, and count parsed, failed,
omitted and pending blocks separately. Treat grammar/scoring as review ordering,
with source review deciding retention. Baseline: missing error/coverage counters,
26/235 debris matches, zero-row previews ignored and nine provenance-poor sidecars.
Acceptance: complete dispositions for supported records; explicitly pending unknown
formats; verified project/skill attribution; preservation of qualitative lessons;
identical-input replay adds no duplicates. Measure accepted findings and source
fidelity per review hour, without equating traversal or file growth with quality.

### Final consolidation and preservation claims

Records **1774–1793** cover two user requests: receipt merging/compaction (**six
calls / 75.389 seconds**) and one file per skill (**three / 34.880 seconds**).
The combined 191.679-second span includes an 81.410-second inter-request gap.
Nine reasoning blocks contain 24,392 characters; recorded usage is 8,035,783
tokens (28,733 input, 7,990,144 cache-read, 16,906 output including 6,032 reasoning).
Reproduction: `tmp/observation-ingest/final-consolidation-metrics.json`. These are
supporting documentation episodes, not explicit skill invocations or domain wins.

The first merge took **4,685 harvest + 81 receipt rows → 4,605 representatives**,
deleted seven receipt files, and trimmed entries to 600 characters before prefix/
Jaccard deduplication. The next pass produced **1,217 archive entries** using a
measurement-OR-grammar filter, 400-character trim, clustering and a cap of 400 per
archive. Loop fell 2,958→400 and RE 1,123→400. The cap retained first qualifying
entries in file order, with no importance ranking; the claim that dropped tails
were “lowest-signal” was unsupported. These differences mix filtering, truncation,
merging and unvisited tails, so do not count them as verified duplicate removals.
An unchanged end cursor and bank containing retained prefixes also meant the
offered re-run did not, by itself, reproduce the omitted material.

Rebuilding each ledger from its pre-Recovered head plus dash bullets deleted the
**21 numbered RE findings**, not the claimed 18. Diagnosis and reintroduction cost
**two calls / 12.300 seconds**, within the first phase. The replacement was a
shortened rewrite, not the claimed verbatim restoration; it removed the unsupported
29-false-boundaries inference but lacked a full preservation comparison. The four
numbered findings previously written to other ledgers received no explicit content
reconciliation. Checking headings alone could not establish they survived elsewhere.

The final one-file merge combined **83 ledger bullets + 1,217 archive entries →
1,283 candidate rows** and deleted nine archives. It imposed no new cap, but could
not recover earlier losses. Its introduction wrongly claimed every row met
grammar-AND-measurement with narration excluded: the archive used OR, and this
merge merely trimmed and deduplicated. The 1,297 final bullet count includes other
ledger content; neither count measures accepted findings or independent support.

The reported preservation “assertion” only printed H2 set differences **after
writing the ledger and deleting its archive**. It neither failed on loss nor
compared section bodies, H3 findings, qualifiers, ordering or source identities.
Zero lost H2 names therefore did not prove “nothing was dropped” or validate the
earlier removals. File count reached the requested nine ledgers plus index, and
195 links/zero broken plus a narrow debris scan over 1,297 bullets passed. These
structural checks support the layout outcome, not the all-important-content claim.

Proposed **compaction reference/tooling**: stage the complete rewrite against an
immutable source snapshot and compare bodies plus input-to-disposition mappings
before replacing/deleting files. Preserve numbered and multiline content, and
review whether shorter text retains evidence and exceptions. Baseline: 21 findings
dropped/reintroduced, two recovery calls, 4,605→1,217 archive reduction without a
semantic loss account, and a diagnostic mistaken for an assertion. Acceptance:
zero unreviewed loss or unsupported merge, every removed sidecar mapped to retained
content or an explicit reviewed disposition, and verified recovery from the pinned
snapshot. Report size reduction only alongside preservation and rework cost.

### Project and global report follow-up

Records **1794–1803** contain **four calls / 49.412 observed seconds**, including
one discovery batch with five commands/four searches. Five reasoning blocks contain
8,880 characters; recorded usage is 4,552,903 tokens (16,644 input, 4,529,664
cache-read, 6,595 output including 2,376 reasoning). Reproduction:
`tmp/observation-ingest/project-global-reports-metrics.json`. This is another
supporting documentation case; agent names alone do not establish skill invocation.

Discovery counted **140 broadly named global report files**, then **107 exact
`*_output.md` matches**, plus **790 project reports**. The six `worker` reports
were a subset of 790, not additional files. A filename regex failed to identify
all **790/790 project agents** because UUID hyphens were unsupported; a later
suffix matcher avoided that defect. The 107 global files were all under the
encoded **`rebof3`** directory, not **`rebof3-simple`**. Calling it this project's
directory and asserting overlap with the same missions was unverified. Their
historical agent distribution (35 bof3-worker, 26 bof3-reviewer, 19 delegate,
18 reviewer, nine worker) did not justify routing every worker/delegate to loop.

The pass traversed 897 selected paths and appended **12 RE candidates**, without
per-root yield or rejection/duplicate counts. It trimmed before filtering, required
grammar plus measurement, rejected broad words such as `master`, and compared
70-character prefixes against the ledger bank. Thus neither complete report
coverage nor deduplication as the cause of low yield was established. Source IDs
were again absent. The promised global relevance gate was absent from this script;
the inventory also still omitted `.pi-subagents` and other artifact formats.

The final report changed its denominator from Important-observations rows to all
dash bullets: naming's 136→145, types' 120→124 and Rizin's 59→60 reflected other
sections, not new findings. The actual section total was **1,283+12 = 1,295**;
**1,309** included fourteen other bullets. Only RE was changed, yet the index's
404-row RE count was not refreshed to 416. The reported survival assertion merely
searched selected heading substrings, without comparison or failure on missing
content. Its 34-link check covered only the observation folder, unlike the prior
195-link broader check; neither validated source coverage or semantic preservation.

Proposed **report-discovery tooling/reference**: pin complete source roots, distinguish
working-directory provenance from skill relevance, bind agents through metadata,
and retain per-source outcomes. Keep historical `rebof3` material as a separate
cohort until relevance/duplicate relationships are reviewed. Baseline: 790 filename
parse misses, 107 unverified same-project attributions, twelve unlocated additions,
fourteen miscounted observations and a stale index row. Acceptance: every candidate
has source/skill evidence, every rate uses a fixed population, index totals agree,
and unresolved cohorts remain visibly pending rather than “fully covered.”

### Delegated editorial rewrite

Records **1804–1822** cover workflow `5bf34940-daea-4d0c-92b2-3e566bdec55d`:
**three parent calls / 227.028 observed seconds**, including two verification
calls/23.424 s. Five reasoning blocks contain 16,790 characters; parent recorded
usage is 4,616,526 tokens (229,195 input, 4,378,880 cache-read, 8,451 output including
4,035 reasoning). Nine fresh workers rewrote distinct ledger files. All had empty
explicit `skills` arrays and model `ninerouter/gpt-combo:medium`; this is a supporting
documentation cohort, not nine invocations of the skills whose ledgers they edited.
Reproduction, full run IDs and pinned runtime/meta paths:
`tmp/observation-ingest/delegated-compaction-metrics.json`.

| Ledger edited | Observation bullets before→after | Child calls | Lifecycle seconds |
| --- | ---: | ---: | ---: |
| loop | 461→35 | 6 | 174.464 |
| RE | 416→32 | 8 | 172.725 |
| naming | 136→31 | 4 | 141.825 |
| types | 120→26 | 3 | 145.811 |
| docs | 64→23 | 3 | 123.443 |
| Rizin | 59→21 | 3 | 135.363 |
| macros | 36→15 | 3 | 125.340 |
| test | 2→0 | 3 | 81.476 |
| emulator | 1→2 | 3 | 80.719 |

All nine have measured calls/lifecycle timestamps: **36 calls**, median three,
nearest-rank p90/max eight; median **135.363 s**, p90/max **174.464 s**. Their
summed lifecycle time is **1,181.166 s**; parallel workflow time is **175.322 s**,
not the sum. Workflow token totals are 310,251 input + 25,140 output = 335,391,
distinct from parent accounting. This one heterogeneous cohort cannot establish
a parallel speedup or cost per independently accepted finding.

**Process completion and acceptance diverged.** All nine exited zero and their
runtime state was complete; all nine metadata acceptance states were rejected
by the staged-files gate. Rizin also failed the no-staged-files evidence field.
The shared tree already contained staged work, while briefs forbade git access;
these failures do not prove children staged anything. No verification runs were
recorded in those acceptance objects. The parent ignored this distinction and
treated completed notices plus structural checks as independent acceptance.

The scope protected each file's preceding content and required a full-file read,
but also directed children to delete anything unmeasured. That inherited the
qualitative-lesson exclusion. Reports describe semantic merges and prefix-byte
preservation; full child transcript and selection review remain pending. The
emulator preview was omitted by the notice budget and was not fetched by the
parent. Its retained report, now read, explains **one compound finding split into
two**, not new discoveries: 2/25 unreachable references and 54 scripts/53 tests
versus an earlier 52/55 inventory. Test's two fragments were removed for lacking
measurements; that rationale alone cannot establish absence of useful lessons.

The correct compression population is **1,295→185 observation bullets** (85.71%),
or **1,309→199 all bullets**; the parent mixed 1,309 with 185. Its 474 KB→116 KB
comparison mixed summed file sizes with allocated `du` usage. The index retained
the old counts. Parent checks confirmed selected headings, link existence and
debris-regex absence, not full byte or semantic preservation. The spot check
returned **8/9**, then was called 9/9 based on an unverified wrong-marker diagnosis.
A thirty-minute mtime listing over selected roots, capped at twenty paths, could
not prove zero out-of-scope writes. Several child reports recommended independent
review; none was dispatched in this episode, and not every child stated the
parent's claimed common limitation.

Proposed **delegation/reference changes**: allow qualitative findings, supply the
complete source/disposition map, and define semantic review separately from
structural validation. Proposed **harness reporting**: distinguish completed,
contract-rejected and accepted runs; compare staging to the pinned starting state
without altering it; fetch every omitted delivery. Baseline: nine acceptance
rejections, one unread preview, a mixed compression denominator and no independent
selection review. Acceptance: all nine deliveries reconciled, protected bytes
compared, every distinct source finding retained or explicitly reviewed, and
cost/outcome rates computed only for that verified population.

### Expanded harvest and second rewrite launch

Records **1823–1839** contain **seven calls / 296.510 observed seconds**, seven
reasoning blocks/23,728 characters and 6,567,461 recorded tokens (31,211 input,
6,523,264 cache-read, 12,986 output including 6,115 reasoning). Reproduction:
`tmp/observation-ingest/expanded-harvest-launch-metrics.json`. The user explicitly
requested all session/subagent history, including reasoning, assistant messages
and tool output, followed by delegated summaries. Nine new workers were dispatched
in workflow `32fff24a-a07e-4e5a-b425-2e374f04ead4`; their return is later evidence.

The new walker added `reasoning` and `summary` fields but still excluded short
blocks, plain string `content`, malformed records and paragraphs lacking selected
measurement/grammar phrases. Trimming to 420 characters preceded acceptance. It
still missed `.pi-subagents`; adding field names did not establish full coverage.
Its initial **8,995 sources = 8,196 JSONL + 799 reports** also exposed a report
discovery defect: the global `**/subagent-artifacts/*_output.md` glob omitted
`recursive=True`, so the earlier 107 nested global reports were absent.

The first **400-file slice admitted zero rows**. The parent explained this as
global files sorting first, but the script concatenated separately sorted lists
**repository first, then global**. The diagnostic counted text blocks in six other
sample files without measuring downstream filter rejections; it did not establish
the cause of the zero yield. These source-order and stage-count omissions caused
an unsupported explanation to be presented as fact.

The following provenance probe mixed populations: `ls | wc -l` counted top-level
directory entries (45/890/two), while recursive matching-file counts were capped
at twenty. These were not session counts or exactly twenty references. The actual
JSONL counts were 257 `rebof3`, 985 `bof3-decomp`, and 32 `rebof3-simple`. A different
working directory supports separate provenance; text mentions alone establish
neither shared history nor skill relevance. The parent then asserted all twelve
earlier additions came from `rebof3`, although that pass retained no per-root yield
or row identity. Potential contamination was real; attribution of all twelve was
unproved, and no specific contaminated row was reconciled.

The source edit and launch shared a dependent batch. The first 900-file slice
still reported the old **8,995-source** list; later slices used **7,069 = 6,238
repository JSONL + 799 reports + 32 project-qualified global JSONL**. The common
repository prefix means this transition alone does not prove skipped repository
files, but it disproves the claim that the first launch used the new scope. The
wrapper's stop total excluded the 32 global paths, producing two empty terminal
slices after cursor 7,069. The paragraph-level global marker remained after path
restriction, still capable of excluding relevant text.

Net additions were **98 rows**: **185→283 observation bullets**, or **199→297 all
bullets**. No source/stream counters distinguished newly covered material from
re-admission after earlier summaries changed the prefix-dedupe bank. Claiming the
reasoning/tool-output streams caused the growth was therefore unsupported. The
second worker brief again discarded unmeasured findings and used visible path
strings to reject other projects; generic target paths cannot prove provenance,
and restricting observations to targets can exclude documentation/tooling lessons.

Proposed **ingestion diagnostics**: emit a pinned ordered manifest, source classes,
schema coverage and per-stage rejection/duplicate counts; keep source identities
through compaction so replays do not depend on summary wording. Apply source-list
changes before launching, and derive stop conditions from that same manifest.
Baseline: misdiagnosed zero/400, omitted nested reports, one old-version slice,
two empty slices and 98 additions without stream attribution. Acceptance: manifest
identity/order verified at launch and resume, no silent omissions or unmapped
re-admissions, and provenance corrections tied to actual rows. Evaluate relevant
older BOF3 cohorts separately rather than converting path guesses into facts.

### Second rewrite return and attribution repair

Records **1840–1858** finish the parent session. Its return/closure phase used
**four calls / 42.393 seconds** (1850–1858), five reasoning blocks/11,850 characters
and 4,773,390 recorded tokens (34,221 input, 4,732,032 cache-read, 7,137 output
including 3,079 reasoning). The complete expanded-harvest/rewrite case, including
the preceding launch, was **eleven parent calls / 483.648 seconds**. Reproduction:
`tmp/observation-ingest/second-compaction-metrics.json`; do not sum that whole-case
cost with the launch phase again.

The second nine-worker cohort again had empty explicit `skills` arrays and model
`ninerouter/gpt-combo:medium`. All nine completed with exit zero but metadata
acceptance **rejected** by the staged-files gate; none recorded verification runs.
The staging and semantic-acceptance qualifications from the first rewrite still
apply. Twelve children are fully reviewed below; loop, RE and naming children in
each cohort remain pending.

| Ledger edited | Observation bullets before→after | Child calls | Lifecycle seconds |
| --- | ---: | ---: | ---: |
| loop | 107→23 | 6 | 144.293 |
| RE | 49→27 | 3 | 121.153 |
| naming | 36→29 | 2 | 93.604 |
| types | 27→26 | 3 | 100.643 |
| docs | 25→21 | 4 | 131.394 |
| Rizin | 21→20 | 4 | 139.371 |
| macros | 16→14 | 2 | 115.837 |
| test | 0→0 | 2 | 72.476 |
| emulator | 2→2 | 3 | 103.077 |

All nine have measured costs: **29 calls**, median three, nearest-rank p90/max six;
lifecycle median **115.837 s**, p90/max **144.293 s**, sum **1,021.848 s**. Workflow
wall time was **145.348 s**, with 228,349 input + 19,935 output = 248,284 recorded
tokens. Compared with the first rewrite, this cohort had much smaller and already
edited inputs; its lower cost is not a controlled improvement or a parallel speedup.

The actual reduction was **283→162 section bullets**, not the reported 297→162;
all-file counts were 297→176. Again the emulator preview was omitted and the parent
did not fetch it; its retained report, now read, describes wording changes with
both findings preserved. Docs removed four entries, including two fragments and
two EU-related items; macros removed two unanchored/truncated entries. These are
reported selection decisions, not independent proof of irrelevant source content.
Repeated unmeasured/target-only selection still leaves qualitative lessons at risk.

Three previews per nonempty ledger, capped at 170 characters, exposed per-function
results under docs/loop. The parent moved **21 loop + five docs = 26 bullets** into
RE, producing loop two, docs sixteen and RE 53, with total 162 unchanged. This
used an opening-address/measurement regex and 70-character prefix comparison,
not original mission ownership. The destination signature set was not updated
between additions, so it also did not establish within-batch duplicate safety.
Byte totals and unchanged bullet totals do not validate this attribution repair.

The final checks excluded the index, did not repeat the earlier heading check,
and counted only selected foreign-path/debris markers. Zero marker matches could
not prove zero foreign evidence; the parent itself acknowledged unresolved origin
of twelve earlier additions while repeating their unproved foreign attribution.
The index remained stale, and a capped twenty-minute mtime scan did not establish
absence of out-of-scope writes. Calling the entire history “complete and verified”
was unsupported by these structural checks or by the still-filtered source corpus.

Proposed **attribution/review tooling**: bind each summary to original record and
mission ownership, retain separate supporting domains, and update index totals from
the same reviewed population. Review relocated and discarded findings explicitly;
provenance uncertainty stays pending even when a marker scan passes. Baseline:
26 heuristic relocations, nine contract-rejected completions, one omitted delivery,
and no independent selection review. Acceptance: every retained/removed/moved
finding reconciled, index counts current, protected content verified, and complete
source coverage demonstrated before claiming ingestion finished.

### Small editorial child review

Complete assistant text/reasoning, tool results and arguments are now reviewed for
`697fbae6` and `94b2d502` (emulator ledger), and `e47c21de` and `6db5cf50`
(test ledger). Full IDs, transcript/source hashes and reproducible measurements:
`tmp/observation-ingest/small-editorial-metrics.json`. These four are a subset of
the two rewrite cohorts above, not additional missions or explicit domain-skill
invocations. All eleven calls have results; none was truncated in the source.

Together they used **11 calls / 337.748 summed lifecycle seconds**, with 64,966
runtime-recorded input/output tokens. Transcript usage separately records 74,752
cache-read tokens; these accounting fields must not be conflated. Summed lifecycle
time is not parallel wall time. All four exited zero, all four had metadata
acceptance rejected by the pre-existing-staging check, and none recorded an
independent verification run. Their prose `noStagedFiles: true` meant “I did not
stage,” while the runtime tested the whole tree; record those distinct predicates.

- **Preservation evidence improves on the parent checks.** All four children
  read their entire assigned ledger and asserted unchanged bytes outside the
  observation body. Three checked the preceding prefix; the section was last.
  `94b2d502` handled a following section too. This verifies surrounding bytes,
  not factual accuracy or semantic preservation inside the rewritten body.
- **Bullet growth is not discovery.** `697fbae6` split one compound bullet into
  two existing claims; `94b2d502` merely reworded those same claims. Both retained
  the unproved claim of reference unreachability, corrected by the
  [emulator source audit](psx-emulator.md#important-observations). Neither child
  read the underlying catalog, and the supplied brief confined reads to the ledger.
- **Empty output is not an empty history.** `e47c21de` removed two clipped,
  unmeasured fragments: a reviewer-checklist ambiguity and a denominator task.
  No original source was consulted to determine their durable value. `6db5cf50`
  then used two calls/72.476 lifecycle seconds to reword the empty introduction,
  adding no finding. An accurate no-op would have preserved the same evidence.
- **Retry cost is visible separately from execution.** `94b2d502` used a non-ASCII
  Python bytes literal, causing a parse failure before mutation. One corrected
  call succeeded: one failed call among eleven total, or seven shell calls.
  The failed tool took 0.023 seconds; receipt-to-corrected-receipt elapsed time
  was 21.313 seconds. Neither figure proves that much recoverable latency.

Proposed **documentation directive/reference**: permit a justified no-op and
preserve qualitative source leads with explicit uncertainty; distinguish fact
verification from surrounding-byte checks. Supply authorized source context when
asking workers to certify factual or selection quality. Proposed **reporting-tool
change**: use separate baseline-relative staging and whole-tree staging fields,
and retain invocation failures in cost totals. Acceptance: each retained/dropped
claim has a source disposition, unchanged evidence permits no-op completion,
protected bytes still match, and first-pass/rework rates use comparable tasks.
This four-run case study does not establish population savings or authorize a
change to installed tooling.

### Macro and docs editorial child review

Full transcripts and arguments for `101ba6e9`, `9221cd41`, `7e859242` and
`f1b6ce5d` are reviewed. Full IDs and hashes are in
`tmp/observation-ingest/macro-docs-editorial-metrics.json`. These four existing
cohort members used **12 calls / 496.014 summed lifecycle seconds**, with all
results present, zero source-truncated results and zero failed tool calls. Runtime
input/output tokens total 95,189; transcript cache reads are separate. All four
retain the historical acceptance rejection described above; no independent
verification run is recorded.

The macro rewrites reduced **36→15** and **16→14** bullets in five calls/241.177
summed lifecycle seconds. Repeated address-form experiments were merged, but the
first rewrite also discarded a measured test-performance lead. The
[recovered original output](bof3-test.md#recovered-transaction-test-timing)
confirms four tests costing 617.39 seconds. The second rewrite removed an
unanchored 43/57 macro-store result and a clipped 51/66→64/66 scratch-access table;
missing identities warranted source recovery, not a conclusion that the evidence
was valueless. These are editorial costs, not macro-extraction productivity.

The docs rewrites reduced **64→23** and **25→21** bullets in seven calls/254.837
summed lifecycle seconds. The first dropped qualitative leads about observation
edits conflicting with mission scope, missing historical evidence for untracked
sources, and duplicate scanners mistaking boilerplate for duplication. The second
removed a randomizer/EU comparison and an EU inventory as unsuitable target
evidence. Yet its index read explicitly exposed the repository's EU-knowledgebase
route. Neither a regional label nor absence of a performance delta establishes
irrelevance to documentation quality. Original sources still require review;
these removals are not independently accepted selection decisions.

Preservation checks also differ: `7e859242` reread the written prefix; both macro
scripts compared only their constructed byte buffers, and `f1b6ce5d` compared
constructed text across two writes. The latter checks support the intended output
but do not independently inspect written protected bytes. Heading/link/marker
checks could not detect the lost operational lessons. Only `f1b6ce5d` read the
mandatory repository index; the shared brief otherwise restricted reads to one
ledger. Mission context should reconcile that conflict before delegation.

Proposed **compaction reference**: classify removed entries as duplicate,
unsupported, unresolved-source or irrelevant, retaining pointers for recovery;
qualitative guidance and test cost remain eligible evidence. Proposed **validation
procedure**: compare written protected bytes with PRE and reconcile source
dispositions independently of formatting checks. Acceptance: every distinct
finding is retained or has an evidence-backed disposition, diagnostic leads remain
recoverable, and no throughput claim treats deleted bullets as eliminated noise.
The recovered timing case demonstrates one consequential omission, not a measured
recall rate for either cohort.

### Rizin and type editorial child review

Full transcripts/arguments for `49d9cbe9`, `b4c6ff45`, `5383d90d` and `6c8e8536`
are reviewed, including recovery of a clipped export. Reproduction and full IDs:
`tmp/observation-ingest/rizin-types-editorial-metrics.json`. These existing cohort
members used **13 calls / 521.188 summed lifecycle seconds** and 123,409 runtime
input/output tokens, with thirteen results and no failed tool calls. They all
exited zero but retained metadata acceptance rejection; these are documentation
operations, not analyzer/type-skill throughput.

The **120→26** type rewrite's exported read contained 32,464 characters and an
explicit truncation flag. Its retained native session, record 9, contains the full
43,415-character result: an identical 32,443-character prefix plus a now-read
10,972-character tail. The worker received the complete result; the export was
clipped. Across all twelve reviewed children, **35/36 result bodies match their
native records exactly**, and this one is recovered. Source hashes, call IDs and
record joins are in `tmp/observation-ingest/editorial-native-results.json`.
Export completeness and what the worker saw must be audited separately.

The second type rewrite's **27→26** reduction removed a supposed duplicate of
`func_801D6668`. The original final report/reasoning, now read at record 276 of
`11ddeb3f-7393-475e-b79a-c96327d2b19d`, identifies a different target:
`emi/etc/shop/00@0x801DE3C8`, with `func_801D6668` as its sibling. Both support
the same address-view pattern, but represent distinct experimental witnesses.
The [type ledger](bof3-types.md#editorial-provenance-and-recovered-witness) restores
that identity without treating the report as independently accepted execution.
Six editorial calls/246.454 summed lifecycle seconds therefore achieved concise
wording while losing one known witness; no general deduplication precision follows.

The Rizin rewrites used seven calls/274.734 summed lifecycle seconds for
**59→21** and **21→20** bullets. The first dropped qualitative caveats about
caller byte masks and structural contiguity, while the second merged two boundary
counts without establishing their source-population relationship. Those source
questions remain open in the Rizin ledger. The first Rizin worker reread its
written prefix; the second checked a constructed buffer. Type's first rewrite
checked its buffer, while its second reread the complete written bytes. Only the
second Rizin worker read the repository index. Its predecessor used
`noStagedFiles: false` to mean unverified; the others used `true` to mean no staging
performed. Neither boolean convention expresses the three distinct states clearly.

Proposed **ingestion/compaction tooling and reference**: follow clipped exports to
native messages by call ID, retain source identities when merging a reusable
pattern, and keep unknown staging separate from inspected state and mutations.
Acceptance: every clipped body recovered or explicitly unresolved; every merged
witness remains traceable to its own target/run; population counts cannot silently
collapse; preservation checks read the written revision. The concrete baselines
are one recovered export among 36 results and one recovered target identity, not
proof that the remaining history has no similar defects.

### Naming editorial child review

Both naming rewrites are fully reviewed, including arguments and native result
recovery. `tmp/observation-ingest/naming-editorial-metrics.json` pins the sources:

| Child | Section bullets | Calls | Lifecycle seconds | Input/output tokens |
| --- | --- | --- | --- | --- |
| `63e83dab-cc0f-40a2-8e79-75eaef786493` | 136→31 | 4 | 141.825 | 39,762 |
| `01efdcf2-ee91-415a-b9b3-a631a517df52` | 36→29 | 2 | 93.604 | 21,252 |

The six calls all returned without tool errors; summed lifecycle is **235.429 s**,
not campaign wall time. Both exited zero, but metadata acceptance was rejected by
the staged-files gate, with zero independent verification runs. Explicit skills
were empty. These are documentation costs, not naming attempts or renamed symbols.

The first exported read clipped at 32,533 characters. Native record 9 contains
50,784 characters: 32,512 shared prefix characters plus 18,272 recovered characters.
That native read itself stopped at line 293/326 under its 50 KB limit; the worker
then requested offset 294 and received the remaining 33 lines in a 10,728-character
result. Both bodies are now read. Across fourteen reviewed children, **40/42**
exported results equal native messages and both clipped exports are recovered.
The native-result manifest distinguishes export clipping from worker pagination.

Both workers verified written bytes, with the first rereading the preserved prefix
and the second the complete output. Their briefs nevertheless required preserving
the incorrect 15-run denominator and absolute capability ceiling outside the edited
section. Successful preservation therefore propagated known factual problems;
the [naming ledger](bof3-naming.md#editorial-provenance-and-retained-claims) records
their later correction. Neither worker read the repository index, under a brief
restricting reads to one file despite the injected repository entry instruction.

The second rewrite deleted its first five bullets by position: a duplicate planning
fragment, a historical prerequisite status, two unanchored allocator fragments,
and a measured 32/32 instruction, 128-byte lift whose filename collision was fixed
with an `_area01613.c` suffix. The latter's target/run identity remains unresolved;
discarding it is not proof that its ownership lesson was duplicated elsewhere.
Two subsequent pairwise merges account for **36−5−1−1=29**. Its foreign-repository
string check established no matching strings, not source provenance.

Proposed **reference/directive improvement**: report protected factual conflicts
without changing out-of-scope bytes; retain unresolved source leads when compacting.
Proposed **harness metric**: count discarded evidence units with recovered source
IDs separately from deleted bullets, and distinguish export recovery, input-page
coverage and written-byte preservation. This pair establishes one known discarded
measured lead, two successful preservation checks and one repaired pagination
chain; it does not establish semantic retention precision or a naming speedup.

### Second loop and RE editorial review

Full assistant text, reasoning, arguments and results for `0203e206-582d-4191-ac5c-daabefe59cb7`
and `a5b02f40-f2ca-4578-b74f-b05f4e061868` are reviewed. Source hashes and metrics:
`tmp/observation-ingest/second-loop-re-editorial-metrics.json`. The loop rewrite
used six calls/144.293 lifecycle seconds for **107→23** bullets; RE used three
calls/121.153 seconds for **49→27**. Together: **nine calls, 265.446 summed seconds,
89,960 input/output tokens**, zero failed tool calls. Both exited zero but retained
staged-files acceptance rejection and zero independent verification runs. Explicit
skill selections were empty; these costs do not count as lift or loop executions.

Both exported ledger reads were clipped, while native messages were complete:
loop 37,379 characters versus 32,500 exported, RE 34,057 versus 32,519. The recovered
tails after identical prefixes contain 4,900 and 1,559 characters. RE's next output
exactly enumerates the same 49 bullets and headings. All sixteen reviewed children
now have **51 joined results: 47 exact exports, four recovered native bodies**.

Loop's selection script mostly retained rows starting with `emi/`,
`exe/slus_004_22@` or `dispatchSoundCue at exe/`, then added three hand-written
summaries. That favored matching results over this ledger's mission-economics
purpose. It dropped five failed profile searches, a 27,355-error permuter run and
repeated-probe cost observations. It also removed the synthesized path for
`updateStateMachine`, whose original fragment gave `GAME.EMI#0`; this avoided
retaining an inferred target path but lost the documentation-drift witness there.
The witness remains in the docs ledger. One merge changed “a broader review” of
730/939 metadata records into “the same review” without proving that identity.

RE retained row 10 and rows 18–46, merging three rows into other retained rows:
**49−19 discarded−3 merged=27**. Among the nineteen discarded rows were the
**30/30 fan-out race** claim and a baseline-excluding sweep regression. They had
measurements but no target-qualified prefix; the former belongs to loop economics,
and the latter crosses loop and RE ownership. The sparse-switch row was also
dropped, but its finding already remained in protected curated finding 17. These
different dispositions cannot all be counted as successful semantic deduplication.

RE verified the actual written prefix; loop checked a constructed buffer and later
reread counts/markers, without comparing the final prefix to its original bytes.
Only loop read the required repository index. Both link checks measured path
existence, not anchors or semantic provenance.

Proposed **compaction/reference improvement**: classify evidence by task ownership
before filtering, preserve transfer pointers and unresolved source leads, and never
merge review populations by wording alone. The [loop ledger](bof3-lift-loop.md#important-observations)
restores the operational leads with original-run verification still pending; the
[RE ledger](bof3-re.md#editorial-provenance-and-sweep-baseline) retains the sweep
obligation. Measure evidence units retained/transferred/unresolved against the
reviewed input set, alongside actual written-byte checks. Bullet reduction and
absence of foreign-path strings do not supply that denominator.

### First loop editorial review

The complete `4f719013-41b2-4436-868d-47ac50479ef9` transcript, arguments and
native inputs are reviewed. It reduced **461→35 bullets** using **six calls,
174.464 lifecycle seconds and 71,452 input/output tokens**. Four calls read the
ledger and two inspected/rewrote it. All six returned without tool errors; exit
was zero but staged-files acceptance remained rejected, with no independent
verification run or explicit skill selection. This is editorial cost, not loop
execution. Reproduction: `tmp/observation-ingest/first-loop-editorial-metrics.json`.

The worker followed native pagination at offsets **1, 219, 369 and 528**, covering
the reported 535-line file without a skipped continuation. Its first three native
results contain **50,732 / 50,850 / 50,738 characters**; their exports contain
32,459 / 32,600 / 32,472. The respective recovered tails beyond exact prefixes
are **18,294 / 18,271 / 18,287 characters**. The final 2,448-character continuation
is exact. Review read 430 new lines and reconciled 110 exact lines against already
reviewed native results, retaining every source pointer in
`tmp/observation-ingest/first-loop-read-reconciliation.json`. Seventeen reviewed
children now have **57 joined results: 50 exact exports and seven recovered bodies**.

The selected output preserved the failed-profile/permuter leads later removed by
the second loop rewrite. It also retained separate equal-score/prefix-regression
examples and correctly limited old compiler-profile findings to the old source
shape. Its input, however, mixed claimed measurements with reasoning, incomplete
metadata, foreign-project references and unrelated memory/gateway operations.
The introductory claim that narration and first-person planning were excluded was
false. Full worker input coverage does not make that input a valid evidence corpus.

The rewrite dropped the **36/40 vulnerable-parser pass** lead and two **30-run
status benchmarks** summarized in the [test ledger](bof3-test.md#recovered-framing-and-status-benchmark-leads).
It also discarded examples of confused target/lane selection, including a report
that tested `0x801E60DC` against a file tagged `0x801E5824`. These are operational
lessons even without a successful matching delta. Their original runs remain to
be recovered; neither their truth nor their resolution follows from compaction.

The worker compared its constructed 4,696-byte prefix before writing, then reread
the file for links and marker/count checks. It did not compare the written prefix
to the baseline, despite the final preservation claim. The injected repository
index instruction was also not followed. Proposed **reference/tooling improvement**:
keep full-input coverage, evidence qualification, witness retention and post-write
preservation as separate checks. Baseline here: three recovered exports, a complete
four-page input, and at least two discarded test-operation leads. The 92.4% bullet
reduction is storage reduction, not measured semantic retention or skill speedup.

### Ownership reference edit validation

Older parent `01a0c062`, records **49–95**, made two reference edits while
designing source-identifier naming; shared costs and revision context are in the
[plans ledger](plans.md#ownership-edits-and-confirmed-scope-revision). Both edit
calls succeeded, but the cosmetics edit introduced **two incorrect cross-skill
links**: from `bof3-naming/references/BYTE_SAFE_COSMETICS.md`,
`../bof3-macros/SKILL.md` and `../bof3-types/SKILL.md` resolve inside
`bof3-naming/`, not beside it. The historical sibling paths required another `..`.
The reasoning explicitly miscounted the directory traversal (record 61).

The follow-up greps reread both links and checked ownership wording/router code,
but did not resolve destinations. Thus **2/2 newly added cross-skill links were
incorrect at this boundary**, despite successful edits; no link-validation result
is recorded in this prefix. The separate macro-guide link had the right traversal
and is not part of that denominator. Records **170–173** later repair both paths
and report zero missing file destinations. The edit-to-repair result interval is
**300.367 observed seconds**, including intervening capability work; it is not
repair execution time. The checker stripped fragments and emitted no checked-link
denominator, so its zero result did not prove anchors or all reference semantics.

Proposed reference/tooling improvement: resolve every changed destination from its
containing file immediately after a skill-reference edit, independently of text
search and routing checks. Measure invalid new links/new links, time to detection
and recovery calls; require zero unresolved edited local links before claiming
reference validation. This audit records the historical defect, not a current
skill edit or a verified present-day broken link.

### Capability guidance and premature closure

The same parent's records **155–180** used **15 calls/58.256 observed seconds**
for task closure, naming guidance, link repair and smoke checks. Full source pin
and phase accounting are in `tmp/observation-ingest/older-guidance-diagnosis-metrics.json`.
One new reference and four edits covered the skill, routing documents and repaired
cosmetics links. Two script-smoke targets and five dispatcher `--help` checks
passed. These are supporting documentation operations, not explicit docs-skill
invocations or proof of domain acceptance.

The written source-identifier guide promoted stronger behavior than the reviewed
implementation demonstrated. It promised declaration order although discovery
sorted by selector/kind/name; advertised `TARGET@0xADDRESS` although transaction
membership used the supplied selector literally against unprefixed hexadecimal
IDs; and equated native exit-zero requirements with unchanged object bytes. That
last requirement was added as an **exact-lift-only** restriction after implementation,
without a successful native transaction. The guide omitted the implemented
diagnostic bypass returning `verified: true`. Its one-name-per-function wording
did not address separate bindings with the same spelling. These are historical
code/reference discrepancies, not a fresh execution of those recipes.

The task tracker accepted naming discovery/transactions before those gaps were
resolved. The later ownership closure characterized remaining dead-macro removal
rules as delegation, although the literal no-macro-policy criterion still differed.
The written reference received no tool reread in this prefix; the skill body did.
Existence-only links and help output could not establish evidence, approval,
transaction or rollback behavior. See the [naming implementation review](bof3-naming.md#initial-source-identifier-implementation)
for the underlying checks and uncovered cases.

Proposed docs directive/reference improvement: reconcile each operational claim
with the exact CLI/parser behavior and required evidence before marking its task
complete. Measure unsupported claims/reviewed claims, executable example failures,
changed-link failures and correction latency separately from smoke passes. A
documentation harness should distinguish file destinations, anchors, syntax and
behavior checks, recording a denominator for each. Acceptance requires every new
recipe and safety claim to have matching proof or an explicit limitation; proposals
here do not modify the skill or authorize new tests.

### First RE editorial review and cohort closure

Run `0380ea14-6ad6-4f2b-aab5-dbbe57a6e1bc` reduced **416→32 bullets** with
**eight calls, 172.725 lifecycle seconds and 77,685 input/output tokens**. Its
complete transcript, reasoning, arguments and four native input pages are reviewed;
source hashes and timings are in `tmp/observation-ingest/first-re-editorial-metrics.json`.
Pagination used offsets 1, 272, 426 and 585. Three clipped exports omitted tails
of **18,151 / 18,180 / 18,071 characters** beyond identical prefixes; the final
result was exact. The read reconciliation records 409 newly reviewed lines and
200 exact lines linked to previously reviewed native bodies. Export clipping did
not demonstrate incomplete worker input.

The worker corrected one invented symbol identity, replacing derived word symbol
`D_80143F84` with the input's high-halfword alias `D_80143F86`. That is observed
self-correction, not evidence validating the underlying matching claim. It checked
the constructed prefix before writing, then checked counts, markers and paths;
it never compared the written prefix with its baseline. The required index read
was absent. All eight calls returned without tool errors, but metadata acceptance
was rejected and independent verification remained zero.

The rewrite retained sparse-switch, race and sweep-baseline witnesses subsequently
lost or merged by the second rewrite. It dropped other useful operational leads:
inconsistent score definitions, a reported 36-variant mission and candidate-screening
classification. The [RE ledger](bof3-re.md#recovered-measurement-and-screening-leads)
and [loop ledger](bof3-lift-loop.md#important-observations) preserve those leads
without promoting historical assertions into verified current behavior.

Both nine-child editorial cohorts are now fully reviewed: **18 runs, 65 calls,
2,203.014 summed lifecycle seconds and 583,675 input/output tokens**. Cache reads
remain separate. Across all 18 measured runs, calls have median **3**, nearest-rank
p90 **6**, maximum **8**; lifecycle seconds have median **124.392**, p90 **172.725**,
maximum **174.464**. There was **one failed tool call/65** (the repaired emulator
editor's syntax error), **18/18 zero process exits**, **18/18 rejected metadata
acceptances**, and **zero independent verification runs**. Rejection labels do not
establish semantic failure; exit zero does not establish preservation. Empty skill
selections make this documentation-method evidence, not 18 explicit skill invocations.
Aggregation: `tmp/observation-ingest/editorial-cohort-metrics.json`.

All **65 result bodies** are joined to native records: **55 exact exports and ten
recovered bodies**. This closes these editorial transcripts, not the original
domain executions or wider history. Proposed directive/reference improvement:
measure retained, transferred, corrected and unresolved evidence units against
the complete input, with written-prefix verification separate from semantic
preservation. Proposed export-tool improvement: retain native result pointers and
explicit clipping metadata; acceptance requires recovery of every clipped body
before declaring coverage. Neither bullet reduction nor these runtime statistics
establishes a quality improvement or speedup.

Historical findings below remain subject to source-level reconciliation.

- The prior harvest's measurement/grammar filter was not a complete history read:
  `/tmp/harvest2.py` rejected short text, truncated candidates to 420 characters,
  then required a measurement and particular phrases. A complete file cursor
  therefore proved traversal, not durable-lesson coverage (2026-09-26 script audit).
- `.pi-subagents` contains 3,526 JSONL files absent from that harvester's source
  list. Parent `custom_message` records also carry child results outside ordinary
  assistant/tool messages. Source-format coverage must precede claims about the
  amount of reusable history (2026-09-26 filesystem/schema inventory).

- Documentation context reached 48,459/48,500 bytes (41 free), requiring about 310 bytes of trimming for a 350-byte addition; a later snapshot reached 51,245/51,300 (55 free).
- Commu extern-array rebinding improved 44/45 matching instructions to an exact byte match; docs/agents/matching-playbook.md recorded the lever.
- stageEmiTransferSlot improved 66/67 to 67/67 matching instructions at 268 bytes by resolving +0xA0 nop versus li v0, 1; docs/specs/runtime/compiler-provenance.md still presented residual guidance after the exact record was added.
- GAME.EMI#0@0x80197378 (updateStateMachine) matched 155/155 instructions and 620 bytes, contradicting the near-exact status and scheduling work item in docs/specs/runtime/frontend.md.
- battle/15@0x800AF66C (func_800AF66C) matched 19/19 instructions and 76 bytes with REGISTER_PIN; compiler documentation still described a 2/19-instruction residual and next matching step.
- A multiline-aware link audit found 73 fragment-bearing occurrences, 52 distinct pairs and zero broken targets or anchors, correcting 72 by including docs/agents/lessons.md:109-110. A differently scoped root-plus-docs audit excluding toolchains found 70 occurrences (69 excluding plans), 49 pairs and zero failures; these totals were not interchangeable.
- Documentation-refresh closure exceeded context limits: Pi 69,392/69,000 bytes, reverse/review 106,875/104,124 against 100,000 each. Another test-skill-scripts.py run measured 107,727/104,982 versus baseline 104,038/101,293, attributing growth to in-scope documentation rather than Pi edits.
- Documentation validation passed 71/71 anchors, found zero missing relative links and zero cross-file duplicate windows of 6-10 lines in docs/specs, and confirmed 30/30 distinct bin commands.
- Audio smoke validation was blocked by exit statuses 127 and 139, CPU-ISA constraints and no sudo; the documentation plan retained the blocker.
- Formatting validation flagged six files: analysis/naming_audit_v3.py, analysis/naming_readiness.py, commands/naming_audit_preflight.py, test_naming_audit_check.py, test_naming_audit_preflight.py and test_rev_query.py.
- Tool-surface and cleanup-skills consolidation preserved all 38 Pi files byte-for-byte during compaction; 264 surface tests, 159 cleanup/router tests and the full 812-test suite passed.
- Live metadata refresh for src/bof3/scenario/func_801F7790_scena16.c measured 171/333 matching instructions (51.35%), 1,332 original versus 1,232 current bytes; func_801F7CC4_scena16.c measured 137/421 (32.54%), 1,684 versus 1,436 bytes.
- In scena00, taking the state address once into a local pointer and reading/storing through it corrected the collapsed frame/prologue and missing s0-held address, improving 52.17% to 100%.
- docs/specs/formats/dialogue-text.md validation resolved 25/25 references and classified 145/200 entries as conforming, with 55 explicitly unsupported.
- func_801DAB90 remained partial at 281/339 instructions (82.89%) with byte DIFFER; metadata agreed with the live result.
- The [fully reviewed reference-repair case](openspec-apply-change.md#complete-parent-case-documentation-repair-and-follow-up) reduced broken links from eleven to zero across 1,367 enumerated references, including external links that were not fetched. The historical preservation claim relied on unchanged line counts and 39 dirty paths; those counts do not prove unrelated bytes were preserved.

### Routing migration after provider failure

Older parent `01a0c062`, records **1369–1395**, contain **14 calls/245.774
observed seconds** for the user's TOC-only index request: nine preparation reads,
one child launch, one wait, two scripted edits and one verification batch. Ten
reasoning blocks contain 41,107 characters. The first edit request came **216.913
seconds** after the user request. These are one migration's costs, not population
percentiles or demonstrated avoidable time. Source hash, record joins and phase
metrics: `tmp/observation-ingest/older-docs-routing-metrics.json`.

The explicitly selected `bof3-docs` child is the **same `b3c6b526` already counted
above**, not another run. Four generation failures preceded zero tools; the parent
waited 23.367 seconds, distinct from the child's 20.338-second event span. The
combo metadata names `ninerouter/gpt-combo:medium`; its provider error reports an
underlying usage-limit failure. Parent execution began 16.316 seconds after the
failure notice, without the notice's requested resume attempt. This records the
historical recovery choice, not evidence that resume would succeed or that the
documentation method failed. Parent/child spans overlap and must not be added.

**Transfer and verification had different outcomes.** Eight directive sections
moved to five owner files, all eight headings appeared exactly once, and INDEX
shrank **199→82 lines**. The second script rewrote **18 incoming references in
16 files**, with zero residual old-anchor matches in its searched roots. Including
the index and Codex owner, the two scripts changed eighteen distinct paths. Despite
calling the search “tracked,” it used recursive Markdown enumeration, not Git's
tracked-file inventory. It correctly excluded session/output roots, whose hits
had inflated the initial broad search. Its scope also included a plan reference;
briefs combining “rewrite docs/**” with “do not edit plans” need an explicit path
set before execution.

The section-transfer script preserved link text without rebasing it. **All three
local outgoing links in the moved bodies changed resolution incorrectly**: two
`agents/codex.md#…` destinations and `agents/combiner.md` became paths under
`docs/agents/agents/`. The incoming-anchor regex could not repair these because
none targeted INDEX. This is **3/3 moved local destinations**, not the whole
repository's broken-link rate. It follows from the retained original text and
executed transformation; it is not a claim about today's link state. No historical
full-link check is shown. Heading existence and zero old-anchor matches missed
this concrete defect. Likewise, skipping transfer when a heading already exists
could discard a different body on rerun; that branch was not taken here and is
a recovery risk, not an observed loss.

The first scoped gate **failed with rc=2** on two raw naming debts from the lift
that finished during preparation. Ruff passed; source validation was unreached.
The separate focused run printed **88 passed in 15.25 seconds**. The context
smoke piped the renderer through `head -8`, exposing only the RE skill's opening;
it supplied neither the renderer's independent exit status nor evidence that
required `docs/agents` bodies reached each applicable role/mode. Static profile
path greps did not establish effective emitted context after the move. No complete
owner-body reconciliation or independent documentation review is shown.

The parent then admitted two lift baseline rows and reran only `symbols check`,
yet claimed scoped green before rerunning the recipe. Record **1403** eventually
shows actual scoped success, **rc=0/9 seconds, 939 valid/zero invalid**, after the
lift review and analysis/index refresh. User request to that result was **643.105
seconds**, including unrelated lift work; it is not pure documentation latency.
The full enclosing slice through 1406 used twenty calls/1,631.504 seconds, including
the next lift's wait. Charge only the fourteen-call migration slice to this case;
its availability failure, successful structural transfer and unresolved reference/
context acceptance remain separate outcomes.

| Improvement destination | Concrete proposal | Baseline and acceptance measurement |
| --- | --- | --- |
| Skill directive | In `bof3-docs`, distinguish structural completion, reference validity, obligation preservation and context delivery before claiming completion | This case had eight heading passes but three changed link resolutions and one premature gate claim. Comparable moves must report each applicable result separately, with unresolved checks explicit. |
| Reference material | Extend the relocation procedure in `DOCUMENTATION_REPAIR.md` with incoming/outgoing destination mapping, full owner-body comparison and an explicit mutation path set | Rebase and resolve every moved local destination; measure checked/applicable destinations and preserved/applicable obligations. Zero unresolved changed destinations and reviewed duplicate-body handling are required; line reduction is supplementary. |
| Harness tooling | Have `harness.context` expose emitted owner paths/sections and truncation for each exercised role/mode; capture renderer status independently of display limits | The eight-line smoke established no required-owner delivery. Evaluate the applicable role/mode matrix against required owners and report missing/truncated content; do not infer delivery from declared paths. |

These are evidence-backed proposals, not implemented policy or measured savings.
Availability reporting should retain failed starts alongside resumed/parent-completed
attempts without counting the fallback as a second accomplished document task.

### Annotation retirement without transfer evidence

Older parent `01a0c062`, records **1443–1471**, implemented the historical user's
request to retire `MATCHING_AID`/`MATCH_AID` and put reusable matching knowledge
in lifting references. This supporting documentation case used **15 parent calls
(9 bash, 6 edit) / 184.019 observed seconds**, with ten reasoning blocks/40,085
characters and zero tool-error flags. First source mutation followed the request
by **43.871 seconds**; first directive mutation by **60.628 seconds**. These
latencies include reasoning and inspection, not just execution. Reproduction and
source pin: `tmp/observation-ingest/older-annotation-retirement-metrics.json`.
No explicit docs child was launched; source changes remain RE-owned work.

The script removed **20 standalone comment blocks in 20 source files** and
reworded one metadata-header reference. Six reference/directive files changed:
matching guide, playbook, compiler-provenance document, naming cosmetics reference,
RE skill and loop skill. The new guidance requested residual class, decisive C
shape and measured before/after capture. This establishes an instruction change,
not twenty completed knowledge transfers or measured future matching gains.

**Preservation was unproved.** Before deletion, the parent inspected marker lines,
comment boundaries and metadata-token presence, without returning every complete
comment body. No source-comment-to-reference map reconciled the twenty removed
blocks. Existing reference content might already preserve some lessons; the record
does not establish either wholesale loss or complete retention. The replacement
playbook example retained a register-liveness explanation, while its old explicit
removal condition disappeared. It also prohibited selectors/addresses/timestamps
in general guidance without supplying a separate provenance receipt. Generic
reference wording and traceable experimental evidence need distinct destinations.
The compiler-provenance edit retained a hardcoded source line range after comment
removal, without checking that range's new meaning.

**Checks measured different scopes.** The removal script found no remaining tokens
in `src/include`, but strict metadata parsing failed on **15/20 touched sources**.
The parent called those pre-existing defects without a pinned pre-edit parser
receipt for those fifteen files. A later scoped gate passed **rc=0/9 seconds,
941 valid/zero invalid**; these different validators cannot be merged into one
metadata-success count. Native spot checks reported six exact results, covering
**6/20 affected sources**, with no complete before/after object comparison. The
focused test command ran skills and naming units: **694 passed, 2 skipped in
36.77 seconds**. Its label also said “docs,” but no separate docs unit was selected;
the pipeline retained summary text rather than pytest's independent exit status.

The whole-tree marker search counted **589 matching files** after excluding `out`
and `.pi/sessions`, but still included `.pi-subagents` history. Only the first
twenty matching lines were displayed. Those samples did not establish that every
remaining match was historical or that a complete tracked-owner inventory was
clean. Preserving historical logs was appropriate; scope classification needed
an authored-file inventory, not a broader deletion. No independent documentation
review or measured later-mission benefit appears in this retirement slice.

| Improvement destination | Concrete proposal | Baseline and acceptance measurement |
| --- | --- | --- |
| Skill directive | Treat retiring annotations and preserving their useful evidence as separate outcomes; report unverified transfers before declaring completion | Twenty removals, no demonstrated complete transfer map. Every removed evidence unit needs a retained owner, a reviewed duplicate, or an explicit unresolved disposition. |
| Reference material | Add an annotation-retirement procedure to the documentation repair reference: inspect complete bodies, preserve removal conditions and retain source receipts outside generic guidance | Count inspected bodies/removed bodies, reconciled lessons/applicable lessons, and verified changed source references/applicable references. Require zero unreviewed losses; measure later corrections separately. |
| Harness tooling | Emit scoped authored-file matches, historical matches and unsupported paths separately; retain validator identity, touched-source coverage and command exit status | Baselines are 589 mixed-root files, 15/20 strict-parser failures and 6/20 native spot checks. Future receipts must reconcile scope and validator differences rather than calling an aggregate green result complete coverage. |

These are proposed improvements, not new policy or authorization to edit source.
Evaluate subsequent comparable lifts for accepted outcomes, repeated searches,
review repairs and elapsed cost before crediting the new capture directive with
improved accuracy or speed. Reference growth alone is not that evidence.
