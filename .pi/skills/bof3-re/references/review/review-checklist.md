# Review checklist

Review one assigned selector, exact or non-exact. `bin/harness agent context review
SELECTOR` preloads this file, `sharing-nonmatches.md` and tracked target evidence
once. Canonical lane routing skips exact-PASS reviewers; this contract governs
reviews actually assigned.

## Scope rules

- Use the executor brief/diff/rung ledger — never re-derive the writer's search.
- Fresh `bin/harness lift byte-match TARGET@0xADDRESS` for an exact claim — never treat the `bin/harness lift status` cache as acceptance.
- Verify the writer's recorded measurements — never re-run catalog `bin/harness lift flag-search`, `bin/harness build variants verify` or permuter runs unless a cited residual class needs ONE bounded probe (cap ~120s).
- Scoped `just check` (ruff, `bin/harness source symbols check`, `validate_sources`; seconds) only if a repo gate is genuinely required — never `just check-all`, or a `brief`/m2c/Rizin/index rebuild without a concrete finding.

Stop at the first answered question.

## Bounded protocol (~25 tool calls)

1. Live `bin/harness lift asm-diff`/`bin/harness lift byte-match` for the selector.
2. Metadata parse.
3. Banned-aid grep.
4. Scope from the writer's reported paths only — no `find -newermt` windows, no repo-wide boundary/hygiene surveys, no cross-transaction attribution.
5. One payload spot-check.

At most one determinism rebuild. Staging/orphan `linker.ld` checks only when writer
claim depends on them. Return item verdicts, overall verdict, at most one finding/
repair. No optional experiments/proposal essays unless verdict requires them.

## Verdict items

| # | Check | Requirement |
| --- | --- | --- |
| 1 | Exact claim | live byte match exits 0 |
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

After the prescribed ladder and each failed experiment, lifter and reviewer ask what safe plausible experiment remains outside the ladder or ledger, searching live diff/source/compiler/nearby-function evidence and recording what was searched and why each candidate is supported or rejected. A remaining candidate means `needs-fix`, `ladder_exhausted:false` and 1–3 ranked experiments with a concrete predicted size/frame/CFG/branch/offset/instruction/register effect; only no remaining safe plausible candidate permits `ladder_exhausted:true`. A budget stop, skipped rung or vague allocation guess never proves exhaustion.

## Item 8 (partial→exact) output

Reviewer returns proposed reusable wording + evidence for an authorized writer in
relevant skill references; matching-specific rules remain proposals. Review never edits these files. Function-only → `lesson: none` + evidence. Omit selector/address, percentages, transient state, dates. Apply `sharing-nonmatches.md` to the sharing decision.

## Workflow translation

| Checklist outcome | Workflow |
| --- | --- |
| `pass` with live exact evidence | `accepted` |
| `needs-fix`, or repairable `block` | `repair` |
| other `block` | `blocked` |
| exhausted partial `pass` | reviewed partial, **not** an accepted exact lift; the exact-lift loop records it as blocked/deferred and does not advance it into exact cleanup |

No-op applies only to a justified unchanged naming/cleanup stage. Preserve the original checklist in the workflow evidence.

Verdict: `pass`, `needs-fix`, or `block`. Every `block` sets `repairable:true` only for concrete executor-fixable source/metadata/binding findings; false for rejected semantics/types, invalid boundary, approval/safety, or external-tool failure. A repeated experiment requires non-empty `new_evidence` explaining its changed expected effect.

```json
{"function":"TARGET@0xADDRESS","verdict":"pass|needs-fix|block","repairable":false,"findings":[],"residual_class":"exact|types|symbols|cfg|frame|allocation|scheduling|compiler|boundary|data","experiments":[{"lever":"","expected_effect":"","accept_if":"","revert_if":"","evidence":"","new_evidence":""}],"ladder_exhausted":false,"lesson":"writer proposal with evidence|parent playbook proposal|none: reason","parent_restore_required":false}
```

Append required fenced `acceptance-report`: copied IDs, actual checks/validation,
risks, fresh staged-index result.
