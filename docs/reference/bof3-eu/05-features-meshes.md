> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 5. Features, object meshes and decoration

### Feature-block dispatch

Every area EMI carries a feature block: a sequence of TYPE-tagged records drawn
by the field engine on top of terrain and walls (roofs, glows, rotating parts,
railings, triggers). Dispatch runs through a pointer table at `0x8017fb34`,
one function pointer per TYPE (index = TYPE), not a chain of compares. The
field handler's only universal skip rule is `size==1` (`lbu 0x2($s5)` +
`beq $s2==1` at `0x801562c4`/`0x801562cc`) — a 1-word record is a pure
reference/marker, never geometry. Any TYPE with no entry below always has
`size==1` in the full 200-area scan and is correctly skipped; this is not a
missing feature, it is the format's own no-op convention.

⚠ Earlier documentation read this as "the field handler skips `b0==1`
records". That was a misreading of the same skip check: `b0`/`b1` are not a
draw flag, they are the CONDITION word consumed by the visibility system (see
below). 15541 of 16500 TYPE-0 sub-feature records across all areas carry the
condition value `0x4001` (constant ON) — including all 128 of McNeil's, which
are simply XY-degenerate point quads, not hidden ones.

### TYPE catalogue

| TYPE | Records / areas | Handler | Meaning |
|---|---|---|---|
| `0x00` (TYPE-0, sub-feature) | — | shared with visibility system | Roof quads (`b0=1` family among others), gated per-record by the condition word, see "Roofs and the visibility-condition system" |
| `0x01`-`0x0e` | — | `0x801566a0` | Star objects |
| `0x04` | — | `0x80156ac0` | Star-object sub-path, individually confirmed |
| `0x0f` | 27 / 10 areas | `0x80156c2c` (151 instr.) | Animated riser — own handler, not part of the star-object family despite the adjacent TYPE range |
| `0x10` | — | `0x80156e8c` | Vertex/texture-word layout shared with `0x21` |
| `0x12`,`0x14`,`0x16`,`0x1a`,`0x1c` | 607 total | `0x80157074` (shared, 52 instr.) | Trigger family: pure logic, no GTE, no packet calls — never rendered, so never a browser gap |
| `0x21` | 698 quads / 57 areas | `0x801572f0` | Light/shadow Gouraud glow quads |
| `0x22` | 66 / 13 areas | `0x801575cc` | Rotating objects (windmill blades) |
| `0x23` | 670 / 93 areas | `0x801578b0` | No-op: handler body is one instruction (`jr $ra`) |
| `0x27` | — | `0x80157c1c` | Confirmed table entry, not otherwise detailed |
| `0x28`-`0x2b` | 0 (occur in no area of the full scan) | point at `0x801ef…`/`0x801f0…` | Per-area code overlay hooks, unused in practice |
| `0x3f`-`0x42` | — | literal texture words (`0xb1800007`,`0xb1810007`,`0xbd800006`,`0xbd810006`) | Panel table's first 4 slots are texture words, not code |
| `0x43`-`0x49` | — | own sub-handlers `0x80158428…0x80158bb4` | Panel family continues as real code from index 4 on |
| `0x47`-`0x4a` | 44, all `size==1` | e.g. `0x49`→`0x80158bb4` | No visible effect: handler exits immediately (`blez`) when `size−1<=0`, which is always true here |
| any other TYPE | `size==1` always | — | Correctly skipped, not a rendering gap |

The panel/railing family (`0x3f`-`0x4c`) is extracted today with `idx=(type−0x3f)>>1`
mapped blanket-style onto 4 `PANEL_DEF`s — reusing `0x41`/`0x42` also for
`0x43`-`0x46`. That yields the right railing CLUT (`208,483`) and matches
ground truth (McNeil fences), but is an approximation, not the disassembled
per-TYPE path; the sub-handlers above are the exact route, still unimplemented.

### Geometry record formats

**TYPE `0x21` (light/shadow glow), corrected layout:** `n = (size−1)/8` quads
of 8 words each = `[4 vertices][4 Gouraud vertex colors]`. The previous parser
read a fixed 2 quads and only one color, and excluded all 1-quad records via
`size>=13`.

Blend rule: the top byte of the first color is `0x80 | semiMode` — `0x81` =
additive (B+F), `0x82` = subtractive (B−F). Across 200 areas: `0x81`×469,
`0x82`×225, `0x00`×4 (the `0x00` flag, 4 quads in AREA077, all colors
`0x000000`, is not drawn at all — 1075 GT dumps show no untextured-opaque-black
class; meaning open).

**TYPE `0x22` (rotating objects), layout confirmed by handler disassembly at
`0x801575cc`:** 3 header words + `n` blocks of 6 words, `n = (size−3)/6`
(divides evenly for all 66 records: sizes 9/15/21/27/45/57 → n=1/2/3/4/7/9).
Block = `[phase word][4 vertices][texture word]`.

Vertex format (corrected from an earlier `{z:i16,y:i8,x:i8}` reading, disasm
`0x8015777c`-`0x801577f0`): one word packs 3×10-bit SIGNED fields —
`x = bits 20-29`, `y = bits 10-19`, `z = bits 0-9`, sign extended via the
`0x200` bit (`| −0x400`). The handler reads exactly 4 words per block
(`slti $a2,4`, `addiu $a1,4`) and writes 3× `sh` into an SVECTOR each.

Rotation: header word at block start `+8` carries three 10-bit fields
(bits 0-9 / 10-19 / 20-29), each multiplied by `(*0x80143e6c & 0xfff) << 2`
(a global timer). Each block's own phase word carries three more 10-bit
fields (bits 18-29 / 8-19 / 0-9), added to the three timer products and
masked to 12 bits: `angle = base phase + field × timer`, then rotation
matrix + RTPT. Header bytes: `lb 0x6/0x7($s3)` = yOff/xOff of the anchor,
`lw 0x4` = z component. Anchor: `X = (col−0x80)·128 + xOff·2` (byte 7),
`Y` analogous with yOff (byte 6), `Z = i16 @+4` → browser world:
`c = col + 0.5 + xOff/64`, `r = row + 0.5 + yOff/64`, `y = −z/128`.

**TYPE `0x0f` (animated riser), handler `0x80156c2c`:** uses the same global
timer `0x80143e6c` as the rotors. `Timer & 7` gives 8 phases; height `$s1`
decreases by 8 per phase (rises); a triangle wave from timer bits 8-12
(`0x10 − (t>>8)`, absolute value) adds pulsation. Anchor is `(col−1, row)`,
like the star objects. 4 vertices are written to scratchpad
`0x1f800018`/`0x20`/`0x28`/`0x30`. Occurrences include AREA000/007
(15,11)/(29,29)/(11,32), AREA002 (4×), AREA028, AREA049 (4×). Meaning open
(candidates: source bubbles, sparks, steam) — geometry is not yet built.

**Object mesh record (40-byte format)**, used by state objects and by the
`0x117000` object-mesh block alike: `typ` field per record selects the CLUT
column — the renderer writes `GetClut(typ·16, 483)` into the FT4 packet, so
the earlier "mysterious typ 0-15" is simply the CLUT column in row 483
(TPage fixed `(704,256)`). Packet buffer pointer at `0x8014598c`; transform
slots at `0x80147d08` (12 B) = `[entityPtr][sortKey=entity[+0x60]]`.

**Placement entry** (`struct[+8] + plcIdx·8`, 8 B): `[count u8][b1][b2][b3]`
+ `[meshPtr u32]`. The renderer reads `count` as signed `s8`
(`lbu`+sign-extend, `0x8015aff0`) and byte `[+3]`: `b3&0x40 → ABR=b3&3, else 2`
(`0x8015b0cc`-`f0`). `meshPtr` points into the `0x117000` block or is
subfile-internal (`0x801fxxxx`) — same 40-byte record format either way.

### Roofs and the visibility-condition system

Roof geometry is TYPE-0 with `b0=1`: sloped/hipped quads with a shingle atlas
(CLUT keys around w275/278). The overworld "black flat roofs" symptom
(AREA016/033/…) is not a texture bug: building tile tops reference filler
cells (page 0/4 `(0,0)` pal1) that hold `0xECEC` in real in-game VRAM (a033
savestate) — on the disc the filler is `0xEF`/`0xFF`, but in the original
there is genuinely no roof texture on the flat top; the dark tops are
correct. The real roofs are the separate TYPE-0/`b0=1` quads.

**Vertex → world formula (final, `roof-calib4.ts`):** a 3D-affine camera fit
over 424 ground quads (mapping UV cell ↔ map `texIdx`, iterative), with roof
corners unprojected at `wy = −z/128 = h/8`, gives
`world = (col + v.x/64, row + v.y/64)` — exactly the wall formula, no axis
swap, no anchor offset. Best fit: 0.18 tiles center error; pair matching
10/10 entries ↔ dump quads at 0.2-0.35 px residual. An earlier partial
calibration, fit over only the ~10-quad hut block, suggested an axis swap
(`v.x`=north-south, `v.y`=east-west) plus an anchor offset — that fit only
covered ~3.5 of the block's 12 tile² roof surface and was superseded once
the full 424-quad least-squares fit converged on the plain wall formula.

Height mapping: roof-z is in 128-unit steps on the 8-unit terrain scale
(trackbed z=−384 ↔ terrain h24; hut eave −368 ↔ wall top edge h23).

**Runtime completion, still open:** the engine draws 18 quads from only 10
manor placement entries — it completes the opposite sides/ridge caps at
runtime (candidate rule: per-slope `b2&1` mirror around the entry's ridge
edge). Without that completion, only the encoded partial surfaces render
(west slopes, N/S strips, eave overhangs); east mirrors and ridge caps are
missing. Separately, the "trio" duplex house's straw roofs have no roof
entries at all — that geometry comes from a further, still-unidentified
feature or object path.

**Visibility condition (final, code-confirmed):** the TYPE-0 sub-feature
halfword is `[b1<<8|b0]`, read as a condition, not a draw flag. Formula from
`0x80156248`/`0x80156260`: `andi a0,(cond>>5),0xf8` computes
`(b1&0x1f)·8`; `sltiu v0,v0,1` is a logical NOT applied when `b1` bit `0x20`
is set. `BITTEST` at `0x8015b848` reads byte `b0>>3`, bit `b0&7`, from bank
`0x80144e88`. `0x8015b868` is the matching flag-toggle setter (`xor 1<<bit`
+ `sb`) — the same routine object-anchor scripts use to set one-time flags.

| `b1` (cond class) | Evaluates to |
|---|---|
| `0x40` | Constant `b0&1` |
| `0x00`-`0x1f` / `0x20`-`0x3f` | `BITTEST(0x80144e88 + (b1&0x1f)·8, b0)`; bit `0x20` set on `b1` inverts the result ("visible while the flag is NOT set") |
| `0xfa` | `b0 == byte[0x80146870]` (story phase) |
| `0xfb` | Camera half-space: `w = ([0x801592dc]−0x200)&0xfff`; `w<0x801 ? !(b0&1) : (b0&1)` |
| `0xfc` | `(b0&1) XOR FSM-active([bb0]==4 \| [b90]==7)`; field-normal case is `b0&1==1` |
| `0xfd` | `b0 != [0x80143f03]` |
| `0xfe` | `b0 == full byte [0x8015933b]` (live value seen `0xa0`; semantics of the byte still open) |
| `0xff` | `[0x8015933c] ^ (b0&1)`, raw — practically always truthy |

