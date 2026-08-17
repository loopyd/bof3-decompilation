---
type: Runtime spec
title: Psy-Q constants and layout evidence
description: SDK-backed constants, ABI declarations, and match-preserving layout rules.
tags: [runtime, psyq, evidence]
---

# Psy-Q constants and layout evidence

The official Psy-Q 4.7 headers are the declaration source for SDK APIs and
hardware/event constants. A constant is promoted into lifted C only when its
numeric value and use are both established; otherwise the address-based or
numeric form remains part of the match evidence.

## Exact promotions

| Lifted use | Official declaration | Owner | Owner status |
| --- | --- | --- | --- |
| `SwCARD`, `HwCARD`, `HwCPU` | `kernel.h` descriptors | `src/bof3/boot/openBootEventSet.c`, `initBootDiscEvents.c` | exact |
| `EvSpIOE`, `EvSpTIMOUT`, `EvSpNEW`, `EvSpERROR` | `kernel.h` event specifications | `src/bof3/boot/openBootEventSet.c` | exact |
| `EvMdNOINTR`, `EvMdINTR`, `EvSpTRAP` | `kernel.h` event modes/specification | `src/bof3/boot/openBootEventSet.c`, `initBootDiscEvents.c` | exact |
| `PADstart` | `libetc.h` controller bit `1 << 11` | `src/bof3/boot/playCapcomStream.c`, `src/bof3/support/slus_slot_table_logo_str.c` | exact (playCapcomStream); support source, not a lift (slot_table_logo_str) |
| `CdlSetloc` | `libcd.h` command `0x02` | `src/bof3/io/func_8014E0FC.c` | partial (not exact) |
| `MODE_NTSC` | `libetc.h` video mode `0` | `src/bof3/boot/initBootDiscEvents.c` | exact |
| `CdlComplete` | `libcd.h` callback status `0x02` | `src/bof3/io/emiCdSyncCallback.c` | exact |

For the **exact** owners above, adopting the official declaration was required to
preserve the original instruction and byte match. `CdlSetloc` (`func_8014E0FC`)
is listed because the same official constant substitution applies at the call
site, but the owning lift is not exact; the row is not an exact promotion
until the owner byte-matches.

The project wrapper [include/bof3/psyq.h](../../../include/bof3/psyq.h)
imports the headers; it does not redeclare their constants.

`bin/harness psyq scan --all` refreshes the ignored catalog at
`out/psyq/4.7/headers.json`. The current catalog contains 2,301 macros, 875
function declarations, 239 types, and 28 variables; those records are header
evidence, not automatic semantic renames.

## CD sector units

Psy-Q `CdGetSector` counts 32-bit words. The SDK names its word unit
`SECTOR_SIZE = 512`; BOF3's `EMI_SECTOR_SIZE = 0x800` in
`include/bof3/core.h` is the corresponding byte size. The remaining
`0x200` call arguments and `>> 11` address arithmetic are retained until each
caller has an owned, match-validated name; do not silently treat them as the
same unit.

## Structures and offsets

`RECT`, `DISPENV`, `DRAWENV`, `MATRIX`, `SVECTOR`, `VECTOR`, `CdlLOC`, and
`CdlFILE` come directly from the official headers. Game and overlay work
records remain target-local because their layouts are inferred from runtime
accesses. Their field offsets must be checked against the owning function's
assembly before changing widths or signedness.

The SLUS loader's `D_80146518` table is an exact address-backed view at
`D_80146494 + 0x84` and is recorded in the target map. The owned source
(`emiCdReadyCallback`, `@source 0x80162230`, exact) binds
`read_progress = &D_80146494` and derives the slot-size table as
`(u32 *)(read_progress + 0x84)` without naming `D_80146518`; keep that
arithmetic until a caller is lifted to use the mapped name.

## Deliberate discrepancies

- The SLUS sound bindings currently use match-preserving local prototypes that
  differ in spelling or return width from `libspu.h`/`libsnd.h`. Correcting
  them globally can change ABI/code generation; validate one function at a
  time before promotion.
- `VPPTR` and the scratchpad pointer cells are intentionally unchanged. The
  existing spelling preserves current matches; changing volatile pointer-cell
  qualification is a code-generation experiment, not a formatting cleanup.
- When original code reloads a volatile pointer cell between mixed-width
  accesses, keep the accesses as distinct pointer-cell expressions instead of
  caching the pointer in one local; the reload order can be match-significant.
- For a signed-halfword store through an address-held pointer cell, retain a
  volatile pointer-cell binding and use a plain signed-halfword assignment.
  Preserve the original reload shape: use a local pointer when it reloads once,
  or a direct pointer-cell expression when that is the matching form.
- Raw event descriptors, EMI state values, and overlay offsets stay numeric
  when the SDK does not provide an authoritative name.

## Verification

For every promotion, run the owning function through both gates:

```sh
bin/asm-diff TARGET@0xADDRESS
bin/byte-match TARGET@0xADDRESS
```

`bin/harness psyq scan --all` remains the source for SDK object identity; it
does not prove that a game-local value has the same meaning as a Psy-Q macro.
