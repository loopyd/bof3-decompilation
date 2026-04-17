# SCENA16 Overlay

This document records the first scenario overlay recovered far enough to guide PSX gameplay-flow recovery.

Target archive:

- `processed/emi_raw/BIN/SCENARIO/SCENA16`

Relevant entry:

- entry `0`
  - load address `0x801f6c00`
  - `type = 0`
  - size `6520`
  - single code-bearing payload

## Why This Overlay Matters

`SCENA16` is the first proven follow-on gameplay slice after the `GAME.EMI` front controller.

The currently proven control flow is:

1. `GAME.EMI` entry `1` calls `game_boot_scenario(0x10)`
2. `GAME.EMI` entry `0` requests slot `0x2a5`
3. slot `0x2a5` resolves to `BIN/SCENARIO/SCENA16.EMI`
4. `GAME.EMI` waits on `emi_ready()`
5. `GAME.EMI` looks up `DAT_801c8454[0x10]`
6. that table entry is `0x801f8538`
7. `GAME.EMI` calls the first word of the object at `0x801f8538`
8. the first word is `0x801f6c90`

@source: 0x801a7704 FUN_801a7704
@source: 0x801a782c FUN_801a782c

That is the first locally proven `GAME -> SCENAxx` handoff concrete enough to document directly.

## Dispatch Root

For scenario index `0x10`, the `GAME.EMI` backing segment stores:

- `DAT_801c8454[0x10] = 0x801f8538`

The object rooted at `0x801f8538` in `SCENA16` currently decodes as:

| Offset | Address | Suggested role | Confidence |
| --- | --- | --- | --- |
| `+0x00` | `0x801f8538` -> `0x801f6c90` | top dispatcher | high |
| `+0x04` | `0x801f853c` -> `0x801f8358` | per-record callback dispatcher | medium |
| `+0x08` | `0x801f8540` -> `0x801f83a0` | null/return-0 helper | high |
| `+0x0c` | `0x801f8544` -> `0x801f83a8` | null/return-0 helper | high |
| `+0x10` | `0x801f8548` -> `0x0` | reserved/null | high |
| `+0x14` | `0x801f854c` -> `0x801f6ccc` | primary state `0` | high |
| `+0x18` | `0x801f8550` -> `0x801f6d90` | primary state `1` | high |
| `+0x1c` | `0x801f8554` -> `0x801f7144` | primary state `2` | high |
| `+0x20` | `0x801f8558` -> `0x801f7180` | secondary state `0` | high |
| `+0x24` | `0x801f855c` -> `0x801f7188` | secondary state `1` | medium |
| `+0x28` | `0x801f8560` -> `0x801f7230` | secondary state `2` | medium |
| `+0x2c` | `0x801f8564` -> `0x801f7790` | secondary state `3` | medium |
| `+0x30` | `0x801f8568` -> `0x801f7cc4` | secondary state `4` | medium |
| `+0x34` | `0x801f856c` -> `0x801f8398` | per-record no-op | high |
| `+0x38` | `0x801f8570` -> `0x801f84ac` | reset/teardown helper | medium |
| `+0x3c` | `0x801f8574` -> `0x801f8530` | empty tail helper | high |

Interpretation:

- the object begins with a callable top dispatcher
- the real primary state table begins at `0x801f854c`
- a nested secondary state table begins at `0x801f8558`

## Top Dispatcher

`0x801f6c90` is the current best semantic root for `SCENA16`.

@source: 0x801f6c90 FUN_801f6c90

Observed pseudocode:

```c
void scena16_tick(void) {
  scena16_primary_table[DAT_80146872]();
}
```

Current interpretation:

- `DAT_80146872` is the primary scenario state for this overlay
- `SCENA16` is not a single routine, it is an object with nested dispatch tables

## Primary States

| Address | Suggested name | Confidence | Role |
| --- | --- | --- | --- |
| `0x801f6ccc` | `scena16_primary_boot` | medium | allocates frontend resources, requests loader slot `6`, waits on `emi_ready()`, then advances to primary state `1` |
| `0x801f6d90` | `scena16_primary_route_area_archive` | medium | examines overlay-owned `_DAT_80143f00` as the active `AREA Archive Id` where the `GAME.EMI` `+0x2ab` path is locally proven, routes to `AREA002` / `AREA004` / `AREA031` helpers, then advances to primary state `2`; the currently proven direct secondary targets are state `2` via `AREA004` and state `3` via `AREA031` |
| `0x801f7144` | `scena16_primary_tick_secondary` | high | dispatches through the secondary state table using `DAT_80146874` |

