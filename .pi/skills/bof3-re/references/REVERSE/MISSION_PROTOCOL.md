# Reverse mission protocol

Lift one selector: `TARGET@0xADDRESS`, or shipped EMI
`BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`. `agent-context.py reverse SELECTOR`
preloads this file and the target evidence. Do not reread bundled paths.

1. Reuse supplied brief; otherwise run `function-brief.py` once. Verify load:
   runtime − load = payload offset.
2. Before declarations, search target header/map, `include/`, PsyQ map/report,
   and index. Reuse types/symbols; no duplicates. A new target-local fixed
   address requires matching `internal.h` extern, `symbols.c`
   `WEAK_SYMBOL_AT`, and target map entry. Check composed Splat maps first.
3. For relevant declared companion calls, run `companion-check`; it proves only
   static identity/call. Require reviewed boundary, ABI, local map, and caller
   declaration. Never create foreign game bindings or source ownership.
4. Before every C edit: live `asm-diff --detail normal`, diagnose `first=`, make
   one structural fix, rerun, and revert regressions. Escalate types → flow →
   ordering → flags → one bounded permuter → one local `REGISTER_PIN` only for
   an asm-diff-proven allocator or entry-register residual. Retain a pin only
   after live exact match with local `MATCHING_AID` and independent review.
5. When the existing evidence is insufficient to lift, investigate the concrete
   gap with the repository Rizin workflow: use `bin/rz-project` status/open and
   `bin/rev-query` first; inspect focused calls/xrefs, code/data boundaries,
   jump-table targets, and ABI setup. Do not widen into global analysis, mutate
   analyzer state, or invent ownership. Report target-qualified findings and
   the next evidence needed.
6. Accept only final live `byte-match` exit 0. If map/Splat changed, also run
   `bin/symbols check TARGET` and `bin/splat TARGET`. A new lift needs a `c`
   Splat boundary with `@source`/`@behavior`. Do not run `just check` or
   `decomp-status` in the mission.
7. On a non-exact re-lift, restore the prior source state and report the best
   residual. Do not decide decomp.me eligibility or publish; the parent checks
   the reviewed layout boundary and owns sharing.

Banned: handwritten asm except sanctioned helpers, direct register pins,
asm-renamed externs, and `INCLUDE_ASM` without user approval; also git writes,
reset/clean/setup, and children. On escalation, only remove a newly-created mission source if that
restores its prior state.

Return:

```json
{"function":"TARGET@0xADDRESS","status":"exact|partial|escalated","match_percent":0,"files_changed":[],"matching_aids":[],"notes":""}
```

Append the required fenced `acceptance-report`: actual commands/validation,
risks, restoration, and fresh staged-index state. Exact requires live byte
match and retained owned facts. Escalated requires empty changed-file lists and
no retained-change summary.