Disc-wide `b1` histogram: `0x34`×797, `0x14`×758, `0xfe`×168, `0xfb`×158,
`0x02`×105, `0x22`×50, `0x2a`×32. There are no `0xfa` roof conditions
anywhere on the disc — roof visibility never hangs on story phase directly,
only on story flags; phase instead steers NPC placement/dialogue/SCENA
selection (confirmed by pixel-identical recons of McNeil at phase 1 vs. 12).

**Flag-poke ground truth method:** compute the flag-byte address from a cond
record (`0x80144e88 + (b1&0x1f)·8 + (b0>>3)`, bit `b0&7`), poke it with
`warp.ts --poke`, then dump. Proven three ways: (1) Sin-City folding bridge
(AREA021): cond `0x810` (deck, ×25 quads) and cond `0x2810` (high plate,
×4 quads) resolve to the SAME byte (`0x28&0x1f=8` → `0x80144eca` bit 0,
since `0x08&0x1f` is also 8) — one poke makes the deck appear AND the plate
disappear simultaneously, because `0x28`'s bit `0x20` marks it as the
inverted reading of the same bit. (2) AREA112 lever: poking `0x80144f2f=0x0f`
switches flat→raised, turning the `0x14xx` quads on and the `0x34xx` quads
off as a complementary pair (not a double lever; an earlier OR-combination
theory is dead). (3) A poke at `0x8014502f`, computed with `b1` WITHOUT the
`&0x1f` mask, had no effect across two independent runs — that address falls
outside the real flag bank, which is exactly why the mask matters. An
earlier "no inversion" finding (`f14≡f34`) had compared two accidentally
identical dumps rather than a real before/after; the corrected re-test
(`f34 ≡ reference`, not `f14`) confirmed inversion is real.

Browser implementation (`features.ts`): `condVisible` evaluates every class
above; by default (no flag bank applied) the scene shows the `0x2x`/`0x3x`
initial states (Sin-City plate up, AREA112 lever at rest); a "story states
(all flags on)" toggle (`condVisible`-`flagsAll`) shows the late-story state
(deck down, plate gone) — GT-grounded, since flag conditions are positive
tests. `build-features` exports `cond` for every record where `cond != 0x4001`;
the renderer draws `0x4001` records plus the `0xfb` camera case in standard
iso view (`b0&1==0`); the rest stay latent pending a section-yaw source for
`0xfb` in non-default views. The former `main.ts` gate that only drew
`b0=1` roofs in the overworld and 8 hand-picked `OBJ_AREAS` has been removed
entirely now that the real per-record condition is implemented.

A sweep of the highest-condition-count areas (105/106/026/128) found real
extra geometry gated by flags in AREA026 (McNeil Manor at night, 113
conditions, classes `0x02`+`0x22`): roof segments and fence runs
(recon 622×951 px vs. 545×715 px). AREA105's apparent diff was a false
positive — the water animation phase, not a flag state (lesson: mask water
zones before diffing dumps). AREA106/128 showed no hit inside the camera
window used (conditions sit outside the ~28-tile capture).

AREA112 also has a **runtime-drawn flat base** for its levers, present in no
export (no feature roofs, no object meshes at all in that area): source is
the runtime code class page `(704,256)`/CLUT `(224,483)`, 4bpp (colorMode 0).
The generic rgeo solver defaulted to `--bpp 8` and rasterized empty texels
(CLUT filled, texel window all zero) — colorMode must always be read from the
dump prims, never assumed. Fixed via `warp --dump`+`--save` in the same run →
`build-rgeo-scene-from-dump --state` (0.20 px DLT) → a 5-quad lever shape
placed on the four condition tiles (absolute anchor still only 2 map anchors,
open) → `build-runtime-geo` → lever quads in `features/area112.json`.

### State objects (object spawn system)

State objects are 40-byte mesh records driven by a small runtime object
system, distinct from the feature block. The object register occupies slots
30-33 of the large field-entity table at `0x80146888` (34 slots of `0x98`
bytes each); object slot base is `0x80147a58` (`= 0x80146888 + 30·0x98`).

Slot layout (same base pattern as the 116-series entities): `+1`=SUBTYPE,
`+6`=STATE, `+8`=scroll class, `+0x34`/`+0x38`=X/Z (16.16 fixed), `+0x3e`=
HEIGHT (ground lookup `0x801549f0` — not rotation; the 121-tent smoke
children carry `0x130`/`0x1b0`/`0x200`/`0x280` there as ascent heights;
per-section camera angles live at `0x801481e0` instead, since objects are
world-fixed), `+0x50`=meshPtr, `+0x54`=plcPtr, `+0x5c`=`[ABR mode, RGB 0x80³]`,
`+0x64`..`+0x6c`=translation offsets (matrix fn `0x8015b520`), `+0x74`=flags
(`0x10`=skip/invisible).

Frame walker `0x8015aa5c` iterates the 4 object slots: if active
(`byte0&1`), dispatch through vtable `0x801c865c[subtype]`:
- `[4]` → `0x801a1584`: MOVABLE object (mine carts, ferries; own movement VM
  via `struct[+0x10]`; always visible).
- `[6]` → `0x801a1c24`: STATIC state object — `if (state==8) visible; else
  slot[+0x74] |= 0x10`. This is the draw gate for latent objects.
- `[7]` → `0x801a3690`: special, always visible.

`state` is written via story events through the object script VM (op family
around `0x801aa868`, secondary table `0x80147cb8`, stride 16).

**Registration** happens statically in the per-area init script: EXE table
`0x8017fe40[area·4]` (the same table the NPC VM uses) points at a per-area
struct where `+0`=init script, `+8`=placement list. The script dispatcher
`0x801a4e78` reads the op byte's high nibble into jump table `0x80195b54`;
op lengths are fixed by table `0x801c86c8`:
`[17,16,19,18,17,18,16,17,12,13,14,14,2,4,7,-]`. `0xff` ends the script;
`0xf0`-`0xfe` are sub-streams (own table `0x80195a64`, parser format open).

High nibble `0xa`/`0xb` = OBJ_SPAWN (`0xa` allocates via a counter into
slots 0-29, `0xb` targets an explicit object-slot record; registrar
`0x801a6b40`/`0x801a6f18`), 14 bytes:
`[state+1<<4|class][xHalf][xTile][zHalf][zTile][subtype][?][mode][?][plcIdx][?][slotIdx][ffff]`.
High nibble `0x0` (17 B) is a normal (non-object) entity spawn,
`@0x801a4fd4`. AREA077's altar orb spawns as
`b5 01 73 01 0b 06 …` at `0x801f638c` = state 10 (latent) — direct proof
the orb stays invisible until a story event flips its state.

21 areas carry object spawns: 006, 009, 014, 017, 055, 057, 061, 068, 077,
104, 108, 117, 118, 121, 122, 123, 135, 145, 149, 186, 193. In four of them
(108, 135, 145, 121) the spawns sit inside `0xfN` sub-streams instead of the
main script, so those instances are recovered via the savestate scan
(116-series + 98-series slots) rather than static parsing.

Type-`0x5c` dispatch (116-series effect entities, SCENA-spawned — e.g. the
121 tent, the black ship) reads its callback set from EXE table `0x801c81e0`
(6 default pointers) or `0x801c81f8` (`[areaNum][6 ptrs]` override groups,
reader `0x8019a3ac`-`598`) — this table is the source of the earlier
"hard area==104 dispatch" observation. A hand-placed `0x5c` entity in an
area without a matching subfile function crashes (jumps into the middle of
unrelated function or data).

Yggdrasil's standing tree (AREA055, child phase) is an ordinary instance of
this system: a 90-record object mesh on page `(704,256)`/CLUT `483`,
record types 3 and 4, rendering as 174 runtime quads — not per-area code
geometry (see "Refuted approaches").

**Implementation:** `build-meshes.ts` statically parses the init script for
all 200 areas plus the 116-series/98-series savestate scan (count as `u8`,
palette per record `typ`) → `public/meshes/area<tag>.json`
(`{objects:[{x,z,state,subtyp,latent,records[]}]}`) + `_p<typ>.png`;
`render/meshes.ts` groups mesh instances per `(object,typ)`, with latent
ones in a separate `latent-objects` sub-group; a "state objects (story)"
toggle in the panel defaults on.

Residual slot state observed during this work: byte-identical object slots
seen in areas 121/067/173 were residuals from AREA014 (slots deactivate on
area change but are never cleared) — 067/173 have no init-script objects of
their own (the 067 bridge is feature-system geometry, not an object mesh);
their apparent trigger objects are event-only.

### Object anchors: interaction table, not mesh placement

The object-anchor table (init-script `struct[+0x2c]`, count−1 as byte
`struct[+0x31]`, 8-byte records `[tx][ty][b2][b3][param:u32]`,
`public/objanchors/`) is the field engine's LOOK-INTERACTION table —
examining furniture, signs, doors, search spots — not a furniture/mesh
placement list. Furniture graphics stay baked into map tiles and walls;
movable objects (train, ferries) run through the mover entities, not this
table.

**Lookup chain:** `0x801b5ed8` is the only consumer of `+0x2c`, reached via
`lhu area@0x80143f00 → 0x8017fe40[area]`. It walks the records backward and
tests the player's look tile (scratchpad ctx: tile `+0x36`/`+0x3a`,
direction `+0x8` → delta table `0x80181fcc`, `(dx,dy)` per direction 0-7:
0=NW, 1=N, 2=NE, 3=E, 4=SE, 5=S, 6=SW, 7=W) against the anchor row:
`b2&0x80` selects the axis (0 → X row `tx..tx+b3−1,ty`; 1 → Y row
`tx,ty..ty+b3−1`); `b3` = extent in tiles (2 or 3); `b2&0xf` = required look
direction (1=N, 7=W). Returns the matching record pointer or 0.

State entry `~0x801b6600` (the 3rd caller) turns the character to face the
required direction (`ctx[8]=dir`), then sets `ctx[1]=7` (examine state),
`ctx[2]=0`. Player-state dispatch is `0x801b03d0` (table `0x801cd330[ctx[1]]`);
sub-state dispatch `0x801b05c8` (table `0x801cd36c[ctx[2]]`): sub-states
14/15 are the two param handlers, entries `0x801cd3a4`/`0x801cd3a8`.

**`param` decode** (handler 1, text/script, `0x801b3060`; handler 2,
one-time find, `0x801b324c`; only the low 24 bits are used):

```
b2:=param>>16, lo:=param&0xffff, b0:=lo&0xff, b1:=lo>>8
b2==0xff OR !(lo&0x8000)  →  SEARCH SPOT (handler 1 sets GET_TEXT(0) "Anything here...?"
                               + textbox request [0x80143bb0]=2, ctx[2]+1 → handler 2):
    b0 = GLOBAL one-time flag bit (BIT_TEST/TOGGLE 0x8015b848/0x8015b868 on bank
         0x80145004, saved; 0xff = no flag/repeatable). Flag set → GET_TEXT(1)
         "Nothing!". Find counter [0x80145034]++; SFX cue 0x106.
    b2==0xff → ZENNY find, amount = b1·40: sprintf("%d" @0x801961bc) → substitution
         buffer 0x801490d8, GET_TEXT(5) "You got \x07", ADD_ZENNY 0x80166920
         (0x80144f50 += n, lifetime find zenny 0x80145030 += n, cap 9,999,999).
    otherwise → ITEM find: b2 = category, b1 = item ID. Category dispatch
         0x80165ffc (jump table @0x80149f28): 0=item @0x801c8be8·18 · 1=weapon
         @0x801c9360·24 · 2=armor @0x801c9b28·22 · 3=accessory @0x801ca100·20 ·
         4=keyitem — the same gamedata tables as items.json. Item name = first
         12 B of the record; GET_TEXT(2/3) "You found:\n\x07"; inventory add
         0x80165368 (slots @[0x801c8bc0/8bd4 + cls·4], stack cap 0x63; return
         0 = full → GET_TEXT(3)).
otherwise (lo bit15 set, b2!=0xff)  →  READABLE OBJECT (handler 1, ctx[2]+2 = the find
                               handler is skipped):
    b2==0 → system text GET_TEXT(lo&0xfff): bank @0x8001a000 = system-text.json
            tableA (0="Anything here...?" 1="Nothing!" 2/3="You found:\n" 4="Smells
            burnt..." 5="You got " 6="You see lots of books." 7="Locked!").
    b2==1 → area dialog text [lo&0xfff] via 0x80150490: u16 offset bank
            @0x80010000 = public/text/areaNNN.json.
    b2==2 → TALK SCRIPT [lo&0xfff] from the script-struct table +0x04 (not +0x14),
            launcher 0x801a41bc, VM context ptr [0x80146884]; return u16 ≠
            0xffff → area text(return). Only 6 anchors game-wide
            (041×2 / 100×2 / 112 / 171).
```

