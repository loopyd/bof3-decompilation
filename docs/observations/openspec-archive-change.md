# openspec-archive-change observations

Single-change archive evidence under the
[measurement contract](INDEX.md#performance-measurement-contract).
Proposals below are not implemented directives or tooling.

## Coverage and measured performance

Seven expanded archive requests occur in the candidate parent session. All seven
episodes, records 320–335, 462–475, 519–531, 602–615, 646–660, 738–752 and
868–884, are reviewed here, plus an eighth contextual archive within the combined
successor request at 1120–1132. Other parent and child histories remain pending. No child
metadata explicitly selects this skill, which does not establish absence of
parent work. The [onboarding archive phase](openspec-onboard.md) is a separate
phase of that mission and must not be counted as another invocation here.

Source: `.pi/sessions/2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4.jsonl`,
SHA-256 `2c4e084cd00b25aa453b20018463e58fd9ed3fee5c4d7c524d863aa6d838d791`.
For the first case, all seven assistant text blocks, seven reasoning blocks and seven tool results
were reviewed, plus the emitted main-spec body and actual move command.
`tmp/observation-ingest/archive_metrics.py` reproduces the scalar metrics in
`archive-repair-metrics.json`; historical commands were not rerun.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Completion and sync assessment | 320–324 | 2 / 2 | 15.671 |
| Inline sync, including rule retrieval and verification | 325–332 | 4 / 4 | 18.881 |
| Move and completion report | 333–335 | 1 / 1 | 5.347 |
| Whole archive | 320–335 | 7 / 7 | 67.215 |

Whole elapsed time includes a 23.083-second user-reply gap and time between
phases; phase spans do not sum to the whole. Calls comprise four `bash`, one
`read`, one `write` and one `ctx_execute`. Recorded usage: 23,263 input,
2,412,288 cached-read and 8,050 output tokens, totaling 2,443,601; separately
reported reasoning tokens (4,121) are not added again. No population distribution,
billing estimate or broader task-success rate follows from this single case.

Observed outcome: one requested archive moved successfully, preserving the listed
five artifact files including `.openspec.yaml`; it disappeared from the active
list. One new main capability was created with three requirements and five
scenarios. Main-spec validation passed 2/2; final all-item validation passed 5/5.
These are structural/workflow outcomes, not semantic acceptance of the preceding
[implementation](openspec-apply-change.md).

## Durable lessons

- **Ordered sync before move worked.** The operator identified the contextual
  change, read CLI artifact paths and 8/8 task counts, recognized a missing main
  spec as a new ADDED-only capability, and presented the sync choice required by
  the historical archive instructions. The user selected sync. One specs-rule
  snapshot preceded the main-spec write; sync and verification completed before
  the move. This case does not justify removing that historical choice or
  attributing its reply delay to execution friction.
- **A collision check must stop the mutation.** Record 333's shell branch only
  printed `FAIL` if the destination existed, then unconditionally continued to
  `mv`. The recorded destination was free and the move succeeded, so no collision
  damage occurred. Nevertheless, the command did not enforce the skill's required
  collision stop; an existing directory could have received a nested source.
  Future archive tooling should fail before movement and verify destination
  identity, not merely print a diagnostic.
- **Heading equality is a weak merge gate.** Record 332 compared requirement
  names, scenario names and normalized Purpose, then structural validation. It
  did not compare requirement or scenario bodies. This audit compared the full
  emitted write body with the freshly read delta, allowing title, delta-header
  and whitespace normalization: they match. The actual merge is supported here;
  the historical check still could not detect changed assertion bodies under
  unchanged headings. Keep verification quality separate from observed content.
- **Archive repeats do not strengthen implementation evidence.** Records 324 and
  335 repeated unchanged line counts, 39 dirty paths and owner-comment claims as
  proof of preservation and correctness. Those are the same weak proxies found
  in the apply episode, not independent checks. `8/8` and `all_done` cannot repair
  an unsupported underlying task claim. Preserve uncertainty in the archive.
- **Structural validity can carry contradictory requirements.** The synced spec
  demands existing files while allowing directory owners in its split-claim
  scenario; it also leaves external-reference treatment unspecified. The planned
  transport retirement would remove the command named by its clean-sweep
  scenario. The operator disclosed that dependency but still reported no
  warnings. Copying an approved delta faithfully and validating its structure
  do not resolve these design obligations. Record them without silently rewriting
  requirements during synchronization.
- **Tool retirement does not prove behavior uncheckable.** The final report said
  the scenario would have no way to be checked after command retirement. That
  proves loss of the named execution path, not impossibility of native inspection.
  The replacement procedure and required spec update needed explicit ownership.

## Second case: ladder guidance

Records 462–475 were fully reviewed: seven reasoning blocks, seven assistant text
blocks, five tool results, emitted main-spec body and actual move command.
`tmp/observation-ingest/ladder_followup_metrics.py` reproduces the measurements in
`ladder-followup-metrics.json` against the source hash above.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Assessment | 462–465 | 1 / 1 | 13.550 |
| Inline sync | 466–472 | 3 / 3 | 16.011 |
| Move and report | 473–475 | 1 / 1 | 5.335 |
| Whole archive | 462–475 | 5 / 5 | 61.167 |

The whole span includes a 21.694-second user-reply gap. Recorded usage: 22,505
input, 3,416,448 cache-read and 6,359 output tokens, totaling 3,445,312; 2,043
reasoning tokens are not added again. Fewer calls than the first case do not prove
an improvement: batching, context and capability scope differ, and cached context
cost increased. Neither observed span is active execution time.

One archive moved, retaining the five listed files including `.openspec.yaml`;
the active list excluded it. Its main spec contained five requirements and eight
scenarios, correcting the assessment's hand-counted nine. Main-spec validation
passed 3/3, all-item validation 6/6. One rule snapshot preceded the write. As in
the first case, this audit verified normalized full-body equality while the
historical verifier checked only names and Purpose.

Across these first **2/2 reviewed archives**, the collision branch only printed failure before
continuing to `mv`; both destinations happened to be free. Thus two successful
moves, zero observed collisions, and two ineffective collision guards are distinct
measurements. Both reports also promoted inherited weak implementation evidence.
These two ADDED-only cases do not establish performance for modifying specs or
recovering collisions.

The ladder archive illustrates a further coherence failure: task files accepted
deferring one file's reduction, but the copied spec still required every applicable
file to shrink. It described all 12 boxes as verified and reported “No warnings”
despite an accepted failing gate and unresolved semantic checks. Its final claim
of identical pre/post Rust failure lacked a pre-change run. User acceptance may
authorize closure; it does not establish the missing evidence. Preserve exception
status and reconcile scope across artifacts before treating archive validity as
implementation verification. Faithful synchronization alone cannot repair an
unreconciled source spec.

## Third case: emulator guidance

Records 519–531 contain an inferred contextual selection from three active changes,
one sync-choice reply, and a successful archive. All six reasoning blocks, six
assistant text blocks and five tool results were reviewed, plus the actual
verification/move command. `tmp/observation-ingest/emulator_metrics.py` reproduces
`emulator-metrics.json`, including normalized full-body delta/write equality.

Assessment used one call in 14.207 seconds; inline sync, move and report used four
calls in 22.835 seconds. Total: **5 calls / 5 results, 40.520 seconds**, including
a 3.478-second reply gap. Usage: 20,299 input, 3,229,312 cache-read and 5,861 output
tokens (3,255,472 total); 1,804 reasoning tokens are not added again. Sync and move
share a call here, so their separate costs are unmeasured.

The five listed artifacts, including `.openspec.yaml`, moved; the active list
excluded the change. One new main spec preserved five requirements/seven scenarios;
the report corrected its earlier count of eight. Validation passed 4/4 specs and
6/6 total items. The comparison still checked names and Purpose rather than bodies.
Inherited claims of unreachable pages, unchanged scripts by mtime, and a smaller
per-mission reading surface remained unsupported at their stated scope. The
archive again reported “No warnings” alongside an accepted failing gate.

Record 529 **improved the collision branch to `exit 1`** and made heading/Purpose
mismatch assert before movement. However, `set -e` without pipeline failure
propagation did not enforce the stated validation precondition: each validation
was piped to Python that only printed `valid`/totals, without asserting them.
A valid JSON failure result could therefore leave the pipeline successful and
allow movement. The observed validations passed, so this is a control-flow defect,
not an observed invalid archive. Also, `-e` alone does not cover dangling symlinks;
destination identity and mutation semantics still need a complete guard.

For these three fully reviewed ADDED-only archives: **3/3 moves succeeded**,
**3/3 copies matched normalized bodies**, **0/3 historical comparators checked
bodies**, and **2/3 collision branches lacked a terminating stop**. There were no
observed collision cases, so collision-recovery success is unmeasured. Calls were
7/5/5 (median 5, nearest-rank p90/max 7); observed seconds 67.215/61.167/40.520
(median 61.167, p90/max 67.215), all including user waits. N=3 from one program
is a descriptive sample, not evidence of a general speedup.

## Fourth case: transport retirement

All seven reasoning blocks, seven assistant text blocks and five tool results in
records 602–615 were reviewed, plus the actual archive command and memory-ingest
arguments. Source/hash are shared above; metrics are reproduced by
`tmp/observation-ingest/retirement_metrics.py` in `retirement-metrics.json`.
Assessment: one call/17.994 seconds; inline sync plus move: three calls/17.320
seconds; closure plus one memory ingestion: one call/4.872 seconds. Total:
**5 calls / 5 results, 55.269 seconds**, including a 3.851-second reply gap.
Usage: 27,982 input, 4,226,304 cache-read and 8,884 output tokens (4,263,170 total);
3,156 reasoning tokens are not added again. Memory publication is part of this
total, not archive movement or an independent skill invocation.

One new main spec preserved four requirements/seven scenarios, including normalized
full-body equality independently checked by this audit. The historical comparator
again checked only headings/Purpose. Five artifacts moved; active listing excluded
the change, all-spec validation passed 5/5 and all-item validation 6/6. The terminating
collision branch survived, but the print-only validation pipelines repeated the
third case's failure-propagation defect. No collision or failing validation occurred.

- **Indexed output is not consumed guidance.** The six-command assessment batch
  reran status/list inside its summary helper, then failed capability-path derivation
  with `relative_to`. Its returned windows repeated the error and status data;
  the archive instructions' body was not shown. Record 605 expressly treated the
  indexed 1.6-KB section as read and assumed it matched earlier guidance. The next
  step correctly fetched full spec text/rules for copying. Required instruction
  consumption needs the returned content, not output size or successful indexing.
- **Keep compact discovery compact.** The assessment indexed 9.6 KB from six outer
  commands and returned 17,863 characters of overlapping windows. Its summary
  re-executed two existing probes rather than consuming retained JSON. That is
  measurable duplication/output amplification, not six independent discoveries.
  Prefer one retained result per CLI probe and capability paths relative to the
  reported change root; report missing guidance instead of inventing its contents.
- **Memory publication does not verify a conclusion.** The ingested completion
  document was acknowledged with `ok:true`, then the final report treated its
  contents as durable knowledge. It repeated the unsupported no-obligation-loss
  claim, the incorrect 159-line baseline, and the previously overstated orphan-page
  claim. Its opening also said all changes plus a follow-up were archived while
  item 5 correctly said the follow-up remained open. A successful write confirms
  storage, not semantic correctness; preserve unresolved facts when consolidating
  session outcomes. This audit did not replay or duplicate that memory upload.

## Fifth case: text-count guard fix

Records 646–660 were fully reviewed: six reasoning, six assistant text and six
tool-output blocks, with write, verification/move and memory-ingest arguments.
`tmp/observation-ingest/guard_metrics.py` reproduces `guard-metrics.json` against
the source hash above. Assessment used one call/15.406 seconds; sync and move used
four calls/19.160 seconds; memory and closure used one call/5.326 seconds. Total:
**6 calls / 6 results, 63.689 seconds**, including a 14.406-second reply gap.
Usage: 27,053 input, 4,576,640 cache-read and 7,395 output tokens (4,611,088 total);
2,395 reasoning tokens are not added again. The extra call split rules and delta
retrieval; it does not establish worse performance than the prior batch.

Three requirements/five scenarios were copied with normalized full-body equality
verified by this audit. Five listed artifacts moved, leaving no active changes;
all-spec and all-item validation both passed 6/6. The historical comparator again
checked headings/Purpose only. The collision branch stopped on an existing target,
but print-only validation pipelines still did not enforce validation success.

- **Return the guidance, then retain its limitations.** This assessment returned
  archive instructions, improving on the fourth case's missing body. Its six-command
  batch still turned 6.2 KB of indexed output into 18,126 characters of repeated
  windows. Required guidance consumption and discovery-output efficiency are
  separate measurements; one should not be sacrificed for the other.
- **Archive scope cannot resolve an implementation contract by inference.** The
  returned context appeared to prohibit hand edits under `out/`; the operator
  noticed this only after the document edit and argued that a test guarding the
  document implied edit authority. A test dependency alone establishes no such
  permission. Resolve conflicts against the actual controlling contract before
  mutation. The user's sync selection did not establish a general exception.
  This audit records the historical tension without deciding the current contract
  from that stale context or reopening unrelated source work.
- **Persist evidence limits with results.** The archive and its acknowledged memory
  update repeated the claims that the sample was gone, coverage was fully retained,
  and the registry failure was unrelated to the entire program. The apply evidence
  supports narrower conclusions above. “Every task backed by a run” and “No warnings”
  also exceeded the evidence: the branch was N/A and content-preservation proof was
  missing. Storage acknowledgment is not independent review. The report reduced
  three edited mentions to “one sentence”; consolidation should retain exact scope.

## Sixth case: reviewed-window registry repair

Records 738–752 were fully reviewed: six reasoning, six assistant text and six
tool-output blocks, plus emitted spec, verification/move and memory-ingest
arguments. `tmp/observation-ingest/registry_archive_metrics.py` reproduces
`registry-archive-metrics.json` against the source hash above. Assessment used
one call/15.034 seconds; inline sync and move four calls/14.266 seconds; memory
and closure one call/6.674 seconds. Total: **6 calls / 6 results, 109.701 seconds**,
including a **57.863-second user-reply gap**. Usage: 26,904 input, 5,304,320
cache-read and 6,949 output tokens (5,338,173 total); 1,673 reasoning tokens are
not added again. The longer whole span cannot be attributed to slower execution.

Four requirements/five scenarios were copied with normalized full-body equality
verified by this audit. Five artifacts moved and the active list became empty;
all-spec and all-item validation passed 7/7. The historical check again compared
only headings/Purpose. Its terminating collision branch retained the later
pattern, as did the print-only validation pipelines that could permit movement
after a failed validation. No failed prerequisite or collision occurred.

- **Separate artifact fidelity from specification truth.** The preserved delta
  still required a window `name` absent from the implementation schema and
  described universal range coverage where the guard checked three named pairs.
  The preceding byte comparison supports this retained registry's equality,
  not every promised property. Faithful copying and 7/7 structural validity did
  not resolve those obligations; the archive nonetheless reported no warnings.
- **Reconcile reported counts from the same retained evidence.** The final report
  and memory update repeated 89 naming findings but listed 13+14+40+21=88. The
  supported binding count is 14. A corrected observation must retain both the
  total and mutually exclusive buckets, not propagate an earlier transcription
  error. The same output claimed no build/output files were touched despite the
  preceding recorded rebuilds and generation steps; retained-registry equality
  cannot establish an unchanged filesystem.
- **Bound completion claims to checks reached.** The stored update called naming
  the only remaining stop before a green gate, although later recipe steps had
  not run. It also misidentified which file had been called tracked and clean,
  and generalized the manifest repair beyond the cases exercised. Its successful
  storage acknowledgment supplied no new verification. The six-command discovery
  batch returned the necessary instructions but amplified 6.3 KB of indexed
  content into 18,264 returned characters through repeated windows.

Across six reviewed ADDED-only archives: **6/6 observed moves and normalized body
copies succeeded; 0/6 historical comparators compared bodies**. Two collision
branches did not stop; the next four did, while all four later piped validations
lacked failure enforcement. All destinations were free, so collision recovery
remains unmeasured. Calls 7/5/5/5/6/6 have median 5.5, nearest-rank p90/max 7.
Spans 67.215/61.167/40.520/55.269/63.689/109.701 seconds have median 62.428,
p90/max 109.701, including user waits and differing closure work. This
single-program sample establishes neither population speedup nor independent
implementation acceptance.

## Seventh case: normalization after the naming split

Records 868–884 were fully reviewed: one aborted response with no tool call,
then six reasoning blocks, six assistant text blocks, six tool results, the
emitted main spec, move command and memory-ingest arguments. The retry's expanded
archive request is record 870; it is one archive delivery, not two successful
missions. `tmp/observation-ingest/naming_archive_metrics.py` reproduces
`naming-archive-metrics.json` against the same parent hash.

Assessment used one call/14.247 seconds; sync and move four calls/15.828 seconds;
memory and closure one call/6.399 seconds. The expanded-request span is **6 calls /
6 results, 110.407 seconds**, including a 57.872-second sync-choice reply gap.
Including the aborted request gives **122.228 seconds**, with the same six calls.
Usage: 26,214 input, 6,341,376 cache-read and 6,003 output tokens (6,373,593 total);
1,319 reasoning tokens are not added again. These boundaries retain the interruption
without misclassifying it as an archive mutation failure or extra completion.

The requested change moved with all five listed artifacts, including metadata;
the successor remained active at 0/19. The new main spec preserved two requirements
and four scenarios with normalized full-body equality checked by this audit.
Historical verification again compared only headings/Purpose. Main-spec validation
passed, all-spec validation was 8/8 and all-item validation 9/9. The collision
branch terminated on existing targets; validation pipelines still only printed
results, so their failure-enforcement limitation remained. No collision or failed
validation was observed. The six-command assessment returned 16,614 characters
from 6.5 KB of indexed output, including the required archive instructions.

- **Preserve the approved split without overstating its proof.** The archive
  clearly recorded normalization as the closed scope and 75 findings as pending
  in the successor. However, “every task backed by a run,” “nothing else moved”
  and byte-identical baseline claims repeated the preceding apply's incomplete
  identity baseline, category-count proxy, prefix comparison and missing write-set
  proof. No new check repaired those limitations. Administrative 9/9 closure and
  successful archiving did not verify the original 89-finding campaign.
- **A correction can propagate new unsupported claims.** The acknowledged memory
  update correctly changed 89 to 75, but stored the five-name adoption plan as
  reviewed evidence, universal negative searches as no names anywhere, address
  containment as resident-engine ownership, and binding-after-naming as necessary.
  It also retained the biased sample's prevalence claim and unisolated seven-second
  refresh estimate. These limitations are established in the
  [apply recovery](openspec-apply-change.md#naming-recovery-and-approved-scope-split).
  Storage acceptance does not corroborate them. Historical memory publication was
  inspected, not repeated; future summaries should transfer uncertainty with facts.

Across **seven** reviewed ADDED-only archives, **7/7 moves and normalized body
copies succeeded; 0/7 historical comparators checked complete bodies**. Two
collision branches lacked a terminating stop; the next five stopped, but all five
later validation pipelines lacked failure enforcement. No collision recovery was
exercised. Calls 7/5/5/5/6/6/6 have median 6 and nearest-rank p90/max 7. Expanded-
request spans 67.215/61.167/40.520/55.269/63.689/109.701/110.407 seconds have median
63.689 and p90/max 110.407, including user waits and mixed closure work. The seventh
case's pre-request interruption is separately reported above. This cohort supports
specific verification improvements, not a population speedup or implementation
acceptance rate.

## Eighth case: native archive and successor creation

Records **1120–1132** contain one user request to create the successor and archive
the evidence-refresh change. Both skills were explicitly selected and their full
procedures read. All six reasoning blocks, six assistant text blocks, six results
and tool arguments were reviewed against the same source hash. Metrics are in
`tmp/observation-ingest/successor-creation-metrics.json`, reproduced by
`successor_creation_metrics.py`.

Two shared skill reads span **8.378 seconds**; two archive-specific calls at
1124–1127 span **6.249 seconds**; two scaffold/handoff calls span **10.364 seconds**.
The combined request is **6 calls / 43.670 seconds**, including intervals between
phases, with no user-reply gap. Combined usage is 251,555 input + 1,238,400
cache-read + 6,955 output = **1,496,910 tokens**; 4,686 reasoning tokens are
included, not added again. These costs also appear in the
[creation ledger](openspec-new-change.md#successor-scaffold-after-naming-rescope)
and must be counted once. Do not add this mixed request to the seven comparable
archive-only spans above as if its whole cost measured only archiving.

The native `openspec archive … -y --json` returned a successful archive receipt:
`2026-09-25-refresh-evidence-snapshots-and-clear-naming-debt`, `specsUpdated: true`,
**three added requirements**, zero modified/removed/renamed. A subsequent listing
showed the new main spec, and the archive directory exposed three Markdown files
and `specs/`. The active set became empty before successor creation.
The native receipt establishes a different execution path from the seven manual
sync/move cases; no collision or failed-sync recovery was exercised.

- **Native success is not an atomicity guarantee.** CLI help says archive updates
  main specs; it does not establish rollback behavior or atomic sync-and-move.
  The operator asserted both without implementation or failure-path evidence.
  The native route removes the visible ad hoc `mv` guard, but its internal
  collision/rollback behavior remains unreviewed here.
- **Retain guidance and checks when changing execution route.** Root and 13/13
  task counts were freshly read, but no old-change status/artifact-path lookup or
  specs-rule fetch occurred in this interval. Archive advice was fetched, then
  each output line clipped to 200 characters, truncating its required context.
  The three operation-guidance entries were visible; the operator acknowledged
  them but treated previously checked boxes as genuinely verified. The
  [closeout audit](openspec-apply-change.md#evidence-refresh-narrowed-scope-verification-closeout)
  establishes missing proof for three verification tasks. CLI acceptance supplies
  no independent repair of that evidence.
- **Preservation remained underchecked.** Post-archive checks counted three
  requirement headings, two `Decision 8` mentions and one disposition-task match.
  They neither compared requirement/scenario bodies nor verified every archived
  file, including hidden metadata. The final “record intact” claim therefore
  exceeds those probes. This eighth body copy is not independently verified by
  this audit; the earlier **7/7 body-equality** result must not become 8/8.
- **Status capture differs by wrapper.** The archive pipeline captured its first
  element's `PIPESTATUS` immediately in the same shell, supporting its printed
  exit zero alongside the native success receipt. This differs from the later
  proposal validator, where `PIPESTATUS` was read outside the relevant subshell.
  Preserve the actual boundary instead of treating every printed zero alike.
- **Authorization and scope are distinct from the old prompt choreography.** The
  user expressly requested archive and successor creation. The operator chose
  native sync plus archive without the historical skill's separate sync-choice
  prompt and did not author successor artifacts until the next user request.
  Record that deviation and the explicit authorization; do not infer a current
  extra-approval requirement from this historical skill or treat sync as semantic
  acceptance of the source requirements.

Across all eight observed archive deliveries, eight moves are reported successful;
seven normalized copies are independently checked and one is unverified. No
historical post-sync comparison inspected all bodies. The eighth route differs
in validation/control ownership, so the seven-case collision/pipeline defect
denominators remain seven. Inherited universal ownership and timing claims were
promoted to main-spec obligations; faithful archival, even if later established,
would not make those claims true.

Proposed archive **directive/reference**: when using the native CLI, map each
required external input/check to its actual owner and retain evidence limitations
in the archive summary. Proposed **tooling**: a receipt with full sync-body and
archive-file comparisons plus explicit validation/rollback scope. Acceptance:
every preserved-content claim has body-level evidence, missing proof remains
visible, and comparable native cases report command/check costs separately.
This case supports investigating the native route, not a measured speedup or
permission to remove review gates.

## Improvement proposals

| Finding | Proposed owner/change | Acceptance measurement |
| --- | --- | --- |
| Collision branch prints failure but still moves | Archive mutation helper/directive: make collision a terminating precondition and verify the resolved destination | Existing-target cases leave both trees unchanged; successful cases retain exact archive identity and metadata |
| Five cases pipe validation through print-only parsers | Archive orchestration: require both command success and successful parsed validation before mutation; preserve failure status through pipelines | Every failed prerequisite stops movement; valid outcomes retain artifact identity, with checks and mutation costs reported separately |
| Indexed instruction body was not visible; assessment reran status/list | Archive discovery reference: consume complete required instructions and reuse retained structured results | Required reads completed before claims of compliance; duplicate executions and returned bytes per comparable assessment |
| Headings used to claim faithful merge | Sync reference and comparison tooling: compare complete requirement/scenario bodies and retained content after only specified normalization | Preserved bodies / applicable bodies; mismatches halt archive; structural validity reported separately |
| Archive repeats weak apply evidence | Archive guidance: carry the implementation evidence limits and unresolved obligations into completion reporting | Inherited unsupported claims promoted to verified / reviewed archives; missing evidence remains visible |
| Registry closure reports buckets totaling 88 as 89 and predicts unreached checks | Closure reference/tooling: derive totals and categories from one retained result; carry reached, failed and unreached checks into summaries and memory | Bucket sum equals total for exhaustive categories; unsupported completeness claims / reviewed closures; correction calls and time separately measured |
| Planned retirement invalidates named verification procedure | Cross-change reference: identify successor verification and reconcile dependent scenarios before retiring a command | Applicable scenarios with executable or documented replacement checks / affected scenarios; unresolved dependencies remain open |

Compare future cases by ADDED-only versus modifying/retiring capabilities,
existing-target status, sync choice and evidence obligations. Measure archive
completion, partial recovery, check/move costs and retained content independently.
This case suggests improvements, not a measured speedup or authorization for new
tests, dependencies or installed-extension changes.
