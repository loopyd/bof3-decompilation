# Design

## Context

The naming-debt campaign cannot resolve a single row, and the cause was established empirically rather
than inferred. Measured on `emi/bmagic/magic004/03` / `function:func_801E5988` (2026-09-26):

- `bin/harness naming conclude …` → **exit 2**, `error: receipt lacks runner-produced owner_resolution
  facts`. The bound payload holds only plain `operation=owner` receipts with no
  `bof3.naming-evidence-facts/v1` facts (`derived.json` → `facts: []`, `conclusion_enabled: false`,
  owner ops `unavailable`).
- `bin/harness naming evidence …` → exit 0 but `rows: {completed 0, planned 0, skipped 10}`: the row is
  a committed checkpoint and is skipped, so collection cannot advance it either.
- `naming/validation.py` `_reject_unsupported_semantic_facts` raises for any `proposed`/`exhausted` row
  with no runner-recomputed capability.
- `naming/equivalence.py` `_validated_capability` returns none when `kind != "data"`, and for data
  requires `rung_status == "exhausted"` — so **no `proposed` row of either kind can be admitted**.
- `naming/facts.py` and `naming/inventory.py` declare `selected_call` and `owner_body` as required
  rungs and required work; **no producer emits them**.
- `tools/python/tests/naming/test_naming_conclusion.py` asserts the rejection as intended behaviour.

So the gate is doing its job; what is missing is capability behind it. The ten `binding/map drift`s
attached to these symbols fail for the same reason.

## Goals / Non-Goals

**Goals:**

- Give every declared required rung a producer, so an imported-function row can actually close
  `selected_call` and `owner_body`.
- Admit a `proposed` conclusion when — and only when — runner-produced facts bound to the current
  report close the row's required rungs.
- Derive the gate's allowance from the row's own kind, locality and closed rungs, so unblocking a new
  target needs no hand-maintained registry entry.
- Leave the fail-closed cases failing: stale binding, missing capability and authored-instead-of-runner
  evidence must still be refused, with tests asserting it.

**Non-Goals:**

- No relaxation of the trust boundary for convenience, and no acceptance of authored observations in
  place of runner facts.
- No naming decision, rename or map edit in this change — applying identities stays with
  `$bof3-naming`'s separate transaction scope.
- No change to the audit contract's rung set, receipt format or `--work-deadline` semantics.
- No write to build or toolchain **sources or configuration**, and no hand-edit of anything under
  `build/` or `toolchains/`. Derived output that the project's own gates produce while running (for
  example cargo's `build/tools/rust/bof3-text` target-dir) is expected, is not a deliverable, and is
  reported rather than suppressed.

## Decisions

### Decision 1: add producers behind the gate rather than relax the gate

The gate refuses `proposed`/`exhausted` rows for a reason worth preserving: *"receipt, checkpoint, and
manifest bytes are recovery evidence, not an authenticity boundary."* The fix is to make the harness
able to produce the facts, so the same gate admits a genuinely backed row.
*Rejected:* widening the gate to accept authored evidence, which would trade a blocked campaign for a
corrupted one — and would make the 61 blocked rows indistinguishable from fabricated ones.

### Decision 2: the capability stays bound to the current report and runner journal

Admission continues to require a digest-bound capability recomputed by the runner, with receipts whose
digests are checked against the journal for the exact command, target and selector.
*Rejected:* binding to the report only, which would let a capability computed for an earlier payload
admit a later, different one.

### Decision 3: derive the allowance from the row, not from an allowlist

Today the only registry entries are reviewed **data** rows of `emi/battle/battle/15`, and data rows are
admitted only as `exhausted`. That makes unblocking a campaign a matter of hand-maintaining entries.
The allowance is instead derived from the row's kind, locality and closed rungs, with the same
runner-recomputation check.
*Rejected:* extending the allowlist by hand for the nine debt symbols — it would not generalise, and it
would put judgement about which rows may be admitted outside the evidence.

### Decision 4: unclosable rungs must name themselves

If a required rung has no producer in a given revision, the row must say so as a capability gap. The
current failure mode is worse than an error: the rung looks like unfinished work, so missions keep
collecting evidence for it. This change makes the absence explicit so a caller stops rather than
retries.
*Rejected:* leaving the rungs silently open, which is what produced repeated, futile collection runs.

### Decision 5: the change carries the harness ownership review

This is a harness behaviour change, and the repository requires the ownership review
`docs/agents/harness.md` specifies, plus updated harness/user documentation.
*Rejected:* landing it inside the naming change that depends on it, which would hide a capability
change inside an identity change.

## Risks / Trade-offs

- [A new producer emits a fact the gate then trusts] → every fact stays receipt-backed with a
  runner-journal-bound digest, and the negative tests for stale or authored evidence are kept.
- [Admitting `proposed` rows for functions re-opens an unsafe path] → admission still requires the full
  required-rung set for that row's kind and locality; a partially closed row is still refused.
- [Existing tests encode the old refusal] → they are updated deliberately, with the fail-closed cases
  preserved, so the change to expectations is visible in review rather than incidental.
- [The unblocked campaign then produces low-quality names] → out of scope here: naming judgement stays
  with `$bof3-naming`, its corroborator and `name_terms` requirements, and independent review.
