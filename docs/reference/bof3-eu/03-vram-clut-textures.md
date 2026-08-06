> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 3. VRAM, CLUTs and texture decoding

### VRAM model

VRAM holds two kinds of texture content: texels uploaded once from the disc, reconstructable
statically from an EMI container, and content the engine uploads only at runtime, which never
appears in the EMI and must be read from a savestate or GPU dump. Decoders that assume everything
is static paint black wherever a runtime-only upload was expected.

Static reconstruction (`reconstructTextureVram(emi, baseVram?)`) rebuilds a VRAM image from an
EMI's ctype0/ctype3 blocks. Anything the EMI does not cover stays zero unless seeded from a donor
base VRAM.

**Framebuffer in VRAM.** A savestate's VRAM rows 0-239, columns 0-319, hold the rendered frame
directly, double-buffered at y=0 and y=256 (`parseSavestate`). Reading this gives a ready-made
ground-truth image without a screenshot or camera framing — used repeatedly to confirm what a given
VRAM/CLUT region actually displays.

**Overworld base tileset is VRAM-resident, not per-area.** Local areas store every texel they use in
their own EMI. The Overworld sub-regions do not. AREA016/033/045/065 (grassland) and
087/088/115/121/151 (sea/coast) store only overlay tiles — coasts, paths, towns, mountains — plus
nav/height/index data in their own EMI. Texture indices 134-137 are valid, but their VRAM cells are
zero. `isBlankTile` discarded them as black, leaving 88% of the walkable Overworld untextured.

The engine loads the shared base tileset (grass/sea/sand) once from a carrier region, then only
partially re-uploads it when scrolling between neighboring regions — a sub-region's own zeros never
overwrite that resident base. Only three Overworld areas carry the base tileset themselves: AREA060
(sea), AREA152, AREA187 (grassland).

Fix (`rom-tiles.ts` + `build-maptex.ts`): `OVERWORLD_BASE` maps each sub-region to a donor **list**.
Compositing is own-if-nonzero-else-base, matching engine behavior. `build-maptex` pre-combines the
donors into one base VRAM, mathematically identical to the runtime's multi-stage chain.

| Sub-regions | Donor list | Terrain |
|---|---|---|
| 16, 33, 45, 65 | `[187, 60]` (187 primary, 60 fills remaining zeros) | grassland |
| 87, 88, 115, 121, 151 | `[60]` | sea |

Without a `baseVram`, reconstruction stays byte-identical to the pre-fix output — zero regression
(AREA000 md5 `81e056e8…` unchanged, all 196 unaffected areas identical). Result: AREA016 89.6%→100%
(black tiles 91→0), 045/065→100%, 033→97.4%, 087→98%, 121→100%; grass tiles stay byte-preserved from
the 187 donor.

⚠ ~2% stays blank after seeding (AREA033: 29 tiles; sea 87/88: ~1.8%) — absent from every carrier
(60/152/187), so classified as genuine runtime water-edge filler rather than a missing donor. The
donor list is empirical; five small camps (90/153/154/191/192) remain unmapped.

**Three runtime-only upload classes** exist beyond the Overworld case and are invisible to static
EMI reconstruction: a top-CLUT for map decorations like campfire glow, map-tile body CLUTs in 17
areas, and wall-texture CLUTs in 20 areas. All three are read from a savestate and folded back into
the static VRAM as a seed — addresses and mechanism in CLUT formula and per-source specifics below.

### The CLUT formula

**Battle enemy sprites — final, GT-verified formula.** Routine `0x8014cfc8` (jump table
@`0x80149998`) selects the palette. `descLookup` (`0x8014de8c`) fills `ctx[0x27]` with descriptor
byte `f2` (low byte) and `ctx[0x28]` with descriptor byte `b6` (the palette **mode**). Base row is
480 when `ctx[0x24]&4` is 0 — the case in battle, and the case behind every formula and GT anchor
below. The base becomes 496 instead when that bit is set. The slot byte `s = f2 & 0xff` then
decomposes mode-dependently by `b6`:

| `b6` | row | column | colors/sub-palette |
|---|---|---|---|
| 0 | `480 + (s>>4)` | `(s&0xf)·16` | 16 |
| 1 | `480 + s` | `0` | 256 |
| 2 | `480 + (s>>3)` | `(s&7)·32` | 32 |
| 3 | `480 + (s>>2)` | `(s&3)·64` | 64 |
| ≥4 | `480 + (s>>1)` | `(s&1)·128` | 128 |