**`+0x04` talk scripts**, launcher `0x801a41bc`: byte loop, `op==0xff` ends,
`op<0xf0` dispatches by high nibble through `0x801a4e78` (handler table
`0x80195b54`, length table `0x801c86c8` — the same table the init-block
grammar uses), `op≥0xf0` runs through the `f` executor `0x801a4680`
(jump table `0x80195a64`). The 6 anchor scripts use:
- `f1 <termOp> <arg>` — IF: evaluate `termOp` via table `0x80182648[termOp&0x1f]`;
  false → block A (up to `fd`), true → block B (`fd..fe`); `fd`/`fe` are pure
  markers consumed by the skip scanners `0x801a4570`/`0x801a45f4`, never executed.
- `f9 <blk>` — selects flag bank `0x8014820c := blk` (launcher default: story-phase
  byte `0x80146870`; op `fa` also sets this default).
- `d0 00 <hh><ll>` (4 B) — return `ctx[0x7c] := BE16` (`0xffff`=no text), handler
  `0x801a7568`; the anchor handler follows with `GET_TEXT(area text[ctx&0xfff])`.
- `d0 08 80 <nn>` — ACTION trigger `ctx[0x7a] := nn`, copied by the anchor handler
  (`0x801b31b8`ff) into the player struct `[0x80146250]+0x11e` (`+0x120 :=` anchor
  param low16) = the field ACTION (picklock/portal); `hi-c` (2 B) sets `0x80143f4e`.

**Term-op catalog (25 handlers, `0x8015b890`ff, complete)** — the same table
also drives the init-block `f0`/`f1` conditions and the `f4` switch:

| op | Test |
|---|---|
| `0x00` | story phase `[0x80146870]==arg` |
| `0x01` | area `[0x80143f00]==arg` |
| `0x02` | `BIT_TEST(0x80144e88 + [0x8014820c]·8, arg)` (bank via `f9`, default = phase) |
| `0x03`-`0x06` | story byte `[0x80146864+op−3]==arg` |
| `0x07` | `[0x80146874]==arg` |
| `0x08` | `[0x80143f03]==arg` (entry byte next to the area cell) |
| `0x09` | `[0x80146871]&1 == (arg==1)` |
| `0x0a` | null slot |
| `0x0b` | player FIGURE `(entity0+0x79 @0x80145f09)==arg` — `arg=4` (Rei) gates the picklock doors |
| `0x0c` | `BIT_TEST(0x80144f28, arg)` — flag bank block `0x14` fixed |
| `0x0d` | list `@0x8014544c` (32 B) contains `arg` (key-item class; checker `0x801663f4`) |
| `0x0e` | story phase `> arg` |
| `0x0f` | `PartyRecord[arg]+0x07` bit0`==0` (stride `0xa4` from `0x80144968`; member status) |
| `0x10` | `BIT_TEST(0x80145548, arg)` = dragon gene `arg` owned (same bitmask as the master gates) |
| `0x11`-`0x13` | SET/WAIT on the story bytes (2-B ops, `(op&0xc)>>2` selects the byte; `0x12` returns −1 = "wait") |
| `0x14` | NOP (0) |
| `0x15`-`0x18` | complex ctx-based checks (multi-byte, scratchpad `ctx+0x4ff`; unused in the anchor context) |

The 6 talk-script anchors: 041[18/19] + 100[1] + 171[0] are Rei-only doors
("It's locked..." / Rei → action `0x3b`/`0x3c`/`0x10`/`0x37`); 100[0] + 112[0]
are the portal-drive stations (relay points B/A: flag not set →
"machine..."/"antenna error", flag set → portal menu text). GT: AREA171
text 5 = "It's locked...\nMaybe Rei could\npick it..." confirmed verbatim.

GT-confirmed anchor content: `"...locked!"` (AREA007); McNeil finds Molotov /
Healing Herb / Antidote at tiles `(96,47)`/`(87,15)`/`(61,2)`; Wyndia Silver
Knife + 1000-zenny drawer (AREA028); fishing bait Worm (accessory 28,
AREA005/010/011). The one-time-flag bit index runs globally consecutive
across areas (1..104 over 214 anchors) — area phase copies (000/007,
005/010/011) share the same flag bits and therefore the same find.

Class distribution over 214 anchors / 32 areas: 97 system text, 58 item
find, 38 area text, 15 zenny, 6 script.

Stale-pointer note: apparent "pointers to the anchor table" seen at
`0x8017fe`xx/`0x801c8`xxx are residual per-area entries of the 200-entry EXE
tables (every area loads its overlay to the same addresses) — only the
overlay directory and struct fields `+0x24`/`+0x28` (warp-condition lists)
and `+0x2c`/`+0x31` (the anchor table itself) matter.

