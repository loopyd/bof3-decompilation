# openspec-propose observations

Historical evidence for improving proposal directives, references and tools.
The [measurement contract](INDEX.md#performance-measurement-contract) governs
counts. These findings propose improvements; they do not implement policy.

## Coverage and provenance

Reviewed all assistant text, reasoning and tool outputs in records **157–257** of
`.pi/sessions/2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4.jsonl`.
The expanded `/opsx-propose` request at 157 establishes ownership of one program
with four proposal deliveries, not four independently invoked skill runs.
Original clipped tool outputs remain limited evidence. Historical commands were
read, not executed. Reproducible metrics/hash:
`tmp/observation-ingest/propose_metrics.py` and `propose-metrics.json`;
`propose-repair-metrics.json` retains the first delivery's finer phase breakdown.

The second expanded proposal request, records 753–771, is reviewed below. Implicit guard-fix
and registry-repair proposals at records 445–461 and 661–696, plus the approved
evidence-refresh split at 853–867, are reviewed below;
downstream implementation is covered in the apply ledger. The 4,171 child metadata records have no explicit selection of
this skill; that does not exclude parent activity. The 56 files mentioning
OpenSpec/opsx include catalogues and copied instructions, not 56 missions.
The [onboarding mission](openspec-onboard.md) is counted under its own owner.

## Measured performance

| Phase / records | Calls with matching results | Observed seconds |
| --- | --- | --- |
| Repair, with shared program discovery, 157–195 | 19 | 290.659 |
| Retirement scope and proposal, 196–228 | 17 | 173.112 |
| Ladder simplification proposal, 229–244 | 9 | 69.257 |
| Emulator layering proposal, 245–257 | 7 | 58.045 |
| Whole program, 157–257 | **52** | **680.917** |

The whole span includes **212.582 seconds between assistant terminal records and
seven user replies**, including one reply after an aborted response. Phase spans
also include internal user waits. These are transcript spans, not active CPU/model
time; the remaining 468.335 seconds are not a CPU estimate either. All 52 call IDs
match results; nested shell commands are not additional calls. Shared discovery
cost cannot be attributed solely to the first change. N=1 program provides no
population distribution or comparative speedup.

Recorded `ds-combo` usage totals 626,128 input, 7,932,416 cache-read and 89,230
output tokens: **8,647,774**, including repeated cached context. The separately
reported 57,003 reasoning tokens are not added again. Zero monetary fields do
not prove free execution. Review covered 125 block occurrences: 39 reasoning,
33 assistant text, 52 tool outputs and one runtime error; one identical write
receipt occurs twice, giving 124 distinct blocks in this episode.

Delivery: **16/16 artifacts** existed across four changes and strict validation
reported valid with zero issues (256). There were 17 artifact writes, including
one replacement after a creation race. The task lists held **44 unchecked tasks**
(7 repair, 13 retirement, 12 ladder, 12 emulator). No implementation was claimed;
independent semantic acceptance is **unmeasured**. Current-change artifact
instruction reads before authoring were **0/16**; only the first change's proposal
instructions were fetched afterward, with content reduced to field counts.

## Iteration audit log

| Case | Outcome and method | Limits / destination |
| --- | --- | --- |
| `repair-docs-drift`, 157–195 | Four artifacts delivered in 19 calls; useful ownership discovery, two clarification replies before creation | Strict validation missed semantic/scope problems; proposals below |
| `retire-docs-transport`, 196–228 | Four artifacts in 17 calls; narrowed scope; explicit surviving dependencies and checker retirement order | One aborted response and one wrapper syntax failure; semantic issues below |
| `simplify-skill-ladders`, 229–244 | Four artifacts in 9 calls; one creation failure and proposal rewrite | Race recovered, but keyword/line-count checks were weak proxies for meaning |
| `streamline-psx-emulator-skill`, 245–257 | Four artifacts in 7 calls; creation serialized after earlier race | No retrieval benchmark; unsupported script/test split and preservation assumptions |

## Durable lessons

- **Structure questions so short answers have one meaning.** Five numbered
  questions contained another numbered change list and unnumbered alternatives
  (167). The reply `1 2` required another interpretation exchange; `A` then
  permitted the first change (168–177). Later `3A` was interpreted despite no
  matching explicit label (199). Count clarification cycles and unresolved
  decisions separately; do not count every clarification as avoidable delay.
  The user later explicitly corrected the inferred ladder scope to simplify
  existing ladders first (229), before that proposal was authored.
- **Follow the artifact's current instructions before drafting.** The expanded
  workflow required per-artifact instructions and fresh dependency reads (157).
  Four writes occurred together (188); only afterward did the operator fetch
  proposal instructions, printing merely context length and rule count (193–194).
  The recap's claim that injected rules constrained generation was unsupported.
  A remembered schema and file-existence status cannot replace that read.
- **Schema validity does not establish semantic consistency.** The written spec
  required every reference to resolve to an existing *file*, while its split-claim
  scenario and task 1.1 explicitly permitted an owning *directory* (188). Its
  universal wording also lacked a local-reference qualification. Strict validation
  still returned valid. Check requirement/scenario/task agreement independently.
- **Separate a plausible owner from proof of a claim.** The MIDI retarget was
  supported by module purpose, five tests and the exact 96-PPQN fixture (181–183).
  The stopped-tone mapping was weaker: a zero-step tuning rejection was promoted
  into ownership of a paragraph about pitch/reassignment, silence and held-sample
  edit rejection (183–188). A function name or module header does not prove those
  broader behaviors, nor prove that an old test file was split into those owners.
  Retain unresolved clauses for focused inspection instead of asserting lineage.
- **Preserve untracked content with content evidence.** Before drafting, the
  operator correctly found three untracked edit targets and one modified tracked
  target (187). Task 3.2 nevertheless proposed Git diff/status as proof that only
  intended lines changed (188). Those checks do not expose untracked content
  changes; a starting snapshot is needed for the claimed preservation result.
- **Capture findings once, normalize presentation separately.** The symbol check
  was executed twice, the second time only to count the first result (172, 174).
  Splitting its text on semicolons produced 89 entries; the initial `error:` prefix
  put one binding/map finding into `other`. Correct buckets are **14 binding/map,
  14 unnormalized maps, 40 raw functions, 21 raw data**, not the reported 13 binding
  findings with the remainder omitted. This is a reporting-parser defect, not
  an extra source defect. No runtime saving is measured for avoiding the rerun.
- **Keep policy scope and authorization distinct.** A comment specifically about
  text-tool gates was quoted as a rule for the whole removal decision (170–175).
  Later, three unfocused goals were treated as a reason to re-request approval
  despite recognizing that they were not automatically binding on this task
  (202–204). Historical goal boundaries are context; check their applicability
  and current authorization before making them a new blocking requirement.
- **Visibility is not enforcement.** The proposed `check-debt` alternative moved
  checks out of the routine gate while claiming no gate was dropped (175, 204).
  That changes enforcement even if a manual detector survives. A failing symbol
  check was observed; removing it did not prove the rest of `just check` would
  pass. Report observed component status, proposed coverage changes and whole-gate
  acceptance separately. These alternatives were proposed, not executed here.
- **Bound retrieval to the ownership question.** The initial nine-command batch
  omitted its OpenSpec-state section from returned windows, prompting another
  root check; its suppressed-stderr reference probe produced a JSON parse failure
  (159–163). The later eight-command batch indexed 55.2 KB, including a 45.4-KB
  caller section dominated by historical goal text (200). Repeated query windows
  are one source result, not additional failures or independent corroboration.
  The next seven-command batch indexed 712.2 KB, including 702.3 KB of callers
  dominated by session metadata (207). Its import-graph shell loop failed because
  the wrapper prefixed an environment assignment directly before `for`; a focused
  native read recovered the graph (209). This is one wrapper failure, not a defect
  in the inspected harness modules.
- **Inspect relative imports and all command-registration owners.** The focused
  read found `context/bof3_cleanup.py` importing `..docs.paths`, missed by the
  earlier absolute-name search (209). The retained graph separates five transport
  modules (692 lines) from four survivor modules (802 lines). Bootstrap test
  inspection then revealed a third command owner, `common/commands.py`, alongside
  registry and test policy (218–221). LOC measures removal scope, not skill speed.
  The initializer test constrained imports, not a package's public API (212).
- **Serialize creation and dependent writes.** Parallel `new change` and proposal
  writing caused `already exists` at 233, with no `.openspec.yaml`. Inspection
  found only the just-written proposal; the operator removed and re-scaffolded
  it, then rewrote four artifacts (236–241). The next change created its scaffold
  before writing (249–250), avoiding this race. A name collision alone would not
  justify deleting existing work; the limited directory evidence matters.
- **Make cross-change requirements executable and coherent.** Retirement correctly
  preserved source gates and documented loss of automated reference checking;
  it required repair verification before deleting `docs refs` (223). Final advice
  also put ladder/emulator link checks before retirement and identified the shared
  `tool-usage.md` edit (257). However, retirement task 1.1 treated absence from the
  active list or checked boxes as proof of verification. Its design said checker
  loss was in proposal impact, but that earlier proposal omitted it. Reconcile
  all artifacts after later discoveries; retain actual prerequisite check evidence.
- **Measure meaning and retrieval, not just text shape.** Ladder tasks required
  fewer lines and fewer `bin/harness` occurrences, while token presence was called
  proof that prohibitions survived (237, 244). Keywords can survive a reversal of
  meaning; useful examples can disappear while counts improve. Moving a rule to
  one owner also conflicts with requiring its token in every original file.
  Compare each decision, condition, exception and evidence obligation with its
  prior owner; then measure reader calls/tokens and decision accuracy on comparable
  missions. A justified no-op must remain possible for already concise guidance.
- **Verify the problem and baseline before prescribing structure.** The emulator
  entrypoint had 23 rows, 21 marked `2.`, but already instructed reading only the
  common contract and selected action (249). The plan's claim that agents could
  not load selectively was therefore unsupported. Inventory proved 107 Lua files
  overall, not the asserted split of 52 scripts/55 tests (247–250). The full-catalog
  baseline also included Runtime and API guidance, so requiring a script for every
  row conflated documentation with executable actions. Task 2.3 demanded equal
  reference sets while task 1.3 added a catalog; preserve the original set plus
  explicit additions/moves instead of requiring contradictory equality.
- **File names and status are incomplete preservation evidence.** Retaining
  reference basenames does not preserve inbound links when paths move (250–257).
  Ladder/emulator tasks demanded clean Git status for untouched directories despite
  known preexisting changes; the final five-line status sample did not prove which
  writes occurred (256). Use before/after content and path baselines, distinguish
  existing failures, and inspect each affected relative link. The known failing
  naming check also makes an unconditional `just check` pass an unresolved
  prerequisite, not a documentation-only acceptance fact.

## Follow-up proposal: text-count guard

Records 445–461 of the same parent contain explicit user approval for a new
`fix-text-payload-claim-guard` proposal alongside acceptance of the ladder change.
All six reasoning blocks, five assistant text blocks, ten tool results and four
written artifact bodies were reviewed. This is an implicit proposal delivery,
not another expanded `/opsx-propose` request. Its **10 calls / 10 results** and
**70.473-second span** include the ladder task acceptance edit: shared workflow
cost, not pure proposal cost. Recorded tokens total 2,857,651 (21,120 input,
2,823,680 cache-read, 12,851 output); 6,958 reasoning tokens are not added again.
Metrics: `tmp/observation-ingest/ladder-followup-metrics.json`.

Four artifacts delivered ten unchecked tasks; strict validation passed. Creation
preceded writes, avoiding the earlier race. Current per-artifact instruction reads
were again **0/4**: record 454 explicitly skipped them to save turns. The proposal
preserved scope boundaries and offered evidence-dependent document-versus-guard
remedies, but semantic acceptance remains unmeasured.

- **Keep hypotheses out of established problem statements.** The actual failure
  concerned “32 heuristic instances.” Help text established an unfiltered scanner
  mode; no probe reproduced 32. The proposal nevertheless called the claim truthful
  and the guard result a false positive. Later tasks appropriately required
  reproduction or a clearly labeled historical measurement. Reconcile that
  uncertainty into the proposal/spec instead of asserting the preferred diagnosis.
- **Read implementation assertions literally.** The design said the retired-value
  list could never shrink; the inspected test required length at least ten while
  the current list contained fourteen. That assertion permits some shrinkage.
  The proposed no-shrink requirement may be legitimate, but was not existing
  enforcement. Likewise, the tasks called the target “both tests” although the
  observed target contained three, including the 102.71-second reproduction work.
- **Validate every behavior the plan promises to preserve.** Requirements covered
  nonzero candidate-run and heuristic-instance claims, but the negative probe
  task named only the candidate form plus a retired number. Its literal `N`
  placeholder also needed an actual nonzero number to match the inspected parser.
  Map each preserved behavior to an existing check or concrete reversible probe;
  do not add tests without authorization. Prefer applicable existing test filters
  for rapid guard iteration, then the required full gate; savings are unmeasured.
- **Avoid making phrasing a substitute for evidence.** Replacing “instances” with
  “candidates” could evade a three-word pattern without correcting a misleading
  claim. The plan's alternative remedies and evidence task help, but accepting
  any explicitly unfiltered comparison still needs provenance for its number.
  Measure accurate claim classification and retained rejection behavior, not
  merely whether a reworded line passes.

Proposed directive/reference changes: consume current artifact instructions,
separate observed defect from diagnosis, reconcile task counts with actual targets,
and map promised guard behavior to concrete existing checks. A tooling improvement
should expose test-target/filter ownership and measured stage duration, with equal
required coverage as its acceptance condition. This additional case strengthens
the evidence for instruction-read and semantic-review problems, not a population
speedup claim.

## Follow-up proposal: reviewed-window registry

Records 661–696 contain a contextual request to scope the next failing gate,
not another expanded proposal prompt. All 15 reasoning blocks, 14 assistant text
blocks, 20 tool results and four written artifact bodies were reviewed against
the same source hash. `tmp/observation-ingest/registry_metrics.py` reproduces
`registry-metrics.json` without replaying historical commands.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Investigation | 661–686 | 14 / 14 | 121.123 |
| Scaffold, artifacts and validation | 687–696 | 6 / 6 | 36.791 |
| Whole proposal | 661–696 | 20 / 20 | 173.751 |

There is no internal user-reply gap. Usage: 45,607 input, 10,225,024 cache-read,
29,093 output tokens (10,299,724 total); 18,264 reasoning tokens are not added again.
Recorded reasoning totals 68,154 characters. Four artifacts delivered ten unchecked
tasks, four requirements and **five scenarios**, not the reported six. Strict
validation passed and all-item validation was 7/7; implementation was correctly
left to the next invocation. Current artifact-rule reads improved to **1/4** before
writing (proposal only), but dependencies were still authored together without
their individual instruction/dependency sequence. Scaffold creation did precede
writes, preserving the earlier race correction.

- **Diagnose values before prescribing representation changes.** The initial claim
  that COMMU00 lacked a `textbin` entry was disproved by its existing range at
  14588–15696. The retained registry also carried that 1108-byte range. Subsequent
  isolated-root failure proved absence of the expected identity, not that the
  reader produced no segment/window. The proposal nevertheless asserted that
  `splat_segments` yielded none and offered flattening or parser changes. The
  implementation later found a different cause: `target_facts` assigned the
  window to a nested manifest member. A diagnosis task was useful, but should
  have kept these remedies conditional on an open cause, including identity
  parsing, instead of constraining work to two unproved causes.
- **A proxy must traverse the actual structure.** The first YAML probe inspected
  only top-level lists; the second recursive helper never descended into mappings.
  Both missed the nested ranges. The operator recognized that limitation in
  reasoning but still wrote that the working sibling had 48 *flat* segments.
  Five versus 48 top-level entries is a count, not proof of nesting differences
  at the affected range. The line reader explicitly ignored indentation. Inspect
  parser outputs and target identity together; qualify surrogate-parser results.
- **Use an actual schema when writing observable requirements.** The inspected
  window object and its constructor had no `name` field. The new spec nonetheless
  required reading a window name from the registry while the design prohibited
  format changes. The record's `T_` consistency check is distinct from an emitted
  registry field. Likewise, the expected-set check covered three archive/entry
  identities, not each individual range's completeness. Names/counts and strict
  validity did not establish these broader promised behaviors.
- **Keep source ownership and remedy size separate.** The design claimed that
  flattening one target record needed no ownership review because it changed no
  harness behavior. Target layout changes have their own evidence obligations;
  touching fewer files does not discharge them. The scoped planning run made no
  such target edit, and its later implementation correctly rejected that remedy.
- **Bound discovery output and recurring friction.** Two batch calls indexed
  3.6/2.3 KB and returned 10,571/9,069 characters of overlapping windows. One shell
  loop again failed under the wrapper's `NODE_OPTIONS=... for` prefix; plain commands
  recovered. Four builder executions in two later calls reproduced/isolated the
  failure, but verbose filtering added no useful values. Their 2.644/2.612-second
  call spans are measured, not recoverable-savings estimates. A root-cause probe
  should expose the actual found identities, with status and unfiltered diagnostics.

Proposed improvements: a directive to keep disproved causes out of ready plans;
reference examples distinguishing range parsing, identity mapping and registry
serialization; and bounded diagnostic output showing expected versus found values.
Compare time/calls to a discriminating observation, unsupported causal assertions,
instruction-read coverage and downstream plan corrections on equivalent repairs.
The [apply case](openspec-apply-change.md#reviewed-window-registry-repair) records
the eventual fix and stronger verification; those costs are separate.

## Second explicit proposal: naming and binding debt

Records 753–771 were fully reviewed, including eight reasoning blocks, seven
assistant text blocks, nine tool results and four artifact write bodies. This is
the second expanded proposal request, with one scope-choice reply, not another
child naming run. `tmp/observation-ingest/naming_proposal_metrics.py` reproduces
`naming-proposal-metrics.json` against the parent source/hash used above.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Initial scope discovery and clarification | 753–758 | 2 / 2 | 35.848 |
| Accepted scope, further discovery and scaffold | 760–763 | 2 / 2 | 15.978 |
| Artifact writing and validation | 764–771 | 5 / 5 | 12.642 |
| Whole proposal | 753–771 | 9 / 9 | 180.138 |

The whole span includes a 77.778-second reply gap and inter-phase time; subtracting
the reply does not yield active computation time. Usage: 36,058 input, 6,247,424
cache-read and 16,930 output tokens (6,300,412 total); 8,517 reasoning tokens are
not added again. Recorded reasoning totals 35,039 characters. These costs are
planning costs, not cost per accepted rename.

Four artifacts delivered 12 unchecked tasks, five requirements and **seven
scenarios**, correcting the final report's nine. Strict validation passed and
all-item validation passed 8/8. One of four artifact instruction snapshots
(proposal) was fetched before four concurrent writes; the other artifact rules
and dependency sequence were not consumed. Creation did precede all writes.
No naming transaction or independent semantic acceptance occurred in this phase.

- **Count spellings separately from proven identities and decisions.** The retained
  report correctly totals 89 findings: 14 binding drifts, 14 formatting findings,
  40 function rows and 21 data rows. The 61 naming rows contain nine unique raw
  spellings (three functions, six data); the proposal promoted that count to
  exactly nine decisions before proving equivalence across targets. Its design
  also requires per-target evidence and allows divergent names, contradicting
  the fixed nine-decision task. Identical address-form names are a grouping lead,
  not proof of shared code, data or semantics. Keep target-qualified obligations
  in the denominator until evidence establishes which work can be reused.
- **Resolve ownership before labeling work mechanical.** CLI help and message
  locations support discovery, but do not prove that every binding drift is
  corrected by adding map ownership or that every raw row needs a semantic
  rename. The plan made both remedies mandatory without inspecting the naming
  skill/reference or complete checker implementation. It also cited the
  argument/local `rollback-source-transaction` helper as recovery for broader map
  naming. The detailed owner workflow must determine remedy, proof and rollback;
  similar command names do not establish interchangeability.
- **Reuse captured findings while preserving status and scope.** This proposal
  captured the global report once and reused it for category, file and spelling
  counts, correcting the earlier lost `error:`-prefixed finding. Its binding
  target regex still returned zero because those messages name support files;
  a follow-up correctly found four files/14 rows/11 spellings. That was recovery
  from extraction failure, not proof of zero affected targets. The scoped check
  printed `symbol maps: OK`, but its reported exit status came from `head` in a
  pipeline, not the checker. The text supports that observed success; the shell
  pattern would conceal a different failure.
- **Do not convert baseline-relative success into absence of all debt.** The
  checker call site supports comparison with a retained baseline. The proposal
  appropriately prohibited absorbing regressions, but asserted that a baseline
  rewrite alone could clear the gate despite separate normalization and binding
  findings. That claimed shortcut was not exercised or established. The spec's
  universal raw-symbol resolution wording also exceeded its non-goal of leaving
  existing baseline debt untouched. State both cohort and checker semantics.
- **Reconcile validation side effects and future checks.** A no-write requirement
  for `out/`, `build/` and `toolchains/` conflicted with planned byte verification
  and the known generation/build steps in `just check`. The proposal again called
  naming the final remaining stop without observing later recipe steps, while
  its last task correctly asked to record any next stop. Structural validation
  missed these contradictions. Require baseline-relative preservation and an
  accurate generated-output scope, rather than an unprovable clean-filesystem
  claim; this observation grants no new source-edit authority.
- **Measure discovery efficiency without inventing effort reductions.** Two
  batches indexed 5.6/5.1 KB and returned 16,524/19,784 characters of repeated
  windows. The first clarification called the scope alternatives two orders of
  magnitude apart; no effort measurement supported that estimate. Later grouping
  changed the apparent scale but still lacked identity evidence. Measure calls
  and time to a justified work-unit count, plus downstream scope corrections,
  instead of estimating effort from raw rows or distinct spellings alone.

Proposed directive: establish target-qualified work units and evidence-dependent
reuse before promising a decision count. Proposed reference: map each finding
class to its owning inspection, authorized remedy, verification and rollback.
Proposed harness improvement: one retained structured report with exhaustive
categories, target/path identities, original exit status and raw-spelling groups
labeled as unverified equivalence. Compare future naming proposals by verified
identity groups, correction rate, discovery cost and accepted outcomes; retain
all unresolved target obligations. No speedup is established by this case.

## Follow-up proposal: evidence refresh and naming

Records 853–867 contain the requested split of the prior naming change and a new
`refresh-evidence-snapshots-and-clear-naming-debt` proposal. Four reasoning blocks,
four assistant text blocks, ten tool outputs and eight artifact bodies were read.
The [apply recovery case](openspec-apply-change.md#naming-recovery-and-approved-scope-split)
owns the shared **10-call/79.780-second** orchestration cost; do not add it again
as a separate invocation. Usage totals 3,582,573 tokens (16,950 input, 3,549,440
cache-read, 16,183 output); 6,332 reasoning tokens are not added again. Metrics:
`tmp/observation-ingest/naming-recovery-metrics.json`, same parent hash above.

Delivery: four old artifacts reconciled to normalization, four new artifacts
created; the successor has **19 unchecked tasks, five requirements and eleven
scenarios**. Both changes passed strict validation and all-item validation passed
9/9. Creation preceded new writes. Current artifact-instruction reads were
**0/4 for the new proposal**; the later apply-instructions call only extracted
old-change progress. Structural validity did not establish semantic readiness.

- Owning the snapshot/index prerequisite explicitly improved scope clarity.
  But the plan inherited unproved nine-identity/five-reviewed-name assumptions,
  generator provenance and mandatory naming-before-binding order. It converted
  address containment plus a map spelling into an adoption rule, without review
  provenance, runtime linkage or cross-target equivalence. Four negative regex
  searches were again called absence of names anywhere. Keep these as evidence
  tasks with conditional remedies, not settled decisions.
- The 25-file alphabetical sample was called proof that staleness was specific;
  recipe mismatch plus mtimes was called proof of the exact causal edits.
  Neither supplied a global stale count or recipe-input history. The plan's
  all-configured-target freshness requirement also exceeded a task enumerating
  only existing snapshot files, which cannot discover missing snapshots.
- The successor still prohibited `build/` writes while requiring live byte
  verification and `just check`; it declared the freshness scan read-only without
  inspecting `prepare_target`. Its universal raw-symbol resolution clause still
  conflicted with leaving baseline debt alone. The old closure's incorrect saved
  status and incomplete aggregate identity baseline were carried into new tasks.
  Resolve these contradictions against workflow owners before calling the plan ready.
- Validation consumers printed `valid` and totals without asserting them or
  preserving upstream pipeline status. Observed results passed; the wrapper
  would not reliably stop on a future semantic `false` result. Distinguish this
  latent reporting defect from an actual validation failure in this delivery.

Proposed improvement: transfer obligations during a split with their evidence
status intact, preserving original outcome counts alongside each new scope.
Review prerequisites, generated-output effects and identity evidence across all
artifacts. Measure inherited unsupported assertions, missed/missing-target
coverage, cross-artifact contradictions and downstream corrective calls on
comparable splits; retain independent acceptance and domain evidence gates.

## Improvement proposals

| Baseline problem | Owner and proposed change | Acceptance measurement |
| --- | --- | --- |
| Two clarification replies before first creation; ambiguous nested choices | Proposal interaction directive: one decision namespace, explicit defaults and clearly named unresolved scope | Compare clarification cycles and calls to settled scope on similarly broad requests; no guesses on material choices |
| 0/16 artifact instructions fetched before writing | Proposal workflow/reference: consume each returned instruction and fresh dependencies before its write; use the returned graph | All required instruction reads precede writes; authored content matches current rules and dependency files |
| Strict-valid artifacts contain file/directory conflict and weak owner claims | Semantic review guidance: trace each requirement through scenario, task and supporting owner evidence | Zero unresolved contradictions presented as ready; speculative mappings remain explicitly unresolved |
| Three untracked targets with Git-only preservation check | Dirty-work reference: retain before/after bytes or hashes for intended paths, including untracked files | Every preservation claim has content evidence; unrelated edits survive without requiring a clean tree |
| One duplicate execution just to count findings; one bucket lost to `error:` prefix | Symbol-report tooling/reference: reuse captured output and expose stable structured finding categories | One probe supplies total and all categories; buckets sum to 89 for this retained fixture; no omitted error prefix |
| Proposed optional detector described as unchanged enforcement | Proposal acceptance guidance: name lost automatic checks and verify the complete recipe before promising green | Coverage and gate claims agree with executable recipe behavior; lower call/time cost retains authorized checks |
| Broad retrieval crowded out requested sections | Discovery reference: scope authored owners first; query historical goals only when relevant and retain complete results once | Fewer rereads on comparable scopes; source/count/exit evidence remains recoverable |
| One create/write race, one replacement write | Proposal tool-ordering directive: await successful scaffold creation before artifact mutation; inspect collisions before recovery | No creation race or unexplained overwrite on comparable multi-artifact proposals; existing content preserved |
| Absolute-import scan missed a surviving consumer | Harness-removal reference: inspect relative imports and canonical command/test registries before fixing deletion scope | Every retained consumer and command mapping accounted for; scoped checks retain their prior behavior |
| Line/token counts presented as contract or usability proof | Compaction reference: use obligation mapping plus comparable mission retrieval/decision measures; keep size secondary | No lost conditions, exceptions or examples; reduced calls/tokens with equal decision accuracy, or justified no-op |
| Later facts absent from earlier artifacts; conflicting set checks | Proposal coherence review: reconcile proposal, spec, design and tasks after scope discovery; distinguish preserved sets from additions | No contradictory acceptance clauses; prerequisite evidence and capability losses agree across all artifacts |

Measure first-pass proposal acceptance, revisions and downstream rework when the
remaining history is reviewed. This case motivates experiments; it does not prove
a speedup or justify new tests, dependencies or installed-extension changes.