`b6` doubles as the **U-split gate** — it is the texture organization mode of the descriptor, not an
independent flag.

Underlying data: block B at `0x80035800` uploads contiguously starting at VRAM row 496.
`linearCol = (row − 496)·256 + col` converts a (row, col) pair from the table above into an offset
within that block. Implementation: `clutColFor` / `clutForF2(vram, f2, b6)` in
`build-enemy-anims.ts`, shared by the static and boss figure builders.

GT anchors: Ripper (`f2=0x83`, `b6=2`) → row 496, column 96, matches the `battle1` GPU dump exactly.
Gazer (`f2=0x41`, `b6=3` → row 496, column 64) matches too — previously rendered rainbow-colored
under the old formula.

**Menu portrait CLUTs.** Two static 512-byte blocks, at `0x80033a00` (block A) and `0x80033c00`
(block B), map to VRAM rows 481 and 482. Both are byte-identical to the menu VRAM dump (256/256
halfwords) — 16 sub-palettes of 16 colors each. Which portrait uses which sub-palette is read, not
guessed, from a table: `SHISU.EMI` sub[0] @`0x801d0c00`, table @`0x801d9b78`, twelve 4-byte entries
`[u, v, clutX, clutRow]` indexed by `charId`. `clutRow` 1 selects block A's row (481), 2 selects
block B's row (482); the sub-palette index is `clutX/16`. Proof is the draw routine @`0x801d44c0`
(`andi a2,a2,0xff; sll a2,2; lui v0,0x801e; addiu v0,-0x6488` = `0x801d9b78`). Full table under
per-source specifics.

**Top-CLUT for map decorations (campfire glow).** Row `483+pal`, column `x0 = 0..255` — **no**
`page&3` column offset. The column-offset rule that governs texel CLUTs does not apply to this
top-CLUT source. `decodeTile`/tops read the CLUT from block A, never from VRAM directly — seeding
VRAM alone has no effect. Block A @`0x80033e00` carries the row as a complete `0x8000` placeholder
(STP black) until the runtime loads the real 256-entry CLUT.

### Texture words

**Two page-decode classes exist; picking the wrong one fails outright.** A 32×32 tile texture
stripe (`reconstructTextureVram`, BPS-16 style) covers normal map and UI tile textures. 8bpp sprite
**rects** (GP0 `0x64`-`0x7f`, page-based VRAM reads with U/V plus a CLUT row) cover full-screen
system screens and boss extra pages. The two are not interchangeable — decoding a rect page as a
tile stripe produces scrambled or empty output.

**Index 0 is transparent, not black.** PSX texels carrying palette index 0 are never drawn; the
original engine relies on a neighboring tile's quad to cover the same edge pixel, since at 320×240
one texel is about one screen pixel. This renderer instead wrote index-0 texels as opaque black with
exact quad edges, so 1-texel seams became visible slits when zoomed in — e.g. Ogre Road, behind the
bridge.

Ruled out before finding the real cause: height cracks between neighboring tiles (corners identical,
46/46/46/46), texture-filter bleeding (`NearestFilter`, no mipmaps, `texload.ts`), a foreign object
on top (raycast only hits the terrain mesh), a wrong UV excerpt (GT dump `pd067`: floor prims are
16×16 with u/v ≡ 0 mod 16, matching the code's own assumption), and a VRAM reconstruction gap (only
2 of 65,536 halfwords missing per savestate comparison).

Fix: `decodeTile`, flag `edgeBleed`, set only by the maptex build. An edge hole exactly one texel
deep is filled from its interior neighbor — orthogonally, and diagonally at corners — but only if
the source texel itself is set. Large transparent areas stay untouched, since their interior
neighbor is also 0, reproducing what the PSX achieves through quad overlap.

