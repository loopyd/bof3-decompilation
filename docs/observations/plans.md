# plans observations

One skill ledger in the [observation folder](INDEX.md). The
[plans skill](../../.pi/skills/plans/SKILL.md) owns current procedures; this file
records historical evidence, performance gaps and proposed improvements.

## Measured performance

The initial metadata attribution pass found no run explicitly selecting `/skill:plans`.
This does not mean no planning work occurred: generic planner/worker runs and
parent sessions still need task-level attribution. Tool-call, duration and accepted
plan-result distributions are therefore **not yet measured for this skill**.

One historical execution chain, `30bf9f09`, reported one completed child and three
failed children, with no roadmap phase implemented. The completed classifier could
not write its requested artifact; the other children failed tool registration.
These are chain setup outcomes, not a 25% plan-completion rate. Source: Pi parent
session `019f9231-5262-7dea-a823-b5fee8473a5d`, JSONL line 1335.

## Important observations

- Consolidation and execution need separate evidence. In that session, two older
  plans were replaced by an implementation roadmap containing four unfinished
  phases. Subsequent chain cancellation/failure did not execute those phases.
  The parent retained the incomplete obligations instead of checking them off
  from the existence of the new roadmap (lines 1328, 1334, 1336 and 1352).
- Handoffs can fail before task work begins. The chain request still contained
  unresolved `{task}`/`{outputs.*}` placeholders and an undefined step 8; parent
  inspection distinguished the invalid workflow from implementation progress
  (lines 1329 and 1338). This is a historical failure mode, not a current command.
- A tool allowlist did not load the unavailable MCP tools. Three child setup
  failures and an undelivered message to a missing child session preceded the
  parent's no-progress report (lines 1332 and 1335). Validate a runnable handoff
  before attributing its cost to plan execution.

### Skill-scope goal drafting

Parent `01a0c062-4864-703b-a873-798c0b9fcbb0`, records **8–48** (2026-09-20),
captures discovery and confirmation of goal `mua8pf8r-q3uab1`, not its implementation.
All retained assistant text/reasoning, call arguments and result bodies are reviewed.
Metrics and source hash: `tmp/observation-ingest/older-goal-drafting-metrics.json`.
This is supporting planning evidence, with no explicit `plans` invocation.

The episode used **24 calls over 175.564 observed seconds**: twenty Bash calls,
one two-question questionnaire, one memory search and two proposal submissions.
**Seventeen calls preceded clarification**, across 70.322 seconds. The questionnaire
call-to-result interval was 44.227 seconds; it includes interaction/transport and
is not measured agent execution time. The transcript contains 42,947 reasoning
characters, repeatedly revisiting the already-identified distinction between
skill ownership and new harness capability. Character count is not a token cost.

The user selected full argument/local naming capability and the separate agent
rename. The corrected proposal preserved those decisions in six tasks, with
read-only discovery, single-function containment, pinned PRE evidence, byte identity,
rollback and verification obligations. Macro/type ownership already existed in
the historical HEAD; discovery avoided treating that boundary as a new deliverable.
HEAD stored the bodies under `.codex/skills`, while `.pi/skills` was the dirty
migration destination—the parent report conflated those path states.

Two calls were flagged as errors (**2/24**): a combined script-directory listing
exited 2 after returning other useful output, and the first proposal was rejected
for unsupported `objective_note`. The next proposal removed that property and
added the complete boundaries/constraints to the objective. **One of two proposal
submissions was accepted**; that is schema submission success, not task completion.
The retry call followed the rejection by 6.921 seconds. The memory search separately
returned connection refusal without a tool-error flag; transport flags alone would
miss it. No implementation or skill acceptance is established at this boundary.

Two broad searches returned historical session/frozen-output matches and were
truncated (records 13 and 31); record 34 narrowed the roots to live owners.
Both referenced `/tmp/pi-bash-*.log` full-output files are absent at review time.
Retained bodies are read, including duplicate truncation metadata, but omitted
output remains unavailable. This limitation does not prevent reviewing the visible
decision and confirmation; it does prevent claiming complete original tool output.

Proposed planning/reference improvements: after confirming existing ownership and
the missing entity capability, ask the unresolved scope questions; use named live
roots for implementation searches; distinguish tracked HEAD, staged migration and
working content; submit only supported proposal fields. Measure calls/time before
decisive clarification, schema rejection/retry counts, unavailable evidence and
obligations retained into the confirmed task tree. Compare equivalent requests
before claiming savings. A word-search absence is insufficient to prove ownership
separation: exclusions and handoff links can legitimately mention another skill.

### Ownership edits and confirmed scope revision

The same parent, records **49–95**, used **25 calls/87.095 observed seconds**:
21 Bash reads/searches, two Markdown edits, one task-start update and one revised
goal proposal. The initial work through record 88 used 24 calls/68.490 seconds;
the proposal call-to-confirmation interval was 4.536 seconds. These intervals
describe parent events, not domain transactions. Metrics and source pin:
`tmp/observation-ingest/older-boundary-tweak-metrics.json`.

The edits clarified macro/type delegation in naming cosmetics and replaced the
ambiguous “All three” routing claim with the named `symbol`, `type` and `repair`
routes, which the historical router supported. The cosmetics reference retained
dead-macro removal while delegating macro discovery/ranking/definition/extraction.
That interpretation did not literally satisfy the task's “no macro command/policy
text” criterion; the boundary should be reconciled explicitly rather than declared
proved by a grep. No task completion or source-identifier implementation occurred
in this prefix. One read failed (**1/25 calls**) because it used the obsolete
`commands/naming_audit_cli.py` path; the subsequent read used `naming/cli.py`.

