> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 11. World structure: navigation map, sections, camera and warps

### Navigation map

Each area's map subfile carries a complete per-tile navigation/zone map, `cols·rows` bytes
sized by `mapHeader.readUInt16LE(20)*4`. The game loads it into RAM at `0x8010bd30`; the
extractor's copy is byte-exact against live RAM. `buildAreaGeometry` exports it as `walk[]`
in the area JSON.

| Code | Meaning |
|---|---|
| `0x00` | interior floor, walkable |
| `0x40` | overworld ground, walkable |
| `0xa_`, `0xc_` | warp door / landing, walkable, triggers a transition |
| `0x10` | blocked / void, not walkable |

`walk != 0x10` is exact walkability and matches the rendered floor (e.g. AREA007 tiles
1216/1222 are walkable and render; walls stay blocked). 192 of 200 areas match exactly,
including every true overworld region: `AREA016/033/045/060/065/087/088/115/121/151/152`
and others use `0x40` for overworld-walkable ground and `0x10` for water/mountain — the
overworld is pixel-exactly walkable with no heuristic.

⚠ Eight areas once assumed to be overworld areas needing a heuristic fallback are actually
script-navigation special cases: `AREA030/031/089/129/149/186/189/190`. Their ROM nav map
is deliberately `0x10` everywhere. Area 189 ("Desert of Death") uses direction-loop
navigation designed to disorient the player; area 186 is a moving "loop elevator" platform;
the rest are narrow scripted corridors. `grid.ts` falls back to a relief heuristic for
these: tiles carrying relief count as structures, every tile is assigned by distance to the
nearest relief tile, and the region richest in relief becomes the main map — an
approximation only; an exact result needs each area's specific navigation mechanic
implemented individually.

### Sections

An EMI area is not a single map: outdoor terrain (village), interiors, and flat filler
layers all share one tile grid. `grid.ts`'s `computeWalkRegions` splits that grid into
sections — the connected components of non-`0x10` tiles, which double as camera zones.
Structural tiles carrying no direct walk code are assigned to the nearest room by BFS. The
whole grid always renders; only the active section drives character placement, walkability,
the minimap, and the warp target, selectable in the UI.

### Camera model and glide

Camera zones are exactly the sections defined above: the active room determines what the
camera frames. The Overworld camera itself never rotates, holding a constant iso-north
orientation. This follows from the compass needle, which is not a sprite but an untextured
red/blue Gouraud quad drawn as a fixed parallelogram at a constant ≈−24° — a rotating
camera would need the needle to counter-rotate every frame, and it does not. A separate
camera-update helper (`f1d` in the disassembly) sits near the warp loader but is not
invoked on the direct warp-load path.

### Warps and transitions

The field FSM tick at `0x80197178` zeroes `0x80143b90` at the start of every frame and
derives state from the pending-action flag `0x80143bb0`; poking `b90` directly has no
effect, since it is overwritten before the next frame draws. The real warp request,
`0x8019fc28`, instead writes a destination struct and sets the trigger byte:

| Field | Address | Content |
|---|---|---|
| area | `0x80143f10` | destination area, u16 |
| X | `0x80143f14` | X position, Q16 fixed-point (`X<<16`) |
| Y | `0x80143f18` | Y position, Q16 fixed-point (`Y<<16`) |
| dir | `0x80143f1c` | arrival facing, u8 |
| trigger | `0x80143bb0` | `5` = warp |

`bb0=5` dispatches through table slot `table[4]` = `0x801973e8` to the loader
`0x8019fca0`, which takes area/X/Y/dir as arguments. `extract/warp.ts` reproduces this
struct write from outside the game (`npm run warp <area> <tileX> <tileY> [--state door]
[--src N] [--dir 4] [--dump tag] [--save name]`), making any area and tile reachable
without HID navigation.

Each warp record also carries, at byte offsets `+8`/`+9`/`+10`:

- `+10` `dir` (0-7) = arrival facing in the engine's 8-direction scheme: `0=NW, 1=N, 2=NE,
  3=E, 4=SE, 5=S, 6=SW, 7=W` — the same convention as the mover/STEP vector table at
  `0x80181f8c`. Proved statistically: pairing every warp with its nearest return warp and
  checking the vector "return-warp source → arrival tile" against `dir` scores 791/966 =
  82% under this scheme. Example, McNeil: `(27,21)` → shop `(31,76)` dir `1`=N (enters
  facing north); return `(31,77)` → `(27,22)` dir `5`=S (exits facing south). Browser:
  `pendingSpawn.face`, set by `doWarp` (maps `dir` to `Dir8`) and applied by
  `buildActiveSection` after positioning — verified with Playwright (`warpTo` dir `4` →
  `down_right` at `(31.5,76.5)`).
- `+8` `b8` = axis of the trigger strip (`0` = strip runs along X, nonzero = along Y).
- `+9` `b9` = length of the trigger strip, in tiles.

The warp-apply routine `0x801a02dc` (field savestate / menu-field context) reads each
record as a strip, not a single tile:

```
lbu v0,9(rec) · addiu v1,v0,-1         ; k = b9-1, loop k..0
lbu v0,8(rec) · beq v0,zero,0x801a032c ; b8 selects axis
  b8 != 0 : x == src_x     && y == src_y + k   (strip along Y)
  b8 == 0 : x == src_x + k && y == src_y       (strip along X)
