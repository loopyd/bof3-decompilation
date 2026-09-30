# Function calls

## Purpose

Use [call.lua](../scripts/call.lua) for an explicit host-directed function call and
optional selected-PC observations. Follow [Runtime](runtime.md#invocation).

## Procedure

Declare `support` and choose target, EXE entry or restored state (`snapshot`
required). Set aligned `function`; `return` defaults to `0x80010000` and
must differ from function/context entry. `a0`–`a3` default to zero.

The action sets PC, RA and A0–A3, preserving other state, then captures before the
return instruction. Optional `trace=ADDRESS` records registers/cycles on visits;
`trace_limit=1..65536` defaults to 4096. Use finite process bounds and
`savestate=1` to retain the result.

## Application

Verify receipt, capture boundary and `trace.json` (`psx.runtime-trace/v1`).
Overflow fails without silent truncation. A host-directed call is not natural
startup or ABI validity proof. Selected-PC observations are not a retired stream;
interpret recursion and exceptions explicitly.
