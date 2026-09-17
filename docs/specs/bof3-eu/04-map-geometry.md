> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 4. Map geometry — tiles, walls, heights, collision

### Tile top textures

Tile textures come directly from the ROM; no emulator is needed. `extract/rom-tiles.ts` is a
pixel-exact port of the verified renderer from `bof3-3d-maps` (`test_mips_render.py`,
`render_area_mips(path='default')`). Per area it produces a top-down tile map
(`public/maptex/areaNNN.png`, `cols·16 × rows·16`) and matching geometry
(`public/areas/areaNNN.json`, from the same parser: heights plus renderable indices).
`render/terrain.ts` maps this texture per UV onto the tile tops of the height mesh;
`main.ts` loads it per area (`flipY=false`, nearest filtering, sRGB). Verified pixel-identical
(0 diff) against the reference renderer for AREA000; AREA037 is generically clean. Standalone
verification viewer: `/texture-lab.html`.

Tile-top entries use the **same texture-word system as walls** (nibble/rect/pair, decoded by
routine `0x801557d4`) — not just plain 16×16 cells. An entry whose byte-1 nibble is non-zero is a
RECT or PAIR top: steep-slope tiles (McNeil mountains: 227 rect-tops in AREA000, same pattern in
007/003) carry 24×16, 32×16 or 32×32 textures at native density for their larger projected area.
An older decoder read byte 0 of such entries as a plain cell coordinate. That single bug explained
three separate symptoms:

- **Wrong mountain textures** — a random 16×16 cell was shown instead of the rect (including the
  "gray columns" on page 4).
- **Black-splotch tiles** — rect-tops with byte0=0x00 were read as "cell (0,0) = uniform black"
  and misclassified as blank/skip (50 of the 229 AREA000 skips). The remaining 179 skips are real
  uniform-black Capcom filler (cell index 250 → `0x8000`) that the game never draws (dump
  evidence: 0 quads originate from cell (0,0)); skip-fill for those tiles is purely free-camera
  cosmetics. ⚠ Blank cells ARE drawn on entities (e.g. dark stairway side faces in a shop) — "never
  drawn" applies only to map tile tops.
- **Phantom "gate texture tiled across the map"** — e.g. the pattern at page1 224,112..255,143 —
  was cliff rect-tops, not a wall or gate texture.

Dump evidence: a sweep draws at tile (7,9)E exactly the rect [80,16..111,31], i.e. top-word
`0x14800106`; the 95 "unpredicted wall quads" of the mountain region are all rect-tops. Fix
locations: `isBlankTile` (byte-1 guard), `renderArea` (rect-tops are rotation-correctly
downsampled to 16×16 into the maptex, so the standard mesh path shows rock), `scene-compile.ts`
(rect-tops render natively via `renderWallImage` including corner permutation; forensics flags
`--no-fill`/`--raw-skips`). All 200 maptex/area JSONs were rebuilt; the faithful scenes for
000/003/007/027 were re-exported. ⚠ "Pixel-identical to the reference renderer" now holds only for
plain nibble tiles — the `bof3-3d-maps` reference renderer has the same rect-top bug, so this
project is now more correct than its own reference.

Two earlier caveats are resolved:
- The claim "pages 4–7 only render correctly for AREA000/003/007" was wrong. `decodeTile()` uses
  one universal MIPS formula, `page&3`, which also covers pages 4–7. The old per-area lookup
  tables (`PAGES_47_LOOKUP`, `TXTY`, `pickClut`, `DERIVED_CLUT`) were dead code and were removed
  with zero regression (AREA000 stayed pixel-exact, AREA033 unchanged).
- "Grass/shared textures live outside the EMI" and "walls are only height-based" are both
  resolved: all texels live in the EMI (the shared-base theory is refuted), and walls have been
  textured since 2026-06-10. Visible furniture renders as panels/billboards; full mesh-group 3D
  models cannot be placed statically (see "Object mesh system" below).

**Stairs/ramps look "noisy" in the flat top-down maptex — this is not a bug.** Slope tiles have
stepped corner heights (e.g. AREA000 col 91, rows 42–44: `[24,24,16,16]→[8,8,0,0]`, `walk=0x60`
stair markers, sequential texture indices 681–684, shared textures) and carry ramp textures
designed for the sloped 3D surface. Baked flat from directly above they look jumbled; rendered in
3D they form a correct stepped ramp. The data is genuine and must not be "corrected."

### Wall textures

System v3; edge assignment is verified exact. `collectAreaWalls()` / `resolveWallUV()` /
`renderWallImage()` (all in `rom-tiles.ts`) build one wall quad per edge: edges are found via a
height predicate (east or south neighbor lower, signed comparison), entries are read sequentially
starting at `texIdx+1` (south before east), and UV comes from the nibble/rect/pair tables in the
map header. `npm run extract:walltex [areaNr]` writes `public/walltex/areaNNN.{png,json}` (a
packed, deduplicated atlas; 186 areas). ⚠ Older documentation describing a "chain to a 00
terminator" and separate `_E/_S.png` step atlases is obsolete — `/wall-lab.html` still loads that
old format and no longer shows correct textures.

### Corner heights

Each map corner height is a **signed byte**. Proof is twofold:

1. **Disassembly** — the map frame loop loads the corner height with `lb` (signed load) at
   `0x8015361c`, from address `0x80104030 + (row·cols+col)·4`, then does `sll 4` (×16) and
   `subu $zero,…` (PSX Y is negated) before feeding the RTPS vertex pipeline.
2. **Ground truth** — warping to AREA005 (33,20) and photographing the window: the character
   stands in a sunken riverbed (`h=248` = −8), a dirt floor with embankments about one tile below
   the bank, with a narrow creek rendered as ordinary map tile texture and a wooden bridge over
   it. The tile is walkable (`walk=0x40`, warp bytes `0xc0`/`0xa3` inside the bed) and is plain
   deep terrain — no runtime water, no special renderer.

**The former rule "divider ≥232 = never draw" is wrong and has been removed** (from
`grid.isDivider`, both `terrain.ts` guards, the `hasNav`-0 clamp, the two `h<232` filters in
main/features, and the imgviewer coloring). The game only skips drawing where `tileTexIdx==0`
(tile renderer `0x80154508` returns immediately at index 0). Textured tiles with `h≥232` (i.e.
negative heights) are real depressions, from −24 to −1. The old heuristic happened to work only
because most deep tiles also had `tileTexIdx==0`; an earlier note "h248 → 224-unit-high fallback
pillars" was the unsigned-byte symptom of the same misreading.

Conversion is centralized in the browser loader (`src/world/loader.ts`): `h≥128 → h−256`. JSON
files stay raw `u8`; extract tools are unchanged. The `probe-w8-holes` "DIV≥232" hole class remains
a valid tool convention, because textured deep tiles were already counted there via
`tileTexIdx==0` — but that count must be read as "deep tiles," not as "missing ground."

**System-wide effect:** the entire 005/010/011/012 riverbed and east-bank half becomes visible and
walkable (the walkable-`0x4_` rule resolves itself once the bed is rendered; section 1 of AREA005
grows from 1729 to 1963 tiles). Dozens of areas with textured deep zones render for the first time
(056: 1704 tiles, 070: 1763, 022/049/074/076/077: each 300–1000). The McNeil basement stairs (area
000, tiles (57–58,67–68), `walk=0x60`, `h=−8`) now correctly lead downward. In the AREA189 desert,
removing the old 0-clamp lets the −1/−2 dunes render as in the original (ground truth "flat sand"
stays visually flat). Verified areas: 000, 003, 005, 011, 016, 147, 189.

### Visibility condition interpreter (roofs, flags, story phases)

One interpreter, `0x801560f0`, evaluates the "cond" field shared by several map subsystems: roof
feature records (type 0x00, handler `0x80156274`), the cell/CLUT animation table's group header
(see table [17] below), the state-patch table's group header (table [20], consumer
`0x80157970`), and the type-0x22 geometry variant. Its FLAG_TEST branch, `0x80156200`, has
disassembly-proven semantics:

| `b1` range | Meaning |
|---|---|
| `b1 < 0x20` | visible when story bit (block `b1`, bit `b0`) is SET |
| `0x20 ≤ b1 < 0x40` | block `b1 & 0x1f`, visible when the bit is NOT set (`sltiu` inversion at `0x80156260`) |
| `b1 == 0x40` | visible when `b0 & 1` |
| `0xfa` | visible when `b0 == ` current story chapter (tail `0x80156260`, an `sltiu` inversion of the XOR) |
| `0xfd` | visible when `b0 ≠ ` entrance-index byte (tail `0x80156264`, direct compare) |

`0x14` and `0x34` are the two most common concrete codes seen in exported roof conditions — they
are ordinary instances of the `b1<0x20`/`0x20≤b1<0x40` ranges above (`0x34 & 0x1f == 0x14`, so the
pair is the SET/NOT-SET reading of the same story-flag block). Histogram over all exported roof
conditions: `0x14`×758, `0x34`×797, `0xfe`×168, `0xfb`×158 (plus flag codes), `0xfd`×16, `0xff`×16.
`0xfa` never occurs in the exported set, so its effect is currently limited to the 16 `0xfd`
records; `0xfe`/`0xff` remain unimplemented ("latent").

Implementation: `condVisible()` in `render/features.ts`, fed by a story-flag bank at
`public/re/story-flags.json` (sourced from a door save at `0x80144e88`, the ground-truth base
save; loaded by `main.ts`). Story-conditional roofs now appear flag-correctly instead of being
uniformly shown or hidden. Thirteen areas that previously had no feature JSON at all are now
covered: 086 (6 roofs) and 105 (672 roofs); the remaining 11 genuinely export nothing.

### Feature type dispatch table

A table at `0x8017fb34` in the EXE, indexed by the feature record's TYPE byte, selects the handler
that draws it (44 slots, `0x00`–`0x2b`):

| Type | Handler | Meaning |
|---|---|---|
| `0x00` | `0x80156274` | roof class (cond-gated quads; can itself chain multiple quads, see "Backdrop" below) |
| `0x01`–`0x03`, `0x05`–`0x0e` | `0x801566a0` | stars |
| `0x04` | `0x80156ac0` | diagonal signs/fences |
| `0x0f` | `0x80156c2c` | animated effect drawer |
| `0x10` | `0x80156e8c` | walls |
| `0x11`–`0x1f` | `0x80157074` | per-tile vertex animation |
| `0x21` | `0x801572f0` | multi-quad 3D |
| `0x22` | `0x801575cc` | cond-checked geometry variant |
| `0x24` | `0x801578b8` | patch-cycle: apply patch, arm the return |
| `0x25` | `0x80157914` | patch-cycle: apply the return patch |
| `0x27` | `0x80157c1c` | billboards |
| `0x28`–`0x2b` | `0x801efefc` / `0x801f016c` / `0x801f0488` / `0x801f0778` | per-area overlay handlers |
| `0x18`, `0x20`, `0x23`, `0x26`, `0x2c` | `0x801578b0` (`jr ra`) | renderer NOP |

Notes per type:
- **`0x04` diagonal signs/fences** (89 records / 11 areas: 013–019, 035, 056, 068, 095, 096): an
  upright quad running NE–SW over the tile, vertices `(col·128−0x4040, row·128−0x3fc0)` →
  `(col·128−0x3fc0, row·128−0x4040)` (browser-space: `(col,row+1)→(col+1,row)`), `0x80` units (one
  tile) tall, foot pinned to the map corner heights (`lhu map+2` / `lbu map+1`, `·16`). A record is
  skipped if the high nibble of its word is non-zero. Texture is a fixed word `0xb1800110`, which
  the entry decoder `0x801557d4` resolves to rect `0x10` of the area rect table, 4bpp window
  column 3, `CLUT(16,483)`. Occupied cases: area 068 = a signpost on the well path; area 035 = the
  diagonal connector segments between the axis-parallel panel fences of the Ogre Road. Exported by
  `build-features.ts` as `signs[]` (col/row/key, atlas via `atlasForWord`); drawn by
  `render/features.ts`.
- **`0x0f` animated effect drawer** (27 records / 10 areas, e.g. 000×3, 002, 049, 057, 092): reads
  a frame counter at `0x80143e6c` (masked `&7`/`&3` for phase), a floor height at `0x801549f0`,
  and applies a divide-by-5 magic constant (`0x66666667`) to scroll a texture — the waterfall/flow
  class (e.g. the McNeil millstream). The geometry-emission loop itself is unread; browser
  reproduction is an open nice-to-have.
