> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 6. Animated surfaces: water, sky, fire and weather

Map-level animation splits into four independent systems: tile water animation (three
distinct encodings), a separate ocean wave-layer overlay, sky (clouds plus their ground
shadows), and localized fire/smoke billboard emitters. There is no weather system and no
day/night cycle on the disc — "night" is a story-state area-variant switch, covered at the
end of this chapter.

### Water animation: three classes

Map water animation reduces to three classes. An early candidate for a fourth class was
proposed and later refuted (see Refuted approaches).

| Class | Entry tag | Mechanism | Areas |
|---|---|---|---|
| VRAM cell upload | `series`/`0x80` | Pre-baked frame banks, stored in the AREA-EMI data itself, are copied into fixed VRAM cells at runtime | AREA121 (Middle Sea); deep-water cells in AREA087/088/121/151/115, remnants in AREA033 |
| CLUT cycling | `[17]` | Palette-row swap on the tile CLUT instead of new texel data | Confirmed to exist; AREA121 explicitly does NOT use it. This source gives no area list or addresses for it beyond that |
| 0x82 rect cycle | `[17]` type `0x82`, multi-pair groups | Each entry pair copies a different source bank into the same destination window, keyed to its own tick byte | AREA104 (sailing lake) |

### VRAM cell upload

AREA121 ("The Middle Sea", boat overworld) cycles page-2 cells 1-15 and 0-1 by VRAM upload.
Each cell has 2-3 distinct phases, played ping-pong. The area uses no CLUT cycling, no UV
change, and no `textureData` rewrite.

Proof method: five time-staggered savestates (`warp --wait 13..17`). ⚠ The same wait value
always reproduces the same animation phase deterministically — this is why two
independently-taken standard saves once looked identical, a methodology trap worth
remembering for any phase-locked capture.

Extracted phase data: `references/re/water-phases.json`. `build-water-anim.ts` renders, per
overworld area, the affected map tiles per phase into `public/water/areaNNN.{png,json}` — a
patch sheet per texture entry (Middle Sea: 78 entries / 5022 tiles). The browser's
`loadTexMask` stamps the patches through a `CanvasTexture` interval, 350 ms, ping-pong;
verified as three distinct browser frames with visible wave motion.

**Overworld deep-water base layer.** AREA087/088/121/151/115 (plus remnants in AREA033)
rendered black at exactly four page-0 cells (8-9, 4-5) — the "runtime deep-water cells",
absent from every static asset carrier. Two ground-truth sources, the `ow016hut` VRAM dump
and an `a033` savestate, show these four cells filled at runtime.

The source frame bank sits inside the AREA-EMI bands themselves. A byte search found frame
data in 30+ areas; AREA016 sub[7] carries all four cells. The engine copies them into the
target cells at runtime, animated.

Fix: frame 0 of the four cells, taken bit-exact from the dump
(`references/re/ow-water-cells.json`), was added as the last base layer in
`build-maptex.overworldBaseVram`, filling only empty spots. All 9 overworld maptex and area
builds were regenerated. AREA087 went from 229 to 86 dark cells; the remainder is
building-roof filler (a separate system) plus individual wave-edge slots that are also empty
in the ground-truth snapshot. Browser-verified: Rhapala bay renders fully textured.

### CLUT cycling

Confirmed as one of the three water-animation classes, entry tag `[17]`, by exclusion —
AREA121 explicitly rules it out for its own cells ("NO CLUT cycling"). This source carries no
area list, addresses, or timing for this class beyond confirming it exists.

### 0x82 rect cycle (frame banks)

`[17]`-type `0x82` groups that carry more than one entry pair are frame cycles, not mere
fill-once seed sources (a fill-once group is used elsewhere purely for static baking). Each
pair copies, at its own tick byte, a different source bank into the same destination window.

AREA104 (the sailing lake) uses two groups of three banks each, period 23 ticks, tick marks
at 7/15/23 — an 8-tick cadence, 640 ms per phase. A savestate (`pd104`) proves the cycle
directly, with no emulator run needed: all six source banks are filled, the destination
equals source-bank 1 at 100.0% match (both groups phase-synchronous), and source-banks 0 and
2 each match 68-72%.

