---
name: psx1-hw
description: "PlayStation 1 hardware reference for reverse engineering. Memory map, registers, DMA, GPU, GTE, SPU, timers, CD-ROM, interrupts, and calling conventions."
---

# PSX1 Hardware Reference

Use this skill when lifting PSX MIPS functions that touch hardware registers,
DMA, GPU, GTE, SPU, timers, CD-ROM, interrupts, or scratchpad. Read
`references/memory-map.md` for the full address space and
`references/registers.md` for the 0x1F80xxxx I/O register layout.

## Memory map

| Range | Size | Description |
| --- | --- | --- |
| `0x00000000–0x001FFFFF` | 2 MB | Main RAM (KUSEG cached) |
| `0x80000000–0x801FFFFF` | 2 MB | Main RAM (KSEG0 cached) |
| `0xA0000000–0xA01FFFFF` | 2 MB | Main RAM (KSEG1 uncached) |
| `0x1F800000–0x1F8003FF` | 1 KB | Scratchpad (fast SRAM) |
| `0x1F801000–0x1F802FFF` | ~8 KB | Hardware I/O registers |
| `0x1FC00000–0x1FC7FFFF` | 512 KB | BIOS ROM |

KUSEG/KSEG0/KSEG1 all access the same 2 MB RAM; the segment determines
caching and TLB behavior. PSX games normally use KSEG0 (cached) for code/data
and KSEG1 (uncached) for DMA/MMIO buffers.

## Scratchpad

1 KB SRAM at `0x1F800000`. Zero-wait-state, private to the CPU. Games use it
for temporary work areas, stack frames, or per-overlay state. The common
scratchpad pointer pattern:

```c
#define SCRATCH_PTR ((volatile void**)0x1F800044u)
#define SCRATCH     ((volatile u8*)0x1F800044u)
```

BOF3 stores a per-overlay scratchpad pointer at `0x1F800044`.

## Hardware I/O registers (0x1F801000–0x1F802FFF)

Access with `REG8()`, `REG16()`, `REG32()` from `include/bof3/defines.h`.
All registers are little-endian. Write-only bits read as zero.

### GPU (0x1F801810–0x1F801814)

| Address | R/W | Name | Description |
| --- | --- | --- | --- |
| `0x1F801810` | W | GP0 | GP0 command register (draw, VRAM, mode) |
| `0x1F801810` | R | GPUREAD | GPU data read (VRAM, info) |
| `0x1F801814` | W | GP1 | GP1 command register (display, mode) |
| `0x1F801814` | R | GPUSTAT | GPU status register |

GP0 commands: `0x00–0x1F` GPU info/reset, `0x20–0x3F` polyline,
`0x40–0x5F` line, `0x60–0x7F` rectangle, `0x80–0x9F` VRAM copy,
`0xA0–0xBF` CPU→VRAM, `0xC0–0xDF` VRAM→CPU, `0xE0–0xFF` env/misc.

### GTE (Geometry Transformation Engine)

Coprocessor 2 (COP2). Accessed via `cop2` instructions or `MTC2`/`MFC2`.
No memory-mapped registers; all GTE registers are internal COP2 registers
selected by the instruction suffix.

Key GTE instructions:
- `RTPT` — perspective transform (3 points)
- `RTPT` with `sf` bit — shift fraction
- `MVMVA` — matrix-vector multiply-add
- `AVSZ3`/`AVSZ4` — average Z (depth sorting)
- `NCCT`/`NCS`/`NCT` — normal color (lighting)
- `DPCS`/`DPCT` — depth cue (fog)
- `SQR` — square vector
- `OP` — outer product
- `GPF`/`GPL` — general purpose interpolation

GTE result registers: `IR0–IR3`, `MAC0–MAC3`, `OTZ`, `XY_FIFO`, `Z_FIFO`,
`RGB_FIFO`, `LZCS`, `LZCR`.

### DMA (0x1F801080–0x1F8010FF)

7 DMA channels. Each channel has 3 registers:

| Offset | Name | Description |
| --- | --- | --- |
| `+0x00` | D_MADR | DMA base address (word-aligned) |
| `+0x04` | D_BCR | DMA block control (word count / block size) |
| `+0x08` | D_CHCR | DMA channel control (sync, direction, start) |

Channel base addresses:

| Channel | Base | Typical use |
| --- | --- | --- |
| 0 | `0x1F801080` | MDECin |
| 1 | `0x1F801090` | MDECout |
| 2 | `0x1F8010A0` | GPU (ordered list / linked list) |
| 3 | `0x1F8010B0` | CD-ROM |
| 4 | `0x1F8010C0` | SPU |
| 5 | `0x1F8010D0` | PIO (external) |
| 6 | `0x1F8010E0` | GPU (reverse clear) |

DMA control register at `0x1F8010F0` (DPCR): enable bits for each channel.
DMA interrupt register at `0x1F8010F4` (DICR): interrupt flags and master enable.

### Timers (0x1F801100–0x1F80112F)

3 timers (root counters). Each timer has:

| Offset | Name | Description |
| --- | --- | --- |
| `+0x00` | TIMER_VALUE | Current counter value |
| `+0x04` | TIMER_MODE | Mode bits (sync, target, IRQ, clock source) |
| `+0x08` | TIMER_TARGET | Compare target value |

