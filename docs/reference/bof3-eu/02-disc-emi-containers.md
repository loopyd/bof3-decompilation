> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 2. The disc: sectors, EMI containers and content types

### Disc layout

The disc's data track is `MODE2/2352`, wrapped in a standard ISO 9660 filesystem; game files sit
under `/BIN/`. A full signature audit classified all 889 files (367 MB total) into known systems
(`scratchpad/disc-audit.ts`, finished off by `scratchpad/classify-unknown.ts` for the remainder) —
practically everything on the disc resolves to a known container or stream type.

Directory groups by family: `/BIN/BENEMY` (200 enemy sound banks, files `ENEMY###.EMI`),
`/BIN/BPLCHAR` (121 battle party-member files, `BPLD###.EMI`), `/BIN/BOSS` (40 boss files,
`BOSS###.EMI`), `/BIN/BATTLE/BATL_DRA` (dragon battle sprites), `/BIN/PLCHAR` (38 field-character
files, `PL###.EMI` — 2D sprite stacks despite the directory name, not 3D models).

No file is addressed by name at runtime; everything goes through indirection tables baked into
the executable:

- A global **file LBA table** at `0x80182910` maps a `fileId` to its disc sector (logical block
  address). `fileId` 209 is `BGM000.EMI`; further BGM files follow ascending by LBA, which is
  essentially random relative to their BGM number.
- A separate 165-slot table at `0x80182384` maps a `bgmId` — the number the engine actually
  requests — to `[u16 fileId][?][u8 block]`. ⚠ `bgmId` is not a file number, and it is not the
  same index space as the per-area BGM-slot table (`0x801ca7d7`), whose values are ISO file
  indices rather than `bgmId`s. Three different numbers can all look like "which BGM" while
  naming different things.

Streamed audio bypasses EMI containers entirely and uses raw interleaved XA sectors instead: a
small **file-slot table** at `0x80183704` indexes into the same global LBA table for three
streams — slot 0 `S_XA00.STR` (8x interleave), slot 1 `MAGIC00.STR`, slot 2 `VOICE.STR` (16x) — at
LBA-table entries 434/683/684. Each slot has its own **track table** at `0x80183710[slot]` (`u16`
entries; bit `0x8000` = switch XA channel, the low bits = start offset in interleave units;
`tab[track]` = start, `tab[track+1]` = end). Clip length = `offset · interleave / 150` seconds,
verified exactly against five extracted `VOICE` clips (entries 71/63/61/60/42 → 7.57/6.72/6.51/
6.40/4.48 s). Track counts: `S_XA00` 15, `MAGIC00` 896, `VOICE` 5. `LOGO.EXE` is a bare PSX
executable, not an EMI container; it plays `CAPCOM30.STR` directly as the boot logo.

### EMI container format

Each EMI's own table of contents is an array of `[len][target][id]` entries, one per subfile:
byte length, target RAM load address, and a resource id. The loader keeps a RAM-resident copy of
the currently loaded file's TOC — for `AREA###.EMI`, the "area load table" at `0x800e48d0`. This
is why subfile payloads land at addresses that look hardcoded per role (battle stub, CLUT,
choreography, …): the target address travels with the file, not with the code that reads it.

