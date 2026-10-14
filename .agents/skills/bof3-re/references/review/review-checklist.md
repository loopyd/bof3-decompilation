# Review checklist

Review one assigned exact/non-exact selector. `bin/harness agent context review SELECTOR`
preloads this file, `sharing-nonmatches.md` and tracked evidence once. Canonical
lanes skip exact-PASS reviewers; apply this checklist to assigned reviews.

## Scope rules

- Use the executor brief/diff/rung ledger without re-deriving the search.
- Verify recorded measurements. Rerun catalog `bin/harness lift flag-search`, `bin/harness build variants verify` or permuter only for evidence-backed probes of a cited residual class.
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
| Any remaining safe plausible candidate | `needs-fix` + `ladder_exhausted:false` + 1–3 ranked experiments (lever, expected effect, evidence, accept/revert) |
| Unknown type/symbol/layout/ABI/CFG/lifetime | focused target-qualified Rizin first |
| Historical profiles | require supported + installed ones unless proven insensitive |
| Coherent partial, nothing left | `pass` + empty experiments + explicit ladder attestation, only under the exhaustion gate |
| Rejected semantics/types, invalid ownership/boundary, approval/safety, external tool blocker | `block` — never ordinary non-exactness |

No evidence-free repeat or reviewer-required restoration.

## Exhaustion gate

After the ladder and every failed experiment, lifter and reviewer search live
diff/source/compiler/nearby-function evidence for safe plausible candidates outside
the ladder/ledger. Record searches and each candidate's support/rejection. Any
remaining candidate requires `needs-fix`, `ladder_exhausted:false` and 1–3 ranked
experiments predicting size/frame/CFG/branch/offset/instruction/register effects.
Set `ladder_exhausted:true` only when none remain. Skipped rungs and vague allocation guesses prove no exhaustion.

## Item 8 (partial→exact) output

Propose reusable wording/evidence for an authorized reference writer; do not edit
references or accept matching rules. For function-only results, return `lesson: none`
with evidence. Omit selector/address, percentages, transient state and dates.
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