```

Across 884 records, `b9=2` (×770) covers ordinary two-tile-wide doors and passages; `b9=3`
(×114) covers wider passages and clusters in cave/passage areas (`048`×6, `074`×5,
`112`×5, `056`×4, `150`×4). Cross-checked against the nav map, 833 of 882 records carry a
warp code (`0xa_`/`0xc_`) on every strip tile; the 49 exceptions sit on walk-`0x00` tiles
(script warps). `b9=3` covers both location↔overworld transitions (e.g. area `0→16`,
`7→33`) and purely internal passages; a "different passage type" reading (e.g. transitions
without a door stop) is plausible but unconfirmed. `b8` splits 375/506 (plus 3 outliers)
and shows no correlation with `b9`. In the browser, `warpCovers()` in `main.ts` now tests
the full strip — earlier it fired only on the first tile even though the nav map marks the
whole width — and `build-warps` exports `len` and `axis` per record.

`public/warps.json` connects sections as a door graph; every warp tile carries `0xa_` or
`0xc_` in the nav map.

### The overworld

The overworld HUD — compass, legend box, region banner, spot marker — is disc-static
inside `AREA016.EMI`. Ground truth: a GPU dump of Yraall showing the compass, a legend box
("×Enter / △Guide / START camp"), the "Yraall Region" banner, and a marker over the
character. VRAM page `(768,256)`, CLUT `494` holds the compass dial, legend box, and
banner bars, plus the ship HUD ("ENGINE"/"OVER HEAT"). VRAM page `(448,0)`, CLUT `497`
holds the "?"/"!" markers, yellow place-name labels (up to three lines: "Fishing Spot",
"Farm", "McNeil", "Cedar Wood", "Ogre Road", …), and the spot-icon / map-figure frames. The
compass needle is not a sprite but an untextured red/blue Gouraud quad, a fixed
parallelogram at ≈−24°, consistent with the camera's constant iso-north (see Camera model).

`extract/build-worldmap-ui.ts` writes `public/worldmap-ui/` (compass/legend/banner body
plus a cap that mirrors the body's left edge; sheet `(0,80)` already holds a second legend
copy). In the browser, `main.ts`'s `owHud` runs only on the 10 `OVERWORLD_AREAS`, at
integer PSX pixel scale, with the needle as inline SVG and the region banner anchored
bottom-center.

Region names come from disc-exact text-block strings, curated per area because the raw
engine title-slot index is unreliable (`AREA016`'s title slot literally reads "ominous
voice"):

| Area | String | Region name |
|---|---|---|
| `016`/`033` | block #4 | "Yraall Region" |
| `045` | block #2 | "Central Wyndia" |
| `065` | block #2 | "Eastern Wyndia" |
| `087`/`088`/`115`/`121`/`151` | title slot | (direct) |
| `187` | — | ship map, no banner |

This mapping is curated as `OW_REGION` rather than read generically. Spot behavior,
clarified by ground-truth clip `g02`: proximity to a spot shows a place-name sign over the
character; standing on the spot shows its type icon (e.g. a tent) or "!" if unvisited —
implemented with a gold-frame/dark-body sign, the original "!" marker, and `friendlyName`
text. Label texels are extracted per area as row strips (`public/worldmap-ui/areaNNN/`,
`labels.json`); automatic per-sign segmentation fails where adjacent signs share no alpha
gap, so exact bounds would need the engine's own label-rect table. Evidence:
`references/screenshots/owhud-2026-07-12/` (`index.html`).

### Story phases

`0x80146870` is a single monotonically increasing story-phase counter. Exactly one routine
writes it, field engine code at `0x8019ff2c`-`0x8019ff58` in `GAME.EMI` (identical across
independent RAM captures). Logic:

```
if (byte[0x80146871] & 0x80) {
  phase = byte[0x80146870] + 1
  if (phase == 0x10) phase++
  store phase to 0x80146870
  call 0x801a7984
}
```

Immediately before that check, the routine copies 8 bytes to `0x801481e0` (`lwl`/`lwr`)
and reads the current area from `0x80143f00` into an `0x8018`-prefixed table — the area
load context. Phase `0x10` is deliberately skipped. Story events advance the phase through
a single flag bit, `0x80146871.7` (bit 7 of byte `0x146871`, one of the dialog sub-state
bytes in the save's `stateBlock16`), set by the SCENA/NPC-VM event/script layer. No further
evaluation exists beyond counter and flag. A save importer could read the phase directly
from save file offset `0x200`.

### Refuted approaches

- The 8 script-navigation areas (`030/031/089/129/149/186/189/190`) were first read as
  "~8 overworld areas" needing a heuristic fallback; their nav maps are fully `0x10` and
  they are not overworld at all — the true overworld (10 areas) needs no heuristic.
- Warp `dir` under an `0=N`-based 8-direction scheme scored only 8% against the
  return-warp statistical check, versus 82% for the actual `0=NW`-based engine scheme.
- `b9=3` as "map-edge transition": refuted, only 20 of 114 cases sit near a map edge.
- `b9=3` as "area change": refuted, 72 of 114 cases stay within the same area.
- Poking `0x80143b90` directly to force a warp: ineffective, the per-frame FSM tick clears
  it before it can take effect; only `0x80143bb0=5` triggers the loader.

### Open

- `b9=3` as a distinct "passage type" (e.g. no door-stop) is unconfirmed; deciding it needs
  the `b9` consumer in the state-5/warp-apply code path.
- `b8`'s split (375/506, 3 outliers) does not correlate with `b9` — cause unresolved.
- Door-opening choreography during a warp (ground-truth item (e)) is still unresolved.
- The 8 script-navigation areas still render via the approximate relief heuristic; an
  exact result needs each area's specific navigation mechanic implemented individually.
- Worldmap HUD polish: individual (segmented) texel signs, animated spot-type icons, and
  the △-guide description text (text block #5) are not yet implemented.
- The engine's label-rect table (exact per-sign bounds) has not been located.

