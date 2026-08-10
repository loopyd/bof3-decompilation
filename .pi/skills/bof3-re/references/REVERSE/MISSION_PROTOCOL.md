# Reverse mission protocol

Lift one selector: `TARGET@0xADDRESS` or shipped EMI `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`. `agent-context.py reverse SELECTOR` preloads this file + target evidence. Do not reread bundled paths.

EU knowledgebase: `docs/reference/bof3-eu/` (README maps chapters) = baseline for format/table/rule guesses; consult the matching chapter first. Addresses EU-only; never copy. Verified US 1.1 difference: append `> **US 1.1 verified:** <claim> (<selector/commit>)` after the EU claim; never edit/delete EU text.

1. Reuse supplied brief; else run `function-brief.py` once. Honor `data_table_probe.warning`: aligned code pointers, no prolog = data table; verify raw, promote Splat asm→rodata (`T_<ADDR>`), escalate restored (sce10eff/00@0x801D2708, scena16/00@0x801F8538).
2. Before declarations: search per SKILL.md §Scope; no duplicates. New target-local fixed address needs `internal.h` extern + `symbols.c` `WEAK_SYMBOL_AT` + target map entry; check composed Splat maps first.
3. Relevant declared companion calls: run `companion-check`; static identity/call only, per SKILL.md §Scope. Never create foreign game bindings or source ownership.
4. Before each C edit: live normal diff, diagnose `first=`, one fix, rerun, revert regression; ledger shape/class/result/retained. Reviewer retry: consume ranked experiments one at a time, record expected/actual instruction effect + accept/revert signal, preserve best; no evidence-free repeat; before retrying a lever search lessons and exact siblings. Ladder/clobber/pin rules per SKILL.md. Missing/guessed type, symbol role, field layout, caller contract, branch target, or value lifetime → focused Rizin context rung first: `bin/rz-project status TARGET`, `bin/rev-query` calls/xrefs/symbols, then target-isolated `bin/rz-project open TARGET` (function/callers/data only); record what each finding supports/rejects; never use Rizin to force allocation or replace byte evidence. Clean-C stall → terminal ladder per SKILL.md (flag-search + each installed compiler via `--compiler ID`); record best scores + first-mismatch changes; skip only with profile-insensitivity evidence.
5. Evidence insufficient to lift: do not escalate before the focused Rizin context rung above. No global analysis, analyzer mutation, invented ownership. Snapshot stale → report parent-owned analysis required; else report target-qualified findings + next evidence.
6. Accept only final live `byte-match` exit 0; map/Splat changed → also `bin/symbols check TARGET` + `bin/splat TARGET`. New lift needs a `c` Splat boundary with `@source`/`@behavior`. No `just check`/`decomp-status` in the mission. Retained map, reviewed Splat/annotation, or manifest source/support/header fact → `snapshot_index_refresh_required: true`; global analysis prohibited here; parent serially refreshes the affected snapshot, rebuilds the global index once, checks both statuses.
7. Non-exact: restore regressions; leave best coherent candidate + owned facts review-pending. Report baseline, best live diff/first mismatch/class, ledger, experiment effects, remaining candidates. Parent reviews, normally resumes; restoration/sharing parent-owned.
8. Exact after retry: report decisive experiment + preceding partial diff for reusable-rule review; never generalize register coincidence.

Banned: handwritten asm (except sanctioned helpers), direct register pins, asm-renamed externs, `INCLUDE_ASM` without approval, git writes, reset/clean/setup, children. Do not delete/restore a non-exact best candidate before independent review; parent owns post-review restoration.

Return:

```json
{"function":"TARGET@0xADDRESS","status":"exact|partial|escalated","match_percent":0,"files_changed":[],"matching_aids":[],"notes":""}
```

Append the required fenced `acceptance-report`: actual commands/validation, risks, pre-mission state, rung ledger, fresh staged-index state. Exact requires live byte match + retained owned facts. Escalated requires a best coherent review-pending candidate, truthful changed-file list, live best diff, `parent_restore_required: true`; must not claim restoration.
