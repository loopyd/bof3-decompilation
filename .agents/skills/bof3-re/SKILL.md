---
name: bof3-re
description: Lift or review a target-qualified BOF3 function, match original bytes, edit target-owned maps/layout, or normalize proven duplicates. Promote only evidence-backed facts.
---

# BOF3 RE

## Authority

Use [standing authority](references/native-execution.md#authority-and-profile) for
parent index refreshes/external review within original scope/budgets. Workers must
not refresh pinned inputs or self-accept. The [operator](../bof3-lift-loop/SKILL.md)
delegates through session tools only; harness Codex/model SDK/API/detached fallbacks
are forbidden. Return proposals for parent acceptance.

## Route before work

| Branch | Read |
| --- | --- |
| Python harness/CLI/skill-script work | [Tooling contract](references/tooling-contract.md) |
| Any target work | [Target contract](references/target-contract.md), [memory API](references/memory-api.md) |
| Lift | [Reverse protocol](references/reverse/mission-protocol.md) |
| Review | [Checklist](references/review/review-checklist.md), [non-match sharing](references/review/sharing-nonmatches.md) |
| Compiler/assembler/linker/registry/invocation change | [Pipeline validation](references/pipeline-validation.md) |
| Match/repair | [Match loop](references/match-loop.md) |
| Data/fragment/clone/shape eligibility | [Levers](references/match-loop.md#eligible-condition-levers) |
| Existing lift diagnosis/audit | [Parent diagnosis and audit](references/parent-diagnosis.md) |

Route explicit cleanup through its selected skill. Keep ordinary lift/match/
review/pipeline contexts free of naming-evidence/documentation-repair bodies.

Use `TARGET@0xADDRESS` or `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`. First run
`bin/harness agent context <reverse|review> SELECTOR` once; reuse its tracked prefill
without rereading bundled paths. Read generated asm/index for concrete tasks and
invoke `$psx-rizin` for concrete questions. Prefer repository `bin`.

## Invariants

- Original bytes, PS-X headers and `t_addr` outrank tools.
  `runtime address - load address = payload offset`.
- Targets independent: metadata-tagged source, local `internal.h`, map, Splat and
  validation. Parsable `@source`/`@behavior` own identity; filename never substitutes.
  Never copy game extern addresses across targets.
- C89. No register pinning, `REGISTER_PIN`, numeric bindings, `CLOBBER_*`, `barrier()`,
  artificial empty asm or alias/no-op substitutes. Historical matches/reviews grant
  no exception. Remove aids in authorized scope, requeue clean-C match and independent
  review. Keep manifest-owned `symbols.c` `WEAK_SYMBOL_AT`. No handwritten asm,
  asm-renamed externs or unapproved `INCLUDE_ASM`.
- SDK external: official PsyQ names/maps/headers; never lift SDK bodies.
- Unknown fields `unk_XX`; canonical map names. `internal.h` order: guard, includes,
  types, extern data, prototypes, macros/helpers.
- Commit requires explicit user approval. No lift behavior tests; tooling tests only
  when explicitly requested.

## Fast evidence

Before first native build, follow [compiler execution](references/native-execution.md)
for host-profile, parent-native approval and live-gate requirements.

| Need | Command / limit |
| --- | --- |
| Missing brief | `function-brief.py TARGET@0xADDRESS` once; no repeated constituent queries |
| Existing-C diagnosis | `bin/harness lift asm-diff TARGET@0xADDRESS --detail normal`; no broad status per edit |
| First/ambiguous hunk | `asm-diff --detail full`; no repeated full diffs |
| Accept/review | `bin/harness lift byte-match TARGET@0xADDRESS`, live not cached |
| Relevant declared companion | `bin/harness lift companion-check TARGET@0xADDRESS`; no global catalog scan |

Use `source splat`/`m2ctx`/`m2c` only for missing evidence. Reserve `lift status` for
parent disposable audits; report map/Splat-caused Rizin/index staleness to parent.

## Scope + evidence

1. Honor selected function/group. None supplied: rank
   `bin/harness analysis query <quick-wins|leafs|duplicates|hotspots|pareto> --unlifted --detail minimal --limit 5`,
   then wait.
2. Brief once; validate identity/boundary/load/payload. Query `calls`/`duplicates`
   only for missing ABI/duplicate evidence.
3. Before declarations, search target `internal.h`/`symbols.txt`, `include/`, PsyQ
   map/report, then index/siblings. Reuse types/names; extend evidenced structs;
   no parallel declarations.
4. Companion record proves catalog identity and original `jal`, not ABI/ownership/
   residency. Require reviewed callee boundary, ABI, local map ownership, caller
   prototype and passing `companion-check`; otherwise escalate.
5. Signatures come from callers/callees, not m2c stubs. Edit owned C and only
   evidence-required header/map/Splat.

Use parent `scripts/mission.py diagnose REQUEST ...` for cold existing-lift evidence
and `scripts/mission.py audit DIAGNOSIS PROPOSAL ...` after scoped edits and confirmed
writer termination. Read the [parent contract](references/parent-diagnosis.md) for
pins, `unverified`/null results and read-only audit limits.

## Match loop

Before editing, read the [clean-C ladder and probe ceiling](references/match-loop.md)
for existing/first-source gates, cumulative attempts, candidate retention and review.

### Eligible-condition levers

Before first edit and before exhaustion, use [eligibility table](references/match-loop.md#eligible-condition-levers).
Record ineligible/declined reasons; new evidence alone reopens eligibility.

## Duplicates + handoff

Duplicate hash is lead, not shared ownership. Prove boundaries, representative and
second target independently before worthwhile shared `src/shared/<domain>/*.inc`.
Keep address wrappers, local maps and boundaries.

One `bin/harness lift gate --target TARGET SELECTOR [SELECTOR ...]` measures each
edited function's live instruction/byte comparison plus target symbols/Splat. No
repeat on unchanged candidate. Retain companion checks and owned `git diff --check`;
gate's global diff check is informational.

New raw `func_`/`D_` spellings require recorded naming debt through
`bin/harness source symbols baseline --write` where owner authorizes it; otherwise
symbols check reports new debt. In lanes baseline is parent-owned, report need only.
Never rewrite baseline to hide existing debt.

Complete the lift gate. Use mission `just check`/`bin/harness lift status` only for
explicit requests or parent-owned tooling/config work. `just check` owns scoped gates;
`just check-all` owns full pytest. Report checks/skips/risks/next and reusable
residual/shape/before-after evidence in references, not source annotations.

Before representation/flow diagnosis read [matching knowledge](references/matching-knowledge.md).
Before regional format research read [regional leads](references/regional-leads.md).
Return tool count, wall time, method and measured result with evidence limits.
Parent archives measurements; reusable directives belong in skill references.
