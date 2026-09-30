# Design

## Context

`bin/harness source symbols check` reports **75 findings — 14 `binding/map drift` and 61
`new naming debt`** over **9 symbols and 59 targets**, and it is where `just check` stops (`justfile`
line 67). The previous change restored the evidence pipeline and *recorded* what blocks these rows
rather than guessing; this change acts on that record. The measured state it inherits:

- **40 function rows**: `func_801E5988` ×22, `func_8014D978` ×11, `func_80196070` ×7.
- **21 data rows**: `D_8014932A` ×11, `D_8014933B` ×4, `D_80146384` ×2, `D_801448EB` ×2,
  `D_80146865` ×1, `D_80149328` ×1.
- **The apply gate**: `bin/harness naming prepare-transaction` refuses any row whose `rung_status` is
  not `proposed`. Across the 26 debt reports on disk the measured state is **1004 `blocked`, 0
  `proposed`** — so producing a *proposed* row is the real unit of work, not applying a rename.
- **Ownership evidence is per-kind**: `analysis query --json owners` returns a reviewed record with
  `payload_contained: 1` for functions, and `[]` for data rows in every target tested.
- **The execution failure is measured**: nine evidence children fanned out in parallel lost **9/9** to
  the 900 s agent timeout queueing on `out/.reviews.lock`, while the same work as five strictly
  serialized missions returned **5/5** inside budget.

**This change proposes no harness behaviour change.** Every step uses commands that already exist —
`naming init`, `naming evidence`, `naming prepare-transaction`, the apply/verify transaction pair, and
`source symbols check`. If a data row's disposition turns out to need a shared-map or promotion
capability the harness does not already have, that is its own change and needs the ownership review
`docs/agents/harness.md` requires — it must not be smuggled in as a side effect of this one.

## Goals / Non-Goals

**Goals:**

- Record the verified harness capability gap that blocks every naming-debt row, with code-level
  evidence rather than assertion (see Decision 8).
- Record the backlog the gap leaves, measured against the starting point (75 findings / 61 rows / 14
  drifts), so it is neither rediscovered nor mistaken for an evidence problem.
- Hand the unblocker off as its own harness change with the required ownership review.
- Keep every touched target byte-identical and the pinned baseline byte-unchanged.

**Non-Goals:**

- No rename in this change, and none attempted again: the gates reject every authored proposal, so
  renaming is not a task this change can defer — it is a task it cannot perform (see Decision 8).
- No harness capability is added here, not even a small one: the missing typed facts and capability
  registry are a harness behaviour change and belong to `enable-function-naming-conclusions` (see
  Decision 9).
- No baseline rewrite and no `baseline --write`.
- No rename performed to reduce the reported finding count, and no spelling retained for convenience.
- No naming of functions outside this change's nine debt symbols *solely* to close a drift (see
  Decision 7 — this is a flagged scope boundary, not a silent expansion).
- No game behaviour change, no new tests, no `build/` or `toolchains/` write.

## Decisions

### Decision 1: one writer at a time, because the parallel failure is measured

Every writing mission runs strictly serialized. This is not a preference: nine parallel evidence
children each recorded verbatim that they were waiting on a concurrent sibling's `naming init`, and
all nine died at the agent timeout having written reports but advanced no row. The retry loops that
followed (`retry.sh 40`, `for i in $(seq 1 60)`) were budget extension by repetition, which the
contract forbids. *Rejected:* wider fan-out with retries — it consumed the entire budget on lock
queueing and produced nothing.

### Decision 2: ownership evidence is per-kind, and a cross-target spelling is a lead

A **function** row may adopt a reviewed spelling when the owning target's payload covers the address
and its map carries that name. A **data** row needs *same-target* containment, because `owners` is
function-only. A spelling carried by a different target — `counter1` in `emi/world00/area008/13`,
`uiMode` in `emi/etc/commu00/00` — is a **lead**, requiring either a shared map or recursive proof of
identical address, content class and runtime role across every composing consumer; that is a shared
fixed-RAM promotion, not an audit-target identity. *Rejected:* treating an equal address in another
overlay as ownership — the addresses match because overlays share a load base, which proves nothing
about identity.

### Decision 3: a row must reach `proposed` before any transaction is prepared

`prepare-transaction` refuses a row that is not `proposed`, so the campaign's unit of work is closing
the typed rungs until a row becomes proposed. A `blocked` row is therefore an **expected, valid
outcome** and is reported as one — not a failure to be forced past. *Rejected:* preparing against a
`blocked` row, which the harness refuses anyway and which would substitute assertion for evidence.

### Decision 4: a data row that cannot meet the proof is ceilinged, never renamed