Subfiles are typed by content (`ct0`…`ct8`, plus at least one higher type, `ct10` — next section)
and, within some content types, indexed by a further container of sub-entries. The clearest
example is the shared furniture/enemy container at `0x800d3800` inside every `AREA###.EMI`:
entries are addressed as `ptr[i] = u32[i·4]` — raw index `i`, no adjustment — where `u32[0]`
doubles as both `count·4` and the byte offset of entry 0, because entries sit directly after the
pointer table. ⚠ A widely used reader instead computed `ents[i]=u32[4+i·4]` and subtracted one
from the index; the two off-by-one errors canceled for every populated entry except index 0,
which was silently dropped (missing Garr's boss figure). The corrected reader indexes raw. Entry
granularity does not always match one enemy identity either: `ENEMY019`'s audio and stats cover
two visually distinct creatures, Ripper and EyeGoo, told apart only by their container entry's
`(base,vOrig)` pair.

A page/CLUT descriptor is a fixed 8-byte record, `[id:u16][f2:u16][base:u8][vOrig:u8][b6][b7]`,
read by a shared lookup routine at `0x8014de8c` (`ctx[0x25]`=base, `[0x26]`=vOrig, `[0x27]`=f2.lo
= CLUT, `[0x28]`=uSplit, `[0x2c]`=b7). Boss battle choreography (`ct0` @`0x800c1800`) calls this
same routine with a constant key in `a0` to reach the *host area's* descriptor table and, through
it, the area's `0x800d3800` container. Ten of forty bosses resolve this way — e.g. `BOSS022` key
`0x153`/`0xaa` = Garr, host `AREA080`; `BOSS013` key `0xee`/`0xef`/`0xf9` = Balio/Sunder,
`AREA011`/`041`; `BOSS055` key `0x30c`/`0x30d`/`0x318` = Myria's parts, `AREA198`. The host area's
descriptor table stays resident for the whole boss battle (`[0x801459f4]=0x800e3804`); bosses
draw through exactly the area's regular enemy pipeline, never their own file. The remaining
thirty bosses (Nue among them) instead resolve through a runtime object-list path (`0x8014d82c`,
`anim&0x80` → `0x80182000`/`0x80182148`), found statically by enumerating descriptor entries that
no regular enemy record references ("orphan" descriptors), filtered by a frequency blacklist
(entries reused across more than 15 areas are generic table noise, not orphans).

A disc-wide **resource id** space is shared by several `ct0` block headers (`[id:u32]` at the
start of the block): `0xfc`–`0x103` (eight 3D object-mesh blocks, one per object area), `0x104`
(a code overlay in `AREA049`, occupying the slot a mesh would use), `0x119`–`0x11d` (`SCE10EFF`/
`SCE15EF0-3` particle overlays), `0x121` (the `PLP` behavior-overlay family — id `0x21`?, not
confirmed), `0x1c1` (`RTEST`/`MTEST`).

Not every subfile is pure data. The per-area subfile at `0x801f2c00` ("nav addendum") holds both
object-placement records and per-area MIPS code: a GTE transform call (`0x801791c4`) returns into
the middle of that subfile's own code (`0x801f61cc`), and a per-area state vtable sits at subfile
offset `+0x3c70`. This is why the object-placement record format is not generic across areas —
each area's copy of the subfile can carry its own placement layout alongside its own draw and
behavior routines. `AREA030`/`089`/`129` similarly share one byte-identical 8784-byte
"special-mechanic" module whose own format has not been determined. The eight object-mesh areas
(`AREA067`/`077`/`104`/`108`/`121`/`135`/`145`/`173`) and the mesh block format itself belong to
the features/meshes chapter; the container mechanics above are what locates and places them.

### Content types

| Type | Holds | Appears in |
|---|---|---|
| `ct0` | Resident MIPS code and/or fixed-layout data, loaded to the address given in its own TOC entry | battle stubs and choreography, CLUTs, descriptor and mesh-group containers, resident game-data tables, code/particle overlays |
| `ct1` | Self-contained sprite package: texel bands + CLUT + animation programs + dispatch table | `PL###.EMI` field characters (38), `BPLD###.EMI` battle party sprites (45 compressed) |
| `ct2` | Not attested in this material | — |
| `ct3` | Finished, ready-to-upload VRAM page | shared battle stub (32 KB); per-area encounter-enemy band (256 KB); `BATL_OVR` game-over screen |
| `ct4` | Not attested in this material | — |
| `ct5` | Not attested in this material | — |
| `ct6` | VAB header ("pBAV"): tone/instrument attributes + a VAG size table | every VAB pair, SFX and BGM alike (detail: audio chapter) |
| `ct7` | VAB body: raw sample data — audio only, in every attested case | `ENEMY###`/`BPLD`, `FIRST`, `START`, boss/battle EMIs |
| `ct8` | Cue table pairing a "cue" number with a `ct6`/`ct7` VAB | `PL###`, `AREA###`, `BPLD###`, `ENEMY###`, system/battle SFX sets |
| `ct10` | SEQ song data (`pQES`, up to 4 independent track blocks) | boss own-BGM trios, `BGM###.EMI`/`BGMBAT##.EMI` |

`ct2`, `ct4`, `ct5` never turned up while classifying the 889 files or auditing subfile roles; the
gap is left visible rather than guessed at.

**`ct0` address map** — fixed targets seen across the disc; each still arrives through its own
file's TOC entry (see EMI container format):

- `0x8001a000` — shared text/description block (`FIRST` subfile 11; reused by `BATTLE`/`BOSS`).
- `0x800f0800` (3692 B) — shared boss/battle stub, byte-identical across all 40 `BOSS###.EMI` and
  in `BATTLE.EMI`.
- `0x800c1800` (1.5–4 KB, per boss) — boss battle choreography, compiled MIPS in BMAGIC style
  (`u32[0]` = global skill id, range `0x11e`–`0x140`, the same id space as player spells).
- `0x800c3800` + `0x8003b800` — VFX/sprite frame-series tables (`BMAGIC`/`KAIZAR`/`BPLD`/`CRYU`).
- `0x8005680c`–`0x8007a80c` (variable) — raw container of the dragon battle sprites.
- `0x80034000`/`0x80035200`/`0x80035400`/`0x80035700` — CLUT blocks for `DEMO` attract mode,
  `SCENA17`, `COMMU02`.
- `0x80036e00` — shared boss/battle CLUT, byte-identical across all 40 `BOSS###.EMI`.
- `0x80117000` — 3D object-mesh blocks, eight areas only (registry ids `0xfc`–`0x103`; block
  format: features/meshes chapter).
- `0x800f5000` (8784 B) — special-mechanics module shared byte-identical by `AREA030`/`089`/
  `129`; format undetermined.
- `0x800d3800` (per `AREA###.EMI`) — the mesh-group container: furniture placements and enemy
  sprite composites in one indexed table.
- `0x800e3800` (per `AREA###.EMI`) — the area's own descriptor table (8-byte page/CLUT records).
- `0x800e4000` (per `AREA###.EMI`) — 72-byte encounter header + up to eight 136-byte enemy stat
  records.
- `0x80195a00` (`GAME.EMI` subfile 0) — resident core tables (party growth, item/weapon/armor/
  accessory records, skill and magic names), copied onward to `0x801c8000+`.
- `0x801d0c00` — cutscene-effect overlays (`SCE10EFF`/`SCE15EF0-3`).
- `0x801eec00` — `BATL_END` battle-end choreography.
- `0x801f2c00` (per object area) — placement records mixed with per-area behavior/render code.

`ct6`/`ct7`/`ct8`/`ct10` always travel together as a VAB(+SEQ) group inside their owning EMI; the
byte-level cue and tone layout is audio-chapter territory, not container structure, and is left
there to avoid stating a cue-format detail this source does not have the final word on.

### Compression

Graphics subfiles carry one of three encodings, chosen by a `u32 mode` field in a shared stream
header: `[u16 w][u16 h][u32 mode][u32 outSize=w·h·2][stream @ +12]`, read by a dispatcher at
`0x8014e820`.

- **mode 2 = LZSS** — handler `0x8014ea4c`, 512-byte scratchpad ring buffer, control byte pattern
  `|0xff00`; a literal/match token decodes as `off = lo | ((hi&0xf0)<<4)`, `len = (hi&0xf)+3`.
- **mode 1** — a 5-bit unpacker, handler `0x8014e948`.
- anything else — raw, uncompressed.

Bit-exact verified against the game's own decoded staging buffers (4096/4096 and 6144/6144 bytes)
for field and area graphics. The dispatcher is table-driven (count at `0x80145a00`, width table
`0x80145a54`, height table `0x80145a7c`, pointer table `0x80145a04`) and normally runs once at
boot, for system/font graphics. The same routine also drains a per-frame queue during gameplay,
built by an enqueue call at `0x8014dd68` (`w = a2[0x25]·64`, `h = a2[0x26]·64`, `ptr = a1 +
a1[0xc]`, mode at `ptr+4`, stream at `ptr+8`) — the queue's entity-side (`a1`/`a2`) layout is
still unresolved (see Open).

⚠ This codec was first filed under the label "`ctype7`" (tool name: `extract/ctype7.ts`). That
label is now known to be wrong: `ct7` holds audio in every attested case (see Content types and
Refuted approaches). The compressed graphics this dispatcher decodes belong to `ct1` sprite
packages instead — field characters, and, via the same LZSS-plus-5-bit-plus-de-interlace path,
the 45 battle party-member sprites. Exact bit-level layout: the sprite-system chapter.

Not all pixel content is compressed. Enemy sprite pixels in the per-area `ct3` band sit raw and
uncompressed, a straight VRAM-stripe upload, as does `WARNING.EMI`'s full-screen frame (see File
inventory).

