# Repeated continuation

## Purpose

Use [replay.lua](../scripts/replay.lua) to compare repeated state continuations
under bounded controller schedules. Follow [Runtime](runtime.md) and
[comparison semantics](states.md#structured-comparison). This measures selected
observations' repeatability, not complete determinism or hardware accuracy.

## Procedure

Supply successful `--origin`, matching BIOS/disc, `--input state=PATH` and
`--input schedule=PATH`; omit EXE. Declare `support`, `snapshot`, `difference`,
`timeline`. Set `runs=2..8` (2), comparison `depth=0..64` (16), per-checkpoint
`limit=1..65536` (1024), finite timeout and fresh output. Inspect receipt and
`replay.json`; a successful run can report `equal:false`.

<a id="schedule-format"></a>

Schedules are data-only UTF-8/ASCII, with CRLF/blank lines allowed:

```text
psx.schedule/v1
0 sample registers
1 buttons 1 START
2 sample registers
2 buttons 1 -
3 stop
```

| Command | Meaning |
| --- | --- |
| `FRAME sample PATH` | Compare subtree; `.` selects the whole state |
| `FRAME screen` | Compare display metadata/pixels; declare `display` |
| `FRAME buttons SLOT NAMES` | Replace held buttons on slot 1/2; comma-separated native names or `-` to release |
| `FRAME stop` | Required final command; stop at frame 1–36000 |
| `FRAME stop CONDITION` | Poll after earlier commands; frame is deadline; declare `condition` |

Frames count GPU Vsyncs after restoration; frame 0 runs paused. Use nondecreasing
frames 0–36000; same-frame commands retain order. Limits: 512 commands, four
state/display checkpoints, 64 KiB schedule. Unknown commands/buttons, duplicates,
missing checkpoints/stop, invalid paths, incompatible boundaries and post-stop
commands reject.

Conditions: `pc VALUE`, `register INDEX VALUE` (GPR 0–31), or `memory OFFSET HEX`
(physical 2 MiB RAM, 1–64 byte pairs). Integers accept decimal/hex; PC must align;
Lua expressions reject. Poll each paused Vsync after earlier commands, from frame
1 onward. Matches retain observation/frame/cycles; unmet deadlines fail with
`condition.json`. Intermediate instructions/MMIO are unobserved.

`--input alternate=PATH` replaces the primary schedule for runs 2 onward.
Checkpoint frame/type/path sequence and stop deadline must match; buttons/
conditions may differ. Otherwise all runs use the primary schedule. Overrides
clear between runs and after final stop; physical input remains active.

## Application

`psx.runtime-replay/v1` compares exact start/stop/checkpoint cycles, selected state
and observed buttons, even unpolled buttons. Any difference sets `equal:false`;
the first divergent boundary retains paths and stop reasons.

| Capture | Meaning |
| --- | --- |
| `baseline-NNN.pbuf` | Baseline full state |
| `baseline-NNN.bin` | Baseline display bytes; metadata in report |
| `divergence.pbuf` | First differing state |
| `divergence.bin`, `divergence.json` | First display difference |
| `state.proto` | Running-build schema |

Captures cap at 120 MiB within the harness's 128 MiB total. Receipts bind state,
schedules, scripts, tools and media. Observations follow paused Vsync dispatch,
not instruction/audio-sample boundaries; [Runtime](runtime.md#evidence-contract)
defines unpinned settings and host effects.

Equal subtrees prove neither whole-state nor execution equality. Hidden device
state, host audio queues and PCM are unexamined. `screen` compares storage/ranges
and dimensions/format/availability separately; equal unavailable displays remain
`image_comparison_available:false`, not image evidence.

Preserve deferred Vsync boundaries. UI polling discards nested nextTick callbacks;
start the next restoration directly when already outside dispatch.
