# openspec-sync-specs observations

Spec synchronization evidence belongs here under the
[measurement contract](INDEX.md#performance-measurement-contract). This ledger
is a coverage record while the source review is incomplete, not a zero-activity
or successful-performance claim.

## Coverage and measured performance

The 4,171 child metadata records contain no explicit selection of this skill.
Parent operations and generic children require separate attribution. The current
history scan found 56 files mentioning OpenSpec/opsx; such mentions include skill
catalogues, copied instructions and unrelated outputs, so they are not run counts.
No independent sync invocation is attributed yet. Seven complete inline sync phases
are reviewed in the archive cases below. Independent outcome rates, call/time
distributions and accepted-result throughput remain **unmeasured**.

## Evidence to retain

Onboarding archive copied four requirements and retained Purpose. This was archive-driven synchronization, not an independently invoked sync mission.
See the fully reviewed [onboarding case](openspec-onboard.md) for source locations
and the distinction between workflow phases and independent skill invocations.
Do not double-count that mission here.

The [first explicit archive case](openspec-archive-change.md) includes records
325–332 of the same candidate parent: four calls in an 18.881-second observed
span to retrieve rules, read the delta, write one main spec and verify it. Those
costs belong to the archive total, not an additional sync invocation. It preserved
three requirements and five scenarios. The historical comparison checked names
and Purpose; this audit separately verified normalized full-body equality from
the read delta and write arguments. No body loss was found in this case, but the
historical gate would miss changed assertions beneath unchanged headings.

The second archive's inline phase, records 466–472, used three calls in 16.011
seconds for five requirements and eight scenarios. Its normalized full-body copy
also matches, but the historical check again examined only headings and Purpose.
Both ADDED-only cases therefore preserved bodies while using an inadequate
body-preservation check: **2/2 observed copies correct; 0/2 historical verifiers
compared full bodies**. The second copied an unreconciled reduction requirement
after tasks accepted an exception. Sync fidelity and source-spec correctness need
separate outcomes. Metrics and provenance are in the archive ledger; these costs
are included there and must not be counted again as independent missions.

The third archive, records 523–531, combines inline sync, verification and movement
in four calls/22.835 seconds; isolated sync cost is unmeasured. Five requirements
and seven scenarios were copied with normalized full-body equality. The historical
comparison again omitted bodies: across the three explicit archive cases,
**3/3 observed copies preserved bodies; 0/3 historical comparators checked them**.
Its piped validation printer did not assert a successful result, despite the
claimed stop-before-move condition. See the archive ledger for the actual command
and distinction between passing observations and inadequate enforcement.

The fourth archive's records 606–612 combine sync and move in three calls/17.320
seconds, preserving four requirements/seven scenarios. Full-body comparison in
this audit matches; historical heading-only comparison and print-only validation
pipelines repeat the earlier limitations. Totals across these explicit archives:
**4/4 normalized body copies preserved; 0/4 historical comparators checked bodies**.
The initial archive assessment had not surfaced its instruction text, but this
sync step did retrieve the complete delta and current specs rules before writing.

The fifth archive's records 650–657 used four calls/19.160 seconds for sync plus
move, with rules and full delta retrieved separately. Three requirements/five
scenarios match the emitted main-spec body after declared normalization. The
historical comparator still omitted bodies and did not enforce piped validation
success. Totals: **5/5 normalized body copies preserved; 0/5 historical comparators
checked bodies**. Its unqualified non-shrink assertion also propagated the proposal's
overstatement of a fixed minimum-list-length check. Faithful transfer does not
validate the transferred claim. Detailed metrics and limits stay in the archive
ledger; this phase is not a sixth independent mission.

The sixth archive's records 743–749 used four calls/14.266 seconds for inline sync
and move. Four requirements/five scenarios match the emitted main spec after
declared normalization; the historical comparator again omitted bodies and its
piped validation did not enforce success. Across six cases, **6/6 normalized
copies preserved bodies; 0/6 historical comparators checked them**. Copying the
registry delta also retained its unsupported window-name and universal-coverage
claims. Sync fidelity is an outcome separate from specification correctness;
these costs remain part of the archive cohort.

The seventh archive's records 875–881 used four calls/15.828 seconds for sync and
move. Two requirements/four scenarios match after declared full-body normalization;
the historical comparator again checked headings/Purpose and did not enforce
piped validation success. Across all seven, **7/7 normalized copies preserved
bodies; 0/7 historical comparators checked them**. The approved normalization scope
was preserved, including its weak category-count scenario for a broader semantic
preservation requirement. A faithful merge does not strengthen that requirement's
verification. Its cost belongs to the archive cohort, not an additional mission.

Proposed sync reference/tool improvement: compare complete requirement and scenario
bodies, retaining unrelated scenarios and applying only declared normalization.
Measure body preservation / applicable bodies and mismatch detection separately
from structural validation. A correct observed merge does not establish that its
comparison method reliably detects incorrect merges.

## Performance questions

Measure: Delta requirements reconciled / applicable deltas; preserved scenarios and purpose text; conflicts, partial replacements and repeated sync attempts; calls/time per capability with unchanged requirement meaning.

For each episode retain source records, task scope, skill-selection evidence,
revision/context, outcome evidence and missing measurements. Compare like work;
completion of a CLI command or artifact cannot establish the broader result.

## Improvement status

The body-comparison proposal is supported by seven inline cases; population effects
remain unmeasured. Continue attributing independent episodes and retain conflicts,
partial merges and failures in the denominator. No new tests, dependency or
installed-extension changes are implied.
