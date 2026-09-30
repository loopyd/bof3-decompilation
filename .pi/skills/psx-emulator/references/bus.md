# Native device writes

## Purpose

Use [bus.lua](../scripts/bus.lua) for VRAM uploads, SPU writes and CD commands.
Debugger memory-file writes bypass handlers; this helper executes MIPS stores.
Declare `support`, `snapshot` and `bus`.

## Procedure

1. Choose a paused target, EXE entry or restored context and aligned
   `scratch=ADDRESS` in physical 2 MiB RAM. The guest, stack, IRQ handlers and
   DMA must not need it during the probe.
2. Supply the action's bounded operands. Scratch consumes **52 + 8 × N** bytes
   for N stores, entirely inside RAM; maximum 262000 stores. Set a finite timeout.
3. Verify the probe report and device before/after state. Use `savestate=1` when
   continuing the experiment.

Pending branch/load delay state rejects; absent serialized delay submessages
mean inactive defaults. Active history or audio recording rejects host setup edits.

## Application

At its terminal breakpoint the probe restores scratch, GPRs including HI/LO and
starting PC, invalidating code caches around replacement. It does not rewind
elapsed device time, undo MMIO effects or isolate IRQ/DMA. Scratch selection alone
does not prove quiescence. Failures cannot publish passing captures; effects stay
inside the emulator process and original media/state inputs remain unchanged.
