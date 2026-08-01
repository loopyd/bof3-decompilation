# Review checklist

Review one selector. `agent-context.py review SELECTOR` preloads this file,
`SHARING_NONMATCHES.md`, `LESSONS.md`, all `docs/specs/**/*.md`, and target
evidence. Do not reread bundled paths.
Use executor brief/diff; run fresh `bin/byte-match TARGET@0xADDRESS` for an
exact claim. `decomp-status` is cache, never acceptance. Do not run `just
check`, brief, m2c, Rizin, or index rebuild unless a concrete finding needs it.

Check:

1. live byte match exits 0 for an exact claim;
2. no banned asm/direct pins/asm-renamed externs/unauthorized `INCLUDE_ASM`;
   a retained `REGISTER_PIN` has allocator/entry-register evidence, local
   `MATCHING_AID`, live exact match, and independent review;
3. types, signedness, flow, data ownership, names, and header order are credible;
4. new game function bindings have local reviewed map+ABI+binding or shared SDK
   ownership; do not block unchanged pre-existing debt;
5. changed map/Splat facts pass `bin/symbols check TARGET` and `bin/splat TARGET`;
6. `git diff --check` is clean and no secrets, `inputs/`, or unintended staged
   files exist; `git diff --cached --quiet` is allowed;
7. for a non-exact escalation, verify first original/current difference,
   rung-specific attempts, restored state, and the next evidence needed. Apply
   `SHARING_NONMATCHES.md` when reviewing a parent sharing decision.
8. record a documentation update only when the evidence establishes a durable,
   cross-function project fact. Edit only the relevant `docs/specs/**/*.md` or
   `LESSONS.md`; omit selector/address, match percentage, transient residual,
   tool-output details, and date-stamped status. Do not record speculation or
   one-function examples. The preloaded documentation text is sufficient for
   this narrow edit; do not reread it merely to prepare the update.

Verdict: `pass`, `needs-fix`, or `block`.

```json
{"function":"TARGET@0xADDRESS","verdict":"pass|needs-fix|block","findings":[{"file":"","line":0,"rule":"","issue":""}]}
```

Append the required fenced `acceptance-report` with copied IDs, actual checks,
validation, risks, and fresh staged-index result.
