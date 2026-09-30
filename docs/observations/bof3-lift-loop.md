# bof3-lift-loop observations

Per-skill half of the central observation folder ([index](INDEX.md)). The skill carries
**directives**; this file carries the **measurements** behind them. Read it before designing a
mission for this skill, and after each mission add one audit row and converge the finding into a
directive in `.pi/skills/bof3-lift-loop/` (or its references) — prose alone is not convergence.

This ledger owns **mission economics**: fan-out shape, turn cost, and the hazards that dominate
wasted calls. It was cherry-picked out of `bof3-re`'s legacy `OBSERVATIONS.md`, which owned the
fan-out and turn-efficiency rules before this folder existed, and out of the legacy `LESSONS.md`.

## Measured performance

The legacy 30-lane summary reports **median 66 tool calls, maximum 176** and
attributes the tail to three mechanical hazards. Its run membership still needs
source-level reconciliation; the [index](INDEX.md) owns the measurement method,
not a missing per-agent table.

The historical **70 tool calls / ~5 minutes** bound is a mission-sizing heuristic,
not a proven success threshold. Timeout does not establish that no useful
checkpoint exists; the naming pilot reused one from a timed-out predecessor.

### Recovered orchestration measurements

The lifecycle audit found **18 workflow parents and 123 child/standalone runs**
for this repository in `/tmp/pi-subagents-uid-1000/async-subagent-runs`.
All 123 match existing metadata IDs; none adds a new child to the 4,171-run
population. A workflow's first child cannot supply its agent identity or cost.
Do not sum parent duration with child duration or count parent failure as another
child timeout. These are orchestration observations; they do not prove that every
historical workflow invoked the current `bof3-lift-loop` skill.

