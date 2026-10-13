# Reverse mission protocol

Selector: `TARGET@0xADDRESS` or shipped EMI `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`.
`bin/harness agent context reverse SELECTOR` preloads this file + tracked target evidence once; never reread bundled paths.

Before regional research read [regional leads](../regional-leads.md). Supplied EU
facts are hypotheses, not US address/behavior proof; verify against original bytes.
Return annotation evidence to parent, no external-document browsing or edits.

## Steps

1. Reuse the brief; else run `function-brief.py` once. Before the first edit, check the [eligible-condition levers](../../SKILL.md#eligible-condition-levers): data/fragment evidence routes to classification or a parent-owned layout probe; a byte-identical exact sibling routes to clone dry-run. Near-identical siblings supply shape evidence, not clone proof. Run only applicable levers, record declines, and use `m2c`/`m2ctx` only for a named remaining gap. Search target-owned declarations and `include/` before a bounded sibling search; never recurse from the repository root into generated/session trees. Honor `data_table_probe.warning` as a data lead, not classification proof: verify original bytes and request the parent-owned reviewed layout transaction; never move `.text` data into `rodata` in a lift lane. `bin/harness analysis data-candidates TARGET` ranks that same suspicion across a whole target before any lift is spent on it.
2. Before declarations: search per SKILL.md §Scope; no duplicates. A new target-local fixed address needs an `internal.h` extern + `symbols.c` `WEAK_SYMBOL_AT` + a target map entry; check composed Splat maps first.
3. For relevant declared companion calls run `companion-check` (static identity/call only, per SKILL.md §Scope). Never create foreign game bindings or source ownership.
4. Carry the ordered attempt ledger and consumed sweep/variant budget through every repair; a new child never resets them. Missing consumption evidence blocks new experiments. A genuinely absent source follows the [bounded first-source route](../match-loop.md#first-source). Before changing existing C: live normal diff, diagnose `first=`, one fix, rerun, revert regressions; ledger shape/class/result/retained. Reviewer retry: consume ranked experiments serially, record expected/actual effect + accept/revert, preserve the best; no evidence-free repeat; search [matching knowledge](../matching-knowledge.md) + exact siblings before retrying a lever. Clean C only — the [clean-C terminal ladder](../../SKILL.md#match-loop) is the single home of its rungs; no register pins, clobbers or artificial barriers. Missing type/symbol role/field layout/caller contract/branch target/lifetime → focused Rizin rung first (`bin/harness analysis rz-project status TARGET`, `bin/harness analysis query` calls/xrefs/symbols, then target-isolated `bin/harness analysis rz-project open TARGET`); record what each supports/rejects; never use Rizin to force allocation or replace byte evidence. Clean-C stall → report best scores, first-mismatch class and untried/blocked rungs. Compiler-profile/permuter work requires selector-specific authorization per the ladder; lack of authorization is not exhaustion.
5. Evidence insufficient to lift: do not escalate before the focused Rizin rung. No global analysis, analyzer mutation or invented ownership. Snapshot stale → report parent-owned analysis required; else report target-qualified findings + next evidence.
6. Require the final live instruction/byte comparison and target symbol/Splat checks through the gate batch below; an exact workflow selector needs no redundant byte-verification reviewer. A new lift needs a `c` Splat boundary with `@source`/`@behavior`. No `just check`/`bin/harness lift status` in the mission. Retained map, reviewed Splat/annotation, or manifest source/support/header fact → `snapshot_index_refresh_required: true`; global analysis is prohibited here; the parent serially refreshes the snapshot, rebuilds the global index once, and checks both statuses.
7. Before declaring exhaustion apply the [exhaustion gate](../review/review-checklist.md): after the ladder and every failed experiment, search for safe untried candidates and record the searched evidence and rejection reasons. The [eligible-condition levers](../../SKILL.md#eligible-condition-levers) are that search surface: `analysis data-candidates` for a data-shaped range, `analysis carve` for a fragment boundary whose head lives in another segment or a bare `bin` blob, and `lift clone` for a range that is byte-identical to an already-exact function. A budget stop is not exhaustion. Non-exact: restore regressions; leave the best coherent candidate + owned facts review-pending. Report baseline, best live diff / first mismatch / class, ledger, experiment effects, remaining candidates. The parent reviews, then continues from the complete ordered attempt ledger; restoration/sharing is parent-owned.
8. Retry reaches exact: report decisive experiment and prior partial diff for
   reusable-rule review. Never generalize register coincidence.

## Banned

Register pins (direct or `REGISTER_PIN`), clobbers, artificial barriers/empty asm, handwritten asm (except manifest-owned `WEAK_SYMBOL_AT`), asm-renamed externs, `INCLUDE_ASM` without approval, git writes, reset/clean/setup, children. Never delete/restore a non-exact best candidate before independent review; the parent owns post-review restoration.

## Return

```json
{"function":"TARGET@0xADDRESS","status":"exact|partial|escalated","match_percent":0,"files_changed":[],"matching_aids":[],"notes":""}
```

Append the required fenced `acceptance-report`: actual commands/validation, risks, pre-mission state, rung ledger, fresh staged-index state. Exact requires a live byte match + retained owned facts. Escalated requires a best coherent review-pending candidate, a truthful changed-file list, the live best diff, and `parent_restore_required: true`; never claim restoration.

## Lift setup order (once, before the first `asm-diff`)

1. Splat boundary `asm` → `c` with `@source`/`@behavior`.
2. Target map row `sym = 0xADDR;` (without it `asm-diff` cannot resolve the symbol).
3. Manifest `sources` claim.
4. Per referenced fixed address: target-local `extern` (`@source`/`@kind`) + `WEAK_SYMBOL_AT` + a map row; per referenced function: a target-local prototype.

Before first gate: evidence-backed name/module, complete ownership and canonical
metadata. Unknown roles stay address-anchored; `@status`/`@match`/`@residual` reflect
measurements. After PASS, confirm naming without rewriting correct source.
Later source/binding edit requires renewed gate.

## Gate batch (one pass, after all edits)

`bin/harness lift gate --target TARGET SELECTOR [SELECTOR ...]` performs one native measurement per selector (instruction and byte comparison) plus target symbol/Splat checks once. Do not repeat those checks for an unchanged candidate. Keep the pre-edit normal diff for existing C and applicable companion checks. Check the owned diff for whitespace: the gate's global diff check is informational, not a waiver. Gate retries handle recognized build races; no outer retry loop. Keep `symbols.txt` address-sorted.

## Ranking fallback

`bin/harness analysis query --json quick-wins --target TARGET --unlifted --limit 8`.
Stale snapshot/index: rank tracked Splat `asm` boundaries minus every claimed
`@source` address by resolved calls, decisions, instructions; screen original
`div`/`break` traps. Parent owns refresh; never rebuild analysis in lane. Native
and target symbol/Splat checks remain available despite stale index.

## Fan-out turn-cost rules

Return tool counts, wall time, method and measured outcomes in mission result.

| Rule | Detail |
| --- | --- |
| Gate once | Use the gate batch above after all owned edits; never duplicate native checks for the same candidate. |
| Iterate with the sweeper | One `bin/harness lift sweep SELECTOR SOURCE [--keep-best] VARIANTS...`, at most four variants across the lane and all repairs; record consumption and residual class. |
| Prefill once | ONE `bin/harness agent context reverse SELECTOR`, without head/tail pipes. If clipped, read the saved output remainder rather than rerunning the command. |
| Pick once | ONE `bin/harness lift next TARGET` (ranking + `div`/`break` screen in one call). |