Effect, opaque-black texel count per maptex: AREA067 710→28, AREA126 796→0, AREA000 34727→34233
(remainder = legitimate black areas with index≠0), AREA104 128→128 (genuine transparency, correctly
left unfilled). Edge-location evidence, by index-based distinct top cells: AREA067 138 edge/**0**
interior texels, AREA126 307/**0** — pure padding cases. Areas with genuine transparency look
different: AREA033 8533 edge/27561 interior, AREA104 4950/16170 — sea/cave edges, not seams.

**A populated CLUT row can still be short.** Wall word `0x15500129` (page 1, palette 5, brightness
`0x50`) exists correctly in the ROM and is assigned correctly by `collectAreaWalls`. But
`renderWallImage` dropped it under a 5%-opacity rule, because CLUT row 488 in the EMI has only
192 of 256 entries populated — the rest is runtime upload. The key fell out of the atlas, and
`terrain.ts` painted the cliff with the tile's bright TOP texture instead of the dark wall.

**A fully zeroed CLUT row is a deliberate no-op, not missing data.** AREA024's c64 dome quads read
CLUT row 483, column 64 — sixteen `0x0000` entries — so the quads draw nothing in the ground truth.
The bright area behind that dome comes from a different draw: the crystal inner wall, page 448, CLUT
column 0 row 488 (opaque bright lavender).

**Additive/semi blend strength is calibrated per case, not one constant.** `feature-semi.json`
values may be `[mode, strength]` (`features.ts` `SemiVal`; `build-feature-semi` merges strengths at
the same mode rather than overwriting). Default strength is 0.5.

| Case | value | reason |
|---|---|---|
| AREA024 c80 dome ring | `[1, 0.7]` | 1.0 oversaturates against wood (sRGB add overstates in the dark); 0.5 stays too dark over black |
| AREA148 crystals | `[1, 1.0]` | raises P90 150→183 against GT 239; lower values under-cluster |
| AREA049 Chrysm facets | `[1, 0.5]` (default) | 1.0 made clustering worse (×3→×4 conspicuous quads) |

### Per-source specifics

**Static enemy battle CLUT block.** AREA-EMI ctype0 CLUT @`0x80035800` (block B above) supplies
VRAM row 496, byte-identical to the `battle1` GPU-dump VRAM. `ct8` is audio, not a texture source —
checked and ruled out.

**BOSS055 / Myria giant form (AREA198).** Descriptors 780/781/792 (`f2=0x10`, `b6=1`; BOSS055 code
keys `0x30c`/`0x30d`/`0x318`) are the final-form pieces: 780 = wings + giant-form main body (golden
mane, ram horns, 244×173), 781 = serpent body (225×225, animated), 792 = tentacles/orbs/mouth parts.
Goddess-form palettes desc609 (blond/turquoise) and desc779 (red/gray) are both real, confirmed
against two variants on GT sheet 40365.

`BOSS055.EMI` is the only boss EMI carrying its own ct3 fragment: sub4 @`0x1a080200` (32 KB, VRAM
rows 256+, i.e. not the sprite band), plus a CLUT splitter (sub6 @`0x80036e00`, sub10 @`0x800357e0`).
Rendered as 4bpp, sub4 turns out to be an arena/UI extra page for the final battle — rubble tile
bands and UI snippets. It is not a duplicate of anything in AREA198, which has no 32 KB ct3 at all.
Its VRAM target `0x1a080200` matches the mode-page address used by the BATE/SHOP HUD ct3s — the
general "system mode graphics" class, not a character asset. The giant form's own descriptor texels
live entirely in AREA198: sub5 covers VRAM columns 448-959, sub7 covers 0-255.

**Menu portrait band.** The shared UI ct3 band @`0x1c080200` (used by `SHISU`/`BATE`/… EMIs; VRAM
target x=896, y=256) and its partner band @`0x1a080200` (x=832) are each a 32-wide ribbon upload
with **BPS=2**: 64 halfwords = 256 px at 4bpp, across 256 rows. Proven by byte diff against the menu
VRAM dump (`menu-field.sav`): the BPS-2 ribbon reading is a 100% match; linear/BPS-4/8/16 readings
score only 5-52%. ⚠ An earlier reading, "4bpp, 512 px, BPS=4," was wrong — it produced scrambled
32px blocks; `build-etc-gfx` was corrected and `minigame_ui*` rebuilt at 256×256.

Portrait cells are 40×48, pitch 40, two rows (v=0/48) inside band `0x1c`. Draw proof: the GP0-`64h`
rects in the menu package buffer (`menufield` RAM @`0x8002095e`/`0x80020f6a`) — Teepo = UV(160,0),
CLUT `0x788c`; Ryu = UV(120,0), CLUT `0x788b`; both 40×48.

Which sub-palette each portrait uses (CLUT formula above) is read from `SHISU.EMI`'s table at
`0x801d9b78`:

| charId | character | tile UV | CLUT block/sub-palette |
|---|---|---|---|
| 0 | Ryu | (120,0) | B/11 |
| 1 | Nina | (40,48) | B/15 |
| 2 | Garr | (80,0) | B/2 |
| 3 | Teepo | (160,0) | B/12 |
| 4 | Rei | (200,0) | B/13 |
| 5 | Momo | (40,0) | B/0 |
| 6 | Peco | (80,48) | A/13 |
| 7 | Ryu (adult) | (0,0) | B/1 |
| 8 | Nina (adult) | (120,48) | A/14 |
| 9 | — | same tile as 0 | — |
| 10 | Whelp | (0,48) | B/14 |
| 11 | unnamed (suspected Rei Weretiger) | (160,48) | A/15 |

**System screens (title / opening / load).** Reconstructed as 8bpp sprite rects (see texture words),
not tile stripes:

| Screen | VRAM pages | CLUT rows |
|---|---|---|
| Title | (576,256), (704,256), (832,256) 8bpp + (960,0) 4bpp | 484-486 / 480 |
| Opening mural | (320-704, 0) | 487-490 |
| Load | ct3 linear-8bpp, 256 wide, embedded ct0 CLUT | self-contained |

`LOAD.EMI` needs no GPU dump — it is fully static. Title and opening need one because their
logo/background pages (576/704) come from the ct1 sprite package plus a runtime upload.
`START.EMI` ct7 is the title jingle (audio, byte-identical to FIRST/SHISU), not graphics; `START.EMI`
ct3 delivers only text and one band. Tool: `extract/build-system-screens.ts` →
`public/entities/system/{title,opening,load}.png` + `index.json`.

**Runtime CLUT upload — map tiles (17 areas).** Found via a campfire-glow hole at AREA053/090: a 3×3
tile block (columns 19-21, rows 25-27) rendered solid black although referenced (index≠0),
renderable, and alpha 255. The cell uses palette indices 1-46 of CLUT row 486, entirely `0x0000` in
the EMI/block A but populated in a settled savestate (sample values 2247, 3374, 2184…) — a runtime
CLUT upload, the same class identified for the 153/154 campfire glow above. Scan signature
(`scratchpad/blackscan.ts`): a referenced 8bpp-nibble top, ≥100 texels, ≥8 distinct indices ("rich
texture"), >90% dead CLUT entries. 20 areas matched: 002, 004, 011, 021, 047, 050, 053, 056, 083,
084, 090, 097, 120, 153, 154, 170, 173, 192, 194, 197; a savestate existed for 17 of them, each
seeded.

**Runtime CLUT upload — walls (20 areas).** Same class, this time CLUT row 488 (see texture words).
Found via prim-hotspot on pd057/pd092 (~3100 mis-pixels at 0 quad findings: "GT dark brick wall,
ours a smooth flat surface"). Scan (`scratchpad/dropwalls.ts`, discarded 8bpp wall words per area):
001, 011, 027, 050, 057, 067, 076, 087, 088, 092, 094, 099, 104, 116, 121, 126, 147, 151, 152, 167.
16 of these were seeded (001/147 had no savestate; 011/050 were already seeded via their tops).

**Seed mechanism (shared by both runtime-CLUT classes).** `scratchpad/mkseed.ts <area> <state>
<row…>` writes savestate CLUT rows into a `clut` block inside `references/re/water-seed-<n>.json`.
For map-tile tops, `build-maptex`'s `applyClutSeed` mirrors that block into block A; for walls,
`build-walltex` plays it into VRAM directly before `renderWallImage` runs. Both apply it
**conservatively** — only entries still at placeholder value (`0x0000`/`0x8000`) get replaced, so
static real colors always win and reseeding is regression-safe. Diagnosis order for "cells filled but
still black": check block A's palette for `0x8000` rows before suspecting a texel gap —
`extract-runtime-water`'s FILL pass only checks texels and misses this class entirely.

### Verification

**Ground truth capture (DuckStation).** Load a save and trigger a single-frame GPU trace, both
scriptable:
```
open -a DuckStation --args -nofullscreen -statefile "<…/savestates/SLES-01304_*.sav>"
osascript -e 'tell application "DuckStation" to activate' -e 'tell application "System Events" to key code 100'
```
`key code 100` = **F8**, bound to `RecordSingleFrameGPUDump` in DuckStation's `settings.ini`. The
trace is written as a zstd-compressed `.psxgpu.zst` file into the `screenshots/` folder. DuckStation's
GDB server (port 2345) gives live RAM access for identifying what is loaded — `npm run gdb:read
80104000 64` reads the map header (columns/rows) to resolve the area ID. Curated dumps live at
`references/gpudump/{mcneil,door,crossing}.psxgpu.zst` (recon tags `inn`/`village`/`crossing`; a
`crossing.json` exists but is not registered in `recon/index.json`).

Tools (`extract/`, run as npm scripts):
- `gpudump.ts` — parser for `PSXGPUDUMPv1`: decodes GP0 (textured polys: screen verts, UV, texpage,
  CLUT), reconstructs VRAM from `0xA0` writes, separates frames on VSync; unpacks zstd/xz/gzip.
- `inspect:gpu` — statistics + VRAM PNG.
- `render:gpu` — pixel-accurate frame reconstruction (the proof of correct extraction).
- `atlas:gpu` — texture pages used, grouped per CLUT.
- `analyze:gpu` / `cell-clut` / `bind-pages` — diagnostics.
- `floor-grid` — iso grid fit.
- `export:recon` — walkable scene export → `public/recon/<tag>.{json,png}`, viewer
  `/recon.html?recon=<tag>`. Captured scenes: `inn`, `village` (outside McNeil).
- `gdb:read` — live RAM read.

Finding from this route: per-tile palette is `(page, cell) → CLUT` **consistent** — no ambiguous
per-tile palette exists; the correct CLUT is a fixed property of the texture cell it hangs off. A
captured scene was confirmed via live RAM to be exactly AREA000 (100×80 map).

**`extract/prim-hotspot.py <pair>`** (judge level 4) reports mis-pixel regions on a 16px grid that
the quad detector cannot see — quads under the fill threshold, non-map content, partial coverage —
together with GT/browser brightness and the nearest resolved quad. Reading rule: GT bright / browser
0 means missing content; browser brighter means painted too bright or with the wrong palette. It
surfaced the AREA053/090 campfire holes (→ the 17-area map-tile CLUT class), the AREA119 wall hatch,
and the AREA057 wall tint (→ the 20-area wall CLUT class). Judge levels: 1 = `audit:prims` (classes),
2 = solver coverage, 3 = `detect:prims` (quad verdicts), 4 = `prim-hotspot` (pixel residuals).

**Judge pass-rate deltas confirm each CLUT fix is a real rendering fix, not a metric artifact.**
Map-tile class: pd053 99→**100/0**, pd090 99→**100/0**, pd011b 96→**99**, pd011c 95→**99**,
pd004/011/021/050/056/097/120/153/154/170/194 each **100%**, pd002/pd173 100% (1 residual quad),
pd047 99%, pd083/084 99%. Wall class: pd057 98→**100/0**, pd092 98→**100/0**, pd151 99→**100/0**,
pd076→**100/0**, pd099/pd116→**100/0**, pd087 96→99, pd088→99, pd094 98→99, pd126/152→100%.
Edge-bleed class: pd000 100/0, pd033 100/0, pd067 98/17, pd104 94/1, pd126 100/6 — each identical to
the pre-fix state (regression-free).

**Portrait verification.** Template matching against `portraits-gt.png`: 10/11 tiles hit, Garr
`d=0.000` @ (181,57). The only miss is the whelp portrait (0,48) — simply absent from the external
rip used for comparison. Five child-era portraits are pixel-exact (`d=0`) via auto-match; the adult
row matches visually. Methodology for solving alignment and palette without prior palette knowledge:
score consistency of the index→framebuffer color mapping (each of the 16 indices must map onto
exactly one framebuffer color), then re-search the resulting 16-color palette in the VRAM/disc block
(`scratchpad/portrait-solve.ts`).

**Enemy CLUT verification.** ENEMY019: saturation 76% vs GT 77% (27/27). 53 of 375 enemies are
legitimately low-saturation (gray/metallic), down from an apparent 375/375 before the fix. Direct
GT-dump anchors exist for Ripper, EyeGoo (original `battle1` dump: `clut(96,496)` = body,
`clut(160,496)` = wings) and Gazer; all other enemies are validated by formula consistency and
palette-swap coherence, not by a direct battle dump.

**Overworld regression check.** Without a donor `baseVram`, reconstruction is required to stay
byte-identical to the pre-fix output — verified via md5 (AREA000 `81e056e8…` unchanged, all 196
unaffected areas identical).

### Refuted approaches

- **"Row 496, `(f2&0xf)·32`" for enemy CLUTs** was only the `b6=2` case; it broke for other modes
  (e.g. `f2=0x9x` incorrectly folded slots 16-31 onto 0-15). The mode-dependent `b6` formula above
  replaces it completely. The earlier veto "object texels not loaded / none of the 32 sub-palettes
  fits" was this same mode error — the sweep that "proved" it only ever swept mode-2 columns. That
  older formula also had no answer for 7 enemies whose `f2` pointed at column 256 or 288, where the
  AREA-EMI has no data (a shared battle upload) — those fell back to column 0.
- **The enemy palette selector was briefly thought to be `b7`.** Goblin (`f2=0x81`) and BossGbln2
  (`f2=0x82`) share geometry but need columns 32 and 64 respectively; `b7` would have given both the
  same column. `f2` is the correct selector.
- **"Missing Overworld tiles = page-0 water" was wrong** — a bug in `scratchpad/water-diag.ts` read
  texture entries at the wrong offset. The real cause was the VRAM-resident base tileset described
  above (AREA016/Yraall is grassland with barely any water; the visible defect was black terrain
  holes, not missing sea).
- **The title-screen "non-standard VRAM-page decode" blocker was a phantom.** The screens were never
  a 32×32-tile stripe, which is why `reconstructTextureVram`/BPS-16 failed on them — they are 8bpp
  sprite rects, a different decode path entirely.
- **The 024 dome glyph bands were suspected to be a decode error** ("text tube = foreign VRAM").
  Refuted: a Spriters Resource rip (40515) shows the identical bands, confirming real design, not
  corrupted VRAM content.
- **Goddess-form palette desc779 was read as black/orange.** GT sheet 40365 shows two real variants
  — desc609 (blond/turquoise) and desc779 (red/gray) — and the black/orange reading matched neither.
- **The whelp portrait's palette was guessed as B/2 (and the tile mislabeled "dragon").** The read
  table gives B/14. B/2 actually belongs to Garr, which had itself been mislabeled "whelp" in the
  guessed version.
- **"All wall drop-words are ROM-side 0%-opaque, hence not separable via the opacity rule" was the
  right measurement with the wrong conclusion.** The words are not transparent; they lack a palette
  because CLUT row 488 is only partially populated in the EMI. An earlier attempt to "restore" them
  as fully transparent was consequently a regression, corrected by seeding the missing CLUT entries
  instead.
- **A CLUT key collision was blamed for AREA148's doubled crystals** — the diagnosis held that
  `build-runtime-geo.ts:84` only folds the CLUT into its dedup key when `q.pg` is set, so same-UV
  quads under different palettes could collide. Retracted: GT dump `pd148`, on page 576,256 with
  CLUT column 0 row 492, has exactly 24 prims; `runtime-geo-148.json` independently has exactly 24
  quads with the same UV rects and the same semi modes. The seed's CLUT genuinely is `[0,492]`, and
  a collision is structurally impossible — all 24 quads are homogeneous, and of 17 seeds only 3
  (067/148/198) even have quads without `q.pg`, all homogeneous too. The misdiagnosis came from
  reading the dump's three CLUTs (c490 = floor/walls, c491 = dark floor tiles + base, c492 = crystals
  + a 2×2 blue field) as "same UVs, several palettes" without counting prim quantities against the
  seed first. The actual bug was a geometry placement anchor, unrelated to CLUTs. Lesson: count
  first, then claim a cause.

### Open

- Overworld: ~2% of tiles stay blank after donor-seeding (AREA033: 29 tiles; sea 87/88: ~1.8%),
  presumed runtime water-edge filler but unconfirmed; five small camps (90/153/154/191/192) have no
  donor mapping yet.
- AREA197 (map-tile runtime-CLUT class) has no matching savestate, so its fix has no GT anchor —
  likewise any scanned area without a `pd`-pair capture.
- AREA001 and AREA147 (wall runtime-CLUT class) have no savestate to seed from yet.
- Game-over screen (`SHISU.EMI`, 死す): its ct3 decodes cleanly as neither linear-8bpp/16bpp nor as a
  tile-stripe-plus-CLUT. Needs a death-state GPU dump (pad input during a lost battle, hard to
  automate). The boss-graphics rect class (K1) shares this status — no longer a format puzzle, just
  waiting on a dump.

