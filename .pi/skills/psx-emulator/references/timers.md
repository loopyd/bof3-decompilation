# Timer state

## Purpose

Use [timers.lua](../scripts/timers.lua) for decoded serialized counters.
Use [History](history.md) for actual reads, transitions and IRQ ordering.

## Procedure

Follow [Runtime](runtime.md#invocation); declare `support`, `snapshot`,
`devices`. Optional `timer=0..2` selects a hardware timer. State input without
target/EXE is offline. Inspect `timers.json` (`psx.runtime-timers/v1`).

Reports retain native mode/target/rate, IRQ/gate state, exact cycles and separate
raw mirrors. Decoding covers clocks, synchronization, target reset, IRQ selection/
repeat/toggle and reached flags. The fourth counter is internal, not a PS1
hardware timer; its state and next scheduled counter remain explicit.

## Application

`count_before_update` uses pinned `readCounterInternal` uint64 subtraction/
division and 16-bit result. It is not a port read: native reads first update
counters and may apply jitter; mode reads clear reached/overflow flags. Inspection
does neither and cannot predict a later load. Mirrors never replace native state.
Invalid sizes, counter counts, divisors and integer encodings reject.

Decode follows [devices.lua](../scripts/devices.lua):

| Mode bits | Serialized interpretation |
| --- | --- |
| 0 / 1–2 | Synchronization enable / raw synchronization mode |
| 3 / 4 / 5 | Reset at target / IRQ at target / IRQ at overflow |
| 6 / 7 | Repeat IRQ / toggle IRQ |
| 8, timers 0/1 | Set selects dotclock/Hblank respectively; clear selects system clock |
| 9, timer 2 | Set selects system clock divided by 8; clear selects system clock |
| 10 / 11 / 12 | IRQ requested when clear / reached target / overflow |

Do not infer synchronization transitions from the mode number alone. Validate
native gates, updates and scheduling against pinned
`third_party/pcsx-redux/src/core/psxcounters.h`, `psxcounters.cc` and `sstate.h`,
with [History](history.md) for observed order. Native timing is reference-emulator
evidence, not hardware fidelity.

Provenance only: [PSX timers](https://psx-spx.consoledev.net/timers/).
Routine interpretation uses this contract; external research requires explicit scope.