The source-joined naming example has nine failed parallel children in a
929.889-second parent workflow, followed by a narrower serialized pilot with
five completed children in 1,329.071 seconds. The latter used 249 calls and
1,325.508 summed child seconds, yet made **zero renames**. Three audits returned
blocked evidence, an apply made no changes, and a reviewer accepted that no-op.
The useful outcome was a verified prerequisite/ownership diagnosis, not closed
naming debt. See [the naming measurements](bof3-naming.md#parentchild-reconciliation-and-pilot-outcomes)
for run identities, phase costs and report evidence.

### Concurrent lift throughput and harness cost-center trial

Parent `01a0c062`, records **1907–2040**, reports the initial ten-lane cohorts,
a harness optimization trial and two further ten-lane cohorts. The parent transcript is pinned by SHA-256
`cb3a63e57b9374ff832c0ec2ef522a2852e5ee54f0dd2d2d5dbc6250c42e6751`; raw child
history is still needed for per-run call counts and start/end joins.

| Cohort | Reported outcome | Wall-time evidence | Throughput conclusion |
| --- | --- | --- | --- |
| First ten-lifter group | 10/10 completed; the parent later enumerated 10 native-exact results and coverage moved exact 860→870 | `bg_wait` returned after 22m37s for 10 runs | At most **26.5 reported exact results/hour** if that wait were the whole interval. Dispatch-to-last-result time is longer or unknown; independent-review throughput is unknown. |
| Following ten-lane group | 9 exact reports; one run remained active | A 50m wait expired with one active child | Incomplete cohort; do not compute accepted throughput or compare it with the first group. |
| First post-trial cohort, records 1984–2009 | 10 async runs; 11 function reports (8 exact, 3 partial; one lane reported two functions) | All 10 completed after a 28m46s wait | At most **16.7 reported exacts/hour**; dispatch-to-last and independent-review time are unknown. |
| Next post-trial cohort, records 2010–2034 | 10/10 lanes completed; 8 exact and 2 partial reports | All 10 completed after a 21m33s wait | At most **22.3 reported exacts/hour**; dispatch-to-last and independent-review time are unknown. |
| Batch 28, records 2386–2478 | 10 exact reports; 2 additional selectors were redirected as SDK-scope mismatches | 25m53s wait | At most **23.2 exact reports/hour**; redirected work is not an exact result. |
| Batch 29 | 10 exact reports | 23m34s wait | At most **25.5 exact reports/hour**. |
| Batch 30 | 9 exact, 1 partial | 23m05s wait | At most **23.4 exact reports/hour**. |
| Batch 31 | 9 exact, 1 partial | 19m36s wait | At most **27.6 exact reports/hour**. |
| Batch 32 | Child reports show 9 exact, 1 partial; parent summary said 10 exact | 39m11s wait | Corrected ceiling **13.8 exact reports/hour**; do not use the conflicting parent count. |

Every wait-derived rate above is an optimistic ceiling, not measured end-to-end
throughput: the full dispatch-to-last-terminal interval and independent review
time are missing. The two post-trial groups have different function mixes, so
their ceilings do not establish a speedup or slowdown from the wrapper, stale
ranking option or configure lock. The user-provided estimate of **100–120 tool
calls per match** has no matched denominator in these cohorts. Earlier September
lifter metadata reports a different population (median 69, p90 121 calls); it
does not validate a current per-match average. Keep native-exact reports separate
from independently reviewed accepted-result throughput.

The later optimization trial added a `bin/lift-gate` wrapper, a stale-tolerant
`rev-query --allow-stale` option and a file lock around shared CMake configure.
The wrapper eventually passed a known exact and rejected a known partial as
expected; it needed two import repairs first. A stale ranking query worked.
An initial touched-module test run had **9 failures/452 passes** because the new
keyword broke a test's one-argument `connect` stub; the compatibility fix led to
**461 passes in 44.90s**. A stale `bin/index` attempt was followed by targeted
analysis and a successful index rebuild. The scoped check passed in 54s with
1,031 valid lift records. In the two later cohorts, at least four lane reports
mention transient build/configure or cross-lane source-claim races and retries.
The failure classes are mixed, and no lock-wait/configure-count telemetry was
retained, so these reports neither establish a lock defect nor measure its effect.
The second cohort's parent checks also reported `bin/index` rc=2 for a conflicting
function/global symbol at `area016/13:D_801F4D7C`, while `lifts valid=1,052`
differed from the batch total `lifted=1,051`; that validation/count disagreement
remains unresolved. These post-change cohorts supply bounded report-rate estimates,
not a controlled performance comparison.

The later five ten-lane batches show substantial variation even on the optimistic
wait-window measure: **13.8–27.6 reported exacts/hour**. These rates divide child
exact-report counts by the observed wait only; they omit dispatch-to-last-terminal
time, independent review, and any overlap with earlier lanes. Batch 32 is a useful
data-quality counterexample: the parent called it 10/10 exact, but the child reports
contain nine exacts and one 93.10% partial (`commu00@0x801F205C`). Batch 28 also
contains two selectors later identified as SDK-scope blocks and redirected to EMI;
count scope correction separately from completed target work.

Batch 33 (records 2490–2517) then exposed a shared-manifest failure: a sibling advertised
`src/bof3/ui/func_801F13B8_commu00.c` before the file existed. Manifest-dependent
tools failed repository-wide for about **10–12 minutes**, and **6/10 lanes** were
reported failed. A strict manifest validator test failed when a proposed tolerance
was tried, so preserve strict validation; the evidence supports atomic source/claim
publication and pre-dispatch claim-set validation as improvement candidates, not
silently accepting missing files.

Subsequent child reports in records 2581–2638 contain **15 distinct exact lift
reports**: **6/15 were first-seed exact**, while 9 needed at least one measured
shape change. One additional battle15 exact was verified by a standby lane with
zero edits after a duplicate writer had authored it. These are selected child
reports across overlapping batch work, not a cohort denominator or net coverage
delta; parent aggregates moved by fewer exacts than the visible child reports.
The same slice records stale selector returns, same-target writers, build-tree
retries, and one respawning background runner. Treat an exact report, standby
verification, newly covered selector, terminal lane and independently accepted
lift as separate units.

The `next-lift` current-worktree `@source` filter returned three selectors that
were checked against existing claims and found unlifted; its touched test suite
passed **155 tests**. This is an implementation smoke result, not a measured
throughput improvement. A useful harness evaluation should count stale picks and
duplicate writer windows before and after under matched lane counts, then join
each selected selector to one terminal child, one final gate and any independent
review.

**Performance estimate, not observed saving:** a single gate command replaces
five agent-level gate invocations with one, saving four orchestration calls when
all five would otherwise run separately; it still runs the same checks internally.
The stale-ranking fallback was estimated at five to eight calls on affected
missions. Together, those estimates imply roughly **9–12 fewer agent calls**
(about **7.5–12%** of a 100–120-call baseline) only when both cost centers occur.
They do not establish wall-time savings. The CMake lock targets observed shared
`build/cmake` removal/reconfigure races, but the transcript does not isolate the
reported 19-minute bottleneck or prove the lock reduced it; serialized lock wait
could also lower concurrent throughput.

Next comparison should record dispatch, first child start and last terminal time;
writer/reviewer calls; exact, partial, blocked and review-repair outcomes; configure
lock wait, configure count, race failures and retries. Compare similar candidate
sizes and concurrency, preserve every native/review gate, and report accepted
exact results per wall hour separately from child seconds. The acceptance test for
the wrapper is unchanged gate coverage and exits plus fewer agent calls; for the
lock it is fewer measured configure races/retries without worse end-to-end
accepted-result throughput.

### Late-September bounded lift yield and D6 alias trials

Parent 01a0d153, JSONL lines 866–1008, is pinned at
SHA-256 bddcd68b698aaea57ed89110fd2672c8368d4fd3f5e7e57295aab55eef402ed0.
The wave outcomes are useful per-task throughput evidence, but their wall-time
coverage is incomplete:

| Cohort | Attempt denominator and retained outcome | Time evidence | Throughput conclusion |
| --- | --- | --- | --- |
| Wave 1 | 6 selectors: 2 independently accepted exact transitions (33.3%), 3 partial improvements, 1 source restored after a runaway lane | First workflow timed out at 50m; accepted review ran separately and its duration is missing. One lane consumed 33m47s, 107 turns and 535,619 tokens before restoration. | 2/6 accepted exact yield is measured; accepted exacts/hour is not. The 50m timeout is only a lower bound on the overall workflow, not its duration. |
| Wave 2 | 6 selectors: 0 exact, 2 blocked on shared artifacts, 4 unchanged partials after about 24 clean-C variants | No full cohort duration retained. Three of six lanes hit a 240s open-bash alert despite a 25m workflow cap and 10-call cap. | 0/6 exact yield. Do not compare an hourly rate with wave 1 or treat many completed lane artifacts as useful output. |
| D6 alias proof | A transient 49/49 byte match for scena16 came with an invalid byte-reading sibling and was reverted. Name-keyed composition later produced one exact transition and a valid partial sibling, with target counts moving 9/11/0 to 10/10/0. | The two domain-test runs passed 104 tests in 4.73s and 3.99s. A shared-map alias then made just index fail on stale Rizin snapshot recipes; only two successive stale targets were observed before this source slice ended. | The local lift gain is measurable, but the capability was not accepted: the index was not green and independent review is absent. A feature-level accepted-throughput rate is unavailable. |

Wave 1's 2/6 and wave 2's 0/6 are different candidate cohorts with different
blockers; their combined 2/12 is not a controlled comparison. The D6 parser
increment was committed as a1615b28, but the broader alias proof remained
unfinished at this source boundary. Its shared-map placement appeared to invalidate
many analysis snapshots; the proposed target-local alias arrangement had not yet
been measured. Keep that cross-target cost as an observed failure hypothesis until
the source and snapshot receipts establish its full scope.

These cohorts show that task throughput needs both an outcome denominator and
phase timing. Preserve separate counts for unique selectors dispatched, first-seed
exacts, improved partials, unchanged partials, blocked selectors, restored
regressions, final gate results and independently accepted results. Capture
dispatch, first child start, last terminal, gate completion and reviewer completion
timestamps from run metadata. Without the complete interval, report accepted
output yield and missing timing fields; do not turn a wait or timeout into an
end-to-end rate.

### 2026-09-25 parent, records 2326-2410: lift yield and supervision

The active parent transcript is
`.pi/sessions/2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4.jsonl`,
whose reconciled full-source SHA-256 is
`40e8500c61545a9d44898e0ccd7ff36d3447de218fcaa047fc5fa952b2d2b5d1`. These
three batches contain useful outcome and recovery evidence, but their waits do
not cover complete dispatch-to-terminal intervals:

| Batch | Task and terminal evidence | Timing and review evidence | Throughput conclusion |
| --- | --- | --- | --- |
| 2 | 10 target lanes reported 29 native-exact lifts and one partial that was reverted. Serial Splat and scoped-build gates passed 10/10; index recovery plus index returned 0; source validation was 2,128 valid / 0 invalid (+29), and indexed coverage moved 1,883→1,912 (+29). | A 30m43s wait covered four asynchronous runs, not the full dispatch interval. Seven reviewer outcomes were reported accepted, but review coverage for all 29 exact reports is not established. The coverage command returned rc=2 while still writing the JSON used for counts. | Do not divide all 29 exact reports by that partial wait or call them 29 independently accepted results. |
| 3 | 24 selector candidates produced 21 exact C-lift reports and 3 data/rodata boundary reclassifications; two lanes still needed broader data-layout work. Splat and scoped builds passed 10/10; index returned 0; source validation was 2,149 valid / 0 invalid (+21), and coverage moved 1,912→1,933 (+21). | A 27m40s wait covered nine asynchronous runs; the report set included 10 lift children and 5 reviewers. The coverage command again returned rc=2 with usable JSON. | Exact-report share was 21/24 (87.5%) within this selector-candidate set; the three reclassifications are not failed C lifts. Accepted exacts/hour is unmeasured. |
| 4 | The wait returned statuses for nine asynchronous runs: 4 complete and 5 failed. Four of the five failed children retained resumable sessions; resume was required before considering replacement work. The remaining lane/terminal accounting is unresolved. | The 45m06s wait is not a complete cohort interval. Multiple children raised open-bash-over-240s attention notices, including duplicated control/detail notices. | No exact-yield or accepted-throughput rate can be computed from this incomplete outcome set. |

Batch 2 also found a concrete review defect: exact lift bodies had an invented
“BankByte4/effect-bank” role and unsupported vocabulary. The writer repaired
names and wording without changing bodies or match proofs; the refreshed gates
passed and the second review accepted the repair. Keep semantic labels attached
to source evidence, and count wording correction separately from native exactness.

The measurements suggest three bounded improvements. Skill directives should
make report states explicit—native exact, partial retained/reverted, data-boundary
reclassification, broader data-layout block, reviewer accepted, failed, resumable
and terminal—and prohibit treating a wait as cohort completion. References should
show how to justify domain labels and distinguish code-lift outcomes from data
classification. Harness receipts should join selector, child, gate and reviewer
identities; expose dispatch, child-start, gate, review and terminal timestamps;
and preserve process exit separately from usable artifact publication. For
attention events, show the active command/path/duration and resume handle once per
child/tool call while retaining the raw duplicate count. Acceptance means each
dispatched child reconciles to one terminal state, resumable failures are visible,
and exact reports/hour and independently accepted exacts/hour have separate
complete denominators and dispatch-to-last-terminal timing. Preserve every native
gate. The batch evidence and RE-specific outcome accounting are detailed in
[`bof3-re`](bof3-re.md#2026-09-27-parent-batches-2-4-outcomes-and-throughput).

## Turn-efficiency policy

The [nine-symbol dispatch review](bof3-naming.md#nine-symbol-campaign-dispatch-and-late-corrections)
adds **18 parent calls/121.426 observed seconds**, including four contract-discovery
calls after launch and ten steering calls. This is supporting orchestration
evidence, not a confirmed invocation of this skill. One invalid launch was
rejected; the accepted script planned 27 stages, while terminal state records
nine failed evidence children and one stopped apply. Nine queued corrections
have no consumption confirmation in the reviewed parent prefix. The following
compaction retained neither the parent ID nor any of the nine evidence-child IDs.
Keep these costs separate from child duration and the later recovery pilot.

Proposed operator/reference improvements: preflight complete requests and shared
write scope, branch on domain readiness, review each transaction before the next,
and retain live child/control state through compaction. Measure valid launches/N,
unnecessary apply stages/N, consumed corrections/N and recoverable checkpoints/N.
The owning naming ledger retains evidence, exact limitations and acceptance
criteria; no installed workflow tooling was changed.

The [complete first-campaign supervision](bof3-naming.md#contention-supervision-timeouts-and-stopped-workflow)
adds 19 calls over 530.299 elapsed seconds, including 347.978 seconds between
episodes while children ran. Nine explicit child timeouts preceded the stopped
workflow receipt; one apply child had already started. Every evidence child had
at least one failed follow-up, so queued steering was not a reliable prerequisite
for future stages. The report's 26 files were not 26 accepted evidence results.
Proposed controls should measure failed-evidence apply launches (baseline one),
confirmed control consumption, run-owned stop/descendant state and preserved
checkpoint integrity. Lock-wait attribution and same-scope speedup remain unmeasured.

The [pilot's parent review](bof3-naming.md#serialized-pilot-parent-cost-and-limits-of-acceptance)
adds **16 parent calls/1,429.432 elapsed seconds**, including 1,251.522 seconds
awaiting native completion. Its five children completed, but the accepted outcome
was a no-op with explicit review uncertainties. Nine parent calls corrected
ownership guidance after launch; four minutes per rung still exceeded the total
child budget when multiplied across the assigned work. Compare coherent total
allowances and accepted task units, not merely completed child processes.

| Rule | Why (measured) |
| --- | --- |
| Prefill ONCE, never piped to `head` | re-invoking `bin/harness agent context` 2–4× to page output was a top turn cost |
| Selector from the parent or the queue file | do not re-derive a candidate the parent named; if the queue lags, say so and use its `selection` command once |
| Bounded probes: permuter ≤120s; no `flag-search`/`compiler-variants` sweeps; no whole-target `bin/harness build` | each costs tens of compiles |
| Retry ONCE on a build-tree race (`stale configured build inventory`, `manifest claim directory changed`) | 10-lane concurrency artifact; then report |
| Batch gates: `bin/harness source symbols check TARGET` + `bin/harness source splat TARGET` once after all edits | per-edit gating is pure overhead |
| Canonical metadata up front (`@status`/`@match`/bare `@residual none`) | non-canonical residual lines made accepted lifts record as `invalid` |
| Bounded review (~25 tool calls) | verify the writer's measurements; no `find -newermt` windows, repo-wide surveys, cross-transaction attribution, staging checks or optional experiments |

## Fan-out turn-cost rules

| Hazard | Rule |
| --- | --- |
| Shared-build-tree / sibling-manifest races | ONE `bin/harness lift gate SELECTOR TARGET` at the very end. It retries in-process and normalizes the map first. Never wrap it in a retry loop; never re-run per edit. |
| Hand-running `cp … && bin/harness lift asm-diff` per variant | iterate with `bin/harness lift sweep SELECTOR SOURCE [--keep-best] VARIANT*.c` (one call, one row per variant). Busiest lanes spent 10–36 serial diffs. |
| Endless sweeps | cap at 8 measured variants. If none improves, keep the best coherent candidate and record the residual class + smallest missing evidence. 16–36-variant sweeps ending at the same offset are waste. |
| Re-running the prefill | ONE `bin/harness agent context reverse TARGET@0xADDRESS` (address form required; bare target is a usage error). For a slice, redirect to a scratch file. |
| Hand-ranking quick-wins rows | pick with ONE `bin/harness lift next TARGET` — it ranks and screens `div`/`divu`/`break` in one call and prints the ready `bin/harness lift gate` line. |

## Serialized writers

The naming campaign reported lock contention during nine parallel evidence
missions, all of which timed out. The later five-child pilot was narrower and
included apply/review roles; it is not a same-work speed comparison. At least one
timed-out run left a reusable checkpoint. The durable lesson is to bound work
and serialize conflicting writers, while measuring useful partial evidence,
lock wait and task completion separately. A held lock remains a stop signal,
not a reason to spend the mission retrying.

## Campaign baseline: denominator and queue evidence

Parent `01a0c062`, records **954–989**, used **14 calls/891.028 observed seconds**
including user confirmation: two context batches, eight shell calls, a
questionnaire, a proposal, a task update and a capability listing. The 13 actual
assistant reasoning blocks contain **53,419 characters**; eleven model-setting
`thinking` fields in the capability response are metadata, not reasoning.
Source pins and joins are in
`tmp/observation-ingest/older-campaign-freeze-metrics.json`. This is preparation
for the loop: zero child launches and zero newly accepted lifts in this slice.

The user confirmed whole-game work through bounded missions and explicitly chose
**documented partials when exact matching is blocked**, with retained requeue
reasons and standing authorization. The five-task goal `muaulke4-1zs3ms` retained
live native gates and independent review for each exact acceptance. Final sampled
checks did not waive those per-function obligations.

| Measurement | Retained evidence | Limit of the conclusion |
| --- | --- | --- |
| Initial inventory | 18 EMI archive directories plus two EXE directories; later metadata listed 23 configured target rows | Directory depth is not the number of individual target images, nor all shipped binaries |
| Original coverage | `--coverage-only`: **15 reported wall s, rc=2**; schema `bof3.decomp-coverage/v1`, **880 archives**, full function denominator **null** | Unconfigured slots and unknown code/data/function boundaries prevent a whole-game function denominator; analyzer candidates cannot replace it |
| Source-tag classification | 32 exact/139 partial/5 other = **176**, versus 921 lift records | Only **19.11%** accounted for, with a **745-record gap**; explicit source lists and ignored parse errors were not reconciled |
| Status command | `bin/decomp-status --json`: **498 reported wall s, rc=0**, 782 exact/139 partial/0 invalid = 921 | Default cache permitted; no cache-hit breakdown or per-function independent-review proof was retained |
| Index subtraction | 2,347 indexed minus 921 lifts = **1,426** | Arithmetic difference, not a verified target/address set difference or whole-game remaining count |
| Corrected ranking | Eight candidates from each of three targets, **24 retained rows** | A ranked subset, not an exhaustive campaign inventory |

The parent first parsed coverage with status-report fields and printed empty
lift/status rows, then inspected the actual schema. The retained blocker summary
was truncated by its command to 300 characters; the complete original blocker
list remains unreviewed. It next noticed its source-tag classification gap and
ran the status command, a useful correction. However, the saved coverage report
later omitted the original unknown-denominator warning and called the subtraction
“unlifted.” Status labels also became “authoritative” without establishing which
rows were freshly measured or independently accepted. A target with zero indexed
functions must not thereby be reported complete.

Ranking initially used positional target arguments in two failing calls. After
reading help, the parent tried another invalid positional variant before using
`--target ... --unlifted`; the corrected three-target result arrived **20.753
observed seconds** after the initial errors. The two context batches executed ten
commands and eleven searches and returned **49,184 characters**, repeatedly
including the same guidance/help. They are two orchestration calls, not ten
independent missions. Zero tool error flags concealed the coverage rc=2 and CLI
usage failures inside successful wrappers.

The frozen queue selected the first five `emi/etc/game/00` rows at
`0x801C3154`, `0x801C3598`, `0x8019AA44`, `0x801C2710`, `0x801B6418`.
`coverage.json`, `coverage.md` and `queue.json` were written under
`out/reviews/lift-campaign/`. The queue retained an index DB hash (prefix
`a4183037f2212af8`), UTC/monotonic freeze times, a **3,600-second duration**, three
attempts/two repairs per function and one writer. Its selection string said
`--limit 5`, while the observed query used eight and the first five were manually
copied. These identify the chosen rows, but not an executed five-row query.

No per-selector original/source/map pins, boot identity, explicit absolute
cutoff, consumed-limit enforcement or reconciliation with the earlier frozen
pilot appeared in the artifact. Start time plus duration can define a cutoff
within the same clock lifetime; restart/resume semantics still need evidence.
At 988 `freeze-queue` nevertheless persisted complete, **1/5**; the following
agent call listed capabilities and launched nothing. Repeated reasoning about a
nearly exhausted 1.66M-token budget cited no verified budget limit.

Proposed **operator directive**: preserve unknown denominators and distinguish
indexed, claimed, native-exact and independently accepted states. Proposed
**references**: show coverage/status schema differences, target-address joins,
valid ranking syntax and resumable limit semantics. Proposed **harness support**:
retain blockers in derived reports, reconcile every record to an explicit state,
serialize actual selection arguments and complete input pins, and bind original
limits to durable consumption. Acceptance requires no unexplained classification
gaps, no whole-game percentage from an unknown denominator, and a queue whose
membership, freshness and remaining bounds can be reverified before delegation.

## First lift dispatch and asynchronous supervision

Parent `01a0c062`, records **990–1029**, used **17 calls over 1,825.571 observed
seconds**: seven shell calls, seven subagent actions and three waits. The subagent
actions were **two launches, four status reads and one steer**, not seven missions.
Three shell calls also read child logs/state. Eighteen reasoning blocks contain
41,525 characters. Call/result joins are in
`tmp/observation-ingest/older-first-lift-supervision-metrics.json`.

Writer `c8df397d-a0df-4d4a-a31f-c4934a3bdfed` received one selector,
`emi/etc/game/00@0x801C3154`, via `$bof3-re`; reviewer
`7ab017d6-3d81-43ad-9e1c-6c7503f773dd` was launched after its partial report.
The initial handoff preserved three attempts/two repairs, but described the hour
as starting “from this mission” rather than binding the earlier queue cutoff.
It allowed source/header/map/Splat changes without naming `target.toml`, which
later appeared among the four reported changes. It also prohibited touching
`out/` evidence while requiring gates that produce disposable output there.
Explicit transaction paths and frozen-input/output distinctions would reduce
these ambiguities; the slice does not independently resolve them.

While the writer ran, two scripts created **18 additional queues**, totaling
19 including the original. Two of 21 targets with arithmetic “unlifted” counts
returned no quick-wins rows: `batl_re2/01` and `game/01`. Absence from a filtered
ranking is not absence of remaining work. New queues copied the original freeze
times and DB hash without checking current pins. A later count of **20 JSON
files** included `coverage.json`, not 20 queues. The “queue pins” check printed
stored values; it did not compare the index. A zero-file banned-aid grep was
also narrower than the full policy: the counting pattern omitted empty-asm
forms present in its display search, and neither covered direct register bindings
or invoked the canonical policy check. Treat it as a limited baseline probe.

| Supervision event | Retained measurement | Interpretation |
| --- | --- | --- |
| First blocking wait | 15-minute window elapsed; same child still running | Wait expiration was not child timeout or failure |
| Attention alert | One event repeated at 1010/1011: 67 turns/tools, 338,251 tokens, bash open 240 s | In-flight snapshot, not final child cost; repeated notices are not separate incidents |
| Steer | Queued at 1013; still pending in three later observations | Requested guidance was not demonstrated delivered |
| Second wait | Returned after a reported 1 ms on attention | Attention did not establish terminal state |
| Third wait, attention ignored | Returned after 12m39s with child complete | Completion established; partial report followed |
| Review dispatch | 12.457 observed s after writer delivery | Review started, no verdict yet in this slice |

The three waits consumed **1,659.645 observed seconds**, overlapping child work;
do not add them to child runtime as independent execution cost. The parent
repeatedly debated native notification versus autonomous continuation and then
used blocking waits despite the tool's native-wake instruction. “Active async
capacity: 0” appeared alongside the running child and was not proof of no live
work. Single-writer serialization barred another conflicting writer, not all
independent evidence work. Repeated claims of a nearly exhausted cumulative
1.7M-token budget had no verified cap and were not this child's measured cost.

The parent called the open permuter expected progress within bounds based on an
old **43/44** diff and a `timeout 900` command fragment. Those show a live command
and a retained candidate, not active iterations or deadline propagation. The
writer later reported the requested 60-second cap was absent downstream and
workers ran about 16 minutes before termination; raw child verification remains
pending. Its eight-row attempt ledger also was not reconciled with three attempts
and two repairs. See the [RE report qualification](bof3-re.md#first-campaign-writer-partial-report).

The queue entry became `disposition: partial` from writer evidence while review
had only just launched. It had no separate review-state field; that mutation must
not be counted as independent acceptance. No goal task advanced in this slice.
Proposed **operator directive**: distinguish live, attention, steer-pending,
terminal and accepted states, and retain proposals until review. Proposed
**references**: resolve native-wake/continuation behavior and define attempts,
repairs and inherited deadlines. Proposed **harness support**: coalesce duplicate
alerts, show process progress and steer delivery, verify deadlines through process
trees, and store writer/reviewer outcomes separately. Acceptance measures include
supervision calls per mission, verified steering latency, bounded worker lifetime
and zero queue acceptances inferred solely from transport completion.

## First lift acceptance and next dispatch

Parent `01a0c062`, records **1030–1047**, used **eight calls / 1,424.289 observed
seconds**: three waits, three child launches and two shell calls. Seven reasoning
blocks contain 13,728 characters. The waits account for **1,364.873 seconds**,
overlapping child work; their continued use after native-wake instructions is
the same supervision issue as above. No parent call had an error flag; independent
review nevertheless required repair. Details and joins are in
`tmp/observation-ingest/older-first-lift-acceptance-metrics.json`.

Review returned two actionable findings, the repair reached exactness, parent
native checks confirmed it, and fresh independent review passed. See the
[RE evidence and four-child costs](bof3-re.md#first-campaign-review-repair-and-exact-acceptance).
Queue mutation at 1046 now included a separate review field and `accepted-exact`,
unlike the earlier provisional partial. This is one parent-recorded acceptance,
with runtime metadata rejection still requiring reconciliation; no goal task
advanced and no refreshed whole-game coverage report appears in this slice.

| Observed transition | Seconds | Scope |
| --- | --- | --- |
| First writer dispatch → queue acceptance | 3,256.611 | One function's parent lifecycle; includes concurrent queue preparation |
| Initial partial delivery → queue acceptance | 1,443.497 | Review, repair, native checks and re-review |
| Repair verdict → repair launch | 12.670 | Parent handoff latency |
| Repair delivery → parent gate result | 15.011 | Includes 4.987-second gate call |
| Repair delivery → independent verdict | 337.904 | Parent checking, dispatch and re-review |

The repair handoff supplied concrete accept/revert measurements, which enabled
verification, but also said to keep `partial` even if the previous step reached
exactness. The writer corrected that contradiction. The parent repeated the
unverified timeout-forwarding diagnosis as a reviewer finding, although review
had only found absence of a retained duration receipt. Repair inspection then
challenged the diagnosis. A handoff should preserve an observation's confidence
and provenance rather than strengthen a writer suspicion through repetition.

Next writer `246bf438-d9e0-4c65-a7d1-003c0a688f7f` received
`emi/etc/game/00@0x801C3598` (45 instructions/four blocks) at 1045–1047. Its prompt
incorporated the fixed-RAM and permitted-aid lessons immediately. It still omitted
`target.toml` from named write scope, prohibited entry 1's files while permitting
edits to shared target maps, and restated an hour/three attempts/two repairs without
an absolute cutoff or consumed counters. Those are unresolved scope/budget
ambiguities, not proof the next writer violated them. Its result is outside this
slice. Earlier repeated concern about a cumulative 1.8M-token “limit” still lacked
a verified cap; it did not justify weakening required review.

Proposed **mission directive/reference**: bind original budget consumption and
allowed hunks, condition metadata on measured outcome, and distinguish hypotheses
from established defects. Proposed **handoff tooling**: retain typed experiment
conditions, source hashes and separate acceptance states. Evaluate on comparable
missions using contradictory-handoff corrections/N, unaccounted repairs/N,
review-to-repair latency and cost per independently reviewed task. No time saving
or permission to relax gates follows from this one case.

## Three further acceptances and accumulated verification debt

Parent `01a0c062`, records **1048–1082**, used **15 calls / 4,129.833 observed
seconds**: six child launches, six waits and three queue-update shell calls.
Twelve reasoning blocks contain 11,604 characters; no parent call had an error
flag. The **4,037.624 seconds** in waits overlap child execution. Six children
contributed 510 calls/4,062.858 summed event-span seconds, detailed in the
[RE ledger](bof3-re.md#next-three-campaign-lifts-exactness-and-unresolved-review-findings).
Joins and pinned sources are in `tmp/observation-ingest/older-next-lifts-metrics.json`.

| Queue entry | Dispatch → queue acceptance, s | Writer delivery → review launch, s | Review verdict → queue update, s |
| --- | --- | --- | --- |
| 2: `0x801C3598` | 820.588 | 9.744 | 14.767 |
| 3: `0x8019AA44` | 1,023.261 | 9.140 | 9.372 |
| 4: `0x801C2710` | 2,297.325 | 10.029 | 8.787 |

These are lifecycle spans, not exclusive CPU time or a controlled comparison.
All three review reports passed and all three entries became `accepted-exact`;
each reviewer nevertheless had runtime acceptance rejected. The parent preserved
entry 3's three notes and entry 4's comment finding as evidence text, but recording
a finding did not discharge it. Shared-map/header edits motivated prior-entry
non-regression checks and serialized review; their measured cost should not be
called avoidable waste without an equally strong replacement.

Entry 2's shell call wrote acceptance **before** the parent recheck and retained
“parent gates pending re-check” in the evidence string afterward. The recheck
displayed one `asm-diff` line per entry through `head -1`, without command exit
capture or a parent byte-match invocation. It supports the displayed matches,
not a complete fresh parent gate receipt; independent review supplied separate
byte-match evidence. Entry 4's update added `instructions:"52/52"` and a note
about the wrong queue estimate, but did not update its original
`instruction_count` field. Preserve estimates and measured values as separate
fields with an explicit reconciliation result.

Handoff scope remained imprecise: entry 3 named source/map/Splat writes, then
changed a header and manifest; review subsequently included those paths without
a preceding amendment in this slice. Entry 4 added the header to named scope,
but support bindings and manifest again appeared only later. Prohibiting prior
entries' files while authorizing changes to their shared target files requires
hunk-level baseline preservation. Mtime-based attribution does not settle these
scope gaps. No deadline/consumption receipt reconciled entry 4's seven experiment
rows with the restated three attempts/two repairs.

Entry 4's 29m21s wait ended terminally; a 240-second open-bash attention event
then appeared twice in parent delivery order (65 tools/263,056 tokens, malformed
`&1` path). This was one historical in-flight alert, not two failures or proof
the child remained live after completion. No steer was issued in this slice.
Its reported 60-second permuter overrun remains a child-verification obligation.

Stale-analysis warnings accumulated through entries 2–4; entry 4 reported three
refused tool families. The parent considered refreshing between review and the
next writer, but launched entry 5 first and deferred refresh again. No index
refresh, full `just check`, refreshed coverage or goal-task advancement occurred
here. Entry 5 (`428c9e06-1497-4734-ae37-b04469a2fb4c`, `0x801B6418`) launched with
new guidance to keep comments factual and free of transient queue state. That
is a concrete lesson transferred into the next prompt; its effectiveness remains
unmeasured until the next result is reviewed.

Proposed **operator/reference** improvements: preserve open contract findings,
bind allowed hunks and remaining budgets, and perform required parent refreshes
at a stable boundary. Proposed **handoff/queue tooling**: capture exit status and
immutable gate receipts, reconcile measured boundaries, retain separate runtime
and domain outcomes, and suppress stale attention after terminal delivery.
Measure stale-tool refusals, contradictory queue fields, unresolved findings at
acceptance and verified lifecycle latency per comparable task. Fast queue updates
alone are not successful completion.

## First batch verification and coverage correction

Parent `01a0c062`, records **1083–1107**, used **12 calls / 1,189.375 observed
seconds**: two waits, two child launches and eight shell calls. Eleven reasoning
blocks contain 23,410 characters; no parent call had an error flag. Waits consumed
**737.841 seconds**, overlapping child execution. The last lift's writer/reviewer
used 157 calls/744.598 summed event-span seconds; the full five-function cohort
is [qualified in RE observations](bof3-re.md#last-frozen-entry-and-five-function-batch-accounting).
Source pins and joins: `tmp/observation-ingest/older-batch-verification-metrics.json`.

Entry 5 moved from dispatch to parent queue acceptance in **769.538 seconds**;
writer delivery to review launch took **9.166 seconds**. All five queue rows then
said `accepted-exact`. The next five shell calls took **40.372 observed seconds**
to inspect and admit 17 naming-debt rows, including a delimiter-parsing failure
and repair. This made the naming gate green by changing its admitted set, with
zero renames; see the [naming baseline case](bof3-naming.md#campaign-baseline-admission-and-parser-recovery).

The batch check at 1101–1102 returned:

| Check | Retained result | Evidence limit |
| --- | --- | --- |
| `bin/index --recover` | rc=0, reported 5 s, `out/index/reverse.sqlite` | No subsequent Rizin freshness check or formerly refused command proved snapshot recovery |
| Full `just check` | rc=0, reported 350 s | Naming admission boundary had changed; this is not the unchanged pre-batch gate |
| Pytest within that recipe | 1,919 passed, two skipped, 341.53 s | Does not establish every lifted function's semantic acceptance |
| Following recipe gates | Ruff passed; symbols OK; 926 valid/0 invalid lifts | Five more valid records than 921; no direct recheck resolved the earlier strict-parser finding |

The combined call lasted **355.345 observed seconds**. First writer dispatch
to this batch-check result was **8,565.287 seconds**, including queue preparation,
review/repair, admission work and waits; child spans must not be added again.
Goal progress still remained 1/5. Neither the full original-function denominator
nor all outstanding source-contract findings was resolved by these checks.

Coverage update at 1103 mistakenly put global counts into the game00 target row:
`exact=782-4+5`, `partial=139-5`. Summation then reported **1,441 exact + 247
partial against 926 lifted**, an internally inconsistent result. At 1105–1106,
**12.369 seconds** after that bad output, the parent hardcoded game00 to 155
lifted/129 exact/26 partial/382 unlifted and recomputed totals: **926 lifted,
787 exact, 139 partial, 2,347 indexed, 1,421 arithmetic unlifted**. This restored
the arithmetic, not a fresh native-status measurement. Contrary to its reasoning,
the correction did not rebuild from `/tmp/status.json`; it edited existing JSON.
The regenerated Markdown still labeled its command `bin/decomp-status --json`
and displayed the old index pin after recovery, with no pin comparison. Evidence
origin and current snapshot identity therefore remained misleading or unverified.

Next writer `1fb4fd01-699b-4454-9ebc-e7907ee18e37` launched for
`emi/battle/battle/15@0x800A82F8` (26 instructions/five blocks), reusing the old
queue pin and restated hour/three-attempt/two-repair limits. Its handoff explicitly
allowed raw names pending later naming work; it did not establish approval for
future baseline expansion. This slice contains no result from that next mission.

Proposed **operator/reference** change: distinguish accepted debt from resolved
debt, index recovery from verified analyzer freshness, and arithmetic report
updates from measured coverage. Proposed **harness/reporting** change: enforce
target/global count invariants, retain baseline versus current hashes, and record
the actual command or update method. Acceptance measurements are contradictory
coverage rows/N, stale-pin handoffs/N, verified recovery of refused operations,
and outstanding findings carried through batch checks. A green aggregate check
must not erase independently observed missing evidence.

## Battle partial supervision and analysis-freshness recovery

Parent `01a0c062`, records **1108–1142**, used **16 calls / 3,199.992 observed
seconds**: three waits, three subagent actions, nine shell calls and one source
comment edit. Subagent actions were one steer and two launches, not three new
missions. Fifteen reasoning blocks contain 22,765 characters. No parent call had
an error flag, despite failing native and test commands. Joins are in
`tmp/observation-ingest/older-battle-partial-recovery-metrics.json`.

The first wait expired after 30 minutes with the writer explicitly still running.
One attention event appeared twice: 78 tools/turns, 349,549 tokens, bash open
240 seconds and malformed `&1` path. Those observations did not establish that
the entire 30 minutes was spent permuting, although the parent said so without
a status inspection. A finalize steer was queued; writer delivery followed
**98.844 seconds** later and reported stopping per steering. That supports the
reported intervention, not independently measured steer-delivery latency or
proof that steering alone caused completion. Three waits consumed **2,368.353
seconds**, overlapping writer/reviewer execution.

Review launch followed writer delivery by **9.638 seconds**. The reviewer required
a residual-comment repair and left an oversize-placement question; the parent
made the comment edit itself, preserved the score, and recorded the partial.
See the [RE evidence and open placement obligation](bof3-re.md#first-battle-partial-residual-correction-and-placement-obligation).
The parent also cleaned a malformed naming baseline row and admitted four new
rows; its first attempt reproduced the newline-parsing error. That is separate
from resolving names or satisfying placement.

| Verification/recovery stage | Retained result | Interpretation |
| --- | --- | --- |
| Full recipe after partial | rc=1, reported 339 s; 1,918 passed/one failed/two skipped in 339.38 s | Analysis-readiness test failed before later recipe gates |
| Focused reproduction | Same failure, 1.31 s | Test requested `batl_re2/01`, but the error named stale `battle/15` analysis |
| `rz-project analyze battle/15` | rc=0, reported 1 s | Readiness then identified stale reverse index instead of stale Rizin recipe |
| `bin/index` | rc=0, reported 3 s; readiness true | Focused test passed in 1.27 s |
| Full recipe after recovery | rc=0, reported 349 s; 1,919 passed/two skipped in 340.32 s; symbols OK, 927 valid/0 invalid | Aggregate checks recovered; placement and post-repair review remained unproved |

Failure return to focused success took **40.887 seconds**; to full success,
**396.827 seconds**. The six parent calls from first full check through recovery
took **736.558 observed seconds**. This is meaningful recovery cost, not a runtime
speed comparison between passing and failing suites. The cross-target readiness
failure shows that a nominally targeted test can depend on global analysis state.

The parent repeatedly treated the earlier successful `index --recover` as having
failed to refresh this state. Chronology does not support that: the earlier call
preceded this partial's edits. Help at 1136 explicitly says `--recover` reanalyzes
stale generated snapshots before rebuilding. No post-edit failure of that recovery
command was demonstrated. Manual snapshot→index recovery succeeded; it does not
prove a defect in the advertised recovery path. Distinguish post-mutation staleness
from a failed recovery on the same inputs.

Queue/coverage JSON was updated before the failing full check, by hardcoding
battle15 to 176 lifted/23 partial/317 unlifted. Totals became 927 lifted, 787 exact,
140 partial and 1,420 arithmetic unlifted; no fresh native status or original
denominator was measured, and the coverage Markdown was not regenerated. The
next writer (`93f4ac11-0168-4b45-93fe-29feee59c027`, `0x800AF6B8`) still received
the old index pin after the rebuild. New prompt guidance bounded permuter calls
with `timeout 120` and required linked-object evidence for delay-slot/copy claims.
Those are concrete lesson transfers; descendant termination and improved outcomes
must be measured in subsequent execution.

Proposed **operator/reference** improvement: bind recovery to the current mutation
generation, retain unresolved review obligations and distinguish queued guidance
from delivered guidance. Proposed **harness/reporting** improvement: expose the
global dependencies of target readiness, verify snapshot and index freshness,
preserve command exits through pipelines, and reconcile every report artifact
against its current pin. Measure recovery calls/time, stale-state repeats and
premature closure claims/N while preserving all existing gates.

## Second battle partial: preflight recovery and stale handoff context

Parent `01a0c062`, records **1143–1163**, used **nine calls/2,371.801 observed
seconds**: two waits, two launches, four shell calls and one comment edit. Nine
reasoning blocks contain 10,695 characters. Both waits returned terminal results
(writer 28m45s, reviewer 3m40s); their **1,945.770 seconds** overlap child work.
No parent call carried an error flag. The parent then read an archive JSON but
received only its artifact manifest, not the verdict; the complete report arrived
in the following notification. Pins and joins:
`tmp/observation-ingest/older-battle-second-partial-metrics.json`.

Review launched **9.398 seconds after writer delivery**. It required another
comment repair despite the stronger linked-object guidance; the next recorded
state was `partial-documented`, with byte match and independent review explicitly
pending in the repaired comment. See the [RE evidence and two-partial cohort](bof3-re.md#second-battle-partial-repeated-residual-errors-after-stronger-guidance).
The parent reused the newline-excluding debt parser, then refreshed snapshot and
index **before** running the full check. Both refreshes returned zero and readiness
was true; the recipe passed on its first displayed run: **355 wall seconds,
1,919 passed/two skipped in 345.68 seconds, 928 valid/0 invalid**. Review delivery
to full success took **385.447 seconds**. This avoids a displayed stale-analysis
failure in this episode; it is not a controlled speedup over the previous repair.
The shell sequence continued regardless of intermediate exits, and the baseline
loop could exhaust without explicitly failing, so its green result does not make
that command composition a reliable general gate.

Bookkeeping again hardcoded the target row: **177 lifted/153 exact/24 partial/
316 unlifted**, totaling **928/787/141/1,419**, with 2,347 indexed functions.
The parent corrected its own initial partial-total arithmetic before writing.
No fresh decomp-status result or regenerated coverage Markdown supported those
counts. Dispatch-to-queue-record time was **2,369.691 seconds**. The next mission
(`a58166a8`, `0x8009CF80`, 27 original instructions) retained the old index pin
after refresh and repeated the prior selector's "real 26-vs-27" example. It also
again omitted manifest writes from its allowed-path list. These are concrete
handoff consistency issues, not evidence the next function has that size delta.

Compaction record **1163** contains **5,006 characters**, but summarizes the
earlier test-organization goal and completion, omitting the active lift campaign,
its queue and newly launched child. The following continuation is outside this
checkpoint. The summary alone is therefore insufficient recovery evidence; it
must not supersede current goal state, queue receipts or live run identity.

Proposed **operator/reference** improvements: retain mutation-bound freshness
checks, resolve result manifests to their content, and validate selector-specific
counts, writable paths and current pins before dispatch. Proposed **harness**
support: fail composed gates on each required exit and include current goal/run
identities plus unresolved obligations in compaction checkpoints. Measure stale
handoff fields, lost active-run references, redundant archive reads, first-run
freshness failures and review-to-verified-disposition latency separately.

## Alignment supervisor decision and duplicate attention

Parent `01a0c062`, records **1164–1188**, used **ten calls/1,813.643 observed
seconds**: three waits, two supervisor calls (reply/pending), three subagent
actions (steer/two launches), and two shell calls. Eight reasoning blocks contain
20,974 characters; the decision alone occupies 13,132. No parent call carried an
error flag. Three waits total **1,399.090 seconds**, overlapping child activity;
the first yielded for a supervisor request with work still active, not a timeout
or failed mission. Source joins: `tmp/observation-ingest/older-battle-alignment-metrics.json`.

The supervisor request distinguished a measurable partial from a scratch-exact
alignment experiment and an out-of-scope linker change. The parent selected the
partial **18.736 seconds after request delivery**, with a persisted reply receipt
at 1169 and success at 1170. A duplicated attention event then appeared, reporting
102 tools/turns and 466,956 tokens. Its embedded send time predates the reply,
although transcript delivery follows it. The parent asserted the child was
"still waiting", checked pending (none), and queued the same decision as a steer
without checking live status. Writer delivery followed the original reply by
**49.176 seconds**, or the queued steer by **43.500 seconds**. Neither interval
proves steer delivery or that the second message was needed. This is one attention
event with duplicate representations, not two independent stalls.

The parent explicitly considered a whole-target placement check but relied on
aggregate checking instead. The independent review accepted the partial and
reproduced the scratch proof while narrowing the tooling claim; see the
[alignment evidence](bof3-re.md#jump-table-alignment-reproducible-scratch-proof-canonical-partial).
It did not close combined-layout validity. Repeated consideration of a tooling
change is not an executed repair; the recorded follow-up requires a separate
owner transaction with pipeline validation.

Before review, the parent intended to complete naming/snapshot/index hygiene
serially, but submitted hygiene and review launch in the same assistant message;
both result records share a timestamp. This does not prove an actual race, or
prove the intended dependency was enforced. The displayed hygiene returned
symbols OK, analyze/index rc=0 and readiness true. Review launch notification
followed writer delivery by **17.014 seconds**. After review, the full recipe
passed in **354 wall seconds**: **1,919 passed/two skipped in 345.28 seconds,
929 valid/0 invalid**. Review delivery to green took **360.549 seconds**;
original dispatch to green took **1,812.081 seconds**.

The queue recorded a reviewed partial and coverage added a tooling-gap entry.
Counts were again hardcoded: target **178 lifted/153 exact/25 partial/315
unlifted**; global **929/787/142/1,418** with 2,347 indexed. No fresh status
measurement or Markdown regeneration appeared. The next writer (`7852c78a`,
`0x800AF5FC`, 28 instructions) still received the old index pin after refresh
and a scope omitting manifest edits. Its handoff did remove the previous
selector's literal 26-vs-27 example; effectiveness remains to be measured.

Proposed **operator/reference** improvement: correlate attention by request ID,
send time and acknowledged reply before repeating a decision; distinguish native
partial, scratch evidence and combined-target acceptance. Proposed **harness**
support: expose delivered/queued/consumed reply states, enforce hygiene-before-review
dependencies, and retain a separately owned capability-gap queue. Measure duplicate
interventions, request-to-reply latency, outstanding placement obligations and
canonical exact results after capability repair, without counting scratch proofs
as completed lifts.

## Fourth battle entry: stale intervention and repair accounting

Parent `01a0c062`, records **1189–1220**, used **14 calls/3,833.429 observed
seconds**: three waits, four subagent actions (three launches/one attempted steer),
one read, three source edits and three shell calls. Thirteen reasoning blocks
contain 29,421 characters. Three waits total **3,335.360 seconds**, overlapping
child execution. Pins: `tmp/observation-ingest/older-battle-frontier-repair-metrics.json`.

The writer completed after a 27m57s wait; first review launched **6.974 seconds**
after delivery. The next wait explicitly returned **done** after 18m24s. A duplicated
attention event then reported 88 tools/61 turns, 257,530 tokens and bash open for
240 seconds. The parent inferred a live compile loop and steered it to finalize;
the tool rejected this because the run was **not running or queued**. This is the
episode's one error-flagged parent call. Terminal state was already available,
so this was an avoidable intervention, not a timeout recovery or successful steer.

First review required repair. The parent changed the body **16.976 seconds**
after review delivery, reproduced the higher score at **33.266 seconds**, rewrote
the residual, and launched a second reviewer at **72.532 seconds**. That review
confirmed the body but required another comment repair, applied **11.190 seconds**
after its delivery. See the [candidate and metric evidence](bof3-re.md#fourth-battle-partial-score-gain-and-repeated-evidence-repair).
The second review's mixed wording (body accepted-partial, overall verdict repair)
did not approve the later comment. The queue nevertheless described that reviewer
as accepted-partial **after** comment correction, reversing the actual order; it
also attributed the repaired lift to the original writer although the parent
made the body change. Preserve author and reviewed-content identity separately.

After final comments, asm-diff remained **9/28**, but no fresh byte-match result
or final comment review appeared. Naming/snapshot/index hygiene completed with
readiness true, then the full recipe passed in **364 wall seconds**: **1,919
passed/two skipped in 355.07 seconds, 930 valid/0 invalid**. Second-review delivery
to green took **401.079 seconds**; original dispatch to green **3,829.348 seconds**.
These are end-to-end workflow costs, not a writer deadline violation or a controlled
test-speed comparison. Parent source/metadata repair work is additional to the
three child-run costs.

Coverage again hardcoded the target to **179 lifted/153 exact/26 partial/314
unlifted**, global **930/787/143/1,417**, with 2,347 indexed. It was recorded before
the full recipe and lacked fresh status/Markdown regeneration. The next writer
(`b739779c`, `0x800AD930`, 31 instructions) was explicitly told to remeasure every
alternative it cited; the old index pin and incomplete writable-path list persisted.
That instruction's effect remains pending.

Proposed **operator/reference** improvement: honor observed terminal state before
late attention, and track body acceptance separately from unresolved report repairs.
Proposed **harness** support: suppress stale controls for terminal runs and attach
each acceptance to content hash, reviewer and timestamp. Measure rejected stale
interventions, time from first repair verdict to verified disposition, and how
often queue claims overstate the actual review receipt.

## Battle batch completion and next-target lookup

Parent `01a0c062`, records **1221–1234**, used **six calls/1,196.383 observed
seconds**: two waits, two launches and two shell calls. Six reasoning blocks
contain 4,966 characters; waits total **814.624 overlapping seconds**. Review
launched **6.518 seconds after writer delivery**. The function was independently
exact, but the overall review required three naming-baseline admissions. The
parent recorded `accepted-exact`, called the review verdict accepted, and wrote
`just check rc=0` into coverage **before** running that check. The later result
did pass; it does not validate recording anticipated evidence as already observed.
Pins: `tmp/observation-ingest/older-battle-batch-completion-metrics.json`.

Symbols, snapshot and index checks returned zero with readiness true. The full
recipe then passed in **347 wall seconds**: **1,919 passed/two skipped in 337.25
seconds, 931 valid/0 invalid**. Review delivery to green took **360.711 seconds**;
this entry's dispatch to green **1,188.173 seconds**; first battle dispatch to
batch-final green **12,426.351 seconds**. The [five-entry cohort](bof3-re.md#battle-batch-closure-one-exact-lift-and-four-partials)
separates its one exact/four partial outcomes from aggregate checks and unresolved
placement/report obligations. "Queue complete" means those five selected entries
received dispositions, not that battle15 was fully lifted.

Coverage again hardcoded target **180 lifted/154 exact/26 partial/313 unlifted**
and global **931/788/143/1,416**, with 2,347 indexed. The batch note was refreshed,
but no fresh status result or regenerated Markdown was shown. Next-target lookup
successfully printed battle03's first three selectors, then failed on guessed
`emi_etc_shop_00.queue.json`; the third lookup never ran. This is the episode's
one error-flagged call, not a failure to retrieve the chosen battle03 selector.
The parent recognized the filename mismatch and launched `8bd43905` for
`battle/03@0x801E71EC`, still carrying the old index pin and omitting manifest/
support paths from the explicit scope. It transferred the qualifier lesson as
permission for evidence-backed declaration corrections, requiring scratch
comparison and review; broadening that into a universal rule remains unjustified.

Proposed **operator/reference** improvement: write receipts after observing their
results and preserve mixed review verdicts. Proposed **harness** support: obtain
queue paths from an inventory instead of guessed naming conventions, and publish
coverage from measured dispositions with original denominators and current pins.
Measure premature success claims, failed queue lookups and scope/pin inconsistencies
alongside exact/partial outcomes and end-to-end batch time.

## First battle03 acceptance retained a parser disagreement

Parent `01a0c062`, records **1235–1246**, used **five calls/1,404.505 observed
seconds**: two waits, two launches and one shell call. Five reasoning blocks
contain 5,500 characters; waits total **1,011.202 overlapping seconds**, returning
terminal results after 11m2s and 5m48s. No parent tool carried an error flag. Review
launched **6.275 seconds after writer delivery**. Pins:
`tmp/observation-ingest/older-battle03-field-lift-metrics.json`.

Review accepted the 47/47 exact lift but explicitly found lifecycle/index metadata
invalid. The parent considered normalizing its own sources, then deferred it as a
pre-existing convention while retaining accepted-exact and a hardcoded global
`invalid=0`. The [RE evidence](bof3-re.md#first-battle03-lift-struct-field-recovery-and-invalid-lifecycle-metadata)
shows why these are distinct validators and acceptance dimensions. A reported
730/939 prevalence does not excuse a newly authored parser-invalid instance.

Global symbols, snapshot and index checks returned zero, readiness true, and the
full recipe passed in **361 wall seconds**: **1,919 passed/two skipped in 351.74
seconds, 932 valid/0 invalid**. Review delivery to green took **377.100 seconds**;
dispatch to green **1,400.690 seconds**. No displayed refreshed lifecycle row
resolved the disagreement. Coverage hardcoded battle03 to **218 lifted/180 exact/
38 partial/154 unlifted**, global **932/789/143/1,415**, with 2,347 indexed; no
fresh native coverage result or regenerated Markdown appeared.

The next mission (`e5385ef0`, `battle/03@0x801DD29C`) inherited the old index pin
after refresh and the manifest scope omission. It did receive a concrete new
instruction: prefer existing struct fields for proven folded-symbol loads, and
use a semantic filename when behavior is clear. Outcome evidence remains pending.
Proposed **operator/reference** improvement: keep parser failures as explicit
obligations even when byte gates pass. Proposed **harness** support: include native,
lifecycle and aggregate-validation states in one receipt with current pins; measure
state disagreements and how many are closed before queue acceptance.

## Second battle03 acceptance: stronger byte proof, incomplete receipts

Parent `01a0c062`, records **1247–1260**, used **six calls/2,152.025 observed
seconds**: two waits, two launches and two shell calls; six reasoning blocks,
3,558 characters, zero error-flagged parent calls. Waits total **1,755.432
overlapping seconds**, not additional cost. Writer delivery to review launch was
**7.215 seconds**, review delivery to full green **375.104**, dispatch to full
green **2,148.240**. Metrics: `tmp/observation-ingest/older-battle03-marker-lift-metrics.json`.

The first archive read returned only the last 3,500 of a 9,378-character report;
the subsequent full notification recovered its content. Resolve the actual artifact
path and consume the full report before accepting it. The independently reproduced
[marker counterfactual](bof3-re.md#marker-reuse-exact-bytes-and-an-independent-counterfactual)
strengthened native evidence, while lifecycle and attempt-accounting gaps persisted.
The reviewer corrected the writer's claim that the new semantic data names required
baseline admission. The parent's generic baseline loop emitted no mutation count;
a green symbol gate alone cannot measure how many admissions it performed.

Analysis/index commands were invoked with discarded output and no individual exit
receipts; readiness was true. Do not infer both return codes from the enclosing
shell success. The full check returned zero in **362 wall seconds**, with **1,919
passed/two skipped in 353.89 seconds, 933 valid/0 invalid**. Coverage was hardcoded
to battle03 **219 lifted/181 exact/38 partial/153 unlifted**, global
**933/790/143/1,414**, out of 2,347 indexed. These are recorded values, not a new
native coverage measurement or resolution of the strict-parser disagreement.

Next writer `86f6ebb6` (`battle/03@0x801DCFD0`) received the marker-reuse lesson,
but again inherited the old pin and omitted manifest/support scope. Its result is
pending. Proposed **operator/reference** improvement: carry forward verified,
qualified lessons and outstanding obligations together. Proposed **harness** support:
emit each gate's return code, actual baseline changes and refreshed input pins;
measure missing receipts, truncated reports recovered and obligations left at acceptance.

## Third battle03 acceptance and regression-population blind spots

Parent `01a0c062`, records **1261–1276**, used **seven calls/1,562.834 observed
seconds**: two waits, two launches and three shell calls. Seven reasoning blocks
contain 3,442 characters; zero parent calls were error-flagged. Waits total
**1,169.026 overlapping seconds**. Writer delivery to review launch was **5.991
seconds**, review delivery to full green **371.383**, dispatch to full green
**1,556.306**. Pins: `tmp/observation-ingest/older-battle03-copy-lift-metrics.json`.

The reviewer independently reproduced both the exact result and the writer's 43/47
counterfactual, but accepted another strict-parser failure. Its index-exact sweep
was a filtered three-function population, not all byte-exact lifts; separate checks
covered prior lifts and table consumers. The archive read again selected only a
report tail; the full notification supplied the missing evidence. These recurring
patterns should be counted, not silently turned into clean acceptance receipts.

The full recipe passed in **360 wall seconds**: **1,919 passed/two skipped in
350.00 seconds, 934 valid/0 invalid**. Readiness was true; individual analysis/index
return codes were again not emitted. Coverage was hardcoded to battle03
**220 lifted/182 exact/38 partial/152 unlifted**, global **934/791/143/1,413** out
of 2,347 indexed. No displayed refreshed lifecycle rows resolved the disagreement.
A queue read correctly selected the next pending function, `0x801E2170`, avoiding
a guessed path. Writer `4007a764` inherited the old pin and manifest scope omission,
plus the verified loop-local-base lesson; its result remains pending.

Proposed **operator/reference** improvement: distinguish complete native regression
coverage from index-filtered samples and carry parser failures forward explicitly.
Proposed **harness** support: show selection predicates, omitted populations and
unique checked selectors in review receipts. Measure gaps closed before acceptance,
not just the number of green commands.

## Gate and writer overlap: verified timing, unproved failure cause

Parent `01a0c062`, records **1277–1296**, used **nine calls/1,722.023 observed
seconds**: three waits, three launches and three shell calls. Eight reasoning blocks
contain 6,817 characters; zero calls carried an error flag despite a failed full
check. Waits total **1,334.118 overlapping seconds**. Three completed children used
**241 calls/1,682.861 summed seconds**; the last child's review is pending. Fourth
writer delivery to review launch took **5.973 seconds**, review delivery to failed
gate **353.780**. Pins: `tmp/observation-ingest/older-battle03-symbol-lift-metrics.json`.

The parent issued gate and next-writer calls in one message. Raw child timestamps
confirm overlap: writer `c7f4044c` began **13:59:50.022 UTC**, 0.425 seconds after
the gate shell began, and remained active throughout its remaining **345.288 seconds**.
Child record **89**, fully read, logs a source-write start at **14:03:15.443**;
the shell returned at **14:05:35.310**. This establishes writer activity during
validation, not the precise cause of its test failure. The full recipe returned
**1 in 338 wall seconds: 1,918 passed/one failed/two skipped in 336.96 seconds**.
`test_live_asm_diff_rejects_inherited_caller_harness` then passed alone in **4.59
seconds**, **147.384 seconds** after the failure receipt. The parent called that
confirmation of a race; the actual failed assertion was not shown. Recovery of the
full gate remains pending. See the [test qualification](bof3-test.md#writer-overlap-does-not-alone-prove-test-causality).

The 2m13s wait was remaining wait time, not total writer duration: its raw session
spanned **481.128 seconds**. The parent's “finished quickly, likely error” inference
and writer's approximately 25-minute claim are both unsupported by that span.
Archive tails were later recovered by full notifications. The next mission added
support-file scope after a review finding that appears to misattribute prior edits;
it still omitted manifest scope and reused the old pin. Coverage hardcoded battle03
**221 lifted/183 exact/38 partial/151 unlifted**, global **935/792/143/1,412** out
of 2,347 indexed. Readiness was true; analysis/index return codes were not emitted.

Proposed **operator/reference** improvement: serialize tree-mutating writers and
validation, measure total runtime separately from wait remainder, and retain failure
uncertainty until diagnostics explain it. Proposed **harness** support: stamp run
intervals and source generations into gate receipts; enforce writer/gate exclusion.
Measure overlapping validation runs, demonstrated race failures, isolated recoveries
and full recoveries as separate counts. A failed test remains a failure regardless
of the shell tool's success flag.

## Battle03 closure: comment repair required an index refresh

Parent **1297–1312** used **eight calls/1,120.156 observed seconds**: one wait,
five shell calls, one edit and one launch. Seven reasoning blocks contain 10,653
characters; zero tool error flags despite a failed full check. The wait was
347.766 seconds. Pins: `tmp/observation-ingest/older-battle03-completion-metrics.json`.

After the fifth review, the parent recorded acceptance and refreshed analysis/index
before correcting the aid comment. Native asm output remained **73/73**, but the
full check failed in **353 wall seconds: 1,918 passed/one failed/two skipped in
352.18 seconds**, at the readiness test. The diagnostic was specifically **stale
reverse-index source fingerprints**, not the Rizin snapshot-recipe error the parent
predicted. Analyze and index then explicitly returned zero, readiness became true,
and the isolated readiness test passed in **1.25 seconds**. Because both commands
ran, this episode does not prove re-analysis was necessary rather than index refresh
alone. Final full check passed in **357 wall seconds: 1,919 passed/two skipped in
347.98 seconds, 936 valid/0 invalid**.

Review delivery to final green took **760.899 seconds**; the second failure to
green **375.593**, and the earlier overlapping-writer failure to green **1,266.663**.
The final pass closes full validation recovery, not proof of the earlier race cause.
The [batch metrics](bof3-re.md#battle03-batch-closure-factor-separation-and-acceptance-metrics)
retain ten child runs, five native exact acceptances and their unresolved findings.
Coverage remained hardcoded: battle03 **222 lifted/184 exact/38 partial/150
unlifted**, global **936/793/143/1,411**, with 2,347 indexed.

Queue inventory avoided another guessed-path failure, but its `first` field printed
the first entry rather than the first pending entry. The chosen new target's first
entry was pending, so no wrong dispatch is shown. Writer `cbcafe20` was launched for
`world00/area030/04@0x801DDEEC` with the old pin, manifest scope omission and a
weakened “build (if it passes)” instruction. It did inherit all four required aid-
comment elements. Its result remains pending. Proposed **operator/reference**
improvement: finish all edits before freshness checks, follow the exact diagnostic's
recovery layer, and keep gate requirements unconditional. Proposed **harness** support:
select pending entries directly and bind validation receipts to final input hashes.

## First area030 lift: serialized validation still needed a retry

Parent **1313–1330** used **eight calls/2,357.950 observed seconds**: two waits,
two launches and four shell calls. Eight reasoning blocks contain 5,923 characters;
waits total **1,595.394 overlapping seconds**. Zero parent error flags coexist with
a failed full recipe. Writer delivery to review launch was **6.541 seconds**,
review to final green **741.221**, dispatch to green **2,349.879**. Pins:
`tmp/observation-ingest/older-area030-cell-lift-metrics.json`.

The accepted 36/36 lift carried a reproduced aid and a 36-consumer shared-declaration
obligation. The parent explicitly deferred the next writer until validation ended.
Nevertheless the full recipe failed in **349 wall seconds**, reporting **1,918
passed/one failed/two skipped in 349.15 seconds** at
`test_type_execution_context_rejects_invalid_run_before_apply[False]`. The isolated
case passed in **0.13 seconds**, and the full rerun passed in **369 wall seconds:
1,919 passed/two skipped in 360.09 seconds, 937 valid/0 invalid**. Failure to focused
pass took **6.212 seconds**; failure to full green **378.566**. The parent labeled
this a known flaky case without displaying the assertion or a causal diagnosis.
This supports observed intermittent failure/recovery, not a specific flake mechanism.

Readiness was true; individual analysis/index return codes and baseline admission
counts were not emitted. Coverage hardcoded area030/04 to **59 lifted/52 exact/
7 partial/115 unlifted**, global **937/794/143/1,410**, with 2,347 indexed.
The next writer (`4ebf8b66`, `0x801E1C10`) received the correct primary load,
canonical residual syntax and an unconditional target-build instruction. It still
inherited the old pin and omitted manifest/support scope despite requiring every
changed path in the report. Outcome remains pending.

Proposed **operator/reference** improvement: preserve serialized gates and exact
failure evidence; do not convert retry success into a diagnosed cause. Proposed
**harness** support: retain the failed assertion and input-generation identifiers
beside focused/full rerun results. Measure first-gate pass rate, retries and recovery
latency independently from writer overlap and accepted native results.

## Second area030 acceptance after four baseline findings

Parent **1331–1340** used **four calls/1,096.251 observed seconds**: two waits,
one review launch and one shell call. Four reasoning blocks contain 2,739 characters;
waits total **715.281 overlapping seconds**, with zero error-flagged parent calls.
Writer delivery to review launch took **6.253 seconds**, review delivery to full
green **371.496**, original dispatch to green **1,099.312**. Pins:
`tmp/observation-ingest/older-area030-initializer-lift-metrics.json`.

Review returned naming-only repair while verifying native **27/27, 108 bytes**.
The parent recorded accepted-exact before its generic baseline loop ran, retained
the review's actual needs-fix wording in queue evidence, and subsequently observed
global symbol success and full green: **358 wall seconds, 1,919 passed/two skipped
in 349.21 seconds, 938 valid/0 invalid**. Readiness was true; individual analysis/
index exits and baseline mutation count were not shown. Coverage hardcoded
area030/04 **60 lifted/53 exact/7 partial/114 unlifted**, global **938/795/143/
1,409** out of 2,347 indexed. Final gate evidence closes the naming prerequisite;
the earlier queue write alone did not.

The parent repeated the reviewer's erroneous header-undercount finding, although
the writer had explicitly attributed the extra extern to entry 1. Proposed
**operator/reference** improvement: reconcile reviewer findings against the starting
state and keep acceptance provisional until its required repair succeeds. Proposed
**harness** support: bind queue acceptance to gate receipts and emit actual baseline
changes. Measure valid review findings resolved, false findings propagated and time
from review to verified closure separately.

## Explicit user correction changed the parent gate's meaning

Parent **1341–1368** paused new lift dispatch to handle the user's request that
per-lift `just check` omit harness pytest units. The recipe split retained Ruff,
symbols and source validation in a **9-second** check, with full pytest moved to
`check-all` and unit pytest retained in `check-unit`. The preceding 358-second
full check is a different workload. The [test-owner audit](bof3-test.md#user-directed-split-of-lift-checks-from-harness-tests)
records 14 correction calls and limited wrapper verification; do not add its costs
again as a separate loop mission.

The parent then dispatched writer `882c0714` for `area030/04@0x801E0ABC`, explicitly
forbidding mission-level `just check` and describing the new parent gate. It carried
forward initializer and scratchpad-view lessons, canonical residual syntax and
support-file scope, but still retained the old index pin and omitted manifest scope.
Focused checks and the launch were issued in one message; no failure or actual
write/test overlap is established here. The wait returned terminal after **728.914
seconds**, but the writer's report is not yet reviewed at this checkpoint.

Proposed **operator/reference** improvement: preserve a scope/version boundary in
validation history when users revise the gate. Proposed **harness** support: attach
selected checks and source generation to acceptance receipts. Measure native lift
outcomes separately from harness-test coverage and preserve the outstanding review
when a new user correction interrupts the campaign.

### 2026-09-23 parent, records 1764–2115: text-index and CLI performance

This pinned cohort
(`.pi/sessions/2026-09-23T00-57-03-517Z_01a0cbc4-211d-7757-881d-33ed2fcd42d7.jsonl`,
SHA-256 `fd34915b19d843ee3a29611c98d53e24938f5e4baa9344c6c7e866c8010aa2bc`)
records historical tool results, not current-tree acceptance:

| Measure | Result | Performance conclusion |
| --- | --- | --- |
| Archive text index | 880 US archives / 259,745,792 bytes; 59,105 instances; build took 5.7–6.1s. Removing duplicate decoded text reduced the artifact from 109.3 MB to 15.4 MB; adding joined readings later made the final artifact 20.4 MB. | Keep serialized size and feature payload separate when evaluating the optimization; the 15.4 MB intermediate was not the final artifact. |
| Query latency | Tested low-cardinality queries took 433–556ms versus 921–939ms for corpus rescans (1.7–2.2× faster). One 40,263-match query took 1,167ms, slower than the scan baseline. | The index helps the tested sparse queries but does not establish a broad-query speedup. Measure by match-count and output-size buckets. |
| Search recall | Across 30 area archives / 2,082 rows, 1,511 command-interrupted phrases had only 4 findable results at baseline; token-aware matching later found 1,511/1,511. | Recall reached the observed corpus denominator; residual false-positive precision was sampled, not measured exhaustively. |
| Integrity and parity | A two-bank offset defect was fixed; then 59,105/59,105 entries verified against the archives in 2.3s. Four parity queries matched exactly: `spring` 9, `scattered` 42, `McNeil` 189, `Nue` 20. A stat-manifest freshness check took 530ms but is heuristic; content verification remains authoritative. | Correctness and freshness gates remained separate from lookup speed. Two report-label/count discrepancies were caught and repaired; no data class was promoted. |
| CLI reachability | The audit found 4/10 modes unreachable and three stale `.txt` examples. The first ten-example run passed 7/10 because `out/text/` was missing; after output-directory creation and dispatch/docs fixes, all 10 passed and extract→pack was byte-identical. | The repair resolved observed reachability and setup failures; the timings do not establish CLI throughput improvement. |

Optimization candidate: the 40,263-result query regressed against the old scan even
while sparse queries improved. Profile result materialization and bounded/paged
output on a representative match-count distribution; accept only with exact query
parity, unchanged archive verification and reported p50/p95/max latency by result
size. For search, retain both **1,511/1,511 recall** and a measured false-positive
denominator. For CLI changes, keep all ten examples passing and extract→pack
byte-identical while recording per-mode dispatch, setup failures and elapsed time.
The single broad-query result prioritizes a controlled trial; it does not prove
that pagination or another implementation will be faster.

## Iteration audit log

Batch 025 (`d453047a`, child `049c112a`) supplied a current baseline: one
17-instruction/68-byte first-seed exact lift, 46 completed calls, 360.895 seconds
workflow time, one gate and no retries/sweeps. Two prefill calls, unused mandatory
m2c/m2ctx and root-recursive searches preceded the seed. Summed call latency is
not exclusive wall time. The five-file protocol delta and PRE snapshots are in
`out/lift-batches/protocol-efficiency/`; it makes prefill/decompiler/search work
gap-driven, checks lever eligibility before editing, names before the first gate,
and carries complete ledgers plus a cumulative 12-call/four-variant ceiling into
repairs. Review found the initially omitted immutable call limit in the repair
brief; re-review passed. The 22-instruction trial was first-seed exact but used
60 writer calls and 586.638 workflow seconds, including 27 unnecessary reviewer
calls: nested lever statuses incorrectly triggered the old regex router, then
reviewer JSON became `complete/unknown`. No speedup established. A focused JSON
membership/outcome router passes eleven disposable replays, including the actual
trial output routed to one-child exact-all and superseded PASS envelopes rejected.
Independent re-review `18b12e02` passed after tightening the whole-response JSON
fence. This is report validation, not native proof or machine budget enforcement.
Native and classification gates remain unchanged.

Batch026 (`cbf08e8b`, child `bc74b33a`) was native-exact24/24 but reporting-blocked:
40 calls/217.805s, one prefill/gate, no sweep. The reserved acceptance-report
wrapper embedded inside lane JSON violated runtime attestation. Keep the runtime
acceptance fence separate; it is validated then removed before strict routing.
Instruction correction reviewed PASS (`7a824a8f`); same-writer report-only recovery
`063ce36b` attested, with unchanged source/config hashes and original consumption.
Recovered-output replay routes one-child exact-all; original blocked receipt is
not rewritten. This is not an end-to-end live workflow success or speedup proof.

Batch027 was native-exact27/27 but extra prose blocked strict routing; report-only
recovery corrected consumption to5/12 without source changes. Live transport pilot
`38bd3cea` isolated a second failure: the runtime appended a saved-output notice to
valid JSON. Explicit child `output:false` in pilot `4385f4ca` passed attestation and
actual-output routing with unchanged source/config hashes. Reviewer `48d236e7`
accepted the canonical delta: no parser/acceptance relaxation; aggregate reports
retain every resolved stage's output, run ID and artifact references.

Batch028 (`aea1bb41`, child `dead26ad`) then completed the canonical initial-lift
path: exact27/27,108 bytes,42 child tools,307.866s workflow,2/12 harness calls,
no sweep/reviewer/repair. Gate wall69.203s. Serial Splat/build/index succeeded;
source validation2564 valid/0 invalid and target symbols passed. Parent wrapper
interrupted before returning coverage; the unfinished command was rerun after
process inspection, rc2. Target asm51→50,c14→15; configured global asm8113→8112.
Support-source discrepancy count15→16 remains open. No causal speedup or live
repair/fanout proof follows. Full evidence: `out/lift-batches/batch-028/` and
`out/lift-batches/report-transport-pilot/`. The owning skill now requires report-only
transport probes for format failures; native work and consumed budgets stay frozen.

| Iteration | Mission | Tools | Duration | Method verdict | Converged into |
| --- | --- | --- | --- | --- | --- |
| 2026-09-27 | Batch029 two-target lane trial | 36/99 child tools; 2/14 actual harness calls | 1061.425s workflow; child times overlap | Both native exact/attested; lane2 exceeded12-call ceiling by excluding3 queries and5 help/examples, so not fully protocol-compliant; coverage rc2 | Explicit every-invocation counting in workflow lift/repair briefs and lift-loop skill; original budget and reports preserved |
| 2026-09-27 | Batch028 and transport recovery | 42 child tools; 2 harness calls in source lane | 307.866s source workflow; recovery separate | Initial-lift exact-all and attested; global coverage rc2; no speedup claim | Lift-loop transport directive; canonical output:false and retained stage reports |
| 2026-09-27 | Batch 025 protocol audit | 46 child calls | 360.895 s workflow | First-seed exact; trial removed repeated prefill/decompiler calls but exposed redundant review; no speedup claim | Workflow, lifter agent, reverse protocol and lift-loop directives; native gates retained |
| — | (fan-out rows above were absorbed from the legacy ledgers) | 66 median / 176 max | — | Dominant waste is mechanical recovery, not lifting | Turn-efficiency policy, Fan-out rules |
| 2026-09-26 | Reconcile parent/child accounting and naming pilot | 249 across 5 pilot children | 1,329.071 s parent | All children completed; zero renames; valid blocked/no-op outcome | Proposed measurement and prerequisite-routing improvements below |
| 2026-09-25 parent, records 2326–2410 | Batches 2–4: lift yield, independent review and async recovery | Parent toolCount unavailable; 10 target lanes/batch reported | Partial waits 30m43s, 27m40s and 45m06s; full dispatch-to-terminal intervals missing | Batch 2: 29 exact reports + 1 reverted partial; batch 3: 21/24 exact reports + 3 data reclassifications; batch 4: 4 complete / 5 failed among 9 observed async runs, 4 resumable; no accepted-exact rate | [Batch yield and supervision](#2026-09-25-parent-records-2326-2410-lift-yield-and-supervision); proposed directive, reference and receipt improvements; no skill contract changed |
| 2026-09-20 parent, records 954–989 | Confirm campaign, measure baseline and freeze first queue | 14 parent calls | 891.028 observed s, including confirmation | Original denominator unknown; 498-second status returned 782 exact/139 partial; five entries frozen, task 1/5; no lift accepted | Denominator/state separation, schema-aware reports and complete queue provenance above |
| 2026-09-20 parent, records 990–1029 | First writer dispatch, waits and review handoff | 17 parent calls | 1,825.571 observed s, including overlapping waits | Writer returned 43/44 partial; review launched; no independent acceptance; timeout/attempt claims require child verification | Async state distinctions, deadline enforcement and proposal/review separation above |
| 2026-09-20 parent, records 1030–1047 | First review, repair, exact acceptance and next dispatch | 8 parent calls | 1,424.289 observed s | One repair led to parent-confirmed 44/44 exactness and reviewer pass; runtime metadata rejected; next writer launched | Conditional handoffs, hypothesis provenance, budget and acceptance-state accounting above |
| 2026-09-20 parent, records 1048–1082 | Entries 2–4 review and queue acceptance; entry 5 dispatch | 15 parent calls | 4,129.833 observed s | Three review passes and native exact results; contract notes, runtime rejections and stale-analysis debt remain | Receipt completeness, scope/budget accounting and findings retained through acceptance above |
| 2026-09-20 parent, records 1083–1107 | Last frozen entry, batch checks and coverage correction | 12 parent calls | 1,189.375 observed s | Full check passed after admitting 17 debt rows; coverage arithmetic repaired, freshness and contract gaps remain | Admission versus repair, coverage invariants and evidence-origin reporting above |
| 2026-09-20 parent, records 1108–1142 | Battle partial, residual repair and freshness recovery | 16 parent calls | 3,199.992 observed s | 21/27 partial retained; failed full check recovered after snapshot/index refresh; placement and repaired-state review remain open | Evidence-based residuals, mutation-bound recovery and unresolved-obligation accounting above |
| 2026-09-20 parent, records 1143–1163 | Second battle partial, preflight refresh and next dispatch | 9 parent calls | 2,371.801 observed s | 7/27 partial; residual repair repeated; first displayed full check green; compaction omits active campaign | Candidate-bound residuals, fresh handoff facts and active-run checkpoint completeness above |
| 2026-09-20 parent, records 1164–1188 | Alignment supervisor decision, independent scratch proof and partial acceptance | 10 parent calls | 1,813.643 observed s | 25/27 canonical partial; scratch code/data exactness reproduced; full check green; combined placement unproved | Scoped capability claims, attention deduplication and canonical-versus-scratch outcomes above |
| 2026-09-20 parent, records 1189–1220 | Fourth partial, two reviews and parent repairs | 14 parent calls | 3,833.429 observed s | 5→9/28 confirmed; residual repair repeated; final comment not independently reviewed; full recipe green | Candidate-bound claims, accurate score terminology and content-bound acceptance receipts above |
| 2026-09-20 parent, records 1221–1234 | Last battle entry, batch closure and next target | 6 parent calls | 1,196.383 observed s | 31/31 exact; naming repair followed by full green; batch one exact/four partials; guessed queue path failed | Scoped qualifier lessons, observed receipts and role-separated batch metrics above |
| 2026-09-20 parent, records 1235–1246 | First battle03 field-based exact lift | 5 parent calls | 1,404.505 observed s | 25→47/47 via existing fields; native and aggregate green, lifecycle invalid retained | Representation witnesses, accurate diagnostics and separate acceptance dimensions above |

| 2026-09-20 parent, records 1247–1260 | Second battle03 exact lift and marker counterfactual | 6 parent calls | 2,152.025 observed s | 45/45 control versus 27/45 literal variant; full green, lifecycle and receipt gaps remain | Qualified aid evidence, baseline applicability and explicit gate receipts above |

| 2026-09-20 parent, records 1261–1276 | Third battle03 lift, copy counterfactual and const repair | 7 parent calls | 1,562.834 observed s | 43→47/47 reproduced; stores justify qualifier change; lifecycle mismatch persists | Population-aware regression receipts and semantic evidence above |

| 2026-09-20 parent, records 1277–1296 | Symbol-bound fourth lift and overlapping fifth writer | 9 parent calls | 1,722.023 observed s | 69/69 accepted; full gate failed during verified writer overlap; isolated case recovered, causality/full recovery pending | Input-generation isolation, scope attribution and qualified recovery above |

| 2026-09-20 parent, records 1297–1312 | Final battle03 review, comment repair and gate recovery | 8 parent calls | 1,120.156 observed s | Four variants separate two causes; stale index fingerprints repaired; full green, five native exact acceptances | Final-input freshness, diagnostic-specific recovery and batch acceptance dimensions above |

| 2026-09-20 parent, records 1313–1330 | First area030 lift and serialized gate retry | 8 parent calls | 2,357.950 observed s | 36/36 with reproduced cell-volatility control; one unexplained full failure then recovery | Consumer-scope measurement and diagnosis-aware retry metrics above |

| 2026-09-20 parent, records 1331–1340 | Second area030 exact lift and naming-only repair | 4 parent calls | 1,096.251 observed s | 26→27/27; four baseline findings closed by parent gate; inherited-hunk finding propagated incorrectly | Finding provenance, measured experiments and receipt-bound acceptance above |

| 2026-09-21 parent, records 1341–1368 | Gate scope correction and next lift dispatch | 16 parent calls | 829.133 observed s, including 728.914 s lift wait | Check changed meaning; native writer terminal, review pending | Separate gate scopes and preserve outstanding reviews above |
| 2026-09-26 parent, records 1984–2040 | Two post-trial ten-lane cohorts and throughput accounting | 10 lanes per cohort | Wait windows 28m46s and 21m33s; full intervals unknown | 8/11 then 8/10 exact reports; 3 then 2 partials; no independent accepted-throughput rate; build/index/count discrepancies remain | Record dispatch-to-last-terminal and review-accepted exacts/hour; instrument lock waits and race classes above |
| 2026-09-26 parent, records 2386–2638 | Five later wait-window cohorts, manifest outage, stale picks and duplicate writers | 10 lanes per reported batch; later reports overlap batch boundaries | 19m36s–39m11s waits; 13.8–27.6 exact-report/hour ceilings | Batch 32 child/parent disagreement; 6/10 batch-33 lanes failed on a missing manifest source; later 15 exact reports include only 6 first-seed results and one standby duplicate; no accepted-throughput rate | Bind dispatch/start/terminal/gate/review identities; instrument source-claim publication, stale picks, writer overlap, lock waits and retry causes |
| 2026-09-27 parent `01a0cbc4`, records 1764–2115 | Cycle 5 semantic review of a text/index implementation cohort | 32 bounded displays and 32 checkpoint entries; one duplicate retry rejected | 394.827 s active review; 688 s timer-to-verified-commit window | 450/450 unique blocks, 698,641 characters: 68.384 blocks/min and 106,169.2 chars/min active; 39.244 blocks/min and 60,928.0 chars/min broad. 171 tool-output, 154 reasoning and 125 assistant-text blocks; prior 400-block rate was 73.5/121.4k with different mix. Cycle 5 reached 5,056/5,000 (101.12%); no causal batch-size claim | [Coverage and throughput record](INDEX.md#full-history-ingestion-status); instrument group timing and compare matched cohorts below |
| 2026-09-27 M13 observation-ingest window | Screen the pinned lift-loop/RE child transcript and checkpoint its unique evidence blocks | 27 reader output calls, 4 DB checkpoints, 15 logical 32 KB cycles; 3 clipped full-cycle attempts recovered in bounded windows | 1,977.782 s first-read to final checkpoint | 380/380 blocks, 511,507 indexed chars and 742 origin pointers: **11.528 blocks/min**, **15,517.6 indexed chars/min**, **10,864.6 novel semantic chars/min** after verified prefix reuse. The largest successful one-call cycle was 75 blocks/31,938 chars. A 40 KB repack planned 13 cycles vs 15 but clipped at 65/38,783 chars and was not adopted. The underlying lift stayed partial with 0/1 exacts | [Coverage snapshot and measurement limits](INDEX.md#full-history-ingestion-status); [RE compiler-ceiling outcome](bof3-re.md#2026-09-27-m13-continuation-compiler-ceiling-with-no-exact-lift); keep character and response caps together; compare matched cycles before claiming a speedup |
| 2026-09-28 parent Cohort K `window320b` | One explicit `bof3-lift-loop` mission; five candidates queued | Native tool count unavailable | Skill duration unavailable; observation screening separately took 529 s wall / 1,006.18 summed reviewer s | One candidate independently accepted and committed by cohort cutoff (1/5); this is interim queue status, not completion or skill throughput | Add mission identity, native-call count, start/terminal times and per-candidate independent verdicts to receipts; preserve the screening/skill timing boundary |
| 2026-09-28 parent Cohort K `window400` | One explicit `bof3-lift-loop` mission; 161 partials in the observed denominator | Native tool count unavailable | Skill duration unavailable; observation screening separately took 502 s wall / 742.357 summed reviewer s | Three partials independently reviewed and committed by cutoff (3/161, 1.863%); this is a cutoff snapshot, not terminal success or skill throughput | Capture mission identity, native calls, dispatch/start/terminal times and candidate-level verdicts; report accepted candidates per mission-hour only from a complete lifecycle interval |

Add one row per mission. A row that changes nothing is itself a finding.

## Converged directives

The rules above are directives in table form; their owning procedure is
`.pi/skills/bof3-lift-loop/SKILL.md` and `references/`. Record the pointer here when one is folded
in.

## Bundled harness improvements

| Evidence | Proposed owner/change | Acceptance measurement |
| --- | --- | --- |
| Two parent wrappers inflated 13 naming children into 15 alleged runs | Observation ingestion and parent status summaries: retain workflow/child identity and classify terminal events directly | One row per child identity; wrappers excluded from child rates; durations alone never label a timeout |
| Five-child pilot completed with zero semantic changes; parent return contained `undefined` fields | Operator handoff/result references: propagate typed stage outcomes and artifact identities | Every stage reports completed/blocked/failed/no-op plus its evidence; completed transport never closes blocked domain tasks |
| Narrower serialized pilot differs in scope and checkpoint reuse | Benchmark reporting: compare equivalent task populations and report both elapsed workflow and summed child time | Every comparison names task count, role mix, prerequisites and reuse; no unsupported causal speedup claim |
| Batch 33 lost about 10–12 minutes to a partially published source claim; stale `next-lift` picks and same-target writers also wasted lanes | Lift-loop launch preflight and `next-lift`: publish source plus manifest claim atomically, filter live worktree claims, and enforce one active writer per selector | Zero manifest-dependent tool failures from an in-flight claim, zero duplicate selector assignments/writer overlaps, with strict manifest validation and all native gates retained |
| Later exact-report ceilings range 13.8–27.6/hour; six of 15 sampled exact reports were first-seed | Lift-loop references: capture measured shape attempts and classify first-seed, adjusted, standby, partial and blocked outcomes | Matched cohorts report full dispatch-to-terminal and independent-review exacts/hour, plus attempts/result and retries by failure class; no causal claim from wait ceilings |
| Batch 2's review sample found unsupported “BankByte4/effect-bank” naming after exact body gates; batch 3 separated 3 data-boundary reclassifications from 21 exact C lifts | Lift/review references: require source witnesses for role labels and explicit result classes for code lifts versus data classification | No unsupported semantic label in reviewed reports; every selector has one reconciled exact/partial/reclassified/blocked/failed disposition and all native gates remain green |
| Batch 4 had 5 failed statuses among 9 observed async runs, 4 resumable sessions and repeated >240s bash attention notices; batch 2/3 coverage commands returned rc=2 with usable JSON | Async supervision and coverage receipts: deduplicate notices by child/tool call, expose command/path/age/resume handle, and report command exit separately from artifact validity | Raw notices retained with a deduplicated actionable count; every child has a terminal/recoverable state; JSON artifact schema/status and CLI exit are both visible and reconciled |
| Three of six wave-2 lanes hit the 240s open-bash alert despite a 10-call cap; scratch under out/ caused CMake inventory failures | Lift-loop runner: enforce a command-level deadline and cancellation; require scratch/variants outside out/ | Every command has a recorded start/stop status within its cap; no CMake inventory failure from concurrent scratch writes; compare accepted task yield and retries on matched cohorts |
| Wave 1 review followed a 50m workflow timeout; wave 2 recorded 0/6 exact and wave 1 2/6 accepted exact, while full intervals are missing | Lift-loop run receipts: persist selector identity, attempt class, dispatch/start/terminal/gate/review timestamps and final independent verdict | For each cohort, reconcile unique dispatched selectors to terminal outcomes; report accepted exacts/hour, partial gains/hour, rework and blocked share, with missing intervals explicit |
| D6's alias reached one exact but a shared-map row surfaced cascading stale snapshot recipes | Symbol-map/index workflow: preflight snapshot blast radius and preserve both base/alias bindings without silently invalidating unrelated targets | Alias and base names both bind; sibling remains non-invalid; index passes with all affected snapshots accounted for; independent review accepts the change |
| The 450-block semantic-review sample measured 68.384 blocks/min and 106,169.2 chars/min active, versus 73.5/121.4k for a different 400-block mix; its 688-second broad window leaves 293.173 seconds beyond active review unallocated. A 30-second wrapper yield initially showed only 27/32 groups while detached checkpoint work continued; one duplicate retry was safely rejected. | Observation-ingest checkpoint tooling: emit per-group semantic-review and checkpoint start/end/ack times, unique block/character counts and exactly-once commit status; keep active and end-to-end rates distinct. | Compare matched 400/450/500-block cohorts with the same display cap and similar content mix; reconcile source hashes and every unique block, report checkpoint p50/p90 and rework, and show no loss or duplicate commits. Treat any rate change as descriptive until the quality denominator also holds. |
| Two explicit `bof3-lift-loop` mission snapshots show 1/5 and 3/161 independently accepted candidates at cutoff; both lack native tool count and duration. Their separate 320- and 400-block screens took 529 and 502 wall seconds | Lift-loop mission receipt/harness: record mission ID, dispatch/start/terminal times, native calls, candidate-level terminal outcomes and independent-review time | Reconcile each queue to accepted/rejected/deferred/active statuses; report accepted candidates per complete dispatch-to-terminal wall hour and native calls per accepted candidate only when lifecycle fields join; exclude screening time and label incomplete queues as cutoff snapshots |

These proposals were not implemented by the earlier naming audit. A later
harness optimization trial is recorded in [concurrent lift throughput](#concurrent-lift-throughput-and-harness-cost-center-trial); its causal performance effect remains unmeasured. No installed extension was changed.

## 2026-09-28 observation-screening throughput and lift-loop mission metrics

The pinned transcript `.pi/sessions/2026-08-03T20-37-30-546Z_019fc958-8932-7a72-a358-0f5d33895319.jsonl` (SHA-256 `29420b39507759c1ae447a0e28bd298869032b2682ab895388b03978f1d0d78c`) contains two explicit `bof3-lift-loop` missions across disjoint observation windows. In `window320b`, five candidates were queued and one independently reviewed candidate was committed by cutoff (**1/5**); its [part 2 receipt](../../tmp/observation-ingest/cycle6-k-window320b-20260928/part2-review.json) (SHA-256 `a222f1748861da3e2ff0fcc49ecae8cde47bdedf2e90093b3830866eb698788f`) pins the load/run at window indexes 184–185, denominator at 197, and acceptance/commit at 313, 317 and 319. In `window400`, **3/161** partials were independently reviewed and committed by cutoff (**1.863%**); its [part 2 receipt](../../tmp/observation-ingest/cycle6-k-window400-20260928/part2-review.json) (SHA-256 `16e36ca80f39abe2c782841e517b94de2232c2795660f89b79e8f98db1779149`) pins the explicit run at indexes 220, 257 and 259, queue denominator at 268 and 271, and accepted units at 308, 312, 313, 341, 343, 344, 374, 376 and 377. Both are cutoff snapshots, not terminal queue success rates. Native tool counts and skill durations are missing, so accepted work per skill-hour and calls per accepted candidate remain unavailable.

The 400-block observation screen reviewed **400 blocks / 159,277 indexed characters** in **502 seconds** of parallel wall time and **742.357 summed reviewer-seconds**: **47.809 blocks and 19,037.1 indexed characters per wall minute**, or **32.329 blocks and 12,873.4 characters per summed reviewer-minute**. Its mix was 297 tool-output, 52 reasoning, 48 assistant-text, one summary and two runtime-error blocks. Forty-one sensitive candidates were withheld. Eight display groups/calls completed with no oversized groups, clipping, omissions or rereads; all 400 hashes reconciled. Nine findings link to 68 blocks, while 332 have no durable finding. One part reported a +5 rendered-versus-indexed character delta of unresolved cause. The [selection](../../tmp/observation-ingest/cycle6-k-window400-20260928/selection.json), [pending receipt](../../tmp/observation-ingest/cycle6-k-window400-20260928-screened-pending.jsonl) (SHA-256 `998fa6c4e7efc8d0dfea9c2b2cdd8a33f67d1a0b2192dc9e671397f17dd9c15d`), [part 1 receipt](../../tmp/observation-ingest/cycle6-k-window400-20260928/part1-review.json) and [part 2 receipt](../../tmp/observation-ingest/cycle6-k-window400-20260928/part2-review.json) retain the screen evidence. These are screening rates, not lift-loop execution rates.

Across the three 320/320/400-block screens, pooled throughput is **35.135 blocks and 34,385.1 indexed characters per wall minute** over 1,776 wall-seconds, or **21.300 blocks and 20,845.6 indexed characters per summed reviewer-minute** over 2,929.537 reviewer-seconds. The windows differ substantially in mean block size and content mix, so keep block and character estimates side by side and treat the pooled values as planning evidence rather than causal batch-size improvement. Harness work should record mission lifecycle timestamps, native calls, queued candidate identities, cutoff status and independent verdict time; measure accepted work per dispatch-to-terminal hour only when those fields join, excluding screening time.

The subsequent 500- and 600-block follow-ups contain no explicit named `bof3-lift-loop` invocation, so they add no skill mission or execution-throughput sample. The 600-block window again ends during an investigation after session-level progress reaches **35/161**; it establishes neither a mission cutoff verdict nor an independently accepted outcome. Do not convert that queue context into skill yield.

The 600-block observation screen reviewed **600 blocks / 175,839 indexed characters** in **563 seconds of parallel wall time** and **813 summed reviewer-seconds**: **63.943 blocks and 18,739.5 indexed characters per wall minute**, or **44.280 blocks and 12,977.0 indexed characters per summed reviewer-minute**. Its mix was 538 tool-output, 33 reasoning and 29 assistant-text blocks. Forty-one sensitive candidates were withheld before selection, with eight previous sensitive hashes excluded. Eleven display groups/calls reconciled all block hashes and rendered characters, with zero clipping, omissions, rereads or oversized groups. Five candidate findings link to 50 blocks; 550 have no durable finding. The [selection](../../tmp/observation-ingest/cycle6-k-window600-20260928/selection.json), [part receipts](../../tmp/observation-ingest/cycle6-k-window600-20260928/part1-review.json) and [part2-review.json](../../tmp/observation-ingest/cycle6-k-window600-20260928/part2-review.json), and body-free [pending receipt](../../tmp/observation-ingest/cycle6-k-window600-20260928-screened-pending.jsonl) preserve the evidence. These rates measure observation screening only; no checkpoint or accepted skill result is claimed.

Across the five completed 320/320/400/500/600-block windows, pooled screening throughput is **43.160 blocks / 26,663.2 indexed characters per wall minute** over 2,975 seconds, or **28.702 blocks / 17,731.6 indexed characters per summed reviewer-minute** over 4,473.537 seconds. Keep these descriptive rates separate from native skill execution and from cycle coverage, which advances only after checkpointing. The rates vary with average block size and reasoning/finding mix, so they support planning and optimization comparisons but do not prove a causal batch-size gain. The separate screening record is in the [observation index](INDEX.md#full-history-ingestion-status).

The next **700-block** screen covered **222,172 indexed characters** across 13 complete capped groups. Of these, 636 blocks / 200,177 characters were timed over 323 seconds (**118.142 blocks/minute; 37,184.582 characters/minute**); the first 64 blocks / 21,995 characters were read outside that interval. One outer output was clipped and reread; all 700 hashes reconciled with no reader-level clipping or omissions. The validated disjoint [800-block selection](../../tmp/observation-ingest/cycle6-k-window800-20260928/selection.json) contains **391,160 characters** (489 per block versus 317 in K700) across 20 capped groups. Applying the timed rates to that actual selection projects **6.77 minutes by blocks** and **10.52 by characters**; the same-mix character scenario is 6.83 minutes. Keep these separate because the next selection is character-dense and the first 64 K700 blocks were untimed. The estimates exclude selection, synthesis and checkpoint work and may be optimistic. The [screening metrics](../../tmp/observation-ingest/cycle6-k-window700-20260928/screening-metrics.json), [part receipts](../../tmp/observation-ingest/cycle6-k-window700-20260928/part1-review.json) and body-free [pending receipt](../../tmp/observation-ingest/cycle6-k-window700-20260928-screened-pending.jsonl) retain the intake record.

### Delegated lift throughput and rework cost

Run-level metadata in the linked [agent metrics](../../tmp/observation-ingest/cycle6-k-window700-20260928/agent-run-metrics.json) records 12 target-lift workers over **26 candidate functions**: 20 reported exacts and six escalations in **17,657.692 summed worker-seconds / 765 calls**. Eleven reviewers recorded **23 verdicts** (20 pass, three needs-fix) in **1,099.068 summed seconds / 84 calls**. Selector matching confirms **18 of the 20 reported exacts**; two pass identities remain unresolved. This gives **3.670 reviewer-matched exacts per summed worker-hour** or **3.455 per combined worker-and-reviewer hour**. These are small-cohort delegated-work rates, not parent skill runtime or campaign wall throughput.

Two reverse missions timed out after **3,600.125 and 3,600.163 seconds**. Together they consumed **7,200.288 seconds and 218 calls** across three functions, produced no accepted exact, and used **40.777%** of target-lift worker time. The parent restored each best interim candidate. A separate cleanup run removed three unused macros in **108.141 seconds / seven calls**; follow-up review verified all three exact matches remained. The single explicit `bof3-lift-loop` invocation in the transcript has no parent duration or native call count, so direct skill throughput remains unavailable.

These measurements support three bounded improvements to evaluate: keep a stable target-qualified selector from dispatch through review so reported exacts join to accepted units; record parent invocation time and calls separately from child costs; and expose per-selector progress checkpoints before long worker deadlines while preserving parent rollback. The macro cleanup also supports a changed-macro zero-user check before review dispatch. Compare accepted exacts, rework, calls, worker and reviewer time, and timeout cost on matched cohorts before changing budgets or treating the measures as stable rates. The source-linked [K700 receipts](../../tmp/observation-ingest/cycle6-k-window700-20260928/part1-review.json) and [part 2 receipt](../../tmp/observation-ingest/cycle6-k-window700-20260928/part2-review.json) contain four findings; this screen has no checkpoint credit.

### 2026-09-28 K800 screen and delegated mission rates

The selected **800 unique hashes / 391,160 characters** were screened in a 347-second first-to-last interval (**138.329 blocks and 67,635.735 characters per minute**). Global ledger reconciliation found six already-reviewed hashes; 794 blocks / 390,483 characters earned new coverage (**137.291 blocks and 67,518.674 characters per minute**, 99.25% yield). Join selection against the global reviewed ledger before display; then compare duplicate-screen rate and net-new throughput on a later cohort. This sample does not measure saved time. At the gross screening rate, a same-mix 900-block window (about 440,055 characters) projects to **6.51 minutes**; 900 newly covered blocks project to **6.56 minutes** at the net-new rate. Both exclude selection, synthesis and checkpoint time. These single-window projections help compare intake-window sizes, not establish a larger-batch speedup. The average was 488.95 characters per block, 54.0% above K700. Descriptively, K800's gross block rate was 17.1% higher and character rate 81.9% higher than K700's timed remainder; K800 was denser and fully timed while K700's first 64 blocks were untimed, so this does not isolate a batch-size effect. One outer response clipped and two constituent groups were rerendered; reader-level clipping and omissions were zero. The pinned [screening metrics](../../tmp/observation-ingest/cycle6-k-window800-20260928/screening-metrics.json), [selection](../../tmp/observation-ingest/cycle6-k-window800-20260928/selection.json), part receipts, body-free [block receipt](../../tmp/observation-ingest/cycle6-k-window800-20260928-screened-pending.jsonl) and [checkpoint sidecar](../../tmp/observation-ingest/cycle6-k-window800-checkpoints.jsonl) retain the evidence.

The [delegated run metrics](../../tmp/observation-ingest/cycle6-k-window800-20260928/agent-run-metrics.json) join ten `bof3-reverse` workers over ten functions: six exact claims and four escalations in **15,019.854 summed seconds / 504 calls**. Five claims match a reviewer pass; the sixth exact claim was still awaiting parent/reviewer reconciliation at the selected-source cutoff. This is **1.438 reported exacts per summed worker-hour** and **1.159 reviewer-confirmed exacts per combined worker-and-reviewer hour**. Seven reviewer runs recorded seven passes in **510.542 seconds / 33 calls**; five cleanup runs took **427.142 seconds / 20 calls**, with two edits and three no-change audits. These are delegated workflow rates, not parent `bof3-lift-loop` runtime.

Three one-hour worker timeouts consumed **10,800.086 seconds / 269 calls**, with no exact result, or **71.905%** of worker time. The parent restored each candidate; a timed-out mission also left one compiler-flags residue that was found and restored separately. In this same selected cohort, **92/105** partial rows already had prior exhausted or non-shareable outcomes, leaving 13 fresh rows. Join the journal before dispatch and expose fresh/reopened/prior-outcome counts; record score checkpoints and verify the complete worker write set at rollback. One read-only review produced a valid pass report but a nonzero process status; a repo-owned completion-guard setting fixed the false failure. The complete five-finding analysis and run-level hashes are linked in the receipts. K800 is checkpointed: **794 new blocks add cycle coverage; six previously reviewed hashes add none**.

## Important observations

Four operational leads recovered from the complete inputs to the
[second loop/RE rewrite](bof3-docs.md#second-loop-and-re-editorial-review).
Those inputs prove what compaction discarded; original run identities and event
logs remain pending. Treat these reported measurements as investigation baselines,
not verified campaign rates. The former `dispatchSoundCue` and `drawTexturedFrame`
matching examples already remain in [RE observations](bof3-re.md#important-observations).

- Five compiler/profile searches reportedly failed before comparison because the
  new source was absent from `compile_commands.json`; a typemap failure also
  prevented a permuter probe. Count eligible comparisons separately from attempted
  launches: these failures cannot establish that matching alternatives were exhausted.
- A permuter report counted **27,355 errors/27,683 iterations (98.82%)**, with
  failure beginning after roughly 285 initial compilations. The remaining 328
  iterations are not established as valid comparisons. Recover failure onset and
  environment identity before diagnosing the cause; repeated invalid attempts
  cannot measure source-shape productivity.
- A legacy summary claimed races in **30/30 fan-out runs**, with **2–6 manual gate
  retries/run** under ten concurrent writers. Membership and per-run retries remain
  unverified. Recover those IDs before using the rate to evaluate serialization;
  measure gate recovery time and accepted outcomes independently of match scores.
- A sweep reportedly replaced an **86/89** baseline with an **84/90, 360-byte**
  candidate after other variants failed because baseline selection excluded the
  pre-sweep candidate. The report required restoring it. Preserve baseline identity,
  comparison eligibility and restoration evidence; different instruction denominators
  prevent reducing this to a two-instruction regression.

Proposed harness/reference measurements: valid comparisons/attempts, invalid-attempt
streak and time after first systemic failure, retry calls/time per run, and sweeps
retaining or improving the pinned baseline/N. Record total attempts across batches:
one recovered reasoning fragment counted about 19 variants over two sweeps while
considering another eight, exposing ambiguity between per-sweep and per-mission
budgets. That intention does not prove the additional sweep ran. No harness changes
or expanded tests are claimed here.

The [first RE input](bof3-docs.md#first-re-editorial-review-and-cohort-closure)
adds a reported execution lead: **five `bin/shape-sweep` calls, each at most eight
variants, totaling 36 measured variants** for `emi/world00/area030/04@0x801DA92C`
(native result 13, line 59). Original execution remains unverified. Unlike the
nineteen-plus-eight reasoning fragment, this claims completed measurements; neither
proves mission authorization. Proposed budget metric: total eligible variants and
cost per target mission across all batches, with exceptions tied to explicit
authorization. Per-call limits alone cannot establish compliance with a mission cap.

The [first rewrite audit](bof3-docs.md#first-loop-editorial-review) confirms that
the profile/permuter leads survived 461→35 compaction before the second rewrite
discarded them. Its input also recorded target-selection confusion: one report
tested `0x801E60DC` for a source tagged `0x801E5824`, then recognized the mismatch;
other fragments repeatedly inferred lane assignments from campaign numbering.
Original mission verification remains pending. Proposed handoff metric: measure
selector/source identity corrections and selection-rederivation calls per mission;
pin the actual target-qualified unit, not a lane number or remembered campaign.

## Autonomous lane workflow (batch-006)

Orchestration moved out of the session into `.pi/workflows/bof3-lift-lanes.js`, a
file-backed ref-driven workflow: it takes `lanes: [{key,target,refs:[DISC_ID#INDEX@0xADDRESS]}]`
plus `maxSelectors`/`repairRounds`, runs lift -> review-only-if-not-exact -> bounded
repair per lane, and isolates every lane failure so one timeout cannot reject the
batch. Validated with `subagent({action:"validate",workflowScriptPath:...})`, then
single-lane tested (`BIN/SCENARIO/SCENA04.EMI#0@0x801F8124` -> exact, gate PASS
twice, no reviewer spent), then run at 10 lanes: `{lanes:10, complete:10,
laneErrors:0}`. Measured: 1855 -> 1990 exact boundaries (17.71% -> 18.99%) across
batches 003-006; the serial scoped-build gate caught a real lane defect
(`shop00_symbols.c` truncated `WEAK_SYMBOL_AT` lines). Also applied: `configure()`
now reconfigures `build/cmake` in place instead of deleting it, so adding a source
no longer triggers a full object rebuild.
