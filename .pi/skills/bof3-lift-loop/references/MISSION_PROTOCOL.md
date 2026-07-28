# Executor protocol

Lift one `TARGET@0xADDRESS`. Load `/skill:bof3-re` and `AGENTS.md`; stay within
mission-owned source, `internal.h`, target map, and Splat boundary.

1. Reuse supplied brief; otherwise run `function-brief.py` once. Do not repeat
   mission/status/byte-match queries. Verify load: runtime − load = payload offset.
2. Before declarations, search target header/map, `include/`, PsyQ map/report,
   and index. Reuse types/symbols; no duplicates.
3. Generate Splat/m2c evidence only for a missing/uncertain boundary or seed.
4. For relevant declared companion calls, run `companion-check`; it proves static
   identity/call only. Require reviewed boundary, ABI, local map, and caller
   declaration; never create foreign binding/source/link ownership.
5. Use official SDK names; recover real signatures from callers/callees, not m2c.
6. Write readable C89. Infer structs from offsets; unknown fields are `unk_XX`.
7. Before every C edit: live `asm-diff --detail normal`, diagnose `first=`, make
   one structural fix, rerun. Revert regressions. Escalate types → flow → ordering
   → flags → one bounded permuter → documented residual after three stalled tries
   per level. Use full diff only when normal evidence is insufficient.
8. Accept only final live `byte-match` exit 0. Never use cached status as proof.
   Report Rizin/index staleness to parent; do not rebuild global analysis.

Banned: handwritten asm (except sanctioned helpers), register pins, asm-renamed
externs, `INCLUDE_ASM` without approval, git writes, reset/clean/rm/setup, children.

Return:

```json
{"function":"TARGET@0xADDRESS","status":"exact|partial|escalated","match_percent":0,"files_changed":[],"matching_aids":[],"notes":""}
```

If acceptance contract exists, append its fenced report; copy IDs, actual
commands/validation, tests, risks, and fresh staged-index state. Exact requires
live byte match plus retained owned facts. Escalated requires restored edits,
empty changed-file lists, first mismatch, restoration commands, risk, and no
retained-change summary. Missing fence/evidence or false exact is failure.

```acceptance-report
{"criteriaSatisfied":[],"changedFiles":[],"testsAddedOrUpdated":[],"commandsRun":[],"validationOutput":[],"residualRisks":[],"noStagedFiles":true,"diffSummary":"","reviewFindings":[],"manualNotes":""}
```

**Exact:** byte match passed; retained owned facts are listed.
**Escalated:** the mission JSON says `status: "escalated"`; both `files_changed` and `changedFiles` are `[]`; record restoration commands, non-empty risk, and no-retained-changes `diffSummary`.
A missing fence, missing evidence, or an
escalation falsely claimed as exact remains a failed acceptance report.
