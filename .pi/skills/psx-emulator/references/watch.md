# Debugger observations

## Purpose

Use [watch.lua](../scripts/watch.lua) for bounded selected Exec/Read/Write hits.

## Procedure

Follow [Runtime](runtime.md#invocation). Declare `support`, plus `snapshot`
for restoration, and select target/EXE/state context.
Set `address=U32`, `width=1..65536` (1), `kind=Exec|Read|Write` (Write),
and `hits=1..4096` (1). The action stops at that count and emits `watch.json`,
RAM/registers and optional `savestate=1`. The harness timeout bounds unreached hits.

## Application

Verify debugger address/alias mapping and receipt. Events retain actual address,
width, cause and registers **before** the selected instruction/access. They do
not prove retirement or capture completed write values; a range watch is not a
complete DMA/device/bus trace.

Callbacks are retained/protected; false removes a hook, true keeps it. Capturing
pauses explicitly. Interpret the stated observation boundary before comparison.
