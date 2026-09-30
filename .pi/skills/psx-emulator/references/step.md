# Native debugger stepping

## Purpose

Use [step.lua](../scripts/step.lua) for bounded native step-into pause transitions.
Follow [Runtime](runtime.md#invocation) and [native limits](native.md); over/out
are unsupported.

## Procedure

Declare `support`, `snapshot`, `execution`; choose target, EXE entry or state.
Required `identity` labels executable/overlay context; the receipt identifies bytes.

`steps=1..1024` (1) bounds transitions; `seconds=1..300` (20) bounds acquisition
plus stepping. Use a longer harness timeout for the cooperative UI timer.
Optional aligned `stop=ADDRESS` succeeds before that instruction; exhausting
bounds first fails. Without stop, reaching the count succeeds.

`exceptions=stop|continue` defaults to stop on native exception pause or changed
ISR/EPC/Cause; an exception before the requested address fails. CP0 writes can
also change context, so this is not independent exception decoding.
`savestate=1` retains final state, including handled failure.

## Application

`steps.ndjson` flushes initial/transition PC, exact cycles, GPR/HI/LO, CP0, deltas,
pending loads/branches, ISR, Cause/EPC/Status/BadVAddr and nearby words. BadVAddr
may be stale; `next_is_delay_slot` describes the next instruction; absent delay
submessages use protobuf defaults. Nearby RAM/ROM/scratch reads have no side
effects but may differ from instruction cache. `last_executed_word` is serialized
CPU state; MMIO/unsupported aliases are not dereferenced.

`step.json` (`psx.runtime-step/v1`) reports reason, success, bounds, final context
and [coverage](trace.md). Journal overflow beyond 16 MiB fails. Handled errors
retain reports/failing receipts; timeouts or aborts may leave only flushed records/
logs. Missing completion is never success.

Exceptions or breakpoints can interrupt a request; IRQs can cross instructions.
One row/request does not imply one retired instruction. Apply the
[native dispatch and exception limits](native.md).