### File inventory

**Battle system**

- **`BOSS###.EMI`** (40) — battle choreography only (`ct0` @`0x800c1800`) plus the shared stub
  content described above; contains no unique sprite graphics for most bosses (see EMI container
  format). Some bosses additionally carry an own battle-BGM VAB trio (`ct6`+`ct10`+`ct7` =
  VH/SEQ/VB) and/or an own sound VAB, identified by `TOC-addr==6`.
- **`BATTLE.EMI`** — the shared battle-common graphics/VAB/CLUT triad also found byte-identical
  in every `BOSS###.EMI`; its own `ct8` battle cue set runs 28 bytes per record.
- **`BATTLE2.EMI`** — code at subfile 15 (`0x80096800`), data at subfile 3 (`0x801d0c00`);
  battle-overlay logic.
- **`BATL_END.EMI`** — battle-end choreography (`ct0` @`0x801eec00`) + a system VAB.
- **`BATL_OVR.EMI`** — `ct3` + CLUT: the battle "GAME OVER" screen, a fully static image.
- **`BATL_DRA.EMI`** — dragon battle sprites, plus the dragon-gene/battle UI icons (OK button,
  gene symbols).
- **`BATL_RE2.EMI`/`RET.EMI`** — flee/retreat graphics + a battle-common VAB (3104/21280 bytes,
  same size class as `BATL_SE`).
