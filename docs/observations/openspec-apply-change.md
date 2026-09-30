# openspec-apply-change observations

Implementation evidence under the
[measurement contract](INDEX.md#performance-measurement-contract). Changes
proposed here are not implemented directives or harness behavior.

## Coverage and measured performance

Eight expanded apply-request records occur in the candidate parent session
`2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4.jsonl`.
The first seven episodes and their follow-ups through the approved naming split
are reviewed below; the eighth now includes its initial recovery and narrowed-scope
verification closeout. Its campaign/pilot parent evidence is in the naming ledger,
its rescope in the update ledger; full child review remains pending. The second and
seventh include requested new proposals, whose costs are shared orchestration
rather than pure apply cost. A ninth contextual apply request explicitly reads
this skill and reaches a capability-blocked pause; its parent evidence is complete
through record 1199, with full child review pending. A tenth contextual case covers
the capability follow-on's launch and first two supervisor decisions through
record 1363, including the first workflow's rejected evidence, the subsequent
task-specific canonical proof and owner-body launch/scope corrections; later
parent work and full child transcript review remain pending.
Zero explicit skill selections in
the 4,171 child metadata records cannot establish absence of apply work.

Two complete child reports provide **supporting task-execution evidence**, not
two proven independent skill invocations. Tool event metrics and runtime status
snapshots join by run ID; the full child reasoning/output review is still pending.

| Task / run | Started tool calls | Lifecycle seconds | Reported result |
| --- | --- | --- | --- |
| Selected-call producer, `985971e6-dc10-4fe3-a562-268457531323` | 40 | 1,157.132 | Partial task 1.1; imported caller discovery and branches unsupported |
| Canonical proof, `f6a32076-b75a-4468-bee5-29ddc8eab8d7` | 35 | 1,199.635 | Task 1.6 checked with retained canonical evidence; production unchanged |

Both process exit codes are zero and runtime states complete; acceptance metadata
is rejected. These separate facts do not support a 100% task-success rate or a
0% productive-work rate. Task 1.1 remained explicitly incomplete; task 1.6 reports
a narrower canonical proof, not completion of the whole feature.
Task 1.1 also failed its implementation criterion and `no-staged-files` evidence
check; its rejection cannot be attributed solely to the global index check.

Sources: matching complete `*_worker_output.md`, metadata and transcripts in
`.pi/sessions/subagent-artifacts`; runtime scalar/hash snapshots in
`tmp/observation-ingest/runtime-metrics.json`. These historical checks were not
rerun by this audit.

## Durable lessons

### Complete parent case: documentation repair and follow-up

Parent records 258–319 (2026-09-25 UTC) contain one explicit apply invocation,
then one corrective follow-up, not two independent invocations. All 23 assistant
text blocks, 23 reasoning blocks and 35 tool-result occurrences were read. Source:
`.pi/sessions/2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4.jsonl`,
SHA-256 `2c4e084cd00b25aa453b20018463e58fd9ed3fee5c4d7c524d863aa6d838d791`.
Metrics in `tmp/observation-ingest/apply-repair-metrics.json` are reproduced by
`apply_metrics.py`; source identity is shared with
the [proposal program](openspec-propose.md). Historical commands were not rerun.

| Phase | Records | Calls / matched results | Observed seconds | Result |
| --- | --- | --- | --- | --- |
| Initial implementation | 258–305 | 28 / 28 | 113.931 | Four documents repaired; seven task boxes complete |
| User-requested follow-up | 306–319 | 7 / 7 | 42.768 | One additional claim-group link; eighth task added complete |
| Whole case | 258–319 | 35 / 35 | 553.606 | Includes 396.907 seconds between completion report and user reply |

Phase spans total 156.699 seconds; neither that sum nor the reply gap is CPU time.
Calls comprise eight `bash`, eleven `read`, ten `edit` and six `ctx_execute` calls;
one wrapper call can contain several commands. Recorded usage totals 69,902 input,
7,959,808 cached-read and 26,910 output tokens (8,056,620 total). The separately
reported 16,876 reasoning tokens are not added again. These are one-case costs,
not a distribution, billing estimate or accepted-task throughput.

- **Useful selection and context behavior:** the operator inferred the first
  change from the preceding program order, announced the selection and override,
  fetched apply state, and read all four returned context artifacts before edits.
  Four active changes did not require another selection question when context
  already identified the intended one.
- **Observable repair, limited acceptance:** the sweep moved from eleven broken
  references to zero; record 298 reports 1,225 local `ok` and 141 `external`
  references. The follow-up adds one locally resolving reference, yielding 1,367
  total references. External status is enumeration, not successful URL checking.
  Both completion states and strict validation passed; independent semantic
  acceptance was not recorded. Do not call all 1,367 references healthy.
- **Scope clarification became avoidable rework:** the task named two test owners,
  while the spec required each split claim to have an owner. Record 305 called the
  omitted third link additional scope and declared completion; after the user's
  request, record 319 recognized it as closing the existing spec scenario. The
  seven follow-up calls are 7/35 of this case's calls, but not all are recoverable
  overhead: ownership inspection and verification still belonged in the work.
- **Claim verification was too weak:** record 300 returned broad module comments,
  test counts and helper names (`fixture`, `new`, `bind`), not the asserted packing
  behaviors. A zero-SPU-step tuning rejection does not establish rejection of
  stopped-tone pitch/reassignment edits. Record 309's test names and silent
  fixtures likewise did not prove held-value behavior of re-encoded first samples.
  Both reports promoted these leads to verified ownership. Exact assertions and
  owner bodies remain unreviewed here; this is insufficient proof, not a finding
  that the underlying behaviors are false.
- **Preservation proxies were promoted to proof:** unchanged counts of 232/221/87/59
  lines and 39 dirty paths do not establish unchanged unrelated bytes. Three
  targets were untracked and the tracked router already dirty. Records 296–305
  recognized the missing starting snapshot but still declared preservation.
  Minimal edit arguments support a narrow intended change, not an independently
  checked whole-worktree preservation result.
- **Result-schema misunderstanding caused extra work:** record 273 labeled the
  filtered array length as total references checked. A full-list reread recovered
  the denominator, but another full-list request exceeded the output bound;
  unchecked empty stdout caused a JSON parse error (record 291). Direct diagnosis
  exposed exit 2 and the bound message. The already available `reference_count`,
  `broken_count`, `unchecked_count` and `status_counts` would have answered the
  question without expanding every result. This historical transport is retired;
  the lesson applies to current structured result consumers, not restoring it.
- **Completion ordering matters:** seven original task boxes were ticked after
  their stated checks, though two checks were semantically inadequate above.
  Follow-up task 1.3 was created checked at record 310, before the document edit
  at 313 and verification at 315–316. The operator disclosed the deviation in its
  final report. Thus 1/8 completed task entries was premature; later passing
  verification repaired final state without repairing the historical ordering.
- **Formatting indecision has measurable context cost:** record 310 alone contains
  10,207 reasoning characters repeatedly reconsidering line wrapping before a
  convention lookup. The entire case has 65,707 reasoning characters. These are
  text-volume measurements, not isolated latency or billed-token savings. The
  mistaken line-count preservation criterion contributed to the indecision.

The archive phase is measured separately in
[openspec-archive-change](openspec-archive-change.md); its repeated preservation
claims are inherited assertions, not additional independent confirmations.

### Ladder compaction: initial attempt through pause

Records 336–416 of the same hashed parent source contain the explicit
`simplify-skill-ladders` apply request and its first attempt, **not the complete
change history**. All 31 reasoning blocks, 27 assistant text blocks and 47 tool
results were read. `tmp/observation-ingest/ladder_metrics.py` reproduces
`apply-ladder-metrics.json`, including exact edit character/line deltas.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Context and baseline | 336–345 | 6 / 6 | 16.298 |
| Allocation ladder | 346–354 | 5 / 5 | 28.460 |
| Terminal ladder | 355–369 | 9 / 9 | 39.940 |
| Lesson levels | 370–378 | 5 / 5 | 31.260 |
| Policy and exhaustion | 379–387 | 6 / 6 | 26.495 |
| Cosmetic ladder | 388–400 | 7 / 7 | 48.694 |
| Compaction and pause | 401–416 | 9 / 9 | 46.257 |
| Entire initial attempt | 336–416 | 47 / 47 | 273.268 |

Phase spans exclude intervals between phases. Recorded usage is 105,756 input,
13,052,160 cached-read and 51,577 output tokens (13,209,493 total); the separately
reported 31,923 reasoning tokens are not added again. Calls comprise four `bash`,
13 `read`, 13 `ctx_execute`, 16 `edit` and one `write`. These are attempt costs,
not accepted-result throughput or measured savings.

The attempt ended at **5/12 checked tasks** with seven guidance files edited.
The 13-file baseline fell from 2,519 to 2,488 physical lines; seven of eight
ladder files decreased, while `tool-usage.md` stayed at 1,242. No tracked gate
keyword disappeared. Neither the scoped `just check` nor documentation-drift
gate had run by this pause. The follow-up below records later checks and acceptance.

- **The reduction metric changed the editing behavior.** Record 349's allocation
  rewrite removed three lines but added 454 characters. Record 401's stale-index
  paragraph removed six lines while adding one character; its skill-footer edit
  removed two lines and only eight characters. Record 406 removed two table lines
  and one character. Joining lines and deleting table scaffolding can satisfy a
  line-count target without reducing reading/context cost. Record 383 contains
  19,877 reasoning characters repeatedly debating how to shrink already terse
  rules; the attempt contains 126,318 reasoning characters overall. Text volume
  is measured; attributable time or token savings from another method is not.
- **Added gates need semantic review.** The allocation rewrite called any change
  in scheduling/allocation residual, spill set or reload order a gate, without
  establishing improvement. Its type rung invoked `byte-match` as if it diagnosed
  a type-shape mismatch. These new formulations were not verified against command
  semantics. Remaining full-byte-match/independent-review rules still applied;
  keyword retention did not prove the newly introduced intermediate gates sound.
- **A verification script missed its own stated requirement.** Task 1.2 required
  both files to state a focused Rizin rung. Record 369 tested Rizin only in the
  protocol, yet record 370 claimed both files passed. The skill text read at 357
  and its ensuing edits did not add that explicit rung. Its reported eleven
  numbered items also combined the five scope steps with six ladder steps.
  A positive count or nearby concept is not proof of the specific requirement.
- **Structural preservation checks were useful but incomplete.** A new protocol
  link used `../SKILL.md` instead of `../../SKILL.md`; the checker found one broken
  link among 16 and the correction passed 16/16. The cosmetics rewrite preserved
  the externally referenced spelling-transaction anchor and all six numbered
  rungs. In contrast, task 1.3 required links to both owners, but the emitted
  `LESSONS.md` retained bare code-path mentions and only its existing playbook
  hyperlink. The task was still checked complete.
- **A hand-counted baseline caused a false alarm.** Level 5 had nine rows, not ten;
  the first verifier asserted 30 total gates and reported loss after the rewrite.
  A second call corrected the count to 29 and found all labels. That correction
  was justified by the original read, but label equality alone still cannot prove
  that every obligation within each row survived. The baseline saved line/token
  counts rather than complete starting text for this untracked file.
- **Duplicate detection was overinterpreted.** Record 411 checked exact trimmed
  lines of at least 45 characters within each of six files; only `tool-usage.md`
  also received an exact normalized sentence check above 80 characters. It found
  three repeated command lines there and none in the other files. It did not
  compare semantic restatement across files or whole CLI surfaces. Record 413
  printed command-bearing lines clipped to 150 characters plus distinct flag
  counts. The pause report nevertheless concluded that no duplication could be
  removed without deleting substance. That universal claim exceeded the search.
- **Owner awareness helped, but did not prove impossibility.** The operator
  recognized `tool-usage.md` as the ordered-workflow owner and left obligations
  open instead of declaring the whole change done. It proposed revising the
  occurrence-count target and deferring this file to planned transport retirement.
  Those options exposed a real planning tension; the supporting claim that the
  requested compaction was impossible remained unproved. Follow-up review must
  distinguish user-approved scope changes from mechanically inferred necessity.
- **Diagnostic scope affected noise.** The inbound-link search at record 392
  returned 9,292 characters, largely historical subagent report excerpts, before
  the useful current anchor references. Such excerpts are leads with provenance,
  not current authority or fully reviewed child missions. The edit tool's brace
  warning also caused two inspection calls; the unchanged net brace imbalance
  was outside the edited block, though the report attributed it to balanced
  `while`/`do` examples rather than the two open `if` examples shown in the output.

Proposed apply/reference improvements: define semantic preservation checks before
editing; permit justified no-op compaction; measure characters/tokens and reading
effect alongside physical lines; derive structural baselines from actual content;
and state each search's matching rules and limits before drawing a no-op or
blocked conclusion. Compare follow-up corrections, unmet stated checks, retrieval
volume and accepted preservation across comparable guidance edits. A lower line
count or keyword count must not substitute for those outcomes.

### Ladder follow-up: scope revision, checks and acceptance

All 19 reasoning blocks, 15 assistant text blocks and 24 tool results in records
417–461 were reviewed, including task-edit arguments and the new proposal's four
artifact bodies. `tmp/observation-ingest/ladder_followup_metrics.py` reproduces
`ladder-followup-metrics.json` against the same source hash.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Approved scope revision and verification | 417–444 | 14 / 14 | 255.674 |
| Acceptance plus new guard-fix proposal | 445–461 | 10 / 10 | 70.473 |
| Whole follow-up | 417–461 | 24 / 24 | 399.956 |

The whole span includes a 73.809-second reply gap; the preceding pause-to-reply
gap was 842.322 seconds and is excluded. Recorded usage: 57,710 input, 8,683,264
cache-read and 29,070 output tokens, totaling 8,770,044; 16,278 reasoning tokens
are not added again. This is mixed follow-up cost, not a new apply invocation or
a pure rework penalty. The initial attempt plus follow-up used 71 calls, including
proposal work; no independently accepted-task throughput follows from that sum.

- **Check the actual aggregate gate.** Record 437's `just check` exited 101 at
  Justfile line 52, before the expected symbol check at line 67. The Rust target
  reported two passes and one failure: a document's claim of 32 heuristic
  instances conflicted with the current-count guard. Its reported test duration
  was 102.71 seconds; call-to-result span was 157.805 seconds. Those measures cover
  different boundaries. Documentation drift passed; a separate symbol check
  still returned 89 findings, now correctly bucketed 14/14/40/21. The enclosing
  recipe did not reach that gate, and the task initially remained open at 11/12.
- **Failure attribution requires a baseline.** Older file timestamps and an empty
  direct-filename search were called conclusive proof that the Rust failure was
  pre-existing and identical before/after (438–444). No pre-change gate run was
  shown. Indirect inputs and changed dependencies were not excluded. Record the
  observed failure and limited write scope without upgrading them to causality
  or byte-preservation proof. The same limitation applies to timestamp-based
  claims that untouched dirty directories were preserved.
- **Acceptance is a decision, not a passing check.** The user accepted the failed
  gate and requested a separate fix (445). Task 3.2 was then checked, retaining
  both “Unticked pending operator decision” and the appended acceptance decision.
  Report 12/12 administratively closed, one documented exception, and the failed
  gate separately. Remove superseded task-state wording when recording a decision.
- **Reconcile all affected planning artifacts.** The user approved deferring
  `tool-usage.md` reduction and replacing the occurrence-count metric. Only task
  files changed; the original spec/design still required every applicable file
  to shrink. The later archive faithfully propagated that unreconciled condition.
  Aggregate grep output also did not prove an owner citation in each file, and
  exact duplicate scans did not justify the new task's universal no-duplication
  claim. Neither a user-approved exception nor strict validity supplies missing
  evidence for a different assertion.
- **Repeated tool friction has a concrete owner.** Two shell loops in record 418
  failed because the batch wrapper placed `NODE_OPTIONS=...` before `for`, repeating
  the proposal-phase wrapper failure. Focused calls recovered. One task edit used
  stale wording, failed at 97% similarity, then succeeded after a fresh read.
  Count failed subcommands separately from wrapper calls; do not label necessary
  recovery or verification as entirely removable cost.

The [new proposal](openspec-propose.md) and [archive](openspec-archive-change.md)
retain their own evidence. Proposed improvements: capture baseline failures before
attributing them; reconcile spec/design/tasks after accepted scope changes; model
accepted exceptions separately from passing gates; and fix wrapper handling of
shell compound commands. Measure unsupported attribution claims, stale-state task
entries, cross-artifact contradictions and failed subcommands per reviewed case.
No time-saving estimate or implemented improvement is claimed.

### Emulator guidance restructuring

Records 476–518 of the same hashed source contain one explicit apply invocation
and its acceptance follow-up. All 17 reasoning blocks, 17 assistant text blocks
and 22 tool-result occurrences were reviewed, with emitted entrypoint/catalog
bodies and acceptance-edit arguments. `tmp/observation-ingest/emulator_metrics.py`
reproduces `emulator-metrics.json`; no historical command was executed.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Context and baseline | 476–485 | 6 / 6 | 15.731 |
| Catalog and entrypoint | 486–498 | 7 / 7 | 38.677 |
| Routes, verification and pause | 499–512 | 7 / 7 | 209.786 |
| Accepted exception and closure | 513–518 | 2 / 2 | 16.050 |
| Whole apply case | 476–518 | 22 / 22 | 726.440 |

The whole span includes a 413.582-second user-reply gap. Usage: 52,945 input,
9,814,272 cache-read and 27,160 output tokens, totaling 9,894,377; 14,722 reported
reasoning tokens are not added again. The 56,084 reasoning characters include
17,533 at record 486, largely reconsidering layer ownership/grouping. This measures
text volume, not wasted latency or recoverable token savings.

Observed delivery: 23 original catalog entries preserved across five layers;
25 original reference paths retained, one catalog added; 54 Lua script names,
53 test names and `runtime.sh` retained. The entrypoint fell from 49 to 33 lines;
the 69-line catalog brought their combined total to 102. Scoped link inspection
reported 121 local `ok`, four external, zero broken/unchecked; the full sweep
reported 1,240 local `ok` and 141 external. External enumeration is not URL health.

- **Refresh the baseline, then reconcile every artifact.** The filesystem count
  corrected the proposal's 52/55 script/test split to 54/53. All four apply context
  files were freshly read. However, only tasks recorded the corrected figures;
  proposal/design retained stale counts. The baseline stored action/reference
  pairs and filename lists, not each action's script mapping required by task 1.1.
  Its 23 entries also included Runtime/API guidance, so “a script for every action”
  needed classification rather than silently treating every row as executable.
- **Prove reachability at the claimed depth.** Two pages absent from the original
  front-page table were called orphaned and unreachable from the entrypoint.
  No traversal of links inside the 23 linked pages supported that stronger claim.
  Adding direct catalog entries improves visibility; it does not prove that access
  was previously impossible. The earlier emulator ledger's unreachable claim has
  been corrected to direct-table omission.
- **Match verification to its semantic unit.** One extra verification call corrected
  a false duplicate warning caused by counting Runtime's shared prerequisite link
  as a second layer placement. Section-scoped rows passed 25/25. The action-set
  check first filtered catalog entries to known baseline descriptions, so it
  could detect losses/duplicates but not arbitrary extra actions. Reference-set
  equality was also replaced by no deletions plus one intended catalog addition,
  without reconciling the task's original equality wording.
- **Do not claim preservation from timestamps.** Untracked script/test directories
  made the Git-status check inconclusive. Comparing their latest mtimes with the
  new documentation writes still did not compare starting/ending content. This
  was nevertheless called decisive proof of unchanged behavior (506–518).
  Limited mutation arguments support the intended scope; they do not make the
  missing content baseline exist. Record what each check actually establishes.
- **Measure the complete retrieval path.** The report asserted a smaller per-mission
  surface without exercising a reader. The new Observe section has 18 physical
  lines including separators: entrypoint plus that section is 51 lines before
  catalog introduction and mandatory Runtime guidance, versus the original
  49-line entrypoint. Other layers differ; line wrapping is not token cost.
  The new label “without driving it” also groups bounded stepping with state
  observation, a semantic inconsistency absent from the filename/link checks.
  Compare actual calls, loaded tokens and correct action selection on like missions.
- **Carry accepted exceptions precisely.** `just check` again exited 101 at line 52
  with two passes/one failure; target duration 104.35 seconds, containing batch
  call-to-result span 162.375 seconds. Unlike the preceding ladder attribution,
  record 437 already showed this exact failure before the emulator edits. That
  supports recurrence, not unchanged content everywhere. Task 4.2 stayed open
  until explicit acceptance, then its edit retained both “Unticked pending” and
  the accepted decision. Final 12/12 closure and strict validity are separate
  from a passing aggregate gate or independent semantic acceptance.

Proposed apply/reference improvements: baseline content and action-to-owner mappings;
classify executable versus guidance entries; reconcile later facts across all
artifacts; distinguish graph reachability from direct links; and evaluate actual
retrieval rather than front-page size. Measure unsupported preservation claims,
false verifier alarms, stale artifact assertions and correctly selected actions
per comparable mission. The [emulator ledger](psx-emulator.md) retains the narrower
skill-guidance implications; the archive is separately counted.

### Documentation transport retirement

Records 532–601 contain one contextual apply invocation. All 29 reasoning blocks,
25 assistant text blocks and 43 tool-output blocks were reviewed; 40 call IDs have
matching results, with three results also containing diagnostic text. The same
source hash applies. `tmp/observation-ingest/retirement_metrics.py` reproduces
`retirement-metrics.json`, including full read/write body sizes.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Context and prerequisite | 532–542 | 7 / 7 | 16.066 |
| Surface removal and package checks | 543–561 | 10 / 10 | 269.408 |
| Skill migration and next document read | 562–575 | 9 / 9 | 28.156 |
| Document contract | 576–583 | 4 / 4 | 29.424 |
| Owners and final checks | 584–601 | 10 / 10 | 128.266 |
| Whole apply | 532–601 | 40 / 40 | 502.347 |

No user reply occurs inside this span. Recorded usage: 78,816 input, 16,429,056
cache-read and 31,696 output tokens (16,539,568 total); 15,820 reasoning tokens
are not added again. These observed costs include checking and recovery, not just
implementation. The obsolete command was removed from registry/canonical argv/
test policy; five transport modules and one wrapper were deleted. Surviving modules
imported, drift returned zero findings, and source validation reported 2,071 valid,
zero invalid lifts. The symbol check retained 89 findings and exit 2.

- **Context consumption must be observed.** The operator announced reading all
  four context artifacts, but issued only three reads: tasks, design and spec.
  Proposal was not freshly read in this episode. Prior authorship is distinct
  from satisfying the returned context-file contract.
- **Select validation scope before paying for duplicate runs.** The first commands
  suite passed 188 tests in 29.44 seconds; the containing `check-unit` recipe was
  then run and reached the known Rust failure (two passes/one failure, 106.78-second
  target duration; recipe span 195.245 seconds, exit 101). Final commands tests
  passed 188 in 28.41 seconds after further documentation edits. The first standalone
  run overlaps the mandated recipe; the final rerun followed later changes and
  a stated task, so repetition alone is not proof it was waste. Task 1.4 applied
  the user's earlier exception decisions with disclosure instead of a third
  identical approval request. Accepted exception, suite pass and recipe failure
  remain separate outcomes despite the later “All checks pass” statement.
- **Retirement needs a working verification replacement.** After deleting the
  checker, the operator called it for its own document rewrite, producing unknown
  domain plus a JSON parse error (581). One replacement call checked seven inline
  links across three files. Its regex/ATX-heading parser did not implement the
  just-written contract's reference-style links, HTML IDs, setext headings,
  escaped/query destinations, root-relative paths or code exclusions. Nor did
  that check cover every later edited owner/plan file. Zero problems in seven
  parsed links is not full contract compliance or a whole-tree link pass.
- **Preserve obligations independently of keywords.** The nine regex checks
  verified selected phrases, not every prior rule. The old skill required retaining
  input hashes and stopping on stale pins; the rewrite substituted a fresh read
  and a general no-overwrite statement. The document rewrite similarly removed
  the explicit external-pin comparison/stop procedure. That is an observable
  evidence-procedure change, so “all obligations preserved/no gate weakened” needed
  review beyond token presence. No source corruption is established by this audit.
- **Measure the actual bodies.** The retained documentation read has **170 lines**,
  not the repeatedly claimed 159; its emitted replacement has 101. Characters
  changed 10,622→6,136. The skill read/write changed 45→42 lines and 2,862→2,374
  characters, despite reasoning calling its baseline 50. The tool-usage hand-off
  yielded 1,242→1,241 lines; harness guidance 1,029→1,028. These are size changes,
  not measured reading-time or correctness improvements.
- **Capture process status and output before formatting.** A helper discarded
  stdout on successful gates (594), requiring fresh drift/validation captures.
  The symbol parser again misclassified the initial `error:`-prefixed binding
  finding: 13 binding plus one `other`, instead of 14 binding. Record 595 then
  ran `echo` before reading `PIPESTATUS`, replacing the symbol pipeline's status
  with zero. The full earlier result still established exit 2. Capture once,
  preserve stdout/stderr/status, then derive counts; headings and truncated prefixes
  are inadequate substitutes for a structured result.
- **Keep negative search and behavior claims bounded.** A no-match `grep` joined
  with `&&` skipped later discovery queries (553); a fresh import read recovered
  the alias. Help's one `docs` occurrence belonged to retained `source docs`, not
  the removed domain. Later dotted-import grep plus module import success did not
  exercise `agent context cleanup docs` path validation, although the final report
  claimed that consumer still behaved identically. Surviving negative results and
  importability support narrower claims than all future behavior being unchanged.

Proposed owners: apply guidance should reconcile the full obligation set before
closure; documentation references should provide a scoped replacement verification
procedure with explicit unsupported forms; harness result helpers should preserve
exit/stdout/stderr once for reuse. Measure missing required reads, unmatched
obligations, invalid checker invocations, repeated probe cost and unsupported
behavior claims on comparable retirements. The [documentation ledger](bof3-docs.md)
retains migration-specific implications; no tool restoration or new tests are
authorized by these observations.

### Text-count guard fix

Records 616–645 contain one explicit apply: 13 reasoning blocks, 12 assistant text
blocks and 16 tool results, all reviewed with edit/probe arguments. The source hash
above applies; `tmp/observation-ingest/guard_metrics.py` reproduces
`guard-metrics.json`. No historical command was executed.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Context and original failure | 616–625 | 6 / 6 | 127.803 |
| Probe identification and edit | 626–631 | 3 / 3 | 25.637 |
| Verification and closure | 632–645 | 7 / 7 | 579.176 |
| Whole apply | 616–645 | 16 / 16 | 745.393 |

No user-reply gap occurs inside this span. Usage: 38,123 input, 8,161,536 cache-read
and 21,379 output tokens (8,221,038 total); 12,426 reasoning tokens are not added
again. The 49,464 reasoning characters include repeated wording/scope deliberation,
not an isolated measurement of avoidable latency. Three mentions in one evidence
document changed from instances to candidates; no guard source edit was issued.

- **Separate target behavior from aggregate success.** The original guard found
  exactly one offending line across 37 documents; after the edit, all three tests
  passed. Two simultaneous plants then triggered their respective failures:
  `**21** candidate runs` and retired figure `58,941`. Restoring the document
  reproduced the post-edit SHA-256 and a fresh 3/3 pass. The full `just check`
  subsequently failed in `text_window_registry` (7 passed/1 failed, exit 101,
  still Justfile line 52). The next observed stop was recorded rather than the
  anticipated symbol gate. That establishes useful progress, not a green recipe.
- **Measure verification scope before streamlining it.** Four standalone guard
  runs reported 104.25/104.97/104.35/103.28 seconds, totaling **416.85 seconds**;
  each also ran the slow reproducibility test. The combined plant/restore call
  spanned 207.875 seconds; the later full recipe spanned 208.466. These boundaries
  differ. Combining both plants preserved separate failure diagnostics and shared
  one restoration. Investigate filtering existing document checks for intermediate
  probes while retaining required full validation; no such timing or savings was
  measured here. The script restored successfully but lacked an interruption
  cleanup trap, and hash mismatch only printed a warning before continuing.
- **A search limit cannot establish sample loss.** The sample search used depth
  three and `head -10`, returning AREA000 paths. It did not identify the disposable
  probe, but the final report upgraded that result to “sample is gone.” No 32/31
  reproduction occurred. Help established the term *candidate* and the meaning
  of `--no-readable`; it did not establish that this flag alone described the old
  probe, whose text mentioned all three opt-outs. Preserve the recorded number's
  provenance and unknown exact settings; do not add unsupported specificity.
- **Passing a lexical guard does not prove prose truthful.** The new phrase
  `32 heuristic candidates` no longer matched the count/noun/kind pattern, and no
  historical skip marker was added. This proves the observed acceptance, not
  semantic truth of the unrerun experiment. Two planted cases prove two exercised
  paths, not exhaustive coverage. The report's “tuned the detector” description
  also conflicts with its accurate statement that the detector was unchanged.
- **Carry baseline limits into completion.** Task 2.3 required a content comparison
  across the document set but was checked using mtimes. A hash for the edited file
  proved its later restoration, not preservation of the other 36. Older config
  mtimes likewise did not prove the newly reached registry failure predated the
  entire program. Record that failure as observed; causality remains unproved.
  Final 10/10 closure includes one explicitly inapplicable branch, and is distinct
  from ten implemented changes or ten independently verified requirements.
- **Context and task wording still drifted.** Only three of four returned context
  artifacts were freshly read despite announcing four. The task's “both tests”
  wording remained beside a result correctly reporting three tests. One conditional
  branch was clearly resolved as N/A; retain that useful distinction while removing
  superseded assertions instead of appending contradictory results.

Proposed owners: apply guidance should distinguish observed, inferred and unreproduced
claims; references should show bounded-search and probe-provenance examples; harness
check orchestration should preserve separate expected-failure diagnostics, guaranteed
restoration and hard-stop comparisons. Compare probe reproduction, unsupported claims,
restoration success and verification seconds on equivalent guard fixes. No new tests
or claimed speedup follow from this case; its archive is separately measured.

### Reviewed-window registry repair

Records 697–737 contain one explicit apply. All 18 reasoning blocks, 16 assistant
text blocks, 22 tool outputs and diagnostic/edit arguments were reviewed against
the source hash above. `tmp/observation-ingest/registry_metrics.py` reproduces
`registry-metrics.json`; the preceding proposal has its own costs and ledger entry.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Context and baseline | 697–706 | 6 / 6 | 22.040 |
| Diagnosis | 707–719 | 7 / 7 | 157.075 |
| Ownership and fix | 720–730 | 6 / 6 | 48.502 |
| Validation and closure | 731–737 | 3 / 3 | 281.628 |
| Whole apply | 697–737 | 22 / 22 | 541.306 |

No internal user-reply gap occurs. Usage: 53,708 input, 13,060,480 cache-read and
30,008 output tokens (13,144,196 total); 18,273 reasoning tokens are not added
again. Reasoning totals 69,289 characters, including 12,013 at record 707 before
reading the test invocation. Text volume is not isolated latency or saved cost.

Observed fix: `target_facts` retained its first parsed identity/address rather
than overwriting them with later manifest-member fields. Instrumentation showed
COMMU00's window previously assigned to `WORLD04/AREA185.EMI#14`. The corrected
fresh registry matched all **14,690,707 retained bytes**, including three reviewed
windows among 48,571 total. The targeted suite changed from 7 pass/1 fail (1.67 s)
to 8/8 pass (1.57 s). Source validation reported 2,071 valid/zero invalid lifts;
the full gate advanced to symbol checking at line 67, exit 2 with 89 findings.
Its enclosing call, including targeted tests and source validation, spanned
250.061 seconds. This supports a concrete repair, not whole-repository acceptance.

- **Inspect the mismatching value, not only its upstream existence.** A surrogate
  parser and the first instrumentation both found the range. The first diagnostic
  filtered out the error/status, prompting a stale-binary theory before its result
  was known. Rebuilding unchanged source disproved that theory. The second diagnostic
  printed the constructed archive/entry and exposed the actual identity error.
  Both rounds executed the builder twice to obtain different output views. Capture
  once, retaining values, status and stderr; derive presentations afterward.
- **Record diagnostic cost and restoration precision.** Four diagnostic/restoration
  builds reported 21.22/20.54/20.18/20.31 seconds (**82.25 s**); the three containing
  calls spanned 24.033/21.975/43.296 seconds. The final changed-source build took
  20.65 seconds. These are different boundaries, not all removable overhead.
  Both source restores matched only the first 16 hexadecimal SHA-256 characters,
  despite reports implying full-digest verification. No interrupted restoration
  occurred, but cleanup was an ordinary chained command without a trap; mismatch
  only printed a warning. Final diagnostics had zero occurrences. A reusable
  bounded diagnostic procedure should guarantee cleanup and compare full bytes
  or digests before proceeding, including for untracked source.
- **Validate the implemented scope, not every similar manifest.** The inventory
  found 20/394 manifests with multiple `disc_id` lines; only COMMU00 among the
  three current reviewed-window owners had them. One call site and whole-artifact
  equality bound the observed change well. They do not prove that every identity
  consumer or all 20 manifests were fixed. First-match guards remain a line-scanner
  convention, not table-aware parsing: a missing/unparseable primary field can
  still fall through to a member. Current valid-input success is not proof of
  every failure condition promised unchanged in the design.
- **Reconcile the plan with the diagnosis.** Tasks disclosed the actual function
  (`target_facts`, not `splat_segments`) and note location (`tool-usage.md`, not
  `harness.md`), but proposal/design/spec retained the old diagnosis and impossible
  registry-name scenario. Only three of four context artifacts were freshly read.
  The ownership search sampled a Python package table, which did not establish
  the whole document's exclusive scope; note and code edits were submitted together
  despite the intended note-first order. Disclosure helps, but does not replace
  artifact coherence or evidence for the selected ownership contract.
- **Keep preservation and closure claims literal.** Exact registry equality
  established no need for a separate regeneration to repair content. The subsequent
  `just check` nevertheless ran generation gates, so “no out/ file was written”
  was not established by an empty Git-status sample. No complete `out/` baseline
  was compared. Task 2.4 was explicitly N/A; Splat regeneration was also omitted
  because no target record changed. Report those dispositions alongside passing
  checks. The final “only remaining stop” claim exceeded a recipe that stopped
  at symbols and did not execute its remainder.
- **Corrections need the right referent.** The design called the target record
  tracked/clean, which its evidence supported. The report incorrectly recast this
  as a prior claim about untracked `cli.rs`. Its task recap also typed offset
  18588 once while the evidence and task file correctly said 14588. Derive recaps
  from retained results; do not turn a scope correction into a new factual error.

Proposed owners: apply guidance should request the discriminating value and update
all affected artifacts when a diagnosis changes; references should distinguish
table identity from textual key order; diagnostic tooling should retain one complete
result and enforce restoration. Measure time to correct cause, redundant executions,
restoration coverage, unreconciled clauses and actual accepted checks on comparable
repairs. Existing tests and byte comparison supported this case; no new test or
broader source change is authorized by the observation.

### Naming and binding debt: initial attempt through pause

Records 772–811 contain the seventh explicit apply request and its first attempt,
not the complete naming implementation. All 18 reasoning blocks, 17 assistant
text blocks and 21 tool outputs were reviewed, including baseline, normalization
and task-edit arguments. The same parent source/hash applies.
`tmp/observation-ingest/naming_initial_metrics.py` reproduces
`naming-initial-metrics.json`; historical commands were not rerun.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Context | 772–778 | 4 / 4 | 10.726 |
| Baseline and normalization | 779–786 | 4 / 4 | 171.694 |
| Binding diagnosis | 787–802 | 9 / 9 | 100.145 |
| Task update, recovery and pause | 803–811 | 4 / 4 | 35.381 |
| Initial attempt | 772–811 | 21 / 21 | 348.407 |

No user reply occurs inside this span. Usage: 55,591 input, 14,735,872 cache-read
and 26,864 output tokens (14,818,327 total); 15,206 reasoning tokens are not added
again. Reasoning totals 57,680 characters, including 11,558 at record 801 while
debating remedies and whether to pause. These are volume and transcript-time
measurements, not proof of recoverable latency or wasted tokens.

Observed delivery: 14 target maps normalized, their formatting findings reduced
14→0, and the global report reduced **89→75** (14 binding, 40 function and 21 data
findings remain). Repeating the formatter preserved all 14 post-normalization
SHA-256 values. The attempt ended with **4/12 boxes checked**, strict-valid
planning artifacts and no completed naming transaction. Accepted-rename
throughput is undefined; task ticks and formatting progress are different units.

- **Store the real status with the retained evidence.** Record 779 correctly
  captured and printed checker exit 2, but wrote `check_exit: 0` literally into
  `/tmp/clear-debt-baseline.json`, then claimed the exit had been saved. The
  retained machine-readable baseline therefore contradicted its source result.
  Later closure compared only the first 16 hexadecimal characters of the naming
  baseline's SHA-256, though the initial temporary record retained a full digest.
  Derive status and checksums once, retain them without transcription, and compare
  full values before reporting unchanged state.
- **Do not substitute aggregate status for each required identity check.** The
  first target list appended four guessed support-file stems to 59 real target
  IDs, producing one parse failure (1.590-second call span). Removing those stems
  recovered a valid aggregate: 206 exact, zero partial/invalid across the naming
  target set, alongside a stale-snapshot coverage error. The recovery call ran
  one target and then all 59, spanning 73.615 seconds. This was aggregate status,
  not each affected target's live byte comparison, and excluded the four binding
  owners. Task 1.3 was nevertheless checked; repo-wide metadata validity likewise
  supplied no missing per-target byte proof. Keep the narrower observation and
  unsatisfied requirement separate.
- **Read the composed input before recreating its checker.** An initial read began
  below `by_address` construction. The operator inferred that only the local map
  mattered and ran generator previews plus a substitute comparison across four
  targets. It reported 165 current drifts for AREA016 where the harness reported
  three; every SDK preview falsely appeared to have 162 drifts. A later short
  read exposed `load_target_symbols`, which combines target, shared and SDK maps.
  The nine-call/100.145-second diagnosis phase includes this detour and useful
  ownership work; it is not wholly recoverable overhead. The substitute-comparison
  call alone spanned 22.074 seconds. Compare a proxy against the owner's known
  result before using it to choose a mutation.
- **A renderer and address range do not prove provenance.** The shared generated
  header and `weak_bindings_c` renderer were called proof that the wrong generator
  had written the files. SDK preview output itself contained raw `func_*` names;
  such names or numeric addresses did not establish game-versus-SDK ownership.
  The historical producer was not identified. Inspect actual manifest ownership,
  generator inputs and references before choosing regeneration, map additions or
  removal; label a stale-file theory as a hypothesis until supported.
- **A failed candidate remedy does not prove a single required remedy.** The
  baseline membership probe examined all 21 raw bindings in four files, including
  ten in AREA016, although that file had only three drift findings. It showed
  those target/raw-name pairs absent from the baseline; it did not prove every
  drift needed a new semantic name or that naming must precede all binding work.
  The task update nevertheless made that ordering a blocker. No correctly
  composed regeneration comparison or ownership investigation ruled out other
  remedies. The unchanged nine-decision claim also still lacked cross-target
  equivalence evidence. Update plans around established facts and remaining
  alternatives rather than promoting an untested explanation to necessity.
- **Distinguish diagnosis progress from verified task closure.** Task 1.1 had a
  conflicting stored exit, 1.2 equated spellings with decisions, and 1.3 weakened
  per-target proof to an aggregate. Their checked status did not establish their
  stated verification. Task 2.1 did have observed category reduction and repeat
  hashes. The later 249 dirty config paths were not a baseline-relative proof
  that only the 14 intended maps changed. Preserve those evidence limits without
  erasing the real formatting improvement.
- **Record recurring tool failures at the right unit.** Two of five subcommands
  in one batch failed under the known `NODE_OPTIONS=... for` wrapper problem;
  focused calls recovered. Its 4.2-KB indexed result returned 14,685 characters
  of repeated windows. One task-edit call used `[x]` in its expected old text and
  failed; a state read confirmed no partial edits, then the corrected edit passed.
  Failed edit through recovery spanned 14.137 seconds and three calls, including
  necessary inspection. Only three of four apply context artifacts were read,
  again omitting proposal. These are separate read-compliance, invocation and
  recovery measures, not one undifferentiated failure rate.

Proposed directive: require the stated task evidence before checking a box, and
separate open remedy choices from demonstrated blockers. Proposed reference:
explain composed symbol ownership, generated-file provenance, target selectors
and the difference between status aggregates and live byte proof. Proposed
harness/result improvements: retain actual exit/stdout/stderr and full hashes,
expose composed ownership for each finding, and reuse captured structured output.
Compare diagnosis calls/time, proxy disagreement, corrected task closures and
accepted target-qualified outcomes on equivalent naming attempts. This initial
case supports those experiments, not a measured speedup; the next continuation
is reviewed below.

### Naming follow-up: artifact revision and snapshot prerequisite

Records 812–838 continue the same apply after the user selected artifact revision
and continuation. All 13 reasoning blocks, 13 assistant text blocks, 13 tool
outputs and the emitted design/task changes were reviewed. Source/hash are shared
above; `tmp/observation-ingest/naming_followup_metrics.py` reproduces
`naming-followup-metrics.json`.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Artifact revision | 812–815 | 2 / 2 | 14.361 |
| Lead and identity investigation | 816–829 | 7 / 7 | 70.157 |
| Report initialization and blocker diagnosis | 830–838 | 4 / 4 | 70.174 |
| Whole follow-up | 812–838 | 13 / 13 | 172.048 |

The preceding 67.643-second user-reply gap is excluded; no further user reply
occurs within this span. Usage: 35,920 input, 11,090,560 cache-read and 19,882
output tokens (11,146,362 total); 11,130 reasoning tokens are not added again.
Recorded reasoning totals 45,537 characters. The initial attempt plus this
follow-up used 34 calls; neither stage completed a naming transaction.

Observed outcome: design ordering changed, tasks were rewritten, and a concrete
report-initialization prerequisite failed. Four tasks remained checked, but the
rewrite added a regeneration-verification task, making **13 tasks (4 checked,
9 pending)**. The final report incorrectly retained **4/12**. There was no new
global debt check, byte-identity gate, artifact validation or baseline-hash check
in this follow-up; its 75-findings/unchanged-baseline recap inherited prior evidence.

- **Plan revisions must preserve evidence limits and recompute task counts.**
  The approved reorder was implemented before continued investigation, but the
  design now called the unproved naming-before-bindings conclusion a measured
  necessity. The task rewrite proposed both binding generators without settling
  which owned each file, and used the generated-file header as part of proving
  generator provenance. A shared header cannot identify the actual producer.
  The whole-file rewrite also changed task IDs and added task 4.2 without updating
  the reported denominator. Derive counts from the emitted artifact and retain
  uncertainty when implementing an approved plan correction.
- **Local preflight success does not admit the whole workflow.** One `naming
  prepare` returned ready with zero findings. The operator immediately declared
  the reviewed path open and the old index blocker irrelevant. Later `naming init`
  failed on another target's stale snapshot. Local readiness, global snapshot
  validity, report creation and transaction admission are distinct gates. Identify
  their dependency chain before interpreting one successful result as permission
  to skip the others; this case does not justify weakening any gate.
- **Lead records are navigation, not semantic review.** `opportunities` and
  `describe-opportunity` returned inventory identity, fingerprint and map hash,
  not behavioral evidence or an approved name. A map scan found non-raw names
  at five of nine queried addresses; the report called them reviewed names ready
  to adopt. No owning body, review record or cross-target equivalence was checked.
  The scan covered target `symbols.txt` files with one regex, so four negative
  results meant no match in that scan, not no semantic name anywhere.
- **Count occurrences and spellings independently.** The scan accumulated counts
  per name but printed the number of distinct name keys as “targets.” Its single
  raw-name key became “1 raw” in the report despite the known 22/11/7 function-row
  multiplicities. `counter2` in two maps and `selectionCounter` in one are two
  semantic spellings, not three; adding the raw spelling gives three total names.
  Different targets can legitimately reuse an address, so address equality alone
  does not establish either a conflict or a propagation obligation.
- **A fingerprint and range check prove only their stated properties.** The
  operator assumed an opportunity fingerprint represented code bytes without
  reading its algorithm. The already-named target was absent from the raw-lead
  list, yielding `None`, so no fingerprint comparison occurred. A subsequent
  128-byte probe found the queried address below MAGIC004's load address
  (offset −0x9278 in a 4,716-byte payload); no two-buffer comparison ran. That
  supports absence from this payload, not the asserted resident-engine ownership
  or runtime linkage to BATTLE03. Even two equal arbitrary prefixes would need
  a justified extent and ownership evidence before transferring a semantic name.
- **Classify invocation failure separately from prerequisite refusal.** One
  nonexistent `analysis index --check` flag and one missing `describe-opportunity`
  argument failed. The corrected description succeeded. A report path under
  `/tmp` was rejected for escaping the repository, then two consumers still tried
  reading the nonexistent result. Retrying inside the repository hit snapshot
  staleness and caused a third missing-file traceback. Branch on producer status
  and output existence before running dependent readers; these cascading errors
  are not three additional naming-pipeline defects.
- **Bound the observed blocker instead of expanding it.** Three repo-local init
  attempts, for MAGIC004, AREA000 and BOSS001, hit the same MAGIC003 stale-recipe
  error; two global queries also stopped there. MAGIC004's own snapshot status
  was fresh. Counting 396 snapshot files did not establish that exactly one was
  stale, and sampling three targets did not prove every operation failed. The
  older conflicting-symbol error was unreached, not reproduced. Preserve the
  first failing prerequisite and unknown later failures rather than calling a
  remembered diagnosis current evidence.
- **Distinguish workflow outputs from a read-only probe.** `naming init` is a
  report-producing operation even when it fails before creating the requested
  file. Its rejection of `/tmp` established repository confinement, not a mandate
  to use `out/reviews`; 2,005 existing directory entries established usage, not
  an exclusive location contract. The no-generated-write task already needed
  reconciliation with the intended workflow, but that should follow its owning
  reference rather than directory popularity. The proposed refresh was not run
  in this interval, so its cost, complete write scope and success were unmeasured.

Proposed directive: consume the domain workflow and check necessary prerequisites
before promising a naming campaign or converting leads into reviewed decisions.
Proposed reference: distinguish lead identifiers, fingerprints, payload containment,
runtime ownership, local readiness and global report prerequisites with bounded
examples. Proposed harness/report improvements: expose those gate statuses
separately, stop dependent consumers after a failed producer, and derive task and
finding totals from retained structured data. Compare calls/time to a valid
readiness decision, invalid invocations, cascaded errors, supported name transfers
and corrected progress reports across comparable attempts. Recovery and the approved
split are reviewed next; no speedup or accepted rename is claimed.

### Naming recovery and approved scope split

Records 839–867 complete this apply case through the user's approved split.
All 11 reasoning blocks, 11 assistant text blocks, 16 tool outputs and eight
written artifact bodies were reviewed against the same parent hash.
`tmp/observation-ingest/naming_recovery_metrics.py` reproduces
`naming-recovery-metrics.json`; historical probes were not executed.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Snapshot refresh and index retry | 839–843 | 2 / 2 | 29.064 |
| Sweep, sample and diagnosis | 844–852 | 4 / 4 | 135.674 |
| Recovery through pause | 839–852 | 6 / 6 | 173.032 |
| Approved split and new proposal | 853–867 | 10 / 10 | 79.780 |
| Recovery plus split | 839–867 | 16 / 16 | 288.768 |
| Entire apply through split | 772–867 | 50 / 50 | 897.539 |

Recovery plus split includes a 35.956-second reply gap; the entire apply includes
three gaps totaling 124.272 seconds. A matching tool result can be an RPC error,
not a successful subprocess termination. Recovery/split usage totals 36,675 input,
9,648,768 cache-read and 25,044 output tokens (9,710,487 total); 10,756 reported
reasoning tokens are not added again. Its 44,016 reasoning characters include
17,412 at record 854 considering the split's naming and structure. These are
observed costs, not CPU time or independently measured avoidable overhead.

Observed outcome: one snapshot refreshed successfully; the index and naming
pipeline remained blocked. The approved split rewrote all four old artifacts
around normalization and created four artifacts for the remaining work. The old
change became **9/9 checked**, with two requirements/four scenarios; the new change
had **0/19**, with five requirements/eleven scenarios. Both were strict-valid and
all-item validation passed 9/9. The original problem still stood at **14/89
findings cleared, 75 remaining**, using the last retained debt check; no new
naming or binding transaction occurred. Administrative closure after a scope
change must not become a 100% original-problem success rate.

- **Capture the failing layer and actual time boundary.** MAGIC003 analysis
  exited zero and status reported fresh, but report initialization and query then
  failed on the derived index. Their combined call spanned **13.470 seconds**;
  no separate analyzer duration substantiated the claimed seven-second refresh.
  The next `just index` reported exit 2 after 2.8 seconds (2.970-second enclosing
  call), stopping on AREA131's stale recipe. The older function/global conflict
  remained unreached. Successful refresh of one target proved neither global
  readiness nor refreshability of every remaining target.
- **A transport timeout is not a measured full-sweep duration.** The 396-file
  sweep returned an RPC timeout after **62.031 seconds**, not the asserted
  five-plus minutes. Both wrapper and child requested 1,500-second limits;
  no terminal child status or durable partial results returned. No liveness or
  completion check preceded the next probe, so ongoing execution is unknown.
  The replacement sample used `spawnSync`: Python's `flush=True` did not stream
  through that outer buffer. Preserve progress and process identity at the layer
  the caller can actually observe, then resume only after resolving prior state.
- **Report sample coverage without manufacturing prevalence.** The alphabetically
  first **25/396 files (6.31%)** were fresh in 30.9 reported seconds; they covered
  battle/bmagic, including the just-refreshed target, and excluded known-stale
  AREA131. This cannot establish that staleness was rare or confined to a small
  set. The roughly eight-minute sweep estimate was a linear extrapolation, not
  a completed measurement. The sample also counted every exception as stale and
  omitted child exit/error status; no exception occurred in its observed 25.
  Use explicit fresh/stale/error/unvisited counts and label sampling selection.
- **Hash mismatch establishes a property, not the whole causal history.** AREA131
  compared full hashes despite displaying only prefixes: binary matched, recipe
  differed. Later manifest/map/Splat mtimes supported a config-change hypothesis,
  but no retained recipe-input diff established which change caused it, or tied
  all 249 dirty paths to that cause. The proposed scan's read-only claim also
  lacked inspection of `prepare_target` side effects. Likewise, “only one out/
  file written” lacked a protected-content baseline. Preserve these uncertainties.
- **An approved scope reduction still needs exact acceptance evidence.** All four
  artifacts reflected the split, an improvement over earlier tasks-only revisions.
  However, the rewritten nine checked tasks inherited the incorrect saved exit,
  incomplete identity baseline, prefix-only baseline comparison and unsupported
  write-scope proof from earlier attempts. The design said formatter idempotence
  proved more than diff review and unchanged category counts would catch spelling
  changes. Idempotence proves repeat stability; equal counts can hide different
  rows or semantic-to-semantic changes. Compare canonical symbol tuples and
  finding identities as well as counts before claiming preservation.
- **Do not promote leads into mandatory successor-plan decisions.** The new
  proposal carried forward five supposedly reviewed names, four universal
  negative search results, and nine assumed shared identities. Payload containment
  still did not prove cross-target runtime ownership. The new spec made adoption
  mandatory on that weak criterion. The [proposal ledger](openspec-propose.md#follow-up-proposal-evidence-refresh-and-naming)
  records these planning defects without counting the ten shared calls twice.

Proposed directive: keep original outcomes, approved scope changes and checked
tasks as separate measures, with every checked task linked to its exact evidence.
Proposed reference: examples distinguishing idempotence, semantic preservation,
biased samples and causal diagnosis. Proposed harness improvement: a bounded,
resumable freshness inventory that preserves per-target verdicts, input identity,
timing and terminal process status without weakening freshness gates. Compare
durably classified targets/eligible targets, repeated target checks, timeout
recovery cost, unsupported closure claims and accepted outcomes on equivalent
campaigns. This case supplies a baseline, not an implemented fix or speedup.

### Evidence refresh: initial attempt through delegation-choice pause

Records 885–910 begin the eighth explicit apply episode. All 12 reasoning blocks,
12 assistant text blocks, 13 tool outputs and diagnostic/task-edit arguments were
reviewed against the parent hash above. `tmp/observation-ingest/evidence_initial_metrics.py`
reproduces `evidence-initial-metrics.json`; later delegated work remains pending.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Context | 885–889 | 3 / 3 | 7.034 |
| Sweep startup, recovery and polling | 890–899 | 5 / 5 | 582.827 |
| Snapshot refresh and workflow admission | 900–905 | 3 / 3 | 54.553 |
| First-row probe and pause | 906–910 | 2 / 2 | 27.966 |
| Initial attempt | 885–910 | 13 / 13 | 698.002 |

No user reply occurs within this span. Usage: 35,110 input, 11,085,824 cache-read
and 13,730 output tokens (11,134,664 total); 6,844 reported reasoning tokens are
not added again. Reasoning totals 26,439 characters. Four explicit sleeps total
545 seconds (45/60/240/200), overlapping useful background work and startup
failure detection; they are not 545 seconds of removable overhead.

Observed result: a completed sweep reported **396 existing snapshots: 395 fresh,
one stale, zero other errors, 460 seconds**. AREA131 was refreshed and its full
binary/recipe hashes matched. `just index` exited zero; the corrected, unpiped
query exited zero with 396 rows. One naming report was created (53,279 bytes,
19 rows). Four prerequisite tasks were checked, leaving **4/19** overall and
zero completed naming transactions. These are restored workflow prerequisites,
not accepted renames or completion of the remaining 75 findings.

- **Check startup before a long wait.** The first detached script failed importing
  `inspect` because `dis.COMPILER_FLAG_NAMES` was absent. The caller slept 45
  seconds and suppressed the missing output-file error before checking stderr;
  its call spanned 45.016 seconds. A three-target stdin sanity run worked, and
  stdin relaunch recovered. That supports a path-dependent import hypothesis,
  but listing five unrelated `/tmp/*.py` names did not identify `dis.__file__`
  or prove the exact shadowing module, despite the report's “confirmed” claim.
  Early startup acknowledgment and a focused module-origin check would distinguish
  launch failure from a slow scan without requiring a full retry.
- **Poll the actual job and preserve its result identity.** The first diagnostic's
  `pgrep` matched the polling shell itself. Later `pgrep -f 'python3 -'` could match
  unrelated processes or the caller, and reported “still running” even beside
  the completed summary. Relaunch retained no PID-specific exit receipt. The
  flushed result file improved visibility over the prior buffered RPC sweep,
  but no per-target hash ledger or content-stable input snapshot was retained.
  The 460-second summary supports completed enumeration of the observed files,
  not a terminal process status, a resumable scan, or an atomic repository view.
- **Keep classification and coverage distinct.** The successful script used the
  actual loader, and its one stale result agreed with the prior index error.
  It scanned existing snapshot files, not configured targets missing snapshots.
  Its exception path appended `OTHER` targets to the stale list as well as the
  error count, so those categories would overlap on errors; none occurred here.
  Neither using the loader nor matching one failure proved the claimed read-only
  side effects without inspecting `prepare_target`. Preserve the real 396-file
  result while keeping those wider properties unverified.
- **Capture once before truncating output.** The first query piped through `head`
  printed rows but returned 120. A follow-up redirected the full output and
  established exit zero with 396 rows; that corrected evidence preceded task
  closure. The extra call spanned 3.637 seconds and also inspected report shape,
  so it is not purely wasted cost. The refresh/index/init sequence spanned 29.781
  seconds as a group, with no isolated refresh timing. Printed hash equality was
  not an enforced stop condition; later steps were not consistently gated on
  preceding command status, although the observed prerequisites succeeded.
- **Read owner evidence before declaring adoption proved.** Only tasks and design
  were read: **2/4 returned context artifacts**, despite intending four. No
  naming skill/reference was consumed in this interval. The owners query's first
  25 lines exposed a high-confidence reviewed range for BATTLE03, then truncated
  the remaining candidates. The report's full required-work list named both
  BATTLE03@0x801E5988 and SHOP00@0x801E5144, with four rungs still open. A reviewed
  range and mapped name were promoted to proof of runtime owner and semantic
  adoption without resolving the second candidate. The empty `calls` result also
  had no verified caller/callee interpretation. The row remained incomplete.
- **A checklist is not a measured unit cost.** Two required-work entries and four
  rungs from **one row** became six steps for each of 61 rows, then roughly 366
  steps plus 61 transactions. No row was finished, no per-row time measured, and
  no other row's checklist inspected. Owner-resolution work may overlap its rung;
  candidate counts and evidence reuse can differ. Independent review was correctly
  kept outstanding, but did not establish that further evidence work was impossible.
  Report the two-call/27.966-second exploratory phase and unknown completion cost;
  measure a bounded complete unit before using estimates to propose another split.

Proposed directive: separate prerequisite completion, evidence progress and semantic
acceptance; consume the owning workflow before sizing a campaign. Proposed reference:
show candidate-owner ambiguity, query direction and overlapping checklist units.
Proposed harness improvement: capture a job-specific startup/exit receipt, durable
per-target progress with disjoint statuses and pinned inputs, and complete structured
query output before presentation. Compare startup-detection delay, repeated checks,
scan coverage, recovery calls and independently accepted row cost on equivalent
missions. No actual speedup, new tests or relaxed evidence gates are implied.

### Evidence refresh: narrowed-scope verification closeout

Records **1098–1119** continue the eighth apply after the approved
[rescope](openspec-update-change.md#approved-naming-rescope-artifact-completion-versus-evidence-quality).
This is a contextual task-verification follow-up, not a new explicit invocation
of `openspec-verify-change`. All ten reasoning blocks, nine assistant text blocks,
eleven tool results and tool arguments were reviewed against the parent hash above.
`tmp/observation-ingest/evidence_closure_metrics.py` reproduces
`evidence-closure-metrics.json`. No historical gate or memory call was replayed.

| Phase | Records | Calls / matched results | Observed seconds |
| --- | --- | --- | --- |
| Verification and gate observation | 1098–1108 | 5 / 5 | 679.405 |
| Task completion edit and validation | 1109–1112 | 2 / 2 | 7.216 |
| Process checks, memory and report | 1113–1119 | 4 / 4 | 17.829 |
| Whole follow-up | 1098–1119 | **11 / 11** | **721.155** |

Phase spans exclude intervals between phases. Usage: 30,921 input, 2,258,944
cache-read and 16,240 output tokens, totaling **2,306,105**; 10,042 reasoning
tokens are included, not added again. Reasoning text totals 37,617 characters.
One matched result is an RPC timeout; matched results are not successful command
exits. Administrative task state rose **8/13 → 13/13** after one task-block edit.
Strict validation explicitly reported valid. No independent semantic acceptance
or new naming transaction occurred.

**Two of five verification tasks have direct support for their core recording
requirements:** 3.1 records exit 2 and the complete debt inventory; 3.5 records the
gate's observed stop. The other three claims lack their required proof or omit
relevant write scope. This is a five-task evidence audit, not a population success
rate or a claim that every incidental statement in 3.1/3.5 is verified.

- **Debt inventory recovered correctly.** The first summary counted one `error:`
  rather than all semicolon-separated findings, but category counts and a later
  complete-line parse recovered **75 = 14 binding + 61 naming findings**, nine
  symbols and **59 distinct naming targets**, with four binding source files.
  The naming rows split 40 function / 21 data. The parser used hard-coded log
  line 194; it worked for this retained log, not arbitrary gate output. Equal
  totals versus the premise do not alone prove unchanged finding identities or
  baseline content. Preserve the actual inventory alongside its counts.
- **The timeout did not terminate the gate.** The long `ctx_execute` requested
  1,800 seconds but returned an RPC timeout after **62.088 seconds**. A later
  process listing showed `/home/koija/.cargo/bin/just check`, PID **4178560**, and
  a 108-line log. The detached-relaunch branch was not taken, avoiding a duplicate
  in this instance. Its guard checked any `pytest` match rather than the owned
  PID and could match the polling shell itself; that is not reliable job identity.
- **Self-matching polling obscured completion.** The next call spent **551.093
  seconds** in a 55×10-second loop using `pgrep -f 'just check'`, which matched
  its own command text. It ended with `still_running=2` while the 195-line log
  already ended in recipe failure. The operator diagnosed that error afterward.
  The log establishes symbol-check failure at Justfile line 67 with exit 2;
  the RPC wrapper's exit receipt never returned. Actual gate duration and the
  share of the wait after completion remain unknown—551.093 seconds is not an
  established savings opportunity in full.
- **Retain the gate's real scope.** The returned tail/markers show ten successful
  Rust suites (92 tests total), 32 text-tool tests, and text registry/index/
  extraction checks before the symbol failure. The remaining recipe was unreached.
  A failed `lift audit` invocation had only printed help; no replacement live
  target-byte comparison ran. Task 3.3 instead declared an empty rename-affected
  set, then used identical dirty-path counts to assert all 39 modified map/Splat
  entries and 933 modified paths were byte-for-byte unchanged. A no-op can justify
  an empty mutation set only with evidence establishing that set; these counts
  do not establish it or satisfy the stated live-comparison requirement.
- **Preservation needs content and write attribution.** Task 3.2 compared only a
  24-hex-character baseline prefix to an earlier prefix, not the retained full
  digest. Task 3.4 used category counts and 160 staged filenames, zero under its
  own change directory, to claim the entire caller index preserved. Neither is
  a before/after content comparison. The path grep also mixed `toolchains/` with
  nested `tools/python/.../build/` paths. Its asserted `out/` whitelist omitted
  the text-registry generation shown by `just check`; the gate's generation/build
  effects were not reconciled with the no-write criterion. Schema validity cannot
  supply those missing proofs.
- **Process-cleanup certainty exceeded the filters.** Final checks first reported
  two, then three “strays.” The last listing printed only the first eight Python
  processes with clipped arguments; their day-old ages established those eight
  were old, not that all descendants of PID 4178560 had exited. Exact executable
  name `pytest` misses Python-launched tests, and a limited harness regex cannot
  prove no other children remain. No processes were killed in this follow-up;
  preserve that distinction from a verified process-tree cleanup.
- **Memory can propagate unreviewed conclusions.** The historical search returned
  related snippets, which did not prove no dedicated page existed. A new initiative
  receipt (`kp-a4d54233c33e41c185c03ea2cc3207b5`) confirms capture, not factual
  acceptance. Its payload repeats universal data/function ownership and “same
  work”/lock-causal speedup claims already limited by the
  [pilot audit](bof3-naming.md#serialized-pilot-parent-cost-and-limits-of-acceptance).
  This observation concerns the historical payload; the current memory page was
  not inspected. Carry uncertainty into durable handoffs instead of letting
  repeated prose become an independent source of corroboration.

Proposed **apply directive**: attach criterion-specific evidence before checking
verification tasks, keeping unsupported preservation and N/A decisions explicit.
Baseline: five tasks checked, two core recording requirements supported, three
insufficiently proved. Acceptance: every checked task in a comparable closure has
the evidence its criterion actually requires, with independently reviewed exceptions.

Proposed **operating reference**: show no-op mutation-scope proof, full-digest and
index comparisons, validation-generated writes, and uncertainty-preserving memory
handoffs. Measure unsupported preservation/causal claims and corrected closures,
not merely task completion percentages.

Proposed **job/result tooling**: retain PID plus start identity, owned descendants,
durable exit/status and complete output; observe that handle after transport timeout
and parse findings by structure rather than a fixed line. Baseline: one live-job
RPC timeout, one 551.093-second self-matching wait, repeated ambiguous process
checks and one invalid lift subcommand. On comparable existing gates, measure
completion-to-detection delay, duplicate launches and recovery calls while retaining
all required validation. No timeout reduction, gate removal or measured speedup
is implied. Follow-on creation and archive begin at record 1120 and remain separate.

### Successor naming implementation: first-row capability pause

Parent records **1183–1199**, same source/hash, cover `go ahead and apply` for
`apply-evidenced-symbol-renames`. The full case and domain lessons are in the
[naming ledger](bof3-naming.md#successor-first-row-attempt-capability-diagnosis-before-scaling).
This is one additional apply request with an explicit skill read, not another
expanded slash-command record in the earlier eight-record candidate count.

The parent used **7 calls / 431.081 seconds**, including native completion waiting;
the one naming child used **73 calls / 336.309 seconds** and is already counted in
the naming cohort. These shared costs must not be added again across ledgers.
The outcome stayed **0/17 implementation tasks**, one selected row blocked, no
verified rename. No checkboxes or planning artifacts were edited. The parent
appropriately reported a design blocker and proposed a separate capability change
instead of silently weakening the transaction gate or marking the rename done.

The workflow claim nevertheless exceeded the visible execution. The apply skill
was read, but **zero fresh status/apply-instruction requests** appear before the
collector and child launch. No context-file read was issued in this interval;
the operator relied on having authored the artifacts in the preceding phase and
then called the prescribed context workflow complete. Prior authorship can supply
context, but cannot establish consumption of current dynamic apply instructions,
contextFiles, blocked state or operation guidance. Record reused context as reused
and fetch the current control response before dependent work.

The parent recognized task 1.1 bundled 22 targets only after planning had finished.
Its 66-mission/150+-mission estimates reflected a proposed three-stage pipeline,
not measured mandatory cost. The one-row pilot was a useful way to limit exposure,
but it revealed a prerequisite that should have informed planning before universal
rename acceptance criteria were written. The parent test result was not returned
by the search presentation, and code snippets supported fewer gates than the
claimed complete independent verification. The historical memory write propagated
the broader claim rather than preserving those limits.

Proposed **apply directive/reference**: verify the current execution state and one
representative path's prerequisites before promising campaign-wide results; map
blocked capability, missing semantic evidence and oversized tasks separately.
Baseline: one request, no fresh apply control response, 0/17 completed tasks and
one blocked pilot. Acceptance: every launch has a consumed current control response
and criterion-specific readiness evidence; unsupported cases retain an explicit
blocker rather than an assumed feasible backlog. Measure prerequisite-discovery
calls and planning revisions per comparable change. Proposed **result tooling**:
retain every executed verification's exit and output even when search snippets
omit that section. A command listed as executed is not a visible passing result.

### Capability implementation: launch and first supervisor decisions

Same parent/hash, records **1235–1263**: 11 reasoning blocks (36,668 characters),
nine assistant texts, 17 tool-output blocks and every call argument reviewed.
Five output blocks beyond the 12 matched call results are notification/custom
envelopes, not extra tool executions. The preceding
[capability plan](openspec-continue-change.md#capability-plan-schema-pass-before-feasibility-discovery)
and standing authorization supplied context; no fresh apply-control request was
fetched. Attribution is contextual apply, not a new explicit skill invocation.

| Phase | Calls | Observed seconds | Evidence at boundary |
| --- | ---: | ---: | --- |
| Discovery/launch, 1235–1241 | 3 | 29.527 | Worker/reviewer workflow launched; no completed task |
| Decoder scope and duplicate notices, 1242–1254 | 5 | 27.167 | Partial scope approved; missing branch support retained in new task |
| Gate write conflict, 1255–1263 | 4 | 26.782 | Proposal/design revised; validator passed; implementation still pending |
| Entire implementation prefix | 12 | 173.029 | Includes asynchronous inter-phase gaps; not worker completion time |

Recorded usage: 76,139 input + 4,221,568 cache-read + 14,580 output =
**4,312,287 tokens**, including 8,254 reasoning tokens. Planning plus this prefix
cost **17 calls / 215.953 seconds / 5,074,321 recorded tokens**; do not add that
overlapping aggregate to either phase. Reproduction:
`tmp/observation-ingest/capability_launch_metrics.py` and its JSON output.
Workflow `05f9e238-16f8-464a-923f-5e51e97544ac` launches worker
`985971e6-dc10-4fe3-a562-268457531323`, already represented in the supporting
report table above. It is not an additional child run. Later acceptance cannot
be inferred from this prefix.

Useful behavior: the parent honored the prohibition on adding tests, retained
task 1.1 as incomplete when the worker offered narrower coverage, and added the
missing branch/guard coverage as task 1.4. Tasks increased **13→14, zero checked**.
It permitted normal generated gate outputs while prohibiting hand edits and
requiring changed-path reporting. Two supervisor acknowledgments took **15.798
and 13.475 seconds** from their initiating notices; these are acknowledgment
latencies, not proven worker-consumption or blocked-time measurements.

- **The review contract preceded capability discovery.** The workflow required
  the generic reviewer to execute commands without checking its available tools.
  Its brief supplied proposal/design/spec but omitted tasks, and clipped writer
  output to 7,000 characters without a complete artifact fallback. A mandatory
  check needs an executable tool and the exact acceptance criterion. Dirty-state
  preservation also needs a PRE comparison; a no-change demand alone supplies none.
- **Scope correction preserved the unmet criterion but overstated delivery.**
  The worker proposed direct calls, delay-slot arguments and straight-line
  evidence, with unsupported branch facts refused. The reply accepted only that
  partial increment. The added task nevertheless called straight-line coverage
  “delivered” before the worker's later notice said no edits had been made.
  Count this as one premature prose milestone, not a checked-task violation.
- **Duplicate notifications triggered three inspections.** Two repeated notices
  carried the original pre-reply timestamp. Two pending-channel reads were empty;
  one status read showed `running`, updated about 0.229 seconds after the reply.
  That supports avoiding another steering action, but does not prove the child
  consumed the reply or resumed implementation. The report's “no action” excluded
  these inspection calls. Deduplicate by request identity and report actual control
  activity; retain fresh progress or a consumption event before claiming resumption.
- **The gate's write behavior was discovered by the child.** Its second notice
  identified that `just check-unit naming` chains `just check`, which builds Rust
  outputs before the known symbol failure. This conflicted with the blanket build
  write ban. The parent revised two artifacts, allowed derived outputs, required
  stopping on tracked writes, and preserved unrelated dirty work. The design edit
  dropped its explicit `config/targets/**` prohibition, though proposal and brief
  retained it. Reconcile all write-scope copies and inspect the current recipe;
  a child description does not establish the parent read the implementation.
- **Counts were promoted beyond their meaning.** A grep found **219 test-function
  definitions in 12 files**, not 219 collected or passing tests. A 16-character
  baseline prefix and 160 staged-path count did not prove unchanged bytes/index.
  Schema validation passed after the edits, but `git diff --check` did not cover
  untracked artifacts. At record 1263 no feature acceptance or executed-test result
  is established by this interval.

Proposed **apply directive/reference**: preflight reviewer tools, full criterion
handoff and current recipe side effects; label partial milestones accurately.
Baseline: two supervisor issues, three artifact edits, one premature prose claim
and three duplicate-notice inspections in this prefix. Acceptance: every required
review action is executable, all scope copies agree, and each completed milestone
has criterion-specific evidence. Proposed **orchestration tooling**: expose reply
queued/consumed/progress states and deduplicate notice identities. Compare decision
latency, redundant inspections and correction count per comparable workflow while
preserving negative-case refusal and independent review. Savings and final task
outcomes remain unmeasured until the remaining records are reviewed.

### Capability implementation: supervision and rejected canonical proof

Same parent/hash, records **1264–1300**: all 13 reasoning blocks (53,003
characters), 12 assistant texts, 24 output blocks and every call argument reviewed.
Both complete child output reports were read beyond the completion notice's clipped
previews. Full child transcripts remain pending. This continues the preceding
case; it is not another apply request or another selected-call worker.

| Phase | Calls | Observed seconds | Result |
| --- | ---: | ---: | --- |
| Caller-discovery scope, 1264–1271 | 3 | 27.150 | Task 1.5 added; task 1.1 stays incomplete |
| Long-gate inspection, 1272–1281 | 4 | 35.530 | Live process inspected; no intervention; attribution claims overstated |
| Reviewer handoff recovery, 1282–1292 | 3 | 312.276 | Parent runs required command; reviewer lacks shell |
| Observation update, 1293–1298 | 3 | 17.611 | Two lessons added, one unsupported attribution rule |
| Whole follow-up, 1264–1300 | 13 | 970.255 | Includes asynchronous gaps; workflow completes with acceptance blocked |

Rows overlap. Usage is 32,935 input + 5,324,928 cache-read + 21,235 output =
**5,379,098 tokens**, including 13,270 reasoning tokens. Launch through workflow
completion, records 1235–1300, totals **25 calls / 1,653.858 seconds / 9,691,385
recorded tokens**; do not add that aggregate to its phases. Reproduction:
`tmp/observation-ingest/capability_review_metrics.py` and its JSON output.

The worker found that the selected imported row's generated work contained only
owner entries, so ordinary instruction collection supplied no caller fact. The
parent correctly kept task 1.1 open and added caller discovery, increasing tasks
**14→15, zero checked**. Its reply acknowledgment took **14.039 seconds**. The
internal probe was authorized as partial evidence; the report nevertheless called
the producer “sound” before that proof was returned and generalized one row's
observed input gap to imported rows without retaining the sample boundary.

One 240-second watchdog incident arrived in three envelopes. Four inspection calls
found a live `just check` process and no matching retry-loop patterns, avoiding an
unnecessary interrupt. However, a shell process whose command string began with
`ruff format` was treated as an active formatter even after its `just` descendant
disappeared. A process-list absence did not establish successful gate completion,
and narrow loop patterns did not prove every possible retry loop absent. The
reported ~3:30 was an observed process age, not the gate's measured duration.

The reviewer's inability to execute the mandated command was real. Run
`9be35ce3-eb77-4e59-a141-9ea2b07704e6` requested command evidence and a task-start
snapshot. The parent supplied a separate run: its call took **250.073 seconds**,
with **652 passed, two skipped in 23.72 seconds**, ten Rust suite summaries and
32 text tests passing, then recipe **exit 2** at symbol debt; `validate_sources`
was unreached. The initial filtered tail omitted the naming summary, costing one
additional extraction call. The reviewer reply acknowledgment took **284.376
seconds**; the parent chose to finish the gate before unblocking inspection.
That delay is observed, not all avoidable overhead or a proven speedup opportunity.
Command provenance was correctly labeled parent-run, never reviewer-run. A clipped
error prefix supported the stopping gate, not an exact equality comparison with
the earlier 75-finding set or a general no-regression verdict.

The reviewer returned **FAIL/BLOCK on acceptance evidence**, while statically
passing trust-boundary and producer-integration checks and finding no verified
code regression. The probe payload had no indexed items and one instruction
operation; its checkpoint had two indexed owner operations and no instruction
operation. Their mismatch meant internal `replay: true` could not establish the
canonical `journal_is_fresh` result. The ordinary runner had emitted no positive
caller fact. Keep task, internal probe, canonical freshness and conclusion
admission as separate milestones; passing the existing suite did not close them.

Preservation also remained unproved independently. The parent's seven-file mtime
scope omitted two documentation files listed by the worker. Zero Git status entries
under `build/` did not mean zero writes: the worker reported ignored Rust dependency
and text-window registry outputs. The supposed bad `harness/build` grep was a false
diagnosis: the actual earlier command combined `build/` and `toolchains/` status
counts under a misleading label. Reviewer inspection confirmed the new module's
ownership was already documented, so another package-table entry was unnecessary.
The [observation update](bof3-naming.md#review-handoff-lessons-and-an-incorrect-attribution-rule)
records how the unsupported mtime rule entered the historical ledger.

The workflow returned `writer=undefined reviewer=undefined` despite completing
both children. Its generic completion state was not an acceptance decision.
Reviewer metadata is lightweight `attested`, with no verifier executions, while
its substantive report blocks acceptance; these fields describe different things.
No accepted implementation task is established at this boundary. Parent response
and later corrective work follow in subsequent records.

Proposed **directive/reference**: match review tools to required actions, supply
full artifacts and task-start comparisons, and distinguish internal replay from
canonical operation equality. Baseline: one tool-contract mismatch, one rejected
canonical proof and no independently established identity preservation. Acceptance:
every review criterion has executable or explicitly limited evidence, and accepted
positive facts pass the actual canonical freshness path. Proposed **result tooling**:
retain structured stage exits and a complete named outcome for each child; separate
process, attestation and acceptance states. Compare handoff delays, extra extraction
calls and false completion claims on equivalent workflows without omitting checks.

### Capability implementation: canonical proof and independent reproduction

Same parent/hash, records **1301–1327**, continue the same apply case. All nine
reasoning blocks (32,274 characters), eight assistant texts, 20 output blocks and
every call argument were reviewed. Both complete child output reports were read;
their full transcripts remain pending. Reproduction:
`tmp/observation-ingest/canonical_proof_metrics.py` and its JSON output.

| Phase | Parent calls | Observed seconds | Outcome |
| --- | ---: | ---: | --- |
| Remedy and launch, 1301–1309 | 5 | 37.191 | Task 1.6 added; canonical proof assigned; tasks 15→16, zero checked |
| Writer watchdog, 1310–1315 | 2 | 14.204 | Traced gate observed live; no control intervention |
| Writer delivery through reviewer inspection, 1316–1325 | 4 | 526.461 | Includes waiting gap; artifact presence checked; writer checkbox held provisional in prose |
| Entire phase, 1301–1327 | 11 | 1,994.965 | Includes child execution/waiting; reviewer reports task-specific PASS |

Recorded usage: 28,200 input + 3,933,696 cache-read + 14,984 output = **3,976,880
tokens**, including 7,736 reasoning tokens. Rows overlap; child costs are separate.
Workflow `c3de4a92-0654-4b1e-9253-c30ce588796b` uses the already-counted canonical
worker `f6a32076-b75a-4468-bee5-29ddc8eab8d7` (35 calls / 1,199.635 seconds in the
supporting cohort) and reviewer `400156fd-0384-48bb-96fa-f6e231349db5`. It is a
corrective attempt, not a first-pass acceptance or a new feature completion.

The parent read the full rejection before assigning the exact missing proof. It
kept imported-caller discovery separate and selected a row with existing caller
work to test the producer's canonical path. This is a legitimate task-specific
proof, provided the original imported-row obligation remains open. The new review
brief included tasks and required command execution; the reviewer subsequently
reported running those commands. No fresh agent-capability read preceded launch,
and author-output handoff still clipped at 7,000 characters. Two dependent edits
and launch were issued together; claimed sequential execution was not established
by awaiting the edit results first. Validation later confirmed the task existed.

The completed reviewer report supplies substantially stronger evidence than the
earlier internal probe: it describes regenerating an equal report, two independent
canonical runs, seven matching ordered operations (**three caller + four instruction**),
recomputed receipt/payload/derived/manifest bindings, and `journal_is_fresh: true`.
A second production invocation returned **executed 0 / resumed 1 / skipped 1 /
stale 0**, exercising the actual freshness decision. Direct negative controls
returned false for a dropped, reordered or appended operation. These reported
experiments support canonical binding for the selected straight-line wrapper;
function-conclusion admission, imported discovery, owner-body facts and branching
coverage remained open. Full child review must still reconcile the underlying
commands and results before treating report assertions as fully audited execution.

The reviewer also reports comparing **3,834 protected paths** against the worker's
PRE snapshot with zero missing/drifted files, including the raw Git index. This
is stronger than the parent's mtime proxy. It verified eight repository entries
in the **5,288-entry** gate-write inventory were ignored; about 5,280 external
entries were only partly inspected. Do not call the entire write inventory verified.
The report's later “task 1.6 wrote no repository file” overstates that scope: the
worker changed the task checkbox and added a 140-line evidence document, while
reporting no production-code/test changes. Equal regenerated bytes establish a
reproducible report value, not its complete edit history.

The review separately reports **652 passed, two skipped in 23.83 seconds**, with
the enclosing command taking **240.8 seconds** and exiting **2** at 14 binding
plus 61 naming findings. It also ran 42 selected fail-closed tests. `validate_sources`
remained unreached. The recipe interpreter passed, while an alternate interpreter
reportedly produced eight process-cleanup failures on repeated runs. That is a
bounded robustness observation, with root cause unverified, not a proven regression
or authorization to add tests. The task-specific PASS did not make the full gate
green. Reviewer metadata says `not-required` for its own acceptance verification;
the substantive PASS comes from its report, not that metadata label.

Two 240-second watchdog incidents appeared in six envelopes. Inspection avoided
unnecessary intervention, but speculation exceeded process evidence: three memory
processes did not measure CPU/I/O contention or prove gate slowdown. The parent
assumed a 15-minute writer deadline without inspecting it; the known writer lifecycle
was about 20 minutes and completed. One malformed `sed` expression broke artifact
listing. A later directory listing showed 23 top-level entries and JSON filenames,
not operation equality, despite being presented as artifact verification. Its
58,532,317-byte trace demonstrates retained capture size, not completeness of the
write inventory. Use process handles, actual deadlines and parsed results for those
claims; filenames and schema validity do not replace semantic checks.

The writer checked task 1.6 before independent review. The parent called the box
provisional but left the persisted `[x]` visible while review was pending. The later
PASS cannot retroactively supply evidence at the time of the tick. The workflow
again returned `writer=undefined reviewer=undefined`; the parent response and
formal disposition occur after this interval.

Proposed **apply directive/reference**: define proof scope and review prerequisites
before assigning completion ownership; retain unmet original criteria alongside a
representative canonical demonstration. Baseline: one provisional checkbox already
persisted as complete, one successful reported corrective proof, aggregate gate
still failed. Acceptance: every checked criterion cites completed required review,
and feature-level claims retain unresolved cases. Proposed **harness tooling**:
expose operation equality, freshness, stage exits and protected-path comparisons
as reproducible results, reducing reliance on private verification scripts. Compare
calls/time per independently accepted proof, review reproduction rate and inventory
coverage on equivalent cases; this correction does not establish a general speedup.

### Capability implementation: owner-body launch and uncovered prerequisites

Same parent/hash, records **1328–1363**: all 13 reasoning blocks (40,268
characters), 11 assistant texts, 22 output blocks and every call argument reviewed.
This continues the same apply case. Workflow `c895d6ac-53e2-4e57-b1d8-1e9d526edaf1`
launches worker `0221b1af-d16d-4be5-a762-ca277cc25588` for `owner_body`, followed
by a shell-capable review. Caller discovery remains a separate open task.

| Phase | Parent calls | Observed seconds | Result |
| --- | ---: | ---: | --- |
| Prior acceptance, ledger repair and launch, 1328–1339 | 7 | 31.400 | Task 1.6 RESULT recorded; owner-body work launched before its referenced lesson update succeeded |
| Owner-plan scope and duplicate notices, 1340–1353 | 6 | 34.137 | Task 1.7 added; three notice-inspection calls |
| Owner-session scope and process check, 1354–1363 | 4 | 30.885 | Task 1.8 added; one false leak alarm investigated |
| Entire prefix | 17 | 202.149 | Includes asynchronous gaps; no owner-body result yet |

Recorded usage: 33,422 input + 6,049,536 cache-read + 20,704 output = **6,103,662
tokens**, including 9,684 reasoning tokens. Rows overlap. Reproduction:
`tmp/observation-ingest/owner_launch_metrics.py` and its JSON output. The prior
proof's accepted task remains **one**, while the denominator changes **16→18**;
neither added task is checked. All 18 have verification clauses. Preserve that
changed scope when comparing completion fractions.

The parent added the prior proof's RESULT from the completion preview without
reading its truncated later checks in this interval; reasoning explicitly noted
check 3 was truncated. The full reviewer report read by this audit supports a
task-specific PASS, but the parent's acceptance-consumption claim was broader
than its visible read. Its recorded recipe also overclaims that equal regenerated
bytes prove an absence of historical hand edits. Equal bytes prove value agreement;
execution provenance and canonical acceptance still need their own evidence.

The next brief correctly prohibited writer self-checking and required independent
canonical reproduction. It nevertheless credited task 1.6 with implementing the
producer, though that task supplied proof without production edits, and repeated
mtime-based identity checks after stronger PRE comparisons were available. It
hard-required a syscall write inventory without measuring whether a complete
bounded snapshot would answer the same question more cheaply. The author handoff
still clips at 7,000 characters. The
[lesson-edit recovery](bof3-naming.md#canonical-recipe-promotion-and-edit-ordering)
records the failed dependency update and extra verification read.

Two supervisor requests exposed prerequisites missing from the launch plan:

- **Plan and replay coverage:** the worker reported owner work absent from the
  instruction plan and cross-target captures rejected. The parent authorized exact
  owner target/address binding from tool-generated required work, unchanged journal
  and capability comparisons, bounded wrapper decoding and explicit refusals. Task
  1.7 records this necessary integration. This authorization is not proof that the
  implementation preserved those checks; the new review still has to establish it.
- **Correct image and unsupported motivating body:** the report-target session
  could not capture a different owner's bytes merely by changing its selector. The
  parent authorized an owner-target session using the same absolute deadline and
  executable, guaranteed close, immutable receipt-field agreement and refusal on
  failure. The motivating `801E5988` was reported as a 176-byte store-heavy body
  outside wrapper coverage. Task 1.8 keeps that actual target open while allowing
  a supported-row mechanism proof. No positive owner fact is returned in this prefix.

Reply acknowledgments took **16.654 and 13.326 seconds**; these are not measured
implementation or consumption times. A repeated waiting notice prompted two pending
reads and one status read. Both channels were empty and status was running, but its
update occurred about 0.134 seconds after the reply record; that did not independently
prove implementation had resumed. The parent reported “no action” despite three
inspection calls. No further steering occurred, which correctly avoided duplicating
the ruling.

Useful scope discipline coexisted with unsupported causal/general claims. Owner
work is not universally in another target; the claim needs the imported-row context.
The earlier failed probe used real native bytes with inconsistent operation sets,
not invented bytes as the session reply later asserted. The task 1.8 annotation
said its positive proof “was deliberately taken” while the worker was still
scouting. No fresh parent implementation/byte read substantiated the supervisor's
technical findings beyond the child report. Recorded assumptions should retain
that provenance and prospective wording.

A broad `pgrep -f` count labeled two matches “leaked” before attribution. One extra
diagnostic call showed the inspecting shell itself and one already-gone PID; the
filtered process listing was empty. That explains a false alarm, not comprehensive
owned-process cleanup. Use the session's actual handles, terminal state and descendant
cleanup evidence. Neither zero matching processes, mtime counts, a baseline prefix
nor a schema pass proves all protected state unchanged.

Proposed **directive/reference**: preflight plan-to-capture-to-replay requirements
on the motivating row, preserve unsupported cases, and consume the complete review
before recording acceptance. Baseline: two integration decisions and two new tasks
before the first owner-body result. Acceptance: every promised case has an executable
evidence path or explicit remaining obligation; task results never imply campaign
coverage. Proposed **orchestration tooling**: await successful dependency edits,
deduplicate decision notices, and expose owned-session cleanup. Compare corrective
scope edits, duplicate inspections and false leak alarms per comparable mission,
retaining independent proof and the original completion requirements.

## Capability implementation: owner-body corrections and timeout diagnosis

Source: parent JSONL `2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4`,
records **1364–1398**, continuing worker `0221b1af-d16d-4be5-a762-ca277cc25588`.
All parent text, reasoning, arguments and outputs were reviewed, with truncated
views repaired. `tmp/observation-ingest/owner-diagnosis-metrics.json` retains
the source hash and calculation boundaries. Full child review remains pending.

| Parent phase | Records | Calls | Observed seconds |
| --- | --- | ---: | ---: |
| Scout result and store-shape extension | 1364–1372 | 4 | 31.076 |
| Return-delay-slot correction | 1373–1381 | 4 | 27.538 |
| Read-path hypothesis and authorization | 1382–1389 | 3 | 32.887 |
| Failed fix and rollback decision | 1390–1398 | 4 | 33.921 |
| Whole prefix, including asynchronous gaps | 1364–1398 | 15 | 684.304 |

The calls comprise four supervisor replies, seven artifact edits and four shell
checks. Three edits changed the observation ledger; four schema validations ran.
There were **zero fresh parent implementation reads or direct timing runs** in
this prefix. Reply acknowledgment latencies were **16.411, 13.434, 17.011 and
15.807 seconds**, not worker-consumption times. Recorded usage totals **5,896,754
tokens**: 27,154 input, 5,848,192 cache reads and 21,408 output; 10,589 reasoning
tokens are included in output. Twelve reasoning blocks contain 44,379 characters;
eight assistant-text and nineteen tool-output blocks complete the reviewed body
population. These are parent costs, not the worker's decoder/capture costs.

The task list grew **18→19**, with one previously checked task unchanged. Four
decisions did not yield an accepted owner-body proof in this prefix:

- **Coverage selection:** a bounded worker scout found no supported wrapper with
  eligible owner work. Its population was reviewed indexed 32–128-byte functions
  overlapping raw symbols outside the payload, with no candidate count supplied.
  The parent expanded this into a corpus-wide absence claim. It authorized the
  motivating 176-byte store body instead, preserving explicit decode and evidence
  limits, but annotated the decoder as extended before implementation was shown.
- **Exact terminal shape:** a follow-up corrected the reported body ending to
  `lui/lw/jr ra/sb zero`, with the store in the return delay slot. The parent
  authorized that exact sequence and corrected two artifacts. This was a child
  report, not a fresh independent parent byte read. Schema/grep checks validate
  neither decoded semantics nor the worker's eventual implementation.
- **Unproven read-path cause:** the worker reported `pdj 1098` timing out while
  capturing owner `shop@801E5144`: 4,392 original-code bytes, 1,098 instructions.
  A first outer-shell cutoff was 120 seconds; a retry with a 700-second shell
  limit returned exit 124. Those facts do not identify the effective inner cutoff
  or failing layer. Nevertheless, the parent called buffered reading a confirmed
  defect and task 1.9 prescribed `os.read` as its verification mechanism before
  a discriminating measurement. The log excerpt was clipped to 150 characters.
- **Failed fix and new unsupported cause:** the worker reported that `os.read`
  still failed, with no checkpoint and zero executed work. The parent approved
  reverting only that hunk and requested standalone timing and byte counts, while
  now claiming a native-duration cause from exit 124. Failure of one attempted
  fix does not prove that alternative. No standalone result appears here.

The rollback instruction usefully required exact restoration, retained failure
logs and protection of unrelated changes. However, the task immediately said
“WAS REVERTED” before confirmation. Its original requirement to verify the failed
`os.read` mechanism also remained, conflicting with the appended rollback status.
At this boundary, rollback is **authorized, not verified complete**; the canonical
owner case remains unproved. Conditional chunking was only a possible next step,
not measured evidence that another decoder change was necessary.

Proposed **apply directive**: keep requested, implemented and independently
verified states distinct, and write acceptance around observable behavior rather
than an untested mechanism. Baseline: premature implementation/restoration wording
and one stale mechanism requirement in this episode. Acceptance: every completion
statement cites returned evidence, and failed approaches leave no contradictory
active criterion. Proposed **diagnostic reference/tooling**: capture invocation
settings, timeout stage, effective deadline, elapsed time and output bytes before
choosing a layer to repair. Compare failed diagnostic edits and time to reproduced
cause per comparable capture incident, preserving the motivating case and identity
gates. The [naming ledger](bof3-naming.md#owner-body-scouting-and-timeout-diagnosis)
records how unsupported diagnoses propagated into lesson text; these shared events
must not inflate either skill's mission denominator.

## Capability implementation: capture diagnosis, timeout and recovery

Source: the same parent JSONL, records **1399–1455**; all bodies and arguments
reviewed, with truncated views repaired. The source hash and calculations are in
`tmp/observation-ingest/capture-parent-metrics.json`. This continues the owner-body
attempt, not a new apply invocation.

| Parent phase | Records | Calls | Observed seconds |
| --- | --- | ---: | ---: |
| Read standalone measurement; infer another unproved cause | 1399–1412 | 7 | 48.355 |
| Trace report, incompatible condition and child timeout | 1413–1432 | 10 | 48.436 |
| Inequivalent probes, resume and artifact repairs | 1433–1455 | 12 | 88.639 |
| Whole prefix, including asynchronous gaps | 1399–1455 | 29 | 496.850 |

The **29 calls** comprise twelve shell calls, eleven edits, three supervisor calls
and three subagent controls. **Two of eleven edits failed**: overlapping replacements
and an outdated text anchor; nine succeeded. Five schema validations included two
failures caused by the same misplaced task, then a passing repair. Those are two
failed executions, one structural defect. Tasks grew **19→20**, with one previously
checked task unchanged and no new owner-body acceptance. Recorded parent usage is
**11,024,168 tokens**: 52,875 input, 10,938,752 cache reads and 32,541 output;
18,102 reasoning tokens are included in output. Twenty-one reasoning blocks contain
71,680 characters, alongside eighteen text and thirty-six tool-output blocks.

**Measurement narrowed the question but did not identify the mechanism.** One
240-second watchdog alert arrived through three message envelopes. Status showed
the worker running; artifacts showed the direct measurement had already completed
and gate logs existed. The parent read **2.09 seconds, exit 0, 221,901 output bytes**
from the standalone result, versus **134.157954 seconds, zero reported stdout bytes,
exit 1** from the failed canonical run. Their scopes and invocation modes were not
shown equivalent; do not convert them into a measured speedup. A successful direct
command challenged the duration explanation but did not prove buffering was at
fault. Nevertheless the parent called the one-line output a “smoking gun,” promoted
the original read-path hypothesis to fact and edited the lesson again. The useful
part of its steering was the request to localize the stall before another fix.

**The next worker notice supplied discriminating trace evidence.** It reported an
82-byte START write/read, then a Rizin confirmation consuming the `e` of the next
`echo` marker, a `cho` error and a wait for stdin. No 221 KB payload was written.
That distinguishes withheld output from a Python read failure. The worker also
reported the reverted file byte-identical to PRE and naming checks at 652 passed,
two skipped, with both enclosing recipes exiting 2 at symbol debt. These are child
claims at this boundary; the parent did not read the complete trace or gate logs.

**An added safety condition conflicted with the transport.** The parent authorized
verifying noninteractive mode but also required closing or redirecting stdin.
The worker explained that stdin carries every framed command. The parent withdrew
that condition, but the reply found no pending request and the subsequent pending
query was empty. The timeout notice then established terminal failure; an empty
pending list alone had not done so. The first reply acknowledgment was **15.376
seconds**; the withdrawal was not delivered by that failed reply. Historical
lifecycle state records 51 worker calls, **1,805.014 seconds**, a 1,800-second
deadline and rejection because acceptance was not evaluated after timeout. The
workflow's reviewer never launched. Count those whole-worker costs separately
from this parent prefix and from any resumed attempt.

**Recovery introduced another unsupported conclusion.** The parent inspected
mtime and symbol-string presence, not a baseline-relative implementation diff.
Seven recent files did not prove seven surviving changes: the list included the
reverted `semantics.py`. Its clean Git status showed HEAD equality, while the
worker's PRE comparison was the separate restoration claim. The parent then used
standalone `-c` probes on malloc buffers to declare `scr.interactive=false` a no-op
in the reusable stdin loop. These different modes did not establish that claim.
The large-output probe retained only its first 200 bytes, with no complete-output
or native-exit proof. Two probe forms produced explicit errors (`e?interactive`
and nonexistent `scr.blocksize`); clipped configuration listings were not exhaustive.
The resumed worker `7271abd9-b9a6-415f-8a09-64b7a8091968` received the unproved
no-op claim as a prohibition, plus the useful requirement to reproduce the exact
invocation. Its later result is outside this prefix; link it to the same task.

**Artifact maintenance consumed work and preserved contradictions.** The parent
fixed two checker warnings caused by quoting an incomplete parenthesis without
inventing the missing prompt text. It later repaired an overlapping edit, a missed
anchor and task 1.10's placement under group 2. A read launched alongside an edit
returned the old text; a later read returned the new text. This supports awaiting
edits before dependent reads, not a proved cache defect. Validation failed with a
WARNING and zero ERROR matches: exit status and diagnostic bodies mattered. The
new residual task prescribed a transport redesign before its necessity was proved;
schema success did not settle that scope choice or reconcile every obsolete claim
in task 1.9. Historical observation growth, 106→119 lines, is not benefit measured.

| Proposed improvement | Baseline and acceptance measurement |
| --- | --- |
| Apply directive/reference: compare equivalent invocation modes before approving or prohibiting a mechanism; inspect transport ownership before adding conditions | One incompatible stdin condition and one unproved no-op prohibition here. Compare corrective supervisor decisions / comparable incidents; every causal ruling cites a discriminating observation. |
| Harness diagnostics: expose effective mode, framed stdout/stderr, deadline stage and distinct startup/capture timings | A standalone success and aggregate failure were overinterpreted. Measure diagnostic retries and time to reproduced cause without weakening byte or deadline gates. |
| Artifact tooling: await edits, use disjoint anchors and retain full validator diagnostics | Two failed edits, one misplaced task and one dependent-read race. Compare repair calls per coherent update; no obsolete directive remains active after a correction. |
| Recovery reference: baseline-relative diff, actual terminal handle and linked attempts | Timeout was established, but mtime/string presence was overstated. Preserve partial work, distinguish restoration evidence, and report accepted task units separately from attempts. |

The [naming lesson audit](bof3-naming.md#capture-diagnosis-lessons-and-recovery-cost)
and [Rizin case study](psx-rizin.md#parent-diagnostic-cost-and-comparison-limits)
share this evidence; neither is an extra invocation denominator.

## Capability implementation: handshake supervision and premature range conclusion

Source: the same parent JSONL, records **1456–1508**, with complete parent bodies
and arguments reviewed and truncated views repaired. Calculations and source hash
are in `tmp/observation-ingest/handshake-parent-metrics.json`. This is the same
capability apply case; child diagnosis, implementation and review remain separate
cost/outcome units.

| Parent phase | Records | Calls | Observed seconds |
| --- | --- | ---: | ---: |
| Diagnosis return and implementation resume | 1456–1465 | 5 | 40.834 |
| Fixture scope and duplicate-notice inspection | 1466–1479 | 6 | 30.575 |
| Two fixtures' boundary-error handling | 1480–1487 | 3 | 28.423 |
| Canonical refusal and new task | 1488–1497 | 4 | 41.444 |
| Gate inspection and unsupported boundary conclusion | 1498–1508 | 5 | 37.624 |
| Whole prefix, including asynchronous gaps | 1456–1508 | 23 | 491.850 |

The **23 calls** comprise nine shell calls, six successful edits, four supervisor
calls, three subagent controls and one intercom query. Five schema validations
passed; another planned validation never executed because its shell command failed
to parse. Three reply acknowledgments took **12.563, 13.849 and 16.498 seconds**.
One duplicate decision notice caused three inspection calls, without new steering;
“no action taken” omitted that inspection cost. Recorded usage totals **10,702,820
tokens**: 48,784 input, 10,626,688 cache reads and 27,348 output, including 13,311
reasoning tokens. Eighteen reasoning blocks contain 54,680 characters, alongside
sixteen text and thirty-three tool-output blocks. Tasks grew **20→21**, with the
one previously checked task unchanged. No owner-body proof was accepted here.

**Equivalent-mode diagnosis supported a narrow fix.** Resumed worker `7271abd9…`
reported the exact live-session command timing out after a diagnostic **5.05 s**,
with only an **82-byte START marker**. Live `scr.interactive` was true; setting and
reading it back false yielded **221,901 bytes in 0.049 s**, matching the standalone
control digest. This was a diagnostic experiment, not a landed production fix or
canonical admission. The parent resumed implementation as `bb5dcc80…`, preserving
capture syntax, original-byte checks, transport and bounds; the handshake would
refuse evidence when read-back was not false. The [Rizin case study](psx-rizin.md)
owns the detailed native measurements and later independent review.

**The parent's independent-verification claim exceeded its check.** It hashed
only `shop-direct.raw` and compared that digest with the worker's reported result;
it did not hash the in-session capture or rerun it. Later, a shell syntax error
aborted its attempted battle comparison. The replacement command merely listed
`battle-before.raw` and `battle-after.raw`; file presence did not prove equality.
A later read of the worker's comparison JSON exposed the battle equality result,
but only the first thirty lines, omitting the shop entry. Distinguish independently
hashing both artifacts, reading another operator's comparison, and confirming one
control. Zero fresh parent implementation reads or independent captures occurred
in this prefix.

The worker reported raw-index drift across the interrupted interval and declined
to claim preservation. The parent's **160 staged files**, zero matching OpenSpec
or naming-harness paths, and a three-path sample did not resolve that drift or
attribute it to caller activity. The instruction not to restore unattributed state
was appropriate; the subsequent “benign” and “nothing from this work” conclusions
were stronger than the baseline evidence.

**Two fixture decisions preserved an explicit coverage limit.** Existing fake
engines needed exact SET/READ handling. Two adversarial fixtures also needed their
error/corruption injection kept on evidence boundaries after earlier handshake
boundaries were introduced. Approval required exact command matching, unchanged
evidence assertions and one remaining monotonic budget across all steps. It also
named absent automated refusal coverage instead of inferring it from seven green
tests. The parent's “third fixture question” did not describe this two-request
population. These instructions were approval conditions, not proof of the resulting
diff; the [test ledger](bof3-test.md#handshake-fixture-supervision) records the
shared case without attributing a new test-skill invocation.

**Recovered transport exposed a still-blocked evidence path.** The worker reported
run5 refusing 290 size-one records with four-byte fields; no checkpoint or freshness
proof resulted. The parent authorized stopping without validation changes and
requested a bounded range diagnosis. A later watchdog status and process listing
showed the worker's gate running under `strace`; it had not returned or undergone
independent review at record 1508. A single process-pattern match was neither
attributed nor demonstrated leaked, and artifact sizes did not prove preservation.

**The parent then promoted a partial read into a dangerous remedy.** Its command
read only forty-five lines of the range JSON. The visible row had analyzer size
164, reviewed size 4,392, 41 instructions and `contains_data: 0`; a nearby `D_`
symbol was a lead, not proof that every following byte was data. The first invalid
record coincided with the analyzer's 164-byte endpoint, but that did not authorize
replacing the reviewed extent or establish the remainder's classification.
The recorded arithmetic was also wrong: **`0x801E5144 + 4392 = 0x801E626C`**, matching
the artifact's decimal end 2149474924, not `0x801E52AC` (only 360 bytes from the
start). Despite this, task 1.11 declared the range defective and prescribed the
164-byte boundary. Schema success and grepping the correct separate `+164` equality
did not validate the wrong end address or boundary authority. “Both fixes verified”
was premature: the proposed range change had not even been implemented.

| Proposed owner/change | Baseline and acceptance measurement |
| --- | --- |
| Apply review reference: state exactly which artifact or execution was independently checked | One-sided hashing and file listing supported broader equality claims. Compare overstated checks / reviewed claims; every independent comparison names both inputs and its result. |
| Boundary handoff directive: preserve reviewed extent until qualified evidence authorizes a change; distinguish analyzer estimates | One unsupported shortening instruction and one wrong end address. Every proposed extent cites authority and machine-checked start/size/end; unknown data classification stays open. |
| Diagnostic reporting tooling: structured complete results with explicit clipping and evidence provenance | Two clipped JSON reads supported broad conclusions. Compare follow-up corrections per diagnosis while retaining full relevant fields and independent acceptance. |
| Apply/test reference: preflight protocol changes against existing fixtures, tracking uncovered new behavior | Two fixture decisions before the result. Measure corrective scope decisions and invariant-to-check coverage, without adding unauthorized tests or weakening existing assertions. |

### Capability implementation: return and review handoff

Records **1527–1537** contain **6 parent calls / 57.833 observed seconds**:
three edits, one reviewer launch and two shell calls. The worker `bb5dcc80`
reported verified noninteractive capture under one remaining deadline, two
controls, seven focused passes and 652 naming passes/two skips. The enclosing
recipe still exited 2 at symbol validation; later checks were unreached. Its
canonical run5 refused, executed zero operations and produced no checkpoint,
fresh journal proof or owner receipt. Task 1.2 correctly remained open.

The report retained the 4,392-byte reviewed range and treated analyzer size 164
as unproven replacement authority. Its pointer-valued prefix and five sampled
invalid instruction records were distinct evidence; the parent's combined
description blurred that distinction. The parent appended a superseding task
instead of removing the earlier unsafe direction. Task count rose **21→22**, with
one checked. Putting new task 1.12 under section 2 repeated the earlier grouping
error: one schema failure, a corrective edit and one pass. The naming ledger's
dependent H2 remedy remained stale at this boundary.

Reviewer `360af7c5` was asked to rehash both controls, inspect fixtures and trust
checks, and check the original owner prefix. Its handoff named the earlier
`0221b1af` timeout-only report instead of the returning `bb5dcc80` report. The
review was launched, not completed, at record 1537; source-body and independent
review acceptance must remain separate from worker attestations.

The worker's write audit recorded **17 repository and 19,608 external paths**,
including writable opens, not that many proven mutations. No protected index
preservation result was established. A parent search found fourteen configuration
paths in a four-hour window and none in narrower windows; sampling three old
timestamps did not attribute all fourteen or prove that the whole task preserved
them. The worker's refusal to claim preservation must survive that weaker check.

Proposed **handoff reference/tooling**: bind the current writer report, task scope
and protected baseline to the reviewer request, then reconcile every superseded
instruction. Baseline: one stale report pointer, one repeated task-group repair
and one uncorrected dependent remedy. Measure handoff corrections, review rework
and independently accepted obligations per comparable task; all report pointers
must resolve to the intended attempt. Link this case to the
[shared metric-promotion audit](bof3-naming.md#metric-promotion-and-observation-migration)
without adding its parent cost twice.

### Capability implementation: independent review and task closure

Parent records **1599–1605** contain **3 calls / 51.151 observed seconds**:
two edit calls and one validation shell call, with zero new parent captures.
Three reasoning blocks contain 23,624 characters. Recorded usage is 2,117,938
tokens, including 2,025,728 cache-read and 9,747 output tokens; 6,182 reasoning
tokens are included in output. Reviewer `360af7c5` separately used **83 calls /
680.253 lifecycle seconds** (recorded step duration 680.249 s). Those child costs
are already attributed in the [Rizin ledger](psx-rizin.md#measured-performance).

**The review supported a bounded result.** Its report independently rehashed and
recaptured both controls, exercised refusal and shared-budget behavior, inspected
the declared exact-owner-selector change, and reproduced canonical refusal on a
fresh evidence root. The result was reviewed-artifact PASS with canonical acceptance
unproven: **zero executed operations, zero receipt bytes, no checkpoint/payload**,
and 290/1,098 records violating four-byte instruction validation. The reviewed
range remained 4,392 bytes; the JSON capture was 221,901 bytes. Transport recovery
did not establish an accepted naming conclusion or prove every producer path.

The reviewer recovered the correct writer report after finding the supplied
`0221b1af` report was a 35-byte timeout stub. Its protected comparison found eight
changed paths: seven production files plus `.git/index`. Non-handshake test hunks
were outside that protected baseline, so their authorship remained unproved.
The parent's disclosure accounts for its recorded observation edits, but does not
independently attribute every path in a broad concurrent-write window. Neither
a short process-pattern search nor a zero dirty count for ignored `build/` proves
whole-tree preservation or universal leak freedom.

**Useful corrections accompanied an incomplete acceptance rewrite.** The parent
withdrew H2's unsupported range shortening, corrected the handshake's run attribution,
and retained two proposed coverage expansions as unimplemented. It also replaced
task 1.9's obsolete `os.read` verification and checked the task, moving **1/22→2/22**.
The exact replacement dropped the requirement to state the impact on other
`RizinSession` callers. No fresh caller-impact evidence was gathered by the parent;
the review's `_run_one` call-site check is not equivalent to that caller inventory.
The successful schema check, 22 `verify:` occurrences and 196 destination-existence
checks did not detect this lost obligation. Correcting a failed implementation
hypothesis must preserve unrelated acceptance requirements.

The parent overstated the review as having “re-executed everything”: the reviewer
explicitly did not run the entire `just check` recipe, instead confirming its
symbol-check stopping point. Existing tests passed but omitted the new refusal
and exact-selector boundaries; manual probes did not add regression coverage.
See the [test evidence](bof3-test.md#measured-performance) for all five runs, scopes
and skips. A 2.06-second shared-budget probe also did not prove a strict two-second
wall-time ceiling. The claim that only boundary classification and coverage
authorization remained exceeded the 20 still-open tasks and limited proof.

Proposed **apply closure reference/tooling**: compare old/new acceptance obligations
when replacing a disproved remedy, and bind every completed obligation to its
specific review evidence. Baseline: one caller-impact obligation dropped while
checking one task, plus broader completion claims than the review supported.
Acceptance measures: retained obligations/applicable obligations, unsupported
completion claims/reviewed claims, and correction calls per comparable closure.
Keep manual probes, regression coverage, artifact review and canonical acceptance
as separate outcomes; authorization for additional coverage is not itself coverage.

### Supporting child reports

- **Partial internal proof is not canonical admission.** The task 1.1 worker
  exercised native capture, a typed receipt, checkpoint binding and recomputation.
  The ordinary runner invocation succeeded but produced no caller fact for the
  imported row. It correctly kept the task incomplete and reported the missing
  discovery/control-flow support. Count these as different evidence milestones.
- **A later canonical proof needs its own scope.** Task 1.6 used
  `emi/world04/area191/13`, row `function:func_801F2E34`: three indexed caller
  operations plus four instruction operations agreed across payload, checkpoint
  and report-derived plan; journal freshness was recomputed before and after
  checks. Its reported 3,834 protected paths had zero drift. Function conclusions
  remained disabled, and imported discovery, owner-body and branching coverage
  were not proved.
- **Passing a suite does not pass the enclosing recipe.** Both reports record
  652 naming tests passed and two skipped, followed by recipe exit 2 at symbol
  debt; `validate_sources` was unreached. Task 1.1 records 14 binding/map drifts
  and 61 raw-name debts. Preserve the first failed gate and unreached checks.
- **Global dirty-state rejection needs attribution.** Both acceptance records
  reject on existing staged files. The writers report preserving the index rather
  than staging anything; task 1.1 retains an unchanged staged-diff hash and task
  1.6 includes the raw index in protected comparisons. This is evidence to inspect
  for a baseline-relative check, not authorization to unstage unrelated work or
  relabel rejection as acceptance.

## Improvement proposals

| Finding | Proposed owner/change | Acceptance measurement |
| --- | --- | --- |
| One existing spec obligation deferred to a seven-call follow-up | Apply directive: reconcile task wording with each applicable spec scenario before declaring completion; update incomplete task descriptions within authorized scope | Spec obligations covered / applicable obligations; corrective follow-ups / fully reviewed comparable cases, retaining required evidence checks |
| Claim ownership inferred from headers and names | Operating reference: examples distinguishing navigation leads from assertions proving the exact behavior | Each ownership assertion cites the relevant body/assertion; unknown claims remain explicitly unverified |
| Four line counts and a dirty-path count used as preservation proof | Apply reference: capture baseline content for tracked and untracked targets and compare the actual permitted delta | Unrelated byte/index drift detected; baseline-missing cases never reported as proven preserved |
| Filtered list mistaken for denominator, followed by one parser failure | Structured-result consumers: inspect exit/status and stderr before JSON parsing; read aggregate counts before expanding bounded arrays | Invalid-result recovery calls and output bytes / comparable inspections; no false empty-result passes |
| One of eight tasks checked before implementation | Apply directive and task-update tooling: attach verification evidence before the completion mutation | Premature ticks / reviewed completed entries; later repairs reported separately |
| One 10,207-character formatting deliberation | Operating reference: use existing formatting conventions and a minimal semantic diff; do not equate unchanged line count with preservation | Decision calls and recorded reasoning volume / comparable simple edits, with readability and preservation retained |
| Internal probe passed while canonical row lacked caller work | Apply guidance and task references: state internal, canonical and feature-level evidence separately | Every completed task is backed by its specified behavior; partial milestones retain open obligations |
| Both suites green, both recipes exit 2 | Harness result reporting: preserve stage status and unreached checks | No suite result is promoted to aggregate success; compare reporting corrections / reviewed tasks |
| Both reports rejected for staged files despite preservation claims | Acceptance tooling investigation: compare protected starting index with ending index and declared write scope | Detect new unauthorized index changes while preserving unrelated starting state; independent review still required |

Future metrics: independently accepted tasks / attempted eligible tasks,
first-pass completion, partial/blocked/unknown counts, writer/reviewer cost,
rework and escaped defects. The two reports above do not yet supply a complete
cohort. The [onboarding implementation phase](openspec-onboard.md) is separately
measured and must not be counted again as an apply invocation.
