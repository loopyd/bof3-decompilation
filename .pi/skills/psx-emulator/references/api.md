# Extending emulator actions

## Purpose

Extend Lua missions using the pinned source and bundled LuaJIT, `pb` and `protoc`.
Prefer that source over documentation for another revision; do not add a decoder
or generated Rust runner.

## Procedure

1. Check [action routing](../SKILL.md#select-an-action) for an existing runner.
2. Locate the owner below; establish timing, side effects, threads and exclusions.
3. Load `support` before native calls: it disables host Lua JIT compilation and
   flushes existing traces for synchronous callback reentry. Never re-enable it
   during maintained missions. Use its bounds, callback retention and protected
   completion/failure; declare modules/inputs through the harness.
4. Validate a real mission and bounded failure; update its procedure. Missing
   bindings require [native customization](native.md), specific source
   authorization and independent semantic review.

Bundled rules below govern routine work; URLs retain provenance only. Consult
external documentation only for explicitly scoped research, then verify against
pinned source and capture reusable findings here. Source-owner paths follow table.

| Provenance | Bundled calling rule |
| --- | --- |
| [Basics](https://pcsx-redux.consoledev.net/Lua/redux-basics/) | Raw state/schema, controller overrides and display capture; GUI states use gzip |
| [Breakpoints](https://pcsx-redux.consoledev.net/Lua/breakpoints/) | Interpreter/debugger observations; retain hooks, guard errors, pause explicitly; false removes a hook |
| [Memory/registers](https://pcsx-redux.consoledev.net/Lua/memory-and-registers/) | Bounds-check pointers; invalidate code caches; verify MMIO effects in source |
| [Events](https://pcsx-redux.consoledev.net/Lua/events/) | Retain listeners; count native Vsyncs; defer host work outside dispatch |
| [Files](https://pcsx-redux.consoledev.net/Lua/file-api/) | Native slices and explicit state inputs; close files |
| [Libraries](https://pcsx-redux.consoledev.net/Lua/libraries/) / [lua-protobuf](https://github.com/starwing/lua-protobuf) | Schema reflection and `int64_as_string` (`#decimal`); verify bundled behavior |
| [CLI flags](https://pcsx-redux.consoledev.net/cli_flags/) | Portable settings and explicit interpreter/debugger options |
| [LuaJIT controls](https://luajit.org/ext_jit.html) / [FFI callbacks](https://luajit.org/ext_ffi_semantics.html#callback) | `jit.off()` stops host compilation; `jit.flush()` clears traces. Native event dispatch may synchronously reenter Lua; do not rely on automatic call blacklisting |

Paths below are relative to `third_party/pcsx-redux/`:

| Owner | Inspect for |
| --- | --- |
| `src/core/pcsxffi.lua`, `pcsxlua.cc` | Pointers, registers, state/schema, screenshots and callbacks |
| `src/core/sstate.h`, `sstate.cc`; `src/spu/types.h`, `freeze.cc` | Version-4 schema, fixed regions, delayed loads, restoration and serialized DSP coverage |
| `src/core/debug.cc`, `psxinterpreter.cc` | Pre-instruction/access observations, retirement and exception behavior |
| `src/core/eventslua.cc`, `pad.cc` | Vsync and digital button overrides |
| `src/core/ui.cc`, `src/main/main.cc`, `src/core/arguments.cc` | EXE handoff, callback dispatch, CLI ordering and settings |
| `src/core/psxmem.cc` | Debugger memory-file versus guest bus effects |
| `src/gpu/soft/gpu.cc` | GP0/GP1, VRAM, display rectangle, rows and pixel formats |
| `src/core/gte-instructions.cc` | Native COP2 arithmetic; interpreter gates on Status.CU2 |
| `src/spu/registers.cc` | Port side effects |
| `src/core/isoffi.lua`, `cdrom.cc` | Mounted media/sector modes/ISO reader versus controller lifecycle |
| `third_party/lua-protobuf/README.md` | Reflection, integer options and decoding behavior |

## Application

`MemoryAsFile::writeBlock` bypasses device handlers. [MMIO probes](bus.md) execute
effects using scratch RAM and advance guest/device time.

`PCSX.nextTick` defers host work; it does not step. The UI poller discards nested
nextTick callbacks. Follow [Step](step.md), [Trace](trace.md) and [Replay](replay.md)
boundaries.

Save/load can alter scheduling; serialized state does not establish determinism
or complete hardware coverage. `PCSX.SPU.playAudio` provides host preview, not
emulated PCM. Analog trajectories, disc switching and hardware-fidelity claims
require explicit inputs, bounds and validation; never approximate silently.
Keep experiment evidence outside procedural references.
