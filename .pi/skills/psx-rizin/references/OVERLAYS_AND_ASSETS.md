# Overlays, modules, and asset containers

## Separate identities

PS1 RAM is constrained → games load code/data modules into reused address ranges. Two files can both execute at `0x80180000` at different times with no semantic relationship.

Identity key:

```text
<source-path>#<sha256>@<load-base>:<load-size>
```

Never key only by address.

## Discovery

Static: inspect disc dir + generic binaries · locate loader calls + CD sector/file read tables · search destination RAM addresses/sizes · identify decompression loops/signatures · locate entrypoint/callback tables · compare pointer density + MIPS call/jump density · inspect archive indices/filenames.

Dynamic: break on CD read APIs/BIOS paths · write-break destination ranges · log DMA/decompression output · execute-break candidate ranges · dump RAM immediately after load/relocation · correlate with frame/level/replay state.

## Overlay ledger

| field | meaning |
|---|---|
| id | stable case-local identity |
| source | disc path/archive member/LBA |
| hash | source bytes |
| stored size | compressed/on-disc size |
| runtime size | decompressed/copied size |
| load base | proven destination |
| entrypoints | direct/registered/dispatch targets |
| loader | function + call site |
| fixups | relocation or pointer patch behavior |
| lifetime | load/unload/replace conditions |
| replay coverage | scenarios loading/executing it |
| runtime dump hash | post-load bytes |

## Runtime dump comparison

Source vs runtime bytes:

- identical → likely raw copy (mutable data may change later)
- expanded → compression/packing
- sparse differences → relocations/fixups
- large prefix match + appended region → BSS/work area
- code differences after execution → self-modifying code, cache/patch behavior, or incorrect capture

Keep source + runtime analysis projects separate when bytes differ materially.

## Entrypoint tables / callback registration

Entrypoints may be: fixed first instruction · header field · init/update/render/destroy callback table · state-machine table · function pointers copied into a global manager · script/native opcode handlers. Trace writes to callback tables + subsequent `jalr` targets; record manual xrefs from each indirect call site to each validated target.

## Relocations

Look for: offset tables within the loaded image · `lui` + low-half immediate patches · absolute word pointer patches · code/data base additions · per-overlay GP setup · cache flush calls after code writes. Represent a runtime pointer into an overlay as both absolute address and overlay-relative offset:

```text
relative = runtime - overlay_load_base
```

## Asset formats driving control flow

Even code-focused tasks: inventory formats that drive control flow — TIM textures/palettes, TMD/HMD models, VAB/VAG + XA audio, STR/MDEC video, memory-card saves/replays, script/bytecode, level/room archives, animation/collision data. A script opcode table can explain indirect call fan-out; a replay/save can expose otherwise unreachable paths.

## Bulk overlay workflow

1. Hash + classify all candidate files.
2. Raw MIPS scan (no base) for density statistics.
3. Obtain load bases from loader/runtime evidence.
4. Re-scan with bases to decode jump/call candidates.
5. Extract/decompress with a documented tool/script.
6. Open each identity separately in Rizin.
7. Export function/string/xref inventories.
8. Compare runtime dumps.
9. Link overlay entries to replay scenarios.
