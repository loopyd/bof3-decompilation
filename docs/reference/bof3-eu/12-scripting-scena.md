> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 12. Scripting: SCENA, the movement VM and object anchors

### Script containers

`SCENA##.EMI` loads to RAM at `0x801f6c00`: a header `[u32 0x0100|ID][u32 flags][NPC handler
pointer table]`, followed by code. Every NPC slot owns a handler pointer; unused slots share
one default/empty handler. Handlers are raw MIPS that call the engine API through `jal`,
targeting either the resident kernel below `0x80195a00` (from the EXE) or the `GAME.EMI` field
overlay (`0x80195a00`-`0x801d0c00`).

The SCENA loader, `0x801a7a84`, computes `file index = 663 (SCENA00) + story phase
[0x80146870]`. SCENA selection depends only on the current story phase, never on area or
region — an earlier "regions overlay" reading (the `scena-area-map` inference) is superseded
by this finding. SCENA N is the
dialogue/handler overlay for story chapter N: its `GET_TEXT` indices resolve against the text
block of whichever area is currently loaded, which is why the same slot works unmodified in
every area visited during that chapter. File-index base is the LBA table `@0x80182910`;
SCENA00-19 occupy indices 663-682.

Ground truth confirms the phase rule, not a region rule: SCENA00 loads at game start (phase 0,
area 000/McNeil) and SCENA01 loads at phase 1 (area 007) — both cases previously read as
"region matches" were in fact phase matches.

### The SCENA engine API

`extract/scena-api.ts` (`--verify --json` → `public/scena-api.json`) catalogs the engine API
that SCENA handlers call. Method: a `jal`-target histogram across all 20 SCENA files, folding
argument patterns (`a0`-`a3`) per target; semantics are cross-checked against a
`references/gpudump/ram1.ram.bin` disassembly.

Verified core API, SLES_013.04:

| Address | Call | Effect |
|---|---|---|
| `0x8015b7f4` | `FLAG_SET(base,bit)` | `byte[base+bit/8] |= 1<<(bit&7)` |
| `0x8015b81c` | `FLAG_CLR(base,bit)` | `byte[base+bit/8] &= ~(1<<(bit&7))` |
| `0x8015b848` | `FLAG_TEST(base,bit)` | returns bool |
| `0x8015b868` | `FLAG_XOR(base,bit)` | `byte[base+bit/8] ^= 1<<(bit&7)` |
| `0x80150490` | `GET_TEXT(strIdx)` | looks up the string in the area text block `@0x80010000` |
| `0x8019fc28` | `WARP(area,x_q16,y_q16,dir)` | writes the warp request `@0x80143f10`.. and sets trigger `0x80143bb0=5` |
| `0x8015c2fc` | `FREEZE` | `0x80146871 |= 0x40` and `0x80146258 |= 0x100` |
| `0x8015c2cc` | `UNFREEZE` | clears the same two bits |
| `0x8019621c` | `ENT_ALLOC` | allocates the first free of 20 entity slots, stride 36 B `@0x80143fc8` |

`FLAG_*`'s `base` parameter indexes into roughly 7 story-flag blocks spanning
`0x80144eb8`-`0x80145548` — the event flag store, which reappears in the save format.
`FREEZE`/`UNFREEZE` lock the player/field FSM during dialogue or a cutscene.

Suspected, not confirmed:

| Address | Call | Notes |
|---|---|---|
| `0x801c00b8` | `SPAWN(x_q16,y_q16,id)` | real-spawn candidate; `a0`=`x_q16` already occupied |
| `0x801c1b00` | `ENT_ACTIVATE(slot)` | called by all 20 SCENAs against a 308-byte entity struct |
| `0x801be434` | `CMD_INTERP` | byte-sequence reader testing `&0xc0` bits; NPC movement/anim-script candidate |
| `0x8014ef18` | `REQ_ENQUEUE` | sound/anim-cue candidate |

About 140 further `jal` targets remain unnamed in the histogram (`public/scena-api.json`).

⚠ `WARP` was misread by `scena.ts` as an NPC spawn call (treating `a0`=area as an id), which
contaminated `spawns.json` with 99 distinct area IDs (1-199) pulled in from warp calls.

### Dialogue: NPC talk state machines

