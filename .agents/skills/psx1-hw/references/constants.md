# PSX1 Common Constants and Bitmasks

## GPU command prefixes (top 3 bits of first word)

| Bits 29–31 | Category |
| --- | --- |
| `0b000` | GPU command (draw, VRAM, env) |
| `0b001` | — |
| `0b010` | Monochrome polygon |
| `0b011` | Textured polygon |
| `0b100` | Line |
| `0b101` | — |
| `0b110` | Rectangle |
| `0b111` | — |

## GPU drawing command structure

Each vertex word: `YYYY_XXXX` (16-bit signed X, 16-bit signed Y).
Color word: `BBGGRR` (24-bit RGB, blue first in memory).

### Polygon vertex count (bits 24–25 of command)

| Bits | Vertices |
| --- | --- |
| `0b00` | 3 (triangle) |
| `0b01` | 4 (quad) |

### Rectangle size (bits 26–27 of command)

| Bits | Size |
| --- | --- |
| `0b00` | Variable (size follows) |
| `0b01` | 1×1 (dot) |
| `0b10` | 8×8 |
| `0b11` | 16×16 |

### Texture page (bits 0–5 of draw mode word)

| Bits | Value | Texture page X |
| --- | --- | --- |
| 0–3 | 0–15 | X base (640 pixels each) |
| 4 | 0–1 | Y base (0 or 256) |
| 5–6 | 0–2 | Color mode (4-bit, 8-bit, 15-bit) |

### Semi-transparency mode (bits 5–6 of draw mode word)

| Mode | Formula |
| --- | --- |
| 0 | `B/2 + F/2` |
| 1 | `B + F` |
| 2 | `B - F` |
| 3 | `B + F/4` |

B = background, F = foreground.

### Texture color modes (bits 7–8 of draw mode)

| Mode | Description |
| --- | --- |
| 0 | 4-bit CLUT (16 colors) |
| 1 | 8-bit CLUT (256 colors) |
| 2 | 15-bit direct (32768 colors) |
| 3 | 24-bit direct (rare) |

## GTE register indices

### Data registers (read with MFC2, write with MTC2)

| Index | Name | Description |
| --- | --- | --- |
| 0 | VXY0 | Vector 0 X,Y (packed) |
| 1 | VZ0 | Vector 0 Z |
| 2 | VXY1 | Vector 1 X,Y (packed) |
| 3 | VZ1 | Vector 1 Z |
| 4 | VXY2 | Vector 2 X,Y (packed) |
| 5 | VZ2 | Vector 2 Z |
| 6 | RGBC | Color+Code (packed RGBA) |
| 7 | OTZ | Average Z value |
| 8 | IR0 | Intermediate result 0 (16-bit) |
| 9 | IR1 | Intermediate result 1 |
| 10 | IR2 | Intermediate result 2 |
| 11 | IR3 | Intermediate result 3 |
| 12 | SXY0 | Screen XY FIFO entry 0 |
| 13 | SXY1 | Screen XY FIFO entry 1 |
| 14 | SXY2 | Screen XY FIFO entry 2 |
| 15 | SXYP | Screen XY FIFO push |
| 16 | SZ0 | Screen Z FIFO entry 0 |
| 17 | SZ1 | Screen Z FIFO entry 1 |
| 18 | SZ2 | Screen Z FIFO entry 2 |
| 19 | SZ3 | Screen Z FIFO entry 3 |
| 20 | RGB0 | Color FIFO entry 0 |
| 21 | RGB1 | Color FIFO entry 1 |
| 22 | RGB2 | Color FIFO entry 2 |
| 23 | RES1 | Reserved |
| 24 | MAC0 | Accumulator 0 (32-bit) |
| 25 | MAC1 | Accumulator 1 |
| 26 | MAC2 | Accumulator 2 |
| 27 | MAC3 | Accumulator 3 |
| 28 | IRGB | Input RGB (write: sets IR1-3) |
| 29 | ORGB | Output RGB (read: IR1-3 as color) |
| 30 | LZCS | Leading zero count source |
| 31 | LZCR | Leading zero count result |

### Control registers (read with CFC2, write with CTC2)

| Index | Name | Description |
| --- | --- | --- |
| 0 | R11R12 | Rotation matrix row 1, col 1-2 |
| 1 | R13R21 | Rotation matrix row 1 col 3, row 2 col 1 |
| 2 | R22R23 | Rotation matrix row 2, col 2-3 |
| 3 | R31R32 | Rotation matrix row 3, col 1-2 |
| 4 | R33 | Rotation matrix row 3, col 3 |
| 5 | TRX | Translation vector X |
| 6 | TRY | Translation vector Y |
| 7 | TRZ | Translation vector Z |
| 8 | L11L12 | Light source direction row 1, col 1-2 |
| 9 | L13L21 | Light source direction row 1 col 3, row 2 col 1 |
| 10 | L22L23 | Light source direction row 2, col 2-3 |
| 11 | L31L32 | Light source direction row 3, col 1-2 |
| 12 | L33 | Light source direction row 3, col 3 |
| 13 | RBK | Background color red |
| 14 | GBK | Background color green |
| 15 | BBK | Background color blue |
| 16 | LR1LR2 | Light source color row 1, col 1-2 |
| 17 | LR3LG1 | Light source color row 1 col 3, row 2 col 1 |
| 18 | LG2LG3 | Light source color row 2, col 2-3 |
| 19 | LB1LB2 | Light source color row 3, col 1-2 |
| 20 | LB3 | Light source color row 3, col 3 |
| 21 | RFC | Far color red |
| 22 | GFC | Far color green |
| 23 | BFC | Far color blue |
| 24 | OFX | Screen offset X (24.0 fixed) |
| 25 | OFY | Screen offset Y (24.0 fixed) |
| 26 | H | Projection plane distance |
| 27 | DQA | Depth queue coefficient |
| 28 | DQB | Depth queue offset |
| 29 | ZSF3 | Z scale factor 3 (for AVSZ3) |
| 30 | ZSF4 | Z scale factor 4 (for AVSZ4) |
| 31 | FLAG | GTE flag/error bits |

