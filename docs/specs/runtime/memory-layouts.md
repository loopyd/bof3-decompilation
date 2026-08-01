---
type: Runtime
title: Recovered layouts
description: Byte layouts required by current lifted code.
tags: [runtime, offsets]
---

# Recovered layouts

These offsets are evidenced by current lifted accesses. Names describe the
analysis context, not original symbols.

## EMI transfer slot

| Offset | Width | Field |
| ---: | ---: | --- |
| `0x00` | 4 | size |
| `0x04` | 4 | remaining size |
| `0x08` | 4 | read offset |
| `0x0c` | 2 | state |

Minimum size: `0x10`.

## Battle target working arrays

| Base | Element | Stride |
| --- | --- | ---: |
| `0x80145e90` | local work | `0x140` |
| `0x801eb630` | enemy work | `0x118` |
| `0x801ec330` | queued slot | `0x78` |

| Element | Offset | Representation |
| --- | ---: | --- |
| local work | `0x00` | `u8` flags |
| local work | `0x0c`–`0x20` | six `u32` fields |
| local work | `0x34`, `0x38`, `0x3e`, `0x48` | `s32`, `s32`, `s16`, `u8` |
| enemy work | `0xe4` | function pointer |
| enemy work | `0xf0` | `u8` |
| queued slot | `0x00`, `0x05`, `0x06`, `0x74` | `u8`, `u8`, `u8`, `u32` |

Recheck offsets against original assembly before moving declarations into a
shared header.

When an access requires symbol-relative relocation, declare and use the owning
mapped target-local data symbol rather than a same-address local alias; identical
addresses do not guarantee identical linked relocation or code generation.
