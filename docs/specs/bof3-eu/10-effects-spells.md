> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 10. Effects, spells and transformations

### Dragon move menus

Battle menu builder `0x800a78b8` reads `formId = lbu [0x800b6f58]` and looks up a move triplet at
`0x800b4ea4 + formId·3` (3 u8 skill ids, 0 = empty; table lives in `BATTLE.EMI` sub `0x80096800`,
disc-static). Mapped sets include: f0 Whelp `[120 WhelpBreath, 8 Blind]` — disc contradicts a
"Snap" claim — f1 Dragon `[121, 13 Snap]`, f2 Tiger `[2 Gambit, 19 Aura]`, f3 Warrior
`[226 MeteorStrike, 12 Blitz, 15 Charge]`, f4-f8 element forms `[Breath, Claw]`
(Flame/Frost/Thunder/Shadow+Chlorine/Divine), f9-f13 hybrids
`[Inferno/Blizzard/Myollnir/Death/Resurrect]`, f17 Knight `[Remedy, Restore, Vitalize]`, f18
`[Ebonfire]`. `build-dragon-moves.ts` exports `dragon-moves.json`; `battle.ts` uses the real sets
for the world element row (1-5 → f4-8). Super Combo (skill 3) is not part of this table — from
level 3 it is a master skill in the ordinary skill menu.

**Gene recipe resolver** (`0x800a6c2c`): records at `0x800b4d58` are 3-byte triplets (check
function `0x800a6cf8`: `req = [0x800b4d58 + form·3 + slot]`; an earlier 2-byte reading was wrong).
For form index 0..10, every non-`0xff` slot of the triplet must occur in the chosen gene list
(`0x801463c4`, count at `0x801463c7`); first match wins on index order as priority, no match falls
back to `0xff` (base/element path). `formId = index + 4` (writer `0x800a69d4ff`, return value
passes through `0x800a7c14`). `formId` 8 sets `|= 0x10000` (Berserk); `formId` 7 and 4 set
`|= 0x20000` (Kaiser).

The 11 disc-exact recipes (`build-dragon-recipes.ts` → `dragons.json.discGeneRecipes`):

| Index | Form | Genes required |
|---|---|---|
| 4 | TrueKaiser | Infinity + Trance + Radiance |
| 5 | Trygon | Flame + Frost + Thunder |
| 6 | Wildfire | Miracle + Reverse + Thorn |
| 7 | Kaiser | Infinity + Failure |
| 8 | BerserkKaiser | Infinity alone (first-match explains the taming: 4 and 7 match before 8) |
| 9 | Whelp (Failure) | Failure |
| 10 | Fusion | Fusion gene AND `[0x801462f1]==3` AND hybrid resolver `0x800a805c`, gated by `0x800a6d84` — "formCode 10 never set" is refuted, it is the hybrid entry point |
| 11 | Tiamat | Shadow + Trance |
| 12 | Myrmidon | Force + Trance |
| 13 | Mammoth | Miracle + ??? |
| 14 | Pygmy | Mutant + ??? |

Side table `0x800b4d4c[idx] → 0x800b6f5c` selects the sprite row (row 7 for
Trygon/Wildfire/Myrmidon/Mammoth). Base formId calculator `0x800a724c` (the `0xff` path): gene
class counters at `0x800b6f6aff` give formId 3 (Warrior, `0x6f6d>0`) / 2 (Tiger, `0x6f6b>0`) / 1
(Dragon, `0x6f6a≥2`) / 0 (Whelp).

**Menu system**: the builder loads the base row `formId·3`, then appends per gene class counter
(`0x800b6f60ff`) extra rows via appender `0x800a7b40` (base `0x800b4eb0 = main row idx+4`):
element genes → rows 4-8 (Breath+Claw), amplifiers → rows 9-13
(Inferno/Blizzard/Myollnir/Death/Resurrect), further genes → rows 15-18 (Focus/Counter/Remedy
trio/Ebonfire). The table has 20 rows; an earlier "sets 20-25" reading was overflow into the
following table. KaiserBreath (skill 131) is in no row — the Kaiser menu uses a special path
(anchor `0x800a7c14`, form→menu mapping unread).

**Special form stat table** at `0x800b4ee0` (directly behind the 20 menu rows — the "sets 20-25"
misreading was this table): 11 records of 14 bytes, applied by transformation executor
`0x800a7c14` after the entity backup: `stat := stat·byte/10` (scaler `0x801de0ac`, /100 magic
constant `0x51eb851f`, clamp 0..999; HP additionally carries a `PartyRecord+0x1a` malus term).
Bytes 0-5 = `[HP, AP, pwr, def, agl, int]` factors in tenths (battle struct offsets
`+0x3c/40/42/44/46/48`); bytes 6-13 are follow-up fields (resist-level candidates + struct bytes
`+0x52/53`); a follow-up table at `0x800b5008` (8 B per form) plus the final formId return remain
unread. Values: TrueKaiser/BerserkKaiser = `[30,25,25,15,15,5]` (×3 HP, ×2.5 pwr, int halved);
tamed Kaiser (Failure+Infinity) = `[12,13,12,12,10,2]` (the Failure malus); Wildfire =
`[1,25,30,20,10,6]` (10%-HP glass cannon); Trygon = `[22,18,18,12,10,6]`.

**Base shape stats** (`0x800a6e14`): scales via column sites `0x800b4e79/7a/7b/7c` (+ `formId·5`,
formId 0-3 = Whelp/Dragon/Tiger/Warrior) the stats `[AP@+0x40, pwr@+0x42, def@+0x44, agl@+0x46]`:
Whelp `[13,12,12,10]`, Dragon `[15,15,12,10]`, Tiger `[20,12,15,10]`, Warrior `[20,20,8,10]`
(tenths; Warrior def×0.8 glass cannon). Gene bonus (`0x800a7648`): `idx = clamp(geneCounter+2,
0..4)` → `lb[0x800b4e8c + idx]`, table `[−5,−2,0,+3,+6]` signed tenths, added to every factor
(more genes = stronger form, disc-exact). Element dragons use their base shape's factors; element
formId 4-8 only exists for the menu.

**Special form move lists** at `0x800b5008` (8-byte-aligned start points per special index, skill
ids up to a 0 terminator; lists may overlap — Myrmidon spans 10 skills, Mammoth starts mid-list).
The `0x800a7c14` executor fills the move slots directly (battle struct `+0x70ff`; conditional
skill 217 "Restore Form", rest from the list): TrueKaiser/Kaiser = `[KaiserBreath, Bonebreak,
Howling]`, BerserkKaiser = `[KaiserBreath, Howling]`, Trygon = `[Flame/Frost/ThundrBreath,
DragonBreath, Snap]` (three-headed), Tiamat = `[Doom/Shadow/Venom Breath]`, Myrmidon =
`[Gambit, Aura, 5 Strikes, AuraBreath, GiantGrowth, MeteorStrike]`, Mammoth =
`[GiantGrowth, MeteorStrike]`, Pygmy = `[DragonBreath, Snap, Magma Breath]`.