If no name can be evidenced, the row keeps its raw spelling and records the missing fact. This is the
decision the pilot already tested once: its no-op was independently reviewed and **passed**, and the
reviewer recorded that even a scope ordering "adopt `counter1`" would have produced zero writes.
*Rejected:* renaming to make the finding count fall. An unchecked count is worth less than an accurate
one, and the baseline must never grow to absorb the difference.

### Decision 5: bindings close only through their owning generators

The four `psyq_source` files are generated; they are regenerated by their owning generators against
reviewed names and never hand-edited. A raw `func_*`/`D_*` map entry would convert a drift into new
debt rather than clearing it. *Rejected:* hand-editing the generated files.

### Decision 6: verification is stated against the measured starting point

The starting point is **75 findings / 61 rows / 14 drifts**. An evidence ceiling does not remove a
finding — it records why a row stays raw — so the success condition is "the reported findings equal
the ceilinged rows", never "the check exits 0". Partial progress is reported as partial, with the
remaining rows named.

### Decision 7: the drift set's scope boundary is explicit

The 14 drifts name functions beyond this change's nine debt symbols (e.g. `func_801C187C`,
`func_801F2C04`, `func_801F61F0`), so a drift closes only when *its* function is named. This change
closes the drifts whose naming dependency it resolves and records the rest with their unresolved
cause. *Rejected:* silently expanding the debt set to nine symbols' worth of neighbours — if the
operator wants all 14 drifts closed, that is a scope decision to take deliberately, not a side effect.

### Decision 8: the binding limitation is a missing harness capability, not missing evidence

An implementation attempt on the first target (`emi/bmagic/magic004/03`, `function:func_801E5988` →
`clearQueuedSlotBytes`) established that **no row can reach `proposed`**, and this was verified
independently rather than taken on report. Four gates, each cited by file and line:

- `naming/validation.py` — `_reject_unsupported_semantic_facts` raises for any
  `proposed`/`exhausted` row whose capability is `None`: *"receipt, checkpoint, and manifest bytes are
  recovery evidence, not an authenticity boundary; a trusted native-output typed analyzer is
  required"*.
- `naming/equivalence.py` — `_validated_capability` returns `None` whenever `kind != "data"`, so
  **function proposals are unreachable by construction**; for data it additionally requires
  `rung_status == "exhausted"`, refusing a `proposed` data row.
- `naming/equivalence.py` — `validate_terminal_capability` blocks the unfiltered full-report gate.
- `naming/facts.py` / `naming/inventory.py` — `selected_call` and `owner_body` appear only as required
  rung and required-work descriptions; **no producer emits positive facts for them**, so two required
  imported-function rungs can never close.

`tools/python/tests/naming/test_naming_conclusion.py` asserts this rejection as intended behaviour
(`test_import_proposed_delegates_existing_proposal_validation`,
`test_import_exhausted_and_idempotent`). Observed live: `naming conclude` →
`error: receipt lacks runner-produced owner_resolution facts`, with the bound payload holding only
plain `owner` receipts and no typed facts; and `naming evidence` skipping the committed-checkpoint row
(`completed 0, skipped 10`).

*Rejected:* pursuing the renames anyway with better evidence — the gates do not inspect evidence
quality, they reject the row's class. *Also rejected:* quietly widening this change into a harness
change to make progress; that would smuggle a reviewed-capability change into a naming change, which
Decision 9 forbids.

### Decision 9: the unblocker is its own harness change with its own review

`enable-function-naming-conclusions` owns the capability work: positive typed facts for
`selected_call` and `owner_body`, and a capability registry not limited to allowlisted
`emi/battle/battle/15` data rows. Because it changes harness behaviour, it requires the ownership
review `docs/agents/harness.md` specifies. *Rejected:* patching one gate to let this change proceed —
the gates exist precisely to stop unverified semantic claims, and a targeted relaxation would trade
one blocked campaign for a corrupted one.

## Risks / Trade-offs

- [Serialized missions make the campaign slow] → accepted: it is the only shape measured to complete.
  Wall-clock is bounded per mission and reported, never extended by retry loops.
- [A `proposed` row still fails validation] → the transaction is reverted, never fixed forward, and
  the row returns to its recorded ceiling.
- [Removing the previous change's requirement silently fails] → the main-spec requirement is removed
  by a `REMOVED` delta whose header byte-matches it; the CLI does **not** validate `REMOVED` headers,
  so the check is observational — **when this change archives, `totals.removed` must be 1**.
- [Cross-target names are adopted too eagerly] → Decision 2 requires same-target containment or the
  shared-RAM proof; an empty `owners` result is never evidence.
- [The baseline absorbs the debt] → asserted byte-identical against `afe50b04b1445d83…`, and
  `baseline --write` is never run.