- **`0x11`–`0x1f` per-tile vertex animation**: iso-window culling (scroll state at
  `0x80149320` ff., window `0x38×0x1c`) checks whether the window-grid cell at
  `0x8012c000[row·0x70+col·4]` is alive, then `0x80157148((type−0x10), slot)` copies the XY
  corners of the tile's GPU package from donor slots in an 80-byte pool at
  `0x8012d880[(type−0x10)·80 + phase·40]`, with `phase` read from a byte at `0x80143d44`. This
  produces a wave/wobble distortion of map tiles. Occurrence counts (even types only): `0x12`×196
  in 27 areas, `0x14`×162/25, `0x16`×19/15, `0x1a`×106/34, `0x1c`×124/37 (McNeil, area 000:
  14/12/3). Not implemented in the browser (subtle effect, nice-to-have).
- **`0x22` cond-checked geometry variant** (66 records / 13 areas, e.g. 037, 143, 127): calls the
  same visibility interpreter `0x801560f0` as type 0. Vertex construction from record bytes is
  unread.
- **`0x23`/`0x24`/`0x25` — the state-patch trigger cycle.** `0x23` is the at-rest marker (its
  handler is the NOP, so a resting record draws nothing); it occurs 670 times across 93 areas. A
  proximity trigger stamps the type to `0x24`, whose handler applies the map-texture patch (table
  [20], below) and re-stamps the type to `0x25`; the `0x25` handler applies the return patch and
  stamps the type back to `0x23`. Net effect: a back-and-forth visual toggle (e.g. a door). Because
  the type dispatch itself never draws anything for a resting `0x23` record, an earlier suspicion
  that this record class was a hidden graphics source (candidate for the area-086 crane) is
  disproved; likelier consumers are non-graphics systems such as sound or encounter zones.
