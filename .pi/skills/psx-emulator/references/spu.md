# SPU voices and DSP state

## Purpose

Use [spu.lua](../scripts/spu.lua) to inspect voices/DSP, export RAM, write ports or
sample Vsync state. Follow [Runtime](runtime.md#invocation) with `support`, `snapshot`;
`action=capture` uses [Audio](audio.md) and its binding requirements.

## Procedure

Inspect/export offline state; write/trace a live paused context. `voice=1..24`
selects one, default 0 all. Labels are one-based, unlike hardware indices.

| Action | Operands / evidence |
| --- | --- |
| `inspect` (default) | Control/status/IRQ/noise, port mirror, sample/loop positions, volumes, ADPCM history and ADSR internals |
| `export` | `offset=0..524288`, `length` (remaining); selected 512 KiB RAM bytes in `spu.bin` |
| `write` | Aligned `address=0x1f801c00..0x1f801dfe`, `value=0..65535`, `scratch`; declare `bus` |
| `trace` | `frames=1..120` (1); initial state plus one voice/DSP observation per Vsync |

Writes follow [BUS](bus.md), retaining halfword-store effects, before/after state
and cycles. Writes/traces accept `savestate=1`; read-only actions reject it.
Check `spu.json`, bytes and receipt.

## Application

Compare voice positions, pitch and ADSR. Writes key voices on/off or alter
DMA/IRQ/DSP; the 256 serialized port mirrors are not MMIO reads. Dynamic
envelopes/positions come from native channels.

Vsync traces miss intervening transitions; they are not register/DSP-tick/PCM
histories. Serialization can affect scheduling. Reverb coverage is limited to
schema fields; inspect/write control ports explicitly.

Query `spu.xa` via [States](states.md) for XA decoder/buffer metadata. Host preview
or substitute synthesis is not SPU evidence; use [PCM capture](audio.md).
Hardware fidelity requires independent comparison.
