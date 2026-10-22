# Standalone audio player

Run `bof3-audio compile` to generate a complete C89 player project from your
original US executable, EMI song, and extracted font and logo assets. This source
directory contains no game media. The generated directory includes `config.h`,
the supplied assets, a PS-X EXE, ELF symbols, a linker map, and build metadata.

Rebuild from the generated directory using the full repository's GCC 2.7.2 and
existing assembler and linker wrappers:

```sh
make REPO=/absolute/path/to/rebof3-simple clean all
```

The player loads at `0x80010000`, keeps the original runtime at `0x80096800`, and
uses a stack below it. The linker rejects player assets that exceed the reserved
low-memory region. The runtime's original sound initialization, bank transfers,
SEP callbacks, sequencer and glyph renderer remain in use. A note-call hook records
the MIDI channel associated with each allocated SPU voice for the meters.

`text_scaled(x, y, string, width, height)` takes glyph dimensions in screen pixels.
It uses the original glyph renderer to produce glyph positions and texture UVs,
then converts its sprites to textured quads when scaling is requested. Ordinary
labels and SMPTE use 8x8 pixels; the musical position and time labels use 6x6.
SMPTE uses 30-frame non-drop numbering, advancing once per two NTSC VBlanks.
Musical position uses the driver's live delta counters and the SEP header's PPQN
and time signature. Bars and beats start at 1, ticks at 0; controller loops advance
musical time, while whole-sequence restarts reset it.

`player_status` exposes the initialization phase, elapsed VBlank count, number of
whole-sequence restarts, error code, and musical tick count for debugger inspection. Phase 4 means
playback started; phase 255 means initialization failed. Native sequence loops
do not increment the whole-sequence restart counter.
