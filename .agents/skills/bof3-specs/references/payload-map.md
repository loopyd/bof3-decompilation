# Payload map

Use this as a classifier, then read the linked canonical spec before promoting
a claim.

## Shipped and runtime objects

| Kind | Known role/location | Boundary |
| --- | --- | --- |
| `SLUS_004.22` | main executable; disc, EMI, loader, and shared services | independent PS-X EXE; header `t_addr` is authoritative |
| `LOGO/LOGO.EXE` | logo executable | independent PS-X EXE loaded at `0x801ce000` |
| EMI archive | sector-aligned entry container | never executable input |
| EMI type `0` | generic RAM payload | code, data, palette, or mixed; review required |
| EMI types `1`, `2` | queued RAM payloads | load argument participates in loader state |
| EMI type `3` | raw VRAM upload bytes | load argument is a packed graphics descriptor |
| EMI types `4`, `5` | shared special path | semantics unresolved |
| EMI types `6`, `7` | VAB header/body | audio resources |
| EMI type `8` | auxiliary audio | audio resource |
| EMI types `9`, `10` | sequence-side payloads; type `10` is SEQ | logical selector, not a CPU pointer |
| STR/XA | streamed movie/audio sectors | media, not EMI code |

EMI is BOF3's custom archive/container system. Do not infer its entry layout
from standard PlayStation TIM/PXL/CLT images, VAG/VAB audio, or STR/XA sector
formats. Identify an official format by its own header and consumer contract;
an EMI entry may contain, wrap, or feed similarly represented data without
being that file format.

Common reused runtime regions include `0x801d0c00` for frontend/game/battle
overlays, `0x801eec00` for battle/effect overlays, `0x80104000` and
`0x801f2c00` for world/area code-data, and `0x80033xxx`–`0x8003axxx` for small
graphics/palette buffers. A region never identifies the current subsystem by
itself.

## Candidate versus reviewed evidence

- Instruction density, aligned MIPS words, strings, type `0`, or a RAM address
  rank code candidates. Require coherent bounded control flow, calls, delay
  slots, returns, and reviewed mixed data before promotion.
- Palette size/alignment/address filters rank CLUT candidates. Require loader
  destination, palette-bank placement, runtime CLUT consumer, and reviewed
  indexed render before associating a texture and palette.
- Static zero tables may be runtime-populated. Inspect writers and indirect
  consumers before declaring them absent.
- Observed struct offsets, access widths, and signedness are separate from field
  semantics and C match status.

Canonical details: `docs/specs/formats/emi.md`,
`docs/specs/runtime/runtime-layout.md`, `docs/specs/runtime/emi-loader.md`, and
`docs/specs/formats/graphics.md`.

External format authority: [psx-spx CD-ROM file/video/audio formats](https://psx-spx.consoledev.net/cdromfileformats/)
and [psx-spx GPU rendering and VRAM](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/).