In the world (key Z spell picker), the current dragon form's attacks list as their own group
ahead of the full spell list, sourced from the same `dragon-moves.json` / `dragons.json` tables
used in battle (menu table `0x800b4ea4`, builder `0x800a78b8`, Kaiser family from `0x800b5008`).
Element row priority: a nested-ternary bug in `battle.ts` matched `group === 'Whelp'` before the
element row, so every Whelp element form fell back to set 0 (`[Whelp Breath, Blind]`) instead of
its own element set — "Whelp · Flame" showed Whelp Breath instead of Flame Breath + Flame Claw.
Centralized as `dragonMoveSetIndex(form)` in `systems/accession.ts` (Kaiser → −1, row 1-5 →
3+row, else Whelp → 0 / base → 1); `battle.ts` and the world picker share it. Verified: Whelp →
`[Whelp Breath, Blind]`, Whelp·Flame → `[Flame Breath, Flame Claw]`, Dragon →
`[DragonBreath, Snap]`, Dragon·Frost → `[Frost Breath, Frost Claw]`.

### Weretiger transformation

Triggered by casting the skill at battle-actor offset `+0x5c = 0x40` ("Weretiger", EU text
"Raises Pwr; slowly become berserk", 0 AP) — castable through the normal confirm path, no menu
hijack required (ability-window entries come from record/battle-actor `+0x5c`; `+0x70` holds the
separate magic list). Requires Rei present in the active battle party.

Ground-truth timeline, frame-exact from a real in-game transformation (video + spectrogram +
brightness curve):

| Offset | Event |
|---|---|
| t−2.0 s | command banner "Weretiger" — silent, Rei in normal idle pose, no crouch |
| t=0 | cue `0x100` = sound s00 growl; battle camera zooms on Rei (~0.35 s, ×≈1.75); screen dims Y 74→45 over 0.3 s (no flash) |
| t+0.4 s | burst grows from Rei's body center; s02 = 5.6-s call sound + s01 27-ms click (cue `0x101`); dense burst phase loops (~5.7 s reference) |
| ≈burst+4.6 s | sprite swap under the dense burst |
| end−0.45 s | second s00 growl (reveal cry) |
| end | brightening over 0.3 s; lines disappear hard, no fade-out (density constant through frame 40) |

An earlier reading ("growl → 4.35 s anticipation → burst") mistook the silent banner/queue phase
for anticipation; the actual growl-to-burst gap is ~0.4 s.

Browser reconstruction (`toggleReiTransform` + `TransformFX.playReal(x,y,z,size,totalMs)`): loops
the dense phase from `LOOP_FROM=6` and fades the last 250 ms; `screenDim()` (black overlay, 0.55
alpha / 300 ms) plus `animateZoom()` (`follow.setZoom` 16→9.2→16). Sequence: s00+zoom+dim → +0.43
s burst (5.6 s) with s01+s02 → swap at +4.6 s → end-s00 → brightening+zoom-back. Verified at scale
17.9→31.3 px/tile, dim 0.55, tiger visible at 4.9 s under the burst.

### Spell replay system

#### Effect container

Every `/BIN/BMAGIC/MAGIC###.EMI` (144 effects + 3 KAIZAR) contains: ct0 at `0x801eec00` = effect
choreography (compiled MIPS, not a byte VM — `[u16 id][u16 flags]` plus a phase pointer directory
for 108/141 effects or direct code for 33; dispatcher reads `ctx[1]=phase`, jumps
`directory[phase]`, calls engine API and manipulates entity structs at `0x80145e90`;
`ctx = scratchpad[0x1f800044]`, the same sprite interpreter used for mesh groups/PLCHAR). ct3
(present in 80/144 files) = effect graphics. ct6/7/8 = spell sound (pBAV VAB format, like SFX,
exported to 221 WAVs).

Graphics geometry: a fixed 256-px-wide macroblock stripe like AREA/enemy pages
(`reconstructTextureVram`, 32×32-halfword blocks), but BPS = width_hw/32 = 2 (4bpp) or 4 (8bpp) —
not a fixed BPS=16, which wrongly imposes a 32-row VRAM wrap. Height follows data size (256×256
@4bpp = 32768 B, @8bpp = 65536 B). bpp varies per effect and is not flagged in the data; detection
uses neighbor coherence (12/12 verified against visual ground truth: 63×4bpp, 34×8bpp). The spell
CLUT is stored inside the EMI itself (ctype0 at `0x80033000-0x80037000`, mostly at `0x80036c00`,
256 BGR555 entries) rather than runtime-bound like enemy CLUTs, giving exact spell colors (files
without a CLUT default to palette 0; 4bpp always uses palette 0). 17 EMIs carry two ct3 subs
(overlay + main sheet). Verified visually: MAGIC004 ice, MAGIC005 fireballs, MAGIC012 summon,
MAGIC088/013 status text ("DEF UP!"/"MISS!"), KAIZAR_N golden emperor dragon.

The id in the ct0 header equals the global engine skill id (MAGIC004 → skill `0x147`, sequential,
linkable to the skill table). Skill-id-to-effect mapping: `skillId → u8 table @0x800b6510`
(BATTLE.EMI overlay `0x80096800`) → effect number → 8-byte descriptor `@0x800b65f8` (`u16[0]` =
disc file id, `0xffff` = resident) → EXE file table `@0x80182910` (u32 LBA per id) → file name.
Result: `MAGIC{n}.EMI` is the effect for engine skill id n (Flare id 91 → MAGIC091, Heal id 70 →
MAGIC070, MeteorStrike id 226 → MAGIC226). The indirection through the effect-number table serves
taught copies (type&3==3 shares the effect number of the original — 40/41 duplicate pairs
identical) and four resident effects with no own EMI. 137/144 files carry a resolved skill name.

Two effects, MAGIC076 ("Raise Dead", full assets present) and MAGIC112, have no entry in the
effect descriptor table `@0x800b65f8` — their file id exists in the LBA table `@0x80182910` but
nothing points to it, leaving them unstartable via the standard path. `runEffectAuto` gives them a
synthetic entry in a free descriptor slot: since the true ct0 header offset used as entry point is
undocumented, every offset occupied by a real descriptor is tried and the run with the most
geometry is kept (Raise Dead: 103 ticks / 10,172 prims this way).

#### Static interpreter

`references/re/vfx-interpreter/run-effect.ts` runs the ct0 as a static MIPS interpreter with
`ctx = scratchpad[0x1f800044]`, driven from a captured battle RAM state (`battlecap12`). Per
20-ms tick it emits the finished GPU primitives plus SFX cue dispatches. `extract/build-spell-
replay.ts` (`npm run extract:spellfx`) exports per spell to `public/spellfx/<TAG>/`.

Caster index gate, found via MAGIC045 disassembly with a `traceRange` instruction trace: the ct0
reads `0x80146384` (caster index) and `0x80146374` (acting combatant) and terminates immediately
if both are below 3:

```
801eec5c lbu v0,[0x80146384] | 801eec6c sltiu v1,v0,3 | 801eec70 beq v1,zero -> real path
801eec98 lbu v0,[0x80146374] | 801eeca4 sltiu v0,v0,3 | 801eeca8 bne v0,zero -> real path
else: 0x801462e8 |= 4 ; jal 0x801e5988 (KILL)
```

Index < 3 means a party slot, ≥3 an enemy; the captured state fixed both to 0, so every enemy
skill died in phase 0 ("phase0 kill"). The plain caster struct address (`0x8014638c →
0x801eb630`) is not sufficient — the indices themselves gate execution. Forcing `casterIdx: 3`
completes MAGIC113 (272 ticks, 19,919 prims, 6 cues) and MAGIC045 Bone Dart (570 ticks);
`runEffectAuto` retries with this override automatically when the standard run yields nothing.

Seven effects carry no own data at all, confirmed disc-side rather than an extraction gap:
MAGIC001 Nue Stomp, 010 Unmotivate, 015 Charge, 045 Bone Dart, 065 Pilfer, 117 Rest, 216 Steal
consist of one ct0 of 928-3328 B with no ct3 sheet, no ct7 audio, no CLUT (file size 4096 B, one
sector; MAGIC045 alone has a sheet but draws like the weapon skills — 1136 calls to
`0x8014d3e0`, zero prim setup). Their visuals and sound come from the combatant animation or
battle engine directly, with SFX cues targeting the running battle's context-dependent bank
(`0x8014871c`, never refilled without an own ct8) — statically unresolvable, so the catalog lists
them empty rather than inventing a replay.

31 further effects — weapon/claw skills: ThundrStrike, Flame/Frost/Wind/Holy Strike, the four
claws, Demonbane, Timed Blow, and others — call `0x8014dc3c` and `0x8014d3e0` roughly 42 times
each, combatant-animation control functions that a drawing effect like Heal never uses (Heal
calls prim-setup `0x8017add0` 10,616 times instead). These effects steer the actor's animation;
the flame/lightning graphic itself (ct3 present, e.g. `public/bmagic/magic005.png`) is drawn by
the battle sprite renderer on the combatant, leaving only ~9 prims in the effect's own stream.
They have no field-context counterpart (no combatants present) and are labeled "weapon effect on
the combatant" or "sound + combatant animation only" in the picker.

Final catalog: 141/141 BMAGIC effects present — 106 full choreography, 16 weapon effect on the
combatant, 12 sound + combatant animation only, 7 without own effect data. A browser sweep across
all 141 effects produced zero errors, no silent spell, no map-wide scattered effects; sound cues
resolved 100% via ct8+VAB with zero fallback cases.

Four fixes were required to reach this state:

1. **Coordinate reading uses s16, not the hardware 11-bit mask.** The interpreter computes in a
   shifted space (effect bases around ~−10,000); reading s11 cut apart prims that never wrap in
   the original (Doom Breath lost all 12,220 of its prims). s16 keeps geometry intact; the base
   offset is resolved afterward via `screenWindow`.
2. **`spanFilterFor` applies the hardware span rule with self-diagnosis.** `primSpanOk` (<1024 x
   / <512 y, measured on differences) holds only once coordinates are correct. For target-based
   effects without actor-position data, a quad corner can hang in nowhere and make every prim
   appear 2500 px wide; if the rule would discard more than 25% of an effect's prims, it is
   treated as inapplicable. Measured discard rates: Weretiger 0.7%, Accession/Sirocco/Inferno 0%,
   Heal 0.5%, Ragnarok 41.5%, Corona 67%, Chill 77%, Tsunami 82%, Doom Breath 100%.
3. **`screenWindow` searches in two stages with an idle safeguard.** y is searched only within
   the prims found in the x window — otherwise, for two-cluster effects, x comes from the caster
   and y from the target, and the window lands on empty space (this dropped MAGIC094 Frost
   entirely). If the window would let nothing through, it is skipped and every prim is kept.
4. **Tick budget raised from 500 to 1500** (was a 10-s cap): the old limit cut six effects
   mid-sequence — Blitz ends only at 30.0 s, Confuse at 26.6 s, Leech Power at 19.2 s, Magma
   Breath at 15.0 s, ShadowBreath at 14.3 s, Dream Breath at 13.9 s. Only Identify and Foretell
   loop endlessly in the original (menu-held open) and remain capped.

#### Primitive kinds and blending

Per-tick exports in `public/spellfx/<TAG>/replay.json`: quads as `[blend, cell, 4×xy, 4×rgb]`,
lines as `[blend, 2×xy, 2×rgb]`, both anchor-relative (anchor = median position, half-width =
92nd-percentile, matching `build-effect-vfx`). Sounds as `[{tick, file, vol}]` (cue dispatches
deduped across wrap/direct forms; cue `0x1nn → magic###_snn.wav`, falling back to whatever sample
is available).

Blend classes, derived from the semi-transparency flag and tpage ABR bits:

| Class | Meaning |
|---|---|
| 0 | back/2 + front/2 |
| 1 | additive |
| 2 | subtractive (back − front) |
| 3 | additive ×¼ |
| 4 | opaque |

Color convention matches the verified rasterizer: textured = `texel·rgb/128`, untextured =
`rgb×2` (both pre-clamped at export time); class-3 additive is scaled ×0.25 again by the player.
Sprite cells are deduplicated by key `(tpage, clut, uRect)` into `atlas.png` plus a cell list;
sheet decode ports `unswizzle`/`effectClut`/`uvToSheet` from `render-frames.ts` (palette index 0 =
transparent).

**Vertex bit width.** The interpreter originally read prim vertices as s16, but the PSX GPU only
honors bits 0-10 (resp. 16-26) — an 11-bit signed value — and ignores the upper 5 bits, which
BoF3 fills with computational garbage. Proven on the real battle GPU dump `battlecap12.psxgpu`:
the upper 5 bits of vertex words scatter freely (`0x2/0x4/0x0/0x5/0x3/0xc …`). Reading as s16 had
produced base offsets in whole 2048-unit steps — the "two coordinate bases ≈ −0x4000 vs. 0" that
`render-frames.ts` had discarded as a foreign cluster. Fix in `parsePrim` (`run-effect.ts`):
`s11 = (read16 << 21) >> 21`; effect: halfW maximum drops from 12501 to 830, Frost (MAGIC094)
from 11812 to 361. The 11-bit values live on a ring modulo 2048: prim span and midpoint must be
resolved cyclically, or a prim crossing 1023/−1024 reads as 2040 px wide instead of narrow.

This 11-bit mask is correct for hardware visibility but wrong as the interpreter's working
coordinate: the effect logic itself computes in a shifted space (bases around ~−10,000), so
masking to 11 bits before computing cuts apart prims that never wrap in the original data (Doom
Breath this way lost all 12,220 prims). The catalog therefore reads vertices as s16 for geometry,
then resolves the base offset separately through `screenWindow`.