### `0x801f6ccc` (`scena16_primary_boot`)

@source: 0x801f6ccc FUN_801f6ccc

Observed pseudocode:

```c
void scena16_primary_boot(void) {
  frontend_reset_slot(0);
  DAT_8014832e = 0x1f;
  queue_frontend_resource(4, 0x1a0000, 0x88000, 5);
  enable_frontend_flags(0x240);
  request_slot(6);
  while (!emi_ready()) {
    scheduler_yield(1);
  }
  DAT_80146864 = 0;
  DAT_80146872 = 1;
}
```

### `0x801f6d90` (`scena16_primary_route_area_archive`)

@source: 0x801f6d90 FUN_801f6d90

This routine branches on overlay-owned `_DAT_80143f00`:

- `2` -> `0x801f6f30` (`scena16_area002_setup_staged_resources`)
- `4` -> `0x801f6eb0` (`scena16_area004_seed_enter_secondary_a`)
- `0x1f` -> `0x801f6e30` (`scena16_area031_seed_enter_secondary_b`)
- otherwise it falls through to primary state `2`

Router follow-up currently proven in this overlay:

- `AREA002` and `AREA004` return through a common tail that sets `DAT_80143c30 = 1`
- `AREA031` special-cases `DAT_80143c30 = 0`
- all handled branches then set `DAT_80146872 = 2`
- `AREA004` directly seeds `DAT_80146874 = 2`
- `AREA031` directly seeds `DAT_80146874 = 3`
- `AREA002` performs staged resource setup here but does not directly write `DAT_80146874`

Current bounded router summary:

- in the proven `GAME.EMI` entry `0` area-load path, `0x8019faa0` writes `_DAT_80143f00` as the active `AREA Archive Id`
- that same path requests slot `id + 0x2ab`, so the currently covered values map into the `WORLD00/AREAxxx` archive range
- `0x801f6d90` is therefore a bounded area-router for this overlay, with locally proven helper-specific secondary-state seeding for `AREA004` and `AREA031`
- this should still only be reused where the `+0x2ab` load path is locally proven

@source: 0x8019faa0 FUN_8019faa0

## Secondary States

`0x801f7144` dispatches through `&PTR_FUN_801f8558[DAT_80146874]`.

@source: 0x801f7144 FUN_801f7144

Observed pseudocode:

```c
void scena16_tick_secondary(void) {
  scena16_secondary_table[DAT_80146874]();
}
```

Current known entries:

| Address | Suggested name | Confidence | Role |
| --- | --- | --- | --- |
| `0x801f7180` | `scena16_secondary_idle` | high | no-op |
| `0x801f7188` | `scena16_secondary_finalize_and_exit` | medium | clears frontend state, queues cue/event ids `0x213` and `0x214` through `0x8015df18`, optionally closes selection FX, then calls `func_0x8014b8b0()` |
| `0x801f7230` | `scena16_secondary_state_machine_a` | medium | nested state machine over `DAT_80146875`, using `0x7a` resource ids and fade-style transitions; directly entered when the `AREA004` helper seeds secondary state `2` |
| `0x801f7790` | `scena16_secondary_state_machine_b` | medium | nested state machine over `DAT_80146875`, using `0x50` resource ids and another transition path; directly entered when the `AREA031` helper seeds secondary state `3`, then later hands off to secondary state `4` |
| `0x801f7cc4` | `scena16_secondary_state_machine_c` | medium | nested state machine over `DAT_80146875`, includes palette copy/fade work through `FUN_801f83b0` and `FUN_801f845c`; reached after secondary state `3` and later returns to secondary state `1` |
| `0x801f84ac` | `scena16_secondary_reset_effect_bank` | medium | resets one effect group and marks a frontend flag set |

## Helper Roles