**Browser implementation:** `extract/build-objinspect.ts` composes
objanchors + system-text tableA + `items.json` + `text/areaNNN.json` into
`public/objinspect.json` (resolved text, script anchors carry a decoded
`script` field: condition character/flag + whenFalse/whenTrue text; the Rei
condition is evaluated against the current world figure, story flags default
false; anchor examine runs in the enter handler before the zone warp, a
second enter triggers the warp). `src/systems/inspect.ts` looks up the tile
via `lastStep`, tracks one-time flags in `localStorage['bof3.inspect.flags']`
(counterpart to save bank `0x80145004`); `main.ts` binds Enter in front of
the object (checked before the search-spot channel `0xf0`/`f1`/`f4`),
opens the textbox, and plays find SFX `/sfx/COMN_SE/s06.wav` (cue `0x106` =
bank 1 / sample 6). The debug pins in `render/objanchors.ts` ("object
anchor" toggle) are unchanged and remain a diagnostic overlay only.

### Runtime geometry overlay (per-area code, not feature or mesh data)

A minority of visible geometry is drawn by per-area code overlays outside
both the feature block and the object-mesh system entirely — bridges,
cranes, tower fittings, crystal skirts. This geometry cannot be parsed
statically; it is recovered by lifting quads out of GT GPU dumps and
calibrating their screen position back to world space.

**Calibration method.** Dump prim XY carry frame draw offsets (GP0 `0xE5`);
the assumption "(176,120) = warp tile" does not hold in general (off by
~12 tiles in one case) — anchors must be absolute, not screen-center
relative. The PSX camera is genuinely perspective (ground tiles project
14→18 px across one window); affine fits plateau at 3-14 px residual. A
DLT 3×4 fit (`P·[c,r,h,1]`, planar-inverted exactly at fixed h, local
Jacobi axes during the BFS) reaches 0.18-0.23 px residual across all
windows and is the standard method (`extract/build-rgeo-scene-from-dump.ts`).

The more robust variant adds savestate calibration (`--state`): capture
`--dump` and `--save` at the same instant. The window grid at `0x8012c000`
(`+row·0x70+col·4`, cell = `[col][row][slotIdx]`) indexed against the slot
pool's FT4 packages at `0x8012d880` (`idx·80`) yields hundreds of exact
tile↔screen correspondences in one shot. Matching pool entries against dump
quads (by UV set + screen shape, ±3 px) gives the draw-offset delta AND
marks which quads belong to the ordinary map-tile system — everything left
over is per-area code overlay (this pool-based classification is automatic).
Class-foreign map quads resolve via BFS anchoring restricted to the export
target class; seeds can carry a per-quad page/CLUT/bpp triple where an area
mixes multiple GPU classes (AREA049).

Floating objects (no ground contact inside the captured window) cannot be
anchored this way — the BFS misplaces them (a crane jib drifted in AREA086;
a AREA049 crystal landed inside its tower in one window). The fix is either
choosing a window where the object touches the ground, or transplanting
already-verified quads of the same object with a measured `Δrow`/`Δh`.

**Per-area results:**

- **AREA086 (dockyard crane).** The class page `(576,256)`/CLUT `(0,484)`
  is ALSO the ordinary map class there (deck/hull/container, already
  rendered from the slot pool) — a full scene bake would duplicate half the
  area. The real overlay is 58 quads: the 3 yellow-black crane jibs +
  platforms (UVs u48-95/v192-255, exactly the slot-pool-excluded range),
  baked from dump `a086crane` (window 41,25). Browser matches GT.
- **AREA049 (watchtowers).** Tower CRYSTAL (page `320/256`, CLUT `(0,487)`),
  CANNON (page `704/256`, CLUT `(160,483)`), and consoles (CLUT `(224,483)`)
  are per-area code, baked on both towers (dumps `a049cv` window 45,25 +
  sweep s1-s5). Conveyor and smoke effects are ambient, not runtime geometry.
- **AREA198 (Station Myria).** The crystal skirt is 8 large facet quads
  (page `320/256`, CLUT `(0,484)`, anchored via the cube top edge of the
  CLUT-485 class), baked; silhouette matches GT.
- **AREA060 (checkpoint bridge sag) — a runtime height patch, not baked
  geometry.** RAM differs from the disc map block: per window roughly 20
  rows of 4 tiles (columns 45-48), corner heights ranging 32→39 in a
  ~20-row-period V profile, inconsistent between capture windows — the
  per-area code animates the bridge sag live, so a static partial bake
  would be wrong.

  The patch function is disassembled: writer in the AREA060 init overlay at
  `0x801f30b0` (south) and `0x801f3184` (north), target `0x80104030`.
  `base = PLAYER ROW` (high halfword of the 16.16 row at `0x8014930c`) — the
  dip travels with the runner, it does not simply swing. Columns 45-48
  (`a0=0x2d..0x30`); at `row = base±3` all four corners are 32
  (`sw 0x20202020`). Southward, `base+4+s` for `s=0..9`: `NW/NE=32+⌊s/2⌋`,
  `SW/SE=32+⌊(s+1)/2⌋` (up to 37). Northward, `base−4−s` for `s=0..13`:
  `NW/NE=32+⌊(s+1)/2⌋`, `SW/SE=32+⌊s/2⌋` (up to 39). The profile is
  edge-continuous; the earlier "inconsistency between windows" is hysteresis
  — rows not yet described by the patch keep their last written value. Disc
  heights are flat 32; the runtime patch only raises the ends.

  Browser: `render/bridgesag.ts` implements an edge buffer with hysteresis
  and a base-jump sweep, shifting terrain top/wall and railing feature
  vertices inside the bridge window (columns 36..71) and writing
  `area.heights` so the player walks the dip; judge/DLT-camera mode keeps
  the savestate height patch directly. Playwright measurement: underfoot
  height 32, far end 38.5, dip travels with the player. The same hook also
  animates rope/post entities via a `0x24`-byte stride table at
  `0x801f3338` (`s5` from `*0x80149308 − 45<<16`) — those entities do not
  swing in the browser yet (railings currently follow the deck; polish item).

Tools: `extract/build-rgeo-scene-from-dump.ts` (`--state`/`--append`/`--bpp`/
`--keep`, self-test), `extract/build-runtime-geo.ts` (per-quad classes,
emits an empty scaffold for featureless areas). GT captures: `a086crane`
(w1-w7), `a060br` (b2-b5), `a049cv` (s1-s5), `a198cr`.

**AREA055 Yggdrasil — the standing tree is NOT runtime code.** See "Refuted
approaches": the child-phase giant tree was first read as per-area code
geometry (like the 060/067 bridge classes) because its GT dump page/CLUT
class `(448,256)`/`(0,485)` appeared unexplained. A state-calibrated re-solve
(savestate `mesh055.sav`, 508 pool matches at `Δ=(0,0)`) showed every target
quad in that class is ordinary map system geometry — the class is simply the
map meadow. The real Yggdrasil tree is the object mesh described under
"State objects": 90 records, page `(704,256)`/CLUT `483`, types 3+4,
174 runtime quads. The area's rgeo seed and cached `rg` keys were deleted;
086/049/198 rgeo data are unaffected and remain correct.

### Decoration and the backdrop packet class

A separate class of small decorative geometry (trees, campfires, crystals,
tent smoke/firelight, floating domes) is generated procedurally at runtime
rather than read from any record table: full RAM scans (including rotated
byte orders) find zero matches for the dump's UV signatures anywhere as
record bytes. The generator's source format therefore stays unrecovered —
UV constants live directly in code. Decoration is instead captured with a
**grid bake recipe**: isolate the target class's quads from a GT dump and
rasterize them with a dedicated barycentric rasterizer (restricted to the
one target class, so no NPC/terrain contamination; additive classes
accumulate in float) into a PNG, then place it as a `DECO_PALETTE` billboard
in `public/deco/areaNNN.json`.

**Camp scene (AREA053/090).** 5 trees appear in the dump — 2 small ones
match the existing star-tree decoration, the 3 large ones are separated by
pixel-connected-component clustering into `camptreea`/`camptreeb`/`camptree1
.png` (a 12-quad pair originally overlapped) — plus a campfire column
(`FIRE_PLACES`). Positions are set visually against the GT recon because the
`pd053` camera matrix is unusable along the height axis (only 12 flat map
anchors give 12:1 anisotropy, which also makes this scene's judge/world
labels unreliable); `pd053`/`pd090` stay at ~90%/24% match as a camera-fit
residual, though visually close to GT.

**Chrysm crystal (AREA049).** The ×8 "different" quads in the dump are the
crystal itself: 8 S1 facet quads, page `320`/CLUT `487`, 8bpp — rasterized
additively into `chrysm049.png`. The deco system gained an `additive` flag
(`AdditiveBlending` material) for this. Placed on the plant pedestal at
`(46, 52.6)`.

**AREA058 tent effects.** The brick cooking shaft beside the tent canvas
produces two effects: (1) a smoke column, handled by the existing chimney
system (`CHIMNEY_SMOKE['058']` at `(33.5,34)`, matching the GT's C160
puffs — smoke animation phasing remains a residual judge-score item, as in
AREA002/102); (2) firelight, added as a new additive deco asset
`firelight.png` — a procedurally generated radial `#f8c800` gradient,
because the GT light is one untextured additive Gouraud quad from the
backdrop packet class (the area's 50 TYPE-`0x21` glows sit elsewhere and
are unrelated). This leaves only the AREA024 dome and the generator's own
code disassembly open from the backdrop class — the dome is resolved next.

**Stereo triangulation (floating geometry without ground contact).**
Domes, crowns, and elevated flames cannot be anchored from one view — depth
along the camera axis is ambiguous (tried and failed with single-view BFS
for a glass surface, flame anchors, and a vertical counter-test on the
AREA024 dome). Two views of the same scene resolve it:

1. `warp.ts <area> <x2> <y2> --state <pdN> --src <area> --dump <pdN>b --save <pdN>b`
   (target tile must be walkable — an unwalkable target produced a 258 KB
   mini dump, a "recon error").
2. `prim-detect.sh <pdN>b` yields a second calibrated frame (P2, ~0.2 px
   residual).
3. A stereo solver matches the class's quads across both frames by UV
   signature (CLUT + the 4 vertex UVs in strip order — 27/27 hits, 0
   ambiguous for the AREA024 dome), then solves 4 equations per vertex
   (2 views × s/t row, linear in `(c,r,h)`) via 3×3 normal-equations least
   squares, gated at <2 px reprojection error in both views, to produce an
   rgeo seed.

**AREA024 dome result:** 27 dome quads triangulated exactly (columns 97-102,
rows 10-15, h 0.7-4.2 — the tilted crystal facets); the crystal wall now
renders where it was previously a black hole. Match score `pd024` improved
35→32 (residual gap is blend nuance: GT paints the surface brighter/pastel,
likely multiple layers, plus partial faces from a `c64` class in another
frame). A third view (104,30) added zero new quads, confirming the set of
27 is complete (dedupe-verified).

**AREA011b flames, 7/7 stereo-exact.** UV-signature matching fails for
animated sprites, since flame cell UVs change between the two capture
instants (`pd011b` × new view `pd011c` at (10,16) — 7 present, 0 matched by
signature). For small counts, geometry assignment substitutes: triangulate
every N×M pair of foot midpoints, then assign greedily by reprojection
error — 7/7 resolved at 0.0-0.2 px. This also established that the solver's
`h` axis IS the browser's world `y` axis directly (`y = 1.0·h`, no further
scaling): lift = footY − sampleY. Flame world heights measured at 1.1-2.2
tiles. The resulting `BLAZE_PLACES['011']` (7 entries) replaced an earlier
photo-based calibration that was off by 1.5-2.5 tiles, a camera-estimation
error in that manual measurement. Judge score for this area stays at 88%,
limited by flame animation phase mismatch, not placement.

