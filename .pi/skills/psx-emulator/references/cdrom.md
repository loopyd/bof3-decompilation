# CD-ROM media and controller

Contents: [purpose](#purpose), [procedure and bounds](#procedure),
[interpretation and evidence limits](#application).

## Purpose

Use [cdrom.lua](../scripts/cdrom.lua) for serialized controller inspection,
read-only media access, a bounded native command or passive journal capture. Follow
[Runtime](runtime.md#invocation); declare `support`, `snapshot`.

## Procedure

Mount `--disc image.cue` with quoted `FILE "name" BINARY` entries.
The harness hashes CUE/tracks, preserves track/index order and stages isolated
track symlinks. Descriptor, target and directory identities are rechecked.
Limits: 1 MiB CUE, 99 FILE entries, 1 GiB/track, 2 GiB total. Unquoted/non-BINARY
files and external CD-TEXT reject; PPF/SBI/SUB sidecars are not implicitly copied.

| Action | Arguments / captures |
| --- | --- |
| `inspect` (default) | Offline state or live boundary; bounded controller/FIFO/sector/XA/attenuation fields |
| `directory` | ISO `path` (root); up to 4096 immediate entries with name/LBA/size/type |
| `file` | Exact ISO `path`, optional zero-based `offset`, `length` (remainder); `file.bin`, maximum 8 MiB |
| `sectors` | Zero-based `lba` (0), `sectors=1..1024`, `mode`; exact-size `sectors.bin` |
| `command` | `command=0..255`, `params` (up to 8 hex bytes), `frames=1..120`, `scratch`; declare `bus` |
| `capture` | Validated native CD profile required; `identity`, target/state, frame/PC bounds; declare modules below |

Media reads pause immediately without target/EXE. Directory/file operations check
the primary ISO9660 descriptor; inspect names before guessing versions, e.g.
`--argument 'path=SYSTEM.CNF;1'`. Missing files/short reads reject.

Sector modes: `RAW` (default, 2352 bytes), `M1` (2048), `M2_RAW` (2336
including subheader), `M2_FORM1` (2048), `M2_FORM2` (2324). LBA is relative
to image start, not BCD MSF with the 150-sector lead-in.

For commands, follow [native writes](bus.md) and choose a parked guest context.
Busy controller/pending IRQ rejects. The action clears parameter FIFO, issues
parameter/command byte stores and samples state each Vsync until the first IRQ.
No visible IRQ within the frame bound fails; process timeout also applies.
`savestate=1` retains the resulting state. Other actions reject that option.

For `capture`, follow [native customization](native.md) before using a new build.
Require `PCSX.History.beginCD`; absence fails before capture starts. Declare
`transport`, `transactions`, `events`, `controller`, `scheduler`, `streams`,
`media`, `image`, `mount`, `sectors`, `feeds`, `eligibility`, `delivery`, `transfers`,
`command`, `opcode`, `submission`, `playback`, `decoded`, plus `support`, `snapshot`;
module names match filenames under `scripts/`. Use a target or restored state
with matching origin receipt. Capture observes the running guest and never
issues commands, consumes FIFOs or acknowledges IRQs.

Bounds: `frames=1..36000` (60), optional `stop=PC`, `seconds=1..300` (20),
`events=53..65536` (65536), `bytes=11116..8388608` (8388608),
`mutations=1..65536` (4096), `reads=1..65536` (65536).
Parentheses give defaults. Native preparation requires 5388 bytes plus configured
audio backend/device strings and twice the image descriptor size (at least 2864
bytes each). These minima cover empty boundaries; guest work needs more capacity.
Set the harness timeout above the mission wall bound.
With `stop`, exhausting frames before that PC fails. Unexpected pauses, native
failures and correlation limits fail; retained evidence does not imply success.

`audio=1` joins [PCM capture](audio.md); also declare `pcm` and `wave`.
Its bounds are `samples=1..2646000` (441000), `blocks=1..65536` (16384),
`spuevents=1..65536` (16384). These options require `audio=1`.
Recorders must start empty. Save the initial state while idle, start Audio before
History, then pause and defer outside device callbacks. Freeze History before
Audio and serialize final state only after both stop. No queue drain is implied.

Capture requires profile-2 contexts, predicates and image records; earlier CD
layouts reject. [image.lua](../scripts/image.lua) also provides offline descriptor
validation. Declare `image`, `support`, `events`. Call `decode(row,payload)`
with one CdImage row and its exact bytes. The result retains all 100 allocated
track slots, raw geometry, ordered file/slice ancestry, decimal uint64 sizes and
offsets, and hexadecimal path bytes without assuming valid UTF-8. Unsupported
layouts, invalid parents, excessive nesting and unused file nodes reject.
`track(image,slot)` reads the allocated slot; `tn`, `td`, `length` and `pregap`
reproduce the native helpers' distinct normalization and fallback rules.

Call `bind(image,entries)` with declared receipt entries containing `key`,
`path_hex`, exact `bytes` and `sha256`. It checks root path/size associations and
reports unused entries; it performs no file I/O or hashing. Track `offset` and
each parent slice's `start` remain separate byte offsets. Capture checks matching
begin/end descriptor bytes and their boundary placement. Neither descriptor
equality nor `bind` establishes file-content stability or verified receipts.

Capture obtains these entries through [mount.lua](../scripts/mount.lua) from
the harness's literal environment. Missing values reject even with no disc.
Every native root must match an exact staged absolute path and size; relative or
basename matches reject. Both boundaries retain input keys/hashes and unused
staged tracks. Direct `transactions.scan`/`export` callers must supply explicit
bindings as their last argument. Accept associations only after the final harness
receipt passes input/staging checks; retain raw evidence when correlation fails.

## Application

Verify `cdrom.json`, capture lengths and receipt identities. Commands report IRQ,
IRQ5 `command_error` and before/after state; response FIFO is neither consumed
nor acknowledged. IRQ3 may only acknowledge a multi-stage command. A guest
handler may consume IRQ before the observer sees it. Command errors are explicit
observations, not successful device operations.

Media reads bypass controller scheduling and prove bytes, not streaming timing,
XA playback or guest consumption. A state must correspond to mounted media;
state hash alone cannot prove this. Native restore refreshes the endpoint, cache
and SubQ/track state from media; inspect the post-restore initial context before
interpreting seeded branches. Mounting neither acquires nor rewrites media,
and does not switch discs mid-run.

For capture, inspect `transport.json` for mission termination; `events.json`,
`events.ndjson`, `payload.bin` preserve native records. `transactions.json`
separately reports controller/response, scheduler, stream, media-source, buffer,
eligibility, feed correlation, READ, PLAY, decoded-buffer and command obligations.
It retains decoded image boundaries and callback snapshots, requiring paired
basic/mode/context records at the same cycle, known command/response identities,
published response bytes and response-buffer continuity. Predicate records retain
their callback owner; branch-specific meaning requires separate validation.
`delivery` classifies inactive, busy, IRQ-delay, delivered and error-zero callbacks
from the initial context and evaluated predicates. It requires exact unscaled
delays, ordered response/lookup work, conditional IRQ assertions, native uint8 MSF
movement and preserved unrelated state/response bytes. READ bodies reject unrelated
records; media/SubQ and XA records remain confined to their delegated windows.
`command` classifies busy, repeated-delay and executed callbacks using exact
uint64 target subtraction. It checks opcode-local writes, schedules, stream
changes, drive errors, IRQ assertions and publication order. Shared parameters
and all sixteen response bytes remain checked, including unwritten tails.
ID status must match current play/position and image type; lid openness remains
an evaluated native predicate. Media/SubQ arithmetic remains a separate obligation.
A host-seeded callback does not prove guest reachability.
`submission` checks write1 register banks, queue replacement/repeats, stored BCD
parameters, seek distance, mode changes and ordered stream stops. Its
`decoder_reset_conditions` counts inferred XA predictor-reset conditions;
`decoder_reset_deferred` does not claim observed predictor writes. Profile 2 lacks
those values. The pinned native save path overwrites the left predictor field
with the right, and restore loads both channels from that field; SPU's XA pointer
also need not represent current CD decoder state. Do not use these snapshots to
validate both predictor channels or reset execution.
`playback` checks seek completion/busy returns, endpoint and autopause stop order,
report bytes, mute/feed control, native uint8 position movement and exact delays.
Reports use unattenuated signed16 sector peaks and pre-callback SubQ; unwritten
response bytes survive. Endpoint stops still require the native CDDA read;
autopause can assert DataEnd using an existing response without resizing it.
Media/SubQ generation, attenuation and feed-content arithmetic remain separate
obligations. The command consumer validates global IRQ-latch continuity.
`decoded` checks play/control/address gates, IRQ 0x200, exact rescheduling and
unchanged controller bytes. SPU reads are evaluated predicates and can change
SPU wait state; they do not prove DSP progress or decoded-buffer timing.
Lid closure with mounted media invokes an ISO read unsupported by the current
native recorder and fails capture; do not infer disc-switch support.
Buffer references address exact bytes in
`payload.bin`; identities/cycles remain decimal uint64 strings. Joined Audio
requires a complete frozen PCM export and forward epoch/event matches. Audio can
include prefix/suffix events outside History's bounds, even at the same cycle.
`initial.pbuf`/`final.pbuf` bracket capture; final state is omitted if freeze fails.

Transfer checks require mode-specific cursor/readiness changes, DMA byte counts,
addresses and scheduling, immediate not-ready completion, and emitted completion/
IRQ transitions. Scheduled callbacks may legitimately do no work when CHCR is
idle. Paired callback CHCR snapshots require completion exactly when the busy bit
was set and require the final value to preserve every other bit. The `callbacks`
list retains both active and idle cases; this does not trace every CHCR write.
Callback owner and current request can differ. Neither stopping a stream nor an
immediate completion necessarily cancels an older scheduled callback.

Correlation alone does not validate every required native callback transition,
track-table/media conformance, command completion, ADPCM/PCM arithmetic, audible
onset or byte-level bus timing. Require independent positive/failure captures for
the exact native build before treating the action as validated support.
