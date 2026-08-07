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
   one structural fix, rerun, revert regressions. Track a compact rung ledger:
   candidate shape, first mismatch class, result, and whether retained. Before
   repeating a failed lever, search `docs/agents/lessons.md` and the target's
   reviewed exact siblings for the same mismatch family. Ladder and clobber
   rules    per SKILL.md. If clean-C/source-shape levers stall, the terminal ladder is
   mandatory before allocator aids or escalation: run `bin/flag-search
   SELECTOR` for the supported flag matrix, then repeat it with each installed
   historical compiler catalog ID via `--compiler ID`. Record every best score
   and whether it changes the first mismatch; skip only with evidence that the
   mismatch class cannot be compiler/profile-sensitive. Retain a pin only for
   an asm-diff-proven allocator or entry-register residual after live exact
   match with local `MATCHING_AID` and independent review.
5. Evidence insufficient to lift: investigate the concrete gap with the repo
   Rizin workflow — `bin/rz-project` status/open, `bin/rev-query` first;
   focused calls/xrefs, code/data boundaries, jump-table targets, ABI setup.
   No global analysis, analyzer mutation, or invented ownership. Report
   target-qualified findings and next evidence needed.
6. Accept only final live `byte-match` exit 0. If map/Splat changed, also
   `bin/symbols check TARGET` and `bin/splat TARGET`. A new lift needs a `c`
   Splat boundary with `@source`/`@behavior`. No `just check`/`decomp-status`
   in the mission.
7. Non-exact result: restore regressions, but leave the best coherent clean-C
   candidate and its owned declaration/map/Splat edits **review-pending**.
   Report the pre-mission file list, best live diff, first original/current
   mismatch, mismatch class, rung ledger, and next evidence-backed lever. The
   parent dispatches independent review and owns final restoration/sharing.
   Do not decide decomp.me eligibility or publish.

Banned: handwritten asm except sanctioned helpers, direct register pins,
asm-renamed externs, `INCLUDE_ASM` without user approval; also git writes,
reset/clean/setup, children. Do not delete or restore a non-exact best candidate
before independent review; the parent owns post-review restoration.

Return:

```json
{"function":"TARGET@0xADDRESS","status":"exact|partial|escalated","match_percent":0,"files_changed":[],"matching_aids":[],"notes":""}
```

Append the required fenced `acceptance-report`: actual commands/validation,
risks, pre-mission state, rung ledger, and fresh staged-index state. Exact
requires live byte match and retained owned facts. Escalated requires a best
coherent review-pending candidate, truthful changed-file list, live best diff,
and an explicit `parent_restore_required: true`; it must not claim restoration.
