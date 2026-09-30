# openspec-update-change observations

Planning revision evidence follows the
[measurement contract](INDEX.md#performance-measurement-contract). Full-history
review remains incomplete; artifact validity does not establish semantic acceptance.

## Coverage and measured performance

The 4,171 child metadata records contain no explicit selection of this skill.
Parent operations and generic children require separate attribution. The current
history scan found 56 files mentioning OpenSpec/opsx; such mentions include skill
catalogues, copied instructions and unrelated outputs, so they are not run counts.
One explicitly selected revision episode and one contextual successor rescope are
fully reviewed below. Population outcome rates,
cost distributions and independently accepted revision throughput remain unmeasured.

## Approved naming rescope: artifact completion versus evidence quality

Source: [parent session](../../.pi/sessions/2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4.jsonl),
records **1076–1097**, SHA-256
`2c4e084cd00b25aa453b20018463e58fd9ed3fee5c4d7c524d863aa6d838d791`.
All assistant text/reasoning, tool results and revision arguments were reviewed.
The user selected rescoping the evidence-refresh change, then approved the four
proposed revisions with `go`. Record 1077 explicitly selects this skill and 1078
returns its contemporary instructions. This is one parent revision episode,
not four independent artifact missions. Later verification and follow-on creation
are outside this interval. Reproduction: `tmp/observation-ingest/update_rescope_metrics.py`
and its `update-rescope-metrics.json` output.

| Phase / measure | Observed result and denominator |
| --- | --- |
| Draft and scope discovery, 1076–1082 | 3 calls / 33.993 s |
| Approved revision and reporting, 1083–1097 | 10 calls / 54.095 s |
| Whole episode | **13 calls / 114.506 s**, including **26.418 s** between draft and user approval; 13/13 matched results |
| Recorded token usage | 45,850 input + 1,369,984 cache-read + 18,660 output = **1,434,494**; 10,932 reasoning tokens are included in output, not added again |
| Context / deliberation | 7 reasoning blocks / 45,191 characters; 6 assistant-text and 13 tool-output blocks; one search batch followed by four exact reads |
| Artifact application | 4/4 selected artifacts updated; two whole rewrites, two targeted edits, then one proposal typo correction; no failed write result |
| Required rewrite guidance | **0/2** whole-rewritten artifacts fetched `openspec instructions` before replacement, despite the returned skill's explicit requirement |
| Task accounting | **4/19 → 8/13**: ten naming/binding tasks replaced by four checked disposition tasks; five integration tasks retained, partly reworded, and still unchecked |
| Spec accounting | **5 → 3 requirements**, **11 → 10 scenarios**; debt-free/binding acceptance deferred, freshness and reviewed-identity constraints retained |
| Returned checks | Strict OpenSpec validation explicitly says valid; `git diff --check` reports clean; status says **in-progress**. No independent semantic review in this interval |

The four agreed artifact locations were resolved from status before writing, and
the user approved the proposed scope before mutation. Task counts were reported
with their new denominator. Those are useful controls. However, **4/4 written
does not mean 4/4 reconciled correctly**, and four newly checked documentation
tasks are not four completed naming decisions. Zero renames occur in this episode.

The search batch returned selected sections, then the operator reread proposal,
spec and tasks in full but only 20 design lines. Its claim to have read the change
in full therefore overstates this interval's fresh coverage. The skill required
rules/templates for substantial rewrites; both complete replacements skipped that
step. Four writes were submitted together although the historical procedure said
one artifact at a time. The aggregate `go` authorized all four; this is a workflow
ordering observation, not evidence of an unauthorized edit. The extra confirmation
followed the historical skill wording; its 26.418 s delay is observed, not a proven
recoverable saving under current instructions.

Several evidence defects became acceptance text rather than remaining caveated
pilot findings:

- “Group 1 kept verbatim” is false: task 1.4 retained its old text but gained a
  router-refusal/canonical-account rule. That addition conflicts with the pilot
  review's successful router validity check. The review located the refusal at
  preparation, not at that router check.
- The **1004 blocked** total is report-row occurrences across 26 reports, not 1004
  distinct debt identities. The new specification freezes this incidental count
  into a scenario without that denominator or a freshness boundary.
- One function-owner control and sampled data queries became universal per-kind
  statements; task 2.2 asserts empty data results prove the query is function-only.
  Task 2.3 also promotes report-file presence into full target coverage. Neither
  conclusion was established by the reviewed parent evidence.
- Task 2.4 calls the serial pilot “the same work” as the failed parallel campaign
  and attributes all nine timeouts to lock queueing. The workloads differ, and
  lock-wait duration/causal share was not measured. **9/9 timed out versus 5/5
  completed** are valid terminal counts, not a controlled performance comparison.

See the [pilot evidence limits](bof3-naming.md#serialized-pilot-parent-cost-and-limits-of-acceptance)
and [campaign supervision](bof3-naming.md#contention-supervision-timeouts-and-stopped-workflow)
for the underlying measurements; do not double-count their child costs here.

Preserving Decisions 1–8 did preserve history, but the revision did not label or
reconcile their stale generalizations. The nine symbol decisions and four generated
binding files were deferred in prose; **no follow-on artifact was created in this
interval**, so obligation transfer was pending. The earlier detailed tasks remain
recoverable from the transcript, not from a completed destination checklist.

The final report says “complete and closed” while also listing five unchecked
verification tasks. CLI status explicitly says in-progress. A 16-hex-character
baseline prefix and unchanged dirty-file category counts cannot prove byte-level
preservation or absence of other writes. Moreover, the printed validation exit
uses `PIPESTATUS` outside the pipeline's subshell: the explicit “valid” result
supports schema validity, but that printed zero is not a reliable independent
capture of the validator's exit. Semantic correctness and no-mutation claims need
their own evidence.

## Successor capability rescope and observation-ledger creation

Same parent/hash, records **1200–1226**. The user authorized both proposed next
steps—record the capability gap and create its harness follow-on—and requested
skill improvement using memory and attempt history. All nine reasoning blocks
(51,600 characters), nine assistant text blocks, 17 results and every write/edit
argument were reviewed. No update/new/docs skill was selected in this interval;
the naming skill was read for editing. These are contextual operations within one
mixed request, not four independent skill successes. Reproduction:
`tmp/observation-ingest/observations_origin_metrics.py` and
`observations-origin-metrics.json`.

| Phase | Calls/results | Observed seconds | Result |
| --- | ---: | ---: | --- |
| Memory and follow-on scaffold, 1200–1207 | 5/5 | 18.726 | Two searches, two page reads, one native change creation |
| Rescope and naming-skill read, 1208–1215 | 6/6 | 8.866 | Four artifact edits, validation/status, one full skill read |
| Observation authoring/checks, 1216–1226 | 6/6 | 51.119 | Ledger and pointer written; router loaded; reference-check invocation failed |
| Whole request | 17/17 | 118.806 | Partial requested work; no rename or capability implementation |

Phase spans omit between-phase gaps, including generation before the first write;
8.866 seconds is not the entire cost of rescoping. Recorded usage is 49,947 input,
3,190,272 cache-read and 21,835 output = **3,262,054 tokens**, including 12,303
reasoning tokens. Four memory responses returned 36,694 characters. No performance
benefit from that retrieval or the resulting skill edit was measured.

Task accounting changed **0/17 → 8/12**: implementation tasks were withdrawn and
eight documentation tasks authored checked; four verification tasks remained open.
The delta changed **two added requirements/five scenarios → one added requirement/
four scenarios**, retaining the old requirement's removal. Native strict validation
explicitly returned exit 0; status remained in-progress. This is supported schema
validity, not eight newly completed naming operations or independent acceptance.

The authorized change was concrete, but its evidence transfer remained incomplete:

- No fresh read of the four affected artifacts or substantial-rewrite instruction
  fetch occurred before the four writes. Recent authorship supplied context but
  did not establish a fresh reconciliation. Only goals/non-goals and two appended
  decisions changed in the design: Decisions 1–7 still describe adoption, binding
  regeneration and the old ceiling-based completion criterion without explicit
  supersession labels. The rescope therefore left conflicting current instructions.
- The new spec and Decision 8 require gate citations by file **and line**, but
  the four gate entries provide **0/4 line-number citations**. Function names aid
  navigation; they do not meet that expressly chosen criterion. Task 1.1 was
  checked despite the same missing line citation.
- Checked task 1.4 requires the named tests rerun. The preceding parent batch
  issued a test command but did not return its result; this interval runs no tests.
  Task 1.3 repeats universal producer/capability claims beyond the parent snippets.
  Task 1.5 requires exact commands but records `<input>` and an ellipsis rather
  than complete invocations. Evidence from the child remains attested pending
  its full transcript review; checking these boxes did not strengthen it.
- Task 2.1 preserves the measured 75/61/14 totals but infers every drift depends
  on the capability gap while acknowledging symbols outside the nine-symbol debt
  set. Task 2.2 claims no map/manifest writes from the refused import without a
  fresh scope comparison. The [first-row audit](bof3-naming.md#successor-first-row-attempt-capability-diagnosis-before-scaling)
  distinguishes what was actually established. Repeating those claims in a spec
  did not resolve their evidence limits.
- The proposal says resolution/binding requirements move to a capable successor;
  the final report calls them withdrawn rather than deferred. The follow-on is
  only a scaffold with no tasks. Preserve the ultimate naming obligations and
  their acceptance criteria in an explicit transfer map; creation alone does not
  transfer them. The baseline task still uses a hash prefix, and archive-time
  `removed == 1` still lacks exact removed/retained-body comparison.

The [naming ledger](bof3-naming.md#origin-of-the-naming-observation-ledger) owns the
skill-edit evidence and validation limits; the
[new-change ledger](openspec-new-change.md#capability-follow-on-scaffold) owns the
creation handoff. Their measurements share this 17-call request and are not
additional cost.

Proposed **revision directive/reference**: label superseded decisions, map every
withdrawn obligation to its destination, and attach criterion-specific evidence
before checking replacement tasks. Baseline: 0/17→8/12 administrative transition,
0/4 required line citations, retained obsolete decisions and a scaffold-only
handoff. Acceptance: every checked criterion is supported at its stated scope;
every prior obligation is retained, explicitly retired or transferred with its
acceptance text. Proposed **review tooling**: show old/new task and requirement
bodies with evidence pointers and unresolved destinations alongside schema status.
Measure correction calls and unsupported checkmarks per comparable rescope;
neither faster authoring nor a smaller denominator proves improved performance.

## Evidence to retain

During onboarding a clean-tree task was rewritten after pre-existing dirty work was discovered. That is an embedded task revision, not proof of this skill's invocation or complete artifact reconciliation.
See the fully reviewed [onboarding case](openspec-onboard.md) for source locations
and the distinction between workflow phases and independent skill invocations.
Do not double-count that mission here.

## Improvement proposals and acceptance measurements

These are proposals, not implemented directives or permission to change tooling.

| Destination | Source-linked change | Acceptance measurement |
| --- | --- | --- |
| Skill directive: revision preflight | Require current whole-artifact reads and substantial-rewrite instructions before drafting; reconcile inherited decisions rather than treating untouched text as current truth. Baseline: 3/4 whole-artifact rereads and 0/2 required rewrite-instruction fetches. | On comparable rescopings, 100% of affected artifacts/read dependencies and substantial rewrites have current guidance; report calls/time separately from schema validation. |
| Reference: rescope and evidence transfer | Add an example mapping old task/requirement IDs to retained, revised, retired or deferred obligations, including destination ownership, evidence scope and pending verification. Baseline: ten replaced implementation tasks, prose deferral, no destination created here. | Every old obligation has an explicit disposition and recoverable acceptance criteria; checked documentation work is never counted as completed implementation. Track unsupported statements promoted into requirements and corrections after review. |
| Reference: measurement and reporting | Distinguish report occurrences from unique rows, observed samples from universal rules, and changed task denominators from progress. Preserve negative and unknown evidence. | Reports agree with CLI task state and body-level comparison; zero unsupported “verbatim,” “closed,” causal-speedup or no-mutation claims in independently reviewed revisions. |
| Tooling: reviewable revision receipt | Assess an OpenSpec receipt showing changed requirement/task IDs, body differences, pending obligations and direct command exits; semantic claims still need their source evidence. Baseline: schema-valid artifacts retained the defects above and required one typo correction. | Measure discovery/rework calls and elapsed time on equivalent four-artifact revisions while retaining all evidence checks; demonstrate detection of unreported task-body edits and unresolved verification, without claiming the receipt proves domain semantics. |

Track requested decisions incorporated / agreed decisions and independently
accepted artifacts / reviewed artifacts alongside these costs. This one episode
supports specific failure hypotheses; it cannot establish population error rates
or expected time savings. No new tests, dependencies or installed-extension edits
are implied.
