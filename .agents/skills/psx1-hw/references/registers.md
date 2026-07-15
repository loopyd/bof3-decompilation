# PSX1 Hardware Registers (0x1F801000–0x1F802FFF)

All registers are little-endian. Write-only bits read as zero unless noted.
Use `REG8()`, `REG16()`, `REG32()` from `include/bof3/defines.h`.

## Interrupt Control (0x1F801070–0x1F801074)

| Address | Bits | R/W | Name | Description |
| --- | --- | --- | --- | --- |
| `0x1F801070` | 0–10 | R/W | I_STAT | Interrupt status (write 0 to ack) |
| `0x1F801074` | 0–10 | R/W | I_MASK | Interrupt mask (1=enabled) |

Interrupt bits:
- 0: VBlank (60 Hz NTSC, 50 Hz PAL)
- 1: GPU (drawing complete)
- 2: CD-ROM
- 3: DMA (channel complete)
- 4: Timer 0 (root counter 0)
- 5: Timer 1 (root counter 1)
- 6: Timer 2 (root counter 2)
- 7: Controller/Memory Card (SIO)
- 8: SIO (serial)
- 9: SPU
- 10: Lightpen

To acknowledge an interrupt, write the bit to I_STAT (write 0 clears, write 1
sets). To clear a specific bit: `I_STAT &= ~bit`.

## DMA Control (0x1F801080–0x1F8010FF)

### DMA Channel Registers (each channel: +0x00, +0x04, +0x08)

| Offset | Bits | R/W | Name | Description |
| --- | --- | --- | --- | --- |
| `+0x00` | 0–23 | R/W | D_MADR | DMA base address (word-aligned) |
| `+0x04` | 0–31 | R/W | D_BCR | Block control (word count / block size) |
| `+0x08` | 0–31 | R/W | D_CHCR | Channel control |

### D_BCR format

| Sync mode | BCR bits | Description |
| --- | --- | --- |
| Manual | 0–15 | Word count (0 = max 0x10000) |
| Request | 0–15: block size, 16–31: block count | Block transfer |
| Linked list | 0–31: unused | — |

### D_CHCR bit layout

| Bits | Name | Description |
| --- | --- | --- |
| 0 | DIR | Transfer direction (0=RAM→Device, 1=Device→RAM) |
| 1–7 | — | Reserved |
| 8 | STEP | Address step (0=+4, 1=-4) |
| 9–15 | — | Reserved |
| 16–17 | SYNC | Sync mode (0=Manual, 1=Request, 2=Linked list, 3=Reserved) |
| 18–19 | — | Reserved |
| 20 | START | Start/Busy (write 1 to start, reads 1 while busy) |
| 21 | — | Reserved |
| 24 | TRIGGER | Manual trigger (for sync mode 0) |
| 28 | — | Unknown |
| 29 | — | Unknown |
| 30 | — | Unknown |
| 31 | — | Unknown |

### DMA Channels

| Ch | Base | Device | Typical use |
| --- | --- | --- | --- |
| 0 | `0x1F801080` | MDECin | Macroblock decode input |
| 1 | `0x1F801090` | MDECout | Macroblock decode output |
| 2 | `0x1F8010A0` | GPU | Ordered list / linked list |
| 3 | `0x1F8010B0` | CD-ROM | CD-ROM data transfer |
| 4 | `0x1F8010C0` | SPU | ADPCM streaming |
| 5 | `0x1F8010D0` | PIO | External (lightpen, etc.) |
| 6 | `0x1F8010E0` | GPU | Reverse clear (OT) |

### DMA Control Register (0x1F8010F0)

| Bits | Name | Description |
| --- | --- | --- |
| 0–3 | — | Unknown |
| 4 | D0_CHCR | Channel 0 enable |
| 5 | D1_CHCR | Channel 1 enable |
| 6 | D2_CHCR | Channel 2 enable |
| 7 | D3_CHCR | Channel 3 enable |
| 8 | D4_CHCR | Channel 4 enable |
| 9 | D5_CHCR | Channel 5 enable |
| 10 | D6_CHCR | Channel 6 enable |
| 11–15 | — | Unknown |
| 16–19 | — | Unknown |
| 20 | D0_CHCR | Channel 0 priority |
| 21 | D1_CHCR | Channel 1 priority |
| 22 | D2_CHCR | Channel 2 priority |
| 23 | D3_CHCR | Channel 3 priority |
| 24 | D4_CHCR | Channel 4 priority |
| 25 | D5_CHCR | Channel 5 priority |
| 26 | D6_CHCR | Channel 6 priority |
| 27–31 | — | Unknown |

