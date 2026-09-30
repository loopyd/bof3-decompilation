---
name: psx-emulator
description: Operate, inspect and debug PS1 programs and devices through bounded PCSX-Redux Lua missions with reproducible captures.
---

# PSX emulator

Use maintained Lua actions through [runtime.sh](scripts/runtime.sh). Harness owns
bounds, cleanup and receipts; requesting project owns semantic acceptance.
Every mission reads [runtime contract](references/runtime.md), then selected
reference below. Load only needed branches; [catalog](references/catalog.md)
retains layer navigation.

## Select an action

| Mission | Read before work |
| --- | --- |
| Startup verification | [Startup](references/startup.md) |
| Stop, capture or restore | [Target](references/target.md) |
| CPU registers | [CPU](references/cpu.md) |
| Physical memory | [Memory](references/memory.md) |
| Breakpoints/access observations | [Watch](references/watch.md) |
| Stepping/exception context | [Step](references/step.md) |
| Execution coverage | [Trace](references/trace.md) |
| Saved-state queries/export/comparison | [States](references/states.md) |
| DMA configuration | [DMA](references/dma.md) |
| Interrupt latches/masks | [IRQ](references/irq.md) |
| Timer state | [Timers](references/timers.md) |
| Native DMA/IRQ/timer capture | [History](references/history.md) |
| Bus observations/MMIO probes | [Bus](references/bus.md) |
| Function invocation/selected-PC trace | [Call](references/call.md) |
| Frame advance/controller input/display | [Frames](references/frames.md) |
| Repeated continuation/scheduled input | [Replay](references/replay.md) |
| VRAM/textures/palettes | [VRAM](references/vram.md) |
| GPU packets/ordering tables/capture | [GPU](references/gpu.md) |
| Geometry coprocessor | [GTE](references/gte.md) |
| Sound voices/DSP state | [SPU](references/spu.md) |
| Native mixed PCM/SPU access capture | [Audio](references/audio.md) |
| Disc media/controller/journal capture | [CD-ROM](references/cdrom.md) |
| Extend Lua action | [API contract](references/api.md) |
| Native customization | [Native authorization and validation](references/native.md) |

Check prerequisites and preserve dirty work. Select an existing action before
writing another runner or state parser. Define inputs, observations and a stop
condition; stage modules/files explicitly and use fresh bounded output. Lua is
trusted host code, not a sandbox. Keep proprietary inputs and captures local.

Verify receipts before interpretation. Distinguish native guest execution, host
edits and serialized state; success alone proves neither behavior nor fidelity.
Route static analysis to `psx-rizin`, and source promotion to the requesting project.
The [agent spec](../../agents/psx-emulator.md) defines delegated scope; the harness
never launches models. Update the owning procedure with capability changes and
validate affected missions; keep this entrypoint broad.
Keep references to purpose, procedure and application; record progress elsewhere.

Return tool calls, wall time and method with mission result; parent archives
measurements and supplies relevant prior evidence. Converge reusable findings into
skill references. User documentation and observation ledgers are not runtime inputs.
