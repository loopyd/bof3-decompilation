# Native device histories

Contents: [purpose](#purpose), [capture procedure/API](#procedure),
[interpretation and validation](#application).

## Purpose

Capture bounded native DMA, CPU-visible IRQ and timer activity with
[history.lua](../scripts/history.lua). For serialized inspection, use [DMA](dma.md),
[IRQ](irq.md) or [Timers](timers.md). Follow [Runtime](runtime.md) and verify
[native bindings](native.md).

## Procedure

Declare `support`, `snapshot`, `events`; supply a state with matching origin
receipt, or executable/target. Required `identity=LABEL` is 1–160 bytes; the
receipt identifies actual inputs/tools. Use the [common invocation](runtime.md#invocation).

| Argument | Bounds / default |
| --- | --- |
| `frames` | Subsequent GPU Vsync events, 1–36000 / 1 |
| `stop` | Optional aligned PC; frame exhaustion before arrival fails |
| `seconds` | Target acquisition plus recording, 1–300 / 20 |
| `events` | Native records, 1–65536 / 16384; includes initial state |
| `bytes` | Payload budget, 0–8388608 / 1048576 |

Leave time for paused export before harness timeout. Unexpected pauses, overflow
and timeouts fail. Finish setup edits before recording; maintained helpers reject
active capture. Audit custom Lua: arbitrary pointer writes are not intercepted.

Inspect `history.json` for outcome; `events.json` for completeness, failure bits,
counts and dropped attempts; `events.ndjson` for raw/named fields; `payload.bin`
for referenced bytes. `initial.pbuf`/`final.pbuf` retain context. Failed runs keep
available files/logs without a success marker.

For custom Lua, use `PCSX.History`:

| Operation | Contract |
| --- | --- |
| `begin(events, bytes)` | Empty recorder, paused owner-thread interpreter, MSAN disabled |
| `stop()` | Pause first; freeze active recording without clearing failures |
| `status()` | Owner-thread copy of version, state, cycles, counts and failures |
| `record(index)` | Paused frozen capture; zero-based index; owning cdata copy |
| `bytes(offset, length)` | Paused frozen capture; bounded copied binary string |
| `clear()` | Explicit paused release of inactive evidence before another begin |

Defer lifecycle/drain outside Pause/device callbacks. States: empty=0, recording=1,
frozen=2. Keep uint64 sequence/cycle/request/related values as cdata or decimal
strings, never doubles. Failure bits: events=1, bytes=2, thread=4, unsupported=8,
association=16, discontinuity=32, bounds=64, clock regression=128. Failures are
sticky; prefix/first failure survive, dropped attempts saturate, buffers never wrap.

## Application

Distinguish starts, copies, schedules, dispatches, busy/DICR changes and IRQ
delivery. Complete intervals may end before DMA completion. Request zero means
absent or pre-capture ownership; use initial device/scheduler state to distinguish.
Deferred work, decoder input, partial buffers, channels and callbacks may have
different owners. Completion separates callback owner from cleared channel.
Never assign new starts to pre-existing work.

| Observation | Interpretation |
| --- | --- |
| Register attempt/result | Guest address, width/value and aligned raw mirror; mirrors are not live timer return values |
| MDEC input | Consumption-time compressed/table bytes; peeks are not consumption |
| MDEC output | Produced spans and partial-buffer provenance; a queued request is not a write |
| GPU DMA | Block ingress/egress and actually visited linked-list headers/payloads; sizing and MADR-update walks are not transfers |
| CD DMA | Actual byte copies, mapped RAM offset and transfer-buffer index/wrap; sector identity needs CD lifecycle evidence |
| SPU DMA | Existing copied halfwords with RAM/SPU addresses and wrap; completion is separately scheduled |
| OTC | Descending stores plus the final terminator overwrite; count every actual store |
| IRQ | Requested mask and latch before/after, including unchanged levels; device deassertion is not necessarily a guest I_STAT store |
| Timer | Pre/post target or overflow state, writes, actual reads/read-clear effects, clock rates and Hblank/Vblank gates; counter 3 is internal |

Payload offsets/counts are bytes; only `dma-data` counts toward transfer totals,
excluding GPU headers, MDEC buffers and peeks. Preserve original/normalized
addresses and byte/halfword dispatcher semantics. Same-cycle order is hook order;
bulk operations lack per-byte timing. Separate scheduled targets from dispatch cycles.

GPU-capable bindings emit `gpu-context`/`gpu-environment` at begin/end and
`gpu-input`/`gpu-result` around each native buffer. Match exact buffer IDs and
DMA request IDs; enclosed `dma-data` owns payload bytes. CPU writes retain their
operand; read results occur only at `gpu-result`. Parser readiness is not full
partial-packet state; processor IDs may identify shared parser families. Interpret
last-page/window/offset only when the context's known-environment bit is set.
Host GPU calls/replay and GUI VRAM edits invalidate capture. DMA count overflow
or a read FIFO larger than its request also fails; native execution is not repaired.
Require these records explicitly for GPU missions; their absence cannot prove an
idle parser or complete command history.

The recorder observes CPU-thread boundaries. SPU mixer requests coalesce atomically
before CPU-visible IRQ delivery, losing count, DSP position and order. Reuse copied
SPU values without rereading concurrent RAM or widening its conditional mixer
lock. Hooks must not call Lua, perform file I/O or allocate unbounded memory;
unexpected threads fail before reading unsynchronized CPU state.

PIO DMA, MSAN and unsupported paths reject; reset/restore are discontinuities.
Cyclic lists, bounds failures and lost associations prevent completeness. Compare
raw records with independent fixture bytes/native state, not merely exporter labels.
Capture establishes emulator observations, not hardware fidelity, PCM or streaming.

Under `third_party/pcsx-redux/src/core/`, `history.*` and `journal.h` own
storage/API; `psxhw.*` register dispatch; `psxmem.h` DMA/IRQ
transitions; `r3000a.*` scheduling and CPU acceptance; `psxcounters.*` timers;
`mdec.cc`, `gpu.cc`, `cdrom.cc`, `psxdma.cc` and `../spu/dma.cc` payloads.
Before publishing hook extensions, independently validate payload/count/order,
deferred/pre-capture ownership, replaced callbacks, timer/IRQ gates, malformed
paths, lifecycle guards and bounded failures.
