> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 1. The engine at a glance

BoF3 is a small set of table-driven subsystems. There is no general scene graph and no scripting
language in the modern sense: the game holds tables, and fixed code walks them.

### Code layout in memory

All game code sits as **overlays on the disc**, loaded to fixed addresses depending on the mode.

| Overlay | Loaded at | Contains |
|---|---|---|
| `GAME.EMI` ct0 | `0x80195a00` (229 KB) | the field engine: field state machine `0x80197178`, SCENA API helpers (`0x8019fc28`, `0x801c00b8`), the entity stepper in the `0x8019xxxx`/`0x801bxxxx` band |
| `STATUS`, `SHOP`, `SISYOU`, `COMMU`, `BATE`, `SHISU`, `LOAD`, `BATTLE` … | `0x801d0c00` | the system modes: main menu, shop, masters, fairy village, fishing, game over, battle |
| per-area init overlay | `0x801f2c00` | the area's warp table and per-area setup code |
| `SCENA##.EMI` | varies | compiled MIPS cutscene scripts |

⚠ The system modes share one address. An address in the `0x801d0c00` band is meaningless without
naming the overlay it belongs to — the same bytes are fishing code in one moment and shop code in
the next. A savestate taken outside that mode shows only the leftover data zone, which has caused
at least one wrong conclusion ("the code is not there").

### From disc to screen

```
disc (MODE2/2352)
  └─ ISO 9660 ─ /BIN/…            chapter 2
       └─ EMI container ─ ct0…ct8  chapter 2
            ├─ tile + wall words   chapters 3, 4
            ├─ features, meshes    chapter 5
            ├─ sprite programs     chapters 7, 9, 10
            ├─ VAB / SEQ audio     chapter 13
            └─ SCENA scripts       chapter 12
```

Everything the reconstruction needs is static: it can be read from the disc without running the
game. The emulator is used to *check* results, not to obtain them — with two exceptions noted in
chapter 18, where a value only exists at runtime.

### Invariants

These hold across the whole game. They were each established the hard way, and several contradict
what seems obvious.

| Invariant | Consequence |
|---|---|
| Every texel of an area lives in that area's own EMI | No shared texture base to hunt for. The single exception is the overworld ground tileset, which is VRAM-resident and seeded from two donor areas. |
| The CLUT formula is universal (routine `0x801557d4`) | VRAM column = `page & 3`, bit 31 = 4bpp, CLUT row = 483 + palette. No per-area lookup tables exist. |
| Tile tops use the full texture-word system | A top can be a nibble, a rect or a pair. Check the mode before interpreting a cell, or mountains and gates decode as garbage. |
| The nav map is the exact walkability | Codes: `0x10` blocked, `0x40` overworld floor, `0x50` furniture, `0x60` stairs, `0x70` ladder, `0xa_`/`0xc_` warp. |
| Stairs are ordinary slope quads | Tread and riser are baked into the rect-top textures. A stair-specific renderer is wrong; flat-looking stairs in the texture are not a bug. |
| Map corner heights are stored mod 256 | The byte is `h mod 256`; the engine works window-relative, so wraps cancel out. Continuous mountain tiers climb past 255. |
| `ct7` is audio | It is a pBAV VAB bank, never a graphics codec — a theory that cost a lot of time before it was disproved. |
| All sprites are `[vc][cells]` sprite programs | Field characters, battle actors, furniture and spells share one 2D model. Nothing here is a 3D mesh. |
| Sprite frames hang on their anchor | `Xs/Ys` are relative to the sprite anchor, not to a bounding box. Using the box makes characters jump when they turn. |
| Boss EMIs contain no graphics | They hold battle choreography and sound. The artwork sits in the host area's EMI. |
| The SEQ program number is a ProgAtr slot | Not the packed tone-block index. Reading it as the index gives 7 % of all notes a foreign instrument. |

