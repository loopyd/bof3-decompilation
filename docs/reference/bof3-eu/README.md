# Breath of Fire III (EU) — imported reverse-engineering reference

Imported verbatim (chapter split only) from the bof3js project's
`references/KNOWLEDGE.md`. It documents the **EU release (SLES_013.04)**.

⚠ **Address-space warning.** Every RAM address in these files is EU address
space. Formats, record layouts, and game rules carry over to our US
SLUS_004.22 targets; addresses do not. A US fact still needs our own
evidence — treat these files as leads and format contracts, not reviewed
facts. Reviewed US facts live in [`../specs/`](../specs/).

Supporting machine-readable data (animation/CLUT phase tables, runtime
geometry dumps, community game data) stays upstream in the bof3js repo
(`bod3js/references/re/`, `bod3js/data/`) — it is large and regenerable.

## Engine and disc

- [01 — The engine at a glance](01-engine-at-a-glance.md) — overlay map, disc-to-screen pipeline
- [02 — Disc, EMI containers, content types](02-disc-emi-containers.md)

## Graphics

- [03 — VRAM, CLUTs, texture decoding](03-vram-clut-textures.md)
- [04 — Map geometry: tiles, walls, heights, collision](04-map-geometry.md)
- [05 — Features, object meshes, decoration](05-features-meshes.md)
- [06 — Animated surfaces: water, sky, fire, weather](06-animated-surfaces.md)

## World and entities

- [07 — Sprite system and field characters](07-sprite-system-field.md)
- [08 — NPCs, spawns, world entities](08-npcs-entities.md)
- [11 — World structure: navigation, sections, camera, warps](11-world-structure.md)
- [12 — Scripting: SCENA, movement VM, object anchors](12-scripting-scena.md)

## Battle and effects

- [09 — Battle sprites, enemies, bosses](09-battle-sprites-enemies.md)
- [10 — Effects, spells, transformations](10-effects-spells.md)
- [14 — Battle mechanics](14-battle-mechanics.md)

## Game rules and data

- [15 — Party data, items, menus, save format](15-party-items-save.md)
- [16 — Game systems: masters, fairies, dragons, fishing, shops, casino](16-game-systems.md)

## Audio

- [13 — VAB, SEQ, SFX banks, XA streams](13-audio.md)

## Method and registers

- [18 — Method: disassembly, ground truth, verification](18-method-verification.md)
- [19 — File register: what produces what](19-file-register.md)
- [20 — Address register (EU)](20-address-register.md)
- [21 — Known limits](21-known-limits.md)
- [22 — Further address findings](22-further-address-findings.md)

## Tooling (bof3js-specific)

- [17 — The browser client](17-browser-client.md) — describes the upstream
  project tooling; kept for provenance of the derived data