Every talk handler is a state machine. It reads the NPC dialogue state — `0x80146864` (global
story progress) plus `0x80146875`/`76` (per-NPC sub-state) — calls `GET_TEXT(strIdx)` to load
the string into the textbox buffer, sets the field-FSM request byte `0x80143bb0=2` ("show
textbox", the same request-byte convention as `5`=warp), and writes the next state, producing
a multi-stage conversation.

`extract/scena-dialog.ts` (`--json` → `public/dialog/scenaNN.json`) extracts the dialogue set
per NPC handler slot. Verified against SCENA00: 17 speaking McNeil NPCs, indices resolved
against area 000 text yield real dialogue (slot 0 guard "Hey, you punks!!", slot 8 farmer
"Ah...spring...", progress dialogue stepping 1→2→3→4).

In the browser, `loadDialogNpcs` resolves each dialogue index live against `text/area<tag>.json`
for the currently loaded area (cached per `scena@area` pair); a baked reference text is used
only as a fallback. This mirrors the disc rule that text follows the loaded area, not the SCENA
file — evidence: the same slot (indices 7/8) reads "Grrrrr You little punks…" in area 007 and
"I didn't think you boys…" in area 000, both genuine in-game strings.

Open limitations: (a) the linear fold used to extract dialogue collects all state branches of a
handler, not the exact progression a player sees — that needs control-flow analysis; (b) the
SCENA-to-area text match was directly verified only for SCENA00/area000 and SCENA01/area007,
other chapters rely on the phase rule without a further per-area check; (c) the slot-to-spawn-
position link resolves only at runtime, via `ENT_ALLOC`/`ENT_ACTIVATE`.

### Cutscene player (SEQ)

842 SEQ sequences across 136 areas (`npcscripts` field `seq[]`) launch through `SEQ_START`,
`0x801be434` — the same address catalogued above as the suspected `CMD_INTERP`, now confirmed
as the party-cutscene launcher; its original triggers are the SCENA story scripts.

Playback is implemented in `npcvm.ts` as a mode of the existing `Npc` class, not a second VM.
Browser surface: a "Cutscenes (cutscene VM)" panel (selector plus play/stop) and debug calls
`__bof3.listScenes`/`playScene`/`stopScene`/`sceneState`.

Actors: one one-shot `Npc` per PROG reference, rendered with party direction sprites
(`PlayerSprites` of the SEQ actor's `charId`; VM direction `0`=NW…`7`=W maps to a frame key;
walk cycle at 8 fps, matching multiplayer remotes). Actors start on the player's tile — the
original SEQ takes over the party entities in place — and an absolute `SET_POS` in the PROG
applies immediately (the AREA000 intro sets party positions to `(15,20)`/`(15,18)`/`(17,19)`).
For `cls≠0` SEQs, header byte `n` is the party size; extra reference bytes are padding, not a
second actor stack.

Auto-dialogue timing was added only to replace the SCENA-side timing source the standalone
player lacks; the VM ops themselves are unmodified:
- a `WAITVAR` watchdog raises the story var to its target after 1.8 s, forward only;
- unfulfilled backward `IF`s — pose-wait loops on `story[k]`, e.g. `IF_EQ arg 17, var 3` —
  resolve after 3 rounds; equality comparisons raise the variable together with the check,
  keeping parallel actors synchronized;
- an unconditional backward `JUMP` is the "hold end pose until the SCENA despawns" idiom:
  after 4 rounds the actor is marked done, halted, end pose held.

Under these rules AREA000-SEQ0 (the Ryu/Rei/Teepo intro) runs to completion in about 31 s. A
cutscene starts on `vars=[0,0,0,0]` (the `SETVAR`-0 convention) and restores the area's own
world vars on end or stop, leaving its idle scripts undisturbed.

Hardening found along the way: `bgm.playSfx` now rejects non-finite volume (a NaN gain value
throws and stops the game loop), and the VM's SFX hook gained a NaN-distance guard.
`__bof3.warpTo` requires numeric `area`/`x`/`y` — a string or missing coordinate produces a NaN
player position, then a NaN camera, then a black viewport; this is a caller error, not an
engine bug.

### Movers: slot format and static census

There is no dedicated mover display format. Subtype-4 objects (movable object slots 30-33)
use exactly the same 40-byte quad records as subtype 6/7. Two earlier readings are superseded:
a "mover format ≠ 40-byte, wild vertices" thesis traced to stale slots in a save (`fish45.sav`,
`b0=0x00`=inactive) whose slot pointers (`0x801f300c`/`0x801f3674`) pointed at the predecessor
area of a warp chain (AREA014); at those addresses the loaded area's own overlay code
happened to sit, producing apparent "wild vertices"/"MIPS as a placement header". A "040 RGB
cube = mis-decode" reading was a follow-up misreading of the same kind — the ground-truth
framebuffer shows the cubes really are tilted, hovering above their pedestals (Momo's tower
switch, area 040).

Slot struct fields:

| Offset | Content |
|---|---|
| `+0x50` | `*(Placement+4)`, pointer to the 40-byte record array |
| `+0x54` | Placement entry address |
| `+0x3e` | runtime height, entity units ≈ 256 per tile |
| `+0x74`.. | movement VM context (`ctx`) |
| `+0x75` | animation dispatch variant, `= n·30` where `n = slot[+2]` (`1`-`3`) |

Placement setter `0x8015af00` initializes `slot[0]=1`, `[0x24]=0x40`, `[0x40]`/`[0x44]=0x1000`
(scale 1.0), `[0x48]=1`, `[0x54]=plc`, `[0x50]=*(plc+4)`, `[0x6c]=table 0x801c86a8[scrollKl·2]`
— identical for every subtype. Runtime height calibrates against ground truth: area 040 cube
hover heights of 254-404 correspond to 1.0-1.6 tiles, and a reference floor skeleton (area 108)
at height 64 sits at ≈ floor level.

Movement dispatch: vtable slot 4, handler `0x801a1584`, calls the movement VM at `0x801a8fb8`
(`ctx = slot+0x74`; program `= struct[+0x10][slot[+0x77]]`, one program table per area — area
045 has 3 programs at `0x801f501c`/`5030`/`5044`) and an integrator at `0x801a27bc`
(`+0xc`/`+0x10`/`+0x14` = dX/dZ/dH velocities in 16.16 fixed point, `+9` = tick countdown; on
expiry it ground-snaps via `0x801549f0` unless `flags&0x80`).

Static census (`0xffff`-anchor scan plus placement validation against `|n|=4096`-unit normals):
more than 40 of the 200 areas carry subtype-4 movers.

| Area(s) | Contents | Records |
|---|---|---|
| `030`/`031` | opening train ride, 10×241 track-sequence stages | 28-rec freight cars + 4-rec segment |
| `014` | 2 "windmills", reclassified as hay carts (see below) | 41 records, ±144 blade sweep |
| `040` | 4 switch cubes, 3 fabric colors via placement A/B/C | 6 records |
| `052` | 12 lava wooden platforms, op byte `xHalf=0x05` | 6 records |
| `121` | ships/ferry | 36/45 records, `0x117000` block |
| `087`/`088` | overworld ferry | 36 records |
| `095`/`096` | overworld ferry | 38 records |
| `126`/`132`/`134` | overworld ferry | 39 records |
| `148`/`149`/`167` | Caer-Xhan | 55+18 records |
| `186` (also `167`/`172`/`173`) | "loop elevator" car | 70 records |
| `170`/`197` | Myria | 15 records |
| `001`/`002`/`004`/`024`/`025` | mine minecarts/cranes | 17-30 records |

`build-meshes.ts` implements movers with three changes: the `xH`/`zH` half-tile guard was
loosened from a fixed value to any nonzero byte — the original registrar only tests `beq
zero`, so any nonzero byte (e.g. area 052's `xHalf=0x05`) is meant to set the half-tile flag;
the old drop rule was replaced by save
replacement (a save slot with `b0≠0` replaces the static instance of equal record count, for
objects up to 12 tiles, carrying over runtime position and height `h`); and mesh loading in
`main.ts` was moved out of the `if(features)` gate, since sequence stages without a feature
JSON — area 031 among them — had never loaded meshes before. Newly exported: areas
026/040/049/052/069/146/151/174, plus updates to 108/135/170, with zero regression against the
prior export.

### The movement VM

The object movement VM at `0x801a8fb8` is the same interpreter as the NPC movement VM: both
share the op-length table `@0x801c8a80` (`OP_LEN`) and the same opcode catalog. Movers and
field NPCs run one shared VM; only the program source differs.

Program lookup (vtable slot 4, `0x801a1584`, resolved at `0x801a1610`):
`prog = *(struct+0x10)[slot[+0x77]]` — the fourth program table in the per-area struct,
alongside `+0x14` (SEQ), `+0x18` (PROG), `+0x1c` (AUX).

Registrar `0x801a6f18` decodes a 14-byte placement op into a slot:

| Slot field | Source | Meaning |
|---|---|---|
| `+8` | `op[0]&0xf` | direction |
| `+1` | `op[5]` | subtype |
| `+2` | `op[7]` | animation variant |
| `+6` | `(op[0]>>4)-1` | state |
| `+0x77` | `op[10]` | program index |
| `+0x78` | `op[6]` | speed class |
| `+0x7c` | `u16(op[0xc..d])` | normally `0xffff` — also the anchor value static scans key on; movers with a real, non-`0xffff` parameter escape that scan |

Slot stride is `0x98` bytes, confirmed by the magic-number divide shared by the registrar and
the `d4` handler.

Dispatcher: a pre-dispatcher at `0x801a9214` loops control ops (`0x01`-`0x0f`, table
`@0x80195ec0`, plus the `0xaN`/`0xbN` families); the main dispatch then executes exactly one
move/pose op per tick, and each handler advances the program counter by its own operand
length before exiting `+1`.

| Opcode | Meaning |
|---|---|
| `0x01`-`0x0f`, `0xbN` | control ops, table `0x80195ec0`, consumed by the pre-dispatcher |
| `0x10`-`0x4f` | MOVE — direction and segment count, see below |
| `0x50` | `ctx[+1] := op&0xf` |
| `0x60` | handler `0x801abca4` (family `0x6`, jump table `0x801960b8`) |
| `0x80` | handler `0x801ab6d8` (family `0x8`, jump table `0x80196040`) |
| `0x90` | handler `0x801abb30` (family `0x9`, jump table `0x80196078`) |
| `0xa1`/`0xa9`/`0xac` | HALT, program counter stays put |
| `0xaX` (general) | `SETVAR`/`WAITVAR`/`INCVAR`/`NOP` on `0x80146864[(op&0xc)>>2]`, one of 4 story-sync bytes; handler table `@0x8018268c` |
| `0xc0` | handler `0x801aad80` (family `0xc`, jump table `0x80196000`) |
| `0xd0` family | jump table `0x80195fc0`; `d4`/`d5`/`d6` below |
| `0xe0` | handler `0x801a9bc4` (family `0xe`, jump table `0x80195f40`) |
| `0xf0` family | handler `0x801aa068` (family `0xf`, jump table `0x80195f80`) |
| `0xf2`/`0xf3`/`0xfa` | `[dir][count]`, count up to 255 |

For `0x10`-`0x4f`, `dir=(op-0x10)>>3` indexes the vector table `@0x80181f8c` (`0`=NW…`7`=W;
directions 2 and 6 move at half rate over `0x20` ticks) and `op&7` is a segment counter in
half-tile units (`ctx[+7]`/`slot[+0x7b]`); segment length is `0x10` or `0x20` units divided by
the divisor table `@0x801820a0[slot[+0x78]]`. Setup routine `0x801abe28` writes `slot[+9]`
ticks plus the `+0xc`/`+0x10` velocities.

⚠ Two earlier readings are corrected here. `0xf2`/`0xf3`/`0xfa` were first read as
`[anim][dir]`; disassembly of the dispatcher (`0x801a90dc`, which indexes the vector table
through `a1`) and of setup `0x801abe28` shows they are `[dir][count]` instead — village
"look-around" scripts only resolve into sensible pose chains, rotating through neighbor
directions, under the corrected reading. This also corrects the shared NPC movement VM, not
just movers.

### d4: rotation glide

`d4 [rx:BEs16][rz:BEs16][rh:BEs16][ticks]` glides the object's orientation, not its position —
an earlier reading of `d4` as translation is superseded. The trio at `+0x64`/`+0x68`/`+0x6c`
holds the object's Euler angles in PSX convention (4096 units = full circle). The object draw
routine `0x8015b6dc` loads the trio as an SVECTOR and passes it to libgte `RotMatrix`
(`0x80179c04`, sin/cos table `0x80186ddc`); rotation order is `M = Rx·Ry·Rz` with column
vectors in a y-down frame, and translation is `X>>9 - 0x4000`.

`d4` writes into a per-slot glide table at `0x80147cb8` (16 bytes: `+0`=active, `+1`/`+2`=
ticks, `+4`/`+8`/`+0xc`=delta/ticks per axis; `ticks=0` sets immediately). Its index,
`(slot-0x80147a58)/0x98`, independently confirms the `0x98` slot stride. Consumer `0x801a4018`
applies the first tick count to the X/Z angle and the second to the H angle. `d5` waits until
the glide finishes, holding the program counter; `d6` toggles bit 0 of the glide channel's
`+0` byte.

Ground-truth proofs: (a) area 014 PROG4, `d4 00 00 0f 40 0c 00 00`, matches the live angle
(`+0x68=3904`, `+0x6c=3072`) byte-exact; (b) with angles applied the area 031 locomotive sits
exactly on the ground (`y∈[-4,0]`; without them, raw offsets read ±1.5, half sunk), and its
cart wheels center correctly on the hub — angles are mandatory for correct mover placement, not
cosmetic; (c) area 040's live cube angles accumulate at the `d4` loop's own rates (pitch
32/tick, roll ±16/tick), reproducing the observed tumble, and timing calibration (4096/32
ticks ≈ 3.5 s/revolution against a reference video) yields a field tick rate of ≈ 36 Hz.

### Browser implementation

`build-meshes.ts` exports, per subtype-4 object, a `vm{prog,spd,dir,av}` descriptor plus the
`struct+0x10` programs as hex (`progs[]`; each window extends to the next base address, since
entries are entry points into one shared byte stream — a guard drops degenerate cases where an
entry falls at or above the table base, e.g. area 030). `render/meshes.ts` builds movers
object-locally; `systems/movervm.ts` is the interpreter (MOVE/`f2`/`f4`/`fa`, `d4`/`d5`, the
`aX` story vars shared per area, `JUMP`/`LOOP`/`LABEL`/`RESTART`/`IF`, per-segment ground glide,
and the exact `RotMatrix` conjugated to a y-up frame).

Marked browser-only additions, absent from the disc logic: a fixed `VM_HZ=36` tick rate; an
auto-cue that satisfies a `WAITVAR` after 2.5 s where the original waits for a story sequence;
an `AUTO_PROG` for area 186 that starts `PROG1` after 4 s where the original trigger is the
switch chain; map wrap at ±24 tiles, since an endless drive leaves the stage in the original
via an area transition; and native calls (the `de` family) rendered as static. This replaces an
earlier `anim:'tumble'` approximation for area 040: the switch cubes are now VM-driven exactly,
with 4 phases (one counter-rotating) and hover height set via opcode `f4` (`512` = 2.0 tiles).
Verified: `tsc` clean, zero geometry regression against a mesh diff, and all 2223 `npcscripts`
regenerate with 0 errors.

### Per-area mover programs: examples

Area 031 train, 5 programs at `struct+0x10`: the locomotive `(4,24)`, speed class 3, runs PROG0
= `d6 · d4(0xc00,0,0xe00) · a9 30` (`WAITVAR var2==0x30`, a story cue) `· LOOP{f2 05 64}` (100×
half-tile steps south). The arrival car `(4,45)` runs PROG3 = N×`16` · `SETVAR var2:=0x71`
("arrived") · N×`20` · HALT. The car at `(4,40)` runs PROG4 = set angle · HALT (parked).

Area 186 loop elevator: the slot parks on PROG0=`ff`; PROG1/PROG2 are endless loops south/north
(`f2 05`/`f2 01` · `64`) and loop over the inline pointer `9c 2f 1f 80` — a program JUMP with a
pointer operand, part of the `0xf` opcode family. In the original, starting the elevator runs
through the switch-/warp-field chain (`0xc0`/`0xa4`).

Area 052 lava platforms: PROG0 = `LOOP{de 01 · de 02}` calls per-area NATIVE functions rather
than byte-VM move ops, so it is correctly rendered as static without a VM interpretation.

⚠ Area 031's hero cars are not yet pixel-faithful. Verified correct: mesh CLUT type 10 =
`(112,72,56)` dark brown/red, matching the baked ore-cart texture; movement, driven by the
auto-cue's `f2` MOVE; geometry, 28 beveled quads forming an ore-cart box `X/Y±192`, `Z 0..-512`;
and rotation, applied exactly (live `moverInfo`: locomotive `rot=[3072,0,0]`, cars
`rot=[3072,0,3584]`, matching the `d4` operands `0xc00,0,0x0` and `0xc00,0,0xe00` from the
programs with `ticks=0`, set immediately). Still wrong: the cars render upright/camera-facing
with a washed-out lower half instead of rail-parallel like the densely baked terrain carts
beside the track — they carry an additional third-axis angle `rh=0xe00` (315°) that the
locomotive does not have. The shared `apply()` RotMatrix is left unchanged, since it is
independently GT-verified against area 014's carts and area 040's cubes; a faithful fix needs
ground truth of the moving opening cutscene (the cars' target pose), which the static warp used
so far does not show.

### Conveyor belt (area 140) and door animation (area 000)

The area 140 conveyor belt is a CLUT-cycle animation, not a UV-scrolling one. An 11-frame F8
dump series (`band140_s01-11`) shows 171 stable quads with 0 UV differences and 0 CLUT
reference changes frame to frame (a double-buffer trap makes vertex Y alternate ±240 between
framebuffers; keys must be normalized via `y%240`), but 4-phase CLUT cycles plus VRAM cell
phases. Pipeline: `extract-clut`/`anim-phases` plus `build-water-anim` write
`public/water/area140` (81 entries/115 tiles/4 phases); the browser belt animates accordingly.
Area 049 co-resolves as the same class (7-phase cycle, 32 entries/196 tiles, mine machinery);
area 148's matching `[17]` groups sit inactive in the warped save state, presumably story- or
machine-gated (12-frame series, 0 VRAM/CLUT diffs).

The belt's purple "workpieces" are not a riding UV element but discrete stationary entities. A
live save (`w13_band140.psxgpu.zst`/`SLES-01304_band140.sav`) shows them as blue-glowing
entities fixed at `0x89` belt-joint tiles: 116-byte effect slots `@0x80143fc8` (active, `b0&1`,
at `(72,29)`/`(78,29)`/`(88,10)`) plus a subtype-4 sprite mover `@0x80146888` at `(74,9)`, both
carrying a `p50` pointer into the AREA EMI descriptor region `0x800dxxxx` — sprite-based, so not
captured by `build-meshes` (which only handles overlay mesh movers). In a reference video they
sit practically stationary at their junctions. Browser: `buildBeltGlows` (`entities.ts`) renders
additive, gently pulsing glows from `public/beltglows/area140.json` (positions read from the
save), gated behind the "ambience" toggle — an approximation, since the original sprite itself
is not yet extracted; the player's `0x89` forced-movement behavior remains open.

The area 000 (McNeil) front door has no opening animation; entering is a plain fade. Reference
video (`gt-tuer-000.mp4`/`-up.mp4`) shows only a camera pan on approach, a statically rendered
door, and constant brightness (75.5) across all 35 approach frames — no fade, no door
animation, because the actual threshold crossing was never reached (pad injection does not
reach the field). This is reinforced by a full descriptor scan of area 000 (31 descriptor keys:
only NPCs, a chest, a sack, birds, and emotes — zero door sprites) and by the confirmed warp
fade elsewhere. A definitive walk-in test was not pursued, given the low expected payoff
against "just a fade".

### Object anchors and switches: the Momo tower (area 040)

A pedestal row at `x=69` (nav-map code `0x50`) carries four mover cubes at runtime positions
`(69,2)`/`(69,4)`/`(69,8)`/`(69,10)`, plus one at `(63,6)` found by a wider scan. Next to it: an
`0xa4` warp field spanning `x=65-68`/`z=3-9` (the elevator platform) and `0xc0` confirmation
tiles at `(71,5)-(71,7)`. No object-inspect anchor, NPC-VM script, or NPC spawn sits at the
pedestals themselves — all three were checked and ruled out.

There is no generic `0xc0` switch consumer. Dispatcher `0x801a8e04` (in the EXE) branches by
area ID to per-area NATIVE overlay hook functions — exactly the plant/elevator areas:

| Area | Hook |
|---|---|
| 40 | `0x801f3400` |
| 48, 111 | native hook present, address not resolved |
| 148 | `0x94` |
| 167 | `0xa7` |
| 169 | `0xa9` |
| 171 | `0xab` |
| 173 | `0xad` |

Area 040's hook, `0x801f3400(x,y)`, gates on `*0x80143f03==6` and story byte
`*0x801448eb==0x10`; its effect zone is the 4×4 field `x=0x94-0x97`/`z=0x20-0x23`. A successful
cube push writes `walk[tile behind the player] := 0x50` through `setWalk`, `0x801a4e40`
(`navBase = *0x8015931c + z·cols + x`; base setter `0x80154324` reads it from `md+0x14`), fires
tile trigger `0x801552f0`/`0x8015548c`, and plays SFX `0x205`. Puzzle check `0x801f3570`
compares the current 4×4 pattern (reader `0x801f30b8`) against the target table `@0x801f4960`;
a match unlocks the elevator's `0xa4` field.

Two more general lookups surfaced alongside this: `walkAt(x,z) = 0x80166f64` (canonical nav-map
lookup, ~80 callers) and the overworld edge-warp consumer for nav code `0xa1`, at `0x801a8d40`
(reads warp struct `0x80145040`/`44`/`48`, then calls `WARP` `0x8019fc28`). The Caer-Xhan
elevators (areas 148/167/173) run through their own area hooks, listed above; decoding exactly
what each hook sequence writes to `slot[+0x77]` (the program selector) remains open.

### World frame convention: x=col, y=row, z=height

The PSX field engine computes throughout in `(x=col/east, y=row/south, z=height, downward)`.
In the 40-byte mesh records, `v.z` is height — every object anchors at `z∈[-N,0]` above its
ground anchor, x/y symmetric — and `v.y` is the row axis. An earlier browser reading treated
`v.y` as height (a three.js convention); this happened to look correct for boxy furniture and
the symmetric area-040 cubes but left anisotropic objects — trains, ships, carts — standing
still, tilted 90° from their true orientation.

Evidence chain:
- The field camera matrix `@0x801492e8` (3×3 s16/4096 plus s32 translation) carries `0.866·z`
  in row 2 (screen Y) — the third world component is the height component; row 1 =
  `0.706·(col-row)`, the isometric diagonal.
- Object draw chain: `0x8015b6dc` loads the angle trio as an SVECTOR, `RotMatrix 0x80179c04`
  builds the rotation, and `0x8015b724` composes it with the camera matrix `@0x801492e8` plus
  `SetRot`/`TransMatrix` (GTE calls).
- Four assignable area-040 cubes with live `d4` angles favor `M = Rx(w0)·Ry(w1)·Rz(w2)`
  (standard CCW matrices in the col/row/down frame, positive angles) clearly over every
  alternative axis ordering tested.
- An effect slot in area 121 (`@0x143fc8`, `p50=0x8011770c`) carries the angle trio
  `(0,0,3584)` — yaw 315° — matching the measured ship diagonal (`Rz(315°)·e_x =
  (0.707,-0.707)`).

Ground-truth method, usable from a savestate alone: the finished, projected `POLY_FT4`
primitives of the object renderer live in the frame double buffer (split at its largest offset
jump; the map slot pool starts at `0x12d880`, object primitives sit in their own buffers).
Filter: `code&0xfc==0x2c`, CLUT row 483 (`clut>>6`), `tpage&0x1f==0x1b` plus 4bpp (object class
704/256). Matching a primitive's UV set against the `public/meshes` records yields exact
local-to-screen correspondences; for repeated identical objects (five area-040 cubes) the match
is picked by screen proximity to the projected slot position. Affine 3D-to-2D fits land at
0.18-0.39 px rms.

⚠ Refuted readings: a "camera-space billboard" reading of the angle trio (yaw billboard, trio
interpreted in the screen plane) was a consequence of the `y`=height misreading, not a separate
error. A companion claim that "the area 031 locomotive stands on the ground only with `d4`
angles applied" is also superseded — with `z`=height the records ground themselves
automatically; the `030`/`031` `d4` trios are a crash-choreography pose, not a grounding fix.
Area 014's "windmills" are hay carts (spoke wheel, load, and shafts; the "windmill cross" was
the tilted misreading). An alleged area 009 train-save object is the station's tree (a tree
texel sits in that UV window; no mesh train is parked at the station — the loading scene runs
through sprite slots at `0x800dxxxx`).

Browser implementation: `render/meshes.ts` builds vertices as `(x/128, -z/128, y/128)`; static
objects carrying an angle trio are baked through `psxTrioMatrix` (exported from `movervm.ts`;
`T:(x,y,z)->(x,-z,y)`, `M_three = T·M·Tᵀ`). `movervm.apply()` combines position with
`psxTrioMatrix`; a separate billboard code path and camera-relative parameters were removed.
`build-meshes.ts` exports the angle trio (`+0x64`/`68`/`6c`) and the runtime height (`+0x3e`,
now read signed) for non-mover save slots too — effect slots such as the area 121 ship, the
area 025 cage, and area 135 crates; most entities carry a standard yaw of 2048. As a marked
browser addition, static save assemblies within 0.75 tiles of a VM mover are baked host-locally
and travel with their holder (in the original, a story sequence moves both slots in sync).

Spot-verified: area 025 train ride (locomotive, cage car, coupled cage), area 031 transport
cages, the area 121 ship diagonal, area 135 black-ship cargo, area 040 (VM tumbling, no
regression), area 014 carts, and areas 077/108/002/170/186/052/087.

Open edges: area 009's tree sits at `h=5.75` against a ground height of ~3.4 for its slot,
possibly a different height convention for non-mover effect slots; a duplicate cage in area 025
(`(36.5,23.5)`) is presumed a spawn from another story phase, unverified; whether objects
without a save witness carry a nonzero registration yaw is open, though all checked cases sit
at 0°; and area 170's 15-record objects now read as flat circular platforms, a Myria-lift look
superseding an earlier "vertical door panel" reading, pending ground-truth comparison.

### Static objects with a save-sourced pose: the Yggdrasil (area 055)

Area 055's tree had been misclassified as a runtime-geometry ("rgeo") solve, which collapsed
into a meadow mis-bake; the whole rgeo approach was a dead end for this area and was discarded
(the rgeo-055 seed deleted; an empty `build-runtime-geo` run idempotently cleans up its keys).
Re-solving one dump against a save from the same window matched 508 pool entries at zero
offset, 0.18 px DLT: every `pg(448,256)`/`clut(0,485)` quad is ordinary map/meadow geometry,
while a 174-quad runtime overlay of class `pg(704,256)`/`clut(0|48|64,483)` is the object-mesh
class — the actual tree.

The tree is an init object at `(16,19)`, 90 records (types 3+4), `z∈[-768,0]` — 6 tiles tall.
Its runtime pose came from a save field slot 30 (trio `(0,0,3072)` = yaw 270°, `h=1152` = 4.5
tiles) and had never been read, for three compounding reasons: a save-filename mismatch
(`mesh67.sav` vs. the expected `mesh055.sav`, inconsistent padding meant the file was never
opened — both spellings are now tried); a non-mover record-count guard (`>64`) that discarded
the 90-record slot; and de-duplication that dropped overlapping save slots instead of using
them to replace rotation/height on the static object. The object's save `state` value is 10,
rendered even without the `0x80` bit set on `b0` — the existing "`state==8`" and "`0x80` bit"
visibility rules are therefore incomplete.

`build-meshes.ts` now sources runtime texel for save-bearing areas from the savestate's own
VRAM (page 704, CLUT row 483) instead of the disc reconstruction, since the Yggdrasil bark is a
runtime upload; area 121 differed by 27% of its page (ship texel), other save areas by ≤0.4%.

### The control-op VM: opcode model corrected

Full disassembly of the control-op chain (`pd106` RAM sample) locates the dispatch table at
`0x80195bc0` (207 entries). The table was found by a pointer-walk scan; a direct constant scan
for `0xde` in the code found nothing, because dispatch runs off the table, not inline compares.

| Op | Handler | Semantics | Length |
|---|---|---|---|
| `0xde` | `0x801a9b88` | **PLAY_BGM** — reads `b1`, calls the BGM switcher `0x80161dd8(track=b1, 0x64, 8)`; compares the u16 track table `0x80182384` (stride 4, values 209-218 = BGM slots) against the current slot `0x80145029`, loads only on a difference | 2 |
| `0x03` | `0x801a8214` | **SCREEN_FX** — fade setup `(4, 0x3e0000, 0x50000, 0x81)` plus `0x801448ec := 4` | 2 |
| `0xd4` | `0x80161ed4` | BGM-family call, not a distinct movement-VM op (see Camera below) | — |
| `0xd5` | `0x80161f84` | BGM-family call, not a distinct movement-VM op (see Camera below) | — |

`movervm.ts` now skips `0x03`/`0xde` instead of halting on them. The earlier HALT killed every
op that followed in the same program, silently truncating scripts that used either opcode.

The same lookup settles why the 052 lava platforms hold position: not because their program is
hard-coded, but because movement is gated by `WAITVAR` on a switch or player trigger. Two
`warp.ts` savestates taken apart in time (`pd052a`/`pd052b`) show identical positions for all
eight platforms, at `(17.5, 7.5-33.5)` — the browser's auto-cue authorization, which respects
the same gates, matches the original. The per-area dispatcher `0x801a8e04` (plant hooks) is
unaffected by any of this: it hangs off switches, not off VM op `0xde`.

### Story chaining: cutscene to phase to flag

Cutscene player, flag bank, condition evaluator, and phase selector existed as separate
building blocks with no link between them: the selector was a debug dropdown, and every reload
forgot all progress.

Engine template (`KNOWLEDGE.md` "STORY PHASE WRITER", the only writer, at `0x8019ff2c`):

```
if (byte[0x80146871] & 0x80) {      // "advance" flag = bit 7
  phase = byte[0x80146870] + 1
  if (phase == 0x10) phase++        // 0x10 is SKIPPED
  sb 0x80146870
}
```

This is a monotonic counter that story events trigger. `main.ts` now implements it directly:

| Building block | Implementation |
|---|---|
| `advanceStoryPhase()` | engine algorithm 1:1, including the `0x10` skip (tested: 15 to 17) |
| Persistence | phase and flag bank in `localStorage` (`bof3.storyPhase` / `bof3.storyFlagBank`) |
| Area change | survives — a load falls back to `auto` only when no progress is saved (the counter is one global byte; only its effect is area-specific) |
| Switch to flag | `toggleFlag` persists too (switch states survive reload) |
| Cutscene to phase | edge detection on `sceneState().running`; opt-in via the drawer toggle "story chain" |
| Debug | `__bof3.story()` · `story('advance'\|'chain'\|'nochain'\|'reset')` |

Verified with Playwright: the `0x10` skip; the world reacting (AREA015 goes from 45 to 53
meshes between `auto` and phase 12, the difference being the `0xfa` conditional features);
persistence across reload (phase 5 stays `5`, stored as `localStorage["5"]`); no console errors;
`tsc` clean.

⚠ The one approximation left at this point was which cutscene sets the advance flag. Dialog
data (`public/dialog/*.json`) carries text only, no flag ops, and the setters sit at the
SCENA/NPC-VM event layer. The browser trigger is therefore "cutscene finished playing," off by
default because the cutscene player also serves testing and must not mutate world state during
it. The flag harvest below closes this gap by reading `FLAG_SET` directly out of each script.

### Systematic flag harvest

The Gaist lesson — two pipelines read the same format, one extracts less — applied mechanically,
across two audit passes.

**Cross-extractor comparison.** `programAt` (the sprite-program reader) is used by 16
extractors. Checked for which ones fetch only program 0:

| Extractor | Programs read | Finding |
|---|---|---|
| `build-enemy-anims` | 0 + all | reference (`attacks`) |
| `build-boss-figures` | was: only 0 | fixed → 195 figures, 6558 frames |
| `build-emotes` · `build-teepo-adult` · `build-enemy-figures-static` | 0 + loop | ok |
| `build-chests` · `build-npc-spawns` · `build-master-sprites` | variable index | ok |
| `build-dragon-anims` | `idle` + `actions` | ok (previously thin too, per the extractor's own file header) |
| `build-dragon-icons` | only 0 | ok — menu icons need one image |

**Unused-field audit.** New tool `extract/probe-unused-fields.ts` collects schema fields from
all `public/**/*.json` (names occurring in at least 3 sibling objects, so atlas keys do not
drown the result) and checks whether `src/` ever references them. Result: 35 files carry fields
nothing reads.

The standout finding: `handler`, present in all 21 SCENA dialog files. Every NPC carries the
code address of its own story script — a field the browser never read, and exactly the field
that answers the story-chaining approximation above.

`extract/build-scena-flags.ts` (`npm run extract:scenaflags`) reads it: `SCENA##.EMI` is a
single code subfile at `0x801f6c00`; the NPC script runs from the handler address up to the
first `jr $ra`. Within that span, three engine calls are read with their immediate arguments:

| Call | Address | Arguments |
|---|---|---|
| `FLAG_SET` | `0x8015b7f4` | `base, bit` |
| `FLAG_CLEAR` | `0x8015b81c` | `base, bit` |
| `FLAG_TEST` | `0x8015b848` | `base, bit` |

| Metric | Value |
|---|---|
| SCENA files with flag ops | 13 |
| NPCs with flag operations | 1072 |
| `FLAG_SET` calls | 4073 |
| `FLAG_TEST` calls | 1193 |
| SETs with both args statically readable | 82% |
| Output | `public/dialog/scena-flags.json` |

The remaining 18% of `FLAG_SET` calls are register-computed and marked `bit: null` rather than
guessed. Values are plausible against the known layout: `base` = flag block 1-79 (scena-api,
roughly 7 blocks at `0x80144eb8ff`), `bit` = 0-119.

**Residual list**, found unused but deliberately not pursued: `prima-bestiary.json` →
`item1`/`item2` (loot cross-check against the disc-exact STEAL/DROP fields) · `enemies.json` →
`aiTail`/`enemyId`/`unk74` · `fishing.json` → `available`/`rawStats`/`usageFlags`/`sortKey` ·
`chests.json` → `b11`/`b12` · `dragons.json` → `spriteRow`/`reqBytes` · `clouds.json` →
`gtScreenBbox` · `waterglanz.json` → `hScreen`/`wScreen`. `fishing/shadows/index.json` →
`schatten`/`vorkommen` is unused on purpose: an observation log, not renderer input.

**Invariant audit across consumers.** Beyond unused fields: does every core invariant hold
everywhere the same data is read? Four checks, all four consistent:

| Invariant | Consumers | Finding |
|---|---|---|
| b6 CLUT mode | 8 extractors, each with its own decomposition | all identical (normalized `>>4`/`&0xf`/`·16` signature) |
| Battle vs. field CLUT | `clutForF2` (battle) vs. `fieldClut` (npc-spawns/emotes/…) | different addressing, both correct: battle linearizes (`blockB` sits in VRAM row 496 ⇒ `(480+row−496)·256+col`), field uses real 2D coordinates; the b6 decomposition itself matches |
| MOD-256 height unwrap | `unwrapHeights` only in `loader.ts` + `main.ts`; 4 extractors read heights | `build-rgeo-scene-from-dump` has the unwrap; `build-features` uses signed `S8h`, the documented special case — only 4 areas have `caps` (the S8h path), and none has a corner jump >128 within a tile, so no wrap seam is missed |
| Three water classes | 61 water JSONs | all three implemented and task-divided: `entries` → `texload.ts` (tile VRAM upload) · `wall` → `texload.ts` (wall water) · `feat` → `features.ts` (feature-floor CLUT cycle); no orphaned data path |

The yield of the lesson sits in the cross-extractor pass (`handler` → 4073 flag mappings) and in
the original Gaist finding itself; the invariant chains were already clean. Both audits are
repeatable tools (`npm run audit:addr`, `npm run audit:unused`) — the point of making them
tools: this error class never surfaces in judge/GT comparisons, since those only check what is
actually rendered.

### Render, camera, and living-world anchors

Verified anchors, not solved subsystems — each entry documents where the next attempt resumes.

#### The render machine and the "second ground level"

The map renderer's frame loop spans `0x801535xx`–`0x80153974`. Per window record
(`[col:u8][row:u8][slotIdx:u12|flags:u4]`), an RTPS visibility test (`screenX+0x32<0x1a5`,
`screenY+0xc8<0x1eb`) gates a slot allocator that runs when `slotIdx==0` (`jal` at
`0x801536f8`, target ≈`0x801547e0` as decoded from opcode `0x0c0551f8` — ⚠ unverified). Slots
are 80-byte packets at `0x8012d880` (2 FT4 at 40 B each; the slot pointer table at `0x801432ec`
is data). XY is patched per frame (RTPT at `0x801538f0`; world X = `col·128−0x4040` at
`0x801535c4`; corner heights at `0x80104030+tile·4`); UV/CLUT are set only on first build,
through the tile renderer `0x80154508` → entry decode `0x801557d4`.

Live traces across a reload cycle give two negative proofs: `0x80154508` never receives a hole
coordinate across 700 hits, and `0x801557d4` shows only four return addresses across 3000 hits
(`0x801545d4` for tops, `0x80154660`+`0x8015460c` for walls, `0x801563f8` for features) — no
unknown caller. GTE `ApplyMatrix` at `0x801791c4` never fires in area 082, ruling out an
object-mesh path. Yet the fill packets sit in the normal slot buffer
(`probe-w8-ft4scan.ts --cellU 5,6 --cellV 0,3` finds 402 repeat FT4 records in
`0x8012`/`0x8013xxxx`).

Strongest hypothesis: a second loop/renderer clone with its own inline entry decoding,
somewhere in `0x80153000`–`0x80155800` (the map-renderer family). ⚠ Both build routines run
only on area (re)load, so a breakpoint in a running game never fires — traces need
`probe-w8-trace.ts … --reload 82,25,20`.

#### Animation timing

Disassembly of `0x80157cf8`–`0x80157d28` gives the model: `divisor = struct[+0x8]`,
`rest = Timer(0x80143e6c) % divisor`, and the frame shows while `rest < struct[+0xa]`. The
struct carries screen offsets at `+0x6`/`+0x7` and a word at `+0x4`, identifying it as the
sprite path (the torch frames). The TYPE-0x10 handler `0x80156e8c` reads its timing bytes from
a 4-byte record array (`byte0 = divisor`); the dispatch table at `0x8017fb34` is indexed by
TYPE — verified for riser `0x0f` → `0x8017fb70` and rotor `0x22` → `0x8017fbbc`.

⚠ Savestate sampling alone cannot settle timing: aliasing leaves samples 17-25 timer ticks
apart with a 2-4 tick cycle, so six hypotheses stay equally consistent with the same data. The
disassembled formula above is the answer; more captures would not have resolved it. Captures
stay useful for coverage, though — a savestate carries the full VRAM (1,048,576 B, i.e. a
524,288-entry `Uint16Array`, the same format as a dump), so `extract-anim-phases.ts` accepts
`.sav` paths directly; 55 areas have multiple savestates, and area 121 alone raised its animated
entries from 78 to 84.

#### Camera

⚠ Correction of an earlier assumption: `0x801481e0ff` is the camera's glide state, not a static
per-section table. Interpolator `0x8019ac60` holds a fixed-point angle plus per-frame deltas at
`0x801481f8`; snap `0x8019acfc` reads its targets from the camera entity at
`+0x64`/`+0x68`/`+0x6c`; map init `0x8015411c` sets the defaults `−682/0/512`, the rz512
standard camera. Section pans are event- and entity-driven, so no static export exists — a 1:1
reproduction needs either GT sweeps per section or extraction of the entity-target setters.

Side finding: the "d4-d6 glide channels" are not movement-VM ops at all. Control ops
`0xd4`/`0xd5` resolve to `0x80161ed4`/`0x80161f84`, both members of the BGM family.

#### Fairy village job module (COMMU00)

Job logic lives in `COMMU00.EMI` (ct0 at `0x801eec00`, the same zone the BMAGIC ct0s use,
occupied per mode). Main update `0x801eed9c` drives six facility updaters; the job filter is
dynamic (`cell+1`), which makes roster job ids 1-based (`0` = free). Roster creation/deletion
sit at `0x801f0148`/`0x801f026c`.

Cells `0x801455c2`/`0x801455c3`/`0x801455c4` are the village-expansion state (init `10/1/0`;
`c3` = level, `c4` = counter; the upgrade check `0x801ef4b0` compares against thresholds at
`0x801f2518` — `[21,24,27,30,40]` — plus an index table at `0x801f2523`). Solved elsewhere and
unaffected by any of this: 60 fairies at `0x801f2700` (name + 4 aptitudes), roster at
`0x801455c8`, job yield = `Σ(apt+3)` tallied at `0x801ef164`, 13 jobs at `0x801d4ca8`.

The job-name bridge itself is still open — it hangs on the a2 register chain of the six
facility updaters. Fastest proof: read `roster[+1]` against assigned jobs from a fairy-village
savestate.

#### Fishing (BATE overlay)

`BATE.EMI sub[0]` is code, proven block by block across all 33,864 B: 93-100% valid MIPS
opcodes, and every 4-KB block's `addiu $sp,-N` prologue is paired with a `jr $ra` — random data
produces prologues but not matching returns. HUD print paths address the format string at
`0x801d0c04`. ⚠ Methodological lesson: "not findable in RAM" dates the savestate, it does not
disprove code — at the capture moment the overlay was simply unloaded.

Structure: `sub0` at `0x801d0c00` opens with data (movement patterns at `0x801d0c50`); the main
state machine dispatches at `0x801d1000` (state byte `0x80148652`, handler table `0x801d8718`).
Accesses to the parameter zone `0x80148330`–`0x8014835e` cluster at `0x801d1044`–`0x801d1370`
(spawn/state machine); a live GT capture shows `0x80148330ff = [1,3,SPOT?,1,x,y]` and
`0x80148354ff = [.,3,0,1,65,0,200,0,0,0,2]`. Two external parameter candidates, `0x80145aa8`
(`lhu` at `0x801d1048`) and `0x80144953` (save byte, `lbu` at `0x801d1028` → `jal`; the bait
candidate), both read `0` in the available field states. The start gate is rod ownership —
early pokes never reach the state at all.

Item tables are solved: fish at `0x801c9008` (18-byte records), rods and bait at `0x801ca300`
(20-byte records, marker `32 40`), text codec `0xff` = space, `=` = hyphen; rods carry no icons.
Bait→fish assignment is still open — next step is a savestate captured past the rod-ownership
gate, reading `0x80145aa8`/`0x80144953` live. Full account: chapter 16, Fishing.

#### Shops

The shop id is read from `0x80148699`. `SHOP.EMI` reads it at `0x801d0fac` (`lbu [0x80148699]`
→ `·23` → list pointer `0x801ca510` → `[0x8018ec9c]`), at a second site `0x801d0fd4`, and again
at `0x801d0fa8` (the same `·23 + 0x801ca510` computation). `SHOP.EMI` also stores to the byte,
at `0x801d0f40` (`lw v0,0x1c(struct)` → `sb → 0x80148699`) — copying a struct field rather than
an immediate, distinct from the setter sweep below and not cross-checked against it here.

The SCENA start sequence (GT `SCENA01` at `0x801fcb70`) is `[0x8014865c]:=−2 ·
[0x80148698]:=1 · [0x80143b90]:=7 · [0x80148650]:=0 · [0x80148699]:=0 · dialog state:=7/25`,
where `0x80143b90` is a sequence-counter channel (family `0x80197238`), ⚠ not a mode request.
No non-zero setter of `0x80148699` exists in the 20 SCENAs or the field band ⇒ the id looks
populated dynamically outside those sites (candidates: the talk-handler API with a
register-passed id, or the menu overlay). The only writer found in `GAME.EMI` is a reset
(`sb $zero`). Fastest proof: a savestate captured inside a shop, reading
`0x80148698`/`0x80148699` and `[0x8018ec9c]` live.

#### AREA086 intro crane

A seven-part slot ensemble sits at `(51-54,18-21)`, height `960-992`, in the 34 bank (slots
7-13 at `0x146888+n·0x98`). Five parts share the geometry pointer `0x800d46c2` (`p54`, count
16); two are hooks `0x800d99ea`/`0x800d999c`. The rotation choreography is mover program 9
(`WAITVAR var0==34` → SFX bank2-13 → 3× `d4` rotate-glide + `d5` → `INCVAR` → `WAITVAR
var0==36` → `d4` big rotation → `HALT`); the day-labourer NPC's program 2 raises `var0`.

⚠ The geometry at `0x800d46c2` is not a 40-byte record format — a stride scan across 40-64 came
back negative, and the hex is a packed stream (disc equals RAM), suspected to be an `0xfN`
sub-stream. Decoding that format is the remaining step; `movervm` already implements
`d4`/`d5`/`f4`, so the choreography itself plays.

### Smaller anchors

| Subsystem | Key facts | Status |
|---|---|---|
| Nav map | entry point `0x8010bd30` (map header `0x80104000` + `u16[10]·4`), runtime pointer `0x8014931c` (10 access sites), writer `0x80155650` | open — lo-nibble semantics unresolved; the consumer (movement collision / encounter check) is pointer-based and not isolable via a `lui` constant scan |
| Dragon genes | 18 genes from the `FIRST.EMI` pointer table `0x8001b980` (ROM-verified), breaths `0x801cb2e8`; `formId` cell `0x800b6f58` written by `0x800a69e4` as `resolver(0x800a6c2c) + 4` (`0xff` = no dragon; `formId` 7/8 = Kaiser family, special flag `+0x128 |= 0x10000`); base-form records `0x800b4d58` are 2-byte entries; check function `0x800a6cf8` | solved |
| Intimidate | setter `0x800a0958` marks the applier (`flags |= 0x10`) and sets `[0x801463ce] = 4`; filter `0x801db898` restricts acting combatants while the timer runs | solved — verified in a random battle (AREA138/Hobgoblin) |
| Battle context copies | phase handler copies combatant contexts at start; copy block begins at `0x801ec2d0` (6× u16 rows plus resist words) | open — hidden fighter field at `+0x1a`/`+0x3a` has no known stat identity |
| Line primitives | `LINE_F2`/`LINE_G2` (codes `0x40`/`0x50`) initialised at `0x8017af4c`/`0x8017af60` | open — many ct0s chain prims inline into the OT instead of going through `0x80155c7c`; read heap `0x80028fcc` up to cursor `0x8014598c` sequentially (`[len@+3][code@+7]`) to capture them |
| Party battle sprite codec | LZSS entry `0x80164318`, mode1 `0x8014e948` (line de-interlace), generic dispatcher `0x8014e820` | open — addresses identified, decoder not yet implemented |
| Master gates | per-area init overlay holds the checks: Bunyan AREA003 and Durandal AREA059 (both `0x801f2c04`) have none (unconditional — availability is story-gated only); Hachio AREA041 `0x801f3bcc` runs 4× `INV_COUNT` + 4× `INV_REMOVE` (ids 73/63/39/29); Deis AREA098 `0x801f5ac8` is a pure dialogue chain; Hondara AREA074 `0x801f2c04` tests skill bitmask `0x80144f94 & 0x20` = bit 165 = skill id 165 (Backhand) | solved |
| Item categories | 3 = accessory (`0x801ca100`, 20-byte records), 4 = key item (`0x801c9260`, 16-byte records); effective-stat derivation at `0x80164da8`; field-side request-struct writer (`+0x1c`, the SCENA/dialogue argument) at `0x801a855c` | solved |
| Backdrop packet generators / mover pose family | procedural generators for camp tree/crystals/glass (arena writer) | open, low priority — raster/stereo bakes already cover everything visible; mover pose family confirmed non-bugs |

### Refuted approaches

| Approach tried | Refuted because |
|---|---|
| `de01` read as a "native placement" hook (106 ramp, 052 class) | `de01` only selects BGM track 1; object position comes from the object instance (save/init data), not the VM program |
| VM interpreter halting on unknown ops `0x03`/`0xde` | HALT killed every op that followed in the same program; both are now known (SCREEN_FX, PLAY_BGM) and are skipped instead |
| 052 lava platforms treated as static "because native" | gated by `WAITVAR` on a switch/player trigger; two time-separated savestates (`pd052a`/`pd052b`) show identical positions absent a trigger |
| "d4-d6 glide channels" treated as movement-VM ops | control ops `0xd4`/`0xd5` resolve to `0x80161ed4`/`0x80161f84`, both BGM-family calls |
| `0x801481e0ff` read as a static per-section camera table | it is the camera glide state (interpolator `0x8019ac60`); section pans are event- and entity-driven, no static export exists |
| Village-expansion cells `0x801455c2`-`c4` read as the facility id | they hold expansion level (`c3`) and counter (`c4`), not an id |
| "COMMU02 triple" at `0x801daf0c` read as the job-name bridge | it is a UI-layout triple `[facility, slot, icon/text]` (base `0x801daf00`) feeding render function `0x801d91d4` |
| `0x80146875 := 7` read as a shop "mode protocol" | the byte is the per-NPC dialog state (hundreds of setters, values 1..55, game-wide); shop sites only set follow-up dialogue |
| `0x80143b90` read as a shop mode request | it is a sequence-counter channel (family `0x80197238`) |
| `BATE.EMI sub[0]` judged "not code" from absence in a live RAM sample | the overlay was simply unloaded at the capture moment; block-level proof (93-100% valid MIPS, paired prologue/`jr $ra` across 33,864 B) confirms code |
| Fishing `flag` field read as a bait mask | discarded on inspection |
| `stateStruct` at `0x80148330` treated as a fishing-state candidate | empty in every available fishing savestate |
| `fishEntityArrayPtr` at `0x8014598c` treated as a fishing-state candidate | points into the kernel area |
| Dragon base-form records at `0x800b4d58` read as 3-field triples | corrected to 2-byte entries |
| AREA086 crane geometry at `0x800d46c2` assumed a 40-byte record format | stride scan across 40-64 came back negative; the data is a packed stream (disc equals RAM) |
| Header byte 30 as the source of the "second ground level" | nonzero in 88 areas with no correlation to the phenomenon |
| TYPE-0 / `b0≠1` as the source of the "second ground level" | no supporting trace found |
| Header's small tables (`[17]`/`[20]`) as the source of the "second ground level" | empty in areas 082 and 034, where the phenomenon occurs |
| Savestate sampling alone to pin animation timing | aliasing leaves six hypotheses equally consistent (samples 17-25 ticks apart, 2-4 tick cycle) |

### Open

| Thread | Concrete next anchor |
|---|---|
| Render machine / "second ground level" | Confirm the slot-allocator target (`jal 0x801536f8`, decoded ≈`0x801547e0` from `0x0c0551f8`, unverified) and trace its return addresses within `0x80153000`-`0x80155800`; needs a reload-triggered trace (`probe-w8-trace.ts … --reload 82,25,20`) |
| Animation timing, remaining TYPE cases | Apply the divisor/rest formula (dispatch table `0x8017fb34`, handler `0x80156e8c`) to TYPE-0x10 entries beyond riser `0x0f` and rotor `0x22` — the torch case |
| Camera section pans | GT sweeps per section, or extraction of the setters that drive the camera-entity targets at `+0x64`/`+0x68`/`+0x6c` |
| Fairy job-name bridge | Read `roster[+1]` against assigned jobs from a fairy-village savestate; the bridge sits on the a2 register chain of the six facility updaters (`0x801eed9c`) |
| Fishing bait→fish assignment | Capture a savestate past the rod-ownership gate and read `0x80145aa8`/`0x80144953` live |
| Shop id population | Capture a savestate inside a shop; read `0x80148698`/`0x80148699` and `[0x8018ec9c]` live; candidates are the talk-handler API or the menu overlay |
| Nav map lo-nibble semantics | Pointer-chase the runtime pointer `0x8014931c` (10 access sites) — the consumer is not isolable via a `lui` constant scan |
| Battle context hidden field | Identify the stat identity of the hidden fighter field at `+0x1a`/`+0x3a` in the copy block at `0x801ec2d0` |
| Inline-chained line prims | For ct0s bypassing `0x80155c7c`, read heap `0x80028fcc` up to cursor `0x8014598c` sequentially (`[len@+3][code@+7]`) |
| Party battle sprite codec | Implement the decoder: LZSS entry `0x80164318`, mode1 `0x8014e948`, generic dispatcher `0x8014e820` |
| AREA086 crane geometry format | Decode the packed stream at `0x800d46c2` (suspected `0xfN` sub-stream); choreography already plays via `movervm`'s `d4`/`d5`/`f4` |
| Backdrop packet generators / mover pose family | Disassemble the procedural generators (camp tree/crystals/glass, arena writer) — low priority, raster/stereo bakes already cover visible output |

