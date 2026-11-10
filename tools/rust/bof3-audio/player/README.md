# Standalone audio player

Run `bof3-audio compile` to generate a complete C89 player project from your
original US executable, EMI banks or song, and extracted font and logo assets. This source
directory contains no game media. The generated directory includes `config.h`,
the supplied assets, a PS-X EXE, ELF symbols, a linker map, and build metadata.

Rebuild from the generated directory using the full repository's GCC 2.7.2 and
existing assembler and linker wrappers:

```sh
make REPO=/absolute/path/to/rebof3-simple clean all
```

`player.c` coordinates the frame loop. `audio.c` owns music sequencing, pause, restarts, musical ticks, and SPU channel
levels. `bank.c` loads embedded banks through the original routines. `sfx.c` owns
effects bank and sample selection and direct SDK-note auditions. `controls.c`
reads the original PAD routines and handles button edges and modal help.
`graphics.c` owns GPU commands,
assets, glyphs, the animated background, and frame buffers. `hud.c` owns layout,
time formatting, meter animation, and the small `Capcom C 1993` driver credit.
The audio, controls, graphics, and HUD interfaces have matching headers. `psx.h` defines
shared PSX types and hardware access.

Up and Down select the previous and next sequence in the compiled SEP. Selection
stops at the first and last track. Cross toggles play and pause. Square restarts
the selected track. Track changes and restarts preserve pause state. Pause uses
the original sequence routine, which releases active notes and retains the event
cursor for resume. SMPTE position and musical ticks stop advancing while paused.

Build an effects bank view without a music sequence:

```sh
bof3-audio compile --mode sfx \
  --archive out/extracted/BIN/BPLCHAR/DRG04_00.EMI \
  --archive out/extracted/BIN/BPLCHAR/DRG05_00.EMI \
  --executable out/extracted/SLUS_004.22 --disc-root out/extracted \
  --output out/effects-player
```

Repeat `--archive` to supply the bank list. Each input must have one VH, VB and
auxiliary entry owned by the same runtime bank slot. Left and Right switch banks;
Up and Down select physical samples. Selection stops at list boundaries and does
not play sound. Cross auditions the selected sample only when it has one
unambiguous, in-bounds SDK tone. It uses voice 0, the tone's center key, fine 0,
and left/right volumes 127. Multiple tone candidates and missing bindings remain
visible as unresolved; Cross does nothing for them. These auditions do not
establish game cue mappings or vocal roles. Square releases the voice. Switching
banks flushes key-off and waits for the old voice's envelope to reach zero before
closing the bank and replacing sample memory. A release that exceeds 600 VBlanks
leaves the old bank in place and reports an error.

The effects view displays source, runtime slot, physical sample, elapsed time and
playback state. It uses the same background and controls popup as music. It does
not open a SEP or require PPQN or a time signature. Qualified game-cue dispatch
and XA vocal playback remain unavailable. `--disc-root` supplies graphics assets.

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
voice for the meters. Effects builds retain the original runtime without that
patch and start its scheduler with no open SEP.

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
