---
name: bof3-re
description: Lift or review one target-qualified BOF3 function, normalize proven duplicates, and promote only evidence-backed source/map/Splat facts. Use for any BOF3 lift, match, target map/layout edit, or duplicate promotion.
---

# BOF3 RE

Work one `TARGET[#INDEX]@0xADDRESS` at a time; canonical paths and
`TARGET@0xADDRESS` both work. For `bof3-reverse`/`bof3-review`, first run
`python3 .pi/skills/bof3-re/scripts/agent-context.py <reverse|review> TARGET[#INDEX]@0xADDRESS`
once. It emits common+role context plus concise target manifest/map/Splat/header,
complete target bindings, and selected source/asm. Never reread a bundled path, including
skill/protocol Markdown; read only an unbundled path for a named concrete gap.
Load a spec or psx-rizin reference only for a concrete question. Repo `bin` wins.

## Invariants

- Original bytes, PS-X headers, and `t_addr` outrank tools. Verify target load:
  `runtime address - load address = payload offset`.
- Keep each target independent: raw `func_<ADDR>.c`, local `internal.h`, map,
  Splat boundary, and validation. Never copy game extern addresses across targets.
- C89 only. No handwritten asm, direct register pins, asm-renamed externs, or
  `INCLUDE_ASM`; only sanctioned `barrier()`/`CLOBBER_CALLER_REG(reg)`
  (or named `CLOBBER_*` wrappers), approved `REGISTER_PIN(type, name, reg)`,
  and target `symbols.c` `WEAK_SYMBOL_AT`
  apply. A legacy direct numeric pin remains only if the macro form changes
  codegen. An approved pin needs a local `MATCHING_AID` comment and a live exact
  byte match; approval extends to independently exact members of its proven
  local duplicate family. No fallback asm without user approval.
- SDK is external: use official PsyQ names/maps/headers; never lift SDK bodies.
- Unknown fields are `unk_XX`; map names are canonical; keep `internal.h` order:
  guard, includes, types, extern data, prototypes, macros/helpers.
- Do not commit without explicit user approval. Do not add behavior tests for lifts;
  add only minimal tooling-contract tests when tooling changes.

## Fast evidence

Use the narrowest sufficient command. Live acceptance is never cached.

| Need | Command | Do not |
| --- | --- | --- |
| Context | `function-brief.py TARGET@0xADDRESS` once | repeat its mission/Rizin queries |
| Diagnose | `bin/asm-diff TARGET@0xADDRESS --detail normal` | broad status/m2c each edit |
| Ambiguous/new hunk | `asm-diff --detail full` | reread unchanged full diffs |
| Accept/review | `bin/byte-match TARGET@0xADDRESS` | accept cached status |
| Parent progress report | `bin/decomp-status TARGET --detail minimal` on request | agent mission/status sweep |
| Companion ABI | `bin/companion-check TARGET@0xADDRESS` only for a relevant declared call | global catalog scan |
| Seed/boundary | `splat`, `m2ctx`, `m2c` only when missing/needed | regenerate after C-only edits |

`decomp-status` is parent-only disposable audit data. `asm-diff` and `byte-match`
are live. Report map/Splat-caused Rizin/index staleness for the parent checkpoint;
do not rebuild global analysis during a one-function mission.

## Scope and evidence

Honor the selected function/group. If none is selected, rank with
`bin/rev-query <quick-wins|leafs|duplicates|hotspots|pareto> --unlifted --detail minimal --limit 5`
and wait for user selection.

1. Run the brief once; validate identity, boundary, load/payload offset.
2. Query `rev-query calls`/`duplicates` only for missing ABI/duplicate evidence.
3. Before declarations, search target `internal.h` and `symbols.txt`, `include/`,
   PsyQ map/report, then index/siblings. Reuse existing types/names; extend
   evidenced structs, never make parallel declarations.
4. A companion record proves catalog identity plus original `jal`, not ABI,
   ownership, residency, or cross-linking. Before retaining such a caller,
   require reviewed callee boundary, ABI, target map ownership, caller prototype,
   and passing relevant `companion-check`; otherwise escalate.
5. Generate Splat/m2c evidence only if required. Recover signatures from callers/
   callees, not m2c stubs. Edit only owned C and evidence-required header/map/Splat.

## Match loop

**Never edit C before a live asm diff.** Diagnose the first mismatch, make one
structural fix, then rerun normal-detail diff. If percentage drops, revert at once.
Use full diff only for first/ambiguous diagnosis. Fix in order, with three
non-progressing diagnosed attempts per level:

1. types/declarations: width, signedness, pointers, fields, prototypes;
2. control flow: branch direction, loop/return/switch shape;
3. expression/register order: temps, hoists, statement order, then an
   asm-diff-proven caller-register `CLOBBER_CALLER_REG(reg)` for delay-slot or
   fixed-address reload scheduling, with local `MATCHING_AID`; never use it to
   encode an opcode or clobber `s*`/`gp`/`sp`/`ra`;
4. compiler profile: `bin/flag-search`; record only clean-C exact profiles;
5. one bounded `bin/permute TARGET@0xADDRESS --time-limit 300 -j N` after shape is right;
6. report residual; never force banned assembly.

Read `first=` first. A percentage is not success. Document artificial aids with
`MATCHING_AID`; do not add generic matching-hack macros. Accept only final live
`bin/byte-match ...` exit 0.

## Duplicates and handoff

A duplicate hash is a candidate, not shared ownership. Confirm boundaries; match
one representative and a second target independently before a worthwhile shared
`src/shared/<domain>/*.inc` body. Keep address wrappers/local maps/boundaries.

Before handoff run live `byte-match` for each edited function,
`bin/symbols check TARGET`, `bin/splat TARGET` only if its map/Splat changed,
relevant companion-check, and `git diff --check`. This is the default complete
lift gate; do not run `just check` or `decomp-status` in an agent mission.
Reserve `just check` for parent-only broad tooling/config changes or explicit
request. Report checks, skipped broad checks, risks, and next step tersely.
