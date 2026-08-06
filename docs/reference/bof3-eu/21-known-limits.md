> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 21. Known limits

**Object and feature geometry.**
- No literal UV fingerprints of dump polygons exist in the EMI data. The engine generates UVs from a base+span formula instead, so fingerprint matching against dump polygons cannot work as a decode strategy.
- Furniture is not represented as elevated map tiles. Bed tiles, for example, use normal floor height.
- Furniture and field-character models are not extractable as 3D polygon meshes. In the original engine they are 2D sprite composites: screen-space textured quads (`POLY_FT4`) drawn per animation frame, not real geometry. `extract/probe-meshdecode.ts` extracts the quad list per mesh group. Integrating them into the walkable 3D world still requires treating them as camera-oriented sprite composites, like the billboards, not as polygon geometry.
- `PL034`'s ct1 container has a larger, 3-fold-nested wrapper (outer header at offset 0 = `0x0c`, pointing to 3 blocks). Not yet unpacked.
- Feature texture words resolve an index through `[idx:u16][b2][b3]`; `idx` lands in map texture data 98.6% of the time. The meaning of bytes `b2`/`b3` is still open.

**Entity overlay recovery.** Tool `extract/build-overlays.ts` separates entity quads (portals, signs) from map-wall quads in a GPU dump. It uses a repetition discriminator: a texture tiled 3 or more times collinear, or 5 or more times anywhere, is blacklisted as a wall. This is reliable only for edge or clear-angle captures. Dense central-village dumps still yield many distinct false-positive quads that aren't tiled and so slip past the discriminator — about 33 of 45 candidate quads in one such dump. No discriminator yet distinguishes an entity from a genuinely unpredicted, undocumented map wall; that stays an open problem.

The McNeil gate specifically cannot be recovered this way. Its texture (page 1, UV 224,112-255,143) is an ordinary structure texture, reused elsewhere on the map: 11 times at each of two other spots, plus 6 more tiles. The discriminator correctly treats it as a wall. No texture, slant, or repetition signal can separate it; recovery would need positional or contextual knowledge that doesn't currently exist.

Tool caveat: AREA008's `cap_3` dump mis-registers onto the self-similar forest floor, in both `probe-wallverify` (picks rows 2-15 instead of the correct ~20-27) and `build-overlays.ts`. Do not build AREA008 overlays from `cap_3`; use `probe-pairquads.ts` for AREA008 wall forensics instead. Dense-cluster capture dumps live in `references/warpdumps-a7/`, outside the extraction pipeline.

**Water animation.** Map-tile water uses a static palette. Confirmed on the AREA060 overworld sea, where the water tiles' CLUT rows (484/485) stay constant across frames; the large VRAM deltas measured there are framebuffer double-buffer churn, not palette writes. Any true water motion in the original would have to come from texel data updates or from entities/camera effects, not palette cycling — a separate, unsolved research question, not a simple shader port.

Separately, a savestate time series in AREA008's storage room (nav section n=64) shows a genuine CLUT cycle: 14 u16 values change in CLUT row 483, columns 97-110. But no map tile in AREA008, 000, 007, 033, or 121 samples those CLUT cells. That cycle likely belongs to an unidentified runtime sprite or entity rather than the map geometry, and may not even represent water. The in-game look of that room is otherwise unverified — no capture exists from inside it; both `warp.ts` access and `capture-walk.ts` fail to reach it.

Cedar Woods' filled pond (AREA003/008, marker tile page5(5,10)) is verified only for that one pond. Other lakes may need their own marker cells, found the same way.

**Chimneys and smoke.** Automatic chimney detection, based on idx-90 hole tiles in the maptex, misses chimneys that lack the hole texture. AREA000's orange house chimney at tile (28,29) is one such case; it had to be added by hand via the chimney editor (key `M`). Two chimneys were observed rendering on alternating frames in the original capture — apparent 25 Hz entity interleaving, likely a performance artifact — and are not reproduced in the browser.

**Furniture, NPCs, and the player character.** The 13 mesh groups in AREA007 are furniture only, 20-33 vertices each; a house's overall look comes from its map tiles and walls, not from mesh groups. Diagnostic tools for this area: `probe-features.ts`, `probe-mesh-match.ts`/`viz.ts`, `probe-bed.ts`, `analyze-ot.ts`, `diag-cells.ts`, `inspect-subfiles.ts`, `find-models.ts`, `dump-featuv.ts`, `find-sharedbase.ts` (a negative-proof tool), `register-dump.ts`. `analyze-box.ts` is obsolete.

NPC/object sprites from `/BIN/PLCHAR/PL###.EMI` are not yet integrated. The player character currently renders as a real field-character sprite only for Teepo, extracted from a GPU dump; the capsule placeholder is now only a fallback for the rest. Still open: battle participation and audio for the player sprite, a Ryu capture, other NPC sprites, walk animation, and directional sprites.

**Other open items.**
- Interior/exterior camera-zone separation is not implemented.
- In-world warp triggers — walking onto a warp tile during normal play, as opposed to the developer warp tool — and their character-sprite handling are not implemented.
- Feature sub-type `TYPE 0x04` is not yet identified.
- `capture-walk.ts`'s automated capture still gets stuck on obstacles during scripted walks; dumps captured before it gets stuck remain usable.
- Building door overlays for outdoor McNeil-style areas needs a check first. AREA000's doors already render correctly via the map wall texture itself; overlays built for it were duplicates, later deleted. Check with the tile-pick tool (`window.__bof3.pick()`) whether a door is already in the wall before adding an overlay.

