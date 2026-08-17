# Breath of Fire III (EU) — imported reverse-engineering reference

Imported verbatim (chapter split only) from the bof3js project's
`references/KNOWLEDGE.md`; documents the **EU release (SLES_013.04)**.

⚠ **Address-space warning.** Every RAM address here is EU address space.
Formats, record layouts, and game rules carry over to US SLUS_004.22
targets; addresses do not. A US fact still needs our own evidence — treat
these files as leads and format contracts, not reviewed facts. Reviewed US
facts live in [`../../specs/`](../../specs/).

**US 1.1 annotations.** These files are read-only for EU content. When our
loop verifies a US 1.1 (SLUS_004.22) difference against an EU claim, append
a greppable quote block directly after that claim:

```
> **US 1.1 verified:** <the US-specific fact> (<selector or commit>)
```

Never edit or delete EU text; corrections accumulate as annotation blocks.
Supporting machine-readable data stays upstream in the bof3js repo
(`bof3js/references/re/`, `bof3js/data/`) — large and regenerable.

## Engine and disc

- [01 — Engine at a glance](01-engine-at-a-glance.md) — overlay map
- [02 — Disc, EMI containers](02-disc-emi-containers.md)

## Graphics

- [03 — VRAM, CLUTs, textures](03-vram-clut-textures.md)
- [04 — Map geometry, collision](04-map-geometry.md)
- [05 — Features, meshes](05-features-meshes.md)
- [06 — Animated surfaces](06-animated-surfaces.md)

## World and entities

- [07 — Sprites, field characters](07-sprite-system-field.md)
- [08 — NPCs, world entities](08-npcs-entities.md)
- [11 — World structure, warps](11-world-structure.md)
- [12 — Scripting (SCENA, movement VM)](12-scripting-scena.md)

## Battle and effects

- [09 — Battle sprites, enemies](09-battle-sprites-enemies.md)
- [10 — Effects, spells](10-effects-spells.md)
- [14 — Battle mechanics](14-battle-mechanics.md)

## Game rules and data

- [15 — Party, items, menus, save](15-party-items-save.md)
- [16 — Game systems, shops](16-game-systems.md)

## Audio

- [13 — Audio (VAB, SEQ, SFX, XA)](13-audio.md)

## Method and registers

- [18 — Method](18-method-verification.md)
- [19 — File register](19-file-register.md)
- [20 — Address register (EU)](20-address-register.md)
- [21 — Known limits](21-known-limits.md)
- [22 — Further addresses](22-further-address-findings.md)

## Tooling (bof3js-specific)

- [17 — Browser client](17-browser-client.md) — upstream tooling
