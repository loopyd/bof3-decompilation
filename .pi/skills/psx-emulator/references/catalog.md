# PSX emulator action catalog

Optional layer navigation for `$psx-emulator`. [Entrypoint](../SKILL.md#select-an-action)
links each action directly; use this grouping only when selecting a layer.

Every mission also reads [Runtime](runtime.md). Selector: `TARGET@0xADDRESS`.

## 1. Runtime and bounds

Mission lifecycle: inputs, invocation, bounds and receipts; startup verification; stop,
capture and restore.

| Action | Reference |
| --- | --- |
| Every mission: inputs, invocation, bounds, receipts | [Runtime](runtime.md) |
| Startup check | [Startup](startup.md) |
| Stop, capture or restore | [Target](target.md) |

## 2. Observe state

Read the machine's state without driving it.

| Action | Reference |
| --- | --- |
| CPU registers | [CPU](cpu.md) |
| Physical memory | [Memory](memory.md) |
| Breakpoints and access observations | [Watch](watch.md) |
| Bounded stepping and exception context | [Step](step.md) |
| Execution coverage and qualified hit counts | [Trace](trace.md) |
| Saved-state queries, export and structured comparison | [States](states.md) |
| DMA channels | [DMA](dma.md) |
| Interrupt latches and masks | [IRQ](irq.md) |
| Timers and counter state | [Timers](timers.md) |
| Native DMA, IRQ and timer capture | [History](history.md) |
| Bus and device observations (detail page, no action of its own) | [Bus](bus.md) |

## 3. Drive execution

Advance, invoke and repeat guest execution under a bounded input schedule.

| Action | Reference |
| --- | --- |
| Function invocation and selected-PC tracing | [Call](call.md) |
| Frame advance, controller input and display | [Frames](frames.md) |
| Repeated continuation and scheduled input | [Replay](replay.md) |

## 4. Capture devices

Bounded capture of device state and output.

| Action | Reference |
| --- | --- |
| VRAM, textures and palettes | [VRAM](vram.md) |
| GPU packets, ordering tables and bounded capture | [GPU](gpu.md) |
| Geometry coprocessor | [GTE](gte.md) |
| Sound voices and DSP state | [SPU](spu.md) |
| Native mixed PCM and SPU access capture | [Audio](audio.md) |
| Disc media, CD controller and journal capture | [CD-ROM](cdrom.md) |

## 5. Extend an action

Only when extending an action: primary API research, and native customization which
requires specific source authorization and independent semantic review.

| Action | Reference |
| --- | --- |
| Only when extending an action | [API research](api.md) |
| Native customization (detail page, no action of its own) | [Native](native.md) |