### DMA Interrupt Register (0x1F8010F4)

| Bits | R/W | Name | Description |
| --- | --- | --- | --- |
| 0–5 | R | — | Unknown |
| 6 | R | D6_CHCR | Channel 6 interrupt flag |
| 7 | R | D5_CHCR | Channel 5 interrupt flag |
| 8 | R | D4_CHCR | Channel 4 interrupt flag |
| 9 | R | D3_CHCR | Channel 3 interrupt flag |
| 10 | R | D2_CHCR | Channel 2 interrupt flag |
| 11 | R | D1_CHCR | Channel 1 interrupt flag |
| 12 | R | D0_CHCR | Channel 0 interrupt flag |
| 13–15 | — | — | Unknown |
| 16 | R/W | D6_CHCR | Channel 6 interrupt enable |
| 17 | R/W | D5_CHCR | Channel 5 interrupt enable |
| 18 | R/W | D4_CHCR | Channel 4 interrupt enable |
| 19 | R/W | D3_CHCR | Channel 3 interrupt enable |
| 20 | R/W | D2_CHCR | Channel 2 interrupt enable |
| 21 | R/W | D1_CHCR | Channel 1 interrupt enable |
| 22 | R/W | D0_CHCR | Channel 0 interrupt enable |
| 23 | R/W | — | Unknown |
| 24 | R/W | — | Force DMA interrupt |
| 25 | R/W | — | Unknown |
| 26 | R/W | — | Unknown |
| 27 | R/W | — | Unknown |
| 28 | R/W | — | Unknown |
| 29 | R/W | — | Unknown |
| 30 | R/W | — | Unknown |
| 31 | R/W | — | Master interrupt enable |

## GPU (0x1F801810–0x1F801814)

| Address | R/W | Name | Description |
| --- | --- | --- | --- |
| `0x1F801810` | W | GP0 | GP0 command register |
| `0x1F801810` | R | GPUREAD | GPU data read |
| `0x1F801814` | W | GP1 | GP1 command register |
| `0x1F801814` | R | GPUSTAT | GPU status register |

### GP0 commands (drawing)

| Command | Bits | Description |
| --- | --- | --- |
| `0x00` | — | NOP |
| `0x01` | — | Clear cache |
| `0x02` | — | Fill rectangle |
| `0x20–0x23` | — | Monochrome triangle |
| `0x24–0x27` | — | Textured triangle |
| `0x28–0x2B` | — | Monochrome quad |
| `0x2C–0x2F` | — | Textured quad |
| `0x30–0x33` | — | Shaded triangle |
| `0x34–0x37` | — | Shaded textured triangle |
| `0x38–0x3B` | — | Shaded quad |
| `0x3C–0x3F` | — | Shaded textured quad |
| `0x40–0x47` | — | Monochrome line |
| `0x48–0x4F` | — | Monochrome polyline |
| `0x50–0x57` | — | Shaded line |
| `0x58–0x5F` | — | Shaded polyline |
| `0x60–0x63` | — | Monochrome rectangle |
| `0x64–0x67` | — | Textured rectangle |
| `0x68–0x6B` | — | Dot (1×1 rectangle) |
| `0x70–0x77` | — | 8×8 rectangle |
| `0x78–0x7F` | — | 16×16 rectangle |
| `0x80` | — | VRAM→VRAM copy |
| `0xA0` | — | CPU→VRAM transfer |
| `0xC0` | — | VRAM→CPU transfer |
| `0xE1` | — | Draw mode (texture page, semi-transparency) |
| `0xE2` | — | Texture window |
| `0xE3` | — | Draw area top-left |
| `0xE4` | — | Draw area bottom-right |
| `0xE5` | — | Draw offset |
| `0xE6` | — | Draw mode (mask bits) |