- **`BATL_SE`/`COMN_SE`** — the system SFX set shared by battle (bank 1, 44-byte `ct8` records,
  identical across `COMN_SE`/`BATTLE`/`BOSS*`/`BATL_END`).
- **`ENEMY###.EMI`** (200, `/BIN/BENEMY`) — audio only (`ct6`/`ct7`/`ct8` VAB trio + cue table).
  Formerly assumed to also hold graphics; the corrected reading is audio-only, with stats
  (`0x800e4000`) and geometry (`0x800d3800`) both living in the host `AREA###.EMI` instead.
- **`BPLD###.EMI`** (121, `/BIN/BPLCHAR`) — battle party-member sprites: `ct1` pixel package
  (LZSS + 5-bit + de-interlace) + a per-slot `ct8` battle cue table (banks 3–5) + `ct6`/`ct7`
  audio.

**Areas**

- **`AREA###.EMI`** (200) — per-region bundle: map textures, the region's encounter-enemy `ct3`
  band (up to 8 enemies side by side), enemy stat/encounter records (`0x800e4000`), the
  furniture-plus-enemy mesh-group container (`0x800d3800`), the area's own descriptor table
  (`0x800e3800`), and, in eight areas, 3D object-mesh blocks (`0x80117000`) plus a placement/
  behavior subfile (`0x801f2c00`). 106 of the 200 areas carry enemy encounter data, covering 168
  distinct enemies. `AREA049` uses the object-mesh registry slot for a code overlay instead of a
  mesh; `AREA030`/`089`/`129` share the `0x800f5000` module.

**Field characters**

- **`PL###.EMI`** (38, `/BIN/PLCHAR`) — field-character sprites, each a self-contained `ct1`
  package (2D sprite stacks, not 3D models, despite the directory name).
- **`PLP###` family** — field-character behavior overlays (registry id `0x121`, unconfirmed).

**Music and streams**

- **`GAME.EMI`** — subfile 0 (`ct0`) holds the resident core tables: party growth, item/keyitem/
  weapon/armor/accessory records, skill and magic names; loaded at `0x80195a00`, copied onward to
  `0x801c8000+`.
- **`BGM###.EMI`/`BGMBAT##.EMI`** — each a VAB+SEQ trio (`ct6`/`ct10`/`ct7`), reached through the
  `bgmId`→`fileId`→LBA chain above. Bosses match their own battle-BGM trio byte-exact against the
  `BGMBAT` family (35/35 matched): `BGMBAT04` is the majority default; `BGMBAT01` serves
  `BOSS030`/`032`/`035`/`037`/`051`; `BGMBAT03` serves `008`/`023`; `BGMBAT05` serves `052`;
  `BGMBAT06` serves `055` (Myria); `BGMBAT00` serves the regular random-battle case plus
  `007`/`014`/`015`/`033` (mini-bosses); `BGMBAT02` matches no `BOSS###.EMI` (open). `BGMBAT00`
  loads like any other bank rather than staying permanently resident — see Refuted approaches.
  Full VAB/SEQ layout, tone/pitch resolution, and looping: audio chapter.
- **`S_XA00.STR`/`MAGIC00.STR`/`VOICE.STR`** — raw interleaved XA streams outside the EMI/
  content-type system; addressing under Disc layout.

**System and menus**

