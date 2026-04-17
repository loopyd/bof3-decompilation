# Source Summary: PSX / PsyQ Manuals

Source site:

- `https://psx.arthus.net/docs/`
- highest-value material is under `https://psx.arthus.net/sdk/Psy-Q/DOCS/`

## What This Source Is

This site is a mirror of official PlayStation and PsyQ documentation plus a few community-maintained hardware references.

For this repo, the durable value is platform-level reference material, not BOF3-specific reverse-engineering notes.

## Highest-Value References

- `LibRef47.pdf`
  - PsyQ library reference
- `LibOver47.pdf`
  - runtime overview and child-process or overlay semantics
- `FileFormat47.pdf`
  - TIM, PXL, CLT, VAB, SEQ, and related file formats
- `Devrefs/os.pdf`
  - low-level OS and executable/runtime structures
- `Devrefs/Hardware.pdf`
  - memory map, GPU/SPU/CDROM registers, DMA, and VRAM behavior
- `Devrefs/Cdgen.pdf`
  - ISO9660 and disc-layout generation details
- `XATUT.pdf`
  - XA streaming behavior
- `Devrefs/Sound20.pdf`
  - PsyQ sound formats and sequencing details
- `Devrefs/sdevtc.pdf`
  - debugger and target-console workflow reference
- `Devrefs/Dtlh2500.pdf`
  - dev hardware and executable-loading reference
- `TECHNOTE/PSXCONS.PDF`
  - console loader and executable-format notes
- `https://psx.arthus.net/docs/PSX.pdf`
  - broad hardware reference; useful as secondary corroboration

## Durable Takeaways For BOF3

### Executable Loading And Overlay Model

The PsyQ manuals give the best platform-level comparison point for BOF3 overlay behavior:

- `Load()`, `Exec()`, and `LoadExec()` define the standard PS-X EXE load-and-run model
- the runtime overview describes child-process execution from a resident module
- process switching requires careful callback, event, and runtime-state handling

This does not prove BOF3 EMI code entries are raw PS-X EXEs, but it is the right baseline for comparing:

- `SLUS_004.22` loader behavior
- overlay activation
- module replacement and shared RAM regions

### CD, ISO9660, And XA

The manuals confirm several durable platform rules relevant to BOF3:

- ISO9660 lookup and directory traversal are the standard file-access model
- compile-time file-location tables are recommended for performance
- XA playback uses sub-header filtering and channel or file selection
- CD initialization can affect audio routing state

For BOF3 this matters because the current local reverse spec already shows:

- a slot-to-LBA table in `SLUS_004.22`
- direct disc-position-based EMI loading
- direct `.STR` entries in the same slot map

### GPU, VRAM, TIM, PXL, And CLUT

This source is especially strong for graphics-side reverse engineering.

The manuals confirm:

- TIM, PXL, and CLT are little-endian PSX-native image formats
- CLUTs are VRAM rectangles, not abstract palette objects
- 4-bit and 8-bit indexed image layouts have fixed width and packing rules
- VRAM layout, texture page selection, and CLUT selection are hardware-shaped concepts
- `LoadImage()` and related GPU upload functions are asynchronous enough to require explicit sync in some flows

This is the best baseline for BOF3’s headerless image and palette payloads inside EMI entries.

### SPU, VAB, SEQ, And Audio Runtime

The manuals confirm the core audio model:

- 24-voice SPU with dedicated sound RAM
- VAB header/body split
- sequence and separation formats
- runtime initialization and hot-reset differences between full init and child-process-safe init paths

That is directly useful for BOF3 because the local reverse spec already ties EMI entry types to:

- `SsVabOpenHeadSticky`
- `SsVabClose`
- SPU transfer behavior
- sequence-bearing payloads

### Debugger And Dev-Kit Semantics

The debugger and dev-console manuals are mostly useful for format and workflow semantics:

- what `PS-X EXE`, `CPE`, and related dev formats mean
- how symbols and downloads were expected to work
- how source, disassembly, and runtime downloads fit together in the original toolchain

These are useful for interpretation, not as a direct workflow recommendation for this repo.

## What Must Still Be Verified Locally

- whether BOF3 code-bearing EMI entries use the same runtime assumptions as standard PsyQ child-process overlays
- whether any BOF3 loader step goes beyond standard `Load()`-style copy behavior
- which BOF3 image payload families map cleanly to TIM/PXL/CLUT semantics and which are game-specific wrappers
- how BOF3 preserves or resets audio state across module switches in practice

## Low-Value Sections For This Repo

These parts of the mirrored docs are lower value for ongoing BOF3 RE:

- installer or host-PC setup walkthroughs
- old Windows 95 or target-manager UI steps
- hardware troubleshooting that does not change runtime semantics

## Current Repo Use

Use this source for:

- naming platform behaviors correctly
- checking BOF3 against official PSX or PsyQ rules
- avoiding false assumptions about GPU, SPU, CDROM, and executable semantics

Do not use it as proof of BOF3-specific behavior without local verification in:

- `SLUS_004.22`
- extracted EMI payloads
- emulator or debugger traces