**Sky backdrop color.** `SKY_AREAS` (`main.ts`) had the sky's GP0 color
read with red and blue swapped: raw word `0xf8c800` was read as "golden
evening sky" `rgb(248,200,0)`; correct is `rgb(0,200,248)` sky blue (PSX
GP0 words are `0xBBGGRR`). `0x200000` likewise produced dark red instead of
night blue `rgb(0,0,32)`. Confirmed three ways: an existing emulator photo
already in the repo showed AREA082's sky as light-to-dark blue; the PSX
color-word specification; and content consistency (AREA011, "Treehouse,
BURNING", correctly needs a black→RED fire glow, not black→blue). A fresh
measurement from 693 GT dumps (one full-screen quad per dump) confirmed all
existing values and added 10 previously-black areas with no prior entry
(013/015/020/021/025/028/030/033/035/038 — all sky blue).

Two tool bugs in `render-gpudump.ts` caused this and a related miss, both
now fixed and commented in the tool: (a) GP0 command color was read as
`(color>>16)&0xff = R`; since the word is `0xBBGGRR`, R and B were swapped
in every recon — the root cause of both the window-backfill "light blue
instead of warm beige" and the backdrop "gold instead of sky blue" errors
(fix: `mr = p.color & 0xff`; textured prims barely change, since their
modulation is mostly gray). (b) The recon renderer draws ONLY textured
polygons — untextured prims (backdrops, TYPE-`0x21` glow overlays, opening
backfills) never appear in the recon image at all, staying transparent.
Anyone hunting an untextured class must read the prim list directly
(`audit-prims.ts` / `parseGpuDump`), never the recon image.

**Horizon bands (measured, not implemented).** AREA060 has 4 additive
Gouraud quads (semi=1), full screen width (0→320), top edge `0xffffff`
fading to bottom `0x000000`, height 17-22 px, bottom edge curving slightly
(86/83/81/83/86). Across 4 camera positions the coordinates are exactly
identical (the apparent y-difference to one capture is only the PSX's
240-line double-buffer offset) — this is a screen-fixed haze overlay, not
world geometry (90 textured prims intersect the band, so it is not simply
over empty sky). Not implemented: an additive full-width band risks
overexposing half the screen if the top-edge alpha is misread, the browser
camera (unlike the original) is movable, and no emulator image of AREA060
existed at the time. Once a single GT screenshot is available, implementation
is a simple screen-fixed overlay at ~27% height, ~8% tall.

**Opening backfill (doors and windows).** The flat-opaque fill quads seen
in front of door/window openings (`0x101010` for dark openings, `0x98b8d0`
for windows) do not come from the feature block at all — they come from the
map WALL renderer. Negative evidence ruling out other sources: the colors
appear nowhere as a data word or `lui`/`ori` immediate in `SLES_013.04` or
any AREA EMI; texel transparency is not the criterion (of 41 transparent
texture regions, 32 are not backfilled, and 2 backfilled regions have 0%
transparency) — it is an attribute of the wall instance, not the texture;
and wall-word bits do not distinguish backfilled walls either (a bit
analysis of 278 wall words from AREA000/007 found brightness/rotation/page/
palette bits overlapping completely between backfilled and non-backfilled,
even for neighboring tiles sharing page/palette).

A GT fingerprint table was built instead: 31 wall tiles get backfill (651
quads across 1075 dumps), 25 of them unambiguous; windows use page
`(576,256)`, CLUT `486`/`487`/`488`, UV columns 16/32/48 (v0-47); doors use
page `(320,256)`, CLUT `484`/`485`/`486`.

Fix (browser side, `build-walltex.ts`, function `backfillOpening`): fill the
opening as a TEXEL patch inside the walltex atlas — optically identical to
the quad behind it, with no z-fighting risk from added geometry. Two
conditions both required: (1) the tile matches the GT fingerprint table
(never guessed without evidence); (2) the transparent texels form a real
hole that does not touch the tile edge (this protects silhouettes — roof
slopes and gables rely on edge transparency to define their outline and
must not be filled). The edge test, validated across all 200 areas, finds
90 hole tiles including all 12 GT-proven hole tiles (the other 19 GT tiles
have 0% transparency, so backfill would be invisible there anyway). Result:
55 tile instances filled across 35 areas. The old hand-curated
`windowlights.json` beam entry for AREA000 is now redundant and removed;
AREA003 keeps its entry because that window is not a wall tile.

The edge test alone is not sufficient without GT evidence, confirmed by two
counter-examples: AREA118 (`0x1500122`, 43.8% transparent) is a dark frame
with a transparent center — a real opening; AREA126 (`0x170080f`, 37.1%) is
a diagonal rock/stair silhouette whose transparency happens to carry a 1px
opaque fringe — it passes the edge test but would be filled incorrectly if
it were. 78 hole tiles remain without GT evidence (e.g. page `(576,256)`
UV `64,0`-`79,39` in 11 areas) — the edge test finds them, but neither their
fill color nor their hole/silhouette status is decidable without a dump.
Also still open: why the engine backfills at all — no wall-word bit
predicts it.

The earlier hand-curated window-glow system (`buildWindowLights` in
`main.ts`, `public/windowlights.json`, 2 windows in 2 areas, built from
hand-measured approximations) has been replaced for its beam geometry —
see "Light/shadow overlays" — the beams were noticeably oversized
(overexposed white wedge); only the hand-curated pane core remains for now.

### Verification: ground-truth proof chains

**TYPE `0x21` (light/shadow glow), AREA000/McNeil inn (`mcneil_s13`, matched
in `a7_intse`).** GT shows 5 untextured polys: 3 flat/opaque `0x98b8d0`
quads coinciding exactly with the wall quads above them (pairs #16↔#22,
#27↔#31, #37↔#43 — the window pane is transparent in the wall texture,
page `576,256`/CLUT `0,488`, UV `[16..63,0..47]`, and the glow quad fills
it) plus 2 Gouraud/semi=1 quads with vertex colors
`505050,505050,000000,000000` — the light beam fading toward the floor.

Count proof, AREA140: extracted records yield `505050`×6 and `303030`×6;
GT dumps `probe140` and `a140` each show 12 of each — exactly 2 camera
positions × 6 quads, with colors and blend mode all matching. Both
directions of the blend rule are GT-proven: AREA000 (`0x81` flag) shows
semi=1 in the dump; AREA140/038 (`0x82` flag) show semi=2. Browser vs. GT
brightness diffs after implementation: AREA140 −10.5 (floor shadow strip),
AREA170 −56, AREA056 −8.5, AREA111 +66 (additive case).

This class had been discarded wholesale before
(`features.ts: if (w.key[0] === 'c') continue`, 438 dead `c<rgb>` wall keys
across 33 areas) because baked opaque it looked like a stray gray shape (one
report described it as a "sheared gray shape" at AREA000 `(55,68)`) — that
shape is exactly the window light beam. The same class also accounts for
what earlier notes labeled "F4" (112 deck glows) and "F5" (dimming clusters
in AREA140/189, meaning previously unclear) — F5 turned out to be cast-shadow
quads on floor and wall, same TYPE, same layout. Current export:
`features.glows`, 698 quads across 57 areas, with real per-quad blend mode.

**TYPE `0x22` windmills — identification proof.** AREA045: each of 6 wheels
is one quad, `(0,−63,63) (0,64,63) (0,−63,−64) (0,64,−64)` — a centered
127×127 blade in the yz-plane at x=0 — with rotation field `(18,0,0)` or
`(16,0,0)`, i.e. rotation about the X axis. AREA069: 4 quads of identical
shape with phases `0 / 256 / −512 / −256`; in the format's 10-bit angle
space (−512…511 = one full turn) that is exactly 0°/90°/180°/270° — the
four blades of one mill.

Cross-check against previously hand-measured decoration: AREA045's 6
one-quad records line up with the 6 existing hand-placed windmill deco
entries at columns 27/30/27/32/27/40 vs. 28.0/30.7/27.8/32.7/27.6/39.5 —
matching to within tile-center precision. AREA069's single 9-quad record
matches the area's one multi-part windmill (4 blades + hub + bolt). The
hand-measured entries' row offset (+1.9…+2.9) is the classic screen-space
measurement error for an object at mast height (height projects along the
row axis in this camera) — the record positions are correct, not the old
hand measurements. The two observed rotation-field values (16 vs. 18)
match the "two tempos" already noted from GT measurement of different mills.

Side result: AREA037 (town houses) has 4 records of 9 quads each
(`(50,24)`/`(14,34)`/`(29,38)`/`(28,52)`) with no matching mill decoration
in the browser at all — confirms the town houses do carry mills, settling
a previously open question. Also unaddressed in the browser: areas 001,
038, 066, 124, 127, 143, 145, 148, 198, 199 (full area list for TYPE
`0x22`: 001, 037, 038, 045, 066, 069, 124, 127, 143, 145, 148, 198, 199).

Open on TYPE `0x22`: the extra word per block (values `0xffffffc0`×90,
`0`×36, `0xffffffb0`×14, `0x40`×12 — plausible as signed 16-bit z values
−64, 0, −80, 64) has unclear meaning; no GT dump directly covers a windmill
record yet (existing dumps miss the positions, e.g. AREA148 has only 145
prims and its 12 CLUT-038 quads have no hit at `(33,43)`). This is not
implemented blind — geometry and texture read the same way as TYPE `0x10`/
`0x21` (`atlasForWord`), and the next step is concrete:
`warp.ts 1 52 38 --dump` (mine entrance) or `warp.ts 148 8 43`, then search
the dump for the resolved UVs to confirm before building.

### Map render window/slot architecture

Understanding the ordinary map-tile renderer was necessary to prove what is
and is not missing feature geometry. The map top-tile draw function is
`0x80153344` (prolog `addiu $sp,-0x78`), with exactly one caller
(`0x80152edc`) and one nested double loop — outer `$s6` over 55 rows
(`slti $s6,0x37`, back edge `0x80153ef4`), inner `$fp` over 28 columns
(`0x1c`, back edge `0x80153ed0`) — then `jr $ra` at `0x80153f30`. There is
no second sub-loop and no layer loop.

**Window grid:** base `0x8012c000`, cell address =
`0x8012c000 + row·0x70 + col·4`; each 4-byte cell is
`[worldCol:u8][worldRow:u8][slotIdx:u12|flags:u4]`. The grid is 28 columns
× 55 rows, ending at `0x8012d810`, directly before the slot buffer. World
X = `col·128 − 0x4040` (`0x801535c4`); window origin follows camera yaw
`0x801592dc`. A slot is allocated when `slotIdx==0` and the result is
cached into the record (`sh $v0,0x2($s0)` at `0x80153704`), so it persists
and is reused across frames (`0x801536cc`).

**Slot pool:** base `0x8012d880`, 80 bytes/slot, address =
`0x8012d880 + idx·80`. Slot fields: `+0x1e` = extra quad count (0-2),
`+0x46`/`+0x4e` = chained extra slot index for rect/pair tops. Allocator
`0x801547e0` has exactly 3 call sites, all inside `0x80153344`
(`0x801536f4` single top, `0x80153754`+`0x801537ac` rect/pair extra). Tile
renderer `0x80154508` has 2 call sites (`0x801536e4`/`0x801537c8`). Entry
decode `0x801557d4` has 12 call sites covering tops, walls, and features —
no unaccounted consumer exists.

### The "second floor layer" investigation: resolved as void, not missing geometry

A large fraction of walkable area (`walk≠0x10` and `tileTexIdx==0`) was
originally suspected to be a hidden second floor-rendering path, because
the game visibly shows terrain in some of these zones while the extractor's
`tileTexIdx` is raw-zero. GT capture across many areas resolved this fully:
**the game draws nothing (black or sky) at `tileTexIdx==0` tiles, exactly
like this reconstruction.** There is no hidden floor renderer.

The investigation initially found an apparent proof of hidden fill
geometry: 966 CLUT-484 FT4 packages sat in the slot buffer near a hole in
AREA082, of which 402 matched the ring cells surrounding the hole ("402
rapport" packages). Reading the slot-pool architecture above resolved this:
the shared slot buffer holds up to ~4000 slots; only entries the CURRENT
window grid references are live. For AREA082's capture, the buffer held
1047 FT4 records (max index 4008) but only 133 were referenced by the
grid — 914 were orphaned. Of the CLUT-484 (floor) slots, 489 existed, 133 in
the grid and 356 stale; CLUT `0`/`485`/`486`/`487` (walls/features) were
100% absent from the top grid (they use separate record structures). After
scrolling through a warp, masses of stale floor slots simply remain in the
shared buffer. The "402 rapport" packages were current band tiles plus
these stale residues, not evidence of a second renderer.

A second early misreading: the initial AREA082 capture point `(25,20)` sat
at the edge of a real tile band and looked INTO that band, so the "12 floor
quads under the character" were the adjacent real tiles (`idx≠0`), not the
hole. A direct capture at the hole's CENTER (`(12,10)`) instead showed the
character standing on blue sky gradient with zero floor quads — confirming
the huge AREA082 hole is intentional sky/void around the Angel Tower, not
missing geometry. A control capture on a real tile of the same area showed
the correct isometric tower structure against the same sky.

A seven-area sample (AREA082, 135, 170, 130, 187 = void; 147, 155 =
apparently real terrain) initially suggested most holes are void and a
minority are real missing terrain — but this sample was edge-biased: all
"real terrain" hits were at area corners, where the visible terrain
belonged to the neighboring real tile, not the hole itself. A same-area
control in AREA147 settled it: the corner tile `(13,2)` shows fortress/
sand/water, but the CENTRAL hole tile `(66,30)`, surrounded on all sides by
other holes, shows the character on pure black. **No confirmed area fills
an `idx==0` hole tile under the character with floor.** The black/void
patches are faithful to the original game, not a rendering gap.

**Walkability, not rendering, is the real fix — and only a scoped one.**
An unrestricted rule ("`!renderable && !featureFloor` → blocked", dropping
the `walk==0x40` restriction) was tested offline and found unsafe: it blocks
only 3.5% of walkable tiles worldwide but severs the traversal graph in many
areas — AREA150's main component drops 7671→1524 (−80%), AREA105 −61%,
AREA141 −64%, AREA028 −39% from only 16 blocked tiles, several more areas
−14…30%. These `idx==0` tiles are real traversal bridges the player is
meant to walk over; a blanket fence would break the game. Void-vs-bridge is
not separable offline (both are `idx==0`, non-roof) — only case-by-case GT
classification or a graph-preserving cut-vertex test could extend the fence
safely, and neither was implemented.

The fix actually shipped is narrower and specific to `walk=0x4_` tiles in
non-overworld areas: `grid.walkable` blocks a `0x4_` tile only when it is
neither renderable nor covered by a feature roof quad above it
(`buildFeatures` collects the bounding box of visible roof quads into
`group.userData.roofTiles`, consumed by `grid.setFeatureFloor`). This rule
is verified (AREA147 catwalk stays walkable, AREA120 dead zone is blocked,
spawns in 120/000/016 are unchanged) and remains in place; it is narrower
than — and unaffected by — the rejected blanket fence above. Consequently,
remaining work for the black-patch symptom is small and cosmetic: an
AREA082-style sky-color backdrop instead of flat black at tower/sky edges
(purely visual, not yet implemented for the general case; see the sky-color
fix above for the areas it already covers).

**Two extraction bugs found and fixed during the same audit:**
- **Pond flood-fill explosion.** `water[]` flood fill had no upper bound: a
  pond marker at page `5(5,10)` hit creek tiles and flooded to 6000 tiles
  (the whole map) in AREA005/010/011/012, producing a huge floating
  semi-transparent plane over the treehouse area (AREA003/008 correctly
  stayed at 38 tiles). Fixed with `WATER_CAP=300` in `buildAreaGeometry`:
  any flood exceeding 300 tiles is discarded (`console.warn`, no water
  drawn) rather than filling the whole map. 12 areas needed rebuilding:
  005/010/011/012 plus newly discovered explosions in 053/090/127/153/
  154/192 (each 43-98% of their map); AREA003/008 (38 tiles) and AREA067
  (242 tiles) were already correct and unaffected.
- **`skipfill` corrections only in `localStorage`.** `public/skipfill/` was
  empty, appearing as if the dev save server did not persist editor
  corrections. It does — `vite.config`'s `KINDS` list and `persist()` on
  every K-editor click already covered it — the directory was simply empty
  because no area had been corrected yet before this was checked; no data
  was lost. `__bof3.exportSkipFill()` remains the export path per corrected
  area.

**Side findings from the same audit:**
- 13 areas ship with no `features` JSON (004, 031, 086, 102, 103, 105, 107,
  138, 149, 159, 189, 190, 198). This is not a build gap: their feature
  blocks contain only non-exported record types (AREA086: type 23 plus
  type-0/`b0`=8/9/10) — `build-features` correctly writes nothing.
  `b0=8/9/10` semantics are still unknown.
- The "satellite dish" (relay points A/B) hangs off AREA112/AREA100; the
  AREA100 tent roofs are GT-proven missing via the roof condition system
  (see above), and AREA112's roof records are 73/73 present at hole tiles —
  the dish itself likely sits in a neighboring section of the same area
  class; exact location still open.
- AREA034's "dragon skull" block is a class-A-style hole with a gray
  fallback lid instead of black.
- AREA044 (Momo's upper area) has a small floor hole next to a staircase,
  presumably the same void class, unchecked in detail.
- Steel Beach sand (AREA075, 2126 `walk=0x40` holes) has its own area; a GT
  warp into it hit the ship instead of the sand — deck and crane render
  correctly, surroundings are black and still need a centered capture to
  classify.

### Other subsystems recorded in this range

The remainder of this range of the work log covers subsystems outside
features/meshes/decoration proper. Kept here in full for the record.

**Battle sprite rendering, ground-truth proof (bestiary).** Pixel/texel/quad
comparison against a battle GPU dump (ENEMY019/Ripper, AREA008) found texel
pages 640/704/768 100.00% byte-identical between the dump's VRAM and
`reconstructTextureVram`. CLUT row 496 is RGB-bit-exact; the battle runtime
only ever sets bit 15 (STP) on top of the stored colors (`0x6e4a`→`0xee4a`),
so an earlier "31/32 colors different" alarm was only that STP bit. All 34
runtime quads of one vsync frame correspond to the cells of a single anim
record (`rec@0x19de`) under one anchor — the figure is fully horizontally
mirrored at battle time (facing direction), not a multi-object composite;
17 of those cells are drawn twice, but pass 2 is `semi=true` over itself, a
visual no-op. Draw order is **last-wins**, not first-wins: comparison scored
96.22% color-exact for last-wins vs. 79.30% for first-wins, so "first cell
wins" in the original extractors was backwards. Fixed in
`build-enemy-anims.renderCells`, the party `renderOnto` (dragons inherit
it), `bmagic`, and `plchar-anims`; static/boss/plchar frames were already
correct. The 96.22% figure was later found not reproducible against
subsequent pipeline changes (~75-76% under either U formula) — last-wins
over first-wins remains correct, but the authoritative
benchmark since then is the strict UV-equality probe (`probe-usplit-gt.ts`),
not this percentage.

Sub-palettes at slot index ≥8 were previously flagged "color-uncertain":
the CLUT block at `0x80035800` actually carries up to 32 sub-palettes of 32
colors laid out LINEARLY (1536-2048 B); slot `s` reads row 496, column
`s·32`, for any `s` including ≥8. Proof: the AREA008-slot5 EyeGoo palette
reappears byte-identically in AREA009 slot 8 (`f2=0x88`) at block offset
`0x200 = 8·64`; AREA027 slot 12 at `0x300` is the canonical brown Roach
palette. `reconstructTextureVram` now stages this linear view after the
texture stripes (battle loads palettes over it at runtime; map-tile CLUT
rows 497/498 are untouched). This corrected Roach, Guard (previously
wrongly blacklisted as broken), Gonghead, the EyeGoo/MageGoo/PuffGoo
variants, ArmorBot/ProtoBot, and GntRoach — `BROKEN_FIGURES` shrank to just
`['worker']`, 15 named slot≥8 instances total.

Enemies without their own battle key (Miner, Bullies, …) show their host
area's unnamed orphan figures as a candidate gallery
(gate: coherence≥0.85, opacity≥250, smoothness≥0.6, top 4 by size) — 69
cards, graphics correct, only the name↔figure link missing. A full sweep of
375 figures confirmed Wraith/Spectre silhouettes are correct as canonical
purple/mint ghost flames and Bomber is a winged bomb, with no remaining
rainbow/fragment rendering defects.

Enemy animation: program steps from the container
(`prog0` = idle loop `[nSteps][nRecs]` + `steps(tick,recIdx)` + `recOffs` →
`[vc][cells]`) produce 2286 frames, animating 280 of 375 enemies (Ripper 12
wing-flap frames, EyeGoo 11, Goblin 8); a coherence gate demotes multi-part
figures to static. Tick unit is PAL vsync (dominant step ≈40 ms). Wired up
in the browser via a shared `SpriteAnim` class (one `rAF` loop; bestiary
view culls to ~6/100 visible sprites via `IntersectionObserver`; battle
view runs an idle loop plus `playOnce` for the attack series).

Party field/battle animation: 7 characters × 8 directions ×
(stand + 6-frame walk) = 436 frames, plus 14 battle views (idle + action
programs) = 1623 frames (`build-plchar-anims.ts`, `build-party-battle-anims.ts`).
Spell animation: the BMAGIC ct0 at `0x800c3800` is a sprite-anim program
table in the identical `[vc][cells]` format used by PLCHAR/enemies, read by
the same interpreter — 34 visibly animated spells (997 frames), 46
procedural sheet effects, 54 pure ct0-logic entries with no visual frames;
colors are exact (CLUT lives in the EMI); 221 SFX references are coupled
per effect, though frame→SFX timing is not encoded per-effect since
multiple spells share one ct0 template (the ct0 id is not a unique skill
ID — MAGIC004 and MAGIC005 share template `0x147`). Rei's Weretiger form
comes from the RTD/RTU EMIs (self-contained ct0): yellow-brown fur, red
mane, blue vest.

Remaining honest gap: truly multi-part sprites (DRG01 plus roughly 3
enemies — hoppers, L-shapes, robots) need a runtime-interned object list
(`objList` at `0x80182148`, consumer disasm `0x8014d82c`) that mixes party,
enemies, and effects at load time; the per-area enemy record at `0x800e4000`
only carries one primary key (`+0x16`), no secondary object field, so these
figures are not statically composable from disc data alone. Egg gangs and
flying insects are legitimately separate single sprites, not multi-part.

**Item records and menu icons.** Item stat layout: `stats[0]` = character
equip bitmask (bit = charId 0-6; confirmed via Dagger `0x19`=Ryu+Teepo+Rei,
Oaken Staff `0x02`=Nina, Rippers `0x40`=Peco, Flame Chrysm/Ammo `0x20`=Momo).
`stats[2]` = slot/subtype (1=weapon, 2=shield, 3=helmet, 4=body, 5=accessory,
`0x0a`=bait, `0x0b`=rod). Weapon: weight `+0x10`, attack `+0x12`, price
`+0x16`. Armor: weight `stats[3]`, defense `stats[4]`, price (u16)
`stats[8]`. Accessory: price (u16) `stats[6]`. Menu icons come from the
FIRST.EMI font sheet (`sub[3]`), 11 framed tiles near y≈449-510 (row A
`x=56+16k`: sword/staff/jug/eye; row C `x=8+16k`: blades/boots/book/doll/…),
colored via CLUT `0x80033a00+288`. Per-item icons do not exist in BoF3 —
item lists are text-only, confirmed against Prima Guide screens.
Accessory effects were confirmed to have NO separate effect byte: the 8
accessory stat bytes are fully interpreted (equip mask, rod flag, subtype,
weight, description-index u16, price u16); effects differ only in those
fields, so effects are wired by item ID in game code. Confirmed consumers:
AP-cost function `0x80166cf0` (skill AP from `0x801ca98e+skillId·20+0x10`;
Spirit Ring, id 26, `→ AP=ceil(AP/2)`, else Shaman's Ring id 25
`→ AP=ceil(AP·3/4)`, Spirit takes priority); weight summer `0x80165df0`
(Midas Stone = 10 weight, all other accessories 0); `countEquipped(itemId,cat)`
at `0x801665xx` (armor slots `+0xf..+0x11`, accessory slots `+0x12/+0x13`,
looped over 8 party members). Remaining per-effect consumers (elemental
rings, Ivory Charm, Soul Gem, Cupid's Lyre, Bell Collar/Holy Mantle,
Hawk's/Artemis hit chance, Coupons discount, Midas zenny) are individually
undisassembled.

**Audio streams and FMV.** Subheader scan of the 4 STR streams: VOICE.STR
has only 297 audio sectors (~32 s) across 5 channels (the rest of its
6.9 MB is padding — BoF3 has almost no voice acting). MAGIC00.STR is 16
mono tracks of ~98 s each (spell audio). S_XA00.STR is 8 stereo tracks
(coding `0x1`; channels 1 and 3 run ≈4.5 min of event music, the rest
3-41 s). CAPCOM30.STR is 1011 MDEC video sectors plus 143 XA stereo
sectors. `extract/build-xa.ts` extracts all of it (stereo decode: sound
units alternate L/R with their own predictor history; long tracks
transcoded to MP3 via ffmpeg) — 31 clips, 37.1 minutes total, into
`public/xa/{voice,magic,event,fmv}/`. `extract/build-fmv.ts` uses ffmpeg's
built-in psxstr demuxer and MDEC decoder directly on the raw 2352-byte
sectors → `public/fmv/capcom30.mp4` (320×240, 15.4 s, 231 frames + audio —
the Capcom logo animation; the in-game intro itself is rendered by the
engine, not FMV).

**Compendium and interactive systems.** `src/systems/compendium.ts`
(`initCompendium`, key G) is a DOM overlay browsing 9 extracted systems:
bestiary (168 enemies), party (7 characters), masters (17), spells (134
effects, playable VAB audio), fishing/fairies/dragons/items (gamedata
JSONs), music (81 BGM tracks). It pauses world hotkeys while open. A
"world features" toggle group controls NPC visibility, decoration
(`decoGroup`), ambience (smoke/fire/water), and teleporter markers (cyan
pillars on warp source tiles, off by default). Clicking an NPC sprite opens
its dialog (`dialog/scenaNN.json`) where extracted — only AREA000 has
extracted dialog initially (`AREA_SCENA 000→00`); slot↔spawn assignment is
by draw order, an approximation of the original's runtime binding. Four
systems became separately interactive, each its own DOM overlay:
`src/systems/master.ts` (apprentice/master pairing → exact stat and level
projection via the growth-writer logic), `src/systems/fairy.ts` (60 fairies
→ jobs, yield = Σ(Aptitude+3), axis↔job mapping still switchable/unverified),
`src/systems/dragon.ts` (1-3 of 18 genes → form/breath/sprite; gene set→
form mapping is derived from inferred combo semantics, not code-verified,
though the Kaiser sprite is the real KAIZAR asset), and
`src/systems/fishing.ts` (rod+bait → canvas minigame with real fish/bait
data; bait→fish and catch-chance are approximated, not the real RNG). A
basic turn-based `src/systems/battle.ts` prototype uses real party/enemy
stats, sprites, spell VFX, and item values, with an approximated damage
formula `max(1,pwr−def/2)·(0.85+RNG·0.3)`, hit chance, turn order, and
EXP/zenny (all labeled as approximated in the UI at the time).

**Living world: NPC movement VM.** `src/systems/npcvm.ts` interprets the
extracted NPC movement bytecode (`public/npcscripts/areaNNN.json`, 200
areas) at runtime, placing NPCs at their real `SET_POS` tiles and running
original choreography (`STEP`/`WALK`/`WAIT`/`LOOP` plus story-variable
handshakes `INCVAR`/`WAITVAR`). JSON stores raw opcode bytes
(`ops[].bytes`), so the byte stream is reconstructible without re-running
the extractor. Implemented ops: `SET_POS`/`STEP`/`WAIT`/`ANIM`/`SETVAR`/
`JUMP` (`0x4000` label rule)/`LOOP`/`IF`/`SPEED`/`TURN_TO`/`WALK_TO`; unknown
ops are skipped via their `OP_LEN` so the interpreter never crashes; gated
waits have a 5 s watchdog against deadlock. 136 of 200 areas carry scripts;
many NPCs run a scripted approach and then park on a story gate rather than
wandering permanently (faithful to event-actor behavior, not a bug); only
one NPC sprite exists (direction tracked internally but not drawn
distinctly); roughly 1028 scripts without `SET_POS` run invisibly as
"directors"; 64 areas are legitimately scriptless.

**Area names and text.** The textbox renderer's control codes are fully
decoded: `0x08` = `<GLYPH x>` with one operand byte (the only real desync
risk, fixed in `text.ts`); `0x09` = conditional box end (no operand);
`0x12`/`0x13`/`0x15` = kerning markers (no operand, peek-only); bytes
`0x17`-`0x1F` are NOT control codes, they fall through to the default
glyph branch. `extract/text.ts` decodes each area's text block (EMI
subfile at RAM address `0x80010000`): layout
`[u16 pointers_size][u16 offsets][strings]`, ASCII-compatible character
table (`0x41`-`5A`=A-Z, `0x61`-`7A`=a-z, `0x30`-`39`=0-9, `0xFF`=space) plus
the control codes above. `build-titles.ts` extracts each area's first
string as its scene title into `public/area-titles.json` — about 25
authentic ROM names (e.g. AREA121 "The Middle Sea", AREA088 "Urkan
Region"). `public/area-names.json` is a ~100-area fallback reference list.
Display prefers the ROM title, falling back to the reference name.
`dump-text.ts <n>` prints an area's full decoded text.

**Dragon transformation arithmetic.** Established from static battle-overlay
RAM (`0x80096800`) plus 22 live measurements via a dispatch-table hijack
(mode table `0x801eabb8` → commit function, breakpoint in its epilog — no
menu navigation needed, since even child Ryu lacks the Dragon command).
Full dossier: `references/re/dragon-arithmetic.json`.

Commit function `0x800a68a4` reads the selection queue
(`0x801463c4`/`63c7`) and first tries a special-combo matcher
(`0x800a6cf8`) against an 11-row × 3 table at `0x800b4d58` (`0xff` = wildcard,
priority = row order): Infinity+Trance+Radiant→TrueKaiser;
Force+Force+Trance→Trygon; Miracle+Reverse+Thorn→Wildfire;
Infinity+Failure→Kaiser; Infinity alone→Berserk-Kaiser (flag `0x10000`);
Failure alone→Whelp; Fusion→Hybrid (only with a 3-member party, gated by
`u8@0x801462f1==3`); Shadow+Trance→Tiamat; Force+Trance→Myrmidon;
Miracle+?→Mammoth; Mutant+?→Pygmy. Otherwise the generic path
(`0x800a6e14`) accumulates 14 axes from an 18×14 signed-byte gene table
(`0x800b4d7c`; Thorn doubles, Reverse negates axes 0-9, Mutant applies a
random ±1) and resolves: `acc[13]>0`→Behemoth, `acc[11]>0`→Warrior,
`acc[10]≥2`→Dragon, else Whelp. Element is 0-7, with 7 = Golden (matches
the earlier palette theory). `formCode` 0-24 maps via `0x800b4c90` to text
index 100-111; gene splices are stored as 6×u32 at `0x80145500`
(`[g0][g1][g2][elem<<5|form]`).

Form stats: base forms at `0x800b4e78` (4×5, ×10 = percent; Whelp 120,
Dragon 180, Warrior 150, Behemoth 300), adjusted by `f(acc)` at
`0x800b4e8c` = `[-5,-2,0,3,6]`. Special forms at `0x800b4ee0` (21×14,
HP/AP/Pwr/Def/Agl plus 9 resist bytes): TrueKaiser 300%, Trygon 220%,
Wildfire 10%, Kaiser 120%, Tiamat 250%, Myrmidon 180%, Mammoth 1000%,
Pygmy 100% (rows 11-20 are hybrid forms). AP: gene costs at `0x800b4c7c`
(elements 5; Force/Defense/Elder/Gross/Thorn/?/Trance 8; Reverse/Mutant 3;
Miracle/Fusion 16; Failure 1; Infinity 40); transform cost = Σ (computed at
`0x800a82f8`); upkeep = `⌈Σ/2⌉` (`0x801d6354`); AP shortage forces a revert
from a backup copy at `0x801e7e6c`. `formCode` 4 or 7 sets
`actorFlags |= 0x20000` (controlled/status-immune Kaiser state).

Corrections against community documentation: Miracle/Force/2-Ability act as
axis thresholds, not ranks; Fusion without a 3-member party yields Whelp
live, contradicting the community's assumption; Mammoth is Pwr 0%/Def 50%/
AP 200%, Wildfire is Pwr 300%/Def 200%/AP 250%; the community's reported
"Warrior bugs" are just the regular doubler/negator arithmetic. An earlier
internal reading of `_sources.spliceStorage` was also wrong — `0x800b7070`
is a 3×6 ownership grid built from the gene bitmask (init `0x800a4818`,
bit `k` = gene `k`). The Dragon command is unlocked by a story grant
(command ID `0x97`, skill 151 "Accession"), confirmed at handler
`0x800974c8+0x40`, not by a bitmask gate — the grant's storage location is
still open. Also open: the hybrid-partner ↔ `formCode` 15-24 resolver
(`0x800a805c`) and accumulator axis 12 (Defender).

**Fairy village job system.** The job-logic module is COMMU00.EMI (ct0 at
`0x801eec00`, the same zone BMAGIC's ct0 occupies, mode-dependent); its
main update (`0x801eed9c`) calls each facility updater in sequence. Facility
slots live at `0x801457a8` (2 bytes/slot: `[facility TYPE = rawTable
index][level/active]`). A fairy roster entry's `+1` field is its slot index
+1 (1-based, 0=free) — fairies are assigned to building slots, and the job
name simply follows `rawTable[slot type]`: Scholar 0, Merchant 1, Inn 2,
Gift 3, Fortune 4, Explorer 5, Antiques 6, Music 7, Casino 8, Copy 9,
Weapons 10, Items 11, Handyman 12, Speed 13, Ability 14. Updater type
filter (disasm): type 4 Fortune (`0x801ef244`/`3c8`), type 5 Explorer
(`0x801ef54c`), type 9 Copy (`0x801ef7e0`), type 11 Items (`0x801efec4`),
type 13 Speed (`0x801efa80`) — an earlier "axis pairs" reading (9→0, 4→3,
13→3, 5→2) was actually slot-type → aptitude-axis mapping.

Village expansion: `[0x801455c2]` inits to 10; `c3` is the expansion level
(inits to 1); `c4` is a counter; the growth check (`0x801ef4b0`) compares
against thresholds at `0x801f2518` = `[21, 24, 27, 30, 40]` (u16) with an
index table at `0x801f2523`. Roster creation is `0x801f0148` (job set to 0),
deletion is `0x801f026c`. Export: `fairy-village.json` carries `discBridge`
and `villageGrowth`. Still open: the slot-creation UI chain, a hard-coded
roster job-9 loop at `0x801ef164` (possibly a fixed slot 8), and the
COMMU02 roster helpers `0x801d7e68`/`7ea0` (an "nth active" walk with a
stride-1 anomaly).

**Battle minor leftovers — two negative proofs.** The percent-row field at
`+0xa8` (value 50) is a dead/reserve field: a full scan of the BATTLE EXE
region and overlay finds no reader at any access width, and no absolute
address `0x5f38` or context copy at `0x801ec320` either. The 5-byte percent
row is therefore fully explained: `[50=unused, 12=sleep proc (+0xa9),
5=crit, 6=dodge, 95=hit]`. Separately, the suspected "hidden 6th stat
field" at `+0x1a`/`+0x3a` of the context copies is mechanically unread: the
only `+0x3a` access found is the damage-number display routine (struct via
`[0x801ec2e0]`; `+0x3a` = display Y with a −9/frame rise animation;
`+0x30`+offset byte = start position) — no formula consumes it. Its
identity is therefore mechanically irrelevant to combat math (caveat: a
purely register-indexed access would not show up in an immediate-operand
scan).

**Battle input and persistence.** Battle was previously unplayable by
keyboard at all: `battle.ts` registered a modal key-capture guard
(`window.addEventListener('keydown', …, true)`) that called
`stopImmediatePropagation()` on every key so the world would not keep
running underneath the menu — but the command-cross menu handlers listened
in the bubble phase on `window` and therefore never received an event; a
code comment describing "click or arrows+enter" had been dead since the
GT-accurate command-cross rebuild. Target selection
(`selectTarget`/`selectTargetRaw`) had no keyboard path at all, only
`card.addEventListener('click')`.

Fix, in `systems/battle.ts`: a `menuKey` switch in battle scope. Menus
register their handler via `pushMenuKey` (unregistered through the existing
`cleanups` mechanism); the capture guard calls the registered handler
first and only falls back to closing the menu via Escape if it returns
unhandled. Target selection now supports arrow keys (left/right/up/down =
next target), Enter/K to confirm, Esc/Backspace to go back; mouse behavior
is unchanged, with hover still setting the selection so mouse and keyboard
state cannot diverge.

End-to-end proof (Playwright, AREA028, fought entirely via keyboard):
victory saved party state Ryu Lv 3→5 (exp 114), Nina 5→6, Rei 5→6,
Teepo 4→5, Peco 1→3, Momo 10, Garr 13 — unchanged after a reload, closing
an earlier open item about whether a real victory has lasting consequences.
Deliberately still open: the spell and item menus inside battle remain
plain clickable button rows; only the basic attack→target loop has full
keyboard support so far.


### Backdrop and band geometry

**Band system 112, the conveyor deck.** The belt deck (yellow-black stripes) sits over idx-0 void tiles (region 21-23/31). It draws from its own packet source: 0 grid slots, 0 override cells. The recon matches live VRAM; the stripe texels live in the dump's VRAM.

Flat-bake recipe: for flat runtime quads at a known height, one view is enough. Per vertex: `(c,r) = crAt(s,t,h_deck)` — no stereo, no foot point needed. Self-selection rule: the solution must land on idx-0 tiles of the region, with reprojection error under 1.2 px. The solver's `h` is world-y (map height divided by 8). The deck quads span both dump frames (double buffer), so baking needs all frames: 86 quads baked, 106 total in seed `runtime-geo-112`.

Still open at this stage: static workpiece crates (ordinary object-mesh geometry) sit on the belt route and cover the deck, while in the original they travel as cargo. The stripe texture itself runs via double-buffer animation, so a static bake only ever catches one phase of it.

VRAM-timing bug found through this deck: `parseGpuDump(...).vram` returns the VRAM state at the END of the dump. For upload-animated classes — double buffer, per-frame cell writes — cells held different texels at the actual best-match frame. That made the baked band-112 quads render dark even though the recon paints them yellow (deck rects averaged ~30 gray instead of stripe yellow). Fix, still open: `build-runtime-geo` must use the VRAM state AT the best-match frame, replaying write packets up to that frame instead of using end-of-dump `d.vram`. This affects every bake of an upload-animated class; static-class seeds are unaffected.

Related tooling: `parseGpuDump(buf, { snapshotFrame })` adds a field `vramAt`, the VRAM state at a frame's first draw primitive. `build-runtime-geo` uses it in two phases: find `bestFrame`, then decode again with the snapshot, letting `vramAt` take precedence over end-of-dump VRAM. For area 112 this changed nothing, since only one full upload exists in that dump. The API is still correct for genuine upload classes. Caveat from area 024: CLUTs can also be written mid-frame, after the first draw primitive, so "first draw" as the snapshot moment is only an approximation.

**Stereo recipe: Needleman-Wunsch alignment.** Repeating cells — belt stripes, rock textures — make plain UV-signature stereo matching ambiguous (one case: 1288 ambiguous candidates, 0 resolved matches). Epipolar bundle matching can also converge on a phantom plane (RANSAC picking a floating mid-plane that doesn't exist). Always check reprojection against the recon; tool `extract/reproj-seed.mts`.

Working method: the engine submits geometry in both dumps in the same order. Align the uvSig streams of a GPU class with Needleman-Wunsch (match +2, mismatch −3, gap −1; one frame per dump, since the double buffer repeats the sequence). Then solve 4 equations by least squares per vertex pair, gated by a 2 px reprojection error in both views.

Tool: `extract/stereo-nw.mts <pairA> <pairB> <area> "pg,cx,cy;…" [--replace]`. It needs two calibrated views (`--all-out` JSONs carrying a projection matrix `P`). Calibrate a second view with `build-rgeo-scene-from-dump --any-target --no-seed --state <sav> --all-out`.

Area 112 case: `pd112` × `a112cal` (from `area112-cal.psxgpu.zst` + `a112cal.sav`) yields 445 candidate pairs, 390 baked, height range 4.8-10.1. The belt is tilted, which is why the earlier flat bake could never fully match it. This also explains the "gray deck texel" puzzle. The gray 16×16 cells are the dark deck plates; the yellow stripes are the slanted belt edges, correctly reproduced once height comes from stereo instead of a flat plane.

Area 049 case (shrine crystal): `pd049` × `a049s5` triangulates all 8 facets, under 2 px reprojection error on both sides. Caveat: `pd049` × `a049s1` produced 4 garbage quads with column > 200. Always check world plausibility before using `--replace` — one such mistake cost 16 good quads, later recovered by rebuilding via `solver2seed`.

Tool: `extract/solver2seed.mts <pair> <area> <pg,cx,cy> [cmin,cmax,rmin,rmax]` takes already-solved ground-truth quads from an `--all-out` state-pool snapshot straight into the seed, assigning UV via bbox match against the dump. Caveat: pool-snap output often includes degenerate quads (two identical vertices, collapsing to an invisible line) — these get filtered automatically. For complex objects like the crystal, NW stereo works better than pool-snap seeding.

**Radar tubes, area 112.** The 4 machines in area 112 carry blue telescope tubes (texture page 704, CLUT 224/483). The browser shows the ROM-static rest pose, which is baked into the wall texture itself (confirmed via `pickObj`, the walltex system). Ground truth shows the tube extended and swiveled instead — mover-VM runtime state at the capture moment, not static geometry. A static bake of that one momentary pose would be wrong in every other animation phase; a billboard attempt in a depth plane produced floating artifacts and was reverted. This is filed under the "mover pose = VM state" family, alongside areas 172, 094, and 104 — a deliberate, permanent gap. The remaining unbaked quads in area 112 are exactly these 8 tube quads; nothing else there is unexplained.

**Seed lesson, area 024 dome.** Attempt: fill dome gaps via `solver2seed`, seeded from single views (`c64`, `c80`, `c488` from `pd024`/`b`/`c`). Result: all three pairs got worse. Mismatched quads rose from 32 to 57, 65, and 58 respectively.

Cause: single-view solutions for runtime quads are depth-ambiguous. Rule of thumb: never project across `w`, the homogeneous depth term. Every OTHER view sees a foreign view's quads placed incorrectly, even though the seeding view's own reprojection looks fine — a self-deception trap. Cross-view judging, not single-view reprojection, is the real test.

Correct approach for the dome: the stereo recipe above still applies — NW alignment over two calibrated views.

Revert method: `solver2seed` entries were removed by matching v-coordinates against the current `--all-out` solutions. Caveat: older batches can carry a slightly different `w`, since re-running the solver changes its solutions — match on class-exact terms.

Residual insights kept despite the revert: area 024's dome CLUT `c64` is genuinely static black; no CLUT writes appear anywhere in the dump. The ground-truth brightness there comes from the inner wall plus `c80`. `solver2seed` does not deduplicate against existing stock, so reruns can add duplicate quads.

**Skip-fill is free-camera cosmetics.** Skip-fill caps are blank tile caps painted with a neighboring tile's top texture. They exist to avoid "black holes" when a free camera flies over blank/skipped tiles — cosmetic infill, not accurate geometry. Ground truth can show real wall geometry in a skipped spot instead. One cliff back-wall renders as a real wall quad (texture page 448/256, CLUT 0/485) using a completely black 16×16 texture (UV 0,0-15,15, 100% black texel) — the void backdrop behind a gap in the cliff.

Fix: `setDLTCamera` hides `terrain.skipMesh`. `rebuildTerrain` keeps it hidden while a DLT-calibrated camera is active, since section composite shots rebuild terrain and would otherwise restore the cap.

Two lessons from this fix. First: a `let dltCam` declared after its first use in the module's startup path throws a `ReferenceError` and blocks the app from booting. `tsc` does not catch temporal-dead-zone errors like this — keep such declarations at the top of the module. Second: a failed verification run (`prim-check.mjs`) leaves the OLD `judge.log` on disk — after a fix, check the screenshot PNG's modification time before trusting an "unchanged" score.
### Refuted approaches

- **Yggdrasil's standing tree as per-area runtime code geometry** (like the
  AREA060/067 bridge class): refuted by a state-calibrated re-solve — every
  quad in the suspected class is ordinary map-meadow geometry. The real
  tree is a normal 90-record object mesh (types 3+4, page `704,256`,
  CLUT `483`). The "collapsed tree" symptom was a mis-baked rgeo backdrop
  lift, not a rendering regression.
- **Object anchors as a furniture/mesh placement table** (`param` →
  concrete mesh): refuted — no such mapping exists in the disassembly. All
  214 anchors in the original candidate set sit on already-raised tiles,
  meaning the furniture geometry is baked into map tiles and walls; anchors
  are the look-interaction table (examine, search, talk), and movable
  objects use the separate mover-entity system instead.
- **A hidden "second floor" renderer filling `tileTexIdx==0` holes**:
  refuted by centered GT captures — every checked hole shows black or sky
  under the character, matching this reconstruction exactly. The "402
  rapport" FT4 packages that looked like fill evidence were stale slot-pool
  residue left behind after camera scrolling, not a second render path. An
  early 7-area sample that seemed to show 2/7 real terrain was corner-biased:
  the visible terrain belonged to the adjacent real tile, not the hole
  itself, as a same-area corner-vs-center comparison in AREA147 proved.
- **A blanket walkability fence for all non-renderable `idx==0` tiles**
  (dropping the `walk=0x40` restriction): tested offline and rejected —
  many such tiles are real traversal bridges, and the fence cut the
  AREA150 traversal graph by 80% alone. Not implemented; the narrower
  `walk=0x4_`-only rule remains the shipped fix.
- **"The field handler skips `b0==1` roof records"**: refuted — the actual
  skip check tests `size==1`; `b0`/`b1` encode a general visibility
  condition, not a draw flag. The overworld-only roof gate that stood in
  for this understanding was removed once the full condition system
  (`condVisible`) was implemented.
- **"`f14≡f34` proves the flag system has no inversion"**: refuted — the
  two dumps being compared were accidentally identical emulator runs, not
  a real before/after pair. A corrected re-test (`f34 ≡ reference`, not
  `f14`) confirmed the `0x20` inversion bit is real.
- **Roof vertex formula needs an axis swap plus anchor offset**: this was
  a partial fit over one ~10-quad block, covering under a third of its
  roof surface. A full least-squares fit over 424 ground quads found the
  roof formula is identical to the plain wall formula, no swap needed.
- **TYPE `0x22` vertex format as `{z:i16,y:i8,x:i8}`, read as an
  "archway/thin strip"**: refuted by corrected disassembly — each vertex
  word is 3×10-bit signed fields, and the geometry is rotor blades
  (windmills), not architectural strips.
- **Assuming 8bpp for every rgeo bake target**: the AREA112 lever class is
  actually 4bpp; assuming 8bpp rasterized empty texels (CLUT filled, texel
  window zero). ColorMode must always be read from the dump prims.
- **Backdrop-class quads (trees, crystals, domes, firelight) exist as
  static records somewhere in RAM**: a full scan, including rotated byte
  orders, found zero matches for their UV signatures. They are generated
  procedurally from constants in code; only image-space capture (grid bake,
  stereo triangulation) recovers them.
- **Draw order is "first cell wins"** for sprite composition: refuted by
  pixel comparison against a battle GT dump — the PSX paints in record/cell
  order, so later cells overwrite earlier ones ("last-wins").

### Open

- TYPE `0x22` windmill geometry is not yet built in the browser. Next:
  `warp.ts 1 52 38 --dump` (or `warp.ts 148 8 43`) and search the dump for
  the resolved UVs to confirm before implementing.
- TYPE `0x22`'s extra per-block word (candidate i16 values −64/0/−80/64)
  has no confirmed meaning yet.
- TYPE `0x0f` animated riser has a GT dump but no interpretation
  (`references/gpudump/t0f_000.psxgpu.zst` + `SLES-01304_t0f000.sav`,
  `warp 0 15 12`); candidates are bubbles, sparks, or steam.
- Overworld roof runtime completion (10 placement entries → 18 drawn
  quads) is unexplained. Next: derive the mirroring rule (candidate:
  per-slope `b2&1` mirror around the entry's ridge edge) from further
  dump pairs beyond the AREA016 manor block.
- The "trio" duplex house's roofs have no feature entries at all; their
  source path is still unidentified.
- Cond class `0xfb`'s section-yaw source for non-default camera views, and
  the exact semantics of cond class `0xfe`'s state byte `0x8015933b`
  (live value seen: `0xa0`), are both open.
- Panel/railing TYPEs `0x43`-`0x49` still use a blanket 4-`PANEL_DEF`
  approximation instead of their real per-TYPE sub-handlers
  (`0x80158428`…`0x80158bb4`). GT-viable today, not exact.
- The backdrop packet class's procedural generator code itself is not
  disassembled — only its output is captured via grid bake / stereo
  triangulation.
- AREA060's horizon band (direct15-F6) is measured but not implemented.
  Next: a single GT screenshot of AREA060, then a screen-fixed overlay at
  ~27% height, ~8% tall.
- 78 opening-backfill hole tiles have no GT evidence and stay unfilled;
  neither their color nor their hole/silhouette status is decidable without
  a dump for each. Why the engine backfills at all is also still unknown.
- AREA112's rgeo lever anchor rests on only 2 map anchors; the flat base
  lever geometry's absolute anchor needs a third.
- AREA024's dome match plateaus at a blend-nuance residual (GT renders
  brighter/pastel, plus partial `c64`-class faces from another frame) —
  cause not yet identified.
- The "satellite dish" relay point's exact area/section is still
  unlocated, narrowed only to a neighbor of AREA112/AREA100.
- AREA034/044/075 hole classification (gray-lid skull block, Momo-area
  floor gap, Steel Beach sand) is unchecked or incomplete; AREA075 needs a
  GT capture centered on the sand itself, not the ship.
- Truly multi-part sprites (DRG01 + ~3 enemies) need the runtime object
  list (`objList @0x80182148`) captured live; not statically composable.
- Dragon arithmetic: the hybrid-partner ↔ `formCode` 15-24 resolver
  (`0x800a805c`), accumulator axis 12 (Defender), and the Accession grant
  storage field remain undisassembled.
- Fairy village: the slot-creation UI chain, the hard-coded roster job-9
  loop (`0x801ef164`), and the COMMU02 roster helpers
  (`0x801d7e68`/`7ea0`, stride-1 anomaly) are unresolved.
- Battle: spell and item menus still lack keyboard support; damage
  formula, hit chance, turn order, and EXP/zenny remain approximated, not
  disassembled exactly.