| Timer | Base | Clock source options |
| --- | --- | --- |
| 0 | `0x1F801100` | Pixel clock / dotclock |
| 1 | `0x1F801110` | Hblank / dotclock |
| 2 | `0x1F801120` | System clock / Hblank |

### SPU (0x1F801C00–0x1F801FFF)

24 ADPCM voices. Voice registers at `0x1F801C00 + voice*0x10`:

| Offset | Name | Description |
| --- | --- | --- |
| `+0x00` | VOL_L/R | Volume left/right |
| `+0x04` | PITCH | Pitch (4.12 fixed point, 0x1000 = 1x) |
| `+0x08` | STARTADDR | ADPCM start address (word address) |
| `+0x0C` | ADSR | Attack/Decay/Sustain/Release envelope |
| `+0x0E` | ADSR_VOL | Current ADSR volume |
| `+0x10` | LOOPADDR | ADPCM loop repeat address |

Main SPU registers at `0x1F801D80`:
- `MVOL_L/R` — master volume
- `RVOL_L/R` — reverb volume
- `KON` — key on (start voices)
- `KOFF` — key off (stop voices)
- `PMON` — pitch modulation enable
- `NOISE` — noise enable
- `ENDX` — voice end flags
- `RVON` — reverb enable

### CD-ROM (0x1F801800–0x1F801803)

| Address | R/W | Name | Description |
| --- | --- | --- | --- |
| `0x1F801800` | R/W | CD_STAT | Status register |
| `0x1F801801` | R | CD_DATA | Response FIFO |
| `0x1F801801` | W | CD_CMD | Command register |
| `0x1F801802` | R | CD_DATA | Data FIFO |
| `0x1F801802` | W | CD_IRQM | Interrupt mask |
| `0x1F801803` | R | CD_IRQF | Interrupt flags |
| `0x1F801803` | W | CD_IRQF | Interrupt acknowledge |

### Interrupts (0x1F801070–0x1F801074)

| Address | R/W | Name | Description |
| --- | --- | --- | --- |
| `0x1F801070` | R/W | I_STAT | Interrupt status (write to ack) |
| `0x1F801074` | R/W | I_MASK | Interrupt mask |

Interrupt bits (bit position):
- 0: VBlank
- 1: GPU
- 2: CD-ROM
- 3: DMA
- 4: Timer 0
- 5: Timer 1
- 6: Timer 2
- 7: Controller/Memory Card
- 8: SIO
- 9: SPU
- 10: Lightpen

### Controller/Memory Card (0x1F801040–0x1F80104F)

| Address | R/W | Name | Description |
| --- | --- | --- | --- |
| `0x1F801040` | R/W | JOY_TX_DATA | TX data |
| `0x1F801044` | R/W | JOY_STAT | Status (TX ready, RX FIFO, etc.) |
| `0x1F801048` | R/W | JOY_MODE | Mode (baudrate, parity, etc.) |
| `0x1F80104A` | R/W | JOY_CTRL | Control (TX enable, RX enable, etc.) |
| `0x1F80104E` | R/W | JOY_BAUD | Baudrate divisor |

## MIPS calling conventions (O32 ABI)

| Register | Name | Purpose |
| --- | --- | --- |
| `$zero` | `$0` | Always zero |
| `$at` | `$1` | Assembler temporary |
| `$v0–$v1` | `$2–$3` | Return values |
| `$a0–$a3` | `$4–$7` | Arguments |
| `$t0–$t7` | `$8–$15` | Temporaries (caller-saved) |
| `$s0–$s7` | `$16–$23` | Saved (callee-saved) |
| `$t8–$t9` | `$24–$25` | Temporaries (caller-saved) |
| `$k0–$k1` | `$26–$27` | Kernel reserved |
| `$gp` | `$28` | Global pointer |
| `$sp` | `$29` | Stack pointer |
| `$fp` | `$30` | Frame pointer |
| `$ra` | `$31` | Return address |

Stack grows downward. Arguments 5+ go on the stack. `$v0` holds return value.
`$gp` is used for position-independent data access in PsyQ binaries.

## Delay slots

Every branch/jump instruction has a delay slot: the instruction after the
branch always executes. The compiler fills the delay slot with:
- A useful instruction from before the branch (preferred)
- A `nop` (when nothing fits)
- An instruction from the branch target (branch likely)

When lifting, preserve delay-slot semantics. `bin/asmdiff` compares the
instruction stream including delay slots.

## Common BOF3 patterns

- `lui $at, %hi(D_XXXXXXXX)` + `lw $reg, %lo(D_XXXXXXXX)($at)` — 32-bit
  address load via two-instruction sequence
- `REG32(0x1F80xxxx)` — direct hardware register access
- `SCRATCH_PTR` — per-overlay scratchpad pointer at `0x1F800044`
- `cop2` instructions — GTE geometry/lighting operations
- DMA channel 2 — GPU ordered-list rendering
- DMA channel 4 — SPU ADPCM streaming

## References

- [Nocash PSX Specifications](https://problemkaputt.de/psx-spx.htm)
- [PSX-SPX](https://psx-spx.consoledev.net/)
- [PSX.Dev Wiki](https://psxdev.wiki/)
