---
name: bof3-re
description: Lift or review one target-qualified BOF3 function, normalize proven duplicates, and promote only evidence-backed source/map/Splat facts. Use for any BOF3 lift, match, target map/layout edit, or duplicate promotion.
---

# BOF3 RE

Selector: `TARGET@0xADDRESS` | shipped EMI `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`. For `bof3-reverse`/`bof3-review` run `python3 .pi/skills/bof3-re/scripts/agent-context.py <agents|reverse|review> [SELECTOR]` once (common+role context, `references/<ROLE>/`, `docs/agents/lessons.md`, manifest/map/Splat/header, bindings, source/asm). Never reread bundled paths; unbundled path only for a named gap; spec/psx-rizin only for a concrete question. Repo `bin` wins.

## Invariants

- Original bytes, PS-X headers, `t_addr` outrank tools. Verify load: `runtime address - load address = payload offset`.
- Targets independent: one metadata-tagged lift source, local `internal.h`, map, Splat boundary, validation. Parsable `@source` + `@behavior` mandatory and authoritative; filenames never supply identity/address fallback. Never copy game extern addresses across targets.
- C89. Banned: handwritten asm, direct register pins, asm-renamed externs, `INCLUDE_ASM`. Sanctioned: `barrier()`/`CLOBBER_*`, `REGISTER_PIN(type, name, reg)`, `symbols.c` `WEAK_SYMBOL_AT`. A pin is autonomous only for an asm-diff-proven allocator/entry-register residual after the clean-C ladder; needs local `MATCHING_AID`, independent review, live exact match. No generic matching macro. Legacy direct numeric pin: explicit user approval + proof the macro form changes codegen. No fallback asm without approval.
- SDK external: official PsyQ names/maps/headers; never lift SDK bodies.
- Unknown fields `unk_XX`; canonical map names; `internal.h` order: guard, includes, types, extern data, prototypes, macros/helpers.
- No commit without explicit user approval. No behavior tests for lifts; minimal tooling-contract tests only when tooling changes.

## Fast evidence

Narrowest sufficient command; live acceptance never cached.

| Need | Command | Do not |
| --- | --- | --- |
| Context | `function-brief.py TARGET@0xADDRESS` once | repeat its queries |
| Diagnose | `bin/asm-diff TARGET@0xADDRESS --detail normal` | broad status per edit |
| Ambiguous/new hunk | `asm-diff --detail full` | reread full diffs |
| Accept/review | `bin/byte-match TARGET@0xADDRESS` | cached status |
| Companion ABI | `bin/companion-check TARGET@0xADDRESS` for a relevant declared call | global catalog scan |

`splat`/`m2ctx`/`m2c` only when missing/needed (matching.md). `decomp-status` = parent-only disposable audit data; `asm-diff`/`byte-match` live. Report map/Splat-caused Rizin/index staleness for the parent checkpoint; never rebuild global analysis in a one-function mission.

## Scope + evidence

Honor the selected function/group. None: rank via `bin/rev-query <quick-wins|leafs|duplicates|hotspots|pareto> --unlifted --detail minimal --limit 5`, wait.

1. Brief once; validate identity, boundary, load/payload offset.
2. `rev-query calls`/`duplicates` only for missing ABI/duplicate evidence.
3. Before declarations: search target `internal.h`/`symbols.txt`, `include/`, PsyQ map/report, then index/siblings. Reuse types/names; extend evidenced structs; never parallel declarations.
4. Companion record proves catalog identity + original `jal`, not ABI/ownership/residency. Retain only with reviewed callee boundary, ABI, target map ownership, caller prototype, passing `companion-check`; else escalate.
5. Splat/m2c only if required; signatures from callers/callees, not m2c stubs. Edit only owned C + evidence-required header/map/Splat.

## Match loop

**Never edit C before a live asm diff.** Diagnose first mismatch, classify per [matching playbook](../../../docs/agents/matching-playbook.md#delay-slots-and-entry-copies), one structural fix, rerun normal diff; revert at once if percentage drops. Full diff only for first/ambiguous diagnosis. Partial-lift catalog = parent audit data, not live diagnosis. 3 non-progressing attempts per level:

1. types/declarations: width, signedness, pointers, fields, prototypes;
2. control flow: branch direction, loop/return/switch shape; equal-valued arms use the playbook's bounded branch-shape matrix before escalation;
3. expression/register order: temps, hoists, statement order; then asm-diff-proven caller-register `CLOBBER_CALLER_REG(reg)` for delay-slot/fixed-address reloads, local `MATCHING_AID`; never encode an opcode or clobber `s*`/`gp`/`sp`/`ra`;
4. compiler profile: `bin/flag-search TARGET@0xADDRESS`; record only clean-C exact profiles;
5. one bounded `bin/permute TARGET@0xADDRESS --time-limit 300 -j N` after shape is right;
6. asm-diff-proven allocator/entry-register residual: one bounded local `REGISTER_PIN` experiment; retain only if exact + independently reviewed;
7. report residual; never force banned assembly.

Frame/size residuals: start at types/calls, address-taken locals, aggregate copies, control flow — never a pin. Same-size relocation/load-order: symbol representation, pointer-cell volatility. Entry `move tN,aN`/`move vN,aN` = allocator residual only after lifetime, clean-C ordering, profile, permuter variants. Lone delay-slot residual needs exact branch/jump operands + liveness before a caller-register clobber.

Non-exact review returns 1–3 ranked untried experiments (expected instruction effects); resume the same executor, ≤6 attempts total; preserve best coherent candidate. Stop: exact; rejected semantics/types; approval/safety or external blocker; reviewer `pass` attesting ladder exhaustion. Retain coherent improvement with atomic `@status partial`/`@match`/`@residual`; revert only no-progress/semantic defects. Partial→exact review identifies the decisive experiment; parent records a generalizable playbook/lesson rule.

Read `first=` first; a percentage is not success. Retained `MATCHING_AID` names original/current instruction or register placement, exhausted rung, and the following exact live byte-match; remove if clean C later matches. No generic matching-hack macros. Third non-progressing attempt: restore best clean-C state, advance; on exhaustion report target, first difference, attempts, next untried/blocked evidence. Accept only final live `bin/byte-match` exit 0.

## Duplicates + handoff

A duplicate hash is a candidate, not shared ownership. Confirm boundaries; match one representative + a second target independently before a worthwhile shared `src/shared/<domain>/*.inc` body. Keep address wrappers/local maps/boundaries.

Before handoff: live `byte-match` per edited function, `bin/symbols check TARGET`, `bin/splat TARGET` only if map/Splat changed, relevant companion-check, `git diff --check`. Complete lift gate; never `just check`/`decomp-status` in a mission — reserve for parent-only tooling/config changes or explicit request. Report checks/skips/risks/next tersely.
