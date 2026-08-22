# Documentation index

Audience-oriented map of the repository. The project overview and quick start
live in [`README.md`](../README.md); the canonical operating contract for
agents and maintainers is [`AGENTS.md`](../AGENTS.md).

## New contributors

1. [README](../README.md) — project purpose, prerequisites, and quick start.
2. [Contributing](../CONTRIBUTING.md) — contribution and pull-request rules.
3. [Tool usage](usage.md) — the ordered workflow and command reference.
4. [Function matching](agents/matching.md) — the loop for lifting one function.
5. [Project context](agents/project-context.md) — target identity, ownership,
   and repository map.

## Agents and maintainers

[`AGENTS.md`](../AGENTS.md) is the canonical contract for repository work;
[`SOUL.md`](../SOUL.md) is the companion identity document to read with it.
These references are the durable procedures beneath it:

- [Coding standards](agents/CODING_STANDARDS.md) — Python naming, module, CLI, and test rules
- [Function matching](agents/matching.md) — C iteration rules, acceptance gates, duplicate promotion
- [Matching playbook](agents/matching-playbook.md) — first-diff symptom-to-lever guide
- [Memory API](agents/memory-api.md) — fixed-RAM, scratchpad, and sanctioned matching helpers
- [Plan authoring](agents/plan-authoring.md) — scoped, evidence-backed implementation plans
- [Lessons](agents/lessons.md) — durable cross-target reverse-engineering gotchas

## Researchers

### Analysis and discovery

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
- [Compiler provenance](specs/runtime/compiler-provenance.md) — historical compiler and delay-slot matching evidence
- [Compiler quirks](specs/runtime/compiler-quirks.md) — register-allocation and delay-slot scheduling residuals
- [Compiler variants](specs/runtime/compiler-variants.md) — historical GCC variant catalog and negative probe records
- [Battle range predicates](specs/runtime/battle-range-predicates.md) — register-level battle/15 range-predicate evidence

### Data

- [Data index](specs/data/index.md) — recorded archive offsets, record layouts,
  and pointer maps (historical vast-violence catalog provenance; no tracked
  byte-verifier)
- [IDs and encodings](specs/data/ids.md) — namespaces, masks, packed values, and sentinels
- [Encoding and formulas](specs/data/encoding.md) — bitmask values, name encoding, stat packing
- [Equipment and shops](specs/data/equipment.md) — items, weapons, armor, accessories, and level growth
- [Characters and masters](specs/data/characters.md) — base stats, master skills, and master names
- [Per-area data](specs/data/areas.md) — monsters, formations, chests, genes, and chrysms
- [Fairy rewards](specs/data/fairies.md) — fairy-gift, exploration, and prize records
- [Schema ledger](specs/data/schema-ledger.md) — evidence status for every documented record family

### Formats

- [EMI container](specs/formats/emi.md) — EMI container and entry layout
- [Audio formats and runtime](specs/formats/audio.md) — XA, VAB, SEP, PSF1, and SPU tooling
- [STR playback](specs/formats/str-xa.md) — extracted STR and XA sector representation
- [Graphics](specs/formats/graphics.md) — type-3 VRAM upload and indexed-palette layout
- [Format conversion](specs/formats/conversion.md) — lossless interchange, derivatives, and provenance

### External reference

- [BoF3 EU (bof3js) RE reference](reference/bof3-eu/README.md) — imported
  EU-release knowledge split by subject: engine, disc/EMI, graphics, world,
  battle, game rules, audio, method, and EU address register. Addresses are
  EU address space — leads and format contracts, not reviewed US facts. The
  EU text is read-only; verified US annotations are append-only.

## Header layout

`include/base/` and `include/memory/` are the canonical source of truth for
fixed RAM addresses, hardware registers, and scratchpad. The aggregate and SDK
contracts under `include/bof3/` remain active public headers.

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

## Plans

- [Project symbol naming cleanup](plans/project-symbol-naming-cleanup.md) —
  active: 1,361 target-qualified raw map rows remain evidence-gated across 23
  targets; 22 target reports still contain blocked initializer rows.

Completed implementation history is preserved in `git log`.

## Community

- [Contributing](../CONTRIBUTING.md) — contribution and pull-request rules.
- [Code of Conduct](../CODE_OF_CONDUCT.md) — expected behavior and enforcement.

## History

- [Changelog](../CHANGELOG.md) — release-history snapshot ending 2026-07-31;
  `git log` is authoritative for complete current history.
- [Credits](../CREDITS.md) — original game, project origin, contributors, and
  third-party software.