Tool: `extract/extract-anim82-phases.ts <area> --state <name>` lifts every bank of a
multi-phase `0x82` group from a settled savestate's VRAM into `anim-phases-<NNN>.json` (with
`seq:'cycle'` and the original `intervalMs`). `build-water-anim` merges this through a new
`perAreaSeq` path into the water JSONs. For AREA104: 62 destination cells (page 0, tile row
0/1 and 8/9 — exactly the `water-seed-104` fill cells) expand to 82 entries / 6964 tiles / 3
phases. The browser sea animates accordingly (Playwright pixel diff ~37-41k px per 640 ms
step, matching the savestate framebuffer).

Verification ceiling: the `pd104` judge score plateaus at 90%. The remainder is
ground-truth-side, not a rendering bug — the warp-dump recon carries stale font glyphs in the
middle of the lake (stale dump VRAM; the savestate framebuffer itself shows smooth water),
plus cloud-shadow drift and edge silhouettes. Rule of thumb for water-cell judging:
framebuffer/savestate VRAM outranks dump recon.

### Water surface geometry

Rivers and the sea render FLAT: ground truth (the `ow121obj2` dump) shows constant
ground-quad vectors across the entire water surface. The navigation-map "heights" recorded
for water tiles (0-20 in AREA121) are NOT render geometry.

The terrain builder originally used those nav heights directly, producing bumpy, "sinking"
water. Fix: `buildTerrain` flattens the animated water tiles (from the `public/water` lists)
to the surface minimum (`flatWater` parameter; the tile lists are loaded in `main`).
Browser-verified on the Middle Sea: a continuous flat water surface.

Slope wedges needed a second pass: `corner()` itself is now `flatWater`-aware, so both tile
tops and walls see a consistent water level. Previously a top-only patch left black wedges at
mountain bases.

### Ocean wave layer

A second, independent animation layer rides on top of the tile water: large traveling wave
quads, found with a new time-series capture tool.

`extract/dump-series.ts <state> <tag> [--n 8] [--gap 1.2]` loads a named savestate in
DuckStation (must run in the foreground), pulls N `F8` GPU dumps on a fixed time grid plus
one `F2` savestate anchor, then exits cleanly. ⚠ It writes the cheat config empty — a
leftover `[AreaSwitch]` entry from a prior `warp.ts` run must not still be armed, or it will
fire. Series `w088ser` sampled 8 frames at 1.2 s spacing from the `meer088` save.

