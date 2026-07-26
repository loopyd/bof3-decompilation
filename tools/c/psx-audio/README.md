# psx-audio — PS1 audio player, decoder, and exporter

> Decode, play, render, and inspect PlayStation audio formats from *Breath of Fire III* (and compatible archives).

## Quick start

```sh
cmake -B build && cmake --build build -j
./build/bof3-audio --help
# Play a BGM archive (EMI):
./build/bof3-audio play out/extracted/BIN/BGM/BGM000.EMI
# Interactive TUI browser:
./build/bof3-audio tui
```

## Common commands

| Task | Command |
|------|---------|
| Interactive TUI | `bof3-audio tui` |
| Play a track | `bof3-audio play <file.EMI>` |
| Render to WAV | `bof3-audio render <file.EMI> -o track.wav` |
| Render to Ogg | `bof3-audio render <file.EMI> -o track.ogg` |
| Render to FLAC | `bof3-audio render <file.EMI> -o track.flac` |
| List tracks | `bof3-audio list` |
| Inspect archive | `bof3-audio emi-inspect <file.EMI>` |
| Inspect sequence | `bof3-audio sep-inspect <file.sep>` |
| Inspect VAB bank | `bof3-audio vab-inspect <file.vh>` |
| Decode XA audio | `bof3-audio xa-decode <file.STR> -o out.wav -c 0` |
| Extract VAB samples | `bof3-audio vab-extract <vh> <vb> -o samples/` |
| Convert to SF2 | `bof3-audio vab2sf2 <file.EMI> -o bank.sf2` |
| Convert SEP to MIDI | `bof3-audio sep2mid <file.sep> -o track.mid` |
| Play individual VAG | `bof3-audio play-vag <VH> <VB> -v 0` |

## TUI controls

| Key | Action |
|-----|--------|
| `j`/`k` or `n`/`p` | Previous / next track |
| `Space` | Play / pause |
| `s` | Stop (reset position) |
| `+`/`-` | Gain up / down (0.0–3.0) |
| `f` | Cycle render format (wav → ogg → flac) |
| `r` | Render current track to `./<name>.<fmt>` |
| `/` | Search tracks by name (substring match) |
| `q` | Quit (saves last track/gain) |

## Build

### Dependencies

| Library | Required | Notes |
|---------|----------|-------|
| CMake ≥ 3.10 | Yes | Build system |
| C11 compiler | Yes | GCC, Clang, MSVC |
| pthreads | Linux/macOS | Audio device threading |
| miniaudio | Bundled | `third_party/miniaudio.h` |
| zlib | Yes | PSF decompression |

Optional render formats (auto-detected, feature gates):

| Format | Dependency |
|--------|-----------|
| Ogg | `libvorbis` + `libogg` |
| FLAC | `libflac` |

### Build & test

```sh
cmake -B build -DBUILD_TESTING=ON
cmake --build build -j
ctest --test-dir build --output-on-forward
```

Tests: `psf_test` (PSF load/CRC/overlay), `psx_machine_test` (bounded CPU
and SPU transfer/DMA writes), `spu_device_test` (voice loop/key-off and pitch
cap), and `xa_test` (synthetic XA decode and WAV header).

### Audio device

Playback requires an available audio device. Linux uses ALSA (`libasound`);
macOS uses CoreAudio; Windows uses WinMM.

## Examples

```sh
# Play sequence 1 of the opening track
bof3-audio play BGMOPN.EMI -s 1

# Play at reduced gain
bof3-audio play BGMBAT06.EMI -g 0.5

# Render all sequences to WAV (-1 = all)
bof3-audio render BGMOPN.EMI -o opening.wav -s -1

# Inspect an EMI archive
bof3-audio emi-inspect BGM054.EMI
```

## Architecture

```
main.c                  CLI dispatch + TUI
├── audio.h             Public API (RenderOutput, VabHeader, SepFile, etc.)
├── render.c            BGM renderer (register-backed SPU)
├── emi.c               EMI archive parser
├── sep.c               SEP sequence parser
├── vab.c               VAB bank parser
├── spu.c               SPU register model + ADPCM
├── spu_device.c        Register-backed SPU device
├── adpcm.c             ADPCM decoder
├── psx_machine.c       Bounded R3000 CPU (for PSF execution)
├── psf.c               PSF1/MiniPSF loader
├── xa.c                XA-ADPCM decoder
├── wav.c / ogg.c / flac.c  Output writers
├── sf2.c               SF2 exporter
└── audio_audit.c       BGM corpus auditor
```

## Format references

- [PSX SPU](https://psx-spx.consoledev.net/soundprocessingunitspu/) — hardware SPU spec
- [DuckStation SPU](https://github.com/stenzek/duckstation/blob/master/src/core/spu.cpp) — reference implementation
- VAB: Sony PsyQ VAB specification
- SEP: Sony PsyQ sequence format
- EMI: BOF3 archive format (VH + VB + SEP containers)