- **`0x28`–`0x2b` per-area overlay handlers**: present in the dispatch table but confirmed **used
  by no area** — every area's feature block has zero records of these types (they are reserve
  slots). An earlier guess that connected them to the area-086 crane graphics is wrong; the crane
  is drawn by separate per-area overlay code with no feature-type record at all (see "Runtime-drawn
  per-area geometry" below). The large number of `0x40`-series "types" visible in raw feature-block
  scans belongs to an unrelated record class (a panel path), not this table.

### Map header table [17] — tile and CLUT animation

**Format.** The map header's `[17]` table holds groups:
`[0x80|wordcount|period:u16] [cond:u16] entry*`, terminated by a `0` word. `cond` uses the same
encoding as the visibility interpreter above (frequently `0x4001`). Word count in the header
includes the 2 header words, i.e. it is the total group length. Each entry is
`[tick:u8][seg?:u8][source:u8][target:u8]`. `tick` rises up to `period`; `source` sequences step
in units of `0x10` and can ping-pong (observed: `10→20→30→40→30→20`). A second group format, type
`0x81`, uses a different layout (suspected to be CLUT-specific) and is not decoded.

Groups drive two independently confirmed rendering mechanisms:

**1. VRAM cell upload** (new texel data streamed into VRAM, e.g. sea/lava). Pipeline:
`extract/extract-anim-phases.ts` derives cell phases from a series of time-offset GPU dumps (the
dumps must come from *separate* warps with different `--wait` values — dumps taken back-to-back in
one emulator run are phase-identical, because the game is paused between them, see "Tooling notes"
below); `build-water-anim.ts` (generalized to arbitrary texture pages) writes
`anim-phases-<NNN>.json` per area and decodes tiles directly rather than through the full area
renderer, so purely local areas are covered too. Example: area 052 (lava) has 4 cells / 2 phases /
216 tiles and now glows and animates.

**2. CLUT cycling** (the palette rotates; UVs, vertex geometry and VRAM texels all stay constant).
An earlier "slot UV" theory — that per-frame UV offsets in the GPU command-list slot pool caused
the animation — is wrong. Ground truth on area 005, comparing several animation phases, found
these values BYTE-IDENTICAL across phases: map quad UVs in the GP0 stream, vertex XY, command
color words, Gouraud vertex colors, in-RAM `textureData` entries, the slot FT4 records at
`0x8012d880` (UV+RGB), and the VRAM texels of the map cells themselves. Only the CLUT changes.
Mechanism: at area 005, CLUT `(96,483)` — 4bpp palette 6 in the BlockA layout (`parseClut16` offset
192) — cycles through 4 phases (A→B→C→D): entries 1–4 and 8–11 each rotate left by one position
(ring rotation); entries 5–7 pulse brightness dark→medium→bright→medium (ping-pong); entries 12–14
run their own 4-entry table. 8bpp palettes 5/6 (VRAM rows `(0,488)`/`(0,489)`) cycle in the same
way. The associated GPU event is one 8×24 VRAM-to-VRAM copy per frame,
`(768+off,488)→(768,488)` — staging of a texel bank in 8 hardware steps; nothing directly
references this staging region, so its ultimate consumer is still unclear.

`[17]` byte semantics for the CLUT-cycling case (established, not fully generalized): target byte
`0x36` always denotes the 4bpp-palette-6 river CLUT (true across all 9 family areas); the source
byte, times `0x10`, is the byte offset into the phase bank (8 hardware steps, matching observed
copy source offsets 776/792); one tick equals 2 PAL frames (area 005: period `0x20`, 4 entries → 8
ticks → 16 frames → 0.64 s per phase; area 022 has period `0x10` → 0.32 s — the clock for any area
is readable straight from its target-`0x36` group). The player/consumer code itself was not
localized (no matching `lhu`/`lui` pattern); a running u16 tick counter was found at map-header
scratch offsets `+0x1a`/`+0x1c` as an anchor for a future search.

**Pipeline (CLUT class, all 9 river-family areas — 005/010/011/012/018/019/022/023/067):**
`extract/extract-clut-phases.ts <area> <dumps…>` writes `references/re/clut-phases-<NNN>.json`
(animated 16-entry CLUT blocks, phases kept in capture order, ring order corrected against the
rotation direction). `build-water-anim.ts`'s CLUT mode patches the phases into a copy of BlockA
(tile decoding reads CLUTs from BlockA, not VRAM), finds affected map entries by diffing decoded
tiles, and also patches the feature-atlas phases (the river is a type-0 FEATURE floor; its atlas
formula is: 4bpp, `b3&0x80` set → `CLUT(pal·16,483)`). Output: `public/water/area<NNN>-feat.png`.
Sheet frame count is the LCM of the per-CLUT phase counts (area 011: 4/6/3 → 12 frames). JSON
carries `seq:'cycle'` (true round-robin, not ping-pong), `intervalMs` (the measured original
clock), and `flat:false` so the flattening step leaves the riverbed relief intact. The browser
applies frames cyclically via `texload.ts` (map canvas) and `render/features.ts` (atlas
`CanvasTexture`).

**Detection and coverage.** An offline audit tool, `extract/probe-w9-audit.ts`, runs 5 detectors
across all 200 areas without an emulator: D1 (object anchor over a hole tile without a feature
floor), D2 (deck gaps — hole tiles with vs. without a roof in deck sections), D3 (blank clusters:
≥8 contiguous skipped tiles, ≥30% walkable — VRAM-upload candidates), D4 (unused `[17]` cell
animation groups, type `0x80`), D5 (roof quads without an atlas key). D1 and D5 found zero
anomalies anywhere. D4 initially flagged roughly 40 areas with 1–7 never-played animation groups
each (e.g. 002:7, 021:7, 056:7, 024:6, 057:6, 092:6). A batch capture round (12 GPU dumps per area,
uneven intervals, feeding both `extract-clut-phases` and `extract-anim-phases` from the same
series) resolved them in two passes:
- First pass added CLUT pulse animations (torches/embers) to 002, 021, 024, 053 (campfire embers),
  056, 057, 076, 092, 094, 109, 126, 153, 154, 155, 192, 194, and VRAM cell "sparkle" animations to
  water/floor surfaces: 092/057 harbor water (1888 tiles), 094/076 (5537/3347 tiles), 126 Black
  Ship sea (5459 tiles), 155 (1296 tiles) — all confirmed against ground-truth pixel diffs as
  wandering glints distributed over the surface, matching the original.
- A second pass cleared the remaining 28 D4 areas, bringing the total to **50 areas with map
  animation**: river-CLUT areas 026/028; ember/torch CLUT areas 027, 041, 075, 078, 079, 090, 097,
  102, 108, 113, 143, 147, 150, 153, 157, 174–179; cell+CLUT area 038; area 068; the Angel Tower
  moat at area 082 (an 80 ms clock, confirmed only at the moat edge — 15878 px of recon diff there,
  none at the plateau center, reinforcing the rule "measure ground truth exactly where the
  animated keys are"); and the Mt. Zublo lava glow at area 103 (128 entries / 2594 tiles / 12
  phases). Areas 116 and 145 have `[17]` groups present but runtime-inactive (zero VRAM/CLUT diff
  across the capture series — presumably gated by an unmet `cond` or targeting a non-VRAM
  resource); they were left without a generated asset rather than guessed. D4 is now complete;
  the only remaining `[17]` work is the full byte-level semantics of the general player, needed to
  cover any future area without per-area RE.

D3 (blank clusters, 11 areas: 004, 102, 117, 135, 136, 137, 139, 140, 150, 167, 188) is fully
negative: one ground-truth dump per area, checked with `extract-runtime-water`, found zero runtime
texel cells anywhere — the referenced cells are exactly as empty live as in the EMI. The
sea/river VRAM-upload class has no further members beyond 104, 141, 156. Nothing to fix.

D2 (deck gaps, areas 147, 170, 197, 139, 167) is also negative for the areas checked: at 170, 197
and 147 the flagged gap tiles sit at the map edge, where a roof deck legitimately ends (a detector
artifact, sometimes `col=−1`). The only true interior gaps are area 139 (86,10)/(62,62) and area
167 (6,37); ground-truth photos there show the character standing in plain black — the original
draws nothing at those tiles either. Nothing to fix.

**Two regressions were introduced and fixed while building this pipeline:**
1. Generalizing the animation build to decode tiles directly (instead of through the full area
   renderer) accidentally pulled river-override areas into the sea/lava VRAM-upload build and
   painted their river cells with sea-phase texels — areas 016 and 033 rendered their rivers
   lava-red for a period. Fix: exclude river-seed cells from the overworld VRAM-upload phase set;
   the stale 016/033 water assets were deleted, restoring static-correct rivers there (their true
   animation is a third, still-unresolved mechanism — see "Open," below). Areas 045/065 keep their
   real sea cells (11/14 entries) unaffected.
2. The same direct-decode switch overlooked that the tile decoder returns RGB triples
   (`Int16Array`, 16×16×3), not 15bpp halfwords. Interpreting one RGB pixel as a single packed
   halfword corrupted every cell-animation sheet built after the switch (overworld seas 045, 065,
   087, 088, 115, 121, 151; lava 052; all sparkle-class areas). The river-CLUT sheets were
   unaffected because they used the newer feature-sheet path. Fixed in `build-water-anim.ts`
   (proper RGB-triple output); all affected sheets were rebuilt.

**Perceptibility threshold.** Both extractors (`extract-clut-phases`/`build-water-anim` and
`extract-anim-phases`) must filter capture noise: a ±1-bit CLUT or texel difference between dumps
is common background noise from unrelated entity streaming. A pixel counts as changed only above a
channel-sum diff of ≥24 (8-bit channels) or ≥3 (5-bit-sum comparison); a tile/cell counts as
animated only with ≥4 such pixels per hardware step. Ground truth confirms: CLUT pairs at areas
056/126 with 0 px of recon diff are exactly the imperceptible-noise case; after thresholding, only
real animations remain (area 002 dropped from 25 flagged map entries to 0, keeping only 1 real
feature key — the Dauna rubble glow is feature-side, not map-side). Some VRAM regions
(`cl(448,480..511)` at area 126; `cl(768/800,·)` texel banks at 192, 194, 153, 154, 053, 155) are
entity/streaming staging areas, not animation source data — they stay in the extracted JSON but
only take visual effect where an actual map entry or feature key references them post-threshold
(area 126: 4 feature keys legitimately do).

**Confirmed non-bugs:**
- The Mt. Glaus (022) slope-roof decks (178 quads) visibly pulse in the original too — ground
  truth recon diffing shows roughly 96k px of change over the whole slope, a rain/mountain shimmer
  effect. The browser already matches it.
- Area 011's fire CLUTs (8bpp palettes 3–6, VRAM rows 486–489, up to 6 phases) cycle along with the
  river mechanism, producing map-side fire flicker (29 entries / 44 tiles + 12 feature keys) in
  addition to the separate particle-sprite fire (see "Fire particle sprites," below).

### Map header table [20] — state texture patches

**Format.** The `[20]` table is an array of groups, `u16[20]·4` addressed: `[head:u16 =
0x8000|flags][wordcount:u16] [3-word entries]`. Each entry is
`[row|col|type|subOffBits][old:u32][patch:u32]`. The runner, `0x801546ac`, executes any group
whose header has bit 0 ("pending") set, then clears that bit. Applying an entry patches the word
at `map + texIdxPtr·4 + texIdx(tile)·4 + ((w0>>14)&0x3c)` — the 4-byte texture entry of the target
tile (`subOff` selects which word, for rect/pair tops). `old`/`patch` are the before/after values
of that entry, e.g. a door's closed/open texture. The group's `cond` field is evaluated by consumer
`0x80157970` through the shared visibility interpreter `0x801560f0`.

**Trigger and proximity mechanic.** Function `0x801551ec` arms a group: it stamps a feature
record's type from `0x23` to `0x24` (see the dispatch-table patch cycle above) and activates the
`[20]` group referenced by the low 16 bits of the record word. Its callers — `0x801b1330`,
`0x801b1518`, `0x801b6da4`, `0x801b8f0c` — are the player interaction/proximity handlers. Scanning
all `t23` feature records for this group reference found 536 candidate triggers, 478 of which
resolve to an actual `[20]` group (recorded as `tile-patches.json`'s `triggers` field). In the
browser, door patch groups fade in/out as the player comes within 1.5 tiles (`tickTilePatches`).

**Catalog.** `extract/build-tilepatches.ts` writes `public/gamedata/tile-patches.json`. The group
walker must not stop at the first group header lacking the `0x8000` bit — the runner actually
walks every group via a word-skip, and only a `head==0` word ends the list. An early version of the
extractor aborted too soon and captured only 76 areas / 243 groups / 676 entries, silently
dropping roughly 60% of the data; after the fix the catalog holds **404 groups / 1655 patches / 99
areas / 670 triggers, with a 100% group-match rate**. Two earlier interpretations of this whole
system — "entity door overlay list" and "bridge height patch" — are both wrong.

**Playable in the browser.** `extract/build-tilepatch-tex.ts` renders the AFTER-state tiles with
full engine semantics: it patches a copy of the map exactly as the runner would, renders the area,
and cuts out the affected tiles (648 tiles / 75 areas), writing `public/tilepatches/areaNNN.png`
plus an `atlasX` field in the catalog. ⚠ Extraction order matters: `build-tilepatches` must run
before `build-tilepatch-tex`, which depends on `atlasX`. A world feature toggle, "State patches
(switches/doors)," overlays the after-state quads on top of the baked before-state tiles (off by
default). Debug hook: `__bof3.tilePatchCount`.

### Hole tiles (`tileTexIdx==0`) — what the original actually draws

An early theory held that the renderer fills `tileTexIdx==0` ("hole") tiles by substituting or
duplicating neighboring grid records. This is disproved at the map-tile level: the on-screen
window grid (`0x8012c000`, 28×55 cells) is a strictly linear iso window — grid column steps
`(wc,wr)+=(+1,−1)`, each row a diagonal half-step — and hole tiles simply appear as **empty grid
cells**, with no substitute or duplicate record anywhere. This was confirmed uniformly across
every one of 11 swept hole areas (002, 034 ×3, 155, 170, 135, 130, 127, 187, 086, 131): zero hole
records in any of them. Whatever visually fills a hole tile in the original — and in several areas
something clearly does — comes from a different rendering layer, never from the map tile grid
itself.

Ground-truth classification of what actually appears at hole tiles, per area:

| Class | Areas | Original appearance | Handling |
|---|---|---|---|
| BLACK | 155, 130, 187, 131, 135, 034 (section 1), 028, 120, 147 (center) | character stands visibly in plain black | already correct — render nothing |
| SKY | 127, 082 | vertical sky gradient (127: `#00b4e3`→`#001334`, large `pg(0,256)` quads `CLUT 448/451`, measured 205×203 px; 082 presumed the same construction) | cosmetic gradient backdrop per area |
| DECO FILL | 002, 034 (Dauna family) | rubble heap plus the green dragon body (034 has a night variant), from large deco quads (`pg704`/`pg576`, plus a 465×465 scene backdrop `pg(960,256)` `CLUT(1008,511)`) | fully explained as ordinary object meshes, not a separate format — see "Object mesh system" below |
| STRUCTURE | 170 (container decks), 086 (ship, covers the character) | large structure quads over/near the character; remaining gaps stay black | same handling as DECO FILL |

Implemented: a `SKY_AREAS` list in `main.ts` (082, 127) renders a vertical gradient as
`scene.background` instead of black; every other area keeps a black background. The hole-anchor
check (detector D1, see above) independently confirms zero anomalies for object placement over
holes.

A now-removed browser-only system, the "hole-fill lid" (opaque black planes placed over any
enclosed component of `tileTexIdx==0` tiles in `terrain.ts`), had been added before the BLACK
classification above was confirmed and caused visible bugs of its own: a "chunky block" under the
area-023 bridge and "black under the bridge" at areas 060/065. Since the original leaves holes
empty (see above), the lid system was obsolete and was removed entirely; area 023's river now
flows freely under both of its bridges, and area 060's sea and floating bridge render correctly.

An earlier, separate "big quad" theory held that DECO FILL/STRUCTURE areas were drawn by a
dedicated large-image system (candidate page/CLUT sets `pg512`/`704`/`960`). The clearest example
was area 112's satellite-dish tower: a `408×327 px` quad, `pg(512,256)` `CLUT(512,288)`, appeared
to cover the whole tower including its roof floor as one drawn structure image, suggesting a
missing-big-quad explanation for "untextured roof floors." This theory is fully retracted (see
"Object mesh system," next): the 112 quad turned out to be a stale RAM artifact from a savestate
scan, not something the game actually draws, and the DECO FILL/STRUCTURE look elsewhere is the
same 40-byte object-mesh format used everywhere else, just spawned through a different code path.

### Object mesh system (40-byte spawn records)

The 40-byte object-mesh record format was already known, but for areas 002, 034, 086, 112 and 170
its spawns live in the `0xfN` sub-streams of the init script that the original static parser
skips, and the record blocks are OVL-internal (no `0x117000` mesh block present for those areas).
A third scan path added to `build-meshes.ts`, an **`ffff` anchor scan**, finds them: `OBJ_SPAWN`
records are 14 bytes ending in an `ffff` terminator, so every `ffff` anchor found anywhere in the
OVL is read backwards as a candidate spawn and strictly validated — `hi ∈ {a,b}`, coordinates land
on the map, the `plcBase` entry count is `1..200`, `meshPtr` falls inside the OVL/mesh window
(checked via `readRecords`), the record's `typ` is `0..15` (this is the CLUT-column formula), and
`subtyp ∈ {4,6,7}` (spurious hits at recurring `(1,0)` corners with `subtyp` around 29 are anchor
coincidence and are filtered out). Results are deduplicated by `(x,z,plcIdx)` and against the two
older scan paths.

**Result: 57 areas with objects, up from 21.** Area 002 alone yields 2×28 records: the rubble heap
at (49,67) and the green dragon body at (44.5,37.5), verified congruent with the Prima Guide
render — fully solving the area's hole-tile mystery with no separate "big-quad" system; the
DECO FILL/STRUCTURE hole classes above are simply `ffscan` object meshes. Area 170 (this is
Station Myria — an earlier "Steel Beach container" label for it was wrong) yields 6 lab apparatus.
Area 034 confirms the (49,67) spawn. Unrelated areas (077 orb, 121/135 init/save objects) were
checked unaffected.

**The `0xfN` streams are story branches.** Identical spawn sets repeat once per branch — e.g. area
067 has the trio `(53.5,87)`/subtyp 6, `(56.5,86.5)`/subtyp 4, `(52.5,89.5)`/subtyp 4 five times
over; area 015 has `(14,62.5)` eight times with alternating subtyp 4/6. The scan simply dedupes to
the first occurrence. Which branch represents the "base" (new-game) state is tied to the
story-phase/branch-selector system (see "Map object spawn systems" below) and was, at this point,
still open; state objects are deliberately kept toggle-visible rather than guessed at.

**112 "dish" — the worked example of a wrong lead corrected.** The relay deck there is ordinary map
tiles (39 tiles, `pg320`/`CLUT484`, around the character) in dark rivet metal; a Prima Guide scan
of the area is brightened and had given a false "untextured" impression. Platform and red lens
already render correctly. The only real remaining delta, missing edge plates, needed **no fix**:
ground truth shows the original also leaves the edge voids undrawn, and texel plus `CLUT(0,488)`
data is byte-identical between a live capture and the browser's reconstruction. The `408×327 px`
quad that had earlier suggested a "big quad" system for this tower (see "Hole tiles," above) was a
**stale RAM artifact** from a savestate quad scan — confirming the rule that a live savestate scan
must never be trusted to answer "does the game draw here"; only a fresh GPU dump or photo can. The
console next to the dish is an ordinary sprite, descriptor key 145 (entity at (12,88), 8 programs).

**086 crane — narrowed to per-area code, not a mesh.** Systematically ruled out: not map tops (the
grid record slots there carry only deck/porthole UVs), not wall edges (zero void walls), not
object meshes, not a map-header texture patch (the only live diff is a story-driven walkability
change, walk bytes `0xa1→0x50/0x21`, unrelated to graphics), not the slot pool or RAM FT4/GT4
records (zero hits). The area's 6 feature roofs there are only the crane's poles (12/64-tile
bands, a 3×8 texture, each position carrying a `0x14xx`/`0x34xx` story-variant pair). The crane
BODY itself is drawn by per-area overlay code at `0x801f2c00` — the same class of mechanism that
draws the area-067 bridge (next section). Its texels already exist in the EMI (page 2, `v192+`);
extraction will be trivial once the drawing code itself is worked out. A broader ground-truth
capture of the same structure measured 158 quads total, `pg(576,256)`/`CLUT(0,484)`, 8bpp: a deck
"rapport" grid (cells `u16-47`/`v0-47`, roughly 53×6.5 px per cell) plus vertical pylons/masts
(`u176-223`/`v112-159`, about 80–100 px tall); the boom itself (the "yellow-black" part singled out
above) sits at `u48-95`/`v192-255`, 17 tiles large. The per-area overlay code contains neither
recognizable immediate constants (`0x7900`/`0x119`) nor a pre-built quad package for this
structure, so it is presumed to be built by a generic quad builder reading a parameter table;
either disassembling the renderer or fitting the flat deck grid to a planar projection (the deck
is flat, so this is a tractable screen-to-world fit) are the two open routes to a static bake.

**Smaller object-mesh diagnoses from the same audit round:** area 055's Yggdrasil renders from a
90-record `ffscan` mesh as a lying giant trunk; whether lying down is the correct story state or
an assembly error needs ground truth (the area's warp target fails silently, so no capture exists
yet). Area 068's well renders correctly from 8 records; a report of it being wrong may instead have
meant a missing roof frame nearby. Area 023's "upper bridge over the abyss" is presumed to be
correct as rendered.

### Runtime-drawn per-area geometry (bridges, cranes, and similar "painted" objects)

Several visible structures are not stored as static geometry at all — they are built at runtime by
per-area overlay code (candidate entry `0x801f2c00`) and painted directly into the GPU command
stream. Area 067's bridges and shelters are the fully solved example.

An earlier diagnosis, "the 067 bridge is drawn by the TYPE-0 feature system," is wrong: a texture
match filter (`pg704`/`CLUT96`) had actually matched the area's water quads (the CLUT-cycling
class from table [17]), not the bridge. Isolating a ground-truth dump by its actual GPU class,
`pg(320,256)` / `CLUT(0,484)` / 8bpp, reveals the complete bridge — deck, framework, A-legs. Static
wooden roof pieces exist additionally at the same tiles (ramps and end segments); the original
paints the runtime pass painterly on top of them. The same runtime class also paints, in area 067,
two north shelters (roofs over an archway), a gorge footbridge, and a footbridge on the northwest
arm; a nearby hut already renders correctly from static data and is filtered out of the bake. None
of these runtime quads exist in any static inventory — a UV scan across all EMI subfiles,
including the `0x117000` mesh block, found zero hits — so they are only extractable from a
ground-truth GPU dump.

