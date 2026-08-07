# Review checklist

Review one selector, exact or non-exact. `agent-context.py review SELECTOR`
preloads this file, `SHARING_NONMATCHES.md`, `docs/agents/lessons.md`, and
 target evidence. Do not reread bundled paths; load an unbundled spec only for
 a concrete finding. Use executor brief/diff/rung ledger; run fresh
`bin/byte-match TARGET@0xADDRESS` only for an exact claim. `decomp-status` is cache, never acceptance. No `just check`, brief,
m2c, Rizin, or index rebuild unless a concrete finding needs it.

Check:

1. live byte match exits 0 for an exact claim;
2. no banned asm/direct pins/asm-renamed externs/unauthorized `INCLUDE_ASM`;
   a retained `REGISTER_PIN` has allocator/entry-register evidence, local
   `MATCHING_AID`, live exact match, independent review;
3. types, signedness, flow, data ownership, names, header order are credible;
4. new game function bindings have local reviewed map+ABI+binding or shared
   SDK ownership; do not block unchanged pre-existing debt;
5. changed map/Splat facts pass `bin/symbols check TARGET`, `bin/splat TARGET`;
6. `git diff --check` clean; no secrets, `inputs/`, or unintended staged
   files; `git diff --cached --quiet` allowed;
7. non-exact escalation: inspect the still-present best candidate and live
   diff; verify the first original/current difference, mismatch class,
   rung-specific attempts, and next evidence needed. Before accepting an
   escalation, require results from the supported flag matrix and every
   installed historical compiler (`bin/flag-search`, then `--compiler ID`),
   unless the report proves the mismatch class is profile-insensitive. Return
   `needs-fix` when this terminal rung or another documented/sibling-proven
   lever was skipped or misapplied. Do not require restored state: parent
   restoration happens only after this review;
8. compare the residual and successful/failed lever against preloaded lessons.
   When evidence establishes a reusable cross-function rule, edit the smallest
   applicable `docs/agents/lessons.md` or `docs/specs/**/*.md` statement before
   verdict. Omit selector/address, percentages, transient state, and dates.
   If it is genuinely one-function-only, explicitly return `lesson: none` with
   the reason. Apply `SHARING_NONMATCHES.md` to the parent's sharing decision;

Verdict: `pass`, `needs-fix`, or `block`.

```json
{"function":"TARGET@0xADDRESS","verdict":"pass|needs-fix|block","findings":[{"file":"","line":0,"rule":"","issue":""}],"residual_class":"exact|types|symbols|cfg|frame|allocation|scheduling|compiler|boundary|data","next_lever":"","lesson":"path updated|none: reason","parent_restore_required":false}
```

Append the required fenced `acceptance-report` with copied IDs, actual checks,
validation, risks, fresh staged-index result.