**Visibility is a second, non-cyclic rule.** The GPU only draws a primitive when it spans less
than 1024 px horizontally / 512 px vertically, measured directly on decoded values (−1024..1023),
not cyclically. An earlier cyclic span computation ("2048 − largest gap") mistook a line from
x=−965 to x=+664 as 419 px — short, so drawn — instead of the true 1629 px, which should be
discarded; in the Weretiger burst this let screen-wide foreign lines appear straight through every
frame, where the ground-truth video shows only radial rays. With the direct rule all 41 Weretiger
frames are pixel-identical to the previously verified state — the s16-only version had been
accidentally right because invalid prims fell outside the raster window anyway. `primSpanOk` and
`screenWindow` in `run-effect.ts` implement both rules and are shared by every extractor.

**Unwrap is separate from visibility.** Display still needs it even though it is irrelevant to
hardware visibility: the interpreter computes in a space shifted by an unknown amount, so parts of
one object can land in different 2048-unit steps (this tore the Accession dome into a trapezoid).
`screenWindow(ticks, w, h).unwrap(p)` moves each prim's midpoint along the shortest path to the
window center and rigidly carries its corners along, preserving shape. Window size is 320×240
where clipping should occur at the screen edge (spell replays), or 1024×1024 where the whole
effect must fit in frame (Accession, compendium frame series — a 320×240 window there would
discard two-thirds of the prims).

**Screen dimensions are 320×240**, confirmed from the same GPU dump (clip rects `0,0,319,239` /
`0,240,319,479` = double buffer; real battle prims stay within x −162..541 / y −209..651, never
at ±1024). `build-spell-replay` searches for the cyclic 320×240 window containing the largest
prim mass (x/y treated separably), discards prims outside it, and clips the remainder with
Sutherland-Hodgman including color and UV interpolation (clipped textured quads carry corner UVs
relative to the atlas cell from field index 22; the player's cell-rectangle default applies only
when unclipped). The GPU's "edge length > 1023 → polygon discarded" rule applies as well. Without
clipping, Gouraud areas that end at the screen edge in battle instead stuck out over half the
world map (Sirocco wedges, tsunami waves); the player also fades the outer ~22% of the window in
the fragment shader (`replay.screen = [midX, midY, 160, 120]`) so the clip boundary is not a
visible rectangle.

The effect catalog grew from 119 to 134 entries: 15 effects have no own geometry and had been
silently discarded, though in the original they are SFX plus combatant animation (Quake, Aura,
Steal, Main Cannon, Purify, Howling, Bone Dance, and others); they run cleanly and deliver real
cue dispatches, so the exporter now only skips effects with neither geometry nor SFX.

**Bit depth (4bpp/8bpp) is a per-primitive property** carried in the tpage mode, not a per-sheet
property: some effects read the same ct3 sheet as tpage `0x3d` (4bpp) in some prims and `0xbd`
(8bpp) in others. A single `detectBpp` call got half of such prims wrong, indexing into palette
entry 0 and producing a black atlas. Fixed by splitting `unswizzleHw` (VRAM width, a sub-level
property from `detectBpp`) from `idxFrom` (bit depth, a prim-level property read from tpage).

**CLUT staging address maps to a VRAM row:** the staging area `0x80033000-0x80037000` is
organized row-wise in 512-byte blocks (256 color entries = the first 256 px of one VRAM row):
`0x80033c00 ↔ row 482`, `0x80036c00 ↔ row 506`, generally `row = 482 + (ramAddr −
0x80033c00)/512`. Effects using only the `0x36c00` block draw exclusively from row 506
(MAGIC012/046/077/093); effects using both blocks draw from row 482 (+506). `effectClut` had
taken the block with the largest file offset unconditionally, picking the wrong palette for
multi-CLUT effects — Tsunami rendered as a grayscale ramp instead of its water colors and was
effectively invisible; Chill and Fire Whip were mis-colored. Fixed via
`clutByRow.get(clutVal >> 6)`.

**ct3 `type[4]` is a VRAM target-region descriptor**, not a RAM pointer (unlike ct0/CLUT subs):

```
x (halfwords) = (d >>> 25) * 64
y             = ((d >>> 16) & 0x1ff) * 32
width (Hw)    = (d & 0xffff) / 8
height        = size / 2 / width
```

The most common value `0x1a080400` decodes to (832, 256) — exactly the previously hardcoded
sheet base `SHEET_X/SHEET_Y` — and height stays ≤256 rows for all six occurring descriptors
(`0x1a080200` ×38, `0x1a080400` ×29, `0x1c080200` ×18, `0x1a0c0200` ×7, `0x1a0e0400`,
`0x1a0c0400`), matching the VRAM space available from y=256. With the previously guessed width
(`detectBpp` → BPS·32), Geo Breath and Meteor Strike fell apart into offset 32-px blocks; with the
descriptor width they form coherent shapes (boulders).

ct3 regions can overlap: for Geo Breath a 64-KB sub covers y 256-512 while an 8-KB sub covers y
384-448, and the prims read at y=384. Sheet selection must search from the back of the sub list
since the later sub wins on load (`sheetFor` in `build-spell-replay.ts`). This fixed the
remaining black atlases (12 → 2 empty; the two left are only thinly populated but with correct
content — Blitz: 2 sprites, Resurrect: the angel), and corrected Tsunami, Chill, Fire Whip, Geo
Breath, Meteor Strike, Lavaburst, Combustion, Firebreath, RottenBreath, Tornado.

#### Scaling and anchoring

Effect scale is a fixed calibration of 1.4 tiles / 48 px (battle combatant ~48 px maps to field
character 1.4 tiles), replacing an earlier 5-tile normalization (`sizeTiles/halfW`) that inflated
small effects by 5× or more (halfW had ranged 24..12501 before the vertex fix). This keeps small
effects small and full-screen effects big, preserving relative original sizes. The anchor point
for world casts is 2.5 tiles ahead of the character in its facing direction (`spellTargetPos` in
`main.ts`, ground height via `grid.sampleY`), not the caster position.

Known limitations: effects whose graphic hangs off the combatant sprite (caster animations) draw
almost nothing in the replay — e.g. MAGIC004/005/008 yield only 6 prims each, since the sword
strikes happen on the actor sprite, not in the effect stream. The 1500-tick budget still caps
endless loopers such as MAGIC012 "Lightning". Gate-dependent extra cues that never fire in the
captured context are missing from the replay (e.g. the Weretiger growl). Screen-space windows
such as Accession/Weretiger encode "3D distribution" only as projected offsets; true world-space
distribution would require tracking entity slot position (`+0x34/38/3c`) inside the interpreter.

#### Sound cues

