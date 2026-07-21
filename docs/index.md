# Documentation index

## Getting started

- [README](../README.md) — project overview and quick path
- [AGENTS.md](../AGENTS.md) — agent rules and boundaries
- [CONTEXT.md](../CONTEXT.md) — identity model and repository map
- [Tool usage](usage.md) — ordered workflow and command reference

## Matching

- [Matching loop](matching.md) — C iteration rules and acceptance gates
- [Matching playbook](matching-playbook.md) — symptom-to-lever table for asm-diff
- [Memory API](memory-api.md) — PSX_PTR/PSX_REF/SPAD macro reference

## Specs

- [Module map](specs/targets.md) — executable and overlay load addresses
- [Data discovery](specs/methods.md) — pointer maps and table extraction methods
- [Verified pseudocode](specs/pseudocode.md) — source-backed runtime and extraction algorithms
- [Archive families](specs/archives.md) — EMI archive roles and entry lists

### Runtime

- [Runtime layout](specs/runtime/runtime-layout.md) — executable, overlay, and load-region boundaries
- [Frontend flow](specs/runtime/frontend.md) — title, menu, and attract-path transitions
- [EMI loader](specs/runtime/emi-loader.md) — SLUS EMI entry dispatch and loading rules
- [Recovered layouts](specs/runtime/memory-layouts.md) — byte layouts required by current lifted code
- [Psy-Q constants](specs/runtime/psyq-constants.md) — SDK-backed constants, ABI declarations, and layout rules

### Data

- [Data index](specs/data/index.md) — verified archive offsets, record layouts, and pointer maps
- [IDs and encodings](specs/data/ids.md) — namespaces, masks, packed values, and sentinels
- [Encoding and formulas](specs/data/encoding.md) — bitmask values, name encoding, stat packing
- [Equipment and shops](specs/data/equipment.md) — items, weapons, armor, accessories, and level growth
- [Characters and masters](specs/data/characters.md) — base stats, master skills, and master names
- [Per-area data](specs/data/areas.md) — monsters, formations, chests, genes, and chrysms
- [Fairy rewards](specs/data/fairies.md) — fairy-gift, exploration, and prize records
- [Schema ledger](specs/data/schema-ledger.md) — evidence status for every documented record family

### Formats

- [EMI container](specs/formats/emi.md) — EMI container and entry layout
- [STR playback](specs/formats/str-xa.md) — extracted STR and XA sector representation
- [Graphics](specs/formats/graphics.md) — type-3 VRAM upload and indexed-palette layout
- [Format conversion](specs/formats/conversion.md) — lossless interchange, derivatives, and provenance

## Lessons

- [LESSONS.md](../LESSONS.md) — cross-cutting gotchas for the lift-and-match loop

## Header layout

Subsystem headers under `include/` are the single source of truth for fixed RAM
addresses, hardware registers, and scratchpad. Legacy aliases live in
`include/bof3/`.

| Directory | Domain |
| --- | --- |
| `include/base/` | Common types and barrier helpers |
| `include/memory/` | PSX_PTR/PSX_REF address macros |
| `include/gpu/` | GPU primitives, palette, VRAM upload |
| `include/frontend/` | Title/menu state and selection FX |
| `include/callback/` | VSync and engine callbacks |
| `include/loader/` | EMI archive, disc LBA, slot table |
| `include/panel/` | PanelTask struct and root pointer |
| `include/battle/` | Battle RAM layout, AbilityObject |
| `include/data/` | Game record structs, encoding constants |
| `include/media/` | STR/XA sector layout |
| `include/ui/` | Panel-task animation macros (shared templates) |
| `include/game/` | Counter-step and workarea macros (shared templates) |
