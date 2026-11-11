# Standalone audio player

Run `bof3-audio compile` to generate a C89 player project from the original US
executable and extracted font and logo assets. Supply a song or banks for music
or SFX mode, or a raw US CUE sheet for vocal mode. The generated directory includes
`config.h`, the embedded assets, a PS-X EXE, ELF symbols, a linker map, and build
metadata.

Rebuild from the generated directory using the full repository's GCC 2.7.2 and
existing assembler and linker wrappers:

```sh
make REPO=/absolute/path/to/rebof3-simple clean all
```

`player.c` coordinates the frame loop. `audio.c` owns music transport, musical
ticks, and SPU channel levels. `bank.c` loads embedded banks through the original routines. `sfx.c` owns
effects bank and sample selection and direct SDK-note auditions. `vocal.c`
services original XA playback and handles play, stop, and repeat. `controls.c`
reads the original PAD routines and handles button edges and modal help.
`graphics.c` owns GPU commands,
assets, glyphs, the animated background, and frame buffers. `hud.c` owns layout,
time formatting, meter animation, and the small `Capcom C 1993` driver credit.
The audio, controls, graphics, and HUD interfaces have matching headers. `psx.h` defines
shared PSX types and hardware access.

Up and Down select the previous and next sequence in the compiled SEP. Selection
stops at the first and last track. Cross toggles play and pause. Square restarts
the selected track. Left and Right seek five seconds backward and forward. The
player mutes output while the original driver plays to the destination. A
backward seek or a seek from pause rebuilds playback from the track start at
normal speed, so the wait depends on the destination. The HUD shows `SEEKING`
until it can display the new SMPTE and musical positions. Track changes,
restarts, and seeks preserve play or pause intent. Seeking clamps to the start
or a finite end. Cross at the end starts the track again. SMPTE position and
musical ticks stay fixed while paused.

Build an effects bank view without a music sequence:

```sh
bof3-audio compile --mode sfx \
  --archive out/extracted/BIN/BPLCHAR/DRG04_00.EMI \
  --archive out/extracted/BIN/BPLCHAR/DRG05_00.EMI \
  --executable out/extracted/SLUS_004.22 --disc-root out/extracted \
  --output out/effects-player
```

Repeat `--archive` to supply the bank list. Each input must have one VH, VB and
auxiliary entry owned by the same runtime bank slot. Left and Right switch banks.
Up and Down select physical samples. Selection stops at list boundaries and
leaves playback alone.

Cross plays the selected sample through the original dispatcher when exactly
one local cue binds all of its layers to that physical sample. The HUD shows the
packed game cue ID. When no such cue exists and the sample has a single valid SDK
tone, Cross auditions that tone on voice 0 at its center key, with fine 0 and
left/right volumes of 127. The HUD labels this path `SDK NOTE`. Multiple cue
bindings or a missing playback binding leave Cross inactive. The build report
lists the cue candidates and the reasons individual rows are unresolved.

Square releases the voices from the current play. Before retriggering or
switching banks, the player flushes key-off and waits for every affected
envelope to reach zero. A release that exceeds 600 VBlanks leaves the old bank
in place and reports an error. The effects view shows the source, runtime slot,
physical sample, cue, elapsed time and playback state. It shares the music
background and controls popup and requires no SEP.

Build the Japanese vocal view with the original two-file US CUE sheet and both
of its raw BINARY tracks:

```sh
bof3-audio compile --mode vocal \
  --disc 'inputs/external/Breath of Fire III (USA).cue' \
  --executable out/extracted/SLUS_004.22 --disc-root out/extracted \
  --output out/vocal-player
```

Mount the same disc when running `player.exe`. The build verifies the two tracks
and the original `BIN/SCE_XA/VOICE.STR` extent, then records the CUE, TOC,
source hashes, packed cue, file, channel and coding in `player.json`.
`--disc-root` supplies graphics assets. The CUE and tracks stay outside the
generated project. Vocal builds use the original CD initialization, XA starter,
completion callback and foreground service at a two-VBlank cadence.

Cross plays cue `0x2000` or stops and retriggers it. Circle stops playback.
Square toggles repeat, which starts the cue again after the original transport
returns to idle. Left and Right have no effect. Up and Down keep the single
qualified cue selected. The displayed time measures elapsed transport time
from the start call in VBlanks.

L1 toggles the controls popup. Its translucent purple gradient and beveled gray
border appear over the animated background. While help is open, playback keeps
its current state and every other control is blocked. L1 has priority on opening
and closing frames. Other buttons need a fresh press after help closes.

`graphics_panel(x, y, width, height, top, bottom, border)` draws a translucent
vertical gradient. Colors use the GPU's `0xBBGGRR` packing. Callers choose any two
colors and the panel size. The optional gray bevel stays inside the requested
bounds, with light top and left edges and dark bottom and right edges. Panels
clip at the 320x240 viewport; zero-sized panels and origins outside it draw
nothing. The HUD and popup share this routine for panels over the cube, meter
wells, transport information, and controls.

The player loads at `0x80010000`, keeps the original runtime at `0x80096800`, and
uses a stack below it. The linker rejects player assets that exceed the reserved
low-memory region. The runtime's original sound initialization, bank transfers,
SDK scheduler and glyph renderer remain in use. Music uses SEP callbacks and a
note-call hook that records the MIDI channel associated with each allocated SPU
voice for the meters. Effects and vocal builds retain the unpatched runtime.
Effects builds start the SDK scheduler with no open SEP.

`graphics_text_scaled(x, y, string, width, height)` takes glyph dimensions in
screen pixels. It uses the original glyph renderer to produce positions and texture UVs,
then converts its sprites to textured quads when scaling is requested. Ordinary
labels and SMPTE use 8x8 pixels. Musical position, time labels, and the driver
credit use 6x6.
SMPTE uses 30-frame non-drop numbering, advancing once per two NTSC VBlanks.
Musical position uses the driver's live delta counters and the SEP header's PPQN
and time signature. Bars and beats start at 1, ticks at 0; controller loops advance
musical time, while whole-sequence restarts reset it.

`player_status` exposes the initialization phase, elapsed VBlank count, number of
whole-sequence restarts, error code, musical tick count, selected track, pause
state, playback position in VBlanks, selected effects bank and sample, and effects
playback state for debugger inspection. `player_controls`
exposes held buttons, accepted button edges, and popup visibility. Phase 4 means
the player is ready; phase 255 means initialization failed. Native sequence loops
do not increment the whole-sequence restart counter.
