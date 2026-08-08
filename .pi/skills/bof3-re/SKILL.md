---
name: bof3-re
description: Lift or review one target-qualified BOF3 function, normalize proven duplicates, and promote only evidence-backed source/map/Splat facts. Use for any BOF3 lift, match, target map/layout edit, or duplicate promotion.
---

# BOF3 RE

One function selector at a time: `TARGET@0xADDRESS`, or shipped EMI
`BIN/FAMILY/ARCHIVE.EMI#INDEX@0xADDRESS`. For `bof3-reverse`/`bof3-review`,
run `python3 .pi/skills/bof3-re/scripts/agent-context.py <agents|reverse|review> [SELECTOR]`
once: it emits common+role context, the role's `references/<ROLE>/` folder,
`docs/agents/lessons.md`, target manifest/map/Splat/header, complete bindings,
and selected source/asm. Never reread a bundled path; read an unbundled path
only for a named gap. Load a spec or psx-rizin reference only for a concrete
question. Repo `bin` wins.

## Invariants

- Original bytes, PS-X headers, `t_addr` outrank tools. Verify load:
  `runtime address - load address = payload offset`.
- Targets stay independent: one metadata-tagged lift source, local `internal.h`, map, Splat boundary, validation. Parsable function-level `@source` and `@behavior` are mandatory and authoritative; filenames never supply identity or an address fallback. Never copy game extern addresses across targets.
- C89 only. Banned: handwritten asm, direct register pins, asm-renamed
  externs, `INCLUDE_ASM`. Sanctioned: `barrier()`/`CLOBBER_CALLER_REG(reg)`
  (or named `CLOBBER_*`), `REGISTER_PIN(type, name, reg)`, `symbols.c`
  `WEAK_SYMBOL_AT`. After the clean-C ladder, a pin is autonomous only for an
  asm-diff-proven allocator or entry-register residual; it needs a local
  `MATCHING_AID`, independent review, and a live exact byte match. Never add a
  generic matching macro. A legacy direct numeric pin stays only with explicit
  user approval and proof the macro form changes codegen. No fallback asm
  without user approval.
- SDK is external: official PsyQ names/maps/headers; never lift SDK bodies.
- Unknown fields: `unk_XX`; map names canonical; `internal.h` order: guard,
  includes, types, extern data, prototypes, macros/helpers.
- No commit without explicit user approval. No behavior tests for lifts; only
  minimal tooling-contract tests when tooling changes.

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

`decomp-status` is parent-only disposable audit data; `asm-diff`/`byte-match`
are live. Report map/Splat-caused Rizin/index staleness for the parent
checkpoint; never rebuild global analysis in a one-function mission.

## Scope and evidence

Honor the selected function/group. With none selected, rank via
`bin/rev-query <quick-wins|leafs|duplicates|hotspots|pareto> --unlifted --detail minimal --limit 5`
and wait for user selection.

1. Brief once; validate identity, boundary, load/payload offset.
2. `rev-query calls`/`duplicates` only for missing ABI/duplicate evidence.
3. Before declarations: search target `internal.h`/`symbols.txt`, `include/`,
   PsyQ map/report, then index/siblings. Reuse types/names; extend evidenced
   structs; never parallel declarations.
4. A companion record proves catalog identity plus original `jal`, not ABI,
   ownership, or residency. Retain such a caller only with reviewed callee
   boundary, ABI, target map ownership, caller prototype, and passing
   `companion-check`; otherwise escalate.
5. Splat/m2c only if required. Signatures from callers/callees, not m2c
   stubs. Edit only owned C and evidence-required header/map/Splat.

## Match loop

