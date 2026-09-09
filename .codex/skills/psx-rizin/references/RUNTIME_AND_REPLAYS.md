# Runtime analysis and replay coverage

## Tool roles

| Tool | Role |
|---|---|
| PCSX-Redux | primary runtime evidence: exec/read/write breakpoints, GDB server, Lua scripting + memory/register access, GPU logger, deterministic narrow experiments |
| BizHawk | input-movie corpus: deterministic replays, TAStudio editing, savestate scenarios, Lua/debug interfaces |
| DuckStation | cross-check: broad CPU trace logging, gameplay/replay testing (logs can reach GBs — constrained windows only) |
| Ghidra + PCSX-Redux | live memory/register inspection via GDB when it materially helps; Rizin artifacts stay the reproducible static DB |

CPU debugging: enable the debugger, disable dynarec/interpreter optimizations as PCSX-Redux docs require; GDB connects through the emulator's server (examples use port 3333). Record core, firmware/BIOS, sync settings, disc hash, region, movie format/version. A movie recorded in one emulator is not portable to another.

## Replay corpus definition

More than emulator movies:

1. built-in attract-mode demos
2. native replay/ghost files
3. memory-card saves
4. scripted cutscenes/tutorials
5. debug menu/level-select paths
6. external TAS/input movies
7. newly recorded minimal reproductions

Search executables/assets for "demo", "replay", "ghost", "record", controller buffers, frame counters, save identifiers; validate discoveries dynamically.

## Replay matrix

Use `templates/replay-matrix.csv`. Dimensions: stable ID · source + hashes · emulator/core/version · BIOS hash + region · initial save/state · controller ports/devices · sync/determinism settings · frame start/end · expected event · overlays loaded · functions expected/hit · watched ranges · trace files · outcome + blockers.

## Function-entry capture

```text
frame/cycle
effective PC + overlay identity
RA + caller call-site
GP, SP, FP
a0-a3
stack words (argument area + selected locals)
relevant object/global snapshots
```

At return capture `v0`, `v1`, mutated memory, return path. Nested/recursive calls: maintain a call-depth or invocation ID.

## PCSX-Redux breakpoint pattern

Exec breakpoints for entry/return; read/write watchpoints for fields or overlay load destinations. Lua callbacks → structured JSONL/CSV with frame + address context. No unbounded logging; filter by frame range, caller/RA, argument value, overlay ID, object pointer range, invocation count.

## Narrowing an unknown behavior

1. Record a movie reaching the behavior.
2. Create a nearby savestate/checkpoint.
3. Compare memory before/after the event.
4. Find writes to changed fields.
5. Break on the writer; inspect the call chain.
6. Add exec breakpoints to candidate functions.
7. Replay the exact input.
8. Merge confirmed addresses/arguments into Rizin.
9. Negative/control replay where the event does not occur.

## Overlay tracing

Write watchpoints on candidate destination pages/ranges. Log: first/last write · source loader/caller · bytes/count · decompression state · cache maintenance calls · first execution · callbacks registered from the range. Dump bytes after loading and before replacement.

## Coverage model

Track at least: function hit coverage · overlay load/entry coverage · branch/state coverage for the target subsystem · replay/scenario status · argument value diversity · field read/write coverage. Coverage is evidence-specific — never claim whole-program coverage from a function hit list.

## Determinism failures

Causes: BIOS/region · emulator/core version · dynarec/interpreter differences · CD timing · uninitialized memory · RTC/random seed · controller device/config · savestate incompatibility · frame pacing/input polling boundaries. On divergence: record the first divergent frame/state and preserve both runs. Never silently "fix" a movie and overwrite provenance.
