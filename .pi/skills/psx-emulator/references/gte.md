# Geometry coprocessor

## Purpose

Use [gte.lua](../scripts/gte.lua) for COP2 inspection, raw edits or native command
execution. Follow [Runtime](runtime.md#invocation); declare `support`, `snapshot`.

## Procedure

| Action | Operands / boundary |
| --- | --- |
| `inspect` (default) | Offline state or live boundary; all 32 data and 32 control registers |
| `write` | Live paused context; `bank=data|control` (data), `register=0..31`, `value=U32` |
| `step` | Supported GTE opcode in RAM, outside a delay slot, Status.CU2 enabled; native execution to PC+4 |

For stepping, establish the original opcode/expected arithmetic independently.
Disabled COP2, non-GTE instructions and reserved commands reject.
Writes/steps accept `savestate=1`.

## Application

Inspect `gte.json` before/after banks, PC, opcode/command, signed MAC0–MAC3 and
FLAG bits 12–31. Banks are exact unsigned words, indices zero-based; interpret
packed halfwords/fixed-point values using the program's conventions.

Raw edits do not apply MTC2/CTC2 sign extension, FIFO or special-register effects;
execute guest instructions for those semantics. Native stepping does not suppress
IRQs, count all retired instructions or establish hardware latency.