- **`FIRST.EMI`** — origin of the shared text/description block convention (subfile 11,
  `0x8001a000`), reused by `BATTLE`/`BOSS`.
- **`AFLDKWA.EMI`** (16.8 KB) — a resident data block in `FIRST`-style layout (`0x8001a000`);
  content not identified (open).
- **`BATE.EMI`** (174 KB) — the fishing-minigame overlay: system code overlay + 2 graphics bands
  + CLUTs.
- **`DEMO.EMI`** (946 KB) — the attract-mode EMI: own BGM (VAB+SEQ+419 KB VB) + 4 graphics bands
  (up to 256 KB each); exact per-band geometry open.
- **`WARNING.EMI`** — `ct0` is one raw, uncompressed 15-bpp full-screen frame (320×240×2 bytes),
  not a sprite package (see Refuted approaches).
- **`LOGO.EXE`** — a bare PSX executable, not an EMI container; plays `CAPCOM30.STR` directly.
- **`SCE10EFF.EMI`/`SCE15EF0-3.EMI`** (SCENARIO family) — small code overlays (`0x801d0c00`):
  per-cutscene particle-effect programs for the ending sequence (registry ids `0x119`–`0x11d`);
  `SCE15EF1` additionally issues 7 SFX calls.
- **`RTEST.EMI`/`MTEST.EMI`** — an opaque blob (registry id `0x1c1`); byte statistics spread
  across the full 0–255 range with `0x00`/`0xff` clusters suggesting bitmasks. Not interpreted.
- **`SHISU.EMI`** — the field-side "you died" screen; still dump-bound, unlike its battle-side
  counterpart `BATL_OVR`.


### Disc inventory: what every container holds

The disc holds 889 files, checked against extractor coverage.

Closed cases — fully explained, nothing left to extract:

| Container(s) | Finding |
|---|---|
| `BGMOPN`, `BGMEND`, `BGMSPC`, `BGMBAT`, `BGM-A`/`B` | Matched by `build-bgm-all`'s pattern `^BGM.*\.EMI$`. Already extracted to `public/bgm/`, including opening, ending, and special tracks. |
| `ENDKANJI.EMI` | Byte-identical copy of the FIRST font bands (`0x1e000200`/`0x1e080200`). A VRAM-reload package for the ending; no new content. |
| `BATL_RET.EMI` / `BATL_RE2.EMI` | Post-battle reload packages: font band copy, pBAV sound, and an identical battle-transition particle track (stars/clouds/swirl, 4bpp) for target page `0x1a` and `0x1c` respectively. CLUT splitter at `0x80036e00`/`0x800357e0`. The particle palette maps to battle context; the exact mapping is still open, noted as an asset candidate. |
| `BATTLE2.EMI` | Byte-identical duplicate of `BATTLE.EMI` (seek optimization). |
| `SISYOU.EMI` (masters screen) | Graphics/CLUT bands byte-identical to `SHISU`. Its only unique content is the UI code overlay sub0 at `0x801d0c00`, with `%3d` format strings. |

Mapped, still open:

| Container(s) | Finding |
|---|---|
| `RTEST.EMI` / `MTEST.EMI` | Debug leftovers. Identical 2404-byte data blob at `0x801d0c00` (byte-pair sequences, format unknown). `RTEST` carries an extra u32 at `0x80195a00` = `0x1c0`. A TCRF curiosity. |
| `SCE10EFF.EMI` / `SCE15EF0`–`3.EMI` | Cutscene effect code overlays at `0x801d0c00` (header `[id:u32][entry/code]`, ids `0x119`, `0x11a`–`0x11d`) for SCENA10/15. 15 is the ending-sequence family. |
| `PLP###.EMI` (18 files + `PLP27A`) | Per-field-character code overlays at `0x801ce400` (header `[0x21][MIPS code]`, 6–8 KB). Presumably character-specific field behavior. Disassembly still open. |
| `LOGO.EXE` | Standalone PS-X EXE, 120 KB. Boot-logo program. |
### Refuted approaches

- **`ct7` as a pixel/sprite codec.** A dispatcher hunt (LZSS/5-bit modes, dispatcher
  `0x8014e820`) was run against `ct7` data expecting compressed sprite pixels; the data was audio
  the whole time. The VAG size table in the paired `ct6` sums exactly to the `ct7` length in
  every checked case (`ENEMY019`: 10 monster sound VAGs; `BPLD034`: party audio, VAG table at
  `ct6+0xe20`), which closed the question. The "predictive `[count][0][14]` archive codec"
  hypothesis never applied to real data.