Bank-1 sound handler `0x8015f260` (dispatch table `@0x801827f8[bank]`) reads, per cue sample k,
the RAM descriptor `0x8014871c + k·4`, filled at spell load from the ct8 sub of the MAGIC EMI
(12-16 B = 3-4 cue entries of 4 B each): `[?, prog|0x80, tone<<4|lo, noteByte]` indexes
`ToneAtr @ VAB 0x820 + prog·512 + tone·32` (offset tables `@0x801826ac = prog·512`,
`@0x801826b4 = tone·32`); `+0x16` of that entry gives the VAG index → sample file `s{vag-1}`.
Tone zones are single-note zones (min==max), so per-VAG extracted WAV playback rates are already
correct with no pitch factor needed.

Example resolutions: Sirocco cue `0x102` → tone 2 → VAG1 = s00 (0.78-s boom, first eight-fold
wave); cues `0x100`/`0x101` → tone 0 → VAG2 = s01 (1.3-s wind howl, follow-up waves on a 16-tick
grid). MAGIC064 (Weretiger): cue `0x100` → VAG2 = s01 (27-ms click); cue `0x101` at the burst →
VAG3 = s02, the 5.59-s call — an earlier reading of "0x101 = s01 click" was an index mix-up; the
gate-dependent growl call site (s00 via cue `0x102`) never fires in the captured context, so its
absence from the replay is expected rather than a bug.

Sound-cue placement is authoritative from the ct0 trace itself, not from spectral analysis of
emulator recordings (which also capture unrelated voices and other banks and is not sufficient
alone for cue assignment); `public/spellfx/<TAG>/replay.json → sounds[]` is the reference to
check.

### Accession — dome size, sprite sheets, and the transformation sequence

Effect `MAGIC151` drives Ryu's Accession transformation, choreographed through the same static ct0
interpreter as every other spell effect and layered over the field-sprite sequence below. A
corrected reference video is rebuilt via `extract/build-accession-video.ts` for comparison against
the sprite sheets.