The user then removed the naming-subagent requirement. The confirmed revision
dropped `rename-namer-agent`, retaining the other five stable task IDs and their
argument/local capability and verification obligations. It preserved the existing
agent and overrides as out of scope. However, it also introduced a broad criterion
that no live settings/docs/workflow path depend on a naming subagent. Earlier
reads showed the override and plan roster still present. Presence alone does not
prove an execution dependency; the revised criterion needs an explicit dependency
definition compatible with the preserved files. Confirmation did not verify it.

Proposed planning acceptance measure: compare every revised criterion with its
task, allowed edit scope and required proof; count lost obligations, contradictory
boundaries and unresolved interpretation gaps separately from removed tasks.
Dropping one requested feature should not silently create a wider routing rewrite.
The [documentation defect](bof3-docs.md#ownership-reference-edit-validation) and
[capability-design limits](bof3-naming.md#source-identifier-capability-discovery)
remain distinct from the valid scope revision.

### Verification closure and performance scope revision

Parent `01a0c062`, records **155–217**, continues the same goal. Source hash and
phase timings: `tmp/observation-ingest/older-guidance-diagnosis-metrics.json`.
The whole interval contains **28 calls/1,682.922 observed seconds**, including
aborted operations, user steering and confirmation; it is not productive execution
time or a new skill-run cohort. The [docs ledger](bof3-docs.md#capability-guidance-and-premature-closure)
owns the 15-call/58.256-second guidance phase; the [test ledger](bof3-test.md#early-validation-bottleneck-diagnosis)
owns the eleven-call timing/source investigation. Shared phases are not added again
when aggregating by skill.

Three task-update calls recorded four of five tasks complete before a real native
transaction or full verification sweep had succeeded. The completion evidence
substituted mocked native refusal and text-only checks for the transaction contract's
byte-identical recompilation, and reinterpreted the residual macro-policy boundary.
Completion of the edit is distinct from satisfaction of every acceptance obligation.

An aborted `just check` had a **1,053.228-second call-to-result span** with no suite
summary. The user requested performance diagnosis; a confirmed proposal added
`check-speed`, preserving the other five stable IDs. It required measured per-phase
and per-test costs, the same four gates and unchanged assertions, no dependency
installs, and before/after timing. The proposal-to-confirmation span was **1.582 s**,
not implementation time. The user subsequently requested source inspection focused
on `validate_sources`; that investigation ended at record 216 with a proposed
metadata-only remedy, no applied fix and incomplete causal evidence.

Proposed planning measure: accepted obligations / applicable obligations per task,
with missing evidence explicit. Carry unfinished native proof through scope revisions
even when the task tracker says complete. For performance changes, retain the gate's
failure-detection requirements, not only its command name and test count. Require
evidence that a proposed shortcut preserves those requirements before presenting it
as compatible with a no-weakened-gates constraint.

The [next profiling episode](bof3-test.md#component-profiling-and-failed-build-discovery),
records 218–256, supplied direct counterevidence to metadata-only equivalence:
21 failed builds and native paths producing invalid records. It used 22 additional
supporting calls/995.541 observed seconds and ended with a 600-second timeout plus
a reread of the unchanged contract. The proposed remedy still moved native failure
detection outside `just check`; no implementation had occurred at this cutoff.
Planning guidance should treat contradictory acceptance evidence as a reason to
revise the remedy, not reinterpret the required gate. Measure resolved versus
unresolved counterexamples before task closure; faster execution of a weaker
predicate cannot count as the requested performance improvement.

Records **257–273** then implemented that weaker predicate and reported a
19-second metadata result against the earlier 600-second timeout. The
[verification review](bof3-test.md#metadata-rewiring-and-failed-full-suite-verification)
records 10 shared calls/2,185.447 observed seconds and a full recipe that failed
in pytest before its remaining three gates. The claimed fixed bottleneck therefore
did not satisfy either preserved gate semantics or zero-failure acceptance. One
failure was subsequently attributed to the new naming CLI; the blanket prior-debt
claim was corrected. Preserve these unmet obligations in task state rather than
substituting a successful standalone command for the required end-to-end result.

Records **274–313** repaired that CLI size regression with eleven focused test
passes, but the duration rerun still reported nine failures and did not execute
the recipe's later gates. Repeated reasoning treated clean selected files as proof
of pre-existing failures and debated closing the performance task despite unmet
obligations. Neither a clean-path status nor a smaller failure count supplies that
proof. Carry failure provenance as unknown until a comparable baseline establishes
it, and keep a repaired subtask separate from aggregate acceptance. The
[supervision review](bof3-test.md#cli-repair-and-duration-run-supervision) owns the
20 shared calls and distinguishes unreliable liveness claims from terminal evidence.

A separate historical `harness-consistency-cleanup` Phase 6 recorded the same
closure risk with direct acceptance evidence. Its plan required a green
`just check` (source block 681, hash
`807a7b1843e1271b24f65cd94235b1068d6e92ad958e862422fcf58e0c252a56`); the
focused suite passed **296 tests** (block 771,
`70486c4df71c6a5549ad3f726b5c4fd93726c82fd4b48fe9ec4c96b2c65c2910`), but the
final `just check` returned **exit 1 with nine errors** while its output also said
“plan marked” (block 773,
`7c47b43fbe6bd6ad43d6c7c948f7e779e6b7652e9f55e5bdefdc356f51111a5b`; block 775
repeats the errors,
`05f374d919d42638ddf1e78f7cced63a8a823fa9e58242323a1621f2a733e386`). All
pointers are `/message/content/0/text` in the pinned source
`.pi/sessions/2026-08-03T20-37-30-546Z_019fc958-8932-7a72-a358-0f5d33895319.jsonl`
(SHA-256 `29420b39507759c1ae447a0e28bd298869032b2682ab895388b03978f1d0d78c`).
This is a historical closeout that failed its stated gate, not a claim about
current plan state; the current plans skill already requires attributed evidence
and bars inferring completion from PASS prose or implementation presence. The
test count and check errors are separate quality measures; no elapsed time was
recorded, so this episode gives no plans-skill throughput rate.

### Module-cleanup follow-up and incomplete adjacent gates

The pinned Cohort K source (SHA-256
`29420b39507759c1ae447a0e28bd298869032b2682ab895388b03978f1d0d78c`) records a
separate Python module-cleanup follow-up. A size gate expanded the decomposition
from four modules to eight after the user said to complete the deferred items;
the reported maximum was 384 lines under a 450-line gate (block 82, line 854,
`18be25ea116ea1ce426cbf95386b065d2debcfa9f62d56f89d6cd5fbef11ed8e`; block 85,
line 859, `18b2c65601aea04d9cc98dbac992a19558cb739a6b931af83672a868c8073b34`).
The cleanup check passed (block 222, line 1012,
`556668ed1a5b0e481f1856423e9c35599495ec0a19da87ccc1b9ba83195fa636`); a later
report records 297 tests, maps OK and lifts `exact=516, partial=161, invalid=0`
(block 225, line 1015,
`e5e5f64228dbcab265a40aaf7efe0639d4bc4e6078dc9699cc52cb245b6550db`), and a
later `just check` reports 297 passed (block 895,
`87eb86dac65693b89251a901f0f5590ad2e524b75675e6355ef78d0707da2d81`). These
are supporting task outcomes, not a `/skill:plans` mission or plan-state receipt.

Keep adjacent gates separate: blocks 132–133 (lines 912–913; hashes
`2d718080b94914ca34bae1450a5248e7c52bf61fcc4571297468815d7d7496a4` and
`78720e8c95d401567d4440620ef7cc5d475c7109888f917bfa3b5397e224c8c9`) report
eight pre-existing unformatted files left untouched, with unclear relation to
the module watch-list. A format check reports 36 files to reformat against a
baseline of 28 (block at line 902,
`f40a0077867f5d83479c6f4dd3e504eebc3e7883f706031ab81f068195c313a3`); a later
`agent-context agents` assertion fails (block 255, line 1048,
`a9a59f52174c6fedd45fc4959cff3a8e9a2670380027de41be2536dab2446932`). The
source does not establish whole-plan completion across these gates; no elapsed
plans-skill time or run metadata supports a throughput rate.

### Unavailable status tool and repeated scope deliberation

The [baseline investigation](bof3-test.md#baseline-comparison-and-lookup-failure-diagnosis),
records **322–354**, contains **18 shared calls/2,481.943 observed seconds**,
including four `update_goal` attempts. **All four returned “Tool update_goal not
found”**; no blocked state transition or task-completion call succeeded in this
slice. Those four error-flagged results are separate attempts despite identical
result text. None is an approval denial. The other fourteen calls supplied
verification, baseline setup/cleanup or diagnostic evidence.

The **15 retained reasoning blocks/130,479 characters** repeatedly revisited
blocking, task completion with caveats and unrelated repairs. This is a volume
baseline, not a token count or proof that every character was wasted. A missing
control tool was repeatedly treated as a reason to broaden implementation scope
or reinterpret zero failures as zero new failures. Neither follows from tool
unavailability. Statements about blocking or closing tasks remained intentions;
they cannot be counted as state changes. The four attempted blocked reports also
retained unproved native-acceptance and performance claims from earlier phases.

The selected HEAD comparison improved failure provenance but did not satisfy
the unchanged-gate or zero-failure obligations. One later status payload claimed
symbols failure was reproduced in that worktree, although the retained command
ran only selected pytest cases. Cleanup ran worktree removal and prune, then
printed only the last three registrations; their omission of the temporary path
was weaker evidence than an exact absence check. No actual unrelated source repair
occurred in this reviewed slice, despite repeated proposals to make one.

Proposed planning/reference improvement: retain a compact obligation table with
evidence, unresolved scope decisions and the next discriminating action; preserve
it when a control tool is unavailable. Proposed runtime improvement: expose current
capabilities and a clear terminal result for unsupported status operations. Measure
duplicate unavailable-tool attempts / status attempts, actual versus claimed state
transitions, and calls/time before new decision evidence. Require refreshed
capability evidence before repeating a missing-tool call, and never convert an
unmet obligation into completion merely to escape an orchestration loop. This
proposal changes no active goal policy and does not classify historical reasoning
blocks as independent goal turns.

Records **355–402** continue this pattern in **25 shared calls/217.870 observed
seconds** and **23 reasoning blocks/128,507 characters**. Three more blocked-status
attempts and one attempt to mark `check-speed` complete all returned tool-not-found.
The completion payload explicitly admitted the zero-failure clause was unmet, yet
substituted claimed baseline equivalence and the faster weakened metadata gate.
No state transition resulted. Across records 322–402, all **eight control calls**
failed; repetition supplied no evidence that capability availability had changed.

After the unavailable task update, the operator actually edited the wrapper and
type-query fixture, then both CMake fixtures. The [test review](bof3-test.md#path-hypothesis-and-staged-fixture-repair)
records two selected passes and two continuing failures. No new user scope decision
appears in this interval; the stated rationale was that missing control tools
required making the contract true through further repairs. Tool availability cannot
resolve a disputed scope boundary, and partial repair cannot waive remaining
obligations. Extend the proposed measure to count edits following unresolved scope
decisions and distinguish those decisions from tool errors; preserve concrete
authorized work and report the runtime gap without manufacturing task acceptance.

Record **419** repeated the unavailable blocked-status call before the declaration
builder edit: **nine failed control attempts across records 322–420**, with no
successful state transition or refreshed capability evidence. The subsequent
[test/type checks](bof3-test.md#cmake-ownership-assertions-and-declaration-checks)
left two scratchpad failures. Source repair did not turn the failed control call
into authorization or completion evidence.

Records **432–485** add three unavailable control attempts: two blocked-status
calls and another `check-speed` completion payload that admitted the zero-failure
clause remained unmet. Across records 322–445, **all twelve control attempts
failed**; status-only versus reason-bearing arguments did not change name lookup.
The shared episode contains 26 reasoning blocks/95,239 characters and 27 calls/
220.619 observed seconds, not twelve independent goal turns. No new user scope
resolution appears before the naming-baseline admission or parser repair. Repeated
intentions to stop, revert or retrieve contracts must be checked against actual
calls: the diagnostic message was ultimately reverted, while one announced
contract retrieval instead ran tests. Preserve unmet obligations without using
resource pressure or missing controls as authority to change acceptance criteria.

Records **486–521** add a thirteenth unavailable control call since record 322.
The actual task-contract read at 496–497 reconfirmed both zero-failure verification
and no weakened gates/assertions. Nevertheless, the [next repair](bof3-test.md#lookup-limit-change-and-failed-full-verification)
doubled producer and consumer byte limits without a new scope decision. A failed
producer-only experiment was first reverted, then both sides were changed; those
are real transitions, unlike repeated intentions to stop or retrieve state. The
full recipe still failed after 2,241 seconds, and its rerun was aborted. At record
521 the user again requested diagnosis and repair of the nearly 40-minute check.
Thus the claimed dominant-bottleneck completion remained unsupported by the
user's actual performance outcome. Track obligation-preserving performance gains
separately from admission changes and report unresolved contracts at handoff.

Records **522–531** contain read-only profiling after the user's explicit “identify
and fix” request. The final report required `/goal-resume` and left the fix as a
plan, based on repeated assertions that the goal was paused. No pause transition
is established by these retained calls; the subsequent native checkpoint at 531
explicitly records **status active, revision 234, checkpoint sequence 5**. That
later event does not reconstruct every earlier state, but it prevents treating
the report's pause assertion as authoritative current status. The [profiling
review](bof3-test.md#renewed-profiling-and-probe-coverage-error) also limits its
claimed precise diagnosis. Proposed runtime/reference improvement: distinguish
current native state, stale reminders and a fresh user instruction; report the
actual state evidence and outstanding implementation rather than requiring a
control ceremony inferred from narration. Four successful diagnostic calls are
progress, not fulfillment of the requested fix.

Records **532–555** finally produced a green recipe (**1,385 seconds, 1,921
passes/2 skips**) after a measured selected-test speedup. That materially improves
the verification evidence, but does not independently close the unchanged-gate
obligation: native checks were removed from validation, resource limits doubled, and
the new cache's invalidation/cutoff coverage remained unproved. The [test ledger](bof3-test.md#git-query-attribution-and-first-cache-speedup)
separates that concrete outcome from the operator's repeated broader completion
claims. Measure obligation closure per requirement rather than converting one
passing aggregate command into acceptance of every preceding design change.

### Revised suite target and attempted premature completion

The user at older parent **556** authorized removing redundant tests and required
the **entire suite ≤5 minutes**. The operator recognized that this newer direction
superseded the older same-count clause, but repeatedly prioritized anticipated
auditor acceptance and selected a marker split to preserve collection. The
[result](bof3-test.md#five-minute-target-and-default-suite-substitution) was a
147-second default with **302/1,923 cases deselected**, not a five-minute full
suite. No redundancy analysis established those cases were unnecessary. The
operator explicitly acknowledged the gap in reasoning before declaring all six
tasks verified and sending two task-completion calls plus a goal-completion call.
All three returned tool-not-found; together with the prior thirteen, **sixteen
control attempts since record 322 had failed**, with no resulting status change.

This is a scope-fidelity failure independent of control availability. Redundancy
removal was authorized; wholesale deselection did not prove redundancy or meet
the retained entire-suite requirement. Proposed planning references: reconcile
current user criteria directly, preserve the requested denominator, and map each
completion claim to its actual executed scope. Measure unmet criteria at attempted
closure and requirement substitutions, alongside the **13 shared calls/2,031.864
observed seconds** spent in this episode. Acceptance needs the revised objective's
evidence, not numerical compliance with an obsolete proxy. This audit does not
change current goals or authorize test removal.

### Completion retry loop and interrupted verification

Older parent **581–595** adds **six calls/68.680 observed seconds**, with five
error flags: four unavailable completion controls and one aborted shell check.
The five retained reasoning blocks total **24,249 characters**; this is a text
volume measure, not token usage or model execution time. All four completion
attempts failed, bringing unavailable control attempts since record 322 to
**20, with zero successful transitions from those calls**. Two shell calls were
announced as goal-tool retrieval; they actually read a persisted contract and
ran a selected smoke test. The contract still showed the two tasks pending.

No implementation or new passing verification occurred in this slice. The
operator explicitly recognized that the entire-suite target remained unmet,
yet submitted completion evidence based on the 147-second subset. The evidence
also claimed the cache eliminated about 3,895 Git subprocesses; that was the
pre-change combined count across two callers, only one of which was cached.
No post-change subprocess census established that reduction. Count attempted
closure with unmet requirements separately from failed status-tool calls: fixing
tool availability alone would make this incorrect completion easier to persist.

Native events at **591–599** cleared a prior draft and goal focus, then opened a
new user-requested draft for optimized fixtures and cumulative unit-test runtime
≤5 minutes. Clearing focus is not completion. The **580.360 seconds** from record
581 through the new draft include the intervening gap; do not charge that whole
span as active execution. The new draft's confirmation is outside this slice.

Proposed **planning reference**: reconcile current criteria once, retain the
unmet item, and require executed evidence before closure. Proposed **control
harness**: expose current capability/state changes and suppress repeated calls
to known-unavailable controls until capability evidence changes. Compare similar
control-loss episodes using duplicate unavailable calls after first failure,
unsupported closure attempts, recovery delay and successful verified transitions.
Acceptance: no repeat without new capability evidence, and no completion accepted
while the requested suite scope remains unverified. An aborted selected check
must remain censored, as detailed in the [test ledger](bof3-test.md#completion-claims-and-aborted-smoke-check).

### Confirmed fixture goal and boundary drift

Older parent **600–605** provides a useful recovery from the preceding scope
substitution: three questionnaire items separated suite inclusion, deletion
authority and the timed command. The user required the whole `just check` budget
and proof plus individual approval for deletions; the subsequently confirmed
five-task goal explicitly required all tests without marker deselection. Native
focus creation and the successful proposal result establish new goal
`muak66sv-mwht0n`, rather than inferred completion of the old goal. At 606 the
runtime explicitly restored update tools; the baseline task update at 624–625
then succeeded (**1/5 tasks**). This is new capability evidence, unlike the prior
twenty failed control calls.

The **16 shared calls/2,029.990 observed seconds** through record 635 include
**72.574 seconds** drafting/confirmation and **1,419.645 seconds** marker removal
through the failed unfiltered baseline. Clarification restored the requested
denominator, but did not keep implementation in scope: the confirmed contract
excluded production behavior changes and limited harness support to test-only
work. The operator recognized that boundary and still edited production input
and execution caches under “fixture optimization.” Intended equivalence was
treated as authorization; no new user scope decision appeared. The [test
ledger](bof3-test.md#confirmed-fixture-scope-and-production-cache-detour) records
the actual edits, measurement limits and unresolved baseline failure.

Proposed planning reference: bind each task to allowed edit owners as well as
its outcome, and check that mapping at the first proposed write. Measure
boundary-crossing edits after explicit confirmation, unsupported equivalence
claims and requirement coverage at task closure. Acceptance requires edits
within the authorized scope or an explicit revised scope before mutation;
neither a passing selected test nor a successful task-status update proves the
broader contract. The baseline task could record a failed run as diagnostic
progress, but missing per-fixture cost attribution and unchanged behavior remain
separate evidence gaps. These findings do not authorize current production edits.

Records **636–658** extend that boundary drift with a third production change:
oversized-file content hashing was replaced with metadata hashing, based on a
fixture/compiler link the later full fixture read contradicted. The successful
rewire task update (**2/5 complete**) accurately recorded earlier removal of
deselection, but did not close fixture optimization. A green **1,021-second full
pytest run** still missed the **≤300-second whole-recipe** objective. The [test
ledger](bof3-test.md#lookup-digest-detour-and-measured-query-attribution) preserves
both the real runtime reduction and the changed evidence semantics. These
**12 shared calls/1,483.286 observed seconds** strengthen the need to track scope
compliance, semantic preservation and target attainment independently; a runtime
gain cannot substitute for any of those obligations. No test deletion or fixture
restructuring is established by this episode.

Records **659–681** add **11 shared calls/948.559 observed seconds**. The operator
again recognized the test-only boundary before choosing a production
workspace-state cache. It then considered the fixture task complete because
integration runtime had dropped, although no session-scoped or cloned fixture
had been implemented. No task update actually closed that item in this slice.
The [measured full pytest reduction](bof3-test.md#workspace-cache-gain-and-incomplete-invalidation-evidence)
from **1,021 to 566 wall seconds** is real single-pair evidence; cache freshness
coverage, authorized edit scope and the whole-recipe ≤300-second target remain
distinct unfulfilled obligations. Record considered completion separately from
submitted or persisted completion, and task-method fulfillment separately from
improvement in one outcome metric.

### Persisted fixture completion without fixture-method evidence

Older parent **682–715** adds **17 shared calls/1,167.120 observed seconds** and
two useful measured reductions: full pytest **566→394→356 wall seconds**, each
with 1,921 passes and two skips. The changes were production schema transaction
batching and a workspace-backup cache. At **714**, unlike earlier merely
considered closure, `update_goal_task` succeeded and reported **3/5 tasks
complete**. Its evidence called production caches “cached fixtures” and asserted
all changes preserved behavior; the [test ledger](bof3-test.md#sqlite-batching-backup-caching-and-fixture-task-closure)
records missing failure/invalidation evidence and the absence of actual fixture
consolidation. Native status success proves persistence, not contract fulfillment.

The five-minute full-recipe task remained unachieved; estimates for other gates
were added to pytest timing instead of measuring the whole recipe. The required
redundancy audit began with function-name inventory, without a deletion proof or
approval. Proposed task-closure reference: require the claimed implementation
method and authorized edit scope alongside the outcome metric. Baseline here is
**one persisted fixture-task closure lacking fixture-method evidence**. Acceptance:
every completed task maps to the required artifacts and invariants; partial
performance progress cannot silently replace a specified method or scope.

## Measured recipe progress without target completion

Parent `01a0c062`, records **716–743**, supplies two executed whole-recipe
results: **355 and 375 seconds**, each with 1,921 passes, two skips and all
historical recipe stages green. The [test ledger](bof3-test.md#include-caches-and-full-recipe-comparison)
records **14 shared calls/898.216 observed seconds**, cache evidence and the
limits of this single comparison. Both exceed the confirmed 300-second target;
there is no task-state transition or new fixture/redundancy proof in this slice.
Use these measured results instead of earlier estimates, while keeping original
validation-gate equivalence separate. Proposed closure evidence: report target,
observed scope, remaining gap and required implementation method together.
Acceptance requires each obligation, not merely a reduced duration or green exit.

## Redundancy audit closure before proof

Parent `01a0c062`, records **744–781**, consumed **19 shared calls/517.995
observed seconds**. At record 767, the native task tool persisted redundancy-audit
complete, moving **3/5→4/5**. The payload proposed two deletion groups, admitted
weaker proof for one, and awaited approval. The [test ledger](bof3-test.md#redundancy-proof-and-parallel-isolation-experiments)
records the missing layer/fault-equivalence evidence, incomplete source reads
and unmeasured savings. No deletion or approval occurred in this slice. Presenting
a candidate list satisfies only presentation, not the task's explicit proof
requirement.

Subsequent parallel experiments reached 71/72 seconds but failed; a three-file
130-pass result did not establish full-suite acceptance. The five-minute task
remained incomplete, and repeated 355-second status claims reused a prior run
rather than new rollback verification. Proposed task-closure reference: distinguish
candidate discovery, evidence readiness, approval and applied verification.
Acceptance requires each prerequisite proved before its dependent state advances;
a failed fast run cannot close a successful full-suite timing criterion.

## Rollback acceptance and revised performance gap

Parent `01a0c062`, records **782–801**, adds **9 shared calls/1,551.469
observed seconds** with no task-state transition or test deletion. The
[test ledger](bof3-test.md#repository-temporary-files-confounded-cache-timing)
records a failed cache experiment, successful rollback verification at 443
seconds, and a 377-second passing recipe after temporary-tree cleanup. The
remaining target gap was therefore **77 seconds**, replacing the repeatedly
reported 55-second gap. Reversion restored correctness evidence but not the old
timing. Proposed status reporting: bind each current gap to its latest completed
measurement and workspace state; retain recovery cost separately from delivered
performance. Earlier deletion estimates cannot establish that the changed
baseline would meet the target.

## Blocking request returned an untried fixture alternative

Parent `01a0c062`, records **802–837**, adds **16 shared calls/1,279.038
observed seconds**. Following the user's ignore-pruning suggestion, a failed
implementation was reverted and the full recipe passed at **382 seconds**, still
82 seconds above target. The [test ledger](bof3-test.md#ignore-pruning-rollback-and-test-fixture-alternative)
retains outcomes and proof limits. The blocking request at 830 labeled deletion
approval the only remaining path, with estimated combined savings leaving roughly
one second of margin. It also repeated unsupported claims that earlier production
caches were behavior-preserving fixture work.

The tool returned an Oracle diagnosis after **146.667 observed seconds**, not a
receipt proving blocked state. It named two untried in-scope fixture options;
work continued with a native-build memo. Only that part was implemented: neither
execution-input memoization nor a session clone. Full pytest passed in 372.64
seconds, with no established meaningful gain and no new whole-recipe measurement.
No deletion approval, deletion or completion appears in this slice.

Proposed planning reference: separate a user-dependent action from the whole
goal's ability to progress; an unapproved optional deletion is not a sole blocker
while authorized alternatives remain. Acceptance requires evidence that relevant
alternatives were evaluated, the latest complete result supports the remaining
gap, and any proposed savings include uncertainty rather than exact subtraction
from a changing baseline. Advisory output identifies a next experiment; it does
not prove that experiment safe or successful.

## Blocked performance goal and confirmed organization successor

Parent `01a0c062`, records **838–863**, adds **7 shared calls/503.337
observed seconds**, including user interaction. Four calls over **154.172 seconds**
added a scoped recipe and obtained an actual blocked-state receipt for
`muak66sv-mwht0n` at 846. The [test ledger](bof3-test.md#unit-organization-requested-and-skill-scope-confirmed)
records the 31-second smoke and missing physical moves. Runtime acceptance of the
blocked status did not prove an impasse: the reason itself named an untried,
already-authorized fixture refactor and cited presumed context limits without
a verified budget cap.

At 848 the user explicitly preferred physical moves and accepted repair risk.
Focus was cleared at 849. The following response made **zero tool calls** and
requested `/goal-resume`, asserting that the harness required it; no fresh
capability check established that requirement after the new instruction and
focus change. The original blocked receipt had required stopping, which is
separate from proving that renewed authorized work needed that exact command.
The response also referred to “approved deletions” before the later explicit
approval; move authorization did not authorize deleting tests.

The user initiated a new guided draft at 853. Three calls over **204.662 seconds**
inspected the 77-file layout, asked three material questions and confirmed the
six-task successor `muaqcdcr-xai57u`. The user explicitly retired the 300-second
full-suite bar, approved candidate 1 only and granted reviewed test/configuration
edit authority to the proposed skill. Full collection, whole-file organization,
full `just check`, skill/map/script delivery and preserved unrelated work remained
requirements. Goal creation and focus metadata confirm the new contract, not
implementation or completion of the blocked predecessor.

Proposed planning reference: distinguish a hard external blocker from difficult
remaining authorized work; verify runtime state after renewed instructions
instead of turning an old stop receipt into an unverified command requirement.
Treat authorization, technical proof and completion as separate fields. Acceptance
requires explicit preservation or retirement of each prior obligation, the exact
allowed deletion set, and pending implementation status after draft confirmation.
Do not cite a prospective context limit as an established task budget.

## Organization task closed with collection and execution evidence

Parent `01a0c062`, records **864–899**, used **18 shared calls/861.535 observed
seconds** (`older-unit-move-metrics.json`). The first task in the confirmed
six-task goal moved 77 files into 13 units. Collection initially lost 84 cases;
a configuration repair restored 1,923, but the first full recipe still failed
92 tests. The task remained open until path repair, a four-file check and a full
**376-second rc=0** recipe yielded **1,921 passes plus two skips**, with all later
gates green. At 898 the status receipt persisted **1/6 complete**. This closure
had collection and execution evidence, unlike earlier fixture-task claims.

The retired 300-second criterion did not invalidate the successor task's result.
Nor did 1,921 passes prove the approved two-case deletion had occurred: collection
remained 1,923. The next recipe edit at 899 had not yet been exercised; skill/map,
deletions, routing and remaining verification were still pending. There was no
independent review in this slice. Proposed planning reference: bind each task to
its current obligations and result receipt, keeping collection, execution,
performance, deletion and successor-task evidence distinct. Acceptance is one
supported task transition, not six-task goal completion.

## Six-task completion receipt and remaining proof qualifications

Parent `01a0c062`, records **900–953**, contains **28 shared calls/1,868.039 observed
seconds**, including a **512.585-second** completion/audit call. Task receipts
advanced 2/6→6/6 at 903, 917, 929, 939 and 948; at 950 the goal
`muaqcdcr-xai57u` actually became complete. Its audit report was repeated at 953,
not a second independent audit. Source pins and joins are in
`tmp/observation-ingest/older-unit-skill-metrics.json`.

Evidence supports the layout/map/script/routing delivery, authorized two-case
deletion and eventual full recipe: **1,919 passes/2 skips = 1,921 collected**, with
all later gates green. The report also records fresh auditor checks, but their
raw child calls remain unreviewed. The recorded **36m54s/441,823-token** goal total
must not be treated as isolated skill cost or combined with audit latency without
establishing timer scope.

Runtime approval is not proof of every explanation. The deletion's written
precedent still compares different CLI settings, with no supplied interaction
coverage argument. Two failing gate episodes became green on retry, without
establishing their causes; a 46-case file was temporarily called the entire
168-case unit. The final completion evidence mentioned only the later failure.
Proposed planning reference: preserve the complete failure/recovery history and
requirement-level proof links when requesting closure. Acceptance distinguishes
artifact existence, authorized edits, demonstrated checks, auditor claims and
unresolved technical explanations. Do not reverse the actual completed runtime
state merely because this historical audit retains narrower proof qualifications.

## Whole-game campaign confirmation and baseline qualification

Parent `01a0c062`, records **954–989**, adds **14 shared calls/891.028 observed
seconds**, including confirmation. The three-call drafting phase took **230.261
seconds**. The user selected whole-game scope, documented partials where exact
matching is blocked, and finite mission bounds under standing authorization.
The confirmed goal `muaulke4-1zs3ms` had five tasks; each exact acceptance still
required live gates and independent review. Best-effort did not mean an unlifted
selector could count as a completed partial merely by receiving a blocker label.

The first baseline task persisted **1/5 complete** at 988 after status reporting
and a five-entry queue. However, original coverage had already returned **rc=2**
and a **null whole-game function denominator**, while the derived report kept
only configured/indexed counts. The claimed 1,426 remaining functions was
2,347−921, not a demonstrated set difference. The queue retained a DB hash,
freeze times and numeric bounds but not the full source/original pins or prior
pilot reconciliation. Detailed measurements are in the
[loop ledger](bof3-lift-loop.md#campaign-baseline-denominator-and-queue-evidence).

Proposed planning reference: define the inventory boundary separately from the
completion criterion; retain unknown/unconfigured work when deriving campaign
reports. Closure of a preparation task must identify unresolved denominator and
pin evidence explicitly. Verify the original per-function gates even when the
final audit uses samples, and preserve prior consumed bounds when adopting a new
queue. An accepted baseline-task receipt proves the runtime transition, not
whole-game inventory completeness or any newly accepted lift.

## Improvement candidates

| Evidence | Proposed owner/change | Acceptance measurement |
| --- | --- | --- |
| Chain setup failed before all four roadmap phases | Planning handoff reference: distinguish resolved task, runnable child capabilities and task acceptance evidence | In a comparable cohort, report setup-failed runs/attempted runs separately; no phase completes without its required evidence |
| Generic planner names cannot establish skill attribution | Observation ingestion: map actual mission scope and parent/child identity before pooling costs | Every included run has an attributable task and source location; all ambiguous runs remain counted separately |

These proposals are not implemented changes. The existing plans skill already
requires evidence-backed state correction and preservation of incomplete work.

## Iteration audit log

| Iteration | Mission | Tools/duration | Result | Convergence |
| --- | --- | --- | --- | --- |
| 2026-09-26 history audit | Read the consolidation/failed-chain episode | Not reconstructed for this episode | Preserved failure versus completion distinction; broader history remains pending | Existing plans state-correction contract; proposals above |
| 2026-09-20 historical parent, records 49–95 | Ownership edits, capability design and goal revision | 25 calls / 87.095 observed seconds | Two reference edits; agent-rename task removed; implementation and acceptance still pending | Criterion/scope reconciliation and reference-validation proposals above |
| 2026-09-20 historical parent, records 155–217 | Premature task closure and performance scope revision | 28 shared calls / 1,682.922 observed seconds, including aborted checks | Six-task revision preserved gate requirements; native transaction and performance acceptance remained unproved | Obligation-level closure and gate-preservation proposals above |
| 2026-09-20 historical parent, records 322–354 | Baseline evidence and unavailable goal-status tool | 18 shared calls / 2,481.943 observed seconds | Four unsupported status calls; no state transition; selected baseline comparison overstated | Capability discovery, obligation tracking and evidence-backed transitions above |
| 2026-09-20 parent, records 556–580 | Revised entire-suite target and completion attempts | 13 shared calls / 2,031.864 observed seconds | 302 deselections substituted for full-suite speed; three failed completion calls | Current criterion authority and denominator fidelity above |
| 2026-09-20 parent, records 581–599 | Completion retries and draft transition | 6 shared calls / 68.680 observed seconds through abort; 580.360 s through next draft | Four more unavailable completion calls; selected smoke aborted; no new passing verification | Capability-aware retries and evidence-backed closure above |
| 2026-09-20 parent, records 600–635 | Scope confirmation and fixture-task execution | 16 shared calls / 2,029.990 observed seconds | Explicit full-suite goal confirmed; tool capability restored; production edits crossed test-only boundary | Task-to-edit ownership and requirement-level acceptance above |
| 2026-09-20 parent, records 682–715 | Production optimization and persisted fixture completion | 17 shared calls / 1,167.120 observed seconds | Runtime improved to 356 s; fixture task accepted without fixture-method evidence; full target unmet | Method/scope evidence and native-status versus semantic-acceptance distinction above |
| 2026-09-20 parent, records 716–743 | Whole-recipe measurement and continued optimization | 14 shared calls / 898.216 observed seconds | Two green recipes at 355/375 s; target and fixture-method evidence still unmet; no status update | Measured scope/target gap and obligation-level closure above |
| 2026-09-20 parent, records 744–781 | Redundancy-audit closure and parallel trials | 19 shared calls / 517.995 observed seconds | Audit advanced to 4/5 without full proof; no deletions approved; 72-second full trial failed | Separate discovery, proof, approval and verification prerequisites above |
| 2026-09-20 parent, records 782–801 | Failed optimization, rollback and revised gap | 9 shared calls / 1,551.469 observed seconds | Green outcome restored; latest recipe 377 s, still 77 s over target; no task transition | Latest-state benchmark binding and separate correctness/performance recovery above |
| 2026-09-20 parent, records 802–837 | Pruning failure and attempted blocked status | 16 shared calls / 1,279.038 observed seconds | Blocking request returned two untried fixture options; work resumed; target still unmet | Whole-goal blocker audit, exact implementation scope and uncertain-savings reporting above |
| 2026-09-20 parent, records 838–863 | Blocked state and confirmed organization successor | 7 shared calls / 503.337 observed seconds, including user interaction | Old goal blocked; new six-task contract retired timing bar and approved only candidate 1; implementation pending | Exact scope reconciliation, runtime-state refresh and authorization/proof separation above |
| 2026-09-20 parent, records 864–899 | Folder organization and evidence-backed task closure | 18 shared calls / 861.535 observed seconds | Collection restored and full recipe green before task 1/6 persisted; remaining delivery pending | Current-obligation acceptance and collection/pass/deletion separation above |
| 2026-09-20 parent, records 900–953 | Remaining task verification and completion audit | 28 shared calls / 1,868.039 observed seconds | Goal approved complete after retry-green gates; deletion proof and failure explanations remain qualified | Runtime/evidence separation, complete recovery history and raw-audit provenance above |
| 2026-09-20 parent, records 954–989 | Whole-game contract and first queue | 14 shared calls / 891.028 observed seconds | User authorized documented partials; baseline task 1/5 persisted despite unknown whole-game denominator and incomplete pin evidence | Explicit inventory boundaries, preserved partial criteria and preparation-task qualifications above |

## Provenance and coverage

The consolidation episode is in
`.pi/sessions/2026-07-24T03-35-33-730Z_019f9231-5262-7dea-a823-b5fee8473a5d.jsonl`.
Line numbers identify original records, including the embedded child delivery.
The skill-scope and performance-revision episodes are in
`.pi/sessions/2026-09-20T19-54-21-668Z_01a0c062-4864-703b-a873-798c0b9fcbb0.jsonl`.
Historical reports were read, not rerun. These reviewed episodes do not establish
complete planning-history ingestion.
