# Frames, input and display

## Purpose

Use [frames.lua](../scripts/frames.lua) for a counted Vsync interval with optional
digital input/display capture. Use [Replay](replay.md) for press/release schedules,
state/display comparisons or terminal PC/register/RAM conditions.

## Procedure

Follow [Runtime](runtime.md#invocation); declare `support`, plus `snapshot`
for restoration. Select target, EXE entry or state, and `frames=1..36000`
(default 1). Use finite process bounds.

Optional `buttons=START,CROSS` holds case-sensitive
`PCSX.CONSTS.PAD.BUTTON` names on `slot=1|2` (1), pad 1.
`screenshot=1` requests native pixels; `savestate=1` retains machine state.
The action pauses at the final Vsync and captures RAM/registers, `frames.json`,
before/after cycles and optional state.

## Application

Frames are native GPU Vsync events, not host UI frames, instruction counts or
fixed time intervals. Observed pressed states are reported before overrides clear.
Digital overrides do not isolate physical input or implement analog trajectories;
the fresh process exits afterward.

`screen.bin` holds pixels; `screen.json` records size/format/availability.
A nonempty display also yields P6 RGB `screen.ppm`. Pixels are little-endian
BGR555 (16-bit) or RGB24, retaining native row order. Uninitialized display means
unavailable, zero bytes and no PPM. This is the displayed buffer, not all VRAM
or GPU command history.
