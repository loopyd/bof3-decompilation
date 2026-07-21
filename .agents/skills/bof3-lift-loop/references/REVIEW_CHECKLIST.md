# Review checklist — bof3-review reviewer

You review ONE just-matched lift `TARGET@0xADDRESS`, read-only. Load `$bof3-re`
for the rules. Return a verdict with concrete, file:line findings.

## Checks

1. **Byte-match integrity**: re-run `bin/byte-match TARGET@0xADDRESS`; confirm exit 0.
2. **Banned constructs** (AGENTS.md): no `__asm__` except `barrier()`/`CLOBBER_*`/
   `WEAK_SYMBOL_AT`; no `register X asm("$N")` pins; no `extern X asm("NAME")`
   renames; no `INCLUDE_ASM` without explicit user approval.
3. **Matching aids documented**: every artificial aid has a `MATCHING_AID`
   comment; no generic header macros added for matching hacks.
4. **PsyQ external**: no lifted SDK bodies; SDK calls use official names +
   bindings, not reinvented `func_8017XXXX`.
5. **Naming/structure**: raw `func_<ADDR>.c` filename kept; unknown struct fields
   are `unk_XX`; canonical symbol names; `internal.h` barrel order correct.
6. **Semantic correctness**: the C reflects original behavior, not just bytes —
   no unjustified hardcoded magic, proper struct/type recovery, correct
   signedness (`lb` vs `lbu`), no dead code masking a mismatch.
7. **Map/Splat consistency**: `symbols.txt` entry present + normalized
   (`bin/symbols check`); reviewed Splat boundary intact.
8. **Hygiene**: no secrets or `inputs/` media staged; `git diff --check` clean.

## Verdict

- `pass` — exact match, guidelines followed, semantically sound.
- `needs-fix` — fixable issue; list findings (the executor retries).
- `block` — fundamental problem (wrong behavior, a banned construct that cannot
  be removed, wrong load address); escalate to the user.

## Return

JSON: `{"function", "verdict": "pass"|"needs-fix"|"block",
"findings": [{"file", "line", "rule", "issue"}]}`.
