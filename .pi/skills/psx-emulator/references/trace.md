# Execution observations and coverage

## Purpose

Use [stepping](step.md) for pause contexts, [watch](watch.md) for selected
Exec/Read/Write ranges, or [call](call.md) for host-directed invocation and a
selected-PC trace. These answer different questions; absence never proves unused code.

## Procedure

Optionally stage `--input symbols=PATH` for step entry counts. Each ASCII line is
an aligned U32 address and qualified name:

```text
0x80010000 program/main
0x80010040 program/overlay/helper
```

Names use letters, digits, `_ . : / @ -`, maximum 160 bytes. Limits: 256 unique
addresses/names, 32 KiB. Empty/omitted input gives unnamed address counts.
Labels do not edit symbols or prove function entries. Virtual aliases remain
distinct; overlays/identities are not inferred from numeric PCs.

## Application

`step.json` counts starting-boundary and labelled-entry hits, and classifies
transfer words between pauses: JAL/JALR=call, J/JR=jump, ordinary conditional
branches=branch, BLTZAL/BGEZAL=conditional link. Encodings alone do not prove
taken calls; unsupported REGIMM forms remain explicit.

Records retain the last executed word and both boundary PCs. IRQ intervals can
contain unseen instructions, so the starting PC need not identify that word.
Delay-slot state separates the transfer instruction from its eventual destination.
No guessed returns, synthetic call stack or automatic tail-call classification
is used. Recursion appears as repeated bounded entry hits.

Counts include interrupted requests and cover observations only, not complete
instruction, branch-outcome or call coverage. Hand off the full journal, receipt,
qualified context and termination reason; traces grant no pruning authority.
