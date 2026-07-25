# Review checklist — bof3-review reviewer

You review ONE just-matched lift `TARGET@0xADDRESS`, read-only. Load
`/skill:bof3-re` for the rules. Return a verdict with concrete, file:line
findings.

## Checks

1. **Byte-match integrity**: re-run `bin/byte-match TARGET@0xADDRESS`; confirm exit 0.
2. **Banned constructs** (AGENTS.md): no `__asm__` except `barrier()`/`CLOBBER_*`/
   `WEAK_SYMBOL_AT`; no `register X asm("$N")` pins; no `extern X asm("NAME")`
   renames; no `INCLUDE_ASM` without explicit user approval.
3. **Matching aids documented**: every artificial aid has a `MATCHING_AID`
   comment; no generic header macros added for matching hacks.
4. **PsyQ external**: no lifted SDK bodies; SDK calls use official names +
   bindings, not reinvented `func_8017XXXX`.
5. **No duplicate declarations**: every new struct, typedef, extern, `#define`,
   or symbol was checked against the target `internal.h`, `symbols.txt`,
   `include/`, SDK maps, and `bin/rev-query symbols`/`variables` before
   creation. No second name for an already-mapped address; no parallel struct
   duplicating an existing one with the same offsets; no local redefinition of
   a shared `include/bof3/` macro or known PsyQ declaration.
6. **Naming/structure**: raw `func_<ADDR>.c` filename kept; unknown struct fields
   are `unk_XX`; canonical symbol names; `internal.h` barrel order correct.
7. **Semantic correctness**: the C reflects original behavior, not just bytes —
   no unjustified hardcoded magic, proper struct/type recovery, correct
   signedness (`lb` vs `lbu`), no dead code masking a mismatch.
8. **Companion static-call records**: if the caller manifest declares one,
   confirm it only records catalog identity and original static-call evidence.
   Block a lift that treats it as ABI or source/map ownership,
   `WEAK_SYMBOL_AT`, cross-overlay linking, or a reverse-index call edge without
   separately reviewed evidence. Re-run `bin/companion-check TARGET@0xADDRESS`
   only when the caller manifest declares a matching companion record; then a
   nonzero result is a `block` verdict. No companion record means this check is
   not applicable.
9. **Map/Splat consistency**: `symbols.txt` entry present + normalized
   (`bin/symbols check`); reviewed Splat boundary intact.
10. **Hygiene**: no secrets or `inputs/` media staged; `git diff --check` clean.

## Verdict

- `pass` — exact match, guidelines followed, semantically sound.
- `needs-fix` — fixable issue; list findings (the executor retries).
- `block` — fundamental problem (wrong behavior, a banned construct that cannot
  be removed, wrong load address); escalate to the user.

## Return

First return review JSON:
`{"function", "verdict": "pass"|"needs-fix"|"block",
"findings": [{"file", "line", "rule", "issue"}]}`.

When the reviewer prompt also includes an `## Acceptance Contract`, finish with the
required fenced `acceptance-report` JSON. Copy the supplied criterion IDs exactly;
include the checks actually run, validation output, residual risks, and a fresh
staged-index result. This is required for every verdict.
