---
name: bof3-re
description: Lift or review one target-qualified BOF3 function, normalize proven duplicates, and promote only evidence-backed source/map/Splat facts. Use for any BOF3 lift, match, target map/layout edit, or duplicate promotion.
---

# BOF3 RE

Use [standing autonomous authorization](../../../docs/INDEX.md#autonomous-execution)
for parent-owned index refreshes and external review; do not ask the user again.
Explicit missions/budgets bind scope and consumption, not new consent. Workers
still cannot refresh pinned inputs or grant semantic acceptance.

[`bof3-lift-loop`](../bof3-lift-loop/SKILL.md) delegates scoped lift/review missions
through tools available in the active session. Never launch Codex, a model SDK/API
or a detached fallback through the harness. Review actual content with distinct
session provenance; worker output is a proposal, not acceptance.

Parent-only `scripts/mission.py diagnose REQUEST ...` obtains [cold diagnosis](../../../docs/agents/codex.md#parent-lift-diagnosis) for an existing claimed lift. After a scoped edit and confirmed writer termination, use `scripts/mission.py audit DIAGNOSIS PROPOSAL ...` for [retained audit](../../../docs/agents/codex.md#retained-lift-audit). Keep original evidence/cutoff and external pins; never guess compiler results. Unmeasured proposals use `unverified`/null until parent gates measure them. Neither command edits/restores source, bypasses stale analysis, grants acceptance/retry, launches a model or completes the loop.

Compiler, assembler, linker, registry, or invocation-wrapper changes must follow [compiler pipeline validation](references/PIPELINE_VALIDATION.md). Role details remain in [reverse mission protocol](references/REVERSE/MISSION_PROTOCOL.md), [review checklist](references/REVIEW/REVIEW_CHECKLIST.md), and [sharing/non-match review](references/REVIEW/SHARING_NONMATCHES.md). Explicit cleanup routes are owned by the structured cleanup router and its selected skill; ordinary lifting, matching, review, and pipeline work must not load naming-evidence or documentation-repair bodies.

Selector: `TARGET@0xADDRESS` | shipped EMI `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`. First command for `bof3-lifter`/`bof3-reviewer`: `bin/agent-context <reverse|review> SELECTOR`, once. Its bounded tracked prefill includes role rules and target-owned facts; never reread bundled paths. Generated asm/index evidence is task-driven; psx-rizin only for a concrete question. Repo `bin` wins.

## Invariants

- Original bytes, PS-X headers, `t_addr` outrank tools. Verify load: `runtime address - load address = payload offset`.
- Targets independent: one metadata-tagged lift source, local `internal.h`, map, Splat boundary, validation. Parsable `@source` + `@behavior` mandatory and authoritative; filenames never supply identity/address fallback. Never copy game extern addresses across targets.
- C89. Ban register pinning (including `REGISTER_PIN` and numeric bindings), `CLOBBER_*`, `barrier()` and artificial empty asm, including aliases/no-op substitutes. Historical exact matches or reviews grant no exception: remove aids and requeue affected lifts for fresh clean-C byte matching and independent review. Keep manifest-owned `symbols.c` `WEAK_SYMBOL_AT`. Handwritten asm, asm-renamed externs and unapproved `INCLUDE_ASM` remain prohibited.
- SDK external: official PsyQ names/maps/headers; never lift SDK bodies.
- Unknown fields `unk_XX`; canonical map names; `internal.h` order: guard, includes, types, extern data, prototypes, macros/helpers.
- No commit without explicit user approval. No behavior tests for lifts; add tooling tests only when explicitly requested.

## Fast evidence

Narrowest sufficient command; live acceptance never cached.
Before the first native build, use the [compiler execution route](../../../docs/agents/codex.md#native-compiler-execution).
On the verified i386/SIGSYS host, request the scoped parent-native gate through
reviewed escalated tool execution; do not repeat the known failing sandbox baseline.
This does not change sandbox policy or authorize retries after denial.

| Need | Command | Do not |
| --- | --- | --- |
| Context | `function-brief.py TARGET@0xADDRESS` once | repeat its queries |
| Diagnose | `bin/asm-diff TARGET@0xADDRESS --detail normal` | broad status per edit |
| Ambiguous/new hunk | `asm-diff --detail full` | reread full diffs |
| Accept/review | `bin/byte-match TARGET@0xADDRESS` | cached status |
| Companion ABI | `bin/companion-check TARGET@0xADDRESS` for a relevant declared call | global catalog scan |

`splat`/`m2ctx`/`m2c` only when missing/needed (matching.md). `decomp-status` = parent-only disposable audit data; `asm-diff`/`byte-match` live. Report map/Splat-caused Rizin/index staleness for the parent checkpoint; never rebuild global analysis in a mission.
## Scope + evidence

Honor the selected function/group. None: rank via `bin/rev-query <quick-wins|leafs|duplicates|hotspots|pareto> --unlifted --detail minimal --limit 5`, wait.

1. Brief once; validate identity, boundary, load/payload offset.
2. `rev-query calls`/`duplicates` only for missing ABI/duplicate evidence.
3. Before declarations: search target `internal.h`/`symbols.txt`, `include/`, PsyQ map/report, then index/siblings. Reuse types/names; extend evidenced structs; never parallel declarations.
4. Companion record proves catalog identity + original `jal`, not ABI/ownership/residency. Retain only with reviewed callee boundary, ABI, target map ownership, caller prototype, passing `companion-check`; else escalate.
5. Splat/m2c only if required; signatures from callers/callees, not m2c stubs. Edit only owned C + evidence-required header/map/Splat.

## Match loop

**Before changing existing C, obtain a live asm diff.** A genuinely absent first source uses the bounded [first-source route](../../../docs/agents/matching.md#first-source-and-required-loop); no seed is accepted without native gates and independent review. Diagnose first mismatch, classify per [matching playbook](../../../docs/agents/matching-playbook.md#delay-slots-and-entry-copies), one structural fix, rerun normal diff; revert at once if percentage drops. Full diff only for first/ambiguous diagnosis. Partial-lift catalog = parent audit data, not live diagnosis. 3 non-progressing attempts per level:

1. types/declarations: width, signedness, pointers, fields, prototypes;
2. control flow: branch direction, loop/return/switch shape; equal-valued arms use the playbook's bounded branch-shape matrix before escalation;
3. expression/register order: ordinary temporaries, hoists and statement order; no artificial allocator/scheduler controls;
4. compiler profile: `bin/flag-search TARGET@0xADDRESS`; record only clean-C exact profiles;
5. one bounded `bin/permute TARGET@0xADDRESS --time-limit 60 -j N` after shape is right;
6. report unresolved allocation/scheduling with the best coherent clean-C candidate; never force registers or assembly.

Frame/size residuals: start at types/calls, address-taken locals, aggregate copies and control flow. Same-size relocation/load-order: symbol representation and evidenced pointer-cell volatility. Entry copies require lifetime, clean-C ordering, profile and permuter diagnosis. Lone delay-slot residuals require exact branch/jump operands and liveness, never a clobber.

Non-exact review returns 1–3 ranked untried experiments with expected instruction effects; preserve the best coherent candidate. The caller owns retry count and stopping. Retain coherent improvement with atomic `@status partial`/`@match`/`@residual`; revert only no-progress/semantic defects. Partial→exact review identifies the decisive experiment; parent records a generalizable playbook/lesson rule.

Read `first=` first; a percentage is not success. No matching-hack macros or historical aid exemptions. Third non-progressing attempt: restore best clean-C state, advance; on exhaustion report target, first difference, attempts and next untried/blocked evidence. Accept only final live `bin/byte-match` exit 0 plus independent review.

## Duplicates + handoff

A duplicate hash is a candidate, not shared ownership. Confirm boundaries; match one representative + a second target independently before a worthwhile shared `src/shared/<domain>/*.inc` body. Keep address wrappers/local maps/boundaries.

Before handoff: live `byte-match` per edited function, `bin/symbols check TARGET`, `bin/splat TARGET` only if map/Splat changed, relevant companion-check, `git diff --check`. Complete lift gate; never `just check`/`decomp-status` in a mission — reserve for parent-only tooling/config changes or explicit request. Report checks/skips/risks/next tersely.
