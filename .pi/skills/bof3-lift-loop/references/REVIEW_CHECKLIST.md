# Review checklist

Review one `TARGET@0xADDRESS`, read-only. First run
`python3 .pi/skills/bof3-re/scripts/agent-context.py review`; it supplies the
ordered required context in one call. Use executor brief/diff as context; run one
fresh `bin/byte-match TARGET@0xADDRESS`.
`decomp-status` is audit cache, never acceptance. Do not run brief, asm-diff,
m2c, Rizin, or index rebuild unless a finding needs it. Run companion-check only
for a declared call in this function. Batch grep/map/hygiene reads where useful.

Check:

1. live byte-match exits 0;
2. no banned asm/pins/asm-renamed externs/unauthorized `INCLUDE_ASM`;
3. artificial aids have `MATCHING_AID`; no generic matching-hack macro;
4. SDK calls are official; no lifted SDK body;
5. new declarations/maps/types reuse existing target/include/SDK facts; no aliases
   or parallel structs;
6. raw filename, canonical names, `unk_XX`, and header barrel order remain correct;
7. C is semantically credible: types, signedness, flow, data ownership, no masking;
8. companion claims remain static-only; block foreign ownership/link/ABI claims;
9. changed map/Splat facts pass `bin/symbols check` and relevant `bin/splat`;
10. `git diff --check` clean; no secrets, `inputs/`, or unintended staged files.

Verdict: `pass`, `needs-fix`, or `block` (fundamental behavior/load/banned issue).
Return:

```json
{"function":"TARGET@0xADDRESS","verdict":"pass|needs-fix|block","findings":[{"file":"","line":0,"rule":"","issue":""}]}
```

If an acceptance contract exists, append its exact fenced `acceptance-report`
with copied IDs, actual checks, validation, risks, and fresh staged-index result.
