# GPU packets and ordering tables

## Purpose

Use [gpu.lua](../scripts/gpu.lua) to decode GP0/GP1 bytes, traverse immutable RAM
ordering tables, or capture native ingress with surrounding VRAM/display state.
Follow [Runtime](runtime.md); use [VRAM](vram.md) for individual images.

## Procedure

Declare `support`, `packets`; set `identity=LABEL` (1–160 bytes).
The receipt identifies actual inputs/tools.

| Action | Inputs and arguments |
| --- | --- |
| `decode` (default) | `--input words=PATH`: complete little-endian 32-bit stream; `port=gp0|gp1` (gp0) |
| `chain` | `--input ram=PATH`: exact physical 2 MiB image; aligned `address=U32`; GP0 only |
| `capture` | State with matching origin receipt, or EXE/target; GPU-capable native [History](history.md) required |

`words=1..1048576` (65536) bounds stream words; `packets=1..65536` (4096) bounds
packets and chain nodes. Exceeded bounds fail, never truncate success. Use finite
timeout/fresh output. Offline decode/chain need no execution context or guest writes.

Chain starts accept physical RAM and cached/uncached aliases/mirrors within 8 MiB.
Nodes retain original/physical addresses, header, next pointer, payload length and
stream offset. Normalize with the 2 MiB mask; repeated physical nodes reject as
cycles. Next-pointer bit 23 terminates. Unaligned pointers, unsupported aliases,
cross-RAM payloads and exceeded bounds reject; expanded RAM/MSAN are unsupported.
Packets may cross node boundaries.

`gpu.bin` preserves exact command bytes. `gpu.json` (`psx.runtime-gpu-packets/v1`)
retains limits, raw words/byte offsets, decoded fields and chain nodes. Decode
errors retain raw bytes/failed receipt without completion. Traversal failures
precede stream publication: inspect original RAM and diagnostics.

Capture additionally declares `video`, `ingress`, `events`, `snapshot`, `display`
and `difference`. It probes GPU context support on an empty recorder and fails
if unavailable. Complete host edits before recording. Bounds/defaults:
`frames=1..36000` (1), `seconds=1..300` (20), `events=1..65536` (16384),
`bytes=0..8388608` (1048576), comparison `limit=1..65536` (1024), plus words/packets
above. Optional aligned `stop=ADDRESS` must arrive before frame exhaustion.
The event budget includes shared DMA/IRQ/timer records, not just GPU commands.
Wall bounds include target acquisition; allow export time before harness timeout.
Offline inputs, `port` and `address` reject during capture.

Capture freezes outside Pause/device callbacks before exporting History's raw
events/payload. `ingress.json` correlates buffers, visited headers, terminals,
GP0/DMA reads and decoded packets; `commands.bin` holds concatenated GP0 bytes only after
successful decoding. Initial/final `.pbuf`, `-vram.bin`, `-screen.bin` and
`-screen.json` retain boundary observations. `effects.json` compares VRAM/display;
`video.json` reports bounds/outcome. Missing context, partial boundary packets,
lost associations, resets interrupting packets, overflow or export/comparison
failure prevent completion. Raw History evidence survives decoding failure.

## Application

GP0: zero-word NOP, cache clear, fill, polygons, lines/polylines, rectangles,
transfers, drawing environment. GP1: reset/FIFO/IRQ, display enable/start/ranges/
mode, DMA direction, query selection. Unknown commands, incomplete packets, early
polyline terminators and zero-area uploads reject. Terminator patterns in a
polyline command, first coordinate or shaded coordinate slot reject because
malformed-input behavior depends on native buffer boundaries.

Interpret encoded operands against pinned `src/core/gpu.cc`/`gpu.h`: signed
11-bit or unsigned positions, raw 16-bit dimensions, texture page/CLUT and native
depth-3-to-16-bit mapping. Colors are encoded, not modulated. Draw offset, clipping,
mask, texture window and prior environment are unapplied; raw words remain authoritative.

Offline streams must begin at packet boundaries and do not model GP0/GP1
interleaving. RAM snapshots cannot prove lists remain unchanged during execution.
A FIFO-clear label does not prove the soft renderer resets its parser.

Capture decodes in packet-completion order; exact source sequences retain GP1
interleaving and split CPU/DMA buffers. Cycles delimit observed buffers, not
individual bus words or rasterization. The parser must be ready at both boundaries;
no reset is injected to manufacture readiness. Environment availability remains
explicit. Structural decoding does not apply draw state or replay rendering.
Whole-interval effects do not prove per-command causality; unavailable displays
remain unavailable even when their byte storage compares equal.
CPU GP1 status reads are not recorded; GP1 command/query writes and returned GP0
data are. Do not infer a complete GPU bus trace.

The upstream [GPU logger](https://pcsx-redux.consoledev.net/Debugging/gpu-logger/)
clears its GUI log between frames; it is not this action's capture source.
Validate native hooks and representative effects under [Native](native.md).
Never infer hardware equivalence from decoded packets or matching images.