### GPUSTAT bits

| Bit | Name | Description |
| --- | --- | --- |
| 0–3 | TXP | Texture page X base |
| 4 | TXP | Texture page Y base |
| 5–6 | ABE | Semi-transparency mode |
| 7–8 | TP | Texture page color mode |
| 9 | ABE | Draw to display area |
| 10 | — | Set mask bit |
| 11 | — | Check mask bit |
| 12–15 | — | Reserved |
| 16 | — | Interlace field |
| 17 | — | Reverse flag |
| 18 | — | Texture disable |
| 19 | — | Drawing active |
| 20 | — | Ready to receive command |
| 21 | — | Ready to send VRAM to CPU |
| 22 | — | Ready to receive DMA block |
| 23 | — | DMA direction |
| 25–26 | — | Height (240/480) |
| 27 | — | Video mode (NTSC/PAL) |
| 28 | — | Display area color depth |
| 29 | — | Vertical interlace |
| 30 | — | Horizontal resolution |
| 31 | — | Horizontal resolution (bit 2) |

## Timers (0x1F801100–0x1F80112F)

### Timer registers (each timer: +0x00, +0x04, +0x08)

| Offset | Bits | R/W | Name | Description |
| --- | --- | --- | --- | --- |
| `+0x00` | 0–15 | R/W | TIMER_VALUE | Current counter value |
| `+0x04` | 0–31 | R/W | TIMER_MODE | Mode bits |
| `+0x08` | 0–15 | R/W | TIMER_TARGET | Compare target value |

### TIMER_MODE bits

| Bit | Name | Description |
| --- | --- | --- |
| 0–1 | SYNC | Sync mode (0=free run, 1=sync, 2=sync, 3=sync) |
| 2 | — | Sync mode bit 1 |
| 3 | — | Reset counter on target (0=no, 1=yes) |
| 4 | — | IRQ on target |
| 5 | — | IRQ on 0xFFFF |
| 6 | — | IRQ repeat (0=one-shot, 1=repeat) |
| 7 | — | IRQ toggle (0=pulse, 1=toggle) |
| 8–9 | CLK | Clock source |
| 10 | — | Interrupt request (0=active) |
| 11 | — | Reached target |
| 12 | — | Reached 0xFFFF |

### Clock sources

| Timer | CLK=0 | CLK=1 | CLK=2 | CLK=3 |
| --- | --- | --- | --- | --- |
| 0 | System clock | Dotclock | System clock | Dotclock |
| 1 | System clock | Hblank | System clock | Hblank |
| 2 | System clock | System/8 | System clock | System/8 |

## SPU (0x1F801C00–0x1F801FFF)

### Voice registers (24 voices, base + voice × 0x10)

| Offset | Bits | R/W | Name | Description |
| --- | --- | --- | --- | --- |
| `+0x00` | 0–15 | R/W | VOL_L | Left volume |
| `+0x02` | 0–15 | R/W | VOL_R | Right volume |
| `+0x04` | 0–15 | R/W | PITCH | Pitch (4.12 fixed, 0x1000=1x) |
| `+0x06` | 0–15 | R/W | STARTADDR | ADPCM start address |
| `+0x08` | 0–31 | R/W | ADSR_LO | Attack/Decay/Sustain/Release low |
| `+0x0C` | 0–15 | R/W | ADSR_HI | Sustain level/rate high |
| `+0x0E` | 0–15 | R | ADSR_VOL | Current ADSR volume |
| `+0x10` | 0–15 | R/W | REPEATADDR | Loop repeat address |

### Main SPU registers (0x1F801D80+)

| Offset | Bits | R/W | Name | Description |
| --- | --- | --- | --- | --- |
| `+0x00` | 0–15 | R/W | MVOL_L | Master volume left |
| `+0x02` | 0–15 | R/W | MVOL_R | Master volume right |
| `+0x04` | 0–15 | R/W | RVOL_L | Reverb volume left |
| `+0x06` | 0–15 | R/W | RVOL_R | Reverb volume right |
| `+0x08` | 0–23 | R/W | KON | Key on (write 1 to start voice) |
| `+0x0C` | 0–23 | R/W | KOFF | Key off (write 1 to stop voice) |
| `+0x10` | 0–23 | R/W | PMON | Pitch modulation enable |
| `+0x14` | 0–23 | R/W | NON | Noise enable |
| `+0x18` | 0–23 | R/W | EON | Reverb enable |
| `+0x1C` | 0–23 | R | ENDX | Voice end flags |
| `+0x20` | 0–15 | R/W | STAT_IRQ | Voice IRQ status |
| `+0x22` | 0–15 | R/W | IRQ_ADDR | IRQ address |

