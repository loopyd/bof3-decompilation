# Reverse Specs Index

Use this as the entry point for repo-owned BOF3 reverse-engineering knowledge.

This tree should answer three questions:

1. How does the engine/source code work?
2. What is each shipped module responsible for?
3. What content does each archive contain?

Read first:

1. `docs/specs/status.md`
2. `docs/specs/glossary.md`
3. `docs/specs/runtime/runtime-layout.md`
4. `docs/specs/runtime/module-map.md`
5. `docs/specs/formats/emi.md`

Read by goal:

- Understand engine/source code:
  - `runtime/runtime-layout.md`
  - `runtime/emi-loader.md`
  - `runtime/asset-loading.md`
  - `runtime/boot-sequence.md`
- Understand responsibility per module:
  - `runtime/module-map.md`
  - `runtime/logo-boot.md`
  - `runtime/game-overlay.md`
  - `runtime/battle-overlay.md`
  - `runtime/scena16-overlay.md`
- Understand what content each archive has:
  - `content/asset-families.md`
  - `content/status-emi.md`
  - `formats/emi.md`
  - `formats/emi-graphics-payloads.md`
  - `formats/emi-audio-payloads.md`
  - `formats/emi-mixed-payloads.md`

Section map:

- `runtime/`: engine behavior, code flow, and module responsibilities
- `formats/`: stable file and payload facts
- `content/`: archive-family and content composition notes
- `sources/`: external-source summaries kept separate from local proof

Rules:

- prefer shipped module names over synthetic names
- for code-bearing archives, treat `archive + slot` as the module identity
- keep durable facts here and leave generated manifests in `processed/inventory/`
