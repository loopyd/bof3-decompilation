# Clean-C decompilation continuation

**Status:** active

> **Baseline refreshed 2026-08-01:** Phase 0 is complete. The live audit is
> `exact=454`, `partial=223`, and `invalid=0` across 677 lifts. Partial lifts
> remain non-acceptance backlog.

## Goal

Increase reviewed exact lifts one independently loaded `TARGET@0xADDRESS` at a
time. Retain only readable C89 that byte-matches; keep target-local ownership,
ABI, and Splat evidence with the owning image.

## Current evidence

- Live acceptance evidence reports `exact=454`, `partial=223`, and `invalid=0`
  across 677 indexed lifts; final `just check` is required before handoff.
- Phase 0 repaired the 26 retained invalid records; partial lifts remain
  non-acceptance backlog and are not candidates for duplicate reuse or
  promotion.
- `bin/symbols check` passes.
- `emi/world00/area016/13@0x801F3460` is an exact 64-byte clean-C lift.
  `SPAD_PTR_TABLE(u8)[0x11]` provides the reviewed scratchpad pointer-cell
  representation; its use is documented in `docs/memory-api.md`.
- `emi/etc/commu00/00@0x801F1294` and `@0x801F14C4` are exact 52-byte
  clean-C lifts. They select the active UI pointer, request modes 14 and 23,
  respectively, then call the reviewed no-argument frontend helper
  `func_8015C088`. `@0x801F1684` is an exact 56-byte lift that calls the
  reviewed frontend reset helper `func_8015C058` and clears UI state bytes
  `D_801448EB..D_801448ED`. `@0x801F18BC` is an exact 60-byte clean-C
  dispatcher through the signed-indexed local jump table `D_801F25EC`.
  `@0x801F1B8C` is an exact 60-byte clean-C dispatcher through the adjacent
  signed-indexed local jump table `D_801F2610`.
- `emi/battle/battle/03@0x801E47A4` is an exact 60-byte clean-C lift. It uses
  `SPAD_PTR_TABLE(Battle03LocalWork)[0x11]` to initialize the scratch work
  record while preserving the original pointer-cell reloads and return delay
  slot.
- `emi/etc/shop/00@0x801E1B80` is an exact 56-byte clean-C indirect dispatcher.
  It preserves the byte mask, scaled callback-table index, call frame, and
  callback argument register placement through a target-local table binding.
- The closed `exe/slus_004_22@0x80162B08` compiler residual remains documented
  in `docs/specs/runtime/compiler-provenance.md`; it is not active work.

## Phase 0 — restore a valid retained-source baseline (complete)

- Repaired all 26 invalid retained records through target-qualified review.
- Validated with `bin/decomp-status --no-cache --detail minimal` and
  `just check`: `invalid=0`.

## Phase 1 — select one evidenced candidate

1. Rank unlifted candidates with `bin/rev-query quick-wins`, `leafs`, and
   `duplicates` using `--unlifted --detail minimal --limit 5`.
2. Select one target-qualified function based on instruction count, confidence,
   callers/callees, duplicate leverage, and target-local evidence. Do not switch
   targets or propagate a duplicate without explicit review.
3. Before editing, run `function-brief.py`, inspect calls and duplicates, check
   existing declarations/maps, and confirm the reviewed Splat boundary and
   binary load address.

## Phase 2 — exact lift only

1. Follow the `/skill:bof3-re` one-change `asm-diff` loop for the selected
   function.
2. Promote source, target-local map, and Splat boundary only after
   `bin/byte-match TARGET@0xADDRESS` is exact.
3. For any out-of-target static call, require `bin/companion-check` plus
   reviewed ABI, residency, boundary, and target-local ownership evidence.
   A catalog record or analyzer edge alone is insufficient.
4. For a duplicate group, byte-match one member first; reuse a body only after a
   second independently loaded target proves the same C shape.

## Deferred until new evidence

- `emi/etc/game/00@0x801993F0`, `emi/etc/game/00@0x80199418`, and
  `emi/world00/area016/13@0x801F4740` need the ABI/ownership evidence required
  for their external calls.
- `emi/world00/area008/13@0x801F3D88` is a partial direct-register-pin lift;
  restore/remove it while partial. Retain it only after explicit approval,
  demonstrated macro-form codegen difference, adjacent rationale, and an exact
  live byte match. Its duplicate group needs two independent exact members
  before any shared implementation is considered.
- Do not reopen the SLUS compiler residual without new compiler provenance.

## Validation and acceptance

For every accepted lift:

```sh
bin/asm-diff TARGET@0xADDRESS --detail full
bin/byte-match TARGET@0xADDRESS
bin/companion-check TARGET@0xADDRESS
bin/splat TARGET
bin/symbols check TARGET
bin/decomp-status TARGET --detail normal
git diff --check
```

Run `just check` before handoff when practical. An accepted change has an exact
byte match and clean target-local Splat, map, and decomp-status evidence.

## Boundaries

- Never hand-edit `build/` or `toolchains/`; never commit `inputs/` or `out/`.
- No inline assembly, direct numeric register pins, `INCLUDE_ASM`, speculative
  object flags, or Splat-assembly fallback. A local `REGISTER_PIN` experiment is
  allowed only for an asm-diff-proven allocator or entry-register residual after
  the clean-C ladder; retain it only with `MATCHING_AID`, independent review,
  and a live exact byte match.
- Do not retain partial lifts, analyzer hypotheses, or cross-target game-address
  bindings.
- Do not stage, commit, push, or mutate external systems without approval.
