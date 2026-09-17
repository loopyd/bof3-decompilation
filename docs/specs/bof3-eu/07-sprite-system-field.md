> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 7. The sprite system and field characters

All field, battle, furniture, and spell sprites share one representation: a self-contained
`[vc][cells]` sprite program, decoded once per character or object file and driven at runtime
by a dispatch table. Field characters (`PL###`/`BPLD###.EMI`, e.g. `PL034` = Ryu) add a second
layer, the `PLP###.EMI` behavior overlays, which select dispatch indices for idle fidgets and
context actions but draw nothing themselves. This chapter covers the on-disc package format,
its geometry records, the anchor point that keeps frames from jittering between directions, the
dispatch tables that select an animation, the id space that separates child from adult sprites,
and the overlay system that drives idle behavior.

### The sprite program model

Every `PL###`/`BPLD###.EMI` character file stores its ct1 chunk as a self-contained sprite
package: texels, CLUT, animation programs, and dispatch table. It sits disc-LZSS-compressed,
resident in RAM at `0x8003b800`, and decodes without any external capture or runtime state.
`PL034` (Ryu) decompresses to `references/sprites-wip/plchar-vram/PL034_ct1_decompressed.bin`,
291900 bytes, and matches more than 20 savestates byte-exact — confirming the package needs no
live capture (see Refuted approaches).

Container layout: `u32[0] = 0x0c` points to the first of three blocks, at offsets `0x0c`,
`0x127d8`, and `0x220a8`. Each block holds four `u32` sub-offsets to:

| Sub-block | Content |
|---|---|
| sub0 | texel record table |
| sub1 | CLUT |
| sub2 | animation table and programs |
| sub3 | dispatch table |

For `PL034`, block 0 holds 29 texel records, block 1 (the field set) holds 23, block 2 holds 33.
Evidence images: `PL034_walk_cycles.png` (all stands plus 6-frame walk cycles down/side/up) and
`PL034_anim_catalog_block{0,1,2}.png`, both under `references/sprites-wip/plchar-vram/`.

### Cell and geometry records

sub0, the texel record table: the first `u32` is `4 × count`; each record is
`[u16 w=128][u16 h][u32 mode]`.

- **mode 1** — a packed 5-bit index stream at `+8`. Each `u32` holds two 16-bit half-words, each
  packing three 5-bit indices plus one gap bit (bit 15/31), at shifts `0, 5, 10, 16, 21, 26` —
  not the evenly spaced `k·5` (`…,15,20,25`) an earlier reading assumed (see Refuted approaches).
  The correct shift schedule comes from the MIPS decoder at `0x8014e948` (`srl 5,5,6,5,5`).
- **mode 2** — `[u32 outSize]` at `+8`, then LZSS data at `+12`, decoded by `decodeCtype7Cell`
  (`extract/ctype7.ts`).

Both modes output a **256×h pixel band**, 8bpp (indices 0–31), streamed 1:1 into VRAM hardware
column 320. Band width is always `2·w` (`w` counts 16-bit VRAM words; 8bpp packs 2 texels per
word) — not a `128×2h` de-interlace, which an earlier reading used and which silently dropped
the right half of wide sprites like Nina (see Refuted approaches).