- **"Bosses = `BOSS###.EMI` sub2, `ct0`@`0x800f0800`."** That `ct0`, its `ct3`, and the CLUT at
  `0x80036e00` are shared stub content, byte-identical across all 40 `BOSS###.EMI` and in
  `BATTLE.EMI`. The boss-specific content is the choreography at `0x800c1800`; graphics come from
  the host area via `descLookup` for ten of forty bosses and from a runtime object-list path for
  the rest — never from the boss file itself.
- **Container reader `ents[i]=u32[4+i·4]` with `b7−1`.** Two off-by-one errors that canceled for
  every populated entry except index 0, which was silently dropped (missing Garr's figure). The
  raw `ptr[i]=u32[i·4]` reader is correct.
- **"`0x800d3800` holds furniture only, not enemies."** The container holds both; an earlier pass
  had only inspected the furniture entries.
- **"`AREA054` pixels above VRAM y256 don't decode."** A render bug — wrong band geometry, not a
  missing format. VRAM y256+ is simply the map-texture window of the area's second `ct3` band.
- **`WARNING.EMI`'s `ct0` as a sprite package.** It is a raw 15-bpp full frame, not a
  `[vc][cells]` program.
- **"`BGMBAT00` is always resident."** Built on a fingerprint match against only the first
  kilobyte of a savestate; the field-mode match was a load-buffer coincidence. A full-length
  comparison (19893 bytes) matches only inside an actual battle savestate — the bank loads like
  any other, on demand.
- **SEQ identification by a 256-byte prefix.** Twelve name groups share their first 256 bytes on
  disc (e.g. `BGM002`~`BGM043`, `BGM058`~`BGM076`~`BGM101`~`BGM133`~`BGM188`) — a prefix match
  reports the wrong file. Only `BGMBAT00`/`BGMBAT02` are fully byte-identical; every other case
  needs a full-buffer compare.
- **A `pQES` reader that assumed a single stream from offset `0x0f`.** A `pQES` resource carries
  up to four independently headed track blocks (see Content types); the single-stream assumption
  silently truncated three of every four tracks.
- **Conflating `bgmId`, `fileId`, and the ISO file index.** Three different numbers for "which
  BGM": the per-area slot table (`0x801ca7d7`) stores ISO file indices, not `bgmId`s; `bgmId`
  indexes the 165-slot table at `0x80182384`, whose `fileId` field is a further index into the
  LBA table at `0x80182910`.

### Open

- No confirmed on-disc table maps an enemy to its pixel slice inside the per-area `ct3` band;
  slices are matched empirically (occupancy scan against ground truth), not via a lookup table.
  `ENEMY###.EMI`'s `ct8` may carry the target VRAM page directly — untested, because the
  area-to-`ENEMY`-file mapping needed to check it is itself missing.
- Enemy sprite slice width is fixed at 128 texels; whether a boss-size sprite over 128 px spans
  two slots, and whether allocation is really 256-texel-block granular, is unconfirmed.
- The object-placement/segment-list format at `0x80117000`/`0x801f2c00` is not generic: the three
  areas checked (`067`, `077`, `121`) each show a different record shape. Per-area renderer
  disassembly would be needed to read it generically.
- The per-area behavior code inside `0x801f2c00` is disassembled only as far as its state vtable
  (`subfile+0x3c70`); the handlers behind it are unread.
- The entity-side (`a1`/`a2`) structure that feeds the per-frame compressed-graphics queue
  (enqueue `0x8014dd68`) is not resolved statically — this is what still ties some
  compressed-graphics content to runtime capture instead of static extraction.
- `AFLDKWA.EMI`'s 16.8 KB resident block is unidentified.
- `DEMO.EMI`'s four `ct3` graphics bands (up to 256 KB each) have unconfirmed geometry.
- `RTEST.EMI`/`MTEST.EMI` remain an uninterpreted blob.
- The `0x800f5000` module shared by `AREA030`/`089`/`129` (8784 B) has no determined format.
- The `PLP` family's registry id (`0x121`, tentatively `0x21`) is unconfirmed.
- `BGMBAT02` matches no `BOSS###.EMI`; a candidate arena/special-mode use is unverified.
- Name assignment for the thirty bosses on the runtime object-list graphics path remains
  heuristic (frequency-blacklist orphan-descriptor enumeration), not code-traced.

