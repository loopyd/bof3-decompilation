# Interrupt state

## Purpose

Use [irq.lua](../scripts/irq.lua) for serialized interrupt latches and masks.
Use [History](history.md) for ordered latch/delivery observations.

## Procedure

Follow [Runtime](runtime.md#invocation); declare `support`, `snapshot`,
`devices`. A state without target/EXE is inspected offline; other contexts use
the common paused boundary. Check `irq.json` (`psx.runtime-irq/v1`) for I_STAT,
I_MASK, eleven lines, enabled pending bits and CP0 Status/Cause masks.

## Application

CPU global enable, hardware enable and stored CPU pending state remain separate.
Device pending bits do not prove handler execution. Software CP0 bits remain in
raw banks/masked-pending values. Inspection neither acknowledges nor consumes
device state; serialized levels do not supply edge order.

Decode follows [devices.lua](../scripts/devices.lua). I_STAT/I_MASK low eleven
bits map in order to Vblank, GPU, CD-ROM, DMA, timer0, timer1, timer2, SIO0, SIO1,
SPU and lightpen/PIO. `pending_enabled = I_STAT & I_MASK & 0x7ff`.
CP0 Status bit 0 supplies global enable; Status bit 10 hardware enable; Cause
bit 10 stored hardware pending. `cpu_masked_pending = Status & Cause & 0xff00`
includes software bits. Neither expression proves CPU acceptance or handler entry.

Acknowledgment follows each controller's semantics; verify native effects against
pinned `third_party/pcsx-redux/src/core/psxhw.cc`, `psxmem.h` and `r3000a.cc`.
[MMIO probes](bus.md) advance time and can alter interrupt state; plain serialized
inspection does neither.

Provenance only: [PSX interrupts](https://psx-spx.consoledev.net/interrupts/).
Routine interpretation uses this contract; external research requires explicit scope.