sub1, the CLUT: `[u32 = nClut·4][nClut × (32 × u16 BGR555)]`, cross-checked against VRAM row
495, column X0 (Teepo's palette). `nClut = u32[0]/4`; multi-CLUT characters (Nina, Garr, …)
chain several 32-entry palettes back to back into one combined palette (index 0–31 = CLUT0,
32–63 = CLUT1, …; mode-1 records only ever use 0–31, mode-2 records can exceed 31). The CLUT is
constant per record — the renderer (`0x8014c62c`) loads it once from the caller's stack
(`0x10(sp)`), not per quad. A separate per-quad U-split (`s4`, `0x8014c918`) controls the TPage
prim at `+0x16`, not the CLUT.

sub2, the animation table and programs: `u16[0]/2` is the program count. A program is
`[nSteps:u8][nRecs:u8]`, followed by `nSteps × (tick, recIdx)` and `nRecs × u16` record offsets
(relative to `prog+2+2·nSteps`). Each offset points to a geometry record — the sprite system's
basic drawable unit, a **cell**:

```
[vc] + vc × [flag][Xs:i8][Ys:i8][U:u8][V:u8]
```

`vc` counts the quads that follow; `U`/`V` index straight into the texel band. `flag` encodes
quad size and mirroring:

| Flag bits | Meaning |
|---|---|
| bits 0-1 | quad width = `((flag&3)+1)·8` px |
| bits 2-3 | quad height = `(((flag>>2)&3)+1)·8` px |
| bit 7 (`0x80`) | horizontal flip of this single quad |

A standard walk cycle is 6 steps at tick 4 (60 fps). The `flag&0x80` quad-flip is the actual
cause of the old "`U≥232` green/cross" artifact; no per-quad TPage bit exists (see Refuted
approaches).

⚠ The extractor's `plausibleRecord` filter originally capped record displacement at 72 px —
correct for stands and walk cycles, but it silently discarded 8 of 10 frames of action programs
with wide arcs (adult Ryu's `d42` sword strike swings 113 px). For dispatch index ≥16, the cap
now matches the canvas limit, 128 px; the action catalog grew from 1717 to 1794 frames once
corrected.

### Anchors

Frames are rasterized onto a shared union box, but that box is computed **per dispatch entry**
— each direction and each animation cycle gets its own origin. Motion is jitter-free within one
cycle but not between directions or cycles.

The renderer's actual reference point is the **sprite anchor**: the entity's screen position,
set by `0x8014c62c`. Geometry-record `Xs`/`Ys` are relative to that anchor, not to the frame's
bounding box. An earlier browser implementation hung every frame image at bottom-center
(`sprite.center = (0.5, 0)`) as an estimate — plausible-looking, but not what the data encodes,
and it produced a direction-change offset of up to 26 px, more than a tile (see Refuted
approaches).

**Fix:** the extractor now writes the true anchor as `ax`/`ay`, in canvas pixels, into every
series — directions, fidgets, field actions, action catalog:

```
ax0 = 1 − u.minX
ay  = 1 − u.minY
ax  = W − 1 − ax0     (when the dispatch entry's hflip bit is set)
```

The hflip case works because `mirrorX` mirrors the frame around the canvas center; mirroring
around the center while carrying the anchor along is equivalent to mirroring around the anchor
itself. `src/player/player.ts` and `systems/multiplayer.ts` read this back as
`sprite.center = (ax/w, 1 − ay/h)`. Sprite sets published without an anchor — Weretiger, dragon
forms, adult Teepo — fall back to center-bottom. Debug switch `__bof3.spriteAnchor(false)`
reverts to the old bottom-center anchoring for comparison.

**Verification** (`references/screenshots/feldfiguren-2026-07-30/`):

- **Mirror test, 0.0 px**: left/right stands sharing one program produce exact mirror-image
  silhouettes relative to the anchor, confirming the hflip formula. Exception: child Ryu, 3 px —
  left and right are two distinct programs there, not a mirrored pair.
- **Foot point across the full walk cycle, 8 directions, mean of 12 sprite variants**: x settled
  from 10.7 px to 4.0 px, y from 4.3 px to 2.0 px; apex jump from 4.5 px to 2.2 px. Worst case,
  child Rei: 26.3 px to 4.5 px — at 22.9 px/tile, the pre-fix drift exceeded a full tile.
- **Browser, fixed camera and world position, crosshair on the projected standpoint**: the foot
  line across down/left/up/right dropped from 15 px to 0 px.
- ⚠ Silhouette center of mass is **not** a valid error metric here — in profile the torso leans
  in front of the foot point, so measuring it reports roughly 40 px of "error" under both the
  old and the fixed anchoring, leading to the wrong conclusion.

### Direction and animation dispatch

sub3, the dispatch table: 3-byte entries `[texelRecIdx][progIdx][hflip]`, selected by index
`ctx[0x4b]`. Directions and animation state map onto fixed index ranges (`PL034`/Ryu shown;
other characters share the same layout, extended for their own program count):

| Dispatch index | Content |
|---|---|
| 0–7 | stands, 8 directions (4 = down, 7 = up, 6/2 = side left/right, live-anchored) |
| 8–15 | 6-frame walk cycles (12 = down, tex3/prog7 · 14 = side-left, tex2/prog5 [live-verified] · 15 = up, tex4/prog9; 9/10/11/13 = flip/diagonal variants) |
| `0x3e`/`0x3f` (62/63) | idle break, short (2 frames) |
| `0x40`/`0x41` (64/65) | idle break, long (18–40 frames; a/b = look-side pair). The raw dispatch catalog first read this pair as a door-opening animation; PLP disassembly later traced the idle-break state machine to the same indices, and the door reading did not hold up |
| `0x42`–`0x45` (66–69) | field action, 4 movement axes (detail below) |
| `0x46`–`0x49` (70–73) | second action quartet, gated behind a height test (`slti 0x41`) — open, presumably a jump |
| `0x50` (80) | single action, e.g. `setAnimState(0x50, 2)` |
| 16–127 (remainder) | one-frame emotes plus the full action catalog: 275 series / 1794 frames, deduplicated by `texIdx+progIdx`, across 12 sprite variants (Ryu 45, Momo 27, Nina 25, … — swings, angling poses, door, startle, nodding, among others). `PL034` (Ryu) reaches up to index 108–127 (`nProg` = 106) |

Diagonal dispatch mapping within 8–15 (indices 0/1/3/5) is confirmed only structurally, not by
animation content (see Open).

**Field action (the action button): dispatch quartet `0x42`–`0x45`.** Disassembly of `PLP034`
(overlay at `0x801ce400`; the identical pattern recurs at `0x801cfc7c`, `0x801cf48c`,
`0x801cea38`):

```
lw   $v0, 0x44($1f80)        ; ctx = field entity, 0x80145e90
lhu  $a0, 0x2c($v0)          ; sprite package index
jal  0x8015e1cc               ; PlaySfx(0x100 + package) = bank 1 (PL###-EMI-ct8 set)
lbu  $a0, 0x08($v0)          ; dir (0=up … 4=down, clockwise)
addiu -1 / srl 31 / addu / sra 1 / addiu 0x42   ; axis = (dir-1)/2, round to zero
jal  0x8014da24                ; setAnim(code)
sb   8, 0x0a($v1)             ; duration, 8 ticks
sb   1, 0x03($v1)             ; busy flag
```

`0x42`–`0x45` are four axes of the same movement, mirrored to the opposite direction by the
row's hflip byte:

| `dir` (`ctx+0x08`) | axis = `(dir−1)/2`, round to zero | dispatch index |
|---|---|---|
| 0, 1, 2 | 0 | `0x42` |
| 3, 4 | 1 | `0x43` |
| 5, 6 | 2 | `0x44` |
| 7 | 3 | `0x45` |

The round-to-zero division is confirmed by disassembly, not an extraction artifact. Which
movement plays is decided entirely by the character's own ct1 dispatch table — the same PLP
code path drives Ryu's sword strike (with a slash visual effect), Teepo's kick, Nina's staff
spell, Momo's shoulder cannon, and Peco's spin. **Garr and Rei have no field action**: their
rows `0x42`–`0x49` duplicate row 4 (down stand), i.e. an empty entry. Rei's lock-picking runs
through the object-anchor scripts instead (`systems/inspect.ts`, condition `char==4`), not
through the action button.

The accompanying sound sample is selected by `ctx[0x2c]`, the character's **ct1 block index** —
confirmed by comparing every DuckStation savestate: hashing the ct1 blob resident in RAM
(`0x8003b800`) identifies the PL file, and the anim cursor (`ctx[0x50]`) against the three block
boundaries gives the block. `PL034` block 1 → `ctx[0x2c]=1` across 304 states; block 0 → 0;
`PL278`/`PL349`/`PL567` block 2 → 2. Only non-field states (e.g. fishing = 12) deviate. Browser:
`field-manifest.players[<PL>] → cue<block>.wav`.

Implementation: `Player.playFieldAction()` picks the axis by the disc formula and aborts on
movement, the same as a fidget. Enter is the last link in the action chain — warp → object
anchor → switch/mover → search spot → field action — matching the original handler order at
`0x801cf940`. The action bar exposes it under category "animation" ▸ "field action", grayed out
for Garr/Rei; `__bof3.fieldAction()` triggers it from the console. Browser verification (rAF
sampler): Ryu plays `Ryu_field_axis1_f00…f05` in 233 ms; Nina 13 frames, Teepo 8, Momo 15
frames/588 ms, Peco 11, adult Ryu 10; Garr and Rei report `ok=false`.

### Character id space: child and adult

Field, battle, and transformation sprites are separate files keyed by character and age, not by
one shared id:

| Sprite id | Context | Role |
|---|---|---|
| plchar anim entry "Charakter 9" | field (world) | child Ryu (`c9`); world-character option "Ryu (child)" |
| `BPLD012` (party-anim) | battle | child battle Ryu ("Ryu") |
| `RYUD00` (dragons-anim) | battle | adult personal battle Ryu, parallel to `REID` = Rei |
| `CRYUD00` | battle (Accession cast) | child Accession caster |

**Adult Teepo** falls outside the PL/PLP party-combination system, which only covers permanent
party bundles; it is a separate, non-party sprite. `AREA172` descriptor 625 carries 25 programs,
including:

| Program | Content |
|---|---|
| p0, p1, p3, p4 | directional stands |
| p5 | 6-frame side walk |
| p6 | cell-identical to p5 — the direction flip lives at the step level, not in the cells (diff-proof 100%) |
| p12 | 14-frame transformation choreography |
| p13 | 9-frame back-throw |
| p15–17 | cloth-shred particles |

There are no down/up walk programs — reference footage (Eden) only ever shows adult Teepo
moving sideways. The field character uses p5 and its mirror p5m as left/right walk plus the
directional stands. The bestiary entry id625 is curated as "Teepo" (`build-boss-figures` +
`enrich-community`).

**Dragon forms** (`systems/accession.ts`) extend the field-character system to non-human
sprites. The form catalog covers the 43 disc-reachable (shape, palette) pairs
(`dragon.ts`/`FORM_SPRITE`: 4 shapes × palette rows 0–6, plus a GOLD row 7 with canonical names;
Kaiser = `RYUD10`; Tiamat and Pygmy; 4 hybrids; Super). Field sprites are sourced only from each
form's dragons-anim **idle** series — there is no dedicated field walk set. D-suffixed forms use
down/left/right frames; U-suffixed forms add an up frame (key regex `^(C?RYU|PAPY|REI|RT)D`);
DRG-shaped forms have no U EMI and fall back to the down frame. `_r{row}` selects the palette.
`FIELD_SCALE` is fixed at 0.7 (field Ryu measures 32 px against battle Ryu's ~49 px), so
relative proportions between forms stay disc-authentic. `player.ts` adds `FieldFrame.tick`
(tick-exact idle playback at 60/s, replacing the flat `IDLE_FPS` rate) and `setTint`
(palette-flicker phase) to support these forms. An asset sweep (HTTP HEAD check) confirmed all
43 forms, across every idle frame and every U view, are present with 0 errors.

Kaiser (gene flags 4/7/8) uses the golden-Ryu series `RYUD10` as its field and world sprite; the
similarly named KAIZAR giant dragon is a BMAGIC spell effect (the KaiserBreath attack), not a
selectable form or field sprite.

### Idle behaviour overlays

19 `PLP###.EMI` files (party-combination codes such as `012`, `034`, …, `678`, `27A`) are
**code overlays** for the field-character system, loaded at `0x801ce400`. `PLP034` is
byte-identical (100%) across both the menu-field and `a033` savestates; all 19 PLPs are
mutually ~97% identical — a shared skeleton plus 6–10 KB of per-character parts.

**Overlay ABI:** header `[id=0x21:u32][MIPS code]`; at the overlay's end, an export table of ~9
function pointers (`PLP034` at `0x801d033c` onward). Slot 3 holds the resident default
(`0x801b2fd8`); slot 4 holds the state dispatcher (`0x801ce404`). The kernel never calls into
the overlay directly, only through a pointer from this table.

**Dispatcher** `0x801ce404` reads the state index at `ctx(0x1f800044)[+2]` and indexes a
12-slot vtable (`PLP034` at `0x801d02f4`; slots 2/8 fall back to the resident default). Handlers
set `ctx[+4]` — the same field later read as the plchar dispatch index — with flag checks (e.g.
`@0x80143f02 & 0x40`) and small state machines. Call profile: the anim-sequencer family
(`0x8014da24` ×10, `0x8014dc3c`, `0x8014dac8`), `rand` (`0x8017e8a0` ×4, driving idle fidgets),
`ENT_ALLOC` ×3 (companion entities), `0x8015e1cc` ×10 (PlaySfx), `0x801b2f84` ×8, and `getNav`
(`0x80166f64`) ×8.

**Two state layers share `ctx[4]`:**

| Layer | Range | Handler table | Notes |
|---|---|---|---|
| resident BASE | `0x00`–`0x1C` | `0x801cd594` (28 handlers), dispatcher `0x801b7b90` | core field states (idle/walk/…); handler 1 = auto progression, `ctx[4]++` |
| PLP-own | `0x40`, `0x41`, `0x42`, `0x50`, … | in-overlay handler-0 state machine | per-combination behavior |

The overlay's internal setters (`0x801cef14`, `0x801ced1c`) are **emote-entity spawners** —
entity type `0x34`, subtypes 0/1/4 forming a `?`-bubble/`!`-marker family — not animation
setters as first assumed. Character animation itself runs entirely through the `ctx[4]` codes:
**`ctx[4]` is the plchar ct1 dispatch index**, the same 3-byte table used for stands and walks.
`PLP034`'s formula `(dir−1)/2 + 0x42/0x46` selects a direction quartet directly in that space
(see Direction and animation dispatch); `d3e`/`d3f`/`d40`/`d41` are the idle-break codes, set by
the idle branch of PLP handler 0.

**Fidget mechanic** (`PLP034`): a chain of `rand` gates with fixed quotas decides whether, and
which, fidget plays.

| Gate | Condition | Effect |
|---|---|---|
| 1 | `rand&0xf < 0xd` | ~81% chance of no fidget; base values 5/2, ×10 (→ 50/20 timer/variant split) when flag `@0x80143f02 & 6` is set |
| 2 | `rand&3` | second gate |
| 3 | `rand&7 < 6` | variant `a0=4`, else `0` |

A fidget calls `PlaySfx(0x10b)` (`0x8015e1cc` = bank 1, sample `0x0b`) and spawns an emote entity
via `ENT_ALLOC` (`0x8019621c`), type `0x34`, at the character's position (helper `0x801cfb4c`:
entity field `@0x80143fcd + slot·116 = 0x34`, position `<<16`).

⚠ `PlaySfx(0x10b)`'s bank is context-swappable, not fixed: boot loads FIRST/`COMN_SE` bank 1
with only 10 samples (s00–s09), while `SISYOU.EMI` and `BATL_RET.EMI` carry their own bank-1
replacements (`TOC+4 = 1`). Cue `0x10b` (sample 11) therefore targets whichever bank is loaded
at the time — resolving it requires knowing the calling handler's `ctx[2]` mode (see Open).

**Implemented in the browser:** `build-plchar-anims` exports the 4 idle-break series per
character (`fidgets` in the plchar-anim index, with original tick timing and the anchor fields
from Anchors) for all 7 core characters — e.g. Ryu long 30 frames/119 ticks, Momo 35F/156T, Peco
26F/253T. `src/player` reproduces the original mechanic: idle roll 3/16 every 2 s (≈ the disc's
`rand&0xf ≥ 0xd`), long:short ratio 2:1 (from the 5/5/2 value distribution), side chosen by
facing direction, tick-exact playback at 60/s, aborted on movement. Verified (Playwright): after
45 s idle, both the `long_a` and `long_b` series play through completely.

The full action catalog (dispatch 16–127, deduplicated by `texIdx+progIdx`) is exposed as
`actions` in the plchar-anim index — 275 series / 1794 frames across the 12 sprite variants —
and playable per character as a button gallery in the compendium's party module.

Emote particle sprites (the `?`-bubble/`!`-marker family) remain unextracted, and the link
between idle breaks and emote spawns is unproven — the spawning vtable handler belongs to an
undetermined `ctx[2]` mode (see Open).

### Extraction

| Tool | Purpose |
|---|---|
| `extract/build-plchar-frames.ts` | Decodes ct1 texel bands and CLUTs into per-character frame sheets; carries the mode-1 bit-packing and band-width fixes (see Refuted approaches). All 18 PLCHAR files render true to color |
| `extract/build-plchar-anims.ts` | Renders every dispatch entry (directions, walks, `fidgets`, `field`, `actions`) with anchor fields `ax`/`ay` into the plchar-anim index |
| `extract/build-teepo-adult.ts` (`npm run extract:teepo-adult`) | Extracts the 25 `AREA172`/625 programs to `public/entities/teepo-adult/` (plus `*m.png` horizontal mirrors) |

**Per-frame extraction recipe:** unpack the ct1 blob, parse the four sub-blocks, decode the
texel bands, and apply the CLUT. For each dispatch entry, render its program: for every geometry
record, draw a quad of size `((flag&3)+1)·8` × `(((flag>>2)&3)+1)·8` px at `(Xs, Ys)`, sampling
`band[V+py][U ± px]` (direction of `±` set by `flag&0x80`), with index 0 transparent. The
dispatch entry's own hflip bit mirrors the finished frame; step ticks give playback timing.

Verification: byte-exact match against more than 20 savestates; mirror test at 0.0 px;
foot-point drift reduced from up to 26 px to ≤4.5 px across the walk cycle (see Anchors);
browser crosshair test, 15 px → 0 px. One savestate (`door.sav`) showed VRAM holding `tex7` at
anim slot 5 — not a contradiction, but a streaming-timing artifact: texture upload happens at
the animation's start, so the resident VRAM content reflects upload order, not an indexing
error. Evidence directory: `references/screenshots/feldfiguren-2026-07-30/`.

### Refuted approaches

- **"Walk frames are capture-bound."** Refuted — the ct1 blob is fully self-contained (texels,
  CLUT, programs, dispatch) and byte-exact across more than 20 savestates; nothing needs a live
  capture.
- **"28 frame/body-part pointers at `+0x20`."** This was sub0, the texel record table, misread
  as a flat pointer list.
- **An early, coarser "animTable" theory for sequencing animations.** Superseded wholesale by
  the precise sub0–sub3 container breakdown documented above.
- **mode-1 bit-packing at evenly spaced `k·5` shifts.** Wrong by one bit from index 3 onward;
  the decoder actually uses shifts `0, 5, 10, 16, 21, 26` (confirmed at `0x8014e948`). The error
  produced the multi-CLUT "rainbow" bug — invisible on single-colored Teepo, visible on
  colorful Nina/Ryu/Garr, and never affecting mode-2/LZSS (walk) frames, only mode-1 stand
  frames.
- **`128×2h` de-interlace for the texel band.** Wrong; band width is always `2·w=256`. The
  de-interlace theory silently dropped the right half of wide sprites such as Nina.
- **A per-quad TPage/page bit causing the "`U≥232` green/cross" artifact.** There is no such
  bit; the cause is `flag&0x80`, the geometry record's own horizontal-flip bit.
- **Bottom-center as the sprite anchor (`sprite.center = (0.5, 0)`).** A plausible-looking
  estimate, not what the data encodes; produced up to 26 px of direction-change drift. The true
  anchor is the entity's screen position, carried explicitly as `ax`/`ay` from the geometry
  records' `Xs`/`Ys` origin.
- **Silhouette center of mass as an anchor-accuracy metric.** Invalid — the torso leans in front
  of the foot point in profile views, so this metric reports ~40 px of "error" under both the
  old and the fixed anchoring.
- **`plausibleRecord`'s flat 72 px displacement cap.** Correct for stands and walks, but
  discarded valid frames of wide-arc action programs (adult Ryu's sword strike, 113 px). Raised
  to 128 px for dispatch ≥16.
- **Dispatch `0x40`/`0x41` (64/65) as a door-opening animation.** An early reading of the raw
  catalog; PLP disassembly later showed the same pair is driven by the idle-break state machine
  (18–40-frame long idle), and the door reading was dropped.
- **PLP internal setters `0x801cef14`/`0x801ced1c` as animation setters.** They spawn emote
  entities (type `0x34`), not animation state.

### Open

- Diagonal dispatch mapping within indices 8–15 (0/1/3/5) is confirmed only structurally; the
  specific frame content per index is not verified.
- The role of ct1 block 0 (a second field-frame set, confirmed present via the `sav1`
  savestate) and block 2 (its own CLUT, possibly cutscene use) is unresolved.
- Semantics of the second field-action quartet, `0x46`–`0x49`, gated behind a height test
  (`slti 0x41`) — presumably a jump.
- Rei's lock-picking animation: the object-anchor script only supplies action numbers
  16/55/59/60, with no matching dispatch entry found yet.
- Whether the field-action button triggers hit effects in the original game is unconfirmed.
- `PlaySfx(0x10b)`'s target bank cannot be pinned down without first resolving the calling
  handler's `ctx[2]` mode, since the bank is swapped by context (boot vs. `SISYOU.EMI` vs.
  `BATL_RET.EMI`).
- Emote particle sprites (`?`-bubble/`!`-marker family, entity type `0x34`, subtypes 0/1/4) are
  not yet extracted; the type-`0x34` entity renderer still needs disassembly to determine which
  sprite program each subtype uses, and only then can idle-break-to-emote coupling be confirmed.

