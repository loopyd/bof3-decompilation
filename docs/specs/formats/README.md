# Format Specs

Stable file and payload facts.

Scope boundaries:

- keep runtime module behavior, call flow, and helper behavior in `docs/specs/runtime/`
- keep generated per-file tables and inventories in `processed/inventory/`
- keep payload-structure and type semantics in this folder

## Recommended read order

1. `emi.md` (container layout + shared type semantics)
2. `emi-graphics-payloads.md` (type-3 and palette/CLUT companion behavior)
3. `emi-audio-payloads.md` (type-6/7/10 and type-8/9 leads)
4. `emi-mixed-payloads.md` (type-0/1 mixed and unresolved semantics)

## Maintenance rules

- Keep one canonical statement per proven fact and cross-link instead of duplicating.
- If a claim changes due to stronger runtime evidence, update the canonical page and remove stale repeats.
- Keep confidence and open points explicit so unresolved areas stay readable.