### ADPCM encoding

- 4-bit ADPCM with 16-byte blocks
- Each block: 1 header byte + 14 bytes (28 samples)
- Header byte: shift (0–12) + filter (0–4)
- Loop flag in header byte bit 0
- End flag in header byte bit 1

## CD-ROM (0x1F801800–0x1F801803)

| Address | R/W | Name | Description |
| --- | --- | --- | --- |
| `0x1F801800` | R/W | CD_STAT | Status register |
| `0x1F801801` | R | CD_DATA | Response FIFO |
| `0x1F801801` | W | CD_CMD | Command register |
| `0x1F801802` | R | CD_DATA | Data FIFO |
| `0x1F801802` | W | CD_IRQM | Interrupt mask |
| `0x1F801803` | R | CD_IRQF | Interrupt flags |
| `0x1F801803` | W | CD_IRQF | Interrupt acknowledge |

### CD-ROM commands

| Command | Description |
| --- | --- |
| `0x01` | Getstat |
| `0x02` | Setloc (minute, second, sector) |
| `0x06` | ReadN |
| `0x09` | Pause |
| `0x0A` | Init |
| `0x0E` | Setmode |
| `0x15` | SeekL |
| `0x1A` | GetID |
| `0x1E` | ReadTOC |

## Controller/Memory Card (0x1F801040–0x1F80104F)

| Address | R/W | Name | Description |
| --- | --- | --- | --- |
| `0x1F801040` | R/W | JOY_TX_DATA | TX data (write to send) |
| `0x1F801040` | R | JOY_RX_DATA | RX data (read received) |
| `0x1F801044` | R/W | JOY_STAT | Status register |
| `0x1F801048` | R/W | JOY_MODE | Mode (baudrate, parity) |
| `0x1F80104A` | R/W | JOY_CTRL | Control (TX/RX enable, etc.) |
| `0x1F80104E` | R/W | JOY_BAUD | Baudrate divisor |

### JOY_STAT bits

| Bit | Name | Description |
| --- | --- | --- |
| 0 | TX_RDY | TX ready (1=ready to send) |
| 1 | RX_FIFO | RX FIFO not empty |
| 2 | TX_RDY2 | TX ready (1=ready for next byte) |
| 7 | /ACK | Acknowledge (active low) |
| 9 | IRQ | Interrupt flag |

### Controller button bits (after ID bytes)

| Bit | Button |
| --- | --- |
| 0 | Select |
| 1 | L3 (stick press) |
| 2 | R3 (stick press) |
| 3 | Start |
| 4 | D-pad Up |
| 5 | D-pad Right |
| 6 | D-pad Down |
| 7 | D-pad Left |
| 8 | L2 |
| 9 | R2 |
| 10 | L1 |
| 11 | R1 |
| 12 | Triangle |
| 13 | Circle |
| 14 | Cross |
| 15 | Square |

## MDEC (Motion Decoder)

| Address | R/W | Name | Description |
| --- | --- | --- | --- |
| `0x1F801820` | R/W | MDEC_CMD | Command register |
| `0x1F801824` | R/W | MDEC_STAT | Status register |

Commands: `0x01` decode macroblock, `0x02` set quant table,
`0x03` set scale table, `0x40` set decomposition.

## SIO (Serial I/O)

| Address | R/W | Name | Description |
| --- | --- | --- | --- |
| `0x1F801050` | R/W | SIO_TX_DATA | TX data |
| `0x1F801054` | R/W | SIO_STAT | Status register |
| `0x1F801058` | R/W | SIO_MODE | Mode register |
| `0x1F80105A` | R/W | SIO_CTRL | Control register |
| `0x1F80105E` | R/W | SIO_BAUD | Baudrate divisor |
