# CPU registers

## Purpose

Use [registers.lua](../scripts/registers.lua) to inspect CPU banks or edit GPR/PC
at a stopped context. Follow [Runtime](runtime.md#invocation).

## Procedure

Declare `support`; restoration also requires `snapshot`. Omit edit arguments
to inspect. `register=1..31` plus `value=U32` edits one GPR;
`pc=ADDRESS` edits aligned PC. Register zero rejects edits; other banks are
inspection-only. `savestate=1` retains the result.

## Application

Check `cpu.json` before/after GPR, PC, HI/LO, CP0, CP2D and CP2C values.
Edits change visible storage, not pending-load/delay machinery, so select a
suitable context. They are host changes, not guest execution.
Use [GTE](gte.md) for COP2 edits or native commands.