**Bake pipeline** (`scratchpad/w13b-bridge-solve.ts <dump> [--append] [--deckh H] [--corr
c0,c1,r0,r1] [--decks "h:box;…"] [--keep box]`) turns the runtime quads of one dump into world-space
geometry in four steps: (1) an affine camera fit (east/south/vertical/origin), seeded from the
known screen diamond and refined by Hough consensus over the origin — every ground-quad-to-tile
hypothesis casts a vote, the densest peak is the true correspondence — then least-squares
refinement (residual ≈6.5 px); (2) a deck pass that inverts flat quads exactly onto the known deck
height (area 067: 5.75); (3) an anchor BFS for vertical/sloped quads, matching shared screen
vertices within ≤3 px so a flat quad with a known anchor is inverted onto the anchor's height
(this prevents 90°-axis flips); (4) edge snapping between coplanar neighbors. Results accumulate
across multiple dumps into a seed file (`references/re/runtime-geo-067.json`, source tagged per
quad); `extract/build-runtime-geo.ts` merges the seed into `features/area067.json` under a new
`rgeo` field — full 3D quads with per-vertex UV (needed for the trapezoid UVs on the bridge's
A-struts) — and pulls the matching texels from the dump's VRAM into the feature atlas under `rg`-
prefixed keys (idempotent). `features.ts` renders `rgeo` quads without the normal renderable gate.

**Painter-vs-Z-buffer lesson:** the original submits coplanar duplicate geometry in draw order —
the runtime class after the static class, and overlapping deck planks after each other — and PSX
painter's-algorithm rendering resolves the overlaps by submission order alone. Rendering the same
data through a Z-buffer instead produces a 16px checkerboard/"parquet" artifact (confirmed by a
software re-render of the baked data without Z-fighting, which came out clean). The fix bakes
`rgeo` geometry globally `+4/128` of a tile above the static geometry, plus a small monotone
per-layer lift in submission order.

A related but more widespread Z-fighting problem affects ordinary lattice-fence feature panels
(`PANEL_DEF` records; areas 002, 015, 017, 021, 026, 027, 037, 038 — area 015 alone has 103 fence
pieces): these panels are positioned exactly coplanar with map wall/floor surfaces, which the
painter-order original renders without any Z-buffer artifact but which Z-buffered rendering flickers
between. Fix: a `polygonOffset` of `−1`/`−2` on the feature material (`render/features.ts`)
deterministically pulls fence/panel features in front of the coplanar map surface.

Ground truth confirms: the character walks on top of the bridge deck (nav heights of the corridor
tiles are 5.75, i.e. deck level, in the original too); framework, legs and roofs match. The bridge
legs reach down to about `h≈−1.9`, below the riverbed, in the original; the painter renderer simply
never shows the part hidden behind nearer geometry, while the browser's Z-buffer achieves the same
result without any clipping being necessary.

**Overlay false-positive veto.** A separate, earlier registration bug in the same overlay system
produced 3 floating phantom quads each in areas 000 and 007. General fix rule added to
`build-overlays`: any overlay-fit quad whose registered top height is greater than 0 (i.e. it
would sit below the walkable surface) is always rejected, alongside the existing `FP_VETO` check.

**007 "double door" — a related phantom-façade bug, unrelated to runtime geometry itself, fixed in
the same round.** A reported second door on a path was a complete phantom façade (door, window,
gable) produced by the overlay-registration system: a screen-fit step had matched tiles
c26-31/r23-25 using texture data pulled from a ground-truth dump of a *different* area (build-
overlays' 80%-registration-similarity threshold is unreliable between texture-similar areas — a
dump from area 000's mountain section registered 84 tiles into area 007). A same-area ground-truth
photo shows the path is clear; the real doors are baked into the map walls. Fix: rebuild area 007
using only area-own dumps, extend the false-positive veto, and add a guard against empty dumps.
Rule derived: for villages that share a tileset (000/007/016/033), always run `build-overlays`
with an explicit per-area dump list, never automatic cross-area matching.

The same GT-dump bake pipeline applies to other known per-area-code object classes still
unresolved at the geometry level: the 086 crane body, a checkpoint substructure at area 060, a
conveyor belt at area 049, and a crystal skirt at area 198 — each needs only 1–2 targeted
ground-truth dumps. A URL parameter, `?x=&y=` (`main.ts`), sets a custom start position for quick
sweep/verification shots.

### Backdrop / multi-quad TYPE-0 records

An earlier idea of a distinct "procedural backdrop packet class" does not exist as its own system.
Three separate findings replaced it:

1. **Camp trees (areas 053/090) are ordinary star objects.** The suspected special generator turns
   out to be the already-known star handler, `0x801566a0` (confirmed in disassembly: `ori
   v0,v0,0x78c0` after `lbu`/`srl 4` is the star CLUT formula at `0x80156948`). The underlying star
   records had already been extracted into the feature data; the browser's `renderable` visibility
   gate had filtered them out because they sit on void tiles at the meadow's edge. Removing that
   gate — the game draws every star record regardless — makes all 4 large and 2 small camp trees
   render from real map data (this is the same lesson as the area-107 edge backdrop case). The
   older raster deco-tree assets (`camptree*`) were rolled back but are kept on disk.
2. **Area 049's Chrysm Crystal is a multi-quad TYPE-0 record — a new sub-format.** A type-0 (roof
   class) record can, after its normal header vertices, carry chains of `[texture word][4
   vertices]` groups, 5 "texWords" per group, starting at group index 0 (`u16 idx` = z as signed
   16-bit; bytes 2/3 = y/x as signed 8-bit). One 41-word record at (50,55) contains all 8 crystal
   facets this way. `build-features.ts` now decodes the chain (an early version started at index 1
   instead of 0, a one-group phase error that made nearly every facet fail the `atlasForWord`
   lookup); the feature-semi harvest catches the facets as class S1. All 188 areas were rebuilt
   after the fix.
3. **Area 024's dome walls are ordinary individual roof quad records**, already exported through
   the normal path; a separate "stereo bake" step covers their sloped faces.

What remains unexplained from the old "backdrop" suspicion: 168 glass panels (`pg576`, with no
page-3 texture route — currently covered by the stereo/vertical bake, but their true record source
is still unknown) and small remnants such as a light-Gouraud effect at area 058.

⚠ **Bake order rule**, found as a regression in this same round: `build-features` rewrites
`public/features/areaNNN.json` from scratch and discards any `rgeo` runtime-geometry quads (see
above) in the process. After every features rebuild, `build-runtime-geo` (over all seeds) must run
again, and only after that `build-feature-semi`. Correct order: **features → runtime-geo →
feature-semi.** Skipping this regressed the judge's per-area mismatch count (area 168 from 20 to
42 different quads, area 002 from 9 to 37); re-running in the correct order restored 18 and 9
respectively (see "Verification methodology" below for what these counts mean).

### Map object spawn systems (INIT script)

Chests, fish/ambient life, and non-story NPCs are not separate systems — they are three different
opcodes inside the **same area INIT script**, at struct field `+0x00` (previously unread; older
tooling only read fields `+0x14`/`+0x18`/`+0x1c`/`+0x3c`).

**Interpreter structure.** The outer loop (`0x801a41bc`) dispatches ops below `0xf0` through
`0x801a4e78`, using a hi-nibble jump table at `0x80195b54` and a per-hi-nibble byte-length table at
`0x801c86c8` (hi `0`=17 bytes, hi `1`=16 bytes, hi `9`=13 bytes — the chest op, hi `E`=7 bytes —
the ambient op). Ops `≥0xf0` go through a separate "f" handler (`0x801a4680`, jump table
`0x80195a64`) that implements **story branching**: `f6 NN` phase blocks, `fd`/`fe` sub-gates, `f0`
event sections, `f4 SS` switch-select, `f1` a 2-operand op, `ff` end-of-stream. The interpreter
loads a phase byte from `0x80146870` (sign-extended) into a cache at `0x8014820c` at the start of
each run; op `fa` reloads it.

- **`f4 SS`** (executor `0x801a4798`) selects a condition function `fn[SS]` from a table at
  `0x80182648` (`andi 0x1f`). `f6 NN` is then a case (the condition function reads `NN` and
  compares; the first true case wins), `f7` is the else-branch, `f8` ends a case, `f5` ends the
  switch; switches can nest.
- **Condition functions:** `fn0`/`fn14` read the phase byte `0x80146870` itself (the same byte the
  roof interpreter reads as code `0xfa`); `fn3`–`fn6` read story bytes `0x80146864`–`0x80146867`
  (identical to NPC-VM variables 3–6); `fn7` reads `0x80146874`; `fn8` reads `0x80143f03` (the same
  byte the roof interpreter reads as code `0xfd`); `fn1` reads a u16 at `0x80143f00`; `fn12`/`fn13`
  call out to helper functions (suspected flag tests, not read). Ground truth: with
  `0x80146870==1`, exactly the `[f6 01]` block is the one that executes — confirming the selector.
- **Two extra context bytes were identified by save comparison:** `0x80143f00` (u16) holds the
  **current area id** (confirmed via saves: a door area shows 7, a chest area shows 102, a fishing
  area shows 45) — this is what `fn1`/selector 1 tests. `0x80143f03` holds the **entrance index**
  used to enter the current section (confirmed values: 2 for one hideout entrance, 1 for a
  save-point entrance) — this is what `fn8`/selector 8 tests (713 uses across all spawns), i.e. it
  selects cast per entered map section rather than per story chapter. Selector-use histogram
  across all extracted spawns: selector 0 ×776, selector 8 ×713, selector 4 ×25, selector 6 ×1.

**Entity spawn ops (hi nibble 0 and 1).** Format: `[op][key:BE16][00][b4][01][halfX][col][halfZ]
[row][…]`, written into the live entity table at `0x80146888` (30 entries × 152 bytes; a
scratchpad counter lives at `0x1f800000`). `key` is passed directly to the sprite loader
`0x8014de8c` (the MODEL_SET kernel): `X = col<<16 | half·0x8000` goes to entity offset `+0x34`, `Z`
analogously to `+0x38`; `op&0xf` goes to `+8`; `op>>4` (the type byte) goes to `+6`. The low nibble
of the op is the **start program index**, written to entity `+8` — ground truth: the three McNeil
children under key 141 (subtypes 0/1/2) live-play exactly band programs 0/1/2 (violin, trumpet,
dance). Two candidate fields, `+0xa` (op byte `0xa`, entity `+1`) and `+0xc` (entity `+2`), were
tested as a possible facing-direction field and ruled out — they are constant (4 and 0) across the
whole ground-truth sample. The still-open direction candidate is spawn field `+0xa` itself read as
`b10` (observed values 3/4/6, written to entity `+1`); field `+0xd` carries behavior flags (entity
`+7`: bit0 ORs `0x40`, bit2 and bit6→`+0x48` via helper `0x801a4c10`); the low nibble of `+0x3`
selects a display mode (entity `+0x5c`); bit `0x80` of `+0x3` ORs entity flag `0x20`. Two outlier
NPCs (keys 759/760) have sprite bands with only 7 programs (header size 14) but a live start
program of 7 — out of range; the extractor falls back to program 0 (idle) for these.

A spawn's `key` resolves statically to a sprite descriptor (via `containerEntries` +
`parseDescriptors` field `b7`) — e.g. McNeil's villagers use keys 141 (children), 757/758 plus 5/6
(a palette-variant pair), 156, 127, 759/760, 152/154/159, and 466 (an adult-phase variant);
descriptors sharing the same `b7` are CLUT variants of one figure.

**Pipeline:** `extract/build-npc-spawns.ts` (`npm run extract:npcspawns`) writes
`public/npc-spawns.json` (1269 spawns / 100 areas, each tagged with its phase/block/gate context)
and `public/npcsprites/areaNNN/kKEY.png` (632 frame-0 sprites at first, later 824 once animated —
see below — field CLUT resolved via the b6-mode formula, anchor `ax`/`ay` per sprite). Chest
positions are deduplicated against this set. In the browser (`entities.buildNpcSpawns` + wiring in
`main.ts`): the core population (the first ungated phase block) renders under a "NPC spawns
(static)" toggle with sequential dialog boxes; every other phase sits behind a second toggle, "NPC
spawns: all story phases" (off by default — otherwise, e.g., every story configuration of the
sitting camp-153 party stacks up at once; the sprites themselves are correct — Rei/Nina/Garr
sitting plus a fish-grilling figure, key 602). A separate, older `spawns.json` layer from manual
SCENA research is kept only where no INIT spawn already covers the same NPC.

