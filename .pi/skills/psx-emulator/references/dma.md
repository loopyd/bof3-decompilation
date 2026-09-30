# DMA state

## Purpose

Use [dma.lua](../scripts/dma.lua) for serialized DMA configuration; use
[History](history.md) for actual transfer/completion observations.

## Procedure

Follow [Runtime](runtime.md#invocation); declare `support`, `snapshot`,
`devices`. State input without target/EXE is inspected offline; other contexts
pause at the common boundary. Optional `channel=0..6` selects one, otherwise all.

Inspect `dma.json` (`psx.runtime-dma/v1`): MADR/BCR/CHCR, DPCR/DICR,
direction, alignment, synchronization, chopping, priority, enable/busy/trigger
and IRQ fields. Manual zero word count is labelled 65536; request count is the
raw block-size product with zero fields flagged. Linked-list/reserved modes
have no derived length.

## Application

The source is the serialized 64 KiB hardware region. Configured counts are not
remaining work, transferred bytes or completion. Stored DICR master state and
the request derived from flags remain separate. No RAM is dereferenced or aliases
folded. This action writes nothing; use [MMIO probes](bus.md) for effects.

Decode follows [devices.lua](../scripts/devices.lua):

| Field | Serialized interpretation |
| --- | --- |
| Channels 0–6 | MDEC input, MDEC output, GPU, CD-ROM, SPU, PIO, OTC |
| CHCR bit 0 / bit 1 | From RAM when bit 0 set, otherwise to RAM; stride -4 when bit 1 set, otherwise +4 |
| CHCR bits 9–10 | 0 manual, 1 request, 2 linked-list, 3 reserved |
| CHCR bits 8 / 24 / 28 | Chopping / busy / trigger; configuration only |
| DPCR per-channel nibble | Low three bits priority, high bit enable |
| DICR bits 16–22 / 24–30 | Channel IRQ enables / flags |
| DICR bits 15 / 23 / 31 | Force IRQ / master enable / stored master flag |

`requested_by_flags` is force IRQ, or master enable with any enabled channel flag.
Keep it separate from stored master flag and observed IRQ delivery. For native
transfer semantics inspect pinned `third_party/pcsx-redux/src/core/psxhw.cc`,
`psxmem.h` and `psxdma.cc`; decoded configuration alone is insufficient.

Provenance only: [PSX DMA registers](https://psx-spx.consoledev.net/dmachannels/).
Routine interpretation uses this contract; external research requires explicit scope.
