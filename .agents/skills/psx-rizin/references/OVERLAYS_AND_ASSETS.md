# Overlays, modules, and asset containers

## Why overlays require separate identities

PS1 RAM is constrained, so games commonly load code/data modules into reused address ranges. Two files can both execute at `0x80180000` at different times and have no semantic relationship.

Use an overlay identity key such as:

```text
<source-path>#<sha256>@<load-base>:<load-size>
```

Do not key only by address.

## Discovery methods

### Static

- inspect disc directory and generic binary files
- locate loader calls and CD sector/file read tables
- search for destination RAM addresses and sizes
- identify decompression loops and signatures
- locate entrypoint/callback tables
- compare pointer density and MIPS call/jump candidate density
- inspect archive indices and filenames

### Dynamic

- break on CD read APIs/BIOS paths
- write-break destination ranges
- log DMA/decompression output
- execute-break candidate range
- dump RAM immediately after load/relocation
- correlate with frame/level/replay state

## Overlay ledger

For each overlay record:

| field | meaning |
|---|---|
| id | stable case-local identity |
| source | disc path/archive member/LBA |
| hash | source bytes |
| stored size | compressed/on-disc size |
| runtime size | decompressed/copied size |
| load base | proven destination |
| entrypoints | direct/registered/dispatch targets |
| loader | function and call site |
| fixups | relocation or pointer patch behavior |
| lifetime | load/unload/replace conditions |
| replay coverage | scenarios loading/executing it |
| runtime dump hash | post-load bytes |

## Runtime dump comparison

Compare source and runtime bytes:

- identical: likely raw copy, though mutable data may change later
- expanded: compression/packing
- sparse differences: relocations/fixups
- large prefix match plus appended region: BSS/work area
- code differences after execution: self-modifying code, cache/patch behavior, or incorrect capture

Keep source and runtime analysis projects separate when bytes differ materially.

## Entrypoint tables and callback registration

Overlay entrypoints may be:

- fixed first instruction
- header field
- table of init/update/render/destroy callbacks
- state-machine table
- function pointers copied into a global manager
- script/native opcode handlers

Trace writes to callback tables and subsequent `jalr` targets. Record manual xrefs from each indirect call site to each validated target.

## Relocations

Look for:

- tables of offsets within the loaded image
- high/low immediate patches (`lui` plus low half)
- absolute word pointer patches
- code/data base additions
- GP setup per overlay
- cache flush calls after code writes

A runtime pointer into an overlay should be represented as both absolute address and overlay-relative offset:

```text
relative = runtime - overlay_load_base
```

This allows cross-load-base and revision comparison.

## Asset formats relevant to code analysis

Even when the task is code-focused, inventory formats that drive control flow:

- TIM textures and palettes
- TMD/HMD/model data
- VAB/VAG and XA audio
- STR/MDEC video
- memory-card saves/replays
- script/bytecode files
- level/room archives
- animation and collision data

A script opcode table can explain indirect call fan-out; a replay/save can expose otherwise unreachable paths.

## Bulk overlay workflow

1. Hash and classify all candidate files.
2. Run `scripts/scan_mips.py` with no base first for density statistics.
3. Obtain load bases from loader/runtime evidence.
4. Re-run scans with bases to decode jump/call candidates.
5. Extract/decompress with a documented tool/script.
6. Open each identity separately in Rizin.
7. Export function/string/xref inventories.
8. Compare runtime dumps.
9. Link overlay entries to replay scenarios.
