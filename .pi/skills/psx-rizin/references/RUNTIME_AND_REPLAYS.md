# Runtime analysis and replay coverage

## Tool roles

### PCSX-Redux

Primary runtime evidence tool:

- execution/read/write breakpoints
- GDB server
- Lua scripting and memory/register access
- GPU logger and debugging facilities
- deterministic narrow experiments when settings are fixed

For CPU debugging, enable the debugger and disable the dynarec/interpreter optimizations as required by PCSX-Redux documentation. GDB typically connects through the emulator's configured server (documented examples use port 3333).

### BizHawk

Primary input-movie/re-recording corpus tool:

- record and replay deterministic input sequences
- TAStudio editing
- savestate-based scenario construction
- Lua/debugging interfaces where supported

Record core, firmware/BIOS, sync settings, disc hash, region, and movie format/version. Do not assume a movie recorded in one emulator is portable to another.

### DuckStation

Useful cross-check:

- broad CPU trace logging
- convenient gameplay/replay testing
- debugger/logging features

CPU logs can grow to many gigabytes. Use narrow frame windows, known breakpoints, or filtered scenarios.

### Ghidra + PCSX-Redux

PCSX-Redux documents a GDB connection path for Ghidra's MIPS debugger. Use it when live memory/register inspection in Ghidra materially helps, while keeping Rizin artifacts as the case's reproducible static database.

## Replay corpus definition

A replay corpus includes more than emulator movies:

1. built-in attract-mode demos
2. native replay/ghost files
3. memory-card saves
4. scripted cutscenes/tutorials
5. debug menu/level-select paths
6. external TAS/input movies
7. newly recorded minimal reproduction scenarios

Search executables and assets for “demo”, “replay”, “ghost”, “record”, controller buffers, frame counters, and save-file identifiers. Validate discoveries dynamically.

## Replay matrix

Use `templates/replay-matrix.csv`. Required dimensions:

- stable replay/scenario ID
- source and hashes
- emulator/core/version
- BIOS hash and region
- initial save/state
- controller ports/devices
- sync/determinism settings
- frame start/end
- expected event
- overlays loaded
- functions expected/hit
- watched ranges
- trace files
- outcome and blockers

## Function-entry capture

At function entry capture:

```text
frame/cycle
effective PC and overlay identity
RA and caller call-site
GP, SP, FP
a0-a3
stack words covering argument area and selected locals
relevant object/global snapshots
```

At return capture `v0`, `v1`, mutated memory, and return path. For nested/recursive calls, maintain a call-depth or invocation ID.

## PCSX-Redux breakpoint pattern

Use execution breakpoints for entry/return, and read/write watchpoints for fields or overlay load destinations. PCSX-Redux's Lua API exposes breakpoint types and memory/register access; script callbacks should write structured JSONL or CSV with frame and address context.

Avoid unbounded logging in a callback. Filter by:

- frame range
- caller/RA
- argument value
- overlay active ID
- object pointer range
- invocation count

## Narrowing an unknown behavior

1. Record a movie that reaches the behavior.
2. Create a nearby savestate/checkpoint.
3. Compare memory before/after the event.
4. Find writes to changed fields.
5. Break on the writer and inspect the call chain.
6. Add execution breakpoints to candidate functions.
7. Replay the exact input.
8. merge confirmed addresses/arguments into Rizin.
9. create a negative/control replay where the event does not occur.

## Overlay tracing

Set write watchpoints on candidate destination pages/ranges. Log:

- first and last write
- source loader/caller
- bytes/count
- decompression state
- cache maintenance calls
- first execution in the range
- callbacks registered from the range

Dump bytes after loading and before replacement.

## Coverage model

Track at least:

- function hit coverage
- overlay load/entry coverage
- branch/state coverage for the target subsystem
- replay/scenario status
- argument value diversity
- field read/write coverage

Coverage is evidence-specific; do not claim whole-program coverage from a function hit list.

## Determinism failures

Common causes:

- different BIOS or region
- emulator/core version
- dynarec/interpreter differences
- CD timing
- uninitialized memory
- RTC/random seed
- controller device/config
- savestate incompatibility
- frame pacing/input polling boundaries

When a replay diverges, record the first divergent frame/state and preserve both runs. Do not silently “fix” the movie and overwrite provenance.
