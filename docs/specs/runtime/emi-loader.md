---
type: Runtime
title: Loader
description: SLUS EMI entry dispatch and loading rules.
tags: [runtime, emi, loader]
---

# EMI loader

## Entry dispatch

| Type | Handler | Role |
| ---: | --- | --- |
| `0` | `0x801625e4` | direct RAM copy |
| `1`, `2` | `0x80162618` | queued RAM load |
| `3` | `0x80162698` | graphics upload |
| `4`, `5` | `0x80162500` | shared special path |
| `6` | `0x80162790` | VAB header |
| `7` | `0x80162898` | VAB body |
| `8` | `0x801629f0` | auxiliary audio buffer |
| `9`, `10` | `0x80162a6c` | sequence-side copy |

## Core functions

The tracked loader module is the contiguous SLUS range
`0x80161f58..0x80162d9c`. The range currently has nineteen reviewed function
boundaries. Functions outside that range are callers or neighboring EXE
services, even when an EMI overlay calls them.

| Address | Proven role |
| --- | --- |
| `0x80161f58` | initialize EMI/CD loader state |
| `0x80161fdc` | begin an EMI stream |
| `0x80162160` | read a slot-table LBA |
| `0x80162178` | initialize a transfer |
| `0x801621e8` | CD sync callback |
| `0x80162230` | service the active entry |
| `0x80162d00` | return whether loader state is ready |

## EXE-side callers and neighboring services

These addresses belong to `SLUS_004.22`, not to a loaded EMI entry. An EMI
target may bind them at its fixed EXE address, but that does not transfer source
ownership to the overlay.

| Address | Proven role | Ownership |
| --- | --- | --- |
| `0x80161808` | select the frontend layout-bank pointer set | neighboring EXE service |
| `0x80161c20` | start and record a selection cue | neighboring EXE service |
| `0x80161cd0` | update a selection cue with level and shape arguments | neighboring EXE service; semantic distinction from `0x80161c20` remains unresolved |
| `0x8016728c` | map a content family/index to a slot and start its stream | EXE-side loader caller |

`0x8016728c` is therefore part of the wider EMI dispatch path, but not part of
the contiguous loader module above. Keep its compiled name address-based until
its public role and argument vocabulary are reviewed.

## Graphics

Type `3` uses a packed load argument and uploads raw sector-sized graphics
chunks. Each `0x800` byte chunk is a `32x32` rectangle of 16-bit VRAM words:
`128x32` pixels at 4bpp or `64x32` at 8bpp. Palette data commonly travels
separately in small type-`0` RAM payloads. See [EMI graphics](../formats/graphics.md).

## Audio

Types `6` and `7` form VAB header/body pairs. Type `10` carries sequence data.
Their load argument is a logical bank selector, not a raw destination pointer.

## Companion static-call records

A caller EMI manifest may record a catalog-verified static call into a separately
owned companion EMI target. The record repeats the companion disc entry, SHA-256,
base, size, original caller address, and direct `jal` target. Catalog validation
checks those immutable fields and the original caller instruction; it never adds
symbols, weak bindings, linker inputs, or reverse-index call edges across targets.

`emi/world00/area030/04` records `0x801E0C28 -> 0x800F500C` into independently
bootstrapped `BIN/WORLD00/AREA030.EMI#5` (`0x800F5000`, `0x2250` bytes). This
proves a static call target within the extracted payload, not concurrent residency,
load order, ABI, entrypoint ownership, relocation behavior, shared source, or a
runtime service. The lift gate requires reviewed boundary/map, ABI, and caller
prototype evidence; it deliberately does not require emulator co-load evidence.

## Matching invariant

Before matching C, the selected entry, load address, function boundary, and
canonical target assembly must reproduce the original function bytes.
