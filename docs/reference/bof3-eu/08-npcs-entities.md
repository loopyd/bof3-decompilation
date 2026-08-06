> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 8. NPCs, spawns and world entities

World entities come from three systems: a per-area init overlay with static placement and event
scripts, SCENA-driven NPC spawns, and a shared particle table for effects like leaf bursts and
birds. A second byte-VM layered on the overlay drives NPC movement and cutscene choreography,
synchronized to a small set of story bytes. Chests, save points, and chimney smoke are
deliberately absent from any static table and run through runtime scripts or entities instead.
Sprite graphics for NPCs, furniture, and particle effects share one descriptor-driven lookup
system.

### Entity sources

**Init overlay.** The per-area init overlay loads at fixed address `0x801f2c00` (fixed load
address via the EMI TOC) and is the compiled per-area entity/event system, not merely a warp
table. It is directory-driven, not fixed-offset: a closing pointer directory references every
section absolutely, and table offsets vary per area. Sections, in order:

- header word
- per-area init code (`fn0`-`fn3`): sets story/dialog state at `0x80146864`-`0x80146867`,
  installs entity anim hooks
- warp/door table, 12 bytes per entry
- object anchor table
- event/behavior bytecode script
- GTE display-list programs (the area's 3D object models)
- closing pointer directory

This layout is verified consistent across areas 000, 003, 007, 008, 033, 060 and 121.

**Particle/effect entity dispatch.** Field particle entities live in a shared 20-slot table at
`0x80143fc8`, 116 bytes per slot (full record layout under Particles and emotes). Each
field-frame tick (`0x8019a2e4`) walks the 20 slots and points scratchpad context `0x1f800044` at
the active entity. It then dispatches through a type-handler table at `0x801c7ef4` (`[typ·4]`,
indexed by the slot's TYPE byte at `+5`). Documented types: `0x34` → `0x8019dfcc`
(particle/emote effects, detailed below) and `0x5c` → `0x8019a530` (object-mesh entities).
`ENT_ALLOC` (`0x8019621c`) is a plain free-slot scan (`active==0`) with no registration step, so
a hand-written slot is processed like any other.

**Movement and cutscene VM.** NPC movement and cutscene choreography run on a second byte-VM,
separate from the SCENA talk handlers. Three layers:

- **SEQ_START** `0x801be434(idx)` — launcher; resolves `area@0x80143f00` → `struct
  *(0x8017fe40+area·4)` → `+0x14[idx]`. A SEQ record is `[class:2][flags:2][n:4]` plus actor
  charIds plus one PROG index per party entity. `class 0x00` physically reorders the entity
  structs and issues FREEZE.
- **STEPPER** `0x801b6e4c` — runs per frame: wait counter, velocity integration, animation.
- **VM_FETCH** `0x801a8fb8` — executes control ops in a loop plus one pose/move byte per tick.

The full opcode catalog, confidence-flagged `V` (verified) or `?` (unconfirmed) per entry, is
kept in `NPC_VM_OPS` (see Open). Operand lengths for every opcode come from the engine's own
256-entry table at `0x801c8a80` (`OP_LEN`), adopted exactly rather than re-derived.

| Opcode | Meaning |
|---|---|
| `01` | JUMP |
| `02` | LOOP |
| `03`/`de` | CALL_NATIVE |
| `04`-`09` | IF/VAR comparisons (var3-6 = story bytes `0x80146864`-`0x80146867`) |
| `0b` | RESTART |
| `0d` | SET_POS `[tX][halfX][tY][halfY]` (pos = tile<<16 \| half·0x8000) |
| `0x10`-`0x4f` | STEP/POSE (anim=(b-0x10)>>3, dir=b&7: 1..7=N/NE/E/SE/S/SW/W, 1 step per byte) |
| `50`-`5f` | WAIT n |
| `80`/`81` | FREEZE/UNFREEZE |
| `86` | CHAIN |
| `a0`-`af` | SETVAR/WAITVAR/INCVAR (story byte `0x80146864+k` = dialog↔cutscene sync) |
| `c8` | TURN_TO |
| `d9`/`da`/`df` | field-FSM gates (`0x80143bb0==2` = textbox) |
| `e6` | DESPAWN |
| `e8` | SPEED |
| `f7` | WALK_TO |
| `ef` | END (entity returns to AI) |
| `ff` | HALT |

The area→struct table at `0x8017fe40` is pre-initialized inside the EXE (200×u32, no scan
needed). The struct holds `+0x14` SEQ, `+0x18` PROG, `+0x1c` AUX, and `+0x3c` native-function
pointers; field order varies per area, so all lookups are order-free.

Verified against disc data: 2223 PROG/AUX scripts across 200 areas tokenize with 0 errors, and
2242 SEQ→PROG references all resolve in range. Position format matches the `ram1` entity structs
at `0x80145e90`, including the half-tile `0x8000` bit. Example: AREA007-PROG0 is the entrance
cutscene — `SET_POS` onto the walk=`0xc0` warp-landing tile (2,20), 4 south STEPs, an SFX cue,
then a WAITVAR on `story[0]` that paces the SCENA dialog. 138 of 200 areas carry scripts; the
rest are legitimately scriptless.

### Spawn records and sprite keys

NPC spawns come from `/BIN/SCENARIO/SCENA##.EMI`, compiled MIPS subroutines. `extract/scena.ts`
scans them statically for `jal` calls to the spawn helpers `spawn_v1` (`0x8019fc28`) and
`spawn_v2` (`0x801c00b8`), then constant-folds the arguments (`$a0`-`$a3`, including the delay
slot):

| Field | Description |
|---|---|
| `id` | spawn/actor identifier |
| `x`, `y` | position, Q16 fixed point |
| `dir` | facing direction |

About 315 spawns resolve this way, written to `public/spawns.json`. A browser-side viewer
overlays a selected SCENA's spawns onto the map as orange cones. Caveat: SCENA-to-area mapping
is not automatic and must be selected manually; spawns outside the active region stay hidden.

Sprite graphics for field entities — NPCs, furniture, and particle effects alike — resolve
through one shared descriptor→container lookup: `descLookup` (`0x8014de8c`) operates on
`@0x800e3800`, and sprite programs sit at `@0x800d3800`. The same system serves enemies and
furniture. Lookups are keyed by a small integer; the particle/emote family's keys are listed
under Particles and emotes.

### Story-phase variants

Four story/dialog-state bytes at `0x80146864`-`0x80146867` gate entity behavior. The init
overlay's `fn0`-`fn3` write them at load time. The movement VM reads them as `var3`-`var6` for
its IF/VAR opcodes (`04`-`09`), and updates or waits on them with SETVAR/WAITVAR/INCVAR
(`a0`-`af`, `0x80146864+k`) to synchronize cutscenes with dialog. Example: the AREA007-PROG0
entrance cutscene issues a WAITVAR on `story[0]` to pace its SCENA dialog.

### Fixed entities

The init overlay's object anchor table (`extract/build-objanchors.ts` →
`public/objanchors/areaNNN.json`, 214 anchors across 32 areas) holds 8-byte records:

| Offset | Field |
|---|---|
| `0` | tile X |
| `1` | tile Y |
| `2` | b2 (∈ {`0x01`,`0x87`}) |
| `3` | b3 (∈ {`0x02`,`0x03`}) |
| `4` | param, u32 |

100% of anchors sit on walk=`0x50` tiles, the furniture zone — the static placement table for
area furniture and object mesh-groups (see Refuted approaches). `param` resolves through the
look-interaction lookup `0x801b5ed8`: it is an "examine" table (texts, scripts, one-time finds),
not a mesh-placement table. `b2` encodes axis plus look direction; `b3` encodes tile extent.
Examine scripts run on the same event-script VM as the SEQ launcher (overlay `+0x274`, same byte
VM as `0x801be434`).

Chimneys are not listed in the overlay at all. Cross-checked against `chimneys/area000.json`,
chimney smoke is confirmed as a purely runtime-spawned entity. Chests and save points likewise
have no own coordinate table — both run entirely through the overlay's event script (`+0x274`).

A separate resident table at `0x801cd4d0` holds camp and random-encounter data, byte-exact from
`GAME.EMI` `sub[0]` at `0x37ad0`, exactly 10 records (complete):

| Offset | Field |
|---|---|
| `0` | overworldArea, u16 |
| `2` | campArea, u16 |
| `4` | x, u16 |
| `6` | y, u16 |

Reader `0x801b66f4` applies it as: on overworld region X, warp to the camp scene at (x,y). An
adjacent table, `0x801cd4a8` (4-byte records, `[overworld][arena]`, reader `0x801b669c`), maps
random-encounter arenas. The repeated `(19,22)` target is a fixed spawn inside the shared camp
scene, area 90, not a town position. Regular world navigation does not use this table; it runs
through the per-area warp table instead (see Refuted approaches).

### Particles and emotes

Addresses in this section are EU EXE offsets, disassembled from `mcneil_ent` RAM captures. Field
particle effects (type `0x34`) use the shared 20-slot entity table at `0x80143fc8`, 116 bytes per
slot:

| Offset | Field |
|---|---|
| `+0x0` | active |
| `+0x1` | SUBTYPE |
| `+0x2` | MODE |
| `+0x5` | TYPE |
| `+0x6` | particle index |
| `+0xb` | flag ("keeps living after phase 0") |
| `+0xc`/`+0x10` | X/Y velocity (`rand&0xfff`) |
| `+0x14` | Z velocity (`0xfff80000` = upward) |
| `+0x34`/`+0x38` | X/Y position, tiles in 16.16 fixed point |
| `+0x3e` | height, u16, encoded as height+`0x200` |

Dispatch for type `0x34` (`0x8019dfcc`) chains to a subtype table at `0x801c83e8`
(`[entity+1]`), then a per-subtype mode table at `0x801c8400` (`[entity+2]`):

| Subtype | Effect |
|---|---|
| 0 | leaf burst (mode 0 spawns 10 child particles in mode 2, plus PlaySfx `0x10c`) |
| 1 | gold ball (a "find") |
| 2 | worm (fishing bait) |
| 3 | find with textbox (copies 12 bytes to `0x801490d8`, checks flag `0x80165368`, PlaySfx `0x106`, sets `0x80143bb0=2` = textbox request) |
| 4 | bird (startled) |
| 5 | immediate kill ("nothing") |

Emote sprite graphics use the shared descLookup system (see Spawn records and sprite keys),
keyed as follows. `descLookup` also copies a byte pair from EXE table `0x80181950[key·2]` into
`ctx[0x70]`/`ctx[0x2b]`; for keys 24, 27, 70 and 530 that pair is `0`/`0`.

| Key | Content |
|---|---|
| 24 | emote family — prog 0="!", 1=sweat drop, 2="?", 3=squiggle/note, 4-7=sparkle, 8=gold-ball anim, 9-15=ball/bomb/worm cells (subtype 1 → prog 8, U184-192; subtype 2 → cells U248V0+U8V8 = prog 11/13) |
| 27 | leaves — 3 programs of 4-5 frames |
| 530 | leaf variant, only in the 9 areas listed at EXE table `0x801c8410`: 7, 8, 9, 10, 14, 17, 19, 29, 96 |
| 70 | bird — 8 programs: flight (8 frames), sitting, pecking, hopping |

The field CLUT formula is the counterpart to the battle formula's "row 496": row `480 +
decomposition(f2)`, decoded per `b6` mode. `b6=1` gives row `480+f2`, column 0; `b6=2` gives row
`480+(f2>>3)`, column `(f2&7)·32`. Proven against F8 dump quads from AREA000: key 24, f2=`0x12`,
b6=1 → (0,498); key 27, f2=`0x9e`, b6=2 → (192,499); key 70, f2=`0x8e`, b6=2 → (192,497). Texels
follow the enemy pipeline exactly, with no U-split: page = `b4·64` (row 0-255, AREA ct3 band), V
base = `b5` (vOrig).

Two handlers spawn emotes from the walkability grid, which therefore doubles as an
interaction-type table:

- **Interaction handler** `0x801cf940` (field action button) reads the look tile via
  `0x80166f64(x,y)` = `walk[y·cols+x]` (table pointer `0x8014931c`). walk `0xf0`/`0xf1`/`0xf4` =
  search spot → leaf burst, plus an additional startled bird at a 2/8 rate, plus find paths
  (ball/textbox, see Open).
- **Bush rustle** `0x801ced1c` checks the current tile plus 2 neighbors while walking. walk
  `0xf2` = BUSH → leaf burst, then `rand&0xf`: `0xd`/`0xe` = WORM, `0xf` = nothing. `0xf2` occurs
  in 50 areas (area 070, "Mt. Levett", alone has 25); `0xf0`/`0xf1` occur in 74 areas.

Earlier labels for these two handlers are corrected under Refuted approaches.

Bird ambience spawns 1-3 birds using the key-70 programs: pairs [0,1]=flight, [2,3]=sitting,
[4,5]=pecking, [6,7]=hopping, mirrored for left/right facing. Birds appear within 2 tiles of walk
`0xf0`-`0xf4` vegetation that is both walkable and renderable — walkable alone can still be
"theater backdrop" void. Birds flee using the flight program, direction away from the player and
upward, once the player closes to under 2.3 tiles. They despawn 2.4 s after fleeing and respawn
after 4-12 s, at least 5 tiles away. A bird corresponds to the subtype-4 "startled" emote,
confirmed against ground truth.

Search-spot interaction: pressing Enter with no zone or dialog open triggers the field action
button on the look tile (the last walk step, `lastStep`). walk `0xf0`/`0xf1`/`0xf4` → leaf burst,
plus an additional startled bird at a 2/8 rate (flight away from the player) — handler
`0x801cf940`. Find paths (gold ball/textbox, subtypes 1 and 3) remain open; their spawn rates and
conditions, including any item grant, are not yet disassembled.

### Extraction

| Script | Output | Notes |
|---|---|---|
| `extract/build-emotes.ts` | `public/emotes/{emotes,leaves,leavesAlt,bird}/` | frames, ticks, cell UVs, exact CLUT |
| `extract/build-objanchors.ts` | `public/objanchors/areaNNN.json` | object anchor table |
| `extract/scena-npc-scripts.ts` | `public/npcscripts/areaNNN.json` (+ index) | NPC movement/cutscene VM scripts |
| `extract/scena.ts` | `public/spawns.json` | NPC spawn coordinates |

Runtime side: `src/render/emotes.ts` implements `EmoteFX` (10-particle burst, original spawn
rates 2/16 for worms, program ticks at 60/s) and `BirdFX`. `main.ts` hooks tile changes on walk
`0xf2` plus an ambient toggle; `grid.walkAt()` supplies tile lookups. Debug entry points:
`__bof3.emoteParts`, `__bof3.birds`, `__bof3.birdPos`. Subtypes without pad input were dumped
directly by writing entity slots by hand (`scratchpad/emote-poke.ts`).

Verification (Playwright): on AREA070, 10 particles spawn and drift away, and birds spawn near
bushes, flee, and respawn. On AREA000, spot (r3,c31): Enter produces 10 particles, and the spot
tile itself is blocked, confirming look-tile semantics.

Tooling caveats for reproducing these captures on DuckStation: Z0 breakpoints fire only under
`ExecutionMode = Interpreter` (`settings.ini` `[CPU]`) — the recompiler swallows them silently,
no error, `Z0` replies `OK`, but the breakpoint never fires. GDB connect pauses emulation; call
`continueRun()` before doing anything else, or `interrupt()` times out and `?` returns `S02`.
`hidkey-pid` pad taps reach only the battle menu and hotkeys, never the field — field-side
triggers need either a direct entity write or a forced call to the spawn helper. DuckStation's
GDB stub supports neither `G` nor `P` (register writes), so forced calls require a code patch.

### Refuted approaches

- **"Mesh-group furniture has no static placement (runtime anchor)."** Refuted: 100% of object
  anchor table entries sit on walk=`0x50` tiles, confirming static placement after all.
- **`0x801cd4d0` as a world-level transition table.** Superseded: it is the camp/random-encounter
  table (see Fixed entities). Regular world navigation runs through the per-area warp table
  instead.
- **`0x801cef14`/`0x801ced1c` as fidget-linked "emote spawners."** Superseded: full disassembly
  identifies them as the search-spot interaction handler and bush-rustle handler instead (see
  Particles and emotes). Idle fidgets spawn no emotes; the earlier reading traced to Z0
  breakpoints that silently failed to fire under the recompiler.

### Open

- NPC movement VM: roughly 40% of opcodes have confirmed length and target address but
  undisassembled kernel-function effect — the `0xb*` (system/BGM/camera), `0xd4`-`0xd6` (object
  sliding), and `0xf8` families. The AUX assignment path is unresolved; SEQ PROG byte count is
  runtime-determined (party count).
- Emote find paths: gold-ball and textbox spawns (subtypes 1 and 3) have known trigger tiles but
  undisassembled spawn rates and conditions, including whether they grant an item.
- Field CLUT formula: only the `b6=1` and `b6=2` decoding modes are documented; other `b6` values
  are unconfirmed.

