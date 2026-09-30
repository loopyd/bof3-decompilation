# psx-emulator observations

Per-skill half of the central observation folder ([index](INDEX.md)). The skill carries
**directives**; this file carries the **measurements** behind them. Read it before designing a
mission for this skill, and after each mission add one audit row and converge the finding into a
directive in `.pi/skills/psx-emulator/` (or its agent instructions) — prose alone is not
convergence.

This is the "record progress elsewhere" the skill's own closing directive asks for: the entrypoint
keeps purpose, procedure and application; measurements land here.

## Measured performance

The 4,171-run metadata snapshot contains **zero explicit selections** of
`psx-emulator`; it does not establish zero emulator activity. Parent missions and
generic workers remain to be attributed. The reviewed case below measures
reference/inventory quality, not emulator run performance. Workers compacting
this ledger are documentation operations, not emulator invocations. The
[four-child editorial audit](bof3-docs.md#small-editorial-child-review) includes
both emulator-ledger rewrites: six calls and 183.796 summed lifecycle seconds,
with no runtime execution. It verifies surrounding-byte preservation while
showing that the unproved reachability claim survived both rewrites.

For each runtime-question cohort record mission attempts, tool calls, startup and
capture seconds, emulated frames, capture bytes, and bounded termination reason.
Measure independently answered questions / attempted questions, reproducible
captures / checked captures, failed startups and missing artifacts. Separate
setup, execution and analysis; faster emulation alone does not imply faster useful
evidence. Compare the same program, emulator revision, initial state and capture
requirements. These operational baselines remain unmeasured.

## Iteration audit log

| Iteration | Mission | Tools | Duration | Method verdict | Converged into |
| --- | --- | --- | --- | --- | --- |
| 2026-09-26 UTC | Layered guidance implementation, parent records 476–518 | 22 / 22 results | 726.440 seconds, including 413.582-second user gap | Preserved catalog membership; retrieval benefit and content preservation not proved | Supporting OpenSpec apply case below; no new directive implemented by this audit |
| 2026-09-26 UTC | Observation mechanism opt-in, parent records 1565–1576 | 7 / 7 results | 32.670 seconds observed | Ledger and links added; one placeholder repair and repeated verification; zero runtime runs | Documentation-ordering proposal below |

Add one row per mission. A row that changes nothing is itself a finding.

## Converged directives

None recorded yet. Directives for this skill live in `.pi/skills/psx-emulator/SKILL.md`, its
[references catalogue](../../.pi/skills/psx-emulator/references/CATALOG.md) and the delegated agent spec; record the pointer here when
one is added.

## Bundled harness improvements

No measured runtime bottleneck has yet been established. Reference-discovery
work should distinguish direct catalog membership from transitive reachability.
The baseline omitted 2/25 references from the front-page table; it did not prove
them unreachable through other linked pages. Inventory automation should reconcile
scripts/tests from the live source and map executable actions to maintained owners,
with exclusions explicit; the 54/53 versus 52/55 discrepancy alone proves neither
test coverage nor speed.
Runtime-tooling proposals need the operational measurements above before claiming
time savings, following the [measurement contract](INDEX.md#performance-measurement-contract).

## Important observations

The [complete apply case](openspec-apply-change.md#emulator-guidance-restructuring)
reviews all assistant text/reasoning/tool outputs in parent records 476–518.
Its source hash and reproducible metrics live there. This is an OpenSpec-owned
documentation operation, not another emulator invocation; do not add its costs
to a runtime-performance cohort.

- The new catalog directly lists BUS.md and NATIVE.md, previously absent from the
  front-page table. The earlier ledger's claim that they were unreachable was
  stronger than the historical inspection proved.
- The inventory found 54 Lua scripts and 53 tests versus the proposal's 52/55.
  Tasks recorded that correction while proposal/design remained stale. All 23
  original catalog entries and 25 reference names survived, but the required
  per-action script mapping was not recorded; Runtime/API rows were guidance,
  not established executable actions.
- Entrypoint size fell 49→33 lines; adding a 69-line catalog made their combined
  total 102. The Observe section alone occupies 18 lines, so a smaller front page
  does not prove a smaller mission read. No comparable reader/capture mission
  measured loaded tokens, calls or correct action selection.
- “Observe state” was described as not driving the machine while including bounded
  stepping. A useful layer name must preserve execution/state effects, not merely
  group related vocabulary. Existing action references remain the semantic owners.
- Script/test filename equality and older mtimes were promoted to unchanged
  behavior. No starting/ending content comparison supported that broad claim.
  The gate failed as before and was explicitly accepted as an exception; that
  decision did not turn it into a passing runtime check.

Proposed reference improvement: retain common bounds/receipt requirements, distinguish
observing from advancing execution, and evaluate layer routing on comparable
missions. Acceptance measures: correct action/owner selection, preserved required
reads and evidence gates, loaded tokens/calls, and baseline-relative protected-file
content. No operational speedup or implemented directive change is claimed.

### Observation mechanism opt-in

Parent records **1565–1579** in the pinned `01a0dad5` session contain **8 calls /
46.171 observed seconds**, including one status call for the still-running naming
reviewer. Wiring itself used **7 calls / 32.670 seconds**: a new template, its
index entry, a skill clause (**33→38 lines**), one corrective edit and verification.
The ledger population grew eight→nine; none of these actions ran the emulator or
populated its runtime-performance baseline. Recorded parent usage is 3,910,653
tokens, including 3,889,536 cache-read and 6,009 output tokens; 2,183 reasoning
tokens are included in output. Six reasoning blocks contain 8,315 characters.

The new template contained a `file:///dev/null` placeholder. The parent recognized
it before submitting its correction and link check together. That check still
saw the old link: **204 destinations / 38 files, one broken**; a subsequent check
of the same scope found zero. Thus the final narrative that the checker discovered
the defect and then triggered its fix reversed the observed decision order.
The record supports one inconsistent same-batch read, not an independently counted
“fourth” occurrence or a proven cache defect. Both checker scripts printed errors
without making broken links cause a failing exit. Only a name-line grep checked
frontmatter, and the router probe exercised naming, not emulator behavior.

Proposed **documentation procedure/tooling**: await dependent edits before checking
their result; make check failures visible in status as well as output. Baseline:
one placeholder, one corrective edit and two same-scope checks. Compare reruns per
edit and defect-detection coverage on comparable changes; every final check must
read the completed revision and report its exact scope. This shared case belongs
to documentation execution costs, not native-emulator throughput.
