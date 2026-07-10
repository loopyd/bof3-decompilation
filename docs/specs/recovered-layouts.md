---
type: Recovered layout reference
title: Recovered memory layouts
description: Locally evidenced working byte layouts for BOF3 lifting work.
tags: [layouts, structs, offsets, evidence]
---

# Recovered Memory Layouts

> Byte layouts declared by local reverse-engineering context; field semantics remain unknown unless named below.

## Evidence boundary

These are compact working facts for lifts. They are not a stable game API and
must be rechecked against the original binary before promotion into shared
headers. `unk_*` and `pad_*` names record only an observed offset or gap.

## EMI transfer state

`EmiTransferSlot` is declared in the lifted EXE function
[`src/core/emi/func_80162b08.c`](../../src/core/emi/func_80162b08.c).
The function reads and writes these fields, so their offsets and widths are
locally evidenced.

| Offset | Width | Declared field | Evidence |
| ---: | ---: | --- | --- |
| `0x00` | 4 | `size` | copied to `DAT_80146454` |
| `0x04` | 4 | `remaining_size` | copied to `DAT_80146458` |
| `0x08` | 4 | `read_offset` | copied to `DAT_8014645c` |
| `0x0c` | 2 | `state` | copied to `DAT_80146460`; compared with `6` |

Declared minimum size: `0x10` bytes. The array base, allocation count, and
the meanings of the destination registers are **UNKNOWN**.

## `BATTLE.EMI#3` working arrays

Legacy build metadata maps `BIN/BATTLE/BATTLE.EMI#3` to `0x801d0c00`
([`cmake/modules/battle.cmake`](../../cmake/modules/battle.cmake)).
The context for that module declares the following array bases and C layout
strides. The byte layouts are evidenced by
[`src/modules/battle/03/internal.h`](../../src/modules/battle/03/internal.h).

| Base | Declared element | Stride / minimum size | Confidence |
| --- | --- | ---: | --- |
| `0x80145e90` | `Battle03LocalWork` | `0x140` | layout declaration |
| `0x801eb630` | `Battle03EnemyWork` | `0x118` | layout declaration |
| `0x801ec330` | `Battle03QueuedSlot` | `0x78` | declaration plus indexed-address macros |

Known layout anchors:

| Element | Offset | Declared representation |
| --- | ---: | --- |
| `Battle03LocalWork` | `0x00` | `u8 flags_00` |
| `Battle03LocalWork` | `0x0c`–`0x20` | six `u32` fields |
| `Battle03LocalWork` | `0x34`, `0x38`, `0x3e`, `0x48` | `s32`, `s32`, `s16`, `u8` fields |
| `Battle03EnemyWork` | `0xe4` | function pointer taking `s32` |
| `Battle03EnemyWork` | `0xf0` | `u8` field |
| `Battle03QueuedSlot` | `0x00`, `0x05`, `0x06`, `0x74` | `u8`, `u8`, `u8`, `u32` fields |

`Battle03*` names identify the analysis context, not proven original names.
Array lengths, ownership, and all `unk_*` meanings are **UNKNOWN**.

## Related facts

- The tracked target configuration for this slot is
  [`config/splat/emi/battle/battle/03.yaml`](../../config/splat/emi/battle/battle/03.yaml).
- The EMI container and its runtime load argument are documented in
  [EMI Format](formats/emi.md); a CPU-looking load argument alone does not
  establish that a payload is executable.
