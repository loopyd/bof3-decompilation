# Reverse mission protocol

Lift one selector: `TARGET@0xADDRESS`, or shipped EMI
`BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`. `agent-context.py reverse SELECTOR`
preloads this file and target evidence. Do not reread bundled paths.

EU knowledgebase: `docs/reference/bof3-eu/` (README maps chapters) is the
baseline for format/table/rule guesses — consult the matching chapter before
deriving a structure blind. Its addresses are EU-only; never copy an address.
A verified US 1.1 (SLUS_004.22) difference updates the chapter
nondestructively: append `> **US 1.1 verified:** <claim> (<selector/commit>)`
after the EU claim; never edit or delete EU text.

1. Reuse supplied brief; otherwise run `function-brief.py` once.
   Verify load per SKILL.md. Honor `data_table_probe.warning`: bytes mostly
   aligned code pointers, no prolog = data table; verify raw, promote Splat
   asm→rodata (`T_<ADDR>`), escalate restored (sce10eff/00@0x801D2708, scena16/00@0x801F8538).
2. Before declarations: search target header/map, `include/`, PsyQ map/report,
   index. Reuse types/symbols; no duplicates. A new target-local fixed address
   requires matching `internal.h` extern, `symbols.c` `WEAK_SYMBOL_AT`, and
   target map entry. Check composed Splat maps first.
3. Relevant declared companion calls: run `companion-check`; it proves only
   static identity/call. Require reviewed boundary, ABI, local map, caller
   declaration. Never create foreign game bindings or source ownership.
4. Before every C edit: live `asm-diff --detail normal`, diagnose `first=`,
   one structural fix, rerun, revert regressions. Ladder and clobber rules
   per SKILL.md; retain a pin only for an asm-diff-proven allocator or
   entry-register residual after live exact match with local `MATCHING_AID`
   and independent review.
5. Evidence insufficient to lift: investigate the concrete gap with the repo
   Rizin workflow — `bin/rz-project` status/open, `bin/rev-query` first;
   focused calls/xrefs, code/data boundaries, jump-table targets, ABI setup.
   No global analysis, analyzer mutation, or invented ownership. Report
   target-qualified findings and next evidence needed.
6. Accept only final live `byte-match` exit 0. If map/Splat changed, also
   `bin/symbols check TARGET` and `bin/splat TARGET`. A new lift needs a `c`
   Splat boundary with `@source`/`@behavior`. No `just check`/`decomp-status`
   in the mission.
7. Non-exact re-lift: restore prior source state, report best residual. Do not
   decide decomp.me eligibility or publish; the parent checks the reviewed
   layout boundary and owns sharing.

Banned: handwritten asm except sanctioned helpers, direct register pins,
asm-renamed externs, `INCLUDE_ASM` without user approval; also git writes,
reset/clean/setup, children. On escalation, remove only a newly created
mission source to restore its prior state.

Return:

```json
{"function":"TARGET@0xADDRESS","status":"exact|partial|escalated","match_percent":0,"files_changed":[],"matching_aids":[],"notes":""}
```

Append the required fenced `acceptance-report`: actual commands/validation,
risks, restoration, fresh staged-index state. Exact requires live byte match
and retained owned facts. Escalated requires empty changed-file lists and no
retained-change summary.