| Address | Suggested name | Confidence | Role |
| --- | --- | --- | --- |
| `0x801f6e30` | `scena16_area031_seed_enter_secondary_b` | medium | area-routed helper for `AREA031`; seeds `_DAT_801492d8 = 0x100`, `_DAT_801492dc = 0`, `_DAT_801492da = 0`, `_DAT_8014932c = 0x100`, calls `func_0x8015c100()`, polls `func_0x8015b5d4(_DAT_8014686c, 1)`, then on success clears `DAT_8014832e`, acknowledges with `func_0x8015b580(..., 1)`, and sets `DAT_80146874 = 3` @source: 0x801f6e30 FUN_801f6e30 |
| `0x801f6eb0` | `scena16_area004_seed_enter_secondary_a` | medium | area-routed helper for `AREA004`; polls `func_0x8015b5d4(_DAT_8014686c, 0)`, then on success clears `DAT_8014832e`, acknowledges with `func_0x8015b580(..., 0)`, calls `func_0x8015c088()`, and sets `DAT_80146874 = 2`; if `DAT_80143f03 == 2`, it also writes `DAT_8014832e = 0x1f` and calls `func_0x8015c100()` @source: 0x801f6eb0 FUN_801f6eb0 |
| `0x801f6f30` | `scena16_area002_setup_staged_resources` | medium | area-routed helper for `AREA002`; waits on loader phases `2` then `10`, clears `_DAT_80146864`, stages resources and one local object, bumps `_DAT_801492d8 += 0xaa`, seeds `_DAT_80145ec4 = 0x330000`, `_DAT_80145ec8 = 0x400000`, then later sets `_DAT_80149308 = 0x260000` and `_DAT_8014930c = 0x1b8000`; does not directly write `DAT_80146874` @source: 0x801f6f30 FUN_801f6f30 |
| `0x801f8358` | `scena16_dispatch_record_callback` | medium | dispatches through a callback table selected by byte `0x7a` in a scenario record and passes `_DAT_8014686c` |
| `0x801f83b0` | `scena16_fade_palette_window` | medium | copies a 16-entry palette block from `0x80033800` to `0x80037800`, clamping each 5-bit component to the requested intensity |
| `0x801f845c` | `scena16_copy_palette_block` | high | copies 16 palette words from `0x80033800` to `0x80037800` and bumps `DAT_80145988` |
| `0x801f8398` | `scena16_return_zero` | high | returns `0` |
| `0x801f8530` | `scena16_noop` | high | returns immediately |

## Current Recovery Meaning

`SCENA16` is now recovered far enough to improve the documented handoff model:

- `GAME.EMI` should not only pass `scenario_index`
- it should also pass the proven scenario dispatch root for covered scenarios

For the currently proven boot path:

- scenario index `0x10`
- slot `0x2a5`
- dispatch root `0x801f8538`
- top dispatcher `0x801f6c90`

That is enough to replace the old opaque `enter_scenario_archive(index)` model with a more faithful `scenario_index + dispatch_root` handoff description.

Current seam clarification:

- `0x801f7188` is still the strongest latest pre-game gameplay-side cutoff
- the queued ids `0x213` and `0x214` should currently be read as cue/event ids, not proven scene-transition ids
- `0x8014b8b0` is the immediate EXE-side slot/thread exit helper below that cutoff, not yet a proven gameplay-engine module by itself

## Recovered `SCENA16` State Chain

Artifact-backed Ghidra C bundles now make the local controller split clearer.

### Top-level dispatcher

- `0x801f6c90` dispatches through `PTR_FUN_801f854c[DAT_80146872]`
- recovered entries currently resolve as:
  - state `0` -> `0x801f6ccc` bootstrap/layout seed
  - state `1` -> `0x801f6d90` route select on `_DAT_80143f00`
  - state `2` -> `0x801f7144` secondary dispatcher on `DAT_80146874`
  - state `3` -> `0x801f7180` no-op / wait state
  - state `4` -> `0x801f7188` finalize-and-exit seam
  - state `5` -> `0x801f7230` large secondary controller

### Secondary dispatcher

- `0x801f7144` dispatches through `PTR_LAB_801f8558[DAT_80146874]`
- recovered entries currently resolve as:
  - state `0` -> `0x801f7180`
  - state `1` -> `0x801f7188`
  - state `2` -> `0x801f7230`
  - state `3` -> `0x801f7790`
  - state `4` -> `0x801f7cc4`

### Route helper relationships

- route `4` (`0x801f6eb0`) writes `DAT_80146874 = 2`, so it enters the
  secondary controller rooted at `0x801f7230`
- route `0x1f` (`0x801f6e30`) writes `DAT_80146874 = 3`, so it enters the
  secondary controller rooted at `0x801f7790`
- route `2` (`0x801f6f30`) performs the heaviest setup/resource work, but it
  does not directly write `DAT_80146874` in the recovered C; its exact
  secondary-controller handoff is still unresolved

### Current strongest branch interpretation

- route `2` remains the strongest current candidate for the branch closest to
  gameplay/resource preparation because `0x801f6f30` performs the largest staged
  setup and object/loader work
- routes `4` and `0x1f` clearly participate in the same broader secondary chain,
  with route `4` eventually able to re-author area/layout `0x1f`