Finding at AREA088 (coast): two separate animation systems overlap.
1. Coastal cells cycle with a ~3.6 s period across 3 phases (36 cells — already matched by
   `water/area088.json`'s `frames:3`).
2. A wave layer: giant additive S1 quads (page 704, CLUT (176,483), 4bpp), two band entries
   at UV (0,160) and (80,192), 80×32 texels each, stretched on screen to ~633 px ≈ 28×11
   tiles per band. These quads travel across the sea toward the coast, swelling and receding
   on a ~10 s cycle (measured screen width: 633 → 548 → … → 0).

Browser: `build-oceanwaves.ts` extracts the two band textures into
`public/entities/oceanwave0/1.png`. `main.ts`'s `buildOceanWaves` lays one world UV mesh per
layer over all current sea tiles (`waterTilesCur`), band rapport 28×11 tiles, additive blend
at ×0.5. Texture drift runs ~0.4 tiles/s southward with a sin² recede pulse over 9.6 s; the
two layers are phase-shifted against each other. `WAVE_AREAS = 87/88/115/121/151`, gated to
the ambience toggle.

⚠ Judge ceiling: `pd088`/`pd121` plateau near 70% match. An animated layer never lines up
pixel-perfect with one static GT snapshot, since quad position, coastal-cell phase, and the
GT wave's land runout onto the beach all vary independently — the same non-bug category as
the cloud-drift mismatch below.

### Sky: clouds and shadows

Ground truth (the `ow121obj2` dump, Middle Sea) shows the game drawing 2 cloud textures
(80×80 and 48×48, map-page 3, block-A palette 11) plus wisps (16×16, palette 13) as giant
quads: additive in the sky (SEMI1 blend) and subtractive on the ground as shadow (SEMI2
blend).

Initial extraction: `public/entities/system/clouds/` (alpha derived from luminance). Browser:
`src/render/clouds.ts` — additive sky sprites at height ~9, dark shadow sprites, drift ~0.35
tiles/s, wrapping, gated to overworld only.

A later re-measurement (AREA045, `fish45_1` capture) mapped the runtime system exactly as
class typ-11 (CLUT (176,483), page 704/256, 4bpp):
- 72 SHADOW tiles: 10×10 UV, `semi=2` (B−F blend), color `0x282828`, terrain-projected and
  tiled on the ground (GT-measured ceiling 290×154 px ≈ 8.6×9.7 tiles).
- 6 CLOUD quads: `semi=1` (B+F blend), color `0x505050`, source shapes 80×80/48×48
  compressed on screen to 282×93/151×52 px, positioned vertically above their own shadow
  (x-offset ≈ 0, screen offset ~101 px ≈ height 3.65 world units). Quads pool around the
  camera and park up to ±1024 units offscreen; the visible frame mixes 2 large + 4 small
  quads.

Re-extraction: `extract/build-cloud-tex.ts` → `public/clouds/cloudA.png` (80×90, RGB-0 edges
for One/One blending), `cloudB.png`, `clouds.json`. Rewritten `clouds.ts`: ground shadow uses
a ReverseSubtract blend, cloud uses an additive billboard. The PSX modulation formula
`texel·color/128` maps to a THREE.js color of `×255/128` (giving `0x505050`/`0xa0a0a0`).
Clouds render from a camera-centered pool of 6 (range ±26), gated by the ambience toggle.

Drift, GT-measured (`drift045c/d` series, Δt = 6.0 s exact, field offset −240 accounted for):
direction ≈ (−0.32, +0.95), i.e. SSW. Speed is individual per cloud, 0.28-1.2 tiles/s (four
clouds measured: 0.28 / 0.49 / 0.73 / 1.2) — adopted directly into `clouds.ts`.

Method notes: 30 s sampling intervals are too long for frame matching, since wraps break
correspondence — use ~6 s instead. `cp` resets file mtimes, so dump-interval timing must
always be read from the original files in the screenshots folder, never from copies.
Semi-transparent prims must be judged with `render-gpudump --blend` (extended to render
modulation and all four semi-transparency modes) — the plain painter recon renders semi
prims fully opaque and will misread them. `gpudump.ts` was extended with a general
`uploads[]` array, one `VramUpload` record per frame, usable for any VRAM-animation RE, not
only clouds.

Evidence: `references/screenshots/wolkenschatten-2026-07-24/`.

### Fire and smoke plumes

Two emitter sites, AREA002 and AREA102, were solved with a player-anchor calibration method.

`warp.ts <area> <x> <y> --dump --save` places the party lead at an exact position; the
slot-1 save stores that position at `0x149308`/`0x14930c` (Q16 fixed-point X/Y; the warp
target itself is given as an integer). The paired GPU dump captures the player sprite stack
(CLUT 495-500). The sprite's on-screen foot position anchors to its known world position,
which converts GT screen coordinates of the target prims into browser pixels — scale factor
GT × 1.125, the browser's standard zoom, via `project`/`pickObj` — and resolves to world
tiles. ⚠ Convention pitfall: the browser's `warpTo` places the player on the tile CENTER
(x+0.5), the disc engine at exactly x,0 — a half-tile remainder resolved using a sight anchor
(the charcoal-kiln chimney).

AREA102 (Mt. Zublo) has 3 steam sources:
- A lava-field pillar at tile (20,32), direction vector (−0.1, +0.54, 0).
- Two slope jets: (24,24, y5,13) with vector (0,0,+0.43), and (28,20, y4,24) with vector
  (+0.4,0,0).

Both jets are small 8-puff chains (GT-measured width grows 8→36 px), color fading from
`0x78` to `0x00`. `buildEntities` gained a JET MODE for this: `Chimney.jet = {vec, w0, w1}`
plus an absolute `Chimney.y`. Opacity follows `0.9·(1−f)` without the usual plume damping — a
tried `0.5·opacity` value was invisible.

AREA002 (excavation) has 2 charcoal-kiln smoke columns at tiles (22,41) and (28,41), width
ladder 33→223 px — close enough to the McNeil chimney stack profile to reuse the normal
chimney entry format (`public/chimneys/area002/102.json`).

Open remainder: the 4 haze bands of the `clut102` series (2040-px stripes) are unsolved. The
next anchor point is a separate dump further south; "band mode" is unexplored.

### Weather and day/night

There is no weather system and no day/night cycle on the disc. "Night" exists only as a
story-state switch to an alternate area variant.

The state byte lives at `0x8014933c` (bit 0 of `0x80146871`, written only by `0x80158244`:
`lbu 0x80146871; andi 1; sb 0x8014933c`). The overworld switch for AREA189 ("Desert of
Death, walking") reads this byte at `0x801b67b4`-`0x801b6824`, looks up table `@0x801cd520`,
and writes the result to `0x80143f10/14/18/1c` (area/X/Y/dir):

```
0x8014933c == 0 → table[3..5] = bf 1e 16 → AREA191 (30,22) dir 4
0x8014933c != 0 → table[0..2] = c0 13 16 → AREA192 (19,22) dir 4
```

AREA191 and AREA192 are both named "Desert of Death" in the area-names guide — two variants
of the same desert, chosen by the story flag. Setting `STATE_FF = 1` in `features.ts` shows
the opposite variant of every `0xff`-conditioned feature; that is the browser's day/night
toggle.

The same state byte also gates general feature visibility, through the condition interpreter
at `0x801560f0`:

```
$a2 = cond & 0xff00                                          ; class byte
b1==0x40 → return cond & 1                                   ; 0x80156240
b1==0xff → return [0x8014933c] ^ (cond&1)                    ; 0x8015611c  (bit 0 only)
b1==0xfe → return (cond&0xff) ^ [0x8014933b]                 ; 0x80156130  (full byte)
b1==0xfd → return (cond&0xff) ^ [0x80143f03]                 ; 0x8015614c  (input byte)
```

`condVisible` (in `features.ts`) originally read these state bytes from the wrong addresses,
`0x8015933b`/`0x8015933c`; the previously noted "live values" (0xfe→0xa0, 0xff→21) came from
that wrong, unrelated address. The correct addresses, confirmed by disassembly of the
interpreter above, are `0x8014933b`/`0x8014933c`. Both state bytes read 0 throughout normal
play, checked in RAM dump `ram1.ram.bin` and across 120 savestates spanning 30 areas. Fixing
the addresses changed feature visibility as follows:

| cond | count | before | now |
|---|---|---|---|
| 0xff01 | 359 | visible | visible |
| 0xff00 | 16 | visible (wrong) | invisible |
| 0xfe01/02/03 | 182 | invisible (wrong) | visible |
| 0xfe00 | 71 | invisible | invisible |

Net effect: +166 features become visible, across 10 areas (015, 077, 106, 108, 109, 128,
135, 148, 173, 197). Browser-verified (015/108/148 load with no errors), `tsc` clean.

No global day/night tint exists. The multiplicative RGB modulation triples at
`0x80143d75-77` and `0x80143e05-07` have exactly five writer sites — `0x8014ed94`,
`0x8014f734`, `0x8014f808`, `0x8014f8ac`, `0x8014f91c` — all inside the screen-fade system;
`0x8014ed60ff` zeros them as a tint reset. There is no other consumer and no time-of-day
clock anywhere in the code.

No weather assets exist anywhere on the disc. All 200 AREA EMIs were checked for a deviating
subfile count, which is how an extra weather/time-of-day subfile would show up:

| Subfile count | Areas | What the extra subfile is |
|---|---|---|
| 14 (standard) | 175 | — |
| 15 | 22 | 21× CODE overlay (per-area handler at `0x801f2c00`: areas 024, 049, 067, 077, 104, 108, 121, 135, 145, 173; at `0x801eec00`: areas 175-185 share one identical 15,696-byte overlay) + 1× extra texture (AREA004, 65,536 B at VRAM `0x1a080400`) |
| 23 | 3 | 030, 089, 129 — the minigame stages |

AREA051 (Mt. Myrneg), the one area suspected of rain, has exclusively 47 star objects: no
overlay handlers, no glow effects, nothing resembling precipitation. Story-sequence particle
EMIs exist (`SCE10EFF`, `SCE15EF0-3.EMI`, tied to cutscenes SCENA10/15) but are one-off
scripted cutscene particles, not a general area-weather system.

Implementation: AREA189, previously an endless wrapping plane with no exit, is now leavable
using the disc-exact destinations above (state 0 → AREA191 (30,22) dir 4; state ≠0 →
AREA192 (19,22) dir 4). Both paths are tested in the browser, arriving at (30.5,22.5) and
(19.5,22.5) respectively, with no errors. ⚠ Only the exit TRIGGER is approximated — the
original stalls the player via a 13×`rand()` script routine, a separate navigation
subsystem outside this chapter's scope; the browser instead exits on the third map
wrap-around (`DESERT_EXIT_WRAPS = 3` in `main.ts`). `__bof3.desert(v?)` reads or sets the
state byte directly, reproducing both of its effects at once: visibility of all 375
`cond`-`0xff` features, and the desert's exit target.

### Extraction and verification tooling

| Tool | Purpose |
|---|---|
| `extract/build-water-anim.ts` | Renders per-area affected tiles per phase into `public/water/areaNNN.{png,json}`; carries both the VRAM-upload and the `0x82`-class (`perAreaSeq`) data |
| `extract/extract-anim82-phases.ts <area> --state <name>` | Lifts every bank of a multi-phase `0x82` group from a settled savestate's VRAM |
| `extract/build-cloud-tex.ts` | Extracts cloud/shadow textures (`cloudA.png`, `cloudB.png`, `clouds.json`) |
| `extract/build-oceanwaves.ts` | Extracts the two wave-layer band textures |
| `extract/dump-series.ts <state> <tag> [--n] [--gap]` | Time-series capture: N GPU dumps on a fixed grid plus one savestate anchor |
| `warp.ts <area> <x> <y> --dump --save` | Player-anchor calibration: exact position plus paired GPU dump; used for the smoke emitters |
| `render-gpudump --blend` | Painter mode that renders modulation and all four semi-transparency blend modes, required for judging any semi-transparent surface |
| `gpudump.ts` `uploads[]` | Per-frame `VramUpload` log, usable for any VRAM-animation RE |
| `__bof3.desert(v?)` | Debug read/write of the day-night state byte |

Recurring verification lessons:
- Savestates taken with the same `--wait` value lock to the same animation phase
  deterministically; vary the wait to sample different phases.
- For judging water-cell classes, trust framebuffer/savestate VRAM over dump recon — dump
  recon can carry stale VRAM content (e.g. leftover font glyphs) that never appears in the
  live framebuffer.
- Judge scores on animated surfaces plateau below 100% by design: `pd104` at 90%, `pd088`/
  `pd121` at ~70%. An animated layer cannot line up pixel-for-pixel with one static GT
  snapshot, since its own phase, camera framing, and any moving neighbor content (cloud
  shadow, waterline runout) vary independently. This is treated as expected error, not a bug.
- Sample dumps roughly every 6 s when matching drifting content across frames; 30 s
  intervals lose correspondence across wraps. Always read dump-interval timestamps from
  original files — `cp` resets mtimes.

### Refuted approaches

- **Entry rotation as a fourth water-animation class.** An early reading of AREA033 (and
  AREA016) found 6 `textureData` entries rewriting their tileX at runtime — e.g. (2,3)→(0,1)
  on page 2, rows 11-13 — and read it as a "river-wave cycle". This was wrong: the cells
  belong to the Yraall intro bridge (column 58-59, row 37-39), and the diff is a `[20]` state
  patch between an intact plank and a broken hole (from the intro cutscene), not an
  animation. The patch groups already existed in `public/gamedata/tile-patches.json` (016
  group 2, head `0x2800`: aux `0x278000b2` → patch `0x278000b0`, tileX 2→0, all 6 tiles; 033
  group 1, head `0x4001`, pending). Evidence: the "hole" cell pair is a plank-frame-plus-opening
  graphic, external ground truth (VGMaps) shows the bridge intact by default, all available
  savestates carry the story flag for the broken state, and a forced animation rebuild
  produced visibly traveling black holes rather than a clean cycle. The experimental
  extractor `extract-entryrot-phases` was built, refuted, and removed.
- **A second cloud-shadow class.** A distinct prim type at AREA045, briefly misread as
  another cloud-shadow variant, turned out to be an unrelated decorative sprite. The
  misreading came from the plain painter recon rendering semi-transparent prims fully
  opaque, which made both clouds and the unrelated sprite look like solid black shapes.
- **Rain at Mt. Myrneg / a general weather system.** An early working note ("rain Mt.
  Myrneg, snow?") was speculative. A direct GT video test (warp to AREA051 (20,20), 4 s)
  showed no rain at the default story state (frame diff = 0); the full 200-area asset survey
  above later confirmed no weather asset exists anywhere on the disc.
- **A global day/night RGB tint.** The only RGB-modulation writers found all belong to the
  screen-fade system; there is no time-of-day consumer and no clock driving one.

### Open

- Exact original per-phase tick timing for the VRAM-cell-upload class is unconfirmed; the
  browser approximates a 350 ms ping-pong interval. Only the 0x82 class (AREA104) has a
  GT-derived exact cadence (640 ms/phase).
- The 4 haze bands of AREA102's `clut102` series (2040-px stripes) remain unsolved.
- The ocean wave-layer judge ceiling (`pd088`/`pd121` ~70%) and the AREA104 lake ceiling
  (`pd104` 90%) are not pursued further — both remainders are attributed to independently
  varying animation phases and stale GT recon content, not renderer bugs.

