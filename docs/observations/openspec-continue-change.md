# openspec-continue-change observations

Next-artifact authoring evidence belongs here under the
[measurement contract](INDEX.md#performance-measurement-contract). This ledger
is a coverage record while the source review is incomplete, not a zero-activity
or successful-performance claim.

## Coverage and measured performance

The 4,171 child metadata records contain no explicit selection of this skill.
Parent operations and generic children require separate attribution. The current
history scan found 56 files mentioning OpenSpec/opsx; such mentions include skill
catalogues, copied instructions and unrelated outputs, so they are not run counts.
Four contextual continuations, covering proposal, spec, design/tasks and a later
four-artifact capability plan, are
reviewed below; none explicitly selects/reads this skill. Population outcome rates,
cost distributions and accepted-artifact throughput remain unmeasured.

## Successor proposal: first-artifact continuation

[Parent source](../../.pi/sessions/2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4.jsonl),
records **1133–1138**, SHA-256
`2c4e084cd00b25aa453b20018463e58fd9ed3fee5c4d7c524d863aa6d838d791`.
All three reasoning blocks, two assistant text blocks, two results and both tool
arguments were reviewed, including the full emitted proposal. The user said
`begin draft` after the [scaffold handoff](openspec-new-change.md#successor-scaffold-after-naming-rescope).
Reasoning refers to the continue/new flow but does not read its skill. Attribute
this as contextual artifact-authoring evidence, not an explicit skill invocation
or a full `openspec-propose` delivery. Later artifacts remain outside this interval.

The phase used **2 calls / 2 results, 23.084 seconds**: one proposal write and one
validation/status wrapper. Usage: 9,146 input + 758,656 cache-read + 4,188 output
= **771,990 tokens**, including 2,060 reasoning tokens; reasoning text is 8,453
characters. One artifact moved to `done`; specs and design were both `ready`,
tasks still `blocked`. Accepted-artifact throughput is unmeasured. Reproduction:
`tmp/observation-ingest/successor_creation_metrics.py` and its JSON output.

Useful behavior: the draft retained the 75-finding starting inventory, named the
existing capability, required reviewed transactions and baseline preservation,
and explicitly refused invented data names. It authored only the requested first
artifact. However, the following claims exceeded the evidence:

- **Validation pass was not captured.** The wrapper kept only the final four
  validator lines, which instructed the operator to add delta requirements and
  scenarios. `PIPESTATUS` was read outside that pipeline's subshell, so the
  printed `exit=0` describes the wrapper outcome, not a reliably captured validator
  exit. The full verdict was discarded. Both reasoning and final text nevertheless
  called the proposal validated. Artifact `done` status means its file exists,
  not that the whole change passed validation.
- **The inherited contract was incomplete.** The preceding instruction response
  was clipped, and this phase fetched no replacement, read no full current main
  spec, and only listed three requirement headings after writing. Its assertion
  that freshness and reviewed-path requirements remain unchanged lacked a
  fresh body-level comparison. Later artifact generation must consume the full
  source requirements and preserve unrelated scenarios.
- **Scope still contradicted acceptance.** The proposal allowed data rows to
  remain explicit evidence ceilings but its Impact required zero findings and
  the gate to advance. Reporting partial work as partial is useful; it does not
  reconcile a terminal ceiling with that universal completion criterion. Preserve
  which rows remain open and the decision required to close or defer them.
- **Pilot findings became universal prerequisites.** The draft assigns proven
  containment to all 40 function rows, treats all 21 data rows as outside every
  candidate payload, and calls five serial missions “the same work” as nine
  parallel timeouts. The
  [pilot evidence](bof3-naming.md#serialized-pilot-parent-cost-and-limits-of-acceptance)
  supports narrower statements. A refusal to invent names is a guardrail, not
  proof of the final report's claim that the pilot caught an invented rename.
- **Dependency reporting remained wrong.** Status showed both specs and design
  ready, yet the report called specs the last planning blocker before tasks.
  The uncreated design obligation did not disappear. Derive next steps from the
  actual graph and distinguish readiness from completion.

Combined archive/create plus this draft cost **8 calls / 93.751 seconds**, including
the 26.997-second user-reply gap. That aggregate is recorded once by the shared
metric script; this ledger's two calls must not be added to it a second time.

## Successor spec, design and tasks: validation versus acceptance

Same parent/hash, records **1139–1182**: two `continue` requests, 19 reasoning
blocks (95,688 characters), 19 assistant text blocks, 23 results and all call
arguments reviewed, including every artifact version. Retained CLI clipping is
an evidence gap, not unread material recoverable from this transcript. Attribution
remains contextual; these are successive phases of one change, not independent
skill successes. Reproduction: `tmp/observation-ingest/successor_planning_metrics.py`
and `successor-planning-metrics.json`.

| Phase | Calls/results | Observed seconds | Outcome |
| --- | ---: | ---: | --- |
| Initial spec, 1139–1146 | 4/4 | 38.174 | Proposal acceptance corrected; first delta explicitly reported valid |
| Diagnosis/revisions, 1147–1168 | 11/11 | 95.794 | Two replacement drafts, one syntax failure, scenario-loss failure recovered |
| Entire spec continuation, 1139–1168 | 15/15 | 141.202 | Final delta has two added requirements/five scenarios and one removal |
| Design/tasks, 1169–1182 | 8/8 | 56.807 | Two artifacts plus one design edit; planning 4/4, implementation 0/17 |
| Combined | 23/23 | 204.896 | Includes 6.887-second user-reply gap; semantic acceptance unestablished |

Rows overlap; do not sum them. Combined recorded usage is 50,113 input +
5,263,104 cache-read + 36,242 output = **5,349,459 tokens**, including 21,800
reasoning tokens. Spec and design/tasks totals are 3,865,963 and 1,483,496.
These cache-heavy transcript totals are not billed cost. There were **three full
spec writes**, two temporary header mutations and two restorations. Eleven
validator invocations executed across the interval, including two deliberately
invalid-header probes; the syntax-failed wrapper's three planned validations
are excluded. Necessary recovery is included in cost, not presumed avoidable.

The author caught the proposal's zero-findings contradiction and correctly
reported planning complete with implementation still 0/17. Proper exit capture
eventually established the final schema pass. These useful corrections coexist
with measurable defects:

- **Presentation prompted costly diagnosis without proving a parser defect.**
  `show --json` exposed requirement bodies without names for both ADDED and
  REMOVED entries. The 11-call diagnosis/revision phase inspected that display,
  searched local examples, tried MODIFIED and returned to REMOVED/ADDED. The
  intermediate rewrite dropped four original scenarios; a full validator result
  later returned exit 1 and named them. A display omission alone never established
  that archive would misidentify the requirement. The whole phase's 95.794 seconds
  is observed cost; attributable wasted time is unknown.
- **Mis-captured status hid actionable failure.** Early wrappers read
  `PIPESTATUS` outside a subshell and printed zero after clipping the verdict.
  The initial spec has an explicit valid message, but the intermediate MODIFIED
  draft's pass was not established; later direct capture disproved it. One
  diagnostic command invoked the validator twice, once for output and once for
  filtering. Retain one complete result and derive all presentation from it.
- **Probe results need bounded claims and restoration evidence.** One bogus
  MODIFIED header produced an archive-refusal INFO message; one bogus REMOVED
  header returned exit 0 without that warning. This supports an observed
  asymmetry for these probes, not a universal statement about every CLI version.
  Commands restored the saved draft and final validation passed, but used no
  cleanup trap or full-content equality check. Exact old-header membership was
  confirmed; a future `totals.removed == 1` check cannot identify the removed body
  or prove unrelated requirements survived.
- **Guidance was fetched too late twice.** Design and tasks writes were issued
  in the same assistant messages as their instruction requests, before those
  responses arrived. Both responses clipped instructions; design's template was
  clipped too. The design rules were returned in full, yet only **6/7 decisions**
  supplied a rejected alternative, contrary to the report's 7/7 claim. Decision 6
  supplies none. The spec instruction call clipped output before template/rules;
  its required capability-list check was not observed in this interval.
- **Acceptance still disagrees across artifacts.** The final spec requires
  findings for exactly ceilinged naming rows and no others, while its bindings
  scenario and task 5.1 allow unresolved binding drift. The bindings requirement
  also universally requires canonical names. These are different outcome sets;
  a schema pass does not reconcile them. Group 1 permits blocked outcomes, but
  tasks 1.1/1.2 require every selected name adopted and its finding gone. Every
  exception must have one consistent disposition across prose and checks.
- **Evidence limits did not survive planning.** Design repeats the unproved
  “same work” comparison of nine timeouts with five serial missions, universal
  data containment claims, and a function-only explanation of empty owner
  results. It says all 14 drifts close only when their functions are named without
  establishing each generator's cause. The
  [pilot review](bof3-naming.md#serialized-pilot-parent-cost-and-limits-of-acceptance)
  preserves the narrower evidence. A baseline hash prefix is not a full baseline
  comparison; the output-write whitelist again omits registry generation by
  `just check`. These limitations were already visible before this handoff.
- **Checklists are not proof of executable task size.** All **17/17** checkboxes
  have a verification clause, but task 1.1 bundles 22 target transactions, and
  execution prerequisites appear after transaction groups. No observed estimate
  establishes one-session feasibility. Task 5.6 depends on a future archive:
  record that as a deferred archive obligation, rather than silently counting it
  as implementation verification. `git diff --check` passed but the authored
  OpenSpec tree was untracked, so that command did not check these artifacts.

The historical memory update and final report repeated planning-complete claims;
neither is an independent artifact review. No current memory correction follows
solely from that historical payload.

For comparable continuations, assess three proposed changes:

| Destination | Concrete change and baseline | Acceptance measurement |
| --- | --- | --- |
| Skill directive | Read complete guidance/dependencies before writes; check each applicable rule. Baseline: 2 writes before guidance, 6/7 alternatives despite a 7/7 claim. | All required reads precede dependent writes; every claimed rule has body-level evidence; report revisions per accepted artifact. |
| Reference material | Add examples for replacing requirements, retaining scenarios, and reconciling naming ceilings with binding findings. Baseline: 3 spec drafts, 4 lost scenarios in the intermediate draft, residual cross-artifact contradiction. | Independently reviewed outcome sets agree across proposal/spec/design/tasks; preserved and intentionally replaced scenarios are explicitly mapped. |
| CLI/result tooling | Return complete structured instructions and validator exits; separate schema validation from archive applicability and preview the exact merge. Baseline: 11 validator executions, 1 shell syntax failure, observed header-check asymmetry. | One retained result per needed validation, verified restore after probes, exact removed/retained bodies; compare calls/time per accepted artifact without dropping gates. |

These single-change measurements identify improvement candidates, not population
failure rates or demonstrated savings. Implementation and archive outcomes belong
to later records.

## Capability plan: schema pass before feasibility discovery

Same parent/hash, records **1227–1234**: the user said `continue` after authorizing
the capability follow-on and implementation. Two reasoning blocks, two assistant
texts, five results and every call argument were reviewed, including all four
authored artifacts. This is contextual continuation, with no fresh skill read;
four files written together do not establish an explicit fast-forward invocation.

The phase used **5 calls / 33.395 seconds**: four writes and one validation/status
wrapper. Recorded usage was 10,113 input + 745,600 cache-read + 6,321 output =
**762,034 tokens**, including 1,779 reasoning tokens (7,881 reasoning characters).
Reproduction: `tmp/observation-ingest/capability_launch_metrics.py` and
`capability-launch-metrics.json`. There were **zero pre-write instruction fetches**
or fresh implementation reads. Status reached 4/4 artifacts and **0/13 tasks**;
the complete validator verdict and directly captured exit established schema
validity. All five design decisions had rejected alternatives and all 13 tasks
had verification clauses. These are artifact-quality measures, not feature proof.

The plan usefully separated semantic naming judgment from mechanical admission,
required native evidence and stale/authored-evidence rejection, and made missing
producers explicit gaps. Three added requirements contain six scenarios. However:

- **Broad acceptance lacked a feasibility boundary.** The plan promised positive
  producers for every required rung and derived admission for every row kind,
  without inspecting decoder/control-flow coverage or the current evidence path.
  The [first worker handoff](openspec-apply-change.md#capability-implementation-launch-and-first-supervisor-decisions)
  immediately needed a narrower straight-line scope. Measure readiness by a
  representative end-to-end path and an explicit unsupported-case matrix.
- **Artifact consistency remained incomplete.** Design describes ten binding
  drifts while the proposal says 14, without defining a subset. “Keep existing
  gates green” conflicts with the known failing symbol-debt gate. A motivating
  row may remain blocked under task 3.1, despite its group's claim to prove the
  capability unblocks naming. Baseline failures and partial acceptance need the
  same stated outcome in every artifact.
- **Compatibility and obligation transfer were underspecified.** A no-receipt-
  format-change goal accompanies new typed evidence, without a compatibility
  plan. Task 4.4 says the prior change's withdrawn tasks 1–3 can resume, although
  those IDs now describe recording dispositions; no mapping restores the old
  implementation obligations. Name the exact transferred criteria and successor
  tasks, not only an earlier change or group number.
- **Checks did not prove preservation or review readiness.** A baseline hash
  prefix is not a full comparison. `git diff --check` excluded the untracked
  planning tree. The asserted harness review requirement was searched only after
  authorship; the next phase had not verified the reviewer's command capability.

Proposed **directive/reference**: consume current guidance and inspect one native
producer-to-admission path before defining universal acceptance; document coverage,
compatibility, baseline gate failures and transferred obligations. Baseline: four
artifacts, zero fresh guidance/implementation reads, followed by two supervisor
issues and three corrective artifact edits. Acceptance: independently reviewed
criteria agree across artifacts, every promised case has a feasible evidence path,
and open cases stay explicit. Proposed **planning tooling**: surface artifact
contradictions and distinguish schema validity from readiness. Compare revisions,
discovery calls and time per accepted plan on comparable changes; this five-call
phase alone cannot establish a speedup over the earlier 23-call continuation.

## Evidence to retain

The onboarding dependency outputs show specs and design follow proposal and tasks follows both; its recap misstated that relationship. Reuse the source finding, not the incorrect recap.
See the fully reviewed [onboarding case](openspec-onboard.md) for source locations
and the distinction between workflow phases and independent skill invocations.
Do not double-count that mission here.

## Performance questions

Measure: Dependency-ready artifacts accepted / authored artifacts; calls and time per artifact; revisions for missing context, unresolved design choices or wrong output paths; blocked versus completed steps.

For each episode retain source records, task scope, skill-selection evidence,
revision/context, outcome evidence and missing measurements. Compare like work;
completion of a CLI command or artifact cannot establish the broader result.

## Improvement proposals

- **Directive:** consume full artifact guidance and dependency bodies before
  authoring; report existence, validation and semantic acceptance separately.
  Baseline: one artifact authored from clipped guidance, one unsupported pass
  claim. Acceptance: every reported pass has the actual retained verdict/exit and
  each dependency has been read at the required scope.
- **Reference:** show how partial evidence, ceilings and deferred obligations map
  to a coherent completion criterion; preserve sampling limits in successor plans.
  Measure unresolved scope/acceptance contradictions and subsequent corrective
  revisions per independently reviewed artifact.
- **CLI result presentation:** retain exit/stdout/stderr before formatting, expose
  complete required instruction fields and render the actual artifact graph.
  Compare calls/time per accepted artifact and false-pass/dependency-error counts
  on equivalent continuations, retaining all required evidence checks.

These are proposed improvements, not implemented policy or authorization for
new tests, dependencies or installed-extension edits.