### GTE FLAG register error bits

| Bit | Name | Description |
| --- | --- | --- |
| 0–11 | — | Accumulator overflow bits |
| 12 | IR0_SAT | IR0 saturated |
| 13 | SY2_SAT | SXY2 saturated |
| 14 | SZ3_SAT | SZ3 saturated |
| 15 | COLOR_SAT | Color FIFO saturated |
| 16 | MAC0_NEG | MAC0 negative |
| 17 | DIV_ZERO | Division by zero |
| 18–22 | — | Accumulator overflow (MAC1-3) |
| 23 | — | Z-axis overflow |
| 24–30 | — | Accumulator overflow bits |
| 31 | ERROR | Any error flag set |

## DMA channel control (D_CHCR) bitmasks

```c
#define DMA_DIR_RAM_TO_DEV  (0 << 0)
#define DMA_DIR_DEV_TO_RAM  (1 << 0)
#define DMA_STEP_FORWARD    (0 << 8)
#define DMA_STEP_BACKWARD   (1 << 8)
#define DMA_SYNC_MANUAL     (0 << 16)
#define DMA_SYNC_REQUEST    (1 << 16)
#define DMA_SYNC_LINKED     (2 << 16)
#define DMA_START           (1 << 20)
#define DMA_TRIGGER         (1 << 24)
```

## Interrupt bitmasks

```c
#define IRQ_VBLANK   (1 << 0)
#define IRQ_GPU      (1 << 1)
#define IRQ_CDROM    (1 << 2)
#define IRQ_DMA      (1 << 3)
#define IRQ_TIMER0   (1 << 4)
#define IRQ_TIMER1   (1 << 5)
#define IRQ_TIMER2   (1 << 6)
#define IRQ_JOY      (1 << 7)
#define IRQ_SIO      (1 << 8)
#define IRQ_SPU      (1 << 9)
#define IRQ_LIGHTPEN (1 << 10)
```

## SPU volume encoding

| Format | Bits | Range | Description |
| --- | --- | --- | --- |
| Linear | 0–14 | 0–0x3FFF | Direct volume |
| Sweep | 15 | — | Sweep mode (bit 15 set) |
| Sweep phase | 0–14 | — | Sweep envelope |

### SPU ADSR encoding

Attack: rate 0–7F, mode 0 (linear) or 1 (exponential).
Decay: rate 0–0F (fixed exponential).
Sustain: level 0–7F, rate 0–7F, mode 0/1.
Release: rate 0–1F, mode 0 (linear) or 1 (exponential).

## Controller ID bytes

| ID | Description |
| --- | --- |
| `0x41` | Digital pad |
| `0x73` | Analog pad (SCPH-1150) |
| `0x79` | Analog pad (SCPH-1180) |
| `0xF3` | Analog pad (rumble) |
| `0x23` | Analog pad (rumble, alt) |
| `0x53` | NeGcon |
| `0x12` | Mouse |

## Common PSX data types

```c
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef volatile u8        vu8;
typedef volatile u16       vu16;
typedef volatile u32       vu32;
typedef volatile s8        vs8;
typedef volatile s16       vs16;
typedef volatile s32       vs32;
```

## Fixed-point formats

| Format | Bits | Range | Resolution | Use |
| --- | --- | --- | --- | --- |
| 4.12 | 16 | ±7.999 | 1/4096 | SPU pitch |
| 16.16 | 32 | ±32767.999 | 1/65536 | GTE MAC |
| 1.3.12 | 16 | ±3.999 | 1/4096 | GTE IR |
| 24.0 | 32 | 0–16777215 | 1 | GTE OFX/OFY |
| 4.8 | 16 | ±7.996 | 1/256 | Texture UV |

## PsyQ library function categories

| Prefix | Category |
| --- | --- |
| `Gs` | Graphics subsystem |
| `GsSort` | Ordering table insertion |
| `GsDraw` | Drawing primitives |
| `Cd` | CD-ROM access |
| `Spu` | Sound processing unit |
| `Pad` | Controller input |
| `VSync` | Vertical sync |
| `Heap` | Memory allocation |
| `Malloc`/`Free` | Memory management |
| `Open`/`Close`/`Read`/`Write` | File I/O |
