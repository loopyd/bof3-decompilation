# Memory inspection and patching

## Purpose

Use [memory.lua](../scripts/memory.lua) for physical bytes without triggering MMIO.
Follow [Runtime](runtime.md#invocation).

## Procedure

Declare `support`; restoration requires `snapshot`. Select the stopped context
and `region=ram|scratch|rom` (RAM), zero-based `offset` (0), `length` (256).
Bounds are 2 MiB, 1024 bytes and 512 KiB respectively.

Inspect `before.bin` and `memory.json`. Optional `hex=12345678` requires
exactly `length` byte pairs, writes RAM/scratch, invalidates code caches and
produces `after.bin`. ROM writes reject. Use `savestate=1` to retain edits.

## Application

Verify before/after bytes and receipt. Direct pointers do not model bus accesses,
watchpoints, caches or DMA. Use [VRAM](vram.md), [SPU](spu.md), [CD-ROM](cdrom.md)
and [native writes](bus.md) for device effects, or saved-state queries for
side-effect-free device inspection.
