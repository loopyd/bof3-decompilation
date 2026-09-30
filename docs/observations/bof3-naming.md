# Naming method observations

Method-performance ledger for BOF3 naming missions. It lives here, not inside
`$bof3-naming`: a skill carries **directives**, this guide carries the
**measurements** behind them. Read it before designing a naming mission, and
after each mission add one audit row and converge the finding into a directive
in `$bof3-naming` or `bof3-namer` — prose alone does not count as convergence.

## Iteration audit log

One row per mission, with the measurement that decided its verdict. Converged findings become
directives below; a row that changes nothing is itself a finding.

| Iteration | Mission | Tools | Duration | Method verdict | Converged into |
| --- | --- | --- | --- | --- | --- |
| 1 | 9 × `bof3-namer` evidence, parallel | 56–112 each | 9/9 killed at ~914 s | **Wasteful** — parallel writers queueing on one lock, missions sized by symbol | D1, D2, H3, H4 |
| 2 | Serialized pilot: 3 audits, 1 apply, 1 review | 249 total; 29–92/run | 1,329.071 s parent wall time | **Bounded, but no rename** — all 5 children completed; 4 audited target/row pairs stayed blocked; independently reviewed apply was a no-op | D1, H4; prerequisite routing below |
| 3 | `selected_call` producer | 40 | 1157 s | **Partial** — internal capture/replay exercised; canonical imported-row caller discovery remained incomplete | D5; canonical-proof corrections below |
| 4 | canonical journal proof | 35 writer calls; review separate | 1199.635 s writer lifecycle | **Reported independently reproduced** — the earlier 4-call/128.483 s prompt diagnosis is a different task, excluded here | D11 |
| 5 | `owner_body` producer (killed at the deadline) + interactivity handshake (landed in the **following** run, exit 0) | 51 | 1805 s killed | **Mixed** — retained artifacts passed review; canonical proof refused during reviewed-range validation. The handshake recovered both controls; broader producer acceptance remained unproved | D12, D13, H1, H2 |
| 2026-09-26 audit | Parent launch of nine-symbol campaign, records 911–939 | 18 parent calls | 121.426 s through steering | Invalid launch recovered; contracts discovered after launch; nine corrections queued with consumption unconfirmed | Proposed dispatch and recovery improvements below |
| 2026-09-26 history audit | Source-identifier implementation, parent `01a0c062`, records 96–154 | 32 parent calls | 198.452 observed s | First test run 8 passed/1 failed; corrected 9 passed, expanded 10 passed; no successful native transaction demonstrated | Proposed parser, verification and receipt improvements below |
| 2026-09-20 parent, records 446–461 | Supporting naming-debt baseline admission | 8 shell calls | 35.486 observed s | Gate rc=0 after one admitted row; no semantic naming repair | Admission-boundary evidence and exact-diff proposals below |
| 2026-09-20 parent, records 1091–1100 | Supporting campaign baseline admission and parser recovery | 5 shell calls | 40.372 observed s | 17 debt rows admitted, zero renames; 16 malformed rows repaired before gate rc=0 | Structured debt output, scoped admission diffs and separate outcome measures below |
| 2026-09-21 parent `01a0c062`, records 1407–1433 | Supporting baseline-recorder implementation | 14 parent calls | 153.997 observed s | Four rows admitted, zero renames; false malformed-row diagnosis corrected; review/scope binding unproved | [Recorder audit](#baseline-recorder-recovery-and-review-boundary); scoped admission and directive-order proposals |

Cost note from iteration 5: the handshake costs **0.30–0.47 s** set/read-back against a capture of
**0.003–0.043 s** — it dominates wall time while being negligible in absolute terms, and the first
handshake also absorbs engine start-up. Re-asserting it per command is deliberate (a reused session's
setting can be changed in between), so the cost is accepted, not optimised away.

## Measured performance

### Reproducible dedicated-agent cohort

The 2026-09-26 audit found **13 unique `bof3-namer` metadata/transcript pairs**,
dated 02:08–03:07 UTC that day. Nine report an explicit timeout check and exit 1;
four exit 0 with attested acceptance. Thus the recorded timeout rate is
**9/13 (69.2%)**, not an independently accepted naming-result rate.

All 13 lack metadata `toolCount` and `durationMs`. Tool-start IDs recover
**942 calls**, median **73**, p90 **98**, maximum **112**. Observed transcript
spans total **9,219.3 seconds**, median **899.2**, p90 **899.4**, maximum **899.5**;
these are event spans, not end-to-end recorded durations or campaign wall time.

Lifecycle status files subsequently recovered for all 13 confirm those **942
calls** and supply start/end time: median **913.892 s**, p90 **914.438 s**, maximum
**914.449 s**. They are distinct from the shorter transcript spans. Source:
`/tmp/pi-subagents-uid-1000/async-subagent-runs/<runId>/status.json`, joined by
run ID; source hashes and scalar snapshots are retained in
`tmp/observation-ingest/runtime-metrics.json`.

| Outcome cohort | Runs | Tool calls | Observed seconds | Limits |
| --- | --- | --- | --- | --- |
| Explicit timeout | 9 | 741 total; 56–112/run | 8,093.9 total | Partial artifacts still require inspection; timeout does not prove no useful output |
| Exit zero / attested | 4 | 201 total; 40–73/run | 1,125.4 total | Attestation does not prove semantic naming acceptance |

Timeout runs consumed **78.7% of observed calls** and **87.8% of summed event
spans** in this cohort. The existing nine-writer lock-contention episode is a
priority for remediation, but these aggregate fields alone cannot establish
that every call was wasted or that serialization caused success.

Provenance: the 13 `*_bof3-namer_meta.json` and matching transcripts under
`.pi/sessions/subagent-artifacts`. `tmp/observation-ingest/runs.jsonl` records
exact paths/hashes; `transcript-metrics.jsonl` records per-run counts and spans.
Nearest-rank p90 is used. Broader `bof3-cleanup` and generic worker histories
still need mission-level attribution before joining this skill's denominator.

**Next measurements for H3/H4:** annotate each failed run's lock owner, wait and
retry calls, target count and retained artifacts; compare serialized missions
of comparable scope. Acceptance remains no lock-retry loops and no deadline
kills, with unchanged evidence/transaction gates. Track timeouts/N, calls per
valid evidence result and unresolved rows separately from proposed renames.

### Parent/child reconciliation and pilot outcomes

The earlier **15 namer / 10 failed / 5 completed** count is a parent/child
double-count: the 13 child records plus the stopped `f9f00993…` and completed
`d920a1b9…` workflows produce exactly those totals when a parent is classified
by its first child's agent. Both parents reference already counted children.
The corrected dedicated-namer population is **13 / 9 / 4**; workflows are a
separate unit, never extra namer attempts. The original aggregation command is
preserved in parent session `01a0dad5…`, record **1513**: it recursively took the
first `agent`, `status` and `durationMs` from each directory, ignored parent/child
identity and project scope, and labeled durations ≥890 seconds as timeouts.
That duration threshold is not terminal-state evidence. Its other role totals
are retired from this skill's metrics rather than pooled with naming work.

The serialized workflow `d920a1b9-fbeb-471b-980f-9b16675f122d` has three naming
audits (`63baf816…`, `92bd40e9…`, `ff503fc0…`), cleaner apply `854ee251…`, and
reviewer `a6291437…`. Their lifecycle durations are respectively 283.874, 275.737,
250.280, 157.847 and 357.770 seconds; tool counts are 44, 44, 40, 29 and 92.
The parent elapsed 1,329.071 seconds; summed child time was 1,325.508 seconds.
All five completed, but the three audits left **four target/row pairs blocked**;
the apply made no identity change and review accepted that no-op. Zero renames
is a valid safety outcome, not successful debt removal. The later separate
`b0e6c635…` namer completed in 336.309 seconds/73 calls and also left its function
row blocked by missing runner capabilities.

The failed parallel parent elapsed 929.889 seconds for nine children; its summed
child time is much larger because runs overlap. Comparing that wall time to
the narrower five-child pilot does not measure a serialization speedup: task
count, scope and prior checkpoint reuse differ.

Sources: both parent `status.json`/`workflow-receipt.json` files, their children’s
matching output reports under `.pi/sessions/subagent-artifacts`, and parent Pi
session `01a0dad5-86b6-74b4-97f0-9382ab99ade4` records 1020–1032 and 1063–1068.

### What the waste actually was

The former “ten of fifteen produced nothing” conclusion was too broad. Nine
unique children timed out, and the pilot reused a committed checkpoint from a
timed-out run. Diagnose preventable work separately from useful partial output.
Three historical hazards motivate the existing directives:

1. **Parallel writers** contending on `out/.reviews.lock` — nine children fanned out, nine
   timeouts, several recording verbatim that they were waiting for a foreign lock.
2. **Missions sized by symbol, not by work** — a 22-target symbol in a single mission.
3. **Unverified premises in the parent's brief** — a supported wrapper owner assumed without
   a demonstrated eligible row; an `owners` query assumed to work for data rows; a probe assumed
   to prove journal freshness when its operation set did not match its checkpoint.

### Lessons and improvement candidates from the serialized pilot

| Measured evidence | Proposed guidance/tooling improvement | Acceptance measurement |
| --- | --- | --- |
| Three audits / four target-row pairs remained blocked; the subsequent 29-call apply and 92-call review established a no-op | Parent handoff and naming references: carry row readiness and the supported non-publishing preparation check before requesting an apply leg | Report unnecessary apply launches/N and valid blocked outcomes/N separately; never bypass independent review or fabricate a proposal to improve throughput |
| All three audits hit missing canonical campaign entries; one first passed a symbol as TARGET | Parent request construction: validate target/report tokens and campaign prerequisites before dispatch | Canonical first-call success/N improves; missing-state missions return one bounded prerequisite result without implicit initialization |
| A completed shard resumed with `terminalized:false`; four rows stayed blocked | Harness result presentation: distinguish collected operations, committed checkpoints, terminal rows and complete target reports | Summaries expose all four states with their denominators; no collected shard is counted as a proposed rename |
| Apply report incorrectly said preparation always writes; reviewer proved `--check` non-publishing and kept the report hash unchanged | Naming operation reference: contrast preparation, its check mode and identity application with exact outcome meanings | Bounded reviewers reproduce the preparation refusal without writes; no false permission blocker or lost validation |
| Twenty stores to one byte provided one repeated mechanism, not twenty independent corroborators | Naming evidence reference: count independent mechanisms and ownership coverage separately from xref quantity | Every semantic proposal identifies genuinely independent corroborators; repeated accesses cannot inflate its evidence count |

The reviewer also showed that a successful `symbol` router prefill does not prove
proposal readiness, and target-scoped symbol checks omit repository-wide naming
debt/binding drift. These distinguish the failing layer from the final blocked
outcome; retain them even when an apply no-op was correct. The complete reports
for the five pilot child IDs above were read. Their transcript-level reasoning
and tool-result review remains part of the broader pending ingestion.

### Serialized pilot: parent cost and limits of acceptance

Parent `01a0dad5…` records **1036–1075** were fully reviewed, including all
launch, correction and design-edit arguments. Metrics are reproducible with
`tmp/observation-ingest/naming_pilot_parent_metrics.py` and
`naming-pilot-parent-metrics.json`, using the parent hash recorded below. Runtime
parent `d920a1b9…` joins the same five children already counted above. Its status
source hash is `582329f05bef5e203aaa7dbe81204d635fdcf23b489121bced43e18d4f3ef8b7`.

| Parent phase | Calls | Observed seconds | Result |
| --- | --- | --- | --- |
| Preflight and launch, 1036–1044 | 4 | 56.370 | Baseline parser failed; report targets recovered; five serial stages launched |
| Late ownership correction, 1045–1062 | 9 | 74.221 | Map evidence located after dispatch; one correction queued; Decision 2 amended |
| Outcome diagnosis and record, 1069–1075 | 3 | 29.222 | Blocked report inventory measured; Decision 8 written; schema/whitespace checks passed |
| Whole parent episode | 16 | 1,429.432 | Includes 1,251.522 s from launch report to native completion notice |

Recorded usage: **129,556 input + 2,658,688 cache-read + 28,561 output =
2,816,805 total tokens**; 17,164 reasoning tokens are not additive. Seventeen
reasoning blocks contain 68,997 characters, including 16,270 before launch.
These parent costs overlap child elapsed time and must not be added to it as
wall-clock cost. No polling loop was used to await native completion.

The pilot improved its scope to **four selected target/row pairs** across three
audits, followed by one apply and one review. Its four reports contained **218
inventory rows** (84/76/40/18), a different denominator. Five serial awaits and
explicit targets removed the original nine-way writer fan-out. However, four
minutes *per rung* over five rungs totaled 20 minutes for a one-target mission,
or 40 for two targets, despite a 15-minute child deadline. Early-return prose
did not encode a coherent total work budget. The unconditional apply stage and
6,000-character handoff truncation remained; final status formatting read missing
`.status` fields and printed five `undefined` values. The terminal receipt, not
that string, established five completed children.

Preflight first guessed the baseline JSON shape and failed with `AttributeError`.
It recovered target strings from report contents but read `status` where the
authoritative field was `rung_status`; `None` was not a terminal state. Four open
rows were then generalized to “all 26 reports initializer-only; no evidence
collected.” The first child's **executed 0/resumed 1/skipped 1** result directly
refutes that claim: a committed checkpoint survived the timeout, even though the
report remained blocked and `terminalized:false`. The boss025 child also resumed
a checkpoint. Receipt-backed collection and report terminalization are separate.

Ownership discovery again followed dispatch. Three empty data `owners` queries
and one function control motivated a useful distinction, but did not by themselves
prove the query's universal capability boundary. Exact-address map searches found
`counter1` in area008 while area134 remained raw; they established a spelling lead,
not common runtime ownership. The later review showed both overlays outside the
address's payload and identified resident-image containment. Exact-text searches
under `config/targets` cannot establish that no semantic name exists anywhere.
Likewise, one clipped function-owner result did not prove all 40 proposed function
rows actionable, and four blocked data rows did not prove all data debt inherently
unrenameable. The parent wrote these broader conclusions into its design record.

The **1,004 blocked/0 proposed** count summed row occurrences across 26 whole-target
reports without deduplicating target/row identities. It establishes the scanned
reports' state, not 1,004 distinct debt units replacing the original 61 findings.
No proposed row there could support preparation; it does not establish a permanent
capability ceiling. The repeated `owners`/map exploration and scope changes make
this a candidate for better preflight guidance, not proof that more analysis is
always unnecessary.

Independent review accepted the **no-op only**, with two inaccurate sub-claims and
three unverifiable claims: transported scope text, the applier's internal PRE/POST
fingerprint, and absence of build/toolchain writes. Its aggregate lift-status count
alone did not establish a fresh native byte comparison. The parent nevertheless reported the full
chain fixed, zero damage and zero lock waits; those claims exceed the accepted
outcome and available measurements. No rename, positive application, rollback or
whole-target completion was demonstrated. The reviewer did reproduce preparation
refusal using non-publishing `--check`, preserving the report's full hash.

Proposed guidance/harness work: preflight the actual symbol kind and query scope
before dispatch; encode one remaining-budget allowance across all rungs/targets;
pass complete artifacts with verified identities instead of truncated prose; and
report row readiness, checkpoint reuse and review uncertainty independently.
Measure late correction calls against this **nine-call** phase, returned-in-budget
missions/N, proposed/blocked rows by frozen identity, and positive transactions
separately from reviewed no-ops. Full child transcript review remains pending;
the observed five completions do not establish a same-scope speedup.

### Successor first-row attempt: capability diagnosis before scaling

Parent `01a0dad5…`, records **1183–1199**, same SHA-256 recorded below. All eight
reasoning blocks (40,333 characters), eight assistant text blocks, seven call
results, the native child delivery and every call argument were reviewed. The
explicitly selected apply skill owns orchestration; the naming child is already
one of the **13** dedicated namers above. Reproduction:
`tmp/observation-ingest/successor_apply_metrics.py` and `successor-apply-metrics.json`.
Full child transcript review remains pending; the embedded report is not that review.

| Unit | Calls | Seconds | Outcome |
| --- | ---: | ---: | --- |
| Parent preflight/launch, 1183–1193 | 5 | 65.522 | One selected row still blocked; one child launched |
| Parent verification/pause, 1194–1199 | 2 | 36.707 | Capability restriction read; 0/17 tasks checked |
| Whole parent attempt | 7 | 431.081 | Includes native completion wait, no polling loop |
| Child `b0e6c635-62c7-492d-b85e-301755947636` | 73 | 336.309 | Process complete/exit zero; blocked naming outcome, attested report |

Parent/child elapsed times overlap. The launch-result-to-notice span is 336.896
seconds; the previous parent reply-to-notice gap is 328.852. Parent recorded usage:
181,331 input + 2,381,952 cache-read + 16,120 output = **2,579,403 tokens**,
including 9,661 reasoning tokens. Child lifecycle metadata records 55 turns and
211,197 input/output tokens; those fields have a different cache convention and
are not a directly comparable billed-cost measure. Runtime status SHA-256:
`9ba68c4fdf28b9dbab7d2b778f388ad660a57e41f18959d11a238722360f88a7`.
Acceptance is default lightweight attestation, with no runtime checks or verify
runs; do not count it as an independently accepted proposal.

The parent selected `function:func_801E5988` in `emi/bmagic/magic004/03`, one
of task 1.1's 22 targets. Its collector returned exit 0 in **7.332141 seconds**,
with **0 planned/0 completed/10 skipped**; the selected row retained four open
rungs and no proposal. Correctly captured process success did not imply row
progress. A separate `--instructions` request omitted required target/report
arguments and printed usage. Discovery also searched a nonexistent old module
path with stderr suppressed, and a broad example search returned historical
Markdown reports instead of usable conclusion JSON. These are concrete reference
discovery failures, not proof that no supported examples existed.

The mission improved on the earlier campaign by selecting one target/row and
prohibiting identity edits. But its brief called the conclusion route established
and said not to re-derive it before capability admission was checked. It froze a
report path without a digest or complete collector command, and gave only an
unspecified mission-budget bound. The child reported an unfiltered collector run:
**9 completed/9 planned/10 skipped**, refreshing 30 evidence files for other rows
while the selected row stayed blocked. The parent had used an explicit selector.
This expansion needs reconciliation against the child transcript; a one-row brief
alone does not prove one-row execution.

The child reported a real import refusal, **exit 2: receipt lacks runner-produced
owner_resolution facts**, and full-report validation `complete:false`, 19 rows.
It retained full unchanged report/payload hashes and marked its conclusion input
unimported. Its account also acknowledges that `queued`/`slot` rely on an owner
annotation and layout shape, and no corroborator has a runner-derived source ID.
A capability restriction therefore coexists with incomplete semantic support;
“not an evidence gap” is too categorical. The inert apply request was not a ready
transaction. No positive rename or debt reduction was demonstrated.

The parent's follow-up directly displayed two important historical checks:
`_validated_capability` rejects non-data kinds and permits only exact allowlisted
`exhausted` data rows; `_reject_unsupported_semantic_facts` rejects unsupported
`proposed`/`exhausted` rows. These support a restriction on that inspected import
path. Its stronger claims need qualification:

- The batch issued a focused test-file run, but its search response omitted the
  `run-test` section entirely. The child attested three selected tests passed;
  neither proves the parent's full-file result. The displayed test bodies assert
  rejection, but the fixture and full call path were not shown in that parent read.
- Literal searches found `selected_call`/`owner_body` only in requirement tables
  and inventory descriptions. That is useful absence evidence, not complete
  producer/dataflow analysis. The displayed typed-rung read stopped at its
  function signature, and terminal-capability validation was not independently
  displayed. “Reproduced every gate” overstates the parent's verification scope.
- The parent inferred no rename could ever be prepared, including through
  `--candidate`, without inspecting that alternative's complete validation path.
  It also said all 14 binding drifts depend on these 61 rows despite earlier
  scope evidence to the contrary. The sampled import failure does not resolve
  every generator's cause or establish all required remedies.
- A partial baseline digest and the child's preservation attestation did not
  prove repository-wide absence of writes. Metadata reports unrelated matching
  files changed during the child window, so “only writer” was a briefing premise,
  not an observed global invariant. No new parent preservation comparison ran.

The historical memory ingest repeated the universal capability/drift claims.
Its receipt confirms storage, not independent corroboration; this audit does not
promote that payload into current authority. The proposed **66 missions** for task
1.1 and **150+** overall were extrapolations from a chosen three-stage workflow,
not measured required workload or a forecast supported by accepted transactions.

| Improvement destination | Measured baseline and proposed change | Acceptance measurement |
| --- | --- | --- |
| Naming directive / parent brief | One 73-call child disproved a presumed admission path. Check row-kind capability and allowed terminal state before scheduling semantic authoring; preserve unknowns in the brief. | Calls/time to actionable blocker; missions launched with verified prerequisites/N; accepted proposals and safe blocked returns reported separately. |
| Operation reference | One invalid instruction request, one obsolete-path search, no usable example, and differing parent/child row selectors. Document exact target/report/selector/digest handoffs and collection versus conclusion admission. | First valid invocation/N; selected versus actually processed rows; unsupported route claims and corrective discovery calls. |
| Harness/results | Parent collector summary exposed ten skipped rows for a one-row selection; verification search hid the test result. Expose per-row skip reasons, capability support and complete command exit/result receipts. | Every selected row has a explained outcome; executed checks retain their own result; no excerpt or missing section is reported as a pass. |

No capability bypass or weakening is proposed. Compare like missions; the
73-call child found a useful blocker, so its entire cost is not established waste.

### Origin of the naming observation ledger

Parent records **1200–1226**, same source/hash, fully cover the first naming
`references/OBSERVATIONS.md` creation. This is skill-maintenance evidence, not
another naming mission. The [rescope case](openspec-update-change.md#successor-capability-rescope-and-observation-ledger-creation)
owns the mixed **17-call/118.806-second** request. Its observation-authoring/check
phase, records 1216–1226, used **6 calls/51.119 seconds**; the earlier full skill
read and four memory calls are shared preparation. Do not count them twice.

The operator read the complete 49-line naming skill, the first 28 of 129 lines
of the lift-observations pattern, and two memory pages. It created a **58-line**
naming ledger and expanded the entry skill **49→51 lines** with a direct pointer.
The former cross-skill reference was an existing example, not a broken link;
the missing naming-specific ledger was the actual gap. Two file mutations
succeeded. One routed target returned exit 0 and **16,959 bytes**, containing the
new pointer and warning once each. That proves entry-body rendering, not reading
the new ledger, compliance with it, or improved naming outcomes. **Zero subsequent
missions** occur within this interval to measure the edit's effect.

The creation made lessons easier to locate, but promoted uncertain observations
into rules without a version or evidence boundary:

- It labeled the capability ceiling “measured on 61 rows” despite one selected
  import attempt and narrower code inspection. It instructed all future debt
  missions to return blocked rather than re-check changed prerequisites. The
  [first-row case](#successor-first-row-attempt-capability-diagnosis-before-scaling)
  preserves the actual sample, admission-path scope and remaining semantic gaps.
- It repeated “same work” for nine parallel timeouts versus five serial missions,
  treated all timeout duration as lock queueing, and called the failed premise
  the costliest failure without a comparable-cost ranking. The measured child
  in question took 336.309 seconds; “a 15-minute mission” described a budget, not
  its observed duration. No speedup or recoverable cost was established.
- The ownership table reduced function evidence to containment plus a spelling
  and data containment to a map entry. Neither alone establishes semantic role
  and full identity acceptance. The report-token workaround was presented as
  generally valid despite unresolved canonical routing from prior attempts.
- Historical memory was credited, but the two returned page bodies did not supply
  the asserted `--max-attempts` default of 20 or the quoted list about converting
  agent specifications into tooling. Their older ladder/serialization records
  also did not establish a controlled result for this naming workload. No fresh
  code or commit check closed those attribution and applicability gaps.

Check discovery exposed a separate workflow problem. `bin/harness docs refs`
returned **exit 2, unknown domain**, confirming that command was unavailable.
The operator had earlier retired it in the same parent history. Recipe/module
searches and that one failure did not establish that every Markdown/reference
check was absent. The resulting historical memory correction nevertheless said
there was no dedicated skill-Markdown gate and treated router loading as the
effective replacement. Router output cannot validate every link or semantic rule.
The new file's existence supports its one direct link; no full reference audit
or independent semantic review ran. `git diff --check` passed, but the newly
created untracked ledger was outside that check. A line count and compact-looking
draft are not evidence of a separate contract-preserving compaction review.

Proposed **skill/reference improvement**: every operational lesson carries source
revision, target/cohort denominator, confidence and a condition for revalidation.
Separate safe blocked outcomes from proven capability ceilings and measured cost
from hypothetical budgets. Acceptance: no universal policy from an unlabeled
sample, no causal speed claim from different scopes, and no current directive
derived solely from stale memory. Proposed **result/observation tooling**: attach
run identities, actual calls/duration, check coverage and follow-up outcomes to
each lesson; retain failed checks and untested effects. Compare recurrence and
accepted outcomes on equivalent missions before claiming the skill improved.

### Review handoff lessons and an incorrect attribution rule

Parent records **1264–1300**, same source/hash, continue the
[capability implementation case](openspec-apply-change.md#capability-implementation-supervision-and-rejected-canonical-proof).
Shared parent cost is **13 calls / 970.255 seconds**, including waiting gaps;
the observation edit/check phase alone, records 1293–1298, is **3 calls / 17.611
seconds / 1,274,661 recorded tokens**. Do not add either phase again across ledgers.
Two historical lessons were added in one edit. Both complete child reports were
read; full transcripts and later recovery remain pending.

One lesson had direct support: a reviewer with read/search/diff tools could not
personally execute `just check-unit naming`. The parent supplied separately run
results and the reviewer labeled them accordingly. Select a reviewer whose actual
tool contract supports the required check; an agent name or remembered tool list
does not prove capability. The specific replacement agent recommended by the parent
was not inspected in this interval. Do not solve an unsuitable assignment by
weakening its restrictions or calling supplied output personal execution.

The second lesson was wrong: **mtime does not isolate an increment's changes**.
The parent searched naming directories over 35- then 90-minute windows, found seven
Python files, and called that the complete task footprint. The worker's report also
listed two documentation edits outside those searched directories. Older timestamps,
zero recent identity paths and a partial baseline hash cannot establish unchanged
bytes; Git status omits ignored outputs, including gate writes the worker reported.
The reviewer correctly left identity preservation independently unverified without
a task-start snapshot. Timestamp searches are navigation aids; neither they nor a
HEAD-relative diff replace before/after comparisons against the actual dirty start.

The edit and its dependent check were issued together. The first check still read
**58 lines**; a second read confirmed both new bullets and **70 lines / 5,728 bytes**.
The parent called the first output stale, but the record does not establish why it
observed the old state. Await the mutation result before checking its postcondition.
Router exit 0 with 16,959 bytes proved rendering, not that either new lesson was
correct or consumed. No subsequent improved mission was measured in this phase.

Proposed **observation directive/reference**: require source-bounded claims and
an evidence review before promoting a lesson into operating guidance. Baseline:
two additions, one unsupported attribution rule; an extra read to establish the
edit landed. Acceptance: preservation claims use complete starting/ending evidence,
unknown attribution remains unknown, and post-edit checks consume the resulting
file. Proposed **handoff tooling**: retain task-start manifests and exact scoped
deltas, including untracked inputs and declared generated outputs. Measure missing
provenance, corrections and review-blocked time per comparable dirty-tree mission.

### Canonical-proof correction and preservation of older lessons

Parent records **1301–1327**, same source/hash, and the complete reviewer report
`400156fd-0384-48bb-96fa-f6e231349db5` provide the
[canonical-proof follow-up](openspec-apply-change.md#capability-implementation-canonical-proof-and-independent-reproduction).
Its shared parent cost is **11 calls / 1,994.965 seconds**; full child transcript
review remains pending. The reviewer reported independent canonical execution,
matching ordered operation sets, production-runner journal reuse, and rejection
of three perturbed operation sets. This is evidence for one straight-line wrapper,
not completion of imported discovery, branching coverage or function admission.

The parent added two useful historical lessons: canonical freshness requires the
actual operation agreement, and missing acceptance evidence differs from weakened
trust checks. However, its edit **replaced** the two-line “Report authority” bullet
instead of appending after it. The rule that selected-row success cannot replace
unfiltered full-report validation disappeared from that historical file. The
post-edit check counted the two new headings and 79 total lines, but never checked
preservation of the old obligation. That is one lost existing rule in one reviewed
lesson-edit transaction. This current ledger retains full-target authority elsewhere;
the historical mutation is evidence, not an instruction to repeat it.

Proposed **observation directive/reference**: compare old and new obligations when
adding lessons, including sample limits and report authority. Acceptance: every
removed rule has an explicit preserved destination or authorized retirement;
heading/line counts alone never establish preservation. Measure lost obligations
and repair revisions per reviewed edit, alongside independently reproduced task
outcomes. No routing/rendering or schema pass can validate lesson correctness.

### Canonical recipe promotion and edit ordering

Parent records **1328–1339**, same source/hash, are part of the
[owner-body launch](openspec-apply-change.md#capability-implementation-owner-body-launch-and-uncovered-prerequisites):
**7 shared calls / 31.400 seconds / 2,311,227 recorded tokens**. One task RESULT
edit succeeded, one observation edit failed on a removed anchor, and the workflow
still launched. Its brief told the worker to read the new recipe in the ledger
before that recipe existed there. Prior complete worker/reviewer reports were also
supplied, limiting the context loss; actual child read timing remains unreviewed.

One recovery read supplied the real tail and a replacement appended the two lessons.
The dependent check issued alongside that edit saw **79 lines / zero new lessons**;
one further read found **92 lines / 7,839 bytes**, both additions and 11 lesson
bullets. The parent again diagnosed a stale/cached view without establishing that
cause. Await the edit result before issuing a dependent check or launch. These
reads establish presence, not that the lesson improved a subsequent outcome.

The canonical recipe usefully requires a separate evidence root, operation equality,
freshness recomputation and native-output agreement. Keep its claims bounded:
equal regenerated bytes do not prove historical non-editing, and one inconsistent
probe does not prove every synthesized artifact fails that particular comparison.
The self-check lesson misattributes the premature tick to the producer writer;
the observed tick was task 1.6's proof worker. Its future brief correctly prohibits
self-checking, but that is changed guidance, not yet measured compliance. The
earlier full-report authority rule remains absent from the historical ledger.

Proposed **observation directive/tooling**: pin each lesson to the correct run and
criterion, read current anchors, await mutations and verify both additions and
retained obligations. Baseline: one failed edit, one recovery read and one extra
postcondition read in this phase. Acceptance: no dependent launch references an
unwritten required lesson, no unsupported generalization becomes a rule, and
comparison cohorts measure later compliance rather than heading or line growth.

### Supporting parent case: naming debt diagnosis

The fully reviewed [initial naming apply attempt](openspec-apply-change.md#naming-and-binding-debt-initial-attempt-through-pause)
is OpenSpec-owned supporting evidence, not an additional dedicated-namer run.
Parent `01a0dad5…` records 772–811 used **21 calls/348.407 observed seconds** and
ended at 4/12 checked tasks. Fourteen normalized maps reduced findings **89→75**;
repeat hashes established formatter idempotence. No naming transaction completed.
Its nine-call/100.145-second binding diagnosis phase included a 22.074-second
substitute-checker call that reported 165 drifts where the owning checker found
three: the substitute omitted shared and SDK maps. These costs belong to the
apply cohort and must not be added to namer totals.

Proposed naming-reference improvements: distinguish raw-spelling groups from
target-qualified identities, trace composed symbol ownership before choosing a
remedy, and distinguish aggregate status from each required live byte proof.
Sixty-one rows grouped into nine spellings did not establish nine reusable naming
decisions; 206 aggregate exact-status entries did not fulfill the per-target
identity requirement. A baseline-membership probe also counted 21 raw bindings
rather than the 14 drift findings, then overclaimed that all drift required
semantic naming first. Preserve open remedy alternatives until ownership evidence
discriminates them. Measure proxy disagreements, supported identity groups,
unnecessary re-probes and verified target outcomes; savings remain unmeasured.

### Nine-symbol campaign: dispatch and late corrections

Parent `01a0dad5-86b6-74b4-97f0-9382ab99ade4` records **911–939** were
fully reviewed, including launch/steering arguments and the compaction summary.
This continues the eighth OpenSpec apply case; these are parent orchestration
costs, not additional namer attempts. Reproduction:
`tmp/observation-ingest/naming_launch_metrics.py` and `naming-launch-metrics.json`;
parent SHA-256 `2c4e084cd00b25aa453b20018463e58fd9ed3fee5c4d7c524d863aa6d838d791`.

| Phase | Calls | Observed seconds | Outcome |
| --- | --- | --- | --- |
| Capability/snippet discovery, 911–914 | 2 | 12.349 | Partial naming entry read; cleaner and identity transaction contract not yet read |
| Launch and correction, 915–918 | 2 | 11.324 | First call rejected nine invalid child keys; second accepted the workflow |
| Contract discovery after launch, 919–926 | 4 | 20.697 | Correct `symbol` mode and transaction requirements found |
| Steering recovery, 927–938 | 10 | 19.413 | Workflow-parent route refused; nine child corrections queued |
| Whole dispatch, 911–938 | 18 | 121.426 | All call results present; no internal user-reply wait |

Phase spans exclude gaps between phases and do not sum to the whole span.
Recorded usage: **28,548 input + 8,554,240 cache-read + 21,769 output =
8,604,557 total tokens**; the 12,856 reasoning-token field is not additive.
Nine assistant reasoning blocks contain 52,869 characters; 29,308 occur in the
launch deliberation. Length alone does not measure wasted work. Twelve other
extractor matches are `model.thinking` settings, not reasoning text.

The workflow planned **9 evidence + 9 apply + 9 review** stages. Its terminal
status joins nine failed namers and one stopped cleaner; it does not establish
27 launched children. The first validation rejection launched none. The retry
changed colon-bearing keys and removed missing-output/status fallbacks; it was
not an identical retry. Workflow `f9f00993-28c0-4733-b5f3-758e2d1a5c24` and child
IDs are joined in the metric file to the existing cohort, avoiding double counts.

The launch exposed several distinct preparation failures:

- Each evidence mission discovered targets for one raw spelling, potentially
  spanning 22 targets, without a parent-frozen report/selector/fingerprint.
  Distinct report paths were treated as independent writes without checking
  shared lock scope. A shell-capable agent lacking an edit tool was described
  as read-only even though harness calls could write reports.
- Audit briefs sought closure of one row and preparation, while the entry
  contract required full-report closure and a separate transaction handoff.
  Apply stages lacked exact old/new requests and evidence, used incorrect
  `function`/`data` modes, and prohibited builds despite identity-gate needs.
  Placeholder apply/verify/rollback commands did not make those stages executable.
- Nine serialized apply stages preceded all reviews, with no conditional stop
  for blocked/failed evidence. Handoffs sliced evidence output to 6,000 characters,
  risking loss of blockers or the subsequently queued correction. Adoption
  criteria treated payload containment and a map spelling as sufficient identity
  evidence; neither alone establishes runtime role across consumers.
- All nine queued corrections repeated a shell-safe command example missing
  the old-symbol argument. Queue receipts establish delivery acceptance, not
  consumption or corrected future apply requests. No consumption acknowledgment
  appears by record 938; full child review remains pending.

Capability discovery also returned model settings different from the agent-file
frontmatter. Actual child metadata must settle the run configuration before a
model comparison. This case cannot establish a model effect or savings from a
different mission design.

The 8,248-character compaction summary at record 939 retained **0/1 workflow
ID and 0/9 evidence-child IDs**, omitted the live launch and queued corrections,
and ended at the earlier delegation-choice pause. That is a concrete recovery
gap, not proof that a duplicate launch occurred.

| Proposed destination/change | Baseline and acceptance measurement |
| --- | --- |
| Naming directive and mission examples: validate a frozen target/report request, lock footprint and exact next-stage grammar before dispatch | One launch required four late discovery and ten steering calls. Compare preflight-valid missions/N, correction calls and blocked prerequisites on comparable scopes; preserve full-report and identity gates |
| Orchestrator handoff: carry artifact identity and typed readiness, gate each apply on evidence and its independent review before the next transaction | Planned pipeline had no domain-status branch and truncated handoffs. Report launched applies with valid requests/N, rejected/blocked/no-op outcomes and review coverage; never equate transport success with domain acceptance |
| Recovery guidance/tooling: checkpoint live parent/child IDs, current stage, queued versus consumed control messages and required next action | This summary retained zero identifiers. On interrupted campaigns, measure recoverable checkpoints/N and duplicate or stale launches; queued correction must remain pending until observed consumption |

These are proposals, not changes to skills or installed extensions. Complete
child reasoning/tool results remain unreviewed for this case.

### First prerequisite escalation and parent waiver

The same parent's **940–973** continuation adds **18 calls/154.117 observed
seconds**, including an **89.396-second gap** between its launch report and the
first child notice. This is elapsed time, not 154 seconds of active parent work.
`tmp/observation-ingest/naming_supervision_metrics.py` reproduces the counts from
the same source hash. Three late memory calls took 12.158 seconds and returned
72,340 characters; the first supervision episode used 15 calls over 52.563 seconds,
including four calls/22.681 seconds to record and check the waiver. Recorded
usage: **158,366 input + 867,072 cache-read + 11,622 output = 1,037,060 total**;
5,053 reasoning tokens are not additional. Eleven reasoning blocks total 21,474
characters. These costs are distinct from the preceding dispatch prefix.

The memory pages are historical tool outputs, not current authority for this
ledger. They described isolated apply→review transactions; the parent called that
confirmation of its all-applies-before-reviews script without reconciling the
difference. Its report called both briefing defects handled and promised evidence
closure and renames, although corrected-request consumption and proposal readiness
were unverified. It correctly retained the separate full-target completion gate.

The `D_80146865` child (`e57732e5…`) stopped for a prerequisite decision at an
attested **42 tools/24 turns/105,869 tokens**. Its request reported one debt target,
`emi/scenario/scena02/00`, absent from a 23-target campaign account; router prefill
and reconciliation refused, while the suggested full initialization refused to
replace retained battle history. These are child-reported facts until its complete
transcript is reconciled. Repeated notification envelopes represent **one** request,
not three blockers. Parent reply acknowledgment arrived **16.456 seconds after
the first notice**, or 13.431 seconds after the detailed request; actual child
resumption time is not established by either measurement.

The parent preserved retained history, prohibited alternate retries on refusal,
and corrected its earlier implication that one successful initializer proved
campaign-wide readiness. It then declared a disposable report path canonical and
recorded that waiver as Decision 7. That declaration did not demonstrate router
acceptance or repair the missing account entry. The reply supplied one exact path;
the eight sibling steers supplied a `<targetslug>` template while claiming the
parent had frozen the path. All eight receipts were **queued**, with consumption
still unverified in this prefix. A parent decision recorded in a design document
does not itself establish equivalence to the owning evidence contract.

OpenSpec validation explicitly reported the change valid; whitespace checking
reported clean. The printed validation exit value followed a subshell pipeline,
so it did not independently preserve the `openspec` process status. Neither check
proved that the waiver worked or that a naming transaction was ready.

Proposed reference/tool improvements: expose campaign membership and retained-set
repairability before evidence dispatch, with an owner-supported non-destructive
repair or an explicit blocked result. Compare calls before actionable prerequisite
diagnosis against this 42-call child notice; retain blocked outcomes and never
improve the rate by bypassing canonical binding. In parent supervision, distinguish
exact frozen requests from path templates and verify queued control consumption.
Measure exact-request coverage, successful canonical prefills and repeat escalations
per mission. No saving or successful waiver is established by this episode.

### Contention supervision, timeouts and stopped workflow

Parent records **974–1035** complete the first campaign's supervision through its
stop and recovery-choice pause. All assistant text/reasoning, tool outputs,
notifications and control/edit arguments were reviewed. Reproduction:
`tmp/observation-ingest/naming_timeout_metrics.py` and `naming-timeout-metrics.json`,
using the same parent source hash. These are **19 parent calls**, distinct from
the preceding 36 calls and the already counted nine namers' 741 calls.

| Phase | Calls | Observed seconds | Established result |
| --- | --- | --- | --- |
| First long-call notice, 974–983 | 5 | 30.079 | Status/process inspection; three stop steers queued |
| Lock diagnosis and shell stops, 984–999 | 6 | 57.508 | Three shell PIDs signaled across two calls; design risk recorded |
| Near-deadline report count, 1000–1007 | 3 | 18.652 | 26 matching filenames, no evidence-row validation |
| Timeouts, stop and closeout, 1008–1035 | 5 | 76.082 | Nine explicit timeouts; workflow stopped after one apply child started |
| Whole supervision prefix | 19 | 530.299 | Includes gaps while children ran; not active parent execution time |

Recorded usage is **50,990 input + 1,916,416 cache-read + 23,793 output =
1,991,199 total tokens**; 14,226 reasoning tokens are not additive. Sixteen
reasoning blocks contain 57,774 characters. Three between-episode gaps total
347.978 seconds, including a 320.499-second interval before the last long-call
notice; phase spans exclude these gaps. Four distinct 240-second-open-tool notices
appear in repeated delivery envelopes, not twelve independent stalls.

The record supports lock contention, but not the parent's claim that all child
time was spent waiting. Process listings showed three retry-loop forms (limits
40/60, not measured iteration counts), a lock poller and a naming process holding
the lock; several final previews reported foreign-lock waits. No per-child wait
duration or attributable timeout fraction was measured. Earlier speculation about
per-operation deadlines was not demonstrated. Status counters established some
delivered messages and others still pending; **all nine children later received
at least one failed-follow-up notice**. Notices lack request IDs, so they cannot
yield a message-level failure rate for the 20 queued child steers across the
campaign. The earlier claim that three loops were already stopped preceded that
evidence. Process disappearance also cannot prove which steer caused a stop.

The stop calls signaled shell PIDs 4033456, 4034043 and 4030428; PID 4045201 was
already absent on the second call. A subshell showing its parent's command line
does not prove a nested retry loop. Nearby PID numbers do not establish run
ownership. The first signaled child's supposedly healthy disassembly was assigned
from a process listing without a PID-to-run join; its listed target was scena02,
the other child's target. Surviving workers, filtered `pgrep` snapshots and one
momentarily free lock do not establish intact checkpoints, complete descendant
termination, or that contention has permanently ended. The final workflow stop
receipt, unlike its initial acknowledgment, does establish terminal workflow state.
It arrived **17.189 seconds after the stop call**; that call followed the first
explicit timeout notice by **8.278 seconds**. No 2.5-hour saving was measured.

Report counts **8→26** and 21 recently modified files establish artifact activity,
not completed evidence or successful canonical-route repair. Per-symbol target
denominators in the final progress table sum to **70**, inconsistent with the
claimed 61-row scope; filenames were never reconciled to a frozen row set. Four
claimed full-coverage groups are therefore not accepted coverage measurements.
Likewise, global `source symbols check` failed on binding drift, while a target
check passed. That proves differing scope, not that TARGET is a mandatory argument.
Two identical global invocations in one call and three in the later diagnosis
repeated the failure; none made the checker a dependable target enumerator.

The closing “no damage” claim exceeded its evidence: both compared dirty-status
histograms were taken after the campaign, omitted file contents, and counted
untracked entries too. The baseline comparison printed only 24 hash characters.
Eight visible child recovery previews attested no tracked changes; the ninth was
clipped by the notice budget, and untracked/generated state was not covered.
The final process filter and retained-file counts did not repair that preservation
gap. Schema/whitespace checks did pass for the recorded design risk, but did not
validate recovery or naming outcomes. The one stopped cleaner's identity is
available in runtime metadata despite `unavailable` in the notification.

| Proposed destination/change | Acceptance measurement |
| --- | --- |
| Naming/operator reference: distinguish initialized reports, committed operations, terminal rows and validated target coverage; hand workers a frozen target list | Reconcile every reported numerator/denominator to retained row identities; no filename/mtime-based evidence-closure or checker-error-as-empty-inventory claim |
| Harness/mission telemetry: expose run-owned processes, actual lock wait, absolute budget remaining and retained checkpoint state | Compare wait seconds, retries, useful committed operations and timeout rate on equal scopes; retained stop evidence covers descendants and checkpoint integrity |
| Parent control: branch on failed evidence before apply; retain queued/delivered/consumed/failed controls separately | Baseline: nine timed-out evidence children and one unnecessary apply launch. Failed evidence launches zero applies; mandatory corrections are consumed or the dependent stage stays blocked |
| Recovery reference: compare exact adopted PRE/POST paths, modes and full hashes; validate retained evidence before reuse | Preservation claims cover tracked, untracked and generated task scope; a stopped receipt or equal status histogram never substitutes for byte-level preservation |

No evidence-gathering or source transaction was replayed by this audit. Full child
transcript review and the subsequent pilot continuation remain pending.

## Naming-debt admission versus repair

Older parent `01a0c062…`, records **446–461**, used **eight shell calls/35.486
observed seconds** to inspect and admit `exe/logo:D_801EB470` to
`config/symbol-naming-baseline.json`. The baseline contained **833 raw-data rows**;
a verified one-line addition made the symbols gate report **rc=0**. This is
supporting parent work, not a naming-skill selection, semantic rename or accepted
identity transaction. Source pin and joins:
`tmp/observation-ingest/older-baseline-bitfield-metrics.json`.

The inspected checker computed current debt minus admitted baseline rows. Adding
the failing row therefore changed the accepted set; it did not resolve the raw
symbol or prove the gate's previous boundary was preserved. No new authorization,
symbol evidence or baseline-maintenance policy decision appears in this interval.
The earlier clean-worktree comparison had not run this symbols gate. Describing
the result as a repaired preexisting baseline gap overstates that evidence.

Two distinct recoveries matter: extracting a path with a regex that assumed a
string assignment failed on `Path(...)`; then a four-space JSON dump rewrote
nearly the whole file despite a one-line line-count delta. That candidate stayed
in `/tmp`. The two-space candidate showed the actual single-line diff before
copying. Its `diff` exit 1 meant expected differences, not a failed repair.

Proposed naming/reference guidance: distinguish semantic resolution, justified
baseline maintenance and a newly admitted exception in outcome metrics. Proposed
reporting: expose admitted-set changes beside gate status and retain exact diffs,
not just line-count changes. Acceptance requires evidence and authorization for
any changed admission boundary; otherwise report the unresolved debt. Measure
resolved raw symbols separately from baseline rows added and calls/time to each
outcome. This audit does not alter the baseline or endorse its historical change.

## Campaign baseline admission and parser recovery

Older parent `01a0c062`, records **1091–1100**, spent **five shell calls / 40.372
observed seconds** on the five-lift batch's naming gate. This is supporting
campaign work, not an explicitly selected naming mission or an identity
transaction. `tmp/observation-ingest/older-batch-verification-metrics.json`
contains exact source and call/result joins.

The repository-wide check reported **17 new rows**: five raw function files,
four raw function symbols and eight raw data symbols, all game00. The parent
added them to `config/symbol-naming-baseline.json` while calling the debt closed.
No spelling changed. It asserted baseline maintenance was sanctioned but cited
no owning admission rule or independent decision in this interval; the need for
a green full check does not itself prove authority to expand an accepted set.
Retain this as 17 admitted rows and zero semantic resolutions, with the admission
decision's justification still to be verified against the historical contract.

Parsing human-readable errors with `\S+` captured a trailing semicolon in **16/17
rows**; only the final `D_801CD224` row matched. Counts grew, yet the gate stayed
rc=2 with 16 rows. Membership inspection and the actual diff exposed the cause;
speculation about caches, write flushing and alternate paths added no evidence.
The diff had 18 added lines because it also contained the previously admitted
`exe/logo:D_801EB470`, not an eighteenth row from this batch.

Recovery stripped trailing semicolons **and slashes across every category**, then
merged rows parsed by a delimiter-aware regex. The gate returned rc=0 **29.982
seconds after the failed result** (31.752 seconds from the faulty edit call).
That broad cleanup was not limited to the 16 malformed additions; no final exact
diff demonstrated that unrelated baseline entries stayed unchanged. The replacement
regex also retained trailing whitespace, which the cleanup did not strip; extra
malformed entries were not excluded by the green gate. Gate success
alone cannot establish scope preservation or semantic naming improvement.

Proposed **naming directive/reference**: distinguish admission from resolution,
retain the evidence and authority for accepted-debt changes, and inspect exact
before/after membership. Proposed **harness support**: expose structured debt rows
with category/identity fields and validate a scoped proposed addition set, rather
than parsing display punctuation. Measure malformed rows/N, recovery calls/time,
unrelated admission changes and resolved names separately; a green gate obtained
by admission must keep the unresolved naming work visible.

### Repeated newline defect in the next admission

Parent `01a0c062`, records **1121–1128**, confirms the earlier cleanup left an
extra `emi/etc/game/00:D_80181BD4\n` row: independent review flagged it, then
the parent's read at 1123 displayed both the clean and newline-suffixed entries.
The prior green gate therefore demonstrably did not validate baseline row shape.
This supersedes the preceding observation's uncertainty about extra malformed rows.

The next repair normalized existing rows, but parsed four new battle15 debt rows
with the same newline-permitting regex and omitted newline stripping on additions.
Three became effective and the final `D_801463B8` remained reported. It also
invoked `symbols check` twice merely to concatenate stdout from one process with
stderr from another. A third check showed rc=2. A newline-excluding regex plus
whitespace stripping then reached rc=0 **11.720 seconds after that failed result**;
one loop repair was followed by two green checks. The shared repair phase used
four parent calls/35.309 seconds, including a source comment edit, so it is not
exclusive naming cost. Pins and joins are in
`tmp/observation-ingest/older-battle-partial-recovery-metrics.json`.

Another output printed rc=0 after `symbols check | head`; it was the pipeline
tail's exit, which the parent later recognized, not the failing check's result.
Four rows were admitted and no names resolved. Global whitespace normalization
again lacked an exact scoped final diff. Proposed **tooling/reference** improvement:
consume one structured result, validate whitespace/category/identity before
mutation, capture the owning command's exit and reject unrelated baseline changes.
Measure recurring malformed-row incidents and unnecessary checker invocations,
not just whether the next gate is green.

The next admission at parent **1157–1158** reused the newline-excluding regex
and whitespace stripping; the displayed final `symbols check` was green before
snapshot/index refresh and the full recipe. It emitted neither added-row count
nor scoped baseline diff, so this supports successful final checking, not a
measured number of legitimate admissions or resolved names. The loop also did
not raise on four exhausted iterations; later commands would still run. Preserve
the prior parser repair while making failure propagation and exact admission
diffs explicit. Source join: `older-battle-second-partial-metrics.json` under
`tmp/observation-ingest/`.

Parent **1228–1230** makes the gate distinction explicit: target-specific
`symbols check battle/15` passed while the repo-wide check returned **exit 2**
for three entry-5 rows (file, function and `D_80148624`). Review simulated exactly
those three additions and found no remaining debt regression. The parent then
used the generic admission loop and global whitespace normalization; its final
gate passed, but no exact baseline diff was emitted. This is three identified
admissions, not three resolved names or proof that no unrelated baseline row
changed. Review recommended a scoped repair; the parent supplied broader mutation
machinery and did not show the requested post-admission native recheck. Proposed
**tooling/reference** improvement: distinguish local map validity from global debt
policy and apply an explicitly enumerated admission set. Pins:
`tmp/observation-ingest/older-battle-batch-completion-metrics.json`.

## Baseline recorder: recovery and review boundary

Older parent `01a0c062`, records **1407–1433**, investigated the user's request
to fix naming-row failures. The correction used **14 parent calls/153.997 observed
seconds**: seven preparation reads, two implementation edits, one command exercise,
one broader test call, two directive edits and one final check batch. This is
supporting naming-harness work, not another dedicated namer or identity transaction.
All retained parent blocks/arguments were reviewed; source pin and call joins:
`tmp/observation-ingest/older-naming-recorder-metrics.json`. The enclosing slice
through 1442 includes lift review/dispatch waits and must not be charged entirely
to this correction.

The global gate correctly returned **rc=2 for four new rows**: one raw filename,
one raw function and two data names from the latest lift. Its current-minus-baseline
comparison was behaving as implemented. The first diagnostic nevertheless called
**all 1,770 existing baseline rows malformed** because its forbidden-character
list included `/`, normal in paths and target names. A category-aware rerun found
no violations of its tested patterns, duplicates or whitespace/semicolon checks;
all four categories were sorted. The false diagnosis was corrected **7.719 seconds**
after its result. That supports recovery from a bad heuristic, not a comprehensive
semantic/schema-validity proof or a defect in the gate itself.

The parent found no baseline-write subcommand, then added `merge_naming_baseline`
in `harness/naming/debt.py` and a `symbols baseline [--write]` handler. The observed
command sequence was **dry-run rc=1/four additions → write rc=0/four recorded rows
→ global check rc=0 → dry-run rc=0/up to date**. Current raw spellings were retained:
**four admissions, zero semantic resolutions**. The first edit was requested 51.109
seconds after the user request; the admissions returned at 59.577 seconds. This
replaced error-message parsing with direct structured debt collection, addressing
the earlier semicolon/newline extraction hazards. It does not measure a speedup
against a comparable reviewed admission task.

**Implementation and workflow did not bind admission to review.** The handler
collected debt from every manifest and merged every new row; it accepted no selected
row set or review receipt. The owner document instructed use **after an accepted
transaction**, while the RE skill instructed writers to run it **before handoff**.
The next writer brief repeated the latter instruction without adding the baseline
file to its explicit path scope. In this episode the four rows were admitted
**457.729 seconds before independent lift-review delivery**. The later reviewer
accepted the lift, but did not independently review the new recorder or establish
that global admission was limited to approved changes. Global collection creates
an unrelated-row admission risk; no unrelated admission was demonstrated here.

The merge preserved existing keys/rows as sets and sorted them; it wrote the file
directly. The subsequent dry-run established no pending rows, not crash-safe writes,
concurrent-update preservation or a complete no-mutation/idempotence check. Existing
naming/commands/build tests reported **891 passed/two skipped in 53.19 seconds**;
Ruff passed. After the directive edits, the scoped recipe returned **rc=0/9 seconds,
940 valid/zero invalid**, and skills tests reported **42 passed in 12.85 seconds**.
No new tests were added. These green existing suites and the live four-row exercise
support those results; they do not supply missing review-binding or isolation
coverage. Pytest output was piped through `tail`, without a separate pytest exit
receipt. The nineteen-call enclosing slice had zero tool error flags despite its
initial failed gate, another reason not to equate flags with domain outcomes.

| Improvement destination | Concrete proposal | Acceptance measurement |
| --- | --- | --- |
| Naming/RE directives | Assign accepted-debt admission to one owner after review and carry its exact path scope; reconcile the before-handoff and after-acceptance instructions | Baseline: one ordering conflict and four pre-review admissions. Every admitted row has an identified reviewed transaction or explicit admission decision; zero semantic resolutions inferred from gate green. |
| Naming references | Distinguish malformed rows, correctly detected new debt and missing workflow tooling; document category-aware diagnostics and dry-run status meanings | Baseline: 1,770 heuristic false positives and a 7.719-second correction interval. On comparable reviewed inputs, report false positives, missed invalid rows and diagnostic recovery cost with labeled denominators. |
| Harness tooling | Preview and apply an explicit row set bound to the baseline revision and review evidence, preserving unrelated rows and reporting exact additions | Baseline: one successful four-row global merge, with isolation/concurrency unmeasured. All intended rows admitted, zero unrelated additions, stale inputs refused, and failed writes preserve the baseline; retain an independently failing check for later unreviewed debt. |

These are proposals for stronger evidence and scoped operation. The user's fix
request and the successful command exercise do not make fewer red checks equivalent
to less naming debt, nor establish that the documented review ordering was followed.

## Converged directives

### A naming mission (`bof3-namer`, `$bof3-naming`)

1. **Bound scope using the actual mission contract.** The historical `audit-target TARGET`
   contract selects one target. The proposed ~70-call/~5-minute limits were unvalidated
   heuristics; lifecycle duration did not measure command time. Calibrate bounds against
   accepted task units and retained partial results, preserving required evidence and review.
2. **Serialize writers.** Never run two report-writing missions concurrently; a held lock is a
   stop, not a reason to retry.
3. **Derive targets from the tool, not from memory.** Expect the router to refuse targets absent
   from `out/reviews/plan-audit-naming/summary.json`; the parent pins the per-mission report
   token instead. Never run `init-all` or modify retained campaign history.
4. **Refuse before inventing.** A row the evidence does not support is left blocked with its
   smallest repair and one bounded next command.
5. **Check a consequential premise with the smallest appropriate probe before delegating.**
   Record discovery cost and avoided or corrective work. The historical claim that this
   was the largest source of waste had no attributable-cost ranking.

### An independent review (`bof3-reviewer`)

6. **Match the reviewer to the required checks.** Verify its actual tool contract before
   dispatch. A read-only reviewer cannot personally execute a command gate; label supplied
   execution evidence and leave that requirement open where personal execution is required.
7. **Supply the task-only scope** using a comparison against the actual starting dirty state,
   with protected content/index evidence. Mtime is only a discovery hint, and a HEAD-relative
   diff cannot attribute the increment. Without a starting snapshot, preserve that limitation.
8. **Keep "the gate was weakened" separate from "the acceptance evidence is insufficient".**
   That distinction is what makes a BLOCK verdict useful instead of merely obstructive.
9. **Never accept a self-checked box.** A writer that marks its own task complete while its
   residual list says review is outstanding has not finished; hold the box provisional.

### Evidence rules

10. **Ownership evidence is per-kind.** Functions: `analysis query --json owners` returning
    `payload_contained: 1` under `provenance: reviewed_range`. Data: same-target containment in
    the map. An **empty** `owners` result is not evidence, and a spelling carried only by a
    *different* target is a lead needing a shared map or recursive proof of address, content
    class and runtime role.
11. **A probe is not a canonical run.** A synthesized artifact can carry a fact, a receipt digest
    and `replay: true` while failing journal freshness. Require payload, checkpoint and
    report-derived operation plans to AGREE, and never weaken that comparison to obtain it.
12. **Measure before naming a layer.** `exit=124` is `timeout`'s own code: something was killed
    at a deadline, not necessarily the native command. Compare the command's standalone wall time
    and byte count against the harness run before blaming the read path, the deadline or the
    decoder — and do not build a workaround for a problem the measurement has not confirmed.
13. **A config probe does not transfer across invocation modes.** A standalone
    `rizin -N -q -c 'e scr.interactive'` reads `false`, while inside Rizin's live stdin command
    loop the same variable is `true` — the mode the harness uses. Reproduce in the mode that
    matters, and accept that both sides of a disagreement can be individually correct.

## Naming, origins and binding rules (absorbed from the legacy `LESSONS.md`)

Cherry-picked from `LESSONS.md` Level 5. These are directive material — fold them into
`.pi/skills/bof3-naming/` (or its references) rather than restating them here long-term.

- **Replace analyzer aliases only after behaviour and signature are proven.** A raw `func_*`/`D_*` goes
  straight to the reviewed semantic name with no compatibility alias.
- **Function names**: verb-led camelCase, role-first; no target prefix except to break a collision.
  Never prefix a raw name with an overlay name (`SCENA16_D_*`) — resolve a collision with a different
  name or a suffix (`D_80146864_BYTE`).
- **Data names**: camelCase plus a role suffix (`...Table`/`...Strings`/`...State`), with
  `@source`/`@kind table|rodata|bss|data` tags; `@kind unknown` until independent evidence exists, and
  preserve proven kinds during spelling-only transactions. Use `/* */` only — `//` breaks gcc-2.6.3.
- **Origins**: every non-address-named map symbol needs one `@source`-tagged definition — a lift file,
  a header/source declaration, or `WEAK_SYMBOL_AT`. `bin/harness source symbols check` enforces
  raw-prefix spelling and origin tracking, not `@kind` evidence or semantic acceptance.
- **Promotion evidence**: keep pre-promotion evidence in an `INFERRED:` comment beside the owning
  metadata-tagged declaration (what was observed, what would verify promotion). Never create a
  semantic alias from a hint alone.
- **Bindings**: every `WEAK_SYMBOL_AT` in a hand-maintained claimed `src/bof3/support/*_symbols.c`
  needs a target-map entry; a different name at a mapped address is a deliberate typed alias (a `u8`
  view of a `u16` global, for example). `check` flags bindings whose address no map owns.
- **Shared-map `D_*`**: a `config/targets/shared/symbols.txt` entry claims data at that VRAM in EVERY
  target; keep it in the owning target's local map unless it is data everywhere, or bogus
  contains-data functions appear.
- **SDK maps and Splat**: check `config/sdk/psyq-*.txt` before adding to a target map — Splat composes
  both and a duplicate name aborts `bin/harness source splat`, so exactly one map owns a symbol;
  record the verified archive member in the manifest's `[psyq.libraries]`. Splat regenerates root
  stubs keyed by the **boundary name**, never the authored `@source` basename: after a collision
  rename, stub projection must look for `source_dir/<boundary-name>.c` or regenerated stubs are left
  behind.
- **Cross-target equality is insufficient**: overlays and PsyQ copies can share a role with different
  addresses or different bytes.

## Pending coverage (needs operator authorization)

Coverage repairs proposed by the independent review of the handshake and producer,
and **not** implemented in that historical episode: project
instructions forbid adding regression coverage without an explicit request, so these wait on the
operator even though the reviewer framed them as authorized variants rather than new tests.

1. **Close the fail-closed coverage gap in existing fixtures.** Answer the `e scr.interactive`
   read-back with `true`, and error the set-leg, asserting
   `pytest.raises(ValueError, match="Rizin refused verified noninteractive capture")` **and** that the
   fake never received the evidence command. At review time the fake engines answered only `false`.
2. **Exercise the exact-selector cross-target boundary.** With a frozen payload, assert admission for
   a *planned* owner selector and refusal (`"instruction capture is not a planned owner selector"`)
   for an unplanned one — the only authorized substitute for the categorical guard that task 1.7
   narrowed, and unproven by tests at that boundary.

The review also found no references under `tools/python/tests` to `replay_instructions`,
`owner_instruction_selectors`, `decode_owner_body` or `owner_body` at all.

## Bundled harness improvements

Each entry is an execution directive, not an observation. `[DONE]` means implemented and
verified; the rest are the outstanding change list.

| # | Improvement | Execution directive | Acceptance |
| --- | --- | --- | --- |
| H1 | **Live interactivity handshake** `[DONE]` | In `RizinSession`, after the command loop is entered and before any framed command, set **and read back** `scr.interactive=false`; fail closed if it is not `false` | Shop capture 221,901 B at sha256 `9211ed05eb6428914b5dc52d4aa0188303b4671e3b9dcf7c3c1e8456546aecbe`; battle control byte-identical |
| H2 | **Classify the shop owner's reviewed range boundaries** | Establish the code/data split of `T_801E5144` (`0x801E5144..0x801E626C`, 4,392 B) from payload bytes, calls and return paths per [boundary evidence](../../docs/agents/boundary-evidence.md). **Do not** shorten the range on the analyser's 164-byte size — it is not established as a valid replacement extent, and the earlier "bound to the analysed extent" direction is withdrawn | Either a corrected reviewed boundary carrying its own evidence, or an explicit record that the range legitimately spans code and data; `validate_instructions` unchanged |
| H3 | **Serialize report writers** | One writer at a time; a held lock is a stop signal | No `retry.sh`/`seq 1 60` loop in any run |
| H4 | **Calibrate mission bounds** | Split target scope according to the mission contract and retain partial results; evaluate proposed limits on comparable tasks | Report timeout rate alongside accepted units, required-check coverage, retries and recovery cost; early partial exits alone cannot establish improvement |
| H5 | **Stream isolation** (residual) | One-shot `-c` execution per capture, or a stream the child provably cannot read | No command can consume the framed command stream |

## Historical capability ceiling

The [first-row attempt](#successor-first-row-attempt-capability-diagnosis-before-scaling)
found a restrictive terminal-capability check and a blocked import in the historical
revision. Later `apply-evidenced-symbol-renames` Decision 8 and
`enable-function-naming-conclusions` refer to this prerequisite. Those historical
claims do not establish current capability state, every preparation route, or that
H2 alone would enable proposals. Revalidate the owning implementation and exact
row before scheduling work; retain the evidence limits above.

**Corpus note (bounded child report):** the owner-body worker reported no eligible
owner work among reviewed, indexed 32–128-byte functions overlapping raw symbols
outside the payload; the wrappers found were target-local. The parent received no
candidate count or full query report. This does not prove that no supported wrapper
owner exists anywhere in the corpus. The motivating 176-byte store body remained
unsupported, so wrapper-only proof could not establish that case's acceptance.

## Owner-body scouting and timeout diagnosis

Parent records 1364–1398 continue worker `0221b1af-d16d-4be5-a762-ca277cc25588`.
The [apply ledger](openspec-apply-change.md#capability-implementation-owner-body-corrections-and-timeout-diagnosis)
owns the shared 15-call, 684.304-second parent cost; do not count it again as a
separate naming mission. Four supervisor decisions produced three observation
edits, growing the historical ledger from 92 to 106 lines. No accepted owner-body
proof resulted within this prefix. Line growth and the parent's “four for four”
claim are not improvement rates: the selected discoveries overlap and lack an
eligible-check denominator.

The scout's bounded zero was promoted into an unsupported corpus-wide conclusion.
A second notice corrected the terminal sequence from `jr ra/nop` to a store in
the return delay slot. Record the exact motivating bytes and query population
before generalizing coverage; parent approval based on a worker report is not an
independent byte inspection. Keep this later owner-body finding separate from the
earlier `selected_call` producer's incomplete caller-discovery result.

The historical observation “a timed-out capture is usually a duration problem”
was written without a frequency denominator or direct timing. The proposed
`os.read` replacement failed to repair `pdj 1098`; that rejects this attempted fix,
but establishes neither the timeout layer nor its cause. D12's narrower diagnostic
rule governs this ledger: compare equivalent invocation settings, deadlines,
wall time and output bytes before assigning a cause. Here 4,392 means original
code bytes, not JSON output size. The requested rollback and standalone timing
remain unverified at the review boundary.

Proposed **directive/reference**: every promoted lesson carries population,
source role, observation versus hypothesis, and acceptance state. Baseline: one
overgeneralized scout and two successive unsupported causal diagnoses in this
episode. Accept the improvement when comparable supervised missions preserve
those distinctions through task and ledger edits. Proposed **capture tooling**:
report timeout stage, effective deadline, invocation mode, elapsed time and
captured bytes together; assess diagnostic retries and time to a reproduced cause,
retaining refusal on incomplete evidence. No speedup is measured here.

## Capture diagnosis lessons and recovery cost

Parent records **1399–1455** add **29 calls / 496.850 seconds** of shared apply
orchestration, not another naming mission. The
[apply ledger](openspec-apply-change.md#capability-implementation-capture-diagnosis-timeout-and-recovery)
owns the detailed denominator and phase costs. The owner-body worker timed out at
its 1,800-second deadline (51 calls / 1,805.014 lifecycle seconds); review never
launched. A resumed diagnosis is another attempt on the same unresolved task.

Five observation-edit calls included one failed anchor and four successful edits;
the historical ledger grew **106→119 lines**, twelve→thirteen lesson bullets.
Neither count establishes improvement. After the earlier duration hypothesis,
the parent again declared buffering proven from a 2.09-second standalone capture.
The worker's subsequent trace instead showed an interactive prompt consuming the
command stream before the payload was written. The parent's added requirement to
close stdin conflicted with that stream's transport role and had to be withdrawn.
Its replacement claim that a standalone false setting proved a live-loop no-op
was also unsupported. D13 preserves the necessary mode distinction, corroborated
by the later diagnosis already summarized in the Rizin ledger.

Correction must replace dependent claims, not merely add another lesson. The
historical ledger temporarily retained both a read-path explanation and the trace
that contradicted it; the withdrawn stdin instruction also survived until a later
repair. Current D12/D13 carry the bounded diagnostic rules. A count of “six” or
“seven” corrected assumptions was a retrospective narrative, not independent
checks with a known eligible denominator or measured prevented work.

Proposed **lesson-edit directive/reference**: preserve observation, hypothesis and
verified cause as distinct states; revise every dependent instruction after a
correction. Baseline: repeated causal promotion and stale instructions in one
supervised incident. Acceptance: source-backed lessons with no contradictory active
remedy after an update. Proposed **orchestration tooling**: serialize dependent
edits and reads, expose full diagnostics and link recovery attempts. Measure repair
calls, invalid comparisons and time to an independently reproduced result; retain
failed attempts and required verification in both numerator and denominator.

## Handshake success did not authorize a smaller owner range

Parent records **1456–1508** add **23 calls / 491.850 seconds** shared with
[apply supervision](openspec-apply-change.md#capability-implementation-handshake-supervision-and-premature-range-conclusion).
The live-mode diagnosis led to an implementation resume, two fixture decisions
and one stop/diagnose decision. The canonical run remained refused; the worker
was still running gates at this prefix's end. One observation edit grew the old
ledger **119→127 lines**, adding the mode-transfer lesson without replacing all
earlier contradictory claims. Growth is not a measured improvement rate.

Two controls reportedly recovered identical bytes, but the parent initially
hashed only the standalone shop control and later listed the battle pair. Those
checks did not independently reproduce both comparisons. Likewise, unchanged
staged-file counts did not settle the worker's reported raw-index drift. Preserve
these evidence limits even when the eventual independent review supports the fix.

The next proposed remedy was not supported: analyzer size **164** and reviewed
size **4,392** disagree, but invalid output beginning at the smaller endpoint does
not authorize shortening the reviewed range. The parent also converted its end
incorrectly: **`0x801E5144 + 4392 = 0x801E626C`**, not `0x801E52AC`. It read only
part of the diagnostic, then wrote the shortening instruction into task 1.11.
Keep the transport fix, instruction-validation refusal and boundary investigation
as distinct outcomes. Neither a `D_` spelling nor `contains_data: 0` settles the
original-byte classification or overrides reviewed ownership.

Proposed **handoff reference/directive**: name each evidence authority, compare
complete artifacts and calculate boundaries mechanically before prescribing a
range change. Baseline: one premature boundary remedy plus overstated independent
checks in this episode. Acceptance: every extent change retains qualified proof,
every comparison states its actual inputs, and unresolved canonical proof remains
open. Measure corrected handoffs and accepted task units per comparable attempt;
do not count a transport recovery as a naming conclusion.

## Metric promotion and observation migration

Parent records **1509–1537**, at the same pinned source hash used above, contain
**17 calls / 203.302 observed seconds**: migration and policy promotion use
11 calls/91.222 s; writer-return handling and review dispatch use 6/57.833 s.
The span includes a 54.247 s gap and is not command runtime. Recorded usage is
6,077,407 tokens, including 5,790,720 cache-read and 27,287 output tokens;
15,474 reasoning tokens are included, not added again. Ten reasoning blocks
contain 59,836 characters. These are parent costs, shared with the
[apply case](openspec-apply-change.md#capability-implementation-return-and-review-handoff),
not additional child missions or accepted naming units.

**The aggregate did not support the promoted limits.** A recursive scan of 120
runtime directories selected first-found fields, mixed workflow parents with
children and called duration ≥890 seconds a timeout. It classified completed
1,157–1,200-second workers as timeouts, despite their 1,800-second deadline, and
reported 15 namers instead of the reconciled 13 unique children. It omitted
field-availability counts and accepted outcomes. The resulting “10/15 killed”
claim entered both skill and agent text; the ~70-call/~5-minute cap also mixed
lifecycle duration with command time. Failed namers included 56-, 62- and 68-call
runs, while a completed audit used 73 calls/336 seconds. This supplies no tested
cutoff or causal estimate of savings. The pilot and canonical-proof rows above
correct further role/task mixtures in the five historical iteration rows.

**Migration did not prove convergence.** Four write/edit calls created a 109-line
external ledger, registered it and changed skill/agent text; the old 127-line
ledger was then removed without a fresh full read or obligation comparison.
A router load, two file-link checks and whitespace checks did not prove retained
lessons or consistent directives. Measurements remained inside the skill/agent,
and “at most three targets” conflicted with the one-target audit contract.
The following three edits added audit rows and superseding boundary tasks, but
left the new ledger's unsafe analyzer-extent H2 instruction active at review
dispatch. Current H2 records its withdrawal; the historical update was incomplete.

| Proposed destination | Baseline and acceptance measurement |
| --- | --- |
| Metric tooling: normalize unique child IDs, parent links, roles, terminal status and measured fields before aggregation | 120 mixed directories and duration-based timeout labels. Reconcile every count to identities; missing fields stay missing; timeout counts require terminal evidence. |
| Skill/agent reference: treat proposed budgets as experiments within the existing target contract | Untested limits became hard directives. Compare accepted units, evidence coverage, timeout/retry cost and retained partial results on equivalent tasks before adopting a limit. |
| Observation editing procedure: trace each corrected fact to dependent directives and retain an obligation map on migration | One stale unsafe H2 survived a superseding task. Every dependent instruction is corrected or explicitly withdrawn before dispatch; record correction and verification calls. |

The [independent review return](openspec-apply-change.md#capability-implementation-independent-review-and-task-closure)
in records 1599–1605 finally triggered H2's withdrawal and the row-5 attribution
repair: **3 parent calls / 51.151 seconds**, separate from the 83-call reviewer.
Its manual probes and fresh controls supported the transport fix, while canonical
execution still refused with zero operations. The parent correctly kept coverage
expansion unimplemented, but dropped a caller-impact requirement when checking
the rewritten handshake task. Measure correction propagation and retained
acceptance obligations, not merely checked boxes or green tests. These are shared
apply/naming observations, not additional skill invocations.

## Acceptance dispositions (790 subagent metas)

`bof3-namer`: 13 missions — rejected 9, attested 4. The rejection tail is dominated by deadline kills and honest blockers, so it is a size signal,
not a quality score: read it beside the bounded-mission rule above.

## Source-identifier capability discovery

The [2026-09-20 goal-drafting episode](plans.md#skill-scope-goal-drafting) separates
three distinct scopes: an already-split naming/macro/type skill boundary, the
remaining `bof3-namer` agent identity, and missing argument/local identifier tooling.
Historical help and owner-code reads accepted only function/data symbol transactions;
filename ownership already had source relocation. The user explicitly selected
new argument/local capability and an agent rename. The confirmed six-task goal
required contained, PRE-bound, byte-identical transactions and rollback; its
creation did not prove that capability worked or that any rename was applied.

The [next reviewed prefix](plans.md#ownership-edits-and-confirmed-scope-revision)
removed the agent rename on user request while retaining argument/local capability.
It clarified ownership references and explored a separate source transaction rather
than extending function/data report kinds; no new harness code or native check is
present through record 95. Design reasoning proposed lexical containment, collision
rejection, immutable PRE receipts and rollback, with native gates stubbed in unit
tests. Such tests could verify orchestration but could not establish native identity.
The reasoning also alternated between unchanged PRE/POST object bytes and exactness
against original game bytes. Those are different predicates for a retained partial.
Proposed acceptance record: state the eligible source class, pin both comparison
operands and distinguish actual native checks from mocked outcomes before counting
a successfully verified source rename. The initial implementation is reviewed below;
later hardening and acceptance remain pending.

This is parent discovery/planning cost (**24 calls/175.564 observed seconds**),
excluded from naming-agent performance and accepted-rename denominators. Proposed
reference improvement: keep a dated entity-to-owner/capability map distinguishing
symbol, source-file, argument and local naming from their types. Measure capability
discovery calls and incorrect-route attempts against comparable missions; verify
the owning code/help revision before treating a historical gap as current.

### Initial source-identifier implementation

Parent `01a0c062`, records **96–154**, contains complete retained assistant,
reasoning, argument and result review. Reproduction and full source hash:
`tmp/observation-ingest/older-source-implementation-metrics.json`. This is one
supporting implementation episode, excluded from the dedicated naming-agent cohort
and accepted-production-rename numerator. Its **32 calls** comprise 25 shell,
two writes and five edits across **198.452 observed seconds**; no results are
missing. Sixteen calls/76.772 seconds covered helper inspection and the initial
module, five/48.900 discovery correction and CLI wiring, and eleven/67.184 fixture,
tests and CLI checks. Phase spans omit intervening gaps; they are not command CPU
time. Repeated implementation announcements preceded further dependency reads;
their necessity and recoverable cost are not established by repetition alone.

Discovery on `emi/battle/battle/15` returned **160 rows**, with only 25 printed.
One visible false local, `xFF`, came from `selection->unk_0D = 0xFF;`. The correction
returned **159 rows** and printed 28 arguments plus any matches of a narrow member
access suspicion filter. The claim of no remaining false positives exceeded that
check. The initial input also contained `int trailing_value`, omitted from the
visible function rows: the classifier rejected built-in type keywords such as
`int`. No complete annotated population establishes precision or recall. The broad
declaration helper also returned names from a call and `return x;` during probes;
syntax eligibility matters before reusing a header-oriented parser. Missing files
and parse failures were silently skipped, and same-spelled locals were deduplicated
per function without lexical-scope identity.

The first nine-test run reported **8 passed/1 failed in 0.44 seconds**: a comment
mentioning `cell` was classified as an identifier outside its function. Masked
validation fixed that case; the same suite then passed **9/9 in 0.33 seconds**,
**13.319 observed seconds** after the failure result. A CLI-choice assertion brought
the suite to **10/10 in 0.33 seconds**. Those three runs are overlapping tests, not
28 independent accepted transactions. All three commands piped pytest through
`tail` without preserving its status; all 32 tool results had a false error flag.
One failing suite among three invocations was therefore invisible to that flag.
The real-target smoke invoked `python -m harness.naming.cli`, not `bin/naming-audit`,
and its 30-line preview did not establish a complete dispatcher result. Ruff/LSP
success and source-file-only nonmutation checks establish narrower properties than
full transaction acceptance.

The retained implementation still used raw regex substitution inside the function,
including literals/comments and same-spelled members or shadowed locals. Masking
the validation check did not constrain that mutation. Verification reversed names
using PRE offsets on POST text, leaving a length-dependent truncation risk; its
passing local rename did not exercise all positions and length changes. The native
failure test replaced the gate with a raising stub; successful verification used
`native=False`. The CLI's diagnostic `--no-native` path nevertheless returned
`verified: true`. Actual native commands, when enabled, checked exit/failure fields
and discarded semantic output; they did not compare compiled PRE and POST objects.
No successful native gate is demonstrated in this prefix.

Receipt review found further untested boundaries: a recorded receipt digest was
not checked on load, apply/verify/rollback handlers ignored their supplied target,
embedded source/backup paths were trusted, backup content lacked a PRE digest check,
and source then receipt writes had no surrounding restoration on second-write
failure. These are historical implementation findings, not present-day exploit or
regression measurements; later repairs still require source review.

Proposed improvements retain this baseline and distinguish three owners:

| Destination | Concrete change | Acceptance measurement |
| --- | --- | --- |
| Naming directive | Require evidence for the named binding, applicable native checks and explicit diagnostic-only outcomes before counting a verified rename | Independently accepted transactions / eligible attempts; report native-skipped, rejected and unknown separately, with zero diagnostic results counted as accepted |
| Naming reference | Document declaration eligibility, lexical scope, PRE/POST versus original-byte predicates, and a minimal target-resolution fixture | Compare discovery calls/time on similar missions; label declarations and skipped inputs to report precision, recall and unsupported syntax with denominators |
| Naming harness | Use binding-aware code edits, correct POST spans, authenticated receipt state and target/path checks; recover interrupted multi-file updates | On a pinned evaluation set, preserve unrelated bindings/literals, reject drift/tampering and demonstrate exact restoration; report each applicable check and unresolved class rather than one green flag |
| Check runner | Retain pytest exit status and distinguish module, wrapper and native checks | The retained failing suite must register failure; compare identical checks and outcomes before claiming any cost reduction |

These proposals are not implemented by this ingestion. The small synthetic suite
does not justify a population accuracy rate, native throughput or speedup claim.

### Full-suite CLI regression attribution

The [subsequent verification episode](bof3-test.md#metadata-rewiring-and-failed-full-suite-verification)
found **11 failures among 1,923 test outcomes**. One was caused by the new source
CLI additions: `naming/cli.py` reached **637 lines**, exceeding the existing
**600-line** module limit. The operator initially labeled every failure pre-existing,
then corrected this attribution after reading the output. The earlier ten naming
tests and Ruff checks had not covered that repository constraint.

Records **274–303** then repaired the structural regression in **14 calls/81.476
observed seconds**: one write, six edits and seven shell calls. Source pin and
call/result joins: `tmp/observation-ingest/older-cli-repair-metrics.json`. Extraction
left `cli.py` at **526 lines**, a new `source_cli.py` at **130**, and
`source_identifiers.py` at **569**. The ten capability tests plus the existing
decomposition check passed **11/11 in 0.53 seconds**; scoped Ruff and runtime
parser import also passed. This establishes the repaired size constraint and
selected checks, not native transaction acceptance. The extracted adapter retained
the earlier target-binding and diagnostic-verification limitations.

**Two of six edit calls failed** because their old-text anchor placed
`_run_prepare_transaction` immediately after `_run_source_identifiers`; the actual
next function was `_report`. The second batch repeated that wrong anchor after
reasoning blamed the matcher. Reading the exact handler boundary enabled the
successful removal **54.100 observed seconds** after the first failure result.
This is recovery latency, not all avoidable work. An intermediate undefined-name
diagnostic followed parser rewiring before its import. Repeated missing-module LSP
deliveries were not independent defects; runtime import later succeeded, without
proof that the LSP state itself refreshed. The pytest command still piped through
`tail` without preserving pytest's exit status.

Proposed naming/tooling reference improvement: identify applicable existing
structural checks while selecting the implementation owner, then run them with
the capability checks before the full suite. Measure new regressions/applicable
checks and correction cost; inspect the initial tree before classifying unrelated
failures. The enclosing verification costs belong to the shared test episode,
not an additional naming mission. No new tests are authorized by this observation.
For edit recovery, inspect the current exact boundary after a failed match before
retrying; measure repeated-anchor failures / failed edits, correction latency and
existing-check outcomes. Two failed batches alone do not establish a tool defect
or justify changing its matching behavior.

## Semantic names did not require raw-name admission

Parent **1251–1256**, `battle/03@0x801DD29C`, retained a raw function identity
already in the baseline while registering semantic file `consumePendingGroupQueue.c`
and data names `groupQueueTable` (`0x801462F6`) / `groupQueueCursor` (`0x80146303`).
The reviewer found the new names absent from baseline categories and the global
symbol gate already green, contradicting the writer's stale-baseline diagnosis.
Count **zero resolved raw function names**, two new semantic data names and one
semantic filename separately; this was supporting RE work, not an independently
completed naming mission. The review's 97 calls/482.810 seconds are shared costs.

The pre-existing `BATTLE_GLOBAL_BYTE_62F6(index)` macro addresses the same cell as
the new table name. Calling the duplication entirely pre-existing overlooks that
the second spelling was newly introduced. This is a convergence lead, not proof of
cross-target ownership or authority to rename other overlays' same-address symbols.
Proposed **reference/tooling** improvement: determine baseline applicability by
actual category and gate output, preserve target identity, and detect new aliases
of existing expressions. Measure false admission diagnoses and duplicate spellings
introduced separately from debt admissions and resolved names. Full raw child review
remains pending; pins: `tmp/observation-ingest/older-battle03-marker-lift-metrics.json`.

## Four baseline findings and inherited-change attribution

Parent **1333–1340**, `world00/area030/04@0x801E1C10`, separates native exactness
from naming hygiene. Target symbol checks passed; the global check returned **2**
with exactly **four new debt rows**: raw filename, raw function, and two raw data
names. The reviewer reported all eleven other new raw-named lift sources already
admitted, making this a pending parent baseline step rather than evidence that the
writer should have edited an out-of-scope ledger. Parent admission hygiene then
reported global success and a full pass. The script did not emit its exact mutation
diff/count; four identified omissions are not independently counted applied rows.
No semantic rename or naming-debt resolution occurred. Shared review cost was
**90 calls/375.204 seconds**; no separate naming mission is added.

The review also charged the current writer for a predecessor's extern already
identified in the starting dirty state, and called that extern's missing target-map
row debt despite the preceding review verifying shared-map/binding ownership.
Conversely, the new array addresses were payload-supported even though the native
linker could resolve their raw names without explicit map rows. Proposed
**reference/tooling** improvement: carry starting hashes, inherited hunks and resolution
provenance into naming review. Measure real omissions, inherited-change false positives,
admissions and semantic resolutions separately. Pins:
`tmp/observation-ingest/older-area030-initializer-lift-metrics.json`; full raw child
review remains pending.

## Editorial provenance and retained claims

Two documentation workers compacted this ledger's observation section, using six
calls and 235.429 summed lifecycle seconds for **136→31** and **36→29** bullets.
They are not naming missions and add nothing to the 13-run namer denominator.
The [docs review](bof3-docs.md#naming-editorial-child-review) records complete source
reads, recovered export clipping, verified byte preservation and rejected metadata
acceptance. Both briefs protected the old denominator and universal capability
claim from edits; byte preservation kept those errors until the source audit above
corrected them. Count factual corrections separately from formatting preservation.

The second rewrite dropped a measured but unidentified lift fragment: **32/32
instructions, 128→128 bytes**, with an intermediate manifest collision repaired
by an `_area01613.c` filename. This is a recoverable ownership/collision lead, not
an accepted naming transaction; its original target/run remains pending. The
worker's positional deletion also removed a historical `0/4` prerequisite status
and two unanchored register-allocation fragments. Missing identity calls for source
recovery before deciding whether a lesson is distinct or duplicated. Track resolved
source leads, naming proposals, applied renames and independently accepted outcomes
as separate numerators; reduction in prose supplies none of those outcomes.

## Important observations

29 measured observations, semantically deduplicated from Pi history.

- Nine parallel naming writers timed out with observed report-lock contention; a narrower five-child serialized pilot completed with zero renames. Different scope and checkpoint reuse prevent a causal speed comparison.
- The naming transaction gate required proposed rows; 26 reports contained 1,004 blocked row occurrences and no proposals. That snapshot established no ready transaction, not a permanent inability to prepare after evidence and capability prerequisites are met.
- A stale snapshot recipe for emi/bmagic/magic003/03 blocked naming init for every target and analysis query; 61 of 89 findings depended on that pipeline.
- The emi/scenario/scena16/00 read-only audit terminated all 13 inventory rows: 1 proposed, 12 exhausted, 0 blocked. The sole proposal was func_801F83B0 → clampPaletteChannels at 0x801F83B0.
- The emi/etc/game/01 read-only audit passed all 11 row checks and the full validator with 11 rows and complete: true; no tracked mutation was needed to close the audit.
- AREA027 func_801F3650 had exact/100.00 metadata but no residual tag; the metadata parser blocked the otherwise semantically acceptable row and the full validator exited 2.
- Three restored callers remained invalid to parse_progress_tags despite native matches of 352, 340 and 340 bytes: their residual tags appended prose to none instead of using the required literal value. Canonical formatting was already prevalent: 149/167 src/bof3/world/*.c files used residual none; func_801F365C kept its Live audit note on a separate comment line.
- func_801F42D4 at 0x801F42D4 passed the live lift gate at 24/24 instructions and 96→96 bytes but lacked all three canonical progress tags; 46 of the other 47 target sources had them.
- dispatchSecondaryState at 0x801F7144 matched 15/15 instructions and 60→60 bytes, but a complete rename would also touch three partial callers at 0x801F7230, 0x801F7790 and 0x801F7CC4, exceeding the cleanup gate requiring exactness for every touched body.
- The proposed isBattlerUnavailable rename at emi/battle/battle/03@0x801DB524 had behavioral evidence but remained transaction-blocked: the lift was only 25% matched and required authority/caller files overlapped existing parent changes.
- D_801EB440 resolved during a 140/140-instruction match despite having neither a map row nor a WEAK_SYMBOL_AT binding; address extraction from the raw symbol name let byte matching pass without explicit ownership.
- func_801F2C04 matched 17/17 instructions and passed byte matching, but Splat rejected its local-map collision with shared-map D_801F2C04 at the same address.
- Green symbol checks did not imply naming-debt closure: one audited state still had 24 filename exceptions; just doctor was 4/5 because PsyQ END.OBJ and VM_NOWOF.OBJ were absent.
- A published coverage snapshot measured 10,467 boundaries across 396 targets, 177 replica slots and 5,773 residual boundaries. Exact/partial/invalid lifts changed from 1,284/193/3 to 1,287/191/2, and naming debt fell from 23,295 to 8,274 against pinned baseline f9b03ca0.
- Configuration coverage overstated accessible code: approximately 347/396 targets were counted configured while 18 payloads had real code parked in data segments, preventing subsequent function lifting.
- AREA176–185 comprised 10 payloads with the same shape: 29 jr $ra instructions and 29 analyzer starts inside [4, 0x6E0); treating those starts as code boundaries would emit 29 false boundaries.
- A last-match-wins bug in target_facts attributed commu00’s window to WORLD04/AREA185.EMI#14. First-match guards fixed ownership; the fresh registry stayed byte-identical at 14,690,707 bytes and all 8 registry tests passed.
- emi/world00/area008/13@0x801F3344 matched 43/43 instructions and 172→172 bytes with byte-match exit 0, yet two lift-gate attempts failed solely on foreign lanes’ missing claims and changing manifest inventory.
- emi/scenario/sce10eff/00@0x801D11A0 was first-seed exact at 30/30 instructions and 120 bytes without matching aids; it still introduced three naming-debt rows requiring a parent-owned baseline refresh.
- emi/scenario/sce10eff/00@0x801D1F50 reached 69/69 instructions and 276/276 bytes on its first attempt without aids by copying an exact same-target sibling’s clean-C shape and changing its one differing immediate.
- emi/etc/game/01@0x801D17D8 improved from 20% to 100% using an indexed struct glyph table and an extern cell; independent review passed.
- emi/etc/game/00@0x80196070 matched 20/20 instructions and 80/80 bytes after replacing invalid pad_03 indexing with scalar pad_03 and field_04 accesses.
- emi/etc/game/00@0x801C3154 completed partially at 43/44 instructions (97.73%); one scheduling residual remained.
- emi/scenario/scena00/00@0x801FC3A0 stayed at 183/269 instructions (68.03%) after 8 prior and 10 new clean-C shapes; repeated allocator-shape experiments did not improve the retained result.
- emi/battle/battle/15@0x800AF5FC exposed a score/schedule tradeoff: the retained clean-C variant matched 4/28 instructions with the original opcode schedule, while naming the half shift matched 5/28 (17.86%) but scheduled shifts earlier, with the first difference at +0x0004 and unchanged 112-byte size.
- emi/world00/area028/13@0x801F318C remained partial at 161/163 instructions (98.77%); size grew from 648 to 652 bytes because of one extra nop, with no matching aids.
- emi/battle/battle/03@0x801E97A8 matched 78/90 instructions (86.67%) with a 328-byte candidate versus the 360-byte original. Missing division-trap expansion required an object-local assembler profile change; a panel +0x03 reload-order residual also remained.
- emitRadialTranslucentQuads remained unchanged across a volatile edit: 146/147 instructions (99.32%), 588 bytes and first difference +0x0074 before and after.
- Validation distinguished naming-suite results from existing debt: one naming-suite run reported 652/2, but its enclosing recipe exited 2 only at existing symbol-gate debt. A separate just check took 37.6 seconds, classified 46,592 registry windows, verified 244/244, passed integrity 57,456/57,456 and 15 pytest tests, and found zero drift before failing at pre-existing naming debt.