**Dome size.** The dome's world scale comes from the extractor's own stored frame width, not a
hand-picked constant. `public/accession-transform/index.json` records `domeWidthPx = 249.5`;
`domeWorldTiles = domeWidthPx × FIELD_SCALE ÷ PXT ≈ 7.64 tiles` — the same scale
`buildDragonWorldSprites` uses to place the dragon forms, since both are battle frames from the
same rasterization. This keeps the dome-to-dragon ratio correct for every form: original
`249.5:152` px matches the world ratio for the largest dragon, Behemoth/`DRG06` (152 px ≈ 4.65
tiles, vs. the dome's 7.64). `play()` now reads the value from the frame metadata without an
argument.
Superseded: an earlier fix set a fixed `DOME_TILES = 1.4438 × 2.22 ≈ 3.2` from a mismeasured dome
frame (99.96 × 73 px, against Ryu's battle sprite `BPLU012` at 45 × 51 px) — too narrow for large
dragons, which then visibly stuck out past the dome edge.

**Sprite sheets.** Child caster `CRYUD00`: `p25` raises the sword, thrusts it into the ground, and
crouches behind it (`f10` = hold loop, `loopStart 10`); `p19` is the held aura — 29 frames of the
same crouch pose cycling blue flame → green ray outline → gold halo, `f28` transitioning into the
cocoon; `p27` covers the dissolve from `f02` onward, a dragon-silhouette color cycle (white →
purple → gold → orange → cyan → blue → pull-up stroke → point) read as the dragon "being
released," with `f00`/`f01` as separate glow-ball states (gold/white); `p29`/`p30` hold orb/ram
variants. Adult caster `RYUD00` mirrors this on `p12`-`p17`: `p13` is the ram, `p14` the 43-frame
crouch+flame-aura+dissolve chain, now played in full across `f04..f38` (`loopStart 42`, ends
empty). Glow and aura are baked into the frames — no runtime tint.
Superseded: the aura was first read from `p27` `f00`/`f01` plus `p25` `f10`, and the adult chain
used three hand-picked `p14` frames instead of the full range — both froze the pose instead of
holding it to the scream.

`buildAccessionCastSets()` (`accession.ts`) builds intro/aura/dissolve `PlayerSprites` for both
ages. All cast frames now share `FIELD_SCALE` with the dragon and dome sprites; cast Ryu standing
(64 px × 0.7 ÷ `PXT` = 1.96 tiles) now matches field Ryu (42 px ÷ `PXT` = 1.84 tiles).
Superseded: cast sets were previously normalized per phase to the current field sprite height
individually, which inflated the low crouch pose toward standing height and made the figure
visibly jump at each phase switch.

**Timed sequence.** Frame-sheet timing (0.1 s resolution) plus a Y-motion curve and spectral
correlation against the reference recording fix the full sequence:

| Phase | Duration | Notes |
|---|---|---|
| Sword ram | ~0.3 s | |
| Aura hold | 4.3 s child / ~2.4 s adult | measured from the Kaiser-form onset; screams at +0.2 s and +1.3 s; form banner just before the phase ends |
| Dissolve | ~0.7 s | |
| Column + ring | 0.47 s | |
| Dome growing | 0.7 s | |
| Bolts | 2.8 s | |
| Fade + trail | 1.1 s | |
| Form cry | — | fires at dome-gone + 0.55 s |
| Total | 21.7 s | full reference sequence |

Stretch factors, recalibrated against this timeline: frame index `<28 → ×2.35`, `<46 → ×1.95`,
`<94 → ×2.9`, fade `→ ×2.5`.
Superseded: a flat fade stretch of `×1.0` made the dragon form visibly "pop" into view instead of
growing.

`runAccession` (`main.ts`) plays the full sequence: cast frames → charge + silhouettes → sound
cues → click grid from `boltStartMs` to `swapMs` → sprite swap → form cry at swap + 0.55 s. The
reverse transformation (caster is already the dragon, no cast sprites) keeps a short tint aura
instead of the full cast chain.

**Dome lag.** Two causes fixed: 98 frame textures were only GPU-uploaded on first bind, so
`ensureLoaded(renderer)` now calls `renderer.initTexture()` to preupload them; and
`material.needsUpdate` had been set on every frame change, now set only once at the initial
assignment inside `play()` (`USE_MAP` define), with `update()` only swapping the `map` reference.
A requestAnimationFrame measurement across the dome phase showed 299 frames with a single
outlier, at the sprite swap.

### Dragon forms and the animation module

`build-dragon-anims.ts` parses dragon programs the same way as party sprites (`readProgramU8`):
idle is dispatch group 0 (`hf0` preferred), and every remaining program becomes a selectable
action series. This replaced an earlier version that emitted only one IoU-clustered idle series
per form and dropped action programs entirely — `DRG00` idles had collapsed to a single frame.

Two parser caps, shared with the party pipeline, had been silently rejecting dragon records:
`party-plausible()` capped geometry at 160 px while dragon records reach ~230 px, and
`party-readGeom` capped vertex count at `vc>48` while `DRG00` records use more than 48 cells — the
same trap that had affected Chimera and FoulWeed, making `readGeom` return zero vertices for every
record. With both caps raised, all 75 dragon forms carry an animated idle, across 825 action
series and 10,299 frames (up from 2,259).

The compendium dragon module shows 9 base form cards (`DRG00`-`DRG08`) with an element selector
(`_00`..`_04` = Flame/Frost/Thunder/Shadow/Radiance), special-form cards for Ryu D/U (×8), Whelp
(`CRYU`), Rei/Weretiger, and Peco (×6) with all their variants, plus a Kaiser card animated
directly from the `KAIZAR` BMAGIC assets (3 programs). Every card exposes playback buttons
(play-once, falling back to idle).

Sprite-sheet identification, corrected: `RYUD10`-`13`/`RYUU10`-`13` are Kaiser (Ryu with golden
hair), distinct from the blue-haired battle series `RYU?0x`. `REI?`-prefixed files are the
Warrior form (a blond, muscular, tailed figure), not Rei. `RT?` is confirmed Weretiger. `PAPY` is
the Whelp — a baby dragon with 5 element genes, not a "Peco form" as first labeled. `CRYU` is the
child-Ryu caster. All were correctly extracted but mislabeled inside a combined "Ryu ×8" module.
The Kaiser card now stands alone (8 variants); other form cards carry visually curated short names
(quadruped/fish/lizard/bird/sea-serpent/toad/kraken/hybrid/armored-dragon) while keeping the `DRG`
code visible.

### Skill-to-spell effect mapping

Engine skill ids map directly to `MAGIC` file numbers: `MAGIC{n}.EMI` is the effect for engine
skill id `n` (Flare, id 91 → `MAGIC091`; Heal, id 70 → `MAGIC070`; MeteorStrike, id 226 →
`MAGIC226`). This closes the earlier gap where the ct0 header id was assumed unrelated to the
skill id.

The lookup chain, from the battle overlay disassembly (`0x800ae1f8`): `skillId` → u8 table
`@0x800b6510` (`BATTLE.EMI` overlay `@0x80096800`) = effect number → 8-byte descriptor
`@0x800b65f8` (`u16[0]` = disc file id, `0xffff` = resident) → EXE file table `@0x80182910` (`u32`
LBA per id) → file name. The table indirection exists for two cases: taught copies (`type&3==3`,
sharing the original's effect number — 40 of 41 duplicate pairs share an identical effect number,
the fingerprint that drove the search) and 4 resident skills with no dedicated EMI file. Confirmed
by constraint scan (duplicate pairs must match, distinct spells must differ), then verified
against the disassembly.

`build-bmagic-anim.ts` now writes a `skills[]` list per effect (137 of 144 files named) plus
`_residentSkills`; the compendium spell module shows real names, lists shared templates honestly
as multi-entry (`MAGIC008` = the status-hit effect for Gambit, Blind, Devour, and others), and
exposes every program per effect. The EXE file table `@0x80182910` (`u32` LBA, indexed by global
file id) is documented as a reusable byproduct for any future "which file does the code load"
question.
Superseded: the bmagic frame parser had used an empirical `(ns, nr)` search that matched all 40
animated files but read jump commands as frames, hanging up to 128 ticks in 17 effects; replaced
by the correct `[nRecs][nSteps]` + jump format, now producing 969 clean frames.

### Spell VFX extraction

`extract/build-bmagic.ts` extracts every `/BIN/BMAGIC/MAGIC###.EMI` (144 spells plus 3 `KAIZAR`
files) to `public/bmagic/`: 97 graphics PNGs, a `sound/` folder with 221 WAVs, and `index.json`.
Each file carries up to four content types: `ct0` `@0x801eec00` is the effect choreography —
compiled MIPS, not a byte VM (`[u16 id][u16 flags]` plus a phase-pointer directory for 108 of 141
dispatching effects, or direct code for 33); the dispatcher reads `ctx[1]` as phase, jumps through
`directory[phase]`, calls engine API functions, and manipulates entity structs `@0x80145e90`
(`ctx` = scratchpad `0x1f800044`, the same sprite interpreter used for mesh groups and `PLCHAR`).
`ct3` (present in 80 of 144 files) holds the effect graphics. `ct6`/`ct7`/`ct8` hold spell sound as
pBAV VAB data, the same format as SFX, yielding the 221 WAVs.

Graphics geometry: sheets are a fixed 256 px wide, macroblock-striped like AREA/enemy pages
(`reconstructTextureVram`, 32×32-hw blocks), but with `BPS = width_hw/32 = 2` for 4bpp or `4` for
8bpp — not the `BPS=16` that would wrongly impose a 32-row VRAM wrap. Sheet height follows data
volume (256×256 @4bpp = 32768 bytes, @8bpp = 65536). Bit depth is not statically flagged per
effect and is auto-detected via neighbor coherence, matching visual ground truth on 12 of 12
checked effects (63 sheets 4bpp, 34 sheets 8bpp). The spell CLUT is stored inside the EMI itself
(`ctype0` `@0x80033000`-`0x80037000`, mostly at `0x80036c00`, 256-color BGR555) rather than
runtime-bound like the enemy CLUT, so spell colors extract exactly (0 of 80 checked effects lack a
CLUT; 4bpp effects use palette 0). 17 EMIs carry two `ct3` blocks — an overlay plus the main
sheet. Visually verified against `MAGIC004` (ice), `MAGIC005` (fireballs), `MAGIC012` (summon),
`MAGIC088`/`MAGIC013` (status text, "DEF UP"/"MISS!"), and `KAIZAR_N` (golden emperor dragon). The
id in the ct0 header is the global skill id (`MAGIC004 = 0x147`, sequential and linkable to the
skill table).

Compendium coverage, corrected: `renderMagic` had hidden the 64 choreography-only effects because
they carry no sound, giving a false "spells incomplete" impression. All 144 are now shown, split
three ways: 40 fully animated, 40 procedural sheet effects (ct0 sets UV coordinates onto a static
sheet, badged "Sheet (procedural)"), and 64 choreography-only effects (badged as moving fighters
or camera with no dedicated effect graphic — e.g. Nue Stomp, Flying Kick, Counter — whose motion
lives in the party/enemy programs `p10`/`p14`/…).

### Sound effects

**SFX banks.** `extract/build-sfx.ts` writes `public/sfx/<bank>/sNN.wav` plus `manifest.json`. SFX
banks are pBAV VABs — the same format as BGM, minus the SEQ sequence, holding individual VAG
samples instead: `ct6` is the VAB header (VAG size table, located via `Σ·8==VB length`), `ct7` is
the VB body (concatenated PSX 4-bit ADPCM). Banks: `COMN_SE` (10 system/UI sounds, byte-identical
to `FIRST.EMI`'s `ct6`/`ct7`/`ct8`), `BATL_SE` (4 battle), `A085SE2` (13) and `A108SE2` (10,
per-area special SE) — 37 samples total. The ADPCM decoder is reused from `build-bgm.ts`
(bit-identical) and smoothness-verified (avg`|Δ|`/range 0.01-0.06 confirms audio content).

**Transformation cues.** The ct0 cue trace — `public/spellfx/<TAG>/replay.json` → `sounds[]`, read
by the static interpreter from the effect's actual cue ops — is the authoritative source for
transformation sound timing, above any spectral comparison against reference recordings:

| Effect | ct0 cues (authoritative) |
|---|---|
| `MAGIC151` (Accession) | `s00` @ tick 4 · `s01` @ tick 19 · `s02` @ tick 27 |
| `MAGIC064` (Weretiger) | `s01` @ tick 1 · `s02` @ tick 57 |

For Accession, `s00` is the rising "charge" staircase at the dissolve; `s01` fires once, and its
1.17 s sample already contains the audible click texture; `s02` follows at tick 27. No `s03` cue
exists for this effect. `AccessionFX.msAtTick(tick)` converts cue ticks to playback time so cues
land correctly despite the stretch lengthening of the frame timeline. For Weretiger, `s02` (a
7.7 s call) is scheduled via `setTimeout(…, 1120)` to land at tick 57 instead of firing
immediately. The growl sample `magic064_s00` stays in playback deliberately — audible in the
reference recording — but is flagged as Rei's voice, not a `MAGIC064` cue, since that effect's own
trace has no `s00`. Screams during Accession's aura use `ryu-kid` `s04`, played twice.

Superseded, both from spectral comparison against the reference recording rather than the ct0
trace: `s01` was read as a repeating 0.25 s click grid and placed in the bolt phase; the ct0 fires
it once at tick 19, and repeating the 1.17 s sample had stacked roughly ten overlapping copies
into audible mush (a separate note describing `s01` as "the 27 ms click" had conflated it with
`MAGIC064`'s `s01`, genuinely 0.04 s long). Ticks 19 and 27 were read as `s02` (at a "column"
moment, 16.85 s into the recording) and a nonexistent `s03` (+0.16 s); the ct0 trace reassigns
them to `s01`/`s02`, and an earlier build had triggered a spurious 2.57 s `s03` the effect never
fires. For Weretiger, `s02`'s call had played 1.12 s too early. ⚠ A spectrogram also captures
party voices and other banks and cannot substitute for the ct0 cue trace as a source of truth.

### Status effects

Disc side: status-inflicting skills carry two otherwise-unused bits in the skill record's element
word (`+0x12`) — `64` and `128` — a candidate encoding for the skill's status class. Observed
assignments: Blind (id 8, elem 128), Chlorine (id 9, elem 128), Molasses (id 30, elem 64), Tarbaby
(id 31, elem 64), Pollen (id 50, elem 128), Venom Breath (id 51), Death (id 106). The status state
machine itself — chance roll, per-fighter state bits, tick order — is not yet disassembled.

Browser side (`battle.ts`), marked as community mechanics rather than disc-read: `Combatant.status`
holds `{poison, sleep(turns), blind, slow}`. A trigger map, `STATUS_SKILLS`, assigns `8`/`9 →
blind`, `30`/`31 → slow`, `50 → sleep`, `51 → poison`, `106 → death`, applied on a 65% chance roll
(otherwise the target resists). Effects: blind cuts the attacker's hit rate to roughly 32%; sleep
lasts 2-3 turns with the turn skipped, and any physical hit wakes the target and connects for
certain; slow halves agility once; poison ticks `maxHP/16` per turn; Death is an instant KO on
success. `statusTick()` runs at the start of each side's turn; enemy roulette skills carrying a
status id now apply the state instead of fizzling as zero-power damage, and the party side applies
the same logic inside `doSpell` (for example Whelp's Blind, via the dragon moves). Status badges
render on the battle cards for poison, sleep, blind, and slow.

Smoke-tested against Ripper (`AREA003`ff), whose Blind roulette entry carries the full weight (8
of 8 eighths, i.e. guaranteed); 88 enemies in total carry status skills.

### Spell replay verification

Playback timing, measured via Playwright frame series against a static scene (NPCs, living-world
updates, and ambience disabled), runs tick-exact: `MAGIC104` Sirocco ends at 3.38 s (169 × 20 ms),
`MAGIC097` Jolt at 0.64 s, `MAGIC094` at 1.3 s, `MAGIC219` Magma Breath at 10.0 s. Sounds fire
tick-exact from `replay.sounds`, with cue sources disc-exact from the ct0 (code path verified).

Two measurement traps surfaced: PNG output size is not a usable activity proxy while NPCs are
still running, and a fixed measurement crop can miss effects that travel. `MAGIC126` ShadowBreath
appeared to end after 2.9 s under a fixed crop, but its particle beam (128 additive Gouraud puffs,
magenta `(255,32,255)` with `(2,2,2)` corners — the white-saturated core is additive-blend
physics, not a texture) actually travels across the full 500-tick duration and simply leaves the
cropped frame.

A capture pass toward real battle recordings used savestates `battlecap12.sav` and
`weretigercap`, driving the battle menu via synthetic hidkey input, with
`screencapture -v -V <s> -R <window rect>` restricted to the DuckStation window region (a
full-screen capture had twice recorded other private windows by mistake). Findings: the battle
command wheel ignores synthetic HID arrow keys, only `K`/confirm passes through — the reason the
Weretiger confirm hijack existed at all; and the `0x800974e8` confirm hijack falls back to a plain
"attack" for command ids `0x61` (Jolt) and `0x46` (Heal), even with skill-list cheats active
(`801461E0`/`80144C54`). The working Weretiger route (`0x40`) is a skill-specific special case, not
a general action injection. An arbitrary spell cast needs the confirm/validation path behind
`0x800974e8` disassembled, at the point where the command id is checked.

Validation status: (a) source is disc-exact, read directly from the ct0 interpreter; (b) the
browser player is tick-exact by direct measurement, confirmed on 5 class representatives against
the 20 ms grid with tick-exact sounds; (c) `MAGIC064` (Weretiger) has a real battle ground-truth
video that matches the replay analysis. A direct A/B video comparison for the remaining individual
spells is not done; effort against benefit is judged not to justify it at this scope.

⚠ Operating rules carried forward for any emulator/screencapture pass: confirm the target window is
free before capturing (check the frontmost application); capture only the target window region
(`-R`), never full-screen; deactivate cheats and remove the `.cht` file after each run, since
leftover cheats corrupt the next one; Playwright captures stay headless and never compete for the
foreground.

### Fade primitive

The resident screen-fade system (EXE `SLES_013.04`, loaded at `0x80096800`) is fully disassembled.
`0x8014ef18(mode)` (and its blocking variant `0x8014ef58`) sets a mode `u16` at `0x80143c90` and
spawns fade task `0x8014efd8` (spawner `0x8014b9a4`, busy flag `@0x80143c40`). The task copies a
21-entry mode table of function pointers, `@0x801499ac`, onto the stack and calls the selected
entry. Each mode stub calls the fade core with a fixed step and a color/buffer flag: modes 0/1
step `±0x800` (16 frames = 0.32 s); modes 2/3 step `±0x2000` (4 frames = 0.08 s, the flash class);
modes 4/5 step `±0x400` (32 frames = 0.64 s — the warp/`SCREEN_FX` fade, matching its ground-truth
measurement exactly); modes 6 and up repeat the same steps with `a2=2`, a second color/target
variant presumed to be white or another buffer.

The core, `0x8014f780(step, x, flags)`, runs a linear ramp: a darkness `u16` starts at 0 for
fade-out (positive step) or `0x7fff` for fade-in (negative step), moving by `step` per frame
(worker `0x8014f970`) until it hits `0x7fff` or `0`. The result feeds a multiplicative RGB
modulation triple at `0x80143d75`-`77` and `0x80143e05`-`07`, one per double buffer, zeroed once
the fade completes. The browser mirrors this with `WarpTransition.begin()` as a `640ms linear`
transition (`areabanner.ts`).
Superseded: the fade curve had been read as an "ease-in," roughly exponential ramp from reference
video; that impression was perceptual only — the disassembled core is strictly linear.

A related address was corrected: `0x8019fc28`-`a0` had been logged as a fade-mode routine, but the
scena-API signature — confidence rating V — holds instead: `WARP(area, x_q16, y_q16, dir)`. The
three stubs at `0x80195bc0` are fixed teleport targets — `[0]`/`[4] → area 108 (17,89)`,
`[1]`/`[2] → area 034 (52,67)`, `[3]`/`[5] → area 004 (62,5)` — all three destination tiles
verified walkable and textured. The script pattern `03 03 … 03 04` (cutscene start/end in area
`173`ff) reads as a two-stage station teleport: transit through area 4, then arrive at area 108 —
i.e. opcode `0x03 [param]` means "fade, then fixed warp to target `[param]`," with the fade itself
running through the primitive above.

### Refuted approaches

- **Dome scaled from the character sprite as a fixed tile constant.** `DOME_TILES = 1.4438 × 2.22
  ≈ 3.2` came from a mismeasured dome frame (99.96 × 73 px instead of the stored 249.5 px width)
  and made the dome too small for large dragons. Replaced by `domeWorldTiles`, derived from the
  stored frame width through the same `FIELD_SCALE` conversion the dragon sprites use.
- **Cast sprites normalized per phase to field sprite height.** Produced a visible jump at each
  phase switch, because the low crouch pose inflated toward standing height. Replaced by a single
  shared `FIELD_SCALE` across all cast frames.
- **Accession aura read from `CRYUD00` `p27` `f00`/`f01`.** Those frames are the separate glow-ball
  states, not the held aura, which is the dedicated 29-frame `p19` sheet; the adult chain likewise
  used three hand-picked `p14` frames instead of the full `f04..f38` range.
- **Spectral comparison against the reference recording as the source of truth for transformation
  sound cues.** It misassigned `MAGIC151` ticks 19/27 to `s02`/a nonexistent `s03`, read `s01` as a
  repeating 0.25 s grid instead of a single 1.17 s sample, and placed `MAGIC064`'s `s02` call
  1.12 s too early. A spectrogram also captures party voices and other banks, so it cannot
  substitute for the ct0 cue trace.
- **Empirical `(ns, nr)` search for the bmagic frame format.** Matched all 40 animated files but
  misread jump commands as frames, causing hangs up to 128 ticks in 17 effects. Replaced by the
  correct `[nRecs][nSteps]` + jump parser (969 clean frames).
- **`0x8014ef18` as the SFX/XA cue-dispatch entry point.** Once logged only as a "`REQ_ENQUEUE`
  candidate" for SFX triggering; the fade-primitive disassembly instead confirms it as the
  screen-fade mode API (`0x8014ef18(mode)`), unrelated to sound-cue dispatch.
- **`0x8019fc28`-`a0` logged as the fade-mode routine.** The address is actually
  `WARP(area, x_q16, y_q16, dir)`; fade and warp are separate systems dispatched together by
  opcode `0x03`.
- **Fade curve read as an "ease-in" exponential ramp from reference video.** The disassembled core
  (`0x8014f780`) is a strict linear ramp; the eased impression was perceptual.
- **Confirm-key hijack (`0x800974e8`) as a generic method to cast arbitrary battle spells for
  ground-truth capture.** The battle command wheel ignores synthetic HID arrow keys (only
  `K`/confirm passes through), and the hijack falls back to a plain "attack" for command ids `0x61`
  (Jolt) and `0x46` (Heal) even with skill-list cheats active. The working Weretiger route (`0x40`)
  is a skill-specific special case, not a general action injection.

### Open

- Status state machine (chance roll, per-fighter state bits, tick order) not disassembled;
  candidate sites are the code around `calcDamage` (`0x801dc044`ff) and the `STATUS.EMI` display
  paths.
- Direct A/B video comparison of individual spell replays against real battle recordings, beyond
  `MAGIC064`, not attempted; the battlecap capture pass was aborted when the target machine's
  foreground was no longer free for an interactive emulator session.
- Arbitrary battle-spell casting for ground-truth capture needs the confirm/validation path behind
  `0x800974e8` disassembled, to find where the command id is checked.
- Sheet-frame extraction for the 40 procedural spell effects requires ct0 interpretation; engine
  API anchor `0x801787c8` (sin/cos, trig table `@0x80185f98`, period `0x1000`) is mapped, but prim
  candidates `0x8017b780`, `0x80178894`, and `0x8017c7a4` still have open signatures.
- Per-sprite UV rects and per-sprite palette selection for spell sheets are runtime-bound, not
  present in the static sheet/CLUT; the bpp-detection heuristic is unverified beyond the 12 checked
  effects; full ct0 opcode disassembly (compiled MIPS) needs a battle RAM dump.
- SFX trigger mapping (which sample fires on which event — steps, doors, text blips, chests) is
  unresolved; candidate path is the SCENA API's SFX cue function, tentatively `0x0A` (a SOUND
  control code found in text) → SFX index, now that `0x8014ef18` is ruled out. SFX sample rate is a
  provisional flat 22050 Hz; the per-tone rate from each VAB's `centerNote` would be more exact.
- Fade mode-table entries 6 and up (`@0x801499ac`, `a2=2` variant) are read as a second
  color/target class, presumably white or another buffer, but unconfirmed against ground truth.
- Warp opcode `0x03`'s exact parameter-to-state-byte path (`0x801448ed`) is unconfirmed; an earlier
  note pointing `[0x03]` at `0x801a8214` may conflate the control-op table with the FX state table.
- Dragon `DRG` numbers are not mapped to in-game form names (needs a mid-transform RAM dump); the
  gene-set-to-`formId` arithmetic is unresolved; per-form command menus are runtime-built and not
  yet extracted (the attack animations themselves are now complete).
- 7 of 144 spell effects remain unnamed in `skills[]` (137 of 144 resolved).

