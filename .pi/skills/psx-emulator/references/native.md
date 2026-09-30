# Native customization

## Purpose

Customize PCSX-Redux bindings with exact source/build identity.

## Procedure

1. Inspect `bin/harness patch list --target pcsx-redux --status` and read-only
   `check`. Patch inputs: `inputs/patches/pcsx-redux/*.patch`.
2. Obtain independent boundary/threading review and specific user authorization
   before changing dependency source. Preserve unrelated edits.
3. Run `bin/harness setup --component pcsx-redux` to apply the approved series
   and rebuild, then `bin/harness runtime status` to verify identities/linkage.
4. Validate guards, positive/failure missions and output semantics before
   publishing support; a build is insufficient.

Commands: `list`, `check`, `apply`, `revert`. Repeat `--target NAME` to filter;
`--patch FILE.patch` requires one target. Omitted filters select all. Plain listing
needs no checkout; `--status` reports applied/pristine/unresolved, with diagnostics
and exit 2 for unresolved state. Apply/revert enforce series dependencies; setup
reconstructs exact images from the pinned base and series before compiling.
Targets may declare prerequisite order; unlisted patches follow by filename.
PCSX-Redux orders debugger, exceptions, history, sound, video, then CD-ROM.

`harness.commands.patch` owns CLI dispatch, `harness.patches` the framework,
`harness.runtime.patches` target policy and `harness.toolchain.pcsx` builds.
Doctor/runtime reject source, recursive revision, patch or binary-receipt drift.
Never bypass dirty-source checks, reset user changes, implicitly change the gitlink
or add fallbacks. Binary changes invalidate origin receipts.

## Application

Use [Step](step.md) for guarded `PCSX.Debugger.stepInto()` and
[History](history.md) for `PCSX.History`, [GPU](gpu.md) for its GPU records and
[Audio](audio.md) for `PCSX.Audio`, and [CD-ROM](cdrom.md) for the explicit
`PCSX.History.beginCD` profile; discover installed patches with `list`.
Step-into requires a paused, nonquitting interpreter with debugger enabled.
Defer outside Pause/breakpoint dispatch and await a pause with bounded CPU/delay/
exception context. Returning means scheduled, not completed; IRQs may cross
several instructions.

In `src/core/debug.cc`, STEP_IN follows debugger observations; IRQ entry can defer
the pause. `triggerBP` resets step state after Pause, losing synchronous re-entry.
Over/out depend on delayed call recognition or tracked stacks and may leave
return breakpoints after other pauses. They are excluded; never substitute
guessed returns or into loops.

`exceptions.patch` cancels the active delayed branch for taken-branch SYSCALL/BREAK.
The interpreter does not mark untaken branches as delay slots; other exception
paths are outside that fix. Retain `-no-pcdrv`: PCdrv-handled BREAK continues
differently. Validate both slot indices when changing traps. Emulator behavior
does not establish hardware equivalence.
