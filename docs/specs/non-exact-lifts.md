# Non-exact lift catalog

**Audit:** `bin/decomp-status --json --no-cache`, 2026-07-26
**Scope:** all tracked `func_XXXXXXXX.c` lifts. `partial` means the source builds
and has required metadata but does not byte-match. This is an observable triage
catalog, not a claim that the listed symptom is the root cause.

## Summary by target

| Target | Partial | Match % (min / median / max) | Same-size | Size delta | ≥80% | ≥95% |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `emi/battle/battle/03` | 103 | 0.60 / 54.76 / 98.94 | 28 | 75 | 15 | 5 |
| `emi/battle/battle/15` | 38 | 11.11 / 50.00 / 89.29 | 6 | 32 | 7 | 0 |
| `exe/slus_004_22` | 18 | 0.09 / 57.55 / 98.51 | 4 | 14 | 4 | 3 |
| `emi/scenario/scena16/00` | 13 | 32.78 / 65.12 / 93.33 | 2 | 11 | 2 | 0 |
| `emi/etc/game/00` | 12 | 15.10 / 43.68 / 83.87 | 2 | 10 | 1 | 0 |
| `emi/etc/shop/00` | 8 | 37.50 / 52.94 / 53.33 | 0 | 8 | 0 | 0 |
| `emi/world00/area008/13` | 6 | 14.29 / 54.92 / 94.12 | 2 | 4 | 2 | 0 |
| `emi/etc/game/01` | 5 | 20.00 / 81.36 / 90.91 | 0 | 5 | 3 | 0 |
| `emi/world00/area024/14` | 5 | 23.33 / 58.82 / 79.03 | 1 | 4 | 0 | 0 |
| `emi/world00/area030/04` | 5 | 40.82 / 68.57 / 90.48 | 0 | 5 | 2 | 0 |
| `emi/world00/area016/13` | 3 | 56.15 / 64.71 / 94.12 | 2 | 1 | 1 | 0 |
| `emi/world00/area027/13` | 3 | 45.10 / 63.75 / 82.46 | 1 | 2 | 1 | 0 |
| `emi/world00/area028/13` | 3 | 47.62 / 69.57 / 71.05 | 1 | 2 | 0 | 0 |
| `emi/etc/sisyou/00` | 1 | 1.10 / 1.10 / 1.10 | 0 | 1 | 0 | 0 |
| `emi/scenario/sce10eff/00` | 1 | 73.08 / 73.08 / 73.08 | 0 | 1 | 0 | 0 |
| **Total** | **224** | — | **49** | **175** | **38** | **8** |

## First-diff symptom categories

The categories below are derived mechanically from each live `asm-diff --detail
normal` first hunk plus code-size delta. They guide which rung of the matching
ladder to revisit; they do **not** authorize a pin or other aid.

| First observed symptom | Count | First response |
| --- | ---: | --- |
| Code-size delta with frame/register operations | 102 | Recheck types, lifetimes, temporary/hoist shape, and frame-causing control flow. |
| Code-size delta, other structural/allocation difference | 73 | Recheck structure, data ownership, branch/loop shape, and materialization. |
| Same-size address/load-register ordering | 28 | Recheck symbol representation, volatile qualification, address materialization, and local ordering. |
| Near-exact scheduling/delay slot (≥95%, same size) | 7 | Diagnose the specific instruction and delay slot; try clean-C ordering, then an evidenced clobber if applicable. |
| Same-size register/expression scheduling | 7 | Recheck temporary lifetime, expression order, and local hoists. |
| Same-size control-flow/scheduling | 7 | Recheck branch polarity, early-return/if-else shape, and delay slot. |

## Highest-priority partials

These are the closest live partials, ordered by match percentage. They are
candidates for a bounded, evidence-driven retry rather than accepted lifts.

| Target@address | Match | Instructions | Bytes | First observed symptom |
| --- | ---: | ---: | ---: | --- |
| `emi/battle/battle/03@0x801D3844` | 98.94% | 93/94 | 376/376 | same-size near-exact scheduling/delay slot |
| `exe/slus_004_22@0x80162B08` | 98.51% | 66/67 | 268/268 | same-size near-exact scheduling/delay slot; source documents the `j epilogue` delay-slot difference |
| `exe/slus_004_22@0x80161FDC` | 96.94% | 95/97 | 388/392 | +4 byte codegen delta |
| `exe/slus_004_22@0x80162500` | 96.49% | 55/57 | 228/228 | same-size near-exact scheduling/delay slot |
| `emi/battle/battle/03@0x801DD448` | 96.47% | 82/85 | 340/340 | same-size near-exact scheduling/delay slot |
| `emi/battle/battle/03@0x801DEDE4` | 96.15% | 25/26 | 104/104 | same-size near-exact scheduling/delay slot |
| `emi/battle/battle/03@0x801DEE4C` | 96.15% | 25/26 | 104/104 | same-size near-exact scheduling/delay slot |
| `emi/battle/battle/03@0x801E4368` | 95.95% | 71/74 | 296/296 | same-size near-exact scheduling/delay slot |
| `emi/battle/battle/03@0x801E31C8` | 94.12% | 16/17 | 68/68 | same-size address/load-register ordering |
| `emi/world00/area008/13@0x801F2C18` | 94.12% | 16/17 | 68/68 | same-size address/load-register ordering |
| `emi/world00/area016/13@0x801F368C` | 94.12% | 16/17 | 68/68 | same-size register/expression scheduling |
| `emi/battle/battle/03@0x801DDAB4` | 93.33% | 14/15 | 60/60 | same-size address/load-register ordering |
| `emi/scenario/scena16/00@0x801F6C90` | 93.33% | 14/15 | 60/60 | same-size register/expression scheduling |
| `emi/world00/area008/13@0x801F3D18` | 93.10% | 27/28 | 112/116 | +4 byte codegen delta |
| `emi/battle/battle/03@0x801E019C` | 92.86% | 26/28 | 112/112 | same-size address/load-register ordering |
| `emi/battle/battle/03@0x801DF8AC` | 92.31% | 24/26 | 104/104 | same-size control-flow/scheduling |
| `emi/battle/battle/03@0x801DDE7C` | 91.30% | 21/23 | 92/92 | same-size address/load-register ordering |
| `emi/etc/game/01@0x801D0E54` | 90.91% | 40/43 | 172/176 | +4 byte codegen delta |
| `emi/scenario/scena16/00@0x801F6E30` | 90.91% | 30/32 | 128/132 | +4 byte codegen delta |
| `emi/world00/area030/04@0x801D6A2C` | 90.48% | 57/63 | 252/248 | -4 byte codegen delta |

## Reproducing the complete table

The complete disposable, per-function audit is written to
`out/non-exact-lifts.tsv`; it is intentionally not tracked. Regenerate it from
live status and `asm-diff` before acting on a row:

```sh
bin/decomp-status --json --no-cache
bin/asm-diff TARGET@0xADDRESS --detail normal
```

The next attempt must follow the [matching ladder](../../matching-playbook.md)
and still accepts only a live `bin/byte-match` result.
