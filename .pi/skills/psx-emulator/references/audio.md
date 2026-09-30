# Mixed PCM and SPU access capture

## Purpose

Capture bounded native mixed PCM and SPU accesses with [spu.lua](../scripts/spu.lua),
`action=capture`. Follow [Runtime](runtime.md); verify `PCSX.Audio` through
[Native](native.md). Missing bindings fail; captures do not establish hardware fidelity.

## Procedure

Declare `support`, `snapshot`, `sound`, `pcm`, `wave` from `scripts/`. Supply state
and matching origin receipt, or executable/target. Set `action=capture` and
`identity=LABEL` (1–160 bytes); the receipt identifies input/tool hashes.

| Argument | Meaning / bounds / default |
| --- | --- |
| `frames` | GPU Vsync bound before the primary stop; 1–36000 / 60 |
| `stop` | Optional aligned PC; frame exhaustion before arrival fails |
| `tail` | Additional guest Vsyncs after primary stop; 0–3600 / 0 |
| `seconds` | Target acquisition, primary interval and tail wall limit; 1–300 / 20 |
| `samples` | Storage capacity in stereo PCM frames; 1–2646000 / 441000 |
| `blocks` | Callback record capacity; 1–65536 / 16384 |
| `events` | SPU event capacity; 1–65536 / 16384 |

Other SPU action arguments reject. Leave export time before harness timeout.
Capacity overflow fails; it is not a duration stop. Bound host callbacks and GPU
Vsyncs according to their different timing.

Save initial state before begin, final state after freeze. Never save, restore,
reset or edit during capture. Maintained helpers reject active history/audio;
arbitrary Lua writes are not intercepted. Audit scripts and avoid GUI/settings
changes. Existing queues and partial mixer work are retained, not flushed.

CPU pause precedes deferred freeze; audio may append between them. `tail`
continues guest execution without key-off, DSP synchronization or guaranteed
release completion. Unexpected pause, timeout, loss, overflow or export errors
fail without a success marker. Inspect retained files/logs; hard timeout can
interrupt export.

| Capture | Content |
| --- | --- |
| `audio.f32`, `audio.wav` | Identical interleaved stereo float32 little-endian payload; WAV uses IEEE-float format and a sample-count `fact` chunk |
| `blocks.ndjson` | Callback index, frame offset/count, dequeued voice/disc counts, flags and observed CPU publication |
| `spu.ndjson` | Native named access/feed records, exact cycles, observed frame cursor and parent identity |
| `audio.json` | Native counts/limits/settings/queues, failure/drop data, export checks and retained PCM duration |
| `sound.json` | Mission bounds, primary/tail observations, stop reason and outcome |
| `initial.pbuf`, `final.pbuf` | Machine contexts surrounding capture; neither contains the complete host audio queues |

For custom missions, begin on a paused owner-thread interpreter. Paused-owner
stop/export/clear can recover failed evidence after backend changes; `status`
requires owner thread, not pause.

| Operation | Contract |
| --- | --- |
| `begin(samples, blocks, events)` | Paused interpreter, empty recorder, initialized stream, positive capacities, no MSAN |
| `stop()` | Freeze active recorder, retaining failures; defer outside device/Pause callbacks |
| `status()` | Owner-thread snapshot; only frozen state has a terminal end cycle |
| `block(index)`, `event(index)` | Frozen capture, zero-based index, owning named table |
| `samples(offset, frames)` | Frozen capture, at most 65536 stereo frames per call, copied binary string |
| `clear()` | Release inactive capture explicitly before another begin |

States: empty=0, active=1, frozen=2. Keep uint64 fields as canonical decimal strings,
never doubles. Failure bits: frames=1, blocks=2, events=4, thread=8, bounds=16,
clock=32, discontinuity=64, streaming-loss=128, output=256. Failures are sticky,
drop counters saturate, buffers never overwrite.

## Application

Capture stage: **44100 Hz stereo float32 after mixer/mute/mono, before SDL
conversion**. Voice/reverb includes host Volume scaling and signed16 clipping;
XA/CDDA uses native resampling/scaling before callback summing/mute/mono. Preserve
values beyond ±1 without clipping/normalizing. SDL/device resampling, gain and
physical playback are excluded. Retain settings and active/configured driver/device.

Block offsets count stereo frames. Flags: mono=1, mute=2, successful SDL
submission=4. Short dequeues zero-fill; empty disc streams do not imply loss.
`zero_fill` totals cover validated block prefixes: check completeness. Enqueue/
submission losses set failure bits; failed prefixes may retain useful PCM.

| Event kind | Native meaning |
| --- | --- |
| 1 | Bus write attempt |
| 2 | Effective write-handler observation |
| 3 | XA/CDDA feed: source rate/count/layout, output count and enqueue acceptance |
| 4 | Bus read attempt |
| 5 | Effective read-handler result |
| 6 | Bus returned value |

Non-feed kinds include address/value/width. Parent IDs connect dispatcher/handler;
zero means no recorded parent, not necessarily guest root. Words may contain
halfword children. Preserve aliases, widths and returns; labels imply no extra accesses.

Block cycles use potentially stale CPU publications; event frame cursors mark
output observed by CPU. Neither locates audible effects or per-sample CPU cycles.
Queue snapshots expose some prehistory/remaining work, not exact latency; partial
batches and host scheduling remain uncontrolled. Freeze proves neither queue drain,
sound end, complete CD lifecycle nor determinism. Independently validate signals,
register sequences and failures before semantic acceptance.
