# Review checklist

Review one assigned exact/non-exact selector. `bin/harness agent context review SELECTOR`
preloads this file, `sharing-nonmatches.md` and tracked evidence once. Canonical
lanes skip exact-PASS reviewers; apply this checklist to assigned reviews.

## Scope rules

- Use the executor brief/diff/rung ledger without re-deriving the search.
- Verify measurements under the owning effective compiler and unchanged flags. Flag catalogs and compiler-variant batches are not review or exhaustion gates. An opt-in permuter requires a cited residual and parent authorization.
- Run scoped `just check` (ruff, `bin/harness source symbols check`, `validate_sources`) only for a required repo gate. Do not run `just check-all` or rebuild `brief`/m2c/Rizin/index without a concrete finding.

Stop at the first answered question.

## Review protocol

1. Run live `bin/harness lift asm-diff`/`bin/harness lift byte-match` for the selector.
2. Parse metadata.
3. Search for banned aids.
4. Inspect writer-reported paths only; do not use `find -newermt`, repo-wide boundary/hygiene surveys or cross-transaction attribution.
5. Spot-check one payload.

Rebuild for determinism only when evidence requires it. Check staging/orphan
`linker.ld` only when the claim depends on it. Return item/overall verdicts and
all consequential findings/repairs.
Include optional experiments/proposals only when the verdict requires them.

## Verdict items

| # | Check | Requirement |
| --- | --- | --- |
| 1 | Exact claim | fresh `bin/harness lift byte-match TARGET@0xADDRESS` exits 0; `bin/harness lift status` cache supplies no acceptance |
| 2 | Aids | no register bindings, `REGISTER_PIN`, `CLOBBER_*`, `barrier()`, artificial empty asm or alias/no-op substitutes; historical matches/reviews are **not** exemptions. Keep manifest-owned `WEAK_SYMBOL_AT`; reject handwritten asm, asm-renamed externs and unauthorized `INCLUDE_ASM`. Removed-aid lifts require fresh clean-C matching and independent review |
| 3 | Semantics | types, signedness, flow, data ownership, names, header order are credible |
| 4 | Bindings | new game function bindings have local reviewed map+ABI+binding or shared SDK ownership; do not block unchanged pre-existing debt |
| 5 | Map/Splat | changed facts pass `bin/harness source symbols check TARGET`, `bin/harness source splat TARGET` |
| 6 | Hygiene | `git diff --check` clean; no secrets, `inputs/`, or unintended staged files; `git diff --cached --quiet` allowed |
| 7 | Non-exact | inspect best candidate / live first diff, mismatch, attempts, prior handoffs |
| 8 | Partial→exact | identify the decisive experiment, compare pre/post diffs |

## Non-exact handling (item 7)

| Situation | Action |
| --- | --- |
| Supported untried candidate | `needs-fix` + `ladder_exhausted:false` + up to three ranked experiments with expected effects and accept/revert criteria |
| No justified probe, evidence incomplete | `needs-fix` + empty experiments + named evidence gap; not exhaustion |
| Unknown type/symbol/layout/ABI/CFG/lifetime | focused target-qualified Rizin first |
| Historical profile searches | Carry as history only; require no rerun or broader catalog |
| Coherent partial, nothing left | `pass` + empty experiments + explicit ladder attestation, only under the exhaustion gate |
| Rejected semantics/types, invalid ownership/boundary, approval/safety, external tool blocker | `block` — never ordinary non-exactness |

No evidence-free repeat or reviewer-required restoration. Separate coherent partial
retention from exact acceptance. Review range proofs and new conversion/flow effects
before retaining a higher score. Retention keeps `needs-fix` / `repair`,
`ladder_exhausted:false` and truthful byte/gate failures; it never advances exact
cleanup. A missing original instruction at a smaller size is not an exact match.

## Exhaustion gate

After the ladder and every failed experiment, lifter and reviewer search live
diff/source/compiler/nearby-function evidence for safe plausible candidates outside
the ladder/ledger. Record searches and each candidate's support/rejection. Any
supported candidate requires `needs-fix`, `ladder_exhausted:false` and a ranked
experiment predicting size/frame/CFG/branch/offset/instruction/register effects.
Set `ladder_exhausted:true` only when none remain. Skipped rungs and vague allocation guesses prove no exhaustion.

## Item 8 (partial→exact) output

Propose reusable wording/evidence for an authorized reference writer; do not edit
references or accept matching rules. Separate reusable investigation methods from
function-specific winning spellings. Preserve preconditions, rejected interactions
and evidence limits; one exact result does not prove universal compiler causality.
For function-only results, return `lesson: none` with evidence. Omit selector/address, percentages, transient state and dates.
Apply `sharing-nonmatches.md` to sharing decisions.

## Workflow translation

| Checklist outcome | Workflow |
| --- | --- |
| `pass` with live exact evidence | `accepted` |
| `needs-fix`, or repairable `block` | `repair` |
| other `block` | `blocked` |
| exhausted partial `pass` | reviewed partial, **not** an accepted exact lift; the exact-lift loop records it as blocked/deferred and does not advance it into exact cleanup |

Use no-op only for justified unchanged naming/cleanup. Preserve this checklist in
workflow evidence.

Verdict: `pass`, `needs-fix`, or `block`. Every `block` sets `repairable:true` only for concrete executor-fixable source/metadata/binding findings; false for rejected semantics/types, invalid boundary, approval/safety, or external-tool failure. A repeated experiment requires non-empty `new_evidence` explaining its changed expected effect.

```json
{"function":"TARGET@0xADDRESS","verdict":"pass|needs-fix|block","repairable":false,"findings":[],"residual_class":"exact|types|symbols|cfg|frame|allocation|scheduling|compiler|boundary|data","experiments":[{"lever":"","expected_effect":"","accept_if":"","revert_if":"","evidence":"","new_evidence":""}],"ladder_exhausted":false,"lesson":"writer proposal with evidence|parent playbook proposal|none: reason","parent_restore_required":false}
```

Append required fenced `acceptance-report`: copied IDs, actual checks/validation,
risks, fresh staged-index result.
