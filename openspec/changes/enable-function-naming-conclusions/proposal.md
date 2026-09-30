# Proposal

## Why

The naming-debt campaign is blocked at the **harness**, not at the evidence. No audited row can reach
`rung_status: proposed`, and only a `proposed` row can be prepared, so the **61 raw-spelling rows and
14 `binding/map drift`s reported by `bin/harness source symbols check` cannot be resolved by any
authored evidence**, however good. The cause is missing capability rather than a wrong verdict: the
harness never produces positive `selected_call` or `owner_body` facts, and its capability gate admits
no non-data row at all.

## What Changes

- **Produce the two typed facts whose absence makes imported-function rungs unclosable**:
  `selected_call` — the instruction-level callsite including delay slot, argument registers, guards,
  result and transition — and `owner_body` — the owning function's body effects, callees, globals,
  tables and consumers. Both are already declared as *required rungs*; what is missing is a producer.
- **Extend the capability gate so a runner-recomputed capability can exist for the rows under
  campaign, including a documented `proposed` path.** Today `_validated_capability` returns none for
  every non-data row and admits data only as `exhausted` (and only for allowlisted
  `emi/battle/battle/15` rows), so no proposal of either kind can be admitted.
- **Do not weaken the trust boundary.** Only runner-produced typed facts bound to the current native
  runner journal, corroborated one-to-one with canonicalized semantic sources, may establish a
  `proposed` or `exhausted` conclusion. This change adds producers; it relaxes no authenticity check,
  and it must not make a fabricated observation admissible.
- **Keep the existing gates green**, including the naming unit suite and the repository's symbol and
  build gates.
- **Carry the harness ownership review.** This changes harness behaviour, so it requires the review
  `docs/agents/harness.md` specifies.

## Capabilities

### New Capabilities
- `naming-evidence-capability`: the harness's typed naming-evidence facts and the capability gate that
  admits a `proposed` or `exhausted` naming conclusion — which facts exist must cover every required
  rung, and admission must stay bound to runner-produced evidence rather than to author assertion.

### Modified Capabilities
(none — no existing spec covers the naming evidence machinery)

## Impact

- `tools/python/harness/naming/` — the fact producers (`facts.py` declares the rungs; `evidence.py`
  produces facts; `equivalence.py` owns the capability gate; `validation.py` enforces it) plus
  `conclusion.py` and `inventory.py` where rung and required-work descriptions are declared.
- `tools/python/tests/naming/` — the existing tests assert the *current* rejection
  (`test_import_proposed_delegates_existing_proposal_validation`,
  `test_import_exhausted_and_idempotent`); they must be updated to assert the new admissions without
  losing the fail-closed cases.
- `docs/agents/harness.md` and `docs/agents/tool-usage.md` where the harness contract and command
  surface are documented.
- **Unblocks** `apply-evidenced-symbol-renames`, whose task groups 1–3 were withdrawn as impossible
  under the current capability gate.
- No write to build or toolchain sources or configuration, and no hand-edit of anything under
  `build/` or `toolchains/`; derived output produced by running the project's own gates is expected
  and is reported rather than suppressed. No `config/targets/**` write;
  `config/symbol-naming-baseline.json` is untouched and `baseline --write` is never run.
