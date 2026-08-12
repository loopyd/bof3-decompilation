# Review checklist

Review one selector, exact or non-exact. `agent-context.py review SELECTOR` preloads this file, `SHARING_NONMATCHES.md`, `docs/agents/lessons.md`, target evidence. Use executor brief/diff/rung ledger; fresh `bin/byte-match TARGET@0xADDRESS` only for an exact claim. `decomp-status` is cache, never acceptance. No `just check`, brief, m2c, Rizin, or index rebuild without a concrete finding.

1. live byte match exits 0 for an exact claim;
2. no banned asm/direct pins/asm-renamed externs/unauthorized `INCLUDE_ASM`; a retained `REGISTER_PIN` has allocator/entry-register evidence, local `MATCHING_AID`, live exact match, independent review;
3. types, signedness, flow, data ownership, names, header order are credible;
4. new game function bindings have local reviewed map+ABI+binding or shared SDK ownership; do not block unchanged pre-existing debt;
5. changed map/Splat facts pass `bin/symbols check TARGET`, `bin/splat TARGET`;
6. `git diff --check` clean; no secrets, `inputs/`, or unintended staged files; `git diff --cached --quiet` allowed;
7. non-exact: inspect best/live first diff, attempts, handoffs. Before 20 attempts, `needs-fix` returns 1–3 ranked safe semantic-preserving variants (lever, expected effect, basis `evidence-backed|speculative`, accept/revert): evidence/Rizin/profiles first, then distinct speculative C shapes; no vague/repeated variant. Safe coherent partial at ceiling → `pass` + empty experiments + attestation. Rejected semantics/types, invalid ownership/boundary, approval/safety, or external-tool failure → `block`, never partial retention. Do not require restoration;
8. partial→exact: identify decisive experiment, compare pre/post diffs. Reviewer records reusable rules only in `docs/agents/lessons.md`/`docs/specs/**/*.md`; matching-playbook-narrower rule → return proposed wording for parent. Function-only → `lesson: none` + evidence. Omit selector/address, percentages, transient state, dates. Apply `SHARING_NONMATCHES.md` to the sharing decision.

Verdict: `pass`, `needs-fix`, or `block`. Every `block` sets `repairable:true` only for concrete executor-fixable source/metadata/binding findings; false for rejected semantics/types, invalid boundary, approval/safety, or external-tool failure. A repeated experiment requires non-empty `new_evidence` explaining its changed expected effect.

```json
{"function":"TARGET@0xADDRESS","verdict":"pass|needs-fix|block","repairable":false,"findings":[],"residual_class":"exact|types|symbols|cfg|frame|allocation|scheduling|compiler|boundary|data","experiments":[{"lever":"","basis":"evidence-backed|speculative","expected_effect":"","accept_if":"","revert_if":"","evidence":"","new_evidence":""}],"ladder_exhausted":false,"lesson":"path updated|parent playbook proposal|none: reason","parent_restore_required":false}
```

Append the required fenced `acceptance-report` with copied IDs, actual checks, validation, risks, fresh staged-index result.
