# Design

## Context

See `proposal.md` — Why. Facts established during the previous change's apply, all measured:

- `bin/harness source symbols check` reports **75** findings after the formatting rows were cleared:
  40 naming (function), 21 naming (data), 14 binding/map drift. Exit 2.
- The 61 naming rows are **9 symbols**; 5 already carry a reviewed name in another target and 4 do
  not; one address has three names (`D_8014932A`).
- The binding check validates against the **composed** set (`symbols.py:96`,
  `load_target_symbols(..., psyq_space=)`), and none of the drifted bindings is in the pinned
  baseline, so raw map entries would move the finding instead of clearing it.
- `naming init` and `analysis query` fail on `stale Rizin snapshot recipe` / `stale reverse index
  snapshot`; `just index` fails on the next stale snapshot. Staleness is a recipe mismatch caused by
  post-snapshot config edits (measured on `emi/world03/area131/13`).
- The four `psyq_source` files are **generated** (`domain/symbols.py::weak_bindings_c` and
  `symbols_psyq.py`), so they must be regenerated, never hand-edited.

## Goals / Non-Goals

**Goals:**

- Restore the evidence pipeline so the naming path and analysis queries work.
- Record the gate that still blocks the remaining debt, and the disposition of each row class, so the
  follow-on renaming change starts from measured facts rather than assumptions.
- Keep every touched target byte-identical and the pinned baseline byte-unchanged.

**Non-Goals:**

- No symbol rename in this change — the nine decisions and the four `psyq_source` regenerations move
  to a follow-on change, for the reasons Decision 8 records.
- No baseline rewrite and no `baseline --write`.
- No clearing of debt the baseline already carries (`raw_functions` 9225, `raw_data` 1342,
  `raw_function_files` 540, `invalid_semantic_files` 346).
- No game behaviour change, no new tests, no `build/` or `toolchains/` write.

## Decisions

### Decision 1: this change owns the precondition, and does it first

The pipeline is the only route to 61 of the 75 rows, so the change starts by enumerating stale
snapshots and refreshing them until `just index` succeeds. *Rejected:* treating the precondition as
"someone else's work" and leaving the naming rows unplannable — the previous change's split moved
this work here precisely so it could be planned explicitly.

### Decision 2: adopt a reviewed name where ownership evidence already exists

For the 5 symbols with an existing name, the evidence is structural: the owning target's payload
covers the address, and its map already carries the reviewed name, while the referencing targets
load outside that range and merely reference it. *Rejected:* inventing a second name for code that
is already named — it would create two names for one function.

The evidence *source* differs by symbol kind, and conflating the two produces a false "no name
exists" conclusion. For **functions**, `bin/harness analysis query --json owners TARGET@0xADDRESS`
returns the reviewed record (name, `payload_contained`, reviewed range). For **data symbols** that
same query returns `[]` in every target tested — it is function-only — so a data symbol's reviewed
spelling has to be found as a map entry instead, e.g. `counter1 = 0x80149328;` in
`emi/world00/area008/13/symbols.txt` or `uiMode = 0x801448EB;` in `emi/etc/commu00/00/symbols.txt`.

A spelling carried in a *different* target from the one being repaired is a lead, not containment:
under [Identity transactions](../../../.pi/skills/bof3-naming/references/IDENTITY_TRANSACTIONS.md),
shared fixed-RAM requires either an existing shared map or recursive proof of identical address,
content class and runtime role in every composing consumer; otherwise the rename stays local to the
target that already carries it and the row remains unresolved. Per-kind evidence, not one rule, is
what makes the adopt decision sound — and `D_80146865` (3 targets) and `D_80146384` (4 targets) are
confirmed to carry *only* the raw spelling, so their absence premise holds.

### Decision 3: the 4 unnamed symbols need behavioural evidence before a name

No name anywhere means the name must come from what the function or datum does, gathered through the
naming path's own evidence steps. *Rejected:* naming from the address or the caller alone — this
repository forbids a semantic name without proven behaviour.

### Decision 4: resolve the divergent address explicitly

`D_8014932A` carries `counter2` (×2), `selectionCounter` (×1) and a raw spelling (×1). The decision
picks one on recorded evidence and states what the others were; a target that keeps a different name
records why, per the spec. *Rejected:* inheriting the majority name silently — the minority spelling
may be the more accurate one.

### Decision 5: byte identity gates every transaction, and the baseline never moves

A rename that changes a touched target's compiled bytes is rejected, not recorded as a residual, and
`config/symbol-naming-baseline.json` is asserted byte-identical at the end. *Rejected:* accepting a
near-identical result — bytes decide in this repository — and *rejected:* absorbing the rows via
`baseline --write`, which would destroy the delta the baseline exists to prove.

### Decision 6: bindings close last, through their own generator

The binding class is downstream of naming (its addresses must be composed-set-owned under the
reviewed name), so it closes after the naming transactions by regenerating the four `psyq_source`
files. *Rejected:* hand-editing those files — they are generated, and a hand edit would be reverted
by the next generator run.

### Decision 7: a closed router route is waived by a parent-frozen report token

