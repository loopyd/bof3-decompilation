# Implementation gap closure

## Goal and evidence baseline

Restore the tracked repository's validation and subagent workflows after
`bb7b4524`, then close plans whose only remaining work is independent review.
This plan excludes the expected decompilation/naming campaign and private or
generated inputs.

Live evidence (2026-08-17):

- `pytest`: 414 passed; Ruff and source validation pass.
- `bin/symbols check`: exit 2 with 301 entries newer than
  `config/symbol-naming-baseline.json` (67 raw lift files, 3 invalid semantic
  filenames, 90 raw function-map entries, 141 raw data-map entries).
- `just check`: fails only at that global naming-debt gate.
- Qwen Scout completes after reducing its model `maxTokens`; Reviewer and
  Oracle fail child-tool validation because their strict tool lists name
  unavailable tools (`qmd_search`; Oracle also names `structured_output`).
  QMD is retired; session history now uses `session_search` and `vcc_recall`.
- No production `TODO`, `FIXME`, `NotImplementedError`, stale harness imports,
  or package-refactor test failures were found.

## Phase 1 — restore subagent execution

Affected files: `.pi/agents/reviewer.md`, `.pi/agents/oracle.md`, and the
project's pi-subagent extension configuration only if tool loading requires it.

1. Remove retired `qmd_search` declarations from project agents and use the
   available `session_search` plus `vcc_recall` history surfaces.
2. Resolve Oracle's separate `structured_output` provider failure without CLI
   flags or environment-variable tool routing and without weakening its
   structured handoff contract.
3. Run one bounded Reviewer and Oracle smoke task and verify both finish without
   unavailable-tool diagnostics.
4. After any `.pi` Markdown edit, run `/skill:agent-skill-compaction` and its
   required audits.

Acceptance:

- Reviewer and Oracle each complete a read-only smoke task.
- Qwen Scout completes at `thinking: xhigh` within its configured context.
- No global startup registration, CLI/env tool routing, or weakened role
  contract is introduced.

## Phase 2 — reconcile naming-debt policy with the tracked tree

Affected files: `config/symbol-naming-baseline.json`,
`docs/plans/project-symbol-naming-cleanup.md`, and only evidence-required files
under `config/targets/emi/world00/area016/13/` and `src/bof3/world/`.

1. Recompute the exact live debt inventory using
   `harness.domain.naming_debt.collect_naming_debt`; classify entries by the
   commits that introduced reviewed lifts rather than accepting all 301 rows
   mechanically.
2. Resolve the three invalid semantic filenames:
   `func_801F350C_area01613.c`, `func_801F3B00_area01613.c`, and
   `func_801F40C4_area01613.c`. Prefer address-only filenames with unchanged
   function metadata and target claims unless reviewed evidence justifies a
   deliberate filename exception. Update manifest/Splat references atomically.
3. Refresh the baseline only for reviewed, intentionally accepted residual raw
   names. Record the refreshed counts/date in the naming-cleanup plan without
   claiming semantic resolution.
4. For any filename/reference transaction, verify the owning target with
   `bin/symbols check`, `bin/splat`, `bin/build`, live `bin/asm-diff`, and
   `bin/byte-match` for every touched lift.

Acceptance:

- `bin/symbols check` passes globally.
- No unreviewed partial lift is silently admitted to the baseline.
- Target identity, source metadata, addresses, boundaries, and bytes are
  unchanged.

## Phase 3 — validation and plan closure

Affected files: `docs/plans/agent-skill-compaction.md` and
`docs/plans/documentation-readability-refresh.md` only after evidence passes.

1. Run `just doctor`, `just check`, `git diff --check`, agent/skill context and
   script checks, and the focused subagent smoke tasks.
2. Obtain independent semantic review of `.pi` compaction and an independent
   full-tree documentation acceptance review.
3. Check the two remaining boxes only when their stated reviews pass; update
   stale text that says naming debt is byte-identical to the earlier baseline.

Acceptance:

- `just check` is green.
- Reviewer/Oracle/Scout smoke tasks are green.
- Both plans contain current evidence and no unchecked closure item.
- Worktree and staged scope contain only reviewed closure changes.

## Blockers, ownership, and non-goals

- Unsupported semantic symbol guesses remain blocked by the two-corroborator
  rule; this plan does not execute the broader naming campaign.
- Missing proprietary media, PsyQ objects, and disposable `out/`/`build/` state
  are environmental and must not be committed.
- No lifted behavior, SDK body, compiler selection, ABI, address, or target
  ownership change is authorized.
- If the missing child tools have no supported provider or behavior-preserving
  replacement, stop with the exact provider/configuration decision required.