**Story-phase visibility in the browser:** every spawn carries a `cond` chain,
`[[selector,case],…]`. `npcSpawnVisible()` in `main.ts` evaluates it: for selector 0 or 14 it uses
the chosen phase; selectors 3–8 default to case 0 (a new game's state); anything else stays
conservatively invisible. A panel, "Story phase (NPC cast)," lists the `f6` phase values available
for the loaded area (`'auto'` picks the earliest ungated block, matching default game behavior).
Example, area 000: phase 1 shows child-era McNeil (10 ground-truth-matched villagers plus
conditional extras), phase 8 shows a later cast (the 466-key adult group).

**Animation.** The low nibble of the spawn op is also the entity's **start program** — ground
truth (McNeil children, key 141, subtypes 0/1/2 = violin/trumpet/dance) confirms the mapping.
`build-npc-spawns` renders, per `(key, program)` pair, a full frame strip of every sequence step
(shared bounding box; loop metadata `n`/`ticks`/`loop` read from `programAt`), producing 824
sprites; `entities.buildNpcSpawns` animates them via texture offset (each step lasts `ticks·20 ms`
at a nominal 50 Hz, looping from `loopStart`, phase-offset randomly per instance). If a subtype
does not resolve to a valid program the renderer falls back to program 0 (idle); some non-child
villager subtypes (5, 7) still show a walk pose in this fallback, and their correct semantics are
unconfirmed against ground truth.

**Speech-bubble coupling:** the "!" emote (EmoteFX key 24, program 0) fires when the player is
within 1.5 tiles of a dialog-carrying spawn NPC (`main.ts` main loop; anchor height uses the
spawn's own `ay`; suppressed while a textbox is open). NPCs without extracted dialog show "?"
(program 2) instead.

**Story chapter counter (the phase byte's writer).** `0x80146870` is a global story-chapter
counter. Its only confirmed writer is the increment routine `0x8019ff2c`–`0x8019ff58` in the field
engine: it adds 1 when bit `0x80` of `0x80146871` is set (an "advance requested" flag, cleared
afterward at `0x801a79d8`); the value `0x10` is skipped in the sequence, so chapters run
`0..0xf, 0x11, …`. The same routine copies a camera base from `0x80182638` to `0x801481e0`. An
extensive search for whoever sets the advance-request bit found only 8 already-explained write
sites (the increment itself, its clear, two dialog-flow bit-0 setters at `0x801fc5c0` and
`0x801fa520` that also touch a talk-state byte and an FSM textbox request, and two related
sub-state bytes at `0x146872`/`0x146875`); no direct `sb rt,0x71(...)`-style store to the bit
itself was found in the scanned overlays or SCENA scripts, and two generic `setBit` helper callers
that looked promising instead targeted a *different* byte, `0x80144e90` (story flag block 1),
reached through a pointer stored at `+0x6c` of the dialog-state block. **A higher-level answer was
found separately:** the advance request is issued by the per-chapter SCENA scripts themselves —
each chapter's script sets the control bit (and a paired dialog-state write, and a `story[0]:=8`
assignment) as part of one of its own event handlers, and the bit is cleared again at SCENA init.
So chapter-end story events request the advance, and the next area load performs the actual
switch; the manual browser "story phase" selector remains the correct way to choose among phases,
since live progression is not simulated. Persistence of the chapter counter across a save/load was
not checked (the byte lies outside the known save memcpy range, and all captured save states so
far are chapter 1).

#### Chests (INIT op `0x90`)

`key 75` is a **global** chest descriptor: a cross-area, CLUT-independent frame-hash scan found
that practically every area carries the identical chest sprite under this key (program 0 = closed,
32×32; program 1 = open, 40×40); `key 79` (item sack) is local per area. The field CLUT of the
chest resolves through the b6=2 formula to `(160,496)`. Live RAM structure: entity records
(marker `0x602`) at `0x80146d70` — `[0602][…][key:u16][col:u32][row:u32]…` plus a sprite SEQ/
program pointer into the shared sprite band at `0x800d3800`; positions match the live entity table
at `0x80146888`.

**Static source:** INIT op `0x90`, 13 bytes: `[90][00][key:u16][00][col][00][row][flagIdx]
[itemId][itemType][b11][b12]`. `flagIdx` is a running open-flag index; `itemType 0xff` is
suspected to mean zenny (money); `b12==1` marks item sacks. A branch-agnostic signature scan
(deduplicated across the `0xfN` story branches) over all 200 areas, after later fixing the scan's
end-of-segment cutoff (the init segment can sit *behind* the struct, past an earlier, too-short
`min(S,…)` bound) and widening the key whitelist to `{75-80,85}`, finds the final count: **147
chest/sack spawns across 69 areas**, including early areas 000 and 005–052 (e.g. a treehouse holds
a chest at (50,8) and a rice sack at (38,6)).

Pipeline: `extract/build-chests.ts` (`npm run extract:chests`) writes `public/chests.json` plus
`public/entities/chest0.png`/`chest1.png`/`sack0.png` (field-CLUT-correct sprites). Browser:
`entities.buildChests` places camera-oriented billboards (foot height from a terrain sample, scale
0.75 tile per 32 px); `main.ts` wires them into the entity rebuild/lookAt loop.

#### Fish and ambient life (INIT op `0xe0`)

`walk=0xf5` marks **fishing interaction tiles** — shore clusters present in every water overworld
area (016, 033, 045, 065, 087, 088, 115, 121). This is only the interaction tile; the fish itself
is a separate entity swimming nearby. Ground truth: a jumping fish draws as RECT primitives,
`pg(448,0)`, `v=200`, with a wandering `u` coordinate (new frames streamed into the band) and
`CLUT(128,497)`, resolving to an entity record (marker `0x602`) with a Q16 world position, e.g.
`(42,46)`. The ambient-class entity layout has no key field at all; its program pointer sits
directly at offset `+0x28` into the shared sprite band.

**Static source:** a second INIT op, `0xe0 [type][00][col][00][row][param]`, 7 bytes, living in
the same `0xfN`-branch init segment as the chest op. `type 0x0e` = fishing-spot fish (ground-truth
verified at area 045: `(55,42)`, `(62,55)`, `(42,46)`); `type 0x18` (196 occurrences) = small
ambient life/birds; `type 0x17` (36 occurrences); types `0x03`/`0x16`/`0x3e` appear once each.

Pipeline: `extract/build-ambient.ts` (`npm run extract:ambient`) writes `public/ambient.json`
(254 spawns / 56 areas; 17 fish across 7 fishing overworlds: 016, 045, 065, 087, 088, 115, 151);
fish frames are segmented from the dump band into `public/entities/fish0.png`…`fish7.png`
(surfacing/arc/falling/diving/ring poses). Browser: `entities.buildFish` plays the jump sequence
`[0,2,1,3,4]` at 8 fps on a 5 s period, phase-offset per fish, behind an ambient toggle.

A side finding, not implemented: a texel segment chain found in one fishing-area dump (around
`cl(176,483)`) belongs to the area's moving train, a separate rail-ambient object. Bird types
`0x18`/`0x17` are not yet wired to these real `0xe0` spawns; the current bird effect
(`BirdFX`) still runs on a placement heuristic.

#### Save points (basin + 4 pillars)

The round basin with four pillars found in large dungeons is the game's actual save point ("Record
your progress" pad) — an earlier label for it, "relay point, not a save point," was a visual
misjudgment. Evidence: these basins occur only in the largest dungeons (areas 036, 059, 100, 112,
116, 143, 145, 146, and 071); the associated NPC-VM pulse program for each ends in
`CALL_NATIVE2 fn[0]`, which is the save-dialog call; `GLIDE_Z`/`VIS_TOGGLE` ops implement the
pillars' glow behavior.

The basin and pillars themselves already render correctly as ordinary map geometry (a basin
texture plus pillars raised `+8` in terrain height) — only the pulsing pillar-top glow was
missing. Its NPC-VM signature: `SET_POS` + `SLOT_BIND` + `OFS_ADD`/`LOOP` +
`SCALE_SUB×4`/`SCALE_ADD×4` loops; a half-tile anchor is encoded directly in the `SET_POS` bytes
`[0d][col][frac][row][frac]`, where `frac 0x80` means a half-tile offset.

Pipeline: `extract/build-savepoints.ts` (`npm run extract:savepoints`) writes
`public/savepoints.json` (29 anchors / 9 areas). `entities.buildSavePoints` places additive radial
glow billboards on the 4 pillar tops of each basin; pillar tops are auto-detected as local terrain
maxima (`tileTop > floor+0.4`) around the median center of each anchor group (an outlier filter is
needed, since a few unrelated pulse actors share the same VM signature, e.g. a console at area 100,
`(77,51.5)`). The glow pulses ±14% over 1.2 s with a slight hover, and has an ambient toggle.

Two related side findings from the same signature search: INIT op `0x90` `key 85` is not a save
object but a **fishbone decoration** (ground truth: area 107, `(35,112)`); and a 28-area VM
signature, `SET_POS, POSE, WAITVAR(story[0]==99), END`, marks **story-gated reserve slots** that
are correctly invisible in a fresh game (ground truth at area 023, `(40,90)`: nothing visible) —
these are deliberately not rendered.

### Walkability byte codes (`walk[]`)

The per-tile navigation byte is read everywhere through one function, `0x80166f64`
(`walk[y·cols+x]`, base pointer `0x8014931c`, `cols` at `0x80104000`). Method for mapping unknown
codes: build a histogram of all `walk` values over all 200 areas, locate concrete tiles carrying
an unmapped code, find the disassembly call sites that branch on it, then confirm behavior against
ground truth.

| Code | Meaning | Status |
|---|---|---|
| `0x70` | Ladder | confirmed |
| `0x80` | Hot floor (visual only) | confirmed |
| `0x81` | Hot floor, second zone | confirmed |
| `0xd0` | Steep slope / slide | slope geometry confirmed, player behavior open |
| `0x89` | Conveyor belt | suspected, open |
| `0x30` | Hop obstacle | suspected, open |
| `0xfd` | Clear-on-entry (self-clearing obstacle) | mechanism confirmed, visual open |
| `0x11` | Flag-conditional block | confirmed |
| `0x20`–`0x2d` | Layer parity (bridge over/under) | mechanism located, full semantics open |
| `0xc0` (at doors) | Door threshold trigger | pattern confirmed, animation open |
| `0x84` | Station-zone marker | unmapped |
| `0x91` | Overworld isolated patches | unmapped |
| `0x40` (indoor context) | Lava flow / decorative surface, blocked | confirmed |

- **`0x70` ladder** (ground truth: McNeil Manor area 015 `(29-30,16-17)`, 12 tiles; Mt. Myrneg area
  051, 56 tiles; also present in 127/026/102/137/086…): the character hangs in a climbing pose
  against the wall; the D-pad axis along the ladder climbs at roughly half walking speed (world-N
  = up, world-S = down). The climb animation is PL dispatch `0x3c` ("d3c", a 4-frame grip cycle,
  back view, present for all 7 party members; `0x3d` is Ryu's variant). The move classifier
  (`0x801bb648`) only allows a `0x70` target tile when the move *starts* from a `0x70` tile too,
  plus a height check — i.e. you can only enter a ladder from its own ends. Implemented in
  `player.ts` (a climb movement mode, speed ×0.55, a `STEP_MAX` exception for the `0x7_` family)
  and `main.ts` (`buildPlayerSpritesFor` loads the `d3c` band).
- **`0x80`/`0x81` hot floor** (Mt. Zublo, areas 102/103 — a visual effect without damage): ground
  truth at area 103 `(62,13)` shows a small flame periodically flaring (~0.5 s) on the character
  while walking; HP is measured unchanged before/after 7 s of standing plus walking (party record
  `curHP` at `+0x14`, `maxHP` at `+0x1c`). `0x81` marks a second zone to the west, `(43-54,17-25)`;
  warping directly onto an `0x81` tile or its neighbors gets silently remapped by the area loader
  to a fallback spawn `(65,12)` (normal spawn placement avoids the zone; its behavior can only be
  observed by walking in). The classifier treats the `0x80` class as walkable only when
  `ctx[+5]==0` — NPCs avoid the embers. Implemented as `hotPuffs` in `main.ts` (additive flame
  sprites, 0.38 s, only while moving).
- **`0xd0` steep slope/slide** (907 tiles across 9 areas: Mt. Glaus area 023 slope, 222 tiles;
  McNeil roof area 028, 220 tiles; also 086 Steel Beach, 137 Black Ship, 127, 049, 102, 070). An
  offline check confirms the slope geometry: all 442 sampled `0xd0` tiles (023+028) have a nonzero
  corner-height gradient (2–8), none are flat. The walkability classifier treats `0xd0` as freely
  walkable, so any special sliding behavior must run in move/tick code rather than the walk
  classifier; ground truth for auto-sliding, a special pose, or the possibility of climbing back
  up is still needed.
- **`0x89` conveyor belt** (suspected): appears only in the Factory (area 140, 132 tiles) and Caer
  Xhan (area 148, 12 tiles), forming 1-tile-wide lines with corners and branches (the Factory belt
  puzzle). Whether it forces movement, and in which direction, needs ground truth.
- **`0x30` hop obstacle** (suspected): 609 tiles across 36 areas, often as 2×1 pairs in open ground
  (possibly tree trunks at area 050). The walkability classifier `0x801a2378` counts `0x30` as
  blocked, but the movement code `0x801b4578` has a distinct `0x30` special case that doubles the
  direction delta — consistent with "jump over," but unconfirmed.
- **`0xfd` clear-on-entry** (confirmed mechanism): handler `0x8019f294` checks every tile under the
  character's width for `0xfd` and, on a hit, calls `0x8015562c`, which zeroes that `walk[]` entry
  and forces a screen-window cell refresh. This is a one-time obstacle that dissolves on first
  entry (a door opening, a floor breaking). 167 tiles across 42 areas (e.g. area 076 ×35; area 003
  at `(73,24)`/`(78,49)`). The visual side (what actually plays) has no ground truth yet.
- **`0x11` flag-conditional block** (almost entirely area 052, 128 tiles, plus area 083): the
  classifier special-cases the player context — blocked if byte `0x80146277 & 1` — and other
  entities via `ctx2[0x12c] & 1`. Note this is distinct from entities *dynamically* stamping
  `0x10`/`0x11` into the live walk map at runtime (field tick `0x8019d470` ff.); the static `0x11`
  tiles of area 052 in ROM are unrelated to that runtime stamping.
- **`0x20`–`0x2d` layer parity** (6692 tiles across 121 areas): the move classifier compares the
  `&0xf1` bits of the start and target tile (`0x801bb884` ff.) — this implements bridge-over/
  bridge-under logic, where a tile is walkable on two vertical layers and the low bits select
  which. Full semantics beyond "it's a layer selector" are open.
- **`0xc0` at house doors — door threshold.** In area 000 the byte pattern is a pair of `0xc0`
  tiles immediately before a pair of `0xa1` (warp) tiles — the same role `0xc0` plays as an
  overworld zone-entrance confirmation trigger before a warp. The door-opening animation itself
  (for houses) is presumed to run as a cell/VRAM animation triggered on `0xc0` entry, but this
  needs a GPU-dump series taken at the moment of entry. Myria's sliding doors are a different
  mechanism, driven by object-mesh states (the `0x117000` class); their trigger is likewise
  unconfirmed.
- **`0x84`** (station zones, areas 141/171) and **`0x91`** (isolated patches in 8 overworld areas)
  remain unmapped.
- **`0x40` indoors** marks a lava-flow/decorative surface that is blocked (area 103's glowing lava
  streams use this code); the same numeric code on overworld tiles means ordinary free passage —
  the walkability classifier's interpretation of `0x40` depends on area context.

### Collision

**The original engine has no body radius at all.** The move-check function, `0x801bb53c`, is now
fully disassembled: it works in 16.16 fixed-point, computes the average of the start and target
position (`sra …,1` at `0x801bb5d0`/`0x801bb620`, the choice of which is flag-dependent via context
offsets `+0x34`/`+0x70`), truncates that average to an integer tile coordinate (`sra 16`), and
calls the walk-byte reader `0x80166f64` with it — twice, once for the start tile
(`0x801bb630`) and once for the target tile (`0x801bb648`). Branching is purely on `walk & 0xf0`
(the code-family switch described above). This is a **pure point check** — no radius, no box —
applied to the midpoint of the move, not even to the character's exact position.

Two independent measurements confirm this against the running game, not just the disassembly:
1. **Sprite scale is pixel-exact.** Template matching of an extracted party-member walk frame
   (`Teepo_fidget_long_a_f17`, 37×42) against the real emulator framebuffer at area 007 finds
   0.000 deviation at its matched position `(144,84)`. Field sprites are drawn unscaled.
2. **World scale is correct.** Tile edges measured in a GPU dump come out to 17–23 px; the
   derived constant is 22.86 px per tile.

**Consequence:** because the original has no body collision, a browser implementation that gives
the character *any* body radius is inherently **stricter** than the original — the real character
reaches closer to walls than ours does. A reported bug ("a large character sometimes visibly sinks
into a house wall") is explained entirely by sprite geometry, not by a broken collision box: Garr's
field sprite is 72 px wide against Ryu's 26 px, and at ~22.86 px/tile Garr's frame spans about
three tiles. He is standing at a perfectly correct tile; his drawn body simply overlaps
nearer wall geometry, exactly as it does in the original (where, per the point above, he can
legally stand even closer to the wall). **This is not a collision bug and should not be revisited
as one.** Rejected fixes: rendering the character sprite with priority over nearby wall geometry
(a visible deviation from the original's own painter-order overlap), and a per-character radius
scaled to sprite width (Garr would need r≈1.4; a reachability sweep over all 200 areas shows
1-tile passages start closing once r≥0.4, e.g. −97% reachable tiles at area 016 alone).

**Why the browser still uses a nonzero radius (`COLLIDE_R = 0.3`) despite the original having
none:** without any radius, a character's *center point* can reach exactly to a tile edge, which
visually sinks small sprites into walls by nearly half a sprite width. `bodyFits()` checks the four
corners of a `COLLIDE_R`-sized square against `grid.walkable` before allowing a move. ⚠ Guard: if
the body is already overlapping something at the *current* position (e.g. right after a warp onto
a tight tile, or a section change), only the plain center-point rule is applied for that frame —
otherwise the character could get stuck unable to move at all.

`COLLIDE_R = 0.3` is a measured choice, not a feel-based one:
- **Sub-tile reachability** (all 200 areas, offline): up to `r=0.35` no walkable tile becomes
  unreachable; from `r=0.4` reachability collapses (−14% overall, −97% at area 016 alone, because
  1-tile passages close).
- **Door width** (1205 warp/door tiles, offline): up to `r=0.4`, not one door becomes too narrow
  to pass.
- **Full in-engine reachability regression** (BFS from the spawn point, with the real
  `walkable()` — sections, feature floor, and the `0x40` rule included — run with and without the
  radius, across all 200 areas): 0 tiles lost, 316200 reachable tiles identical either way. ⚠ An
  earlier version of this sweep falsely reported up to 100% loss; the bug was starting the with-
  and without-radius BFS from different tiles and counting isolated islands as losses. Always
  compare both runs from the identical start tile.
- With the radius applied, measured clearance at a wall edge (area 000, 4 sample spots) improved
  from 0.05 tiles (point check) to 0.30–0.36 tiles.

**Left deliberately open:** NPCs are not solid — the player walks through them, while the original
blocks on them; `STEP_MAX = 10` (about 1.25 tiles of climbable height) compares against the
average of a tile's four corner heights, which is soft at mixed-height wall edges; and the NPC-VM
movement system still moves purely point-based, unaffected by the player's body-radius fix.

**A related, narrower placement bug:** gate NPCs (e.g. a McNeil scythe-farmer, spawn `141_0` at
`(14,33)`) stood visibly sunk to the shins. Cause: the spawn tile is a gate-warp tile (`walk=0xc0`,
height 32) directly beside a raised barn door (height 56); sampling terrain height only at the
tile center let the sprite's foot edge intersect the taller neighbor. Fix, scoped narrowly to
`buildNpcSpawns`: **only** for spawns sitting on a warp/gate tile (`0xa_`/`0xc0`), sample the
maximum terrain height over the foot's circumference (±0.45 tiles) instead of the single center
point. Ordinary NPCs (wells, etc.) are untouched. Ground truth cannot confirm original placement
for these specific NPCs — story-phase gating means no captured dump shows them at all — so the fix
is a plausible correction (marked 🔎), not a verified one.

### Billboard foot clamp (sprites sinking into slopes)

Trees and other billboard decoration sometimes appeared to sink partway into the ground on sloped
terrain ("tree through ground"). Cause: on a slope, a sprite's ROM anchor height can sit below the
actual tile-top height at that point, because the PSX original draws painterly, over the ground,
with no Z-buffer to sink into — while the browser's Z-buffered renderer lets the sprite's base
clip through the terrain mesh. A detector (`scratchpad/w10g-treecheck.ts`) compares each anchor's
foot height (`−z/128`) against the maximum of its tile's four corner heights (scaled by
`HEIGHT_SCALE=0.125`); confirmed sinking cases include area 009 `(8,27)` at `Δ=−0.88`, areas
003/008 `(79,49)` at `Δ=−2.0`, area 008 `(45,17)`, and area 024 `(179,22)`, among others. Fix, in
`render/features.ts`: raise a sinking foot up to terrain height, but never lower a foot that sits
above it — ordinary anchors that already match tile height are left exactly untouched. Verified by
image diff at area 009. Positive height differences (`Δ≈+0.75`/`+1.0`) occur where an anchor
legitimately sits on a cliff or wall overhang; these are an artifact of using the tile's maximum
corner height as the reference and are not necessarily errors, so they were left untouched.

### Degenerate skirt triangles on floating tile blocks

At a ground-truth hotspot in the Ice Palace (area 142), the original draws a **degenerate quad —
a triangle**, not a normal wall: vertices `(-16,299)`, `(39,268)`, `(0,344)`, `(0,344)` — the two
lower vertices are identical. The skirt under a floating platform tapers to a single point below
it, rather than staying parallel-sided.

**The existing wall renderer cannot produce this shape.** `terrain.ts`'s `emit()` builds every
wall quad from `[a-top, b-top, a-bottom, b-bottom]` over one fixed `(x,z)` edge, so the bottom edge
is always parallel to the top edge. Where a platform doesn't slope down on that side, this produces
a thin vertical-edged "spike" instead of a fan triangle converging on an apex below the tile
center. Ground truth shows wide pyramid-shaped skirts under the ice platforms; the unfixed browser
shows narrow spikes instead.

**Prevalence** (measured by `scratchpad/degen.ts`, counting degenerate GT quads in the best-matched
frame per dump): present in nearly every area with floating geometry. Worst hotspots: area 142
(42 of 444 quads), 126 (34/871), 097 (27/681), 148 (23/283), 118 (23/350), and two dumps of area
011 (23 each). A stubborn unexplained mismatch remainder at area 148 (an elevator room) is
presumed to belong to this same class.

**Why the automated quad-comparison tool misses this class:** a thin triangle has a bounding box
that is mostly empty, so the tool's fill-ratio guard (`g_fill < 0.55`) treats it as "ground truth
itself is thin, no verdict" and stays silent. Only pixel-level comparison (a hotspot tool) sees it
— which is exactly the signature of "high mis-pixel share at zero quad findings."

**Apex rule, measured directly:** back-projecting the ground-truth apex points (via the same DLT
camera used for verification, see below) lands them on integer tile corners at void height `h=0`
with under 1 px of error — specifically at the **center of the floating tile block**. Area 142's
platforms are 2×2 tile blocks (columns 24–25/27–28/30–31 × rows 30–31/33–34); the measured apexes
— `(25,31)`, `(28,31)`, `(31,31)`, `(25,34)`, `(28,34)` — are exactly those block centers. **Rule:
a free-standing tile block fans from all of its edge corners onto one apex at the block center, at
void height.**

**Implemented** in `terrain.ts`: a 4-way flood fill over renderable tiles finds blocks of at most
12 tiles that are surrounded ring-wise by void tiles with a height drop greater than 16; these get
a computed `blockApex`. `emit()` collapses the bottom edge of their skirt walls onto that apex
(producing the same degenerate quad ground truth shows), and the normal floating-deck skirt rule
is suspended for them. The free camera now shows pointed pyramid skirts under these platforms
instead of spikes. Regression-checked unchanged: areas 000, 022, 034, 043, 049, 056, 077, 097,
112, 118, 126.

**Open:** the area-142 judge score is still stuck at 85%. Without an active tile mask the main
pixel hotspot at `(104,376)` disappears; with the mask active it does not. This means the
verification tool's per-fragment tile mask (see below) is discarding the new skirt fragments even
though their world `(x,z)` position lies inside the correct block and should pass. Next step:
resolve the mask/fragment interaction (fragment world position vs. grid tile assignment); the
remaining mismatch is expected to clear once that is fixed.

### Verification methodology — the prim detector / judge chain

Map geometry is checked automatically by rendering the browser scene from the same camera as a
captured ground-truth GPU dump and comparing quads and pixels (`extract/prim-detect.sh <pair>`;
per-area scores are referred to as `pd<area>`). Several fairness/correctness fixes apply only to
this comparison tool, not to the game itself:

- **Ground-truth tile mask.** `extract/grid-tiles.ts <pair>` exports the savestate's window-grid
  tiles (`0x8012c000`); the judge feeds them to `__bof3.setTileMask` (in `drawwindow.ts`), which
  discards any rendered fragment outside `floor(worldPos.xz)` of a valid tile via a `DataTexture`.
  A boundary tolerance of `fract > 0.06` is required, because wall fragments sit exactly on cell
  boundaries and otherwise tip to the wrong side (an early version cost 2 phantom "DIFFERENT"
  verdicts at area 027). This fix correctly resolves level-ceiling comparisons at area 024 and
  ground-truth window-edge black regions in the 027/043/045 family.
- **Grid-derived section detection.** Composite (multi-level) section detection now also uses the
  grid tiles instead of solver-computed world centers, which had been losing upper levels to
  `row<0` ambiguity. This brings area 024's upper level into the composite comparison (raising
  `pd049` from 90 to 93, since 049 shares the detection code path).
- **Judge FX toggle** (`__bof3.setJudgeFX`, gated by `JUDGE_FX=1`, **off by default**): disables
  drifting/pulsing ambient effects for a comparison run. Measured effect is a wash: campfire embers
  gain accuracy (`pd058`: 93%/15 mismatches → 96%/3) but only when `showAmbiente=true`, because the
  fire animation is driven by the main game loop and does not run from an isolated group toggle
  (several intermediate approaches confirmed this dead end). Brazier/torch areas simultaneously
  *lose* accuracy because their flicker sits at a different phase than ground truth even with glow
  removed (`pd007`: 94%/2→91%/4; `pd026`: 96%/12→96%/17; `pd033`: 81%/0→80%/7). Net effect across
  the two is roughly zero, for one fewer perfectly clean comparison pair — so the standard
  comparison chain keeps ambience off; documented fire/ember animation remains a known,
  intentionally-excluded mismatch family.
- **Skip-fill must be hidden from the judge.** Skip-fill caps (blank filler quads with a
  neighbor's top texture, used to hide "black holes" in the free camera — see "Tile top textures"
  above) are purely free-camera cosmetics and must not appear in a ground-truth comparison. One
  concrete case: area 043 (83%, 3 mismatched quads by the quad detector, but 17% wrong pixels) —
  ground truth actually draws a real wall quad there, `pg(448,256)`/`CLUT(0,485)`, using a
  completely black 16×16 texture (100% black texels — it *is* the void backdrop of the window),
  while the browser instead painted its skip-fill cap. Fix: `setDLTCamera` hides
  `terrain.skipMesh`, and `rebuildTerrain` keeps it hidden while a comparison camera is active
  (composite/section shots rebuild the terrain mid-run, so without this the cap could reappear in
  a later shot and be caught by the pixel-maximum measurement). Effect: `pd043` 83%→99%, `pd027`
  90%→96%, `pd150` 88%→97%. ⚠ A `let dltCam` declared *after* `rebuildTerrain`'s call site threw a
  `ReferenceError` in the module startup path (a temporal-dead-zone bug); `tsc` does not catch
  this class of error, so the declaration was moved to the top of the module. ⚠ If the comparison
  script itself fails to run (e.g. the app is broken), the previous run's log file is left in
  place — always check an "unchanged" score against the screenshot's file modification time before
  trusting it.

**Mesh geometry audit.** Object-mesh placement (see "Object mesh system" above) had only ever been
checked as an inventory (does the record exist), never as rendered geometry. Running the existing
dump/savestate pairs through the quad comparison (78 of them fall in mesh-bearing areas) for the
nine mesh-richest areas gives:

| Area | Mesh objects | Match | Mismatched quads |
|---|---|---|---|
| 052 | 12 | 100% | 0 |
| 135 | 10 | 100% | 0 |
| 167 | 9 | 96% | 1 |
| 170 | 7 | 100% | 0 |
| 121 | 6 | 99% | 4 |
| 040 | 5 | 100% | 0 |
| 083 | 5 | 99% | 4 |
| 111 | 5 | 100% | 0 |
| 148 | 4 | 93% | 23 (12 clusters) |

Object meshes are placed correctly: five of the nine areas are pixel-perfect, three others are off
by only 1–4 quads. Area 148 is the exception, and its cause is not a mesh placement error but a
color bug in a different system:

**Bug: runtime-geometry (`rgeo`) atlas keys omit the CLUT when a quad inherits it from its seed.**
`build-runtime-geo.ts` builds each atlas key as
`` rg${u0}_${v0}_${w}x${h}${q.pg ? `_p…_c${c.cl[0]}_${c.cl[1]}` : ''}${…} `` — the CLUT suffix
(`_c…`) is appended only when the quad explicitly carries its own page (`q.pg`). A quad that
instead uses the seed's page/CLUT gets no CLUT suffix at all, so every quad sharing the same UV
rectangle collapses onto one atlas entry **including its color**, regardless of which palette it
actually used. Confirmed at area 148 (Caer Xhan): a ground-truth dump shows three different CLUTs
on the same texture page — `(0,490)` (215 quads), `(0,491)` (42 quads), `(0,492)` (24 quads) —
several sharing identical UV rectangles. The baked atlas keys correspond to the UVs of `(0,492)`
but are colored using whichever CLUT the seed happened to record, producing 17 "DIFFERENT" and 6
"MISSING" verdicts. **Fix:** always include the CLUT in the atlas key. This changes every `rgeo`
key string, so it must be rolled out deliberately: extend the key schema, rebake every area with
`rgeo` data (086, 049, 198, 067, 148, and others) together with `feature-semi.json` (which stores
keys of the same form and would otherwise desync from the new keys), then re-check against the
judge baseline (a full 194-pair sweep — with the tile mask and grid-section fixes, ambience off —
had established an average score of 99.37% before this bug was found). ⚠ A sweep must not overlap
with source changes: an earlier full-sweep attempt was discarded because hot-module-reload picked
up interim versions of `setJudgeFX` mid-run, making the results internally inconsistent.

Representative scores from this round: area 112 at 98% (the remaining 8 mismatches are a radar-
tube animation, a documented exclusion), area 049 at 92–93%, area 024 at 95%.

### Fire particle sprites (area 011)

Separately from the map-side CLUT flicker described above, area 011's hearth fires are drawn as
**16×16 particle sprites**: page `(320,256)`, 8bpp, CLUTs `(0,484)` and `(0,486)`, with several
cell variants (`u0/v160`, `u0/v176`, `u240/v16`, and others). Per hearth, several fire columns are
stacked with additive blending, each instance running at a different phase. An earlier "pink jags"
rendering bug in the browser was the same ember texels drawn with the static wall CLUT instead of
the fire's own `484`/`486` palettes. Implementation: real cell sprites exported to
`public/entities/blaze0.png`…`blaze2.png` (green-masked), placed via a `BLAZE_PLACES['011']` table
in `entities.ts` (5 hearths, 3 stacked billboards per hearth, roughly 6 fps, phase-offset per
instance). ⚠ Hearth positions are a ground-truth-photo-calibrated approximation, not an extracted
spawn table — the original spawn locations live in a per-area init overlay class (`ENT_REQUEST`)
that is not otherwise covered by this chapter. The original's red sky backdrop for this area is
likewise not reproduced (backdrop color appears to be set per area; compare the SKY_AREAS gradient
system above, which only covers areas 082/127). A frame-0 gallery scan of area 011's descriptor
table also turned up several unresolved cutscene-only sprite keys (238/239, 67/68, 2, 512, 434),
cataloged but not otherwise identified or used.

### Ground-truth capture — tooling notes

Practical lessons for capturing and trusting emulator ground truth, gathered while building the
systems above:

- **DuckStation pauses whenever it loses focus.** A capture series must be taken with the emulator
  in the foreground the whole time; a photo series taken with the window unfocused shows 0 diff
  pixels after about 2 s because the game simply isn't running. Standard practice: one warp,
  followed by roughly 12 GPU dumps at uneven intervals (0.4–1.3 s), each triggered with its own
  window-activate step. Every dump begins with a full VRAM snapshot, so diffing the relevant VRAM
  region (e.g. CLUT rows `y≥480`) across a whole series recovers every animation phase at once.
  `gpudump.ts` can read GPU command color words, Gouraud vertex RGB, and VRAM-to-VRAM copy events
  (`copies[]`, traced within VRAM).
- **Savestate VRAM is not reliable for reading texels in general** — the hardware renderer and any
  upscaling factor mean what a savestate reports may not match the true low-level VRAM content;
  always prefer a GPU dump's VRAM for texel extraction. This was later refined: under one specific
  configuration (Metal renderer backend, resolution scale 1×, i.e. native `320×240`), savestate
  VRAM was cross-checked and found to be byte-usable (a full area tileset seed was pulled this way
  and matched correctly). `extract-runtime-water --state <name>` is the standard tool for this
  path. Above 1× upscale the original caution still applies. Rule of thumb: before trusting
  savestate texels, check the rendered framebuffer PNG that `parse-savestate` writes alongside its
  output (`<prefix>.vram.png`) — if the visible game image looks correct, the texels are usable
  too.
- **A live savestate's quad/entity scan can read stale data from a previous area.** Both
  `scanSavestateQuads` (graphics) and the `0x602` entity-record buffer (spawns) can carry residue
  from whatever area was loaded immediately before the current one, especially in a save taken
  right after a warp completes. For "does the game draw this here," only trust a fresh GPU dump or
  a photo, never a live quad scan. For spawn/entity verification, a save taken immediately after a
  warp is unreliable; let the game run for several seconds first (so stale records get overwritten
  or the relevant table's real at-rest flag stabilizes) before saving, or read the live entity
  table directly with a debugger instead of relying on one savestate.
- **Hole-area probe regions can overlap spatially** — one area's detected "section 0" bounding box
  can fully enclose its "section 1," so picking the bounding-box center as a warp target can land
  in the wrong section. Always cross-check a chosen target tile against the section index and
  `walk` data before capturing.
- Emulator key input was switched from `osascript`-driven keystrokes to `hidkey.py`, because
  `osascript` produced sporadic macOS input errors. ⚠ `hidkey` posts input system-wide rather than
  to a specific window, so DuckStation must actually be the frontmost application whenever it
  runs, or keystrokes go to whatever else has focus — always confirm the frontmost application
  first when driving the emulator this way.
- Browser-side debug helpers used throughout this work: `__bof3.tileInfo(r,c)` reports a tile's
  walk code, section and active state; `__bof3.teleport` switches the active section as well as
  position, not position alone.

### Reference material

`public/primamaps/` holds scanned Prima Guide pages — in-game 3D renders of every area — kept as
comparison reference. `index.json` lists pages 141–179 with content names; `area-map.json` maps
area number to page number. Only pages 141/142/143 plus a McNeil excerpt exist as PNG so far,
shown in a reference panel inside `/texture-lab.html`; source scans live in
`references/extmaps/`. This material was used for a time to improve top-down maps by hand; that
role has been superseded by the object-mesh and runtime-geometry reverse engineering described
above.

`references/` layout: `community/` holds copied community references (recap and index text, guide
map scans, independent ROM/texture copies); `screenshots/` holds verification screenshots from RE
sessions; `gpudump/` holds GPU/RAM dumps and their sidecar files; `extmaps/` holds the Prima Guide
source scans; `rom/` holds the disc image itself.

A legacy manual paint editor, `edit/painter.ts` (operating on baked atlases under
`public/textures/AREA###/…`, composited via `edit/atlas.ts`), predates the real ROM-based
texturing pipeline and is largely obsolete, but remains present and working: textured overlay
quads, autosave to `localStorage`, and per-area JSON export (`bof3paint_<tag>`). Toggled with the
**B** key.

### Field findings outside map geometry (recorded in the same investigation rounds)

A few findings from these rounds concern characters and UI, not map geometry, but are recorded
here because they were established alongside it:

- **Adult Teepo has no field walk cycles on the disc.** Proof by combinatorics: the 19
  `PL###.EMI` party-combination files encode character-ID combinations from the set
  `{0,1,2,3,4,5,6,7,8,9,A}`, and no combination includes an adult-Teepo slot (ID `A`/`10` is
  instead a "Goblin" dummy placeholder). His field appearance is therefore a plain standing
  figure. An NPC sprite that had been suspected to be him (key 775 at area 174) is not Teepo on
  visual inspection (a hatted merchant); the real adult-Teepo art is area 172, descriptor id 625
  (a 35×52 battle-stand image, previously cataloged but unlabeled); the adjacent descriptors id
  727 (a lying dragon head) and id 707 ("D>Lord") are his dragon form. Implemented as a "Teepo
  (Adult)" character selector, rendered down-facing only (the same pattern used for the Weretiger),
  at native battle scale (52 px, comparable to adult Ryu's 45 px).
- **Field-menu pixel metrics**, measured against a ground-truth screenshot: a zenny amount ending
  in "118" has its number ending at frame-x 274, with the currency icon crop starting at x 275; the
  browser had been drawing the number up to about x 290, overlapping the icon, with the icon itself
  sitting 9 px too far right. Fixed layout: the number is right-aligned at window-local x 67, with
  the icon fixed at the same x 67. A selected inventory icon had been given a solid white
  background, turning its transparent regions into a white block; ground truth only shows a white
  double-line outline with the background staying transparent. Icon row sits at y 42 in ground
  truth (the selection frame at y 41). The quantity badge on a stacked item is 12×12 px in ground
  truth; the browser's digit rendering for it is a font approximation only — the original's italic
  badge digit set was not extracted.
- **Weretiger transformation timing was re-measured against raw ground-truth video** (a Y-brightness
  curve at 10 fps plus 20 ms RMS audio onsets): the burst loop actually lasts about 6.9 s, not the
  5.6 s an earlier reference had used (that number came from a differently-lengthed reference clip)
  — constants updated: `BURST_MS` 5600→6900, `ENDGROWL_MS` 450→270. Re-measured browser output then
  matches: brightening at 7.33 s, sprite swap at 5.03 s, call onset at 1.55 s vs. 1.58 s in ground
  truth. Ground truth also shows a permanently saturated white core around the transforming
  character during the burst (additive saturation at the effect's origin), missing from the
  rasterized effect frames (whose center averaged only about brightness 22); added as a dedicated
  `TransformFX.burstCore` sprite (roughly 0.24× the effect size, a slight 17 Hz flicker). Dim depth
  during the transformation is confirmed close to ground truth (browser 65→28, a factor of 0.43,
  vs. ground truth 74→34, a factor of 0.46).
- **Accession (dragon transformation) timing was confirmed 1:1 against ground-truth video, no fix
  needed:** measured ground-truth beats — aura screams at +0.2 s/+1.2 s, charge at 15.9 s, pillar
  flash at 16.85 s (peak brightness 84), a dark dome phase from 17.4–20.9 s (3.5 s, brightness
  79→50), brightening over 0.2 s, cry at +0.55 s — all matched by the browser (aura ≈4.3 s, dark
  dome phase 6.4–9.9 s = 3.5 s, total ≈10.4 s vs. 10.2 s in ground truth, within a 0.25 s raster
  tolerance). Measurement method for both effects: `ffmpeg`-derived brightness curves plus 20 ms
  RMS audio-onset detection, compared against a matching series of browser frame captures.

### Refuted approaches

- **"Divider ≥232 = never draw."** Wrong. The game only skips drawing at `tileTexIdx==0`; tiles
  with `h≥232` (negative signed heights) are real, textured depressions. The rule appeared to work
  only because most deep tiles happened to also have `tileTexIdx==0`.
  See "Corner heights."
- **Reading tile-top entry byte 0 as a plain cell coordinate.** Wrong for any entry whose byte-1
  nibble is non-zero (a rect/pair top); caused wrong mountain textures, false "blank" tiles, and a
  phantom "gate texture tiled across the map." The reference `bof3-3d-maps` renderer shares this
  bug and is now the less accurate of the two. See "Tile top textures."
- **"Pages 4–7 render correctly only for AREA000/003/007."** Wrong; one universal formula
  (`page&3`) covers all pages in `decodeTile()`. The old per-area lookup tables were dead code.
- **The shared-base / "grass textures live outside the EMI" theory**, and **"walls are only
  height-based."** Both wrong — every texel lives inside the area's own EMI, and walls have been
  correctly textured since 2026-06-10.
- **Grid substitution for hole tiles** ("the renderer fills `tileTexIdx==0` tiles by substituting
  or duplicating a neighboring record"). Disproved across 11 swept areas: the window grid simply
  leaves hole tiles empty; anything visible there comes from a different rendering layer (deco/
  object sprites, a sky backdrop, or nothing). See "Hole tiles."
- **A dedicated "big quad" system for large drawn structures** (the AREA112 satellite dish, the
  Dauna rubble/dragon, the 086 ship, the 170 containers). Disproved: these are the ordinary
  40-byte object-mesh format, reached through the init script's `0xfN` story-branch streams rather
  than the normal static parser. The specific 112 measurement that started this theory was a stale
  RAM artifact, not something the game actually draws. See "Object mesh system."
- **AREA112's "missing edge plates."** Not a bug — the original also leaves those edge voids
  undrawn; a brightened Prima Guide scan had given a false impression of missing texture.
- **"The 067 bridge is drawn by the TYPE-0 feature system."** Wrong — a texture-match filter had
  actually matched the area's animated water quads, not the bridge. The real bridge is built at
  runtime by per-area overlay code and has no static feature record at all.
- **The AREA007 "second door."** Not a real second door — a complete phantom façade produced when
  an overlay-registration step matched area 007's tiles against texture data captured in a
  *different* area's ground-truth dump (a mountain region of area 000). Fixed by restricting each
  area's overlay rebuild to its own dumps.
- **The "slot UV" theory of water/river animation** (per-frame UV offsets in the GPU command slot
  pool). Wrong — ground truth shows UVs, vertex positions, vertex colors, and VRAM texels all stay
  byte-identical across animation phases; only the CLUT palette changes. See "Map header table
  [17]."
- **Feature-type `0x23` records as a hidden graphics source** (a candidate explanation for the
  area-086 crane). Disproved — a resting `0x23` record's handler is a no-op; it renders nothing
  until a proximity trigger advances it through the patch cycle.
- **Feature types `0x28`–`0x2b` as the area-086 crane's drawing mechanism.** These handler slots
  exist in the dispatch table but are used by zero areas in practice (confirmed reserve slots); the
  crane is drawn by unrelated per-area overlay code instead.
- **A dedicated object-slot "train format" at AREA045.** Refuted — the slots that seemed to define
  it were stale RAM residue from the *previous* area (014's windmill spawns), not train data.
  Movers use the same ordinary 40-byte object-mesh record format as static objects; no separate
  mover display format exists.
- **D3 "blank cluster" tiles as missing runtime texture uploads**, for all 11 swept areas, and
  **D2 "deck gap" tiles as missing deck geometry**, for the areas checked. Both negative: ground
  truth confirms these tiles are legitimately empty (D3) or sit at a real map edge/genuinely empty
  interior (D2) in the original.
- **The browser's own "hole-fill lid"** (opaque black planes bridging enclosed hole-tile
  components). Not a original-accuracy feature — it caused its own visible bugs (a "chunky block"
  under the area-023 bridge, black patches under the area-060/065 bridges) and was removed once
  holes were confirmed to render as empty in the original.
- **A standalone "procedural backdrop packet class."** Does not exist — every case resolved into
  one of: ordinary star objects (camp trees), the multi-quad TYPE-0 chain sub-format (the area-049
  crystal), or ordinary individual roof quads (area-024 dome walls).
- **Savestate VRAM as universally unusable for texel extraction.** Overstated — under a specific
  renderer configuration (native resolution, Metal backend) it is byte-usable; the caution applies
  mainly above 1× internal resolution scaling.
- **A live savestate quad/entity scan as proof of "what the game draws/spawns here."** Repeatedly
  shown unreliable — both graphics quad scans and `0x602` entity records can carry stale data left
  over from a previously loaded area.
- **Two rejected fixes for the "character overlaps wall" collision report:** rendering the
  character sprite with draw priority over nearby wall geometry (would deviate from the original's
  own overlap behavior), and a per-character radius scaled to sprite width (large characters would
  need r≈1.4, and any radius ≥0.4 starts closing 1-tile passages game-wide). The report itself was
  not a bug — see "Collision."

### Open

- **Table `[17]`:** full byte-level semantics of the general animation player (the `seg` byte, the
  `0x81` group format) are still not disassembled; needed to cover any future area without
  per-area reverse engineering. The VRAM staging copy's ultimate consumer is unidentified — no
  primitive references it directly.
- **A third map animation mechanism** ("entry rotation," distinct from VRAM cell upload and CLUT
  cycling) is the real animation behind areas 016/033's rivers after their CLUT-class assets were
  removed as a misapplied regression; not yet reverse engineered.
- **Feature type `0x0f`** (animated waterfall/flow effect, e.g. the McNeil millstream): the
  geometry-emission loop is unread; no browser reproduction exists yet.
- **Feature types `0x11`–`0x1f`** (per-tile vertex wobble animation): mechanism is understood but
  not implemented in the browser (a subtle effect, low priority).
- **Feature type `0x22`** (cond-checked geometry variant): vertex construction from its record
  bytes is unread.
- **Roof condition codes `0xfe`/`0xff`**: present in the data (168 and 16 occurrences respectively)
  but their visibility rule is not implemented; both stay latent.
- **The story-chapter advance-request bit's exact write instruction** is still not located at the
  instruction level (only the higher-level fact that per-chapter SCENA scripts request it is
  known). Next concrete step: compare two playthrough saves taken immediately before/after a
  chapter change to confirm the moment, then set a breakpoint on the increment routine
  (`0x8019ff2c`) and trace backward through a reload cycle — requires the emulator.
- **INIT-stream `f0` event sections** (variable-length condition expressions) are parsed only
  conservatively as "gated" rather than evaluated; flag-conditional spawns are therefore uniformly
  treated as latent. `fn12`/`fn13` condition-function semantics (suspected flag-test helpers) are
  unclarified.
- **NPC facing direction:** the concrete field is narrowed to spawn byte `+0xa` (written to entity
  `+1`) but not confirmed; the direction-dispatch animation-set table at `0x80182148` and matching
  browser facing/walk animations are the next expansion. Non-child villager subtype semantics (5,
  7 falling back to a walk pose) are unconfirmed against ground truth.
- **Walk codes still open:** `0xd0` slide (does the character auto-slide, with what pose, and can
  it climb back up?), `0x89` conveyor belt (forced movement and direction?), `0x30` hop obstacle
  (confirm the "jump over" reading of the doubled direction delta), `0xfd` clear-on-entry (what
  actually plays when it fires), the `0xc0` door-open animation and its trigger timing, and the
  fully unmapped `0x84`/`0x91` codes. The `0x20`–`0x2d` layer-parity family's exact semantics beyond
  "a two-layer walkability selector" are also open.
- **Degenerate skirt triangles:** the apex rule is implemented, but area 142's judge score is
  stuck at 85% because the ground-truth tile mask discards the new skirt fragments even though
  their world position is inside the correct tile block. Next anchor: resolve the mask/fragment
  world-position-vs-grid-tile interaction in the comparison tool.
- **The `rgeo` atlas-key CLUT bug fix is designed but not yet rolled out**: extend the key schema,
  rebake every `rgeo`-bearing area (086, 049, 198, 067, 148, and others), regenerate
  `feature-semi.json` to match, then re-verify against the pre-fix judge baseline (194-pair average
  99.37%).
- **168 glass panels** (`pg576`, no page-3 texture route) and a light-Gouraud remnant at area 058:
  currently covered only by the stereo/vertical bake; their true record source is still unknown.
  Area 049's judge comparison also stays at 8 mismatched quads (facet fine offset/blend, treated as
  polish rather than a correctness bug).
- **Runtime-drawn per-area objects still needing a ground-truth bake**, using the same pipeline
  built for the area-067 bridge: the area-086 crane/ship body (renderer disassembly or a
  planar-projection fit of its flat deck are the two candidate routes), a checkpoint substructure
  at area 060, a conveyor belt at area 049, and a crystal skirt at area 198.
- **Area 055's Yggdrasil** cannot currently be reached by warp (it fails silently as a story area),
  so its lying-down pose is unverified against ground truth.
- **Area 026's lanterns**: negatively narrowed (no sprite descriptor, no object mesh found) to the
  same per-area-code class as the crane/bridge, but not solved — the light cones are already baked
  into the map texture; the lantern objects themselves are still missing.
- **Area 050's flags** and **area 025's abyss** both still need a ground-truth capture at the
  specific in-map location where the reported issue occurs (the general area has been checked, but
  not the exact spot).
- **Chest open-state persistence:** each chest's `flagIdx` is extracted but not yet wired to the
  story-flag bank, so open/closed state does not yet follow story progress; item byte fields
  (`itemId`, `itemType`) are not yet cross-checked against community item lists.
- **Ambient life follow-ups:** bird spawn types `0x18`/`0x17` are not yet coupled to their real
  `0xe0` spawn positions (the browser's bird effect still uses a placement heuristic); a moving
  train visible in one fishing-area capture is a separate rail-ambient object, not yet implemented.