The canonical per-target report is normally resolved by the cleanup router
(`bin/harness agent context cleanup audit-target TARGET`). Some debt targets are absent from the
campaign account — `out/reviews/plan-audit-naming/summary.json` accounts 23 targets — and for those
the router exits non-zero with *canonical naming campaign target entry is missing*. The named repair
(`naming init-all out/reviews/plan-audit-naming`) cannot run: the retained `emi/battle/battle/03`
naming history (`*.checkpoint-*` / `*.generation-*`) makes that report set non-replaceable, and
`naming reconcile TARGET` fails with the same missing-entry error.

*Decision:* where — and only where — the router refuses for that reason, the parent pins a
per-mission disposable report path under `out/reviews/` and freezes it as the mission's REPORT token
for `init`, `evidence`, `validate` and `prepare-transaction`. The path is parent-chosen, never
child-chosen, and the report, receipts and checkpoints are still produced solely by those harness
commands, so the runner remains the report's author and no JSON is hand-edited.
*Rejected:* retiring or relocating the retained `battle/03` history to make `init-all` succeed — that
destroys retained naming evidence and mass-rebuilds every target report; and blocking the symbol
outright, since a sanctioned route exists. Every waiver records the exact refusal text in the
mission's return so it stays auditable rather than becoming a silent shortcut.

### Decision 8: the pilot fixes the mission shape, and finds the apply gate still closed

The first campaign attempt fanned out nine evidence children in parallel and lost all nine to the
900 s agent timeout. The cause is worth recording because it was structural, not behavioural: every
`naming init` / `naming evidence` invocation takes `out/.reviews.lock`, so the children serialized
*invisibly* behind one lock and spent their whole budget queueing on each other — several recorded
verbatim that they were "waiting for the foreign lock" and had stopped retrying. No tracked file
changed, and no row advanced. The corrective pilot (`d920a1b9-fbeb-471b-980f-9b16675f122d`) ran
**five missions strictly serialized**, each handed its targets explicitly: evidence for`D_80149328`,
`D_80146865` and `D_80146384`, then one apply leg and one independent review. **All five returned
inside budget — zero timeouts, zero lock waits**, which is the shape the wider campaign must use.

The pilot's substantive finding is that the apply path is gated by the report's own row state, and
nothing in this change has passed that gate yet:

- `bin/harness naming prepare-transaction … --check` refuses with *"data:D_80149328 is not a proposed
  row"* (`naming/reports.py`: `rung_status != "proposed"`). No rename can be prepared — let alone
  applied — until the row is *proposed*.
- Measured across all 26 debt reports: **1004 `blocked` rows and 0 `proposed` rows.**
- The two populations differ in kind. **Function** rows carry real containment evidence:
  `analysis query --json owners emi/bmagic/magic004/03@0x801E5988` returns `payload_contained: 1`,
  `provenance: reviewed_range`, `name: clearQueuedSlotBytes`. **Data** rows do not: `describe`
  reports `contained=false, present_in_binary=false, authority=[]` for both the debt target and the
  target carrying the candidate spelling, the true payload owner being `exe/slus_004_22` with no
  symbol at that address.

So this change's premise — that the data symbols can be *renamed* here once a reviewed spelling is
adopted — does not hold as written. A cross-overlay spelling is a shared fixed-RAM promotion owned by
[Identity transactions](../../../.pi/skills/bof3-naming/references/IDENTITY_TRANSACTIONS.md#authority-ceiling),
not an audit-target identity, and an out-of-payload row with no owner symbol, no storage authority
and no initializer may have no obtainable local name at all. *Rejected:* forcing a rename to make the
campaign look productive — the pilot's no-op was independently reviewed and **passed**, and the
reviewer recorded that even a scope ordering "adopt `counter1`" would have produced zero writes.

## Risks / Trade-offs

- [The snapshot sweep is larger than measured] → enumerate before refreshing, report the count, and
  treat the sweep as its own verifiable step so its cost is visible rather than assumed.
- [A refreshed snapshot hides a real input change] → the refresh is only a re-derivation of the same
  inputs; the spec requires the refreshed recipe to match the target's current inputs, and the
  binary hash to match.
- [Adopting a name for code that differs between targets] → Decision 2 requires the owning target to
  cover the address in its own payload; a target whose code differs records its own evidence.
- [Baseline drift] → asserted byte-identical against a hash captured before the work.
- [Scope creep into baseline debt] → Non-Goals pin the scope to the 75 regressions.
- [Parallel evidence children contend on one `out/.reviews.lock`] → observed in the first campaign
  run (`f9f00993`): every `naming init` / `naming evidence` invocation takes that lock, so nine
  parallel children serialized invisibly behind it and two responded with self-written retry loops
  (`retry.sh 40`, `for i in $(seq 1 60)`) plus a lock-poll loop — budget extension by repetition,
  which the contract forbids. The parent issued a stop signal on the loop shells only, letting the
  in-flight harness command finish, and required each child to run once and report a bounded blocked
  row instead. Lesson: report-writing phases are *writes* and must be serialized under the
  repository's parallel-reads/serialized-writes rule rather than fanned out — the fan-out here was a
  parent design error, not a child fault.
