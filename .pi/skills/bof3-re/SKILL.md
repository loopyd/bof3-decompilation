---
name: bof3-re
description: Lift or review a target-qualified BOF3 function, match original bytes, edit target-owned maps/layout, or normalize proven duplicates. Promote only evidence-backed facts.
---

# BOF3 RE

## Authority

[Standing authorization](references/native-execution.md#authority-and-profile) covers
parent index refreshes/external review; no repeated consent. Mission scope and
original budgets still bind. Workers never refresh pinned inputs or self-accept.
[Operator](../bof3-lift-loop/SKILL.md) delegates through active-session tools only;
no harness Codex/model SDK/API/detached fallback. Worker output remains proposal.

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

Explicit cleanup uses structured router and selected skill. Ordinary lift/match/
review/pipeline work must not load naming-evidence or documentation-repair bodies.

Selector: `TARGET@0xADDRESS` or `BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`.
First lifter/reviewer command: `bin/harness agent context <reverse|review> SELECTOR`,
once. Reuse bounded tracked prefill; never reread bundled paths. Generated asm/index
reads need concrete task; `$psx-rizin` needs concrete question. Repository `bin` wins.

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

Before first native build, follow [compiler execution route](references/native-execution.md).
Verified i386/SIGSYS host needs reviewed scoped parent-native gate via escalated tool;
never repeat known sandbox failure, alter policy or retry denial. Live acceptance
never cached.

| Need | Command / limit |
| --- | --- |
| Missing brief | `function-brief.py TARGET@0xADDRESS` once; no repeated constituent queries |
| Existing-C diagnosis | `bin/harness lift asm-diff TARGET@0xADDRESS --detail normal`; no broad status per edit |
| First/ambiguous hunk | `asm-diff --detail full`; no repeated full diffs |
| Accept/review | `bin/harness lift byte-match TARGET@0xADDRESS`, live not cached |
| Relevant declared companion | `bin/harness lift companion-check TARGET@0xADDRESS`; no global catalog scan |

`source splat`/`m2ctx`/`m2c` only for missing evidence. `lift status` is parent-only
disposable audit. Report map/Splat-caused Rizin/index staleness; parent refreshes.

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

Parent `scripts/mission.py diagnose REQUEST ...` captures cold existing-lift evidence.
After scoped edit and confirmed writer termination, `scripts/mission.py audit
DIAGNOSIS PROPOSAL ...` checks retained result. Keep original cutoff/external pins;
unmeasured results `unverified`/null. Neither edits/restores source, bypasses stale
analysis, grants acceptance/retry, launches model or completes operator loop.

## Match loop

Read [clean-C ladder and probe ceiling](references/match-loop.md) before editing.
Existing C requires live pre-edit diff. First-source route, cumulative attempt
ledger, best-candidate retention, independent review and native gates remain binding.

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

Complete lift gate; no mission `just check`/`bin/harness lift status` except explicit
request or parent-owned tooling/config work. `just check` owns scoped gates;
`just check-all` owns full pytest. Report checks/skips/risks/next and reusable
residual/shape/before-after evidence in references, not source annotations.

Before representation/flow diagnosis read [matching knowledge](references/matching-knowledge.md).
Before regional format research read [regional leads](references/regional-leads.md).
Return tool count, wall time, method and measured result with evidence limits.
Parent archives measurements; reusable directives belong in skill references.
