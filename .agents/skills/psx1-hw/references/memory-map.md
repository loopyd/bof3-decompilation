# PSX1 Memory Map

## Full address space

| Range | Size | Segment | Cache | Description |
| --- | --- | --- | --- | --- |
| `0x00000000–0x001FFFFF` | 2 MB | KUSEG0 | Cached | Main RAM (TLB-mapped) |
| `0x00200000–0x003FFFFF` | 2 MB | KUSEG0 | Cached | RAM mirror (aliased) |
| `0x80000000–0x801FFFFF` | 2 MB | KSEG0 | Cached | Main RAM (direct) |
| `0x80200000–0x803FFFFF` | 2 MB | KSEG0 | Cached | RAM mirror |
| `0xA0000000–0xA01FFFFF` | 2 MB | KSEG1 | Uncached | Main RAM (direct) |
| `0xA0200000–0xA03FFFFF` | 2 MB | KSEG1 | Uncached | RAM mirror |
| `0x1F800000–0x1F8003FF` | 1 KB | KSEG1 | — | Scratchpad SRAM |
| `0x1F801000–0x1F802FFF` | ~8 KB | KSEG1 | — | Hardware I/O |
| `0x1FC00000–0x1FC7FFFF` | 512 KB | KSEG1 | Cached | BIOS ROM |

## RAM layout (typical BOF3)

| Range | Typical use |
| --- | --- |
| `0x80010000` | PS-X executable load address (after header) |
| `0x80010000–0x800XXXXX` | Code (.text) |
| `0x800XXXXX–0x800XXXXX` | Read-only data (.rodata) |
| `0x800XXXXX–0x800XXXXX` | Initialized data (.data) |
| `0x800XXXXX–0x801XXXXX` | BSS + heap |
| `0x801FFFF0` | Stack top (grows downward) |

PS-X executables have a 2048-byte header (`PS-X EXE\x00` + metadata).
The `t_addr` field in the header specifies the load address (usually
`0x80010000`). `bin/normalize` strips the header and produces a raw image.

## Scratchpad details

- 1024 bytes at `0x1F800000–0x1F8003FF`
- Zero-wait-state SRAM, private to CPU
- Not accessible by DMA or GPU
- Common use: temporary work area, stack frames, per-overlay state
- BOF3 pattern: `SCRATCH_PTR` at `0x1F800044` points to current overlay's
  scratchpad work area

## Memory segments and caching

| Segment | Address bits | Cache | TLB | Typical use |
| --- | --- | --- | --- | --- |
| KUSEG0 | `0x00000000–0x7FFFFFFF` | Cached | Yes | User-mode RAM |
| KSEG0 | `0x80000000–0x9FFFFFFF` | Cached | No | Kernel-mode RAM |
| KSEG1 | `0xA0000000–0xBFFFFFFF` | Uncached | No | I/O, DMA buffers |
| KSEG2 | `0xC0000000–0xFFFFFFFF` | — | Yes | Kernel only |

PSX games use KSEG0 (`0x80xxxxxx`) for normal code/data and KSEG1
(`0xA0xxxxxx`) for DMA source/destination buffers and hardware registers.

## Bus widths

- CPU bus: 32-bit, little-endian
- RAM: 32-bit, 2 MB (64K × 32-bit words)
- GPU bus: 32-bit
- SPU: 16-bit
- CD-ROM: 8-bit
- Controller/Memory Card: 8-bit serial

## Wait states

| Region | Wait states |
| --- | --- |
| RAM (KSEG0) | 0 (cached) |
| RAM (KSEG1) | 6 cycles |
| Scratchpad | 0 |
| Hardware I/O | 6 cycles |
| BIOS ROM | 6 cycles |
