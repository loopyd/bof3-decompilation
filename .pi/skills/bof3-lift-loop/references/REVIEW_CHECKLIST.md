# Review checklist

Review one `TARGET[#INDEX]@0xADDRESS`, read-only. First run
`python3 .pi/skills/bof3-re/scripts/agent-context.py review TARGET[#INDEX]@0xADDRESS`;
it supplies common/role plus concise target source/config/asm context in one
call. Never reread a bundled file, including checklist Markdown; read only an
unbundled path for a named concrete finding. Use executor brief/diff; run fresh
`bin/byte-match TARGET@0xADDRESS`.
`decomp-status` is audit cache, never acceptance. Do not run `just check`, brief,
asm-diff, m2c, Rizin, or index rebuild unless a finding needs it. Run companion-check only
for a declared call in this function. Batch grep/map/hygiene reads where useful.

Check:

1. live byte-match exits 0;
2. no banned asm/direct pins/asm-renamed externs/unauthorized `INCLUDE_ASM`;
   any mission-added `REGISTER_PIN` has function-specific approval, a local
   `MATCHING_AID` rationale, and the live exact match; a direct numeric `"$N"`
   spelling also proves the macro form changes codegen;
3. artificial aids have `MATCHING_AID`; no generic matching-hack macro;
4. SDK calls are official; no lifted SDK body;
5. new declarations/maps/types reuse existing target/include/SDK facts; no aliases
   or parallel structs. Check the composed maps in Splat order: an existing
   shared-map entry satisfies map ownership; never require an invalid duplicate
   in the target-local map;
6. raw filename, canonical names, `unk_XX`, and header barrel order remain correct;
7. C is semantically credible: types, signedness, flow, data ownership, no masking;
8. companion claims remain static-only. For every mission-added/changed
   game-function extern or `WEAK_SYMBOL_AT`, require local reviewed map+ABI+
   binding or shared SDK-map ownership. Do not block on unchanged pre-existing
   header/public contracts; flag relevant debt separately. Block new foreign
   target definitions/bindings and signature conflicts; report owner path,
   symbol, and both signatures;
9. changed map/Splat facts pass `bin/symbols check TARGET` and `bin/splat TARGET`;
10. `git diff --check` clean; no secrets, `inputs/`, or unintended staged files.
    Read-only `git diff --cached --quiet` is also allowed; no other git command.

Verdict: `pass`, `needs-fix`, or `block` (fundamental behavior/load/banned issue).
Return:

```json
{"function":"TARGET@0xADDRESS","verdict":"pass|needs-fix|block","findings":[{"file":"","line":0,"rule":"","issue":""}]}
```

If an acceptance contract exists, append its exact fenced `acceptance-report`
with copied IDs, actual checks, validation, risks, and fresh staged-index result.