**Never edit C before a live asm diff.** Diagnose the first mismatch, classify
with the [matching playbook](../../../docs/agents/matching-playbook.md#delay-slots-and-entry-copies),
make one structural fix, rerun normal diff; revert at once if percentage drops.
Full diff only for first/ambiguous diagnosis. The partial-lift catalog is
parent audit data, not a substitute for live diagnosis. Fix order, three
non-progressing diagnosed attempts per level:

1. types/declarations: width, signedness, pointers, fields, prototypes;
2. control flow: branch direction, loop/return/switch shape;
3. expression/register order: temps, hoists, statement order, then an
   asm-diff-proven caller-register `CLOBBER_CALLER_REG(reg)` for delay-slot or
   fixed-address reload scheduling, with local `MATCHING_AID`; never to encode
   an opcode or clobber `s*`/`gp`/`sp`/`ra`;
4. compiler profile: `bin/flag-search TARGET@0xADDRESS`; record only clean-C exact profiles;
5. one bounded `bin/permute TARGET@0xADDRESS --time-limit 300 -j N` after shape is right;
6. asm-diff-proven allocator or entry-register residual: one bounded local
   `REGISTER_PIN` experiment; retain only if exact and independently reviewed;
7. report residual; never force banned assembly.

Frame/size residuals: start at types/calls, address-taken locals, aggregate
copies, control flow — never a pin. Same-size relocation/load-order: symbol
representation, pointer-cell volatility. A `move tN,aN`/`move vN,aN` entry
copy is an allocator residual only after source lifetime, clean-C ordering,
profile, and permuter variants are exhausted. A lone delay-slot residual needs
exact branch/jump operands and liveness before a caller-register clobber.

After the bounded loop, retain a reviewed coherent net improvement with atomic `@status partial`, `@match NN.NN`, `@residual ...`; revert only no-progress or semantic/type defects.

Read `first=` first; a percentage is not success. A retained `MATCHING_AID`
names the original/current instruction or register placement, exhausted rung,
and the immediately following exact live byte-match; remove it if clean C
later matches. No generic matching-hack macros. Third non-progressing attempt:
restore best clean-C state and advance. On exhaustion report target, first
difference, attempts, next untried/blocked evidence. Accept only final live
`bin/byte-match TARGET@0xADDRESS` exit 0.

## Duplicates and handoff

A duplicate hash is a candidate, not shared ownership. Confirm boundaries;
match one representative and a second target independently before a worthwhile
shared `src/shared/<domain>/*.inc` body. Keep address wrappers/local
maps/boundaries.

Before handoff: live `byte-match` per edited function, `bin/symbols check
TARGET`, `bin/splat TARGET` only if map/Splat changed, relevant
companion-check, `git diff --check`. This is the default complete lift gate;
never run `just check` or `decomp-status` in an agent mission. Reserve
`just check` for parent-only broad tooling/config changes or explicit request.
Report checks, skipped broad checks, risks, next step tersely.

## Order of operations: loop completion → cleanup

After a lift loop completes (all selected exact), cleanup organizes before
the next loop:

1. Symbols: `bin/symbols normalize TARGET --write`, then `bin/symbols check
   TARGET`; proven names enter the target map as sorted `name = 0xADDRESS;`.
2. Naming: `bof3-cleanup` ladder (references/CLEANUP/), one evidence-gated
   rename at a time; `func_XXXXXXXX`/`D_XXXXXXXX`/`unk_XX` stays until the
   two-corroborator gate passes.
3. Organization: source filenames are flexible because the domain registry resolves function-level `@source`/`@behavior`; source filename, compiled symbol, and Splat label remain separate. Plan moves and never move mid-loop.
4. Gates: fresh live `bin/asm-diff TARGET@0xADDRESS --detail normal` (no
   first-difference) and `bin/byte-match TARGET@0xADDRESS` per touched
   selector, `bin/splat TARGET` if map/Splat changed, fresh `bof3-review`.
   Failure → revert, never fix forward.

### Pipeline-test contract

On changes to the compiler catalog (`config/compiler/variants.json`), object
flags (`config/compiler/object-flags.cmake`), compiler selection
(`BOF3_OBJCOMPILER_`), `bin/cc`, maspsx, `bin/as`, or linker toolchain code,
run:

    python -m pytest tools/python/tests/test_bin_cc_pipeline.py -v
    python -m pytest tools/python/tests/test_asm_link.py -v

Then live `bin/asm-diff TARGET@0xADDRESS --detail normal` and
`bin/byte-match TARGET@0xADDRESS` on every affected lift. Source-only lifts
are exempt.
