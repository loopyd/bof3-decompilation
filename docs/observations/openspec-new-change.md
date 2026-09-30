# openspec-new-change observations

Change creation evidence belongs here under the
[measurement contract](INDEX.md#performance-measurement-contract). This ledger
is a coverage record while the source review is incomplete, not a zero-activity
or successful-performance claim.

## Coverage and measured performance

The 4,171 child metadata records contain no explicit selection of this skill.
Parent operations and generic children require separate attribution. The current
history scan found 56 files mentioning OpenSpec/opsx; such mentions include skill
catalogues, copied instructions and unrelated outputs, so they are not run counts.
One explicit selection is reviewed below within a combined archive/create request,
plus a contextual capability-follow-on creation.
Population outcome rates, cost distributions and accepted-plan throughput remain
unmeasured; successful scaffolding is a different outcome from a complete plan.

## Successor scaffold after naming rescope

[Parent source](../../.pi/sessions/2026-09-25T23-10-21-878Z_01a0dad5-86b6-74b4-97f0-9382ab99ade4.jsonl),
records **1120–1132**, SHA-256
`2c4e084cd00b25aa453b20018463e58fd9ed3fee5c4d7c524d863aa6d838d791`.
All retained assistant text/reasoning, tool outputs and arguments were reviewed.
The user requested successor creation and old-change archive; record 1121 selects
both skills, and both complete skill bodies returned. Metrics:
`tmp/observation-ingest/successor_creation_metrics.py` and
`successor-creation-metrics.json`.

| Unit | Calls / matched results | Observed seconds | Outcome |
| --- | --- | --- | --- |
| Shared skill reads, 1120–1123 | 2 / 2 | 8.378 | Both procedures consumed |
| Archive-specific phase, 1124–1127 | 2 / 2 | 6.249 | Native CLI archive receipt; measured in archive ledger |
| Scaffold and handoff, 1128–1132 | 2 / 2 | 10.364 | One requested local-root scaffold created, proposal ready |
| Combined request | **6 / 6** | **43.670** | Archive plus scaffold, **0/4 planning artifacts** authored |

The combined request records 251,555 input + 1,238,400 cache-read + 6,955 output
= **1,496,910 tokens**; 4,686 reasoning tokens are included in output. Six reasoning
blocks total 20,029 characters, including 6,204 debating whether to author the
proposal at record 1128. These are mixed costs, not six calls exclusively for
creation or measured avoidable overhead. Shared reads and archive work must not
be added again when aggregating the two skill ledgers.

Root discovery identified the existing repository before mutation. The CLI created
`apply-evidenced-symbol-renames`, returned its concrete root, and showed proposal
ready with specs/design/tasks blocked. The directory listing showed only
`.openspec.yaml`; reporting **0/4 artifacts** and **0/0 no-tasks** preserved those
different denominators. No duplicate or unintended root creation was observed
in this one attempt. Independent plan acceptance was not attempted.

The historical new-change skill explicitly stopped before artifact authoring, and
the operator followed that boundary. The next user said `begin draft` **26.997 s**
after the scaffold report. That delay belongs to the handoff, not CLI creation
time; whether it was necessary under broader session authorization requires that
context, not a blanket extra-confirmation rule derived from this case.

Three limitations matter for improving the handoff:

- The operator claimed archiving first was necessary to avoid duplicate capability
  declarations. It was a coherent order, but a scaffold contains no spec delta:
  either creation order could precede later capability resolution. Main-spec
  existence alone also does not decide whether a future change modifies a
  requirement, adds one within that capability, or leaves requirements untouched.
- Proposal instructions were fetched once but deliberately clipped to **900
  instruction characters / 1,200 template characters**, ending the template at
  `## `. Context/rules were not printed, and `artifactPaths` printed `null` from
  that instruction result. Thus one successful fetch is not one fully consumed
  artifact contract. The subsequent draft inherited this omission.
- The recap rendered proposal → specs → design → tasks as a linear sequence.
  The next status actually made **specs and design ready together**. Preserve the
  returned dependency graph rather than inventing a serial order. The proposed
  inputs also repeated unsupported universal ownership and lock-causal claims
  from the pilot; archiving them did not make them reviewed facts.

The [archive case](openspec-archive-change.md#eighth-case-native-archive-and-successor-creation)
owns archive evidence; the [first-artifact continuation](openspec-continue-change.md#successor-proposal-first-artifact-continuation)
owns the subsequent two calls. No double-counted completions are implied.

## Capability follow-on scaffold

Same parent/hash, records **1200–1226**: the user authorized recording the
capability gap, its separate harness change, and an improvement loop. One native
creation call made `enable-function-naming-conclusions` with `spec-driven` schema;
the returned message and correctly captured pipeline exit support creation.
Subsequent listing showed **0/0 no-tasks**. No proposal/status/instruction fetch
for this new change or artifact write occurred in the interval. The final report
correctly called it scaffolded rather than implemented; its claimed 0/4 artifact
state was not freshly returned by this change's status command.

The [mixed rescope case](openspec-update-change.md#successor-capability-rescope-and-observation-ledger-creation)
owns the **17-call/118.806-second** aggregate. Creation shares a five-call phase
with four memory requests and is not five independent creation calls. The new
scaffold names a prerequisite but contains no accepted transferred rename tasks.
Measure time to the requested actionable plan and explicit obligation transfer,
not merely directory creation; the continuing authorization did not require a
new approval to draft the next artifacts. Later planning remains outside this case.

## Evidence to retain

The onboarding change-creation phase demonstrates one successful local-root creation; it is counted only in the onboarding ledger.
See the fully reviewed [onboarding case](openspec-onboard.md) for source locations
and the distinction between workflow phases and independent skill invocations.
Do not double-count that mission here.

## Performance questions

Measure: Correctly scoped changes created / creation attempts; root/store selection errors; discovery calls and time before the first ready artifact; unintended root creation and duplicate changes.

For each episode retain source records, task scope, skill-selection evidence,
revision/context, outcome evidence and missing measurements. Compare like work;
completion of a CLI command or artifact cannot establish the broader result.

## Improvement proposals

- **Directive:** resolve the requested deliverable and existing authorization
  before the creation handoff; distinguish scaffold, proposal and complete plan.
  Measure correctly scoped deliveries / attempts, extra round trips and time to
  the requested artifact while retaining applicable approval boundaries.
- **Reference:** show successor setup against a newly archived capability without
  implying that archive-before-scaffold or a linear artifact order is mandatory.
  Require each inherited fact to retain its evidence scope; measure dependency
  errors and unsupported assumptions introduced into successor artifacts.
- **Instruction-result tooling:** provide complete required instruction/context/
  rules plus resolved output paths without arbitrary character slicing. Baseline:
  one fetched but clipped first-artifact contract. Acceptance: all required fields
  consumed before authoring, with retrieval bytes/calls measured separately from
  correctness and no skipped checks.

These are proposals, not implemented changes. No new tests, dependencies or
installed-extension changes are implied.
