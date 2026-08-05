# Review checklist

Review one selector. `agent-context.py review SELECTOR` preloads this file,
`SHARING_NONMATCHES.md`, `docs/agents/lessons.md`, and target evidence. Do not
reread bundled paths; load an unbundled spec only for a concrete finding. Use
executor brief/diff; run fresh `bin/byte-match TARGET@0xADDRESS` for an exact
claim. `decomp-status` is cache, never acceptance. No `just check`, brief,
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
7. non-exact escalation: verify first original/current difference,
   rung-specific attempts, restored state, next evidence needed; apply
   `SHARING_NONMATCHES.md` to a parent sharing decision;
8. record a documentation update only for a durable, cross-function project
   fact in the relevant `docs/specs/**/*.md` or `docs/agents/lessons.md`;
   omit selector/address, percentages, transient state, dates. No
   speculation or one-function examples; preloaded text suffices.

Verdict: `pass`, `needs-fix`, or `block`.

```json
{"function":"TARGET@0xADDRESS","verdict":"pass|needs-fix|block","findings":[{"file":"","line":0,"rule":"","issue":""}]}
```

Append the required fenced `acceptance-report` with copied IDs, actual checks,
validation, risks, fresh staged-index result.
