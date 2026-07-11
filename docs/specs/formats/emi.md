---
type: Format
title: EMI container
description: BOF3 EMI container and entry layout.
tags: [emi, format]
---

# EMI

EMI is a sector-aligned archive. The archive is a container, not an executable
target; extracted entries are classified independently.

## Header

| Offset | Size | Type | Meaning |
| ---: | ---: | --- | --- |
| `0x00` | 4 | `u32` | entry count |
| `0x04` | 4 | `u32` | format version (`1` in the US v1.1 corpus) |
| `0x08` | 8 | `char[8]` | `MATH_TBL` |

## TOC entry

| Offset | Size | Type | Meaning |
| ---: | ---: | --- | --- |
| `0x00` | 4 | `u32` | payload size |
| `0x04` | 4 | `u32` | type-dependent load argument |
| `0x08` | 4 | `u32` | cached first payload word |
| `0x0c` | 2 | `u16` | type id |
| `0x0e` | 2 | `u16` | padding (`0x2e2e` in the US v1.1 corpus) |

The first payload starts at `0x800`. Every payload starts on a `0x800` byte
boundary:

```c
next = current + ((size + 0x7ff) & ~0x7ff);
```

## Load argument

The field at TOC offset `0x04` is not always a CPU pointer:

| Entry type | Interpretation |
| ---: | --- |
| `0`, `1`, `2` | CPU destination or loader state input |
| `3` | packed graphics upload descriptor |
| `6`–`10` | audio bank or sequence selector |

## Type map

| Type | Confirmed or bounded role |
| ---: | --- |
| `0` | generic RAM payload; may contain code or data |
| `1`, `2` | queued RAM payload with bookkeeping |
| `3` | raw graphics upload payload |
| `4`, `5` | shared special handler; semantics not established |
| `6` | VAB header (`VH`) |
| `7` | VAB body (`VB`) |
| `8` | auxiliary audio payload |
| `9`, `10` | sequence-side payload; type `10` is `SEQ` |

Type `0` alone does not prove executable code. Target promotion additionally
requires archive identity, slot, extracted bytes, load address, and reviewed
code evidence.

The header, TOC, and alignment agree with the pinned
[`BoF3-Data-Doc`](../../../third_party/references/bof3-data-doc/src/DataStructures/1_TheEmiFiles.md).
Local archives and `SLUS_004.22` remain authoritative.

## Canonical data

- Tracked layouts: `config/splat/`
- Tracked symbols: `config/symbols/`
- Generated entry catalog: `out/catalog/`
- Extracted entries: `out/extracted/`
