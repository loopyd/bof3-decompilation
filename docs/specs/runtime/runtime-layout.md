---
type: Runtime model
title: Runtime layout
description: BOF3 executable, overlay, and load-region boundaries.
tags: [runtime, overlays, psx]
---

# Runtime layout

## Boot chain

```text
SYSTEM.CNF -> SLUS_004.22 -> LOGO.EXE / EMI entries / STR-XA media
```

`SLUS_004.22` owns disc access, slot resolution, EMI parsing, payload streaming,
and shared runtime services. `LOGO.EXE` is a separate PS-X executable. EMI
archives contain independently loaded code and data entries.

## Identity

An executable target is identified by:

```text
archive path + entry slot + payload hash + load address
```

Identical payload bytes loaded at different addresses remain separate targets.
An EMI archive is never passed directly to Splat or the matcher; its extracted
entry is.

## Reused regions

| Region | Observed role |
| --- | --- |
| `0x801d0c00` | shared frontend, game-mode, and battle overlay base |
| `0x801eec00` | battle/effect overlay region |
| `0x8003b800` | character/effect work region |
| `0x80104000` | world/area code-data region |
| `0x801f2c00` | compact world/area work region |
| `0x80033xxx`–`0x8003axxx` | small graphics or palette buffers |

The region does not identify the subsystem; the currently loaded target does.

## Runtime load path

1. Resolve a logical slot to a disc LBA.
2. Read and validate the EMI header.
3. Compute the aligned entry offset.
4. Dispatch by entry type.
5. Stream into RAM, graphics, or audio state.
6. Transfer control separately when the payload is executable.
