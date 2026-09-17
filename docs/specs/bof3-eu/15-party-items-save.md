> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 15. Party data, items, menus and the save format

### The party record

Party member data lives in 8 fixed-size records of 164 bytes each at RAM `0x80144968`
(`partyRecords[8]×164` in the save file, data offset `0x090`). A parallel 320-byte
battle-actor mirror exists at `0x80145f04 + idx·320` for in-battle bookkeeping, addressed
by the same ability-learning routine as the field record. ⚠ Not all 164 bytes are mapped;
the table below lists only fields verified against live RAM or ground truth (GT).

| Offset | Field | Detail |
|---|---|---|
| `+0x07` bit0 | in-party flag | gates `EQUIP_FIND` (`0x80166540`) when it counts equipped items |
| `+0x07` bit1 | leader flag (?) | unverified |
| `+0x0e` | weapon index | index into the weapon table `0x801c9360` (24 entries) |
| `+0x0f` | shield index | index into the armor table `0x801c9b28` (22 entries, subtype 2) |
| `+0x10` | helmet index | armor table, subtype 3 |
| `+0x11` | body-armor index | armor table, subtype 4 |
| `+0x12` | accessory slot 1 index | index into the accessory table `0x801ca100` (20 entries) |
| `+0x13` | accessory slot 2 index | accessory table |
| `+0x1c` | effective stats (`eff`) | equip/status deltas baked in while active; `eff ≠ base` persists in the save |
| `+0x3c` | base stats | level-up growth and master deltas add onto this |
| `+0x5c..+0x65` | healing magic list | 10 × 1-byte engine ability id, `0` = free slot |
| `+0x66..+0x6f` | assist magic list | 10 slots |
| `+0x70..+0x79` | attack magic list | 10 slots |
| `+0x7a..+0x83` | transferable skills list | 10 slots |
| `+0x84` | master level snapshot | set to current level when joining a master, zeroed on leaving |
| `+0x85..+0x8a` | `permStatBonus` | master stat delta, 6 × signed i8 `[HP,AP,Pwr,Def,Agl,Int]` |

Index `0` in the weapon/armor/accessory tables is the empty "Nothing" entry.

Learning a skill into one of the four 10-slot lists goes through a shared routine.
`ABILITY_LEARN` (`0x80165490`, args `id, charIdx, toBank, isBattle`) resolves the target
record through `ARRAY_BASE` (`0x80166e48`) — field record `0x80144968 + idx·164` or battle
actor `0x80145f04 + idx·320` — then reads the skill's own record byte at `+0x0f` and does
`switch(typeByte & 3)` to pick the bank. Insertion uses the first free (`0`) slot; capacity
is fixed at 10. When `toBank ≠ 0`, the skill instead goes to a global skill-notes bank (128
slots at `0x8014546c`) instead of one of the four per-character lists. Callers: the growth
writer (`0x801ae3f4`/`0x801ae41c`, reading skill-grant bytes `+6`/`+7` of the level record)
on level-up, battle code (`0x800aa9bc`/`0x800aa9e0`), and the STATUS screen
(`0x801d1534`/`0x801d8298`).

A separate 4-pass modifier pipeline at `0x80164cf8` (calling `0x80165a14`, `0x80165b90`,
`0x80165df0`, `0x80164b54`) applies STATUS-effect stat changes on top of the equip deltas
already baked into `+0x1c`.

Verification examples, matched against live RAM and canon: Ryu's heal list is `[70]`
(Heal) with a Dagger equipped; Momo's heal list is `[70,75]` (Heal, Purify), assist list
`[60,68,82,87]`; Nina's attack list is `[100,94,97]` (Cyclone/Frost/Jolt) with an Oaken
Staff; Rei carries a BallockKnife and the Pilfer ability.
`public/gamedata/party-record.json` stores `equipment` and `abilities` per member
alongside a `liveExample` RAM snapshot; the party UI module renders them as equipment and
ability lists.

### Level, growth and skills

The skill/magic definition table (`GAME.EMI sub[0]`, engine table base `0x801ca98e`, 20-byte
records) backs every ability consumer: master curricula, level-up grants, the four
per-character ability lists, and enemy AI skill slots.

| Offset | Field |
|---|---|
| `+0x00` | id |
| `+0x01` | icon byte |
| `+0x02..+0x0d` | name (font-encoded, max 12 characters) |
| `+0x0e/+0x0f` | flags (low 2 bits of `+0x0f` select the ability-list bank on learn; `3` = transferable/teach copy) |
| `+0x10` | AP cost |
| `+0x11` | power |
| `+0x12` | element bitmask |
| `+0x13` | type |

Element bits: `0x01` fire, `0x02` ice, `0x04` lightning, `0x08` earth, `0x10` wind, `0x20`
holy; Simoon/Sirocco = `0x11` (fire+wind), matching canon. AP costs confirm the layout:
Heal 4, Rejuvenate 7, Restore 12, Vitalize 20, Flare/Frost 2, Inferno/Blizzard 10,
Lightning/Typhoon 7, Sirocco 12, Simoon 4, Kyrie 5, physical skills 0. Power scales
consistently (Flare 10 < Inferno 40; Heal 20 < Restore 120).

Each spell sits twice in the table: a base entry (Heal `id64`, flags byte `0x00`) and a
duplicate in the `163-207` range (Heal `id168`, flags byte `0x33`) that is the mage-master
teach/transferable-skill copy (`type&3==3`), not a separate id space. The table holds 227
engine entries; AI-only `id226` = "MeteorStrike" (used by Nue), `id72` = Restore.
`skills.json` keys on the corrected engine id (`build-chardata.ts`/`build-masters.ts` both
use it), 222 entries total.

**Master system.** `extract/build-masters.ts` writes `public/gamedata/masters.json` (17
masters, resolved with 0 modifier mismatches). Master names — Bunyan, Mygas, Yggdrasil,
D'lonzo, Fahl, Durandal, Giotto, Hondara, Emitai, Deis, Hachio, Bais, Lang, Lee, Wynn,
Ladon, Meryleep — come from `FIRST.EMI sub[11]` at `0x8001a000` (the resident text block
starting at `0x8001b3e8`, with title and location per master; this is the same container
as the general system text bank, see Menu assets and fonts).

The stat-modifier record:

| Field | Type |
|---|---|
| HP | signed i8 |
| AP | signed i8 |
| Pwr | signed i8 |
| Def | signed i8 |
| Agl | signed i8 |
| Int | signed i8 |

Location `SISYOU.EMI sub[0]@0x801d4154`, stride 6 bytes, one record per master (indexed by
`masterId · 6`). The apply routine `0x801d2e60` writes the record into party record
`+0x85..+0x8a`. The growth writer `0x801ae0a0` re-adds this delta on every level-up onto
the base stats (`+0x3c`) — the permanent channel through which a mastered discipline
affects growth. The reset routine `0x801d3500` zeroes `+0x84..+0x8a` on leaving a master;
`+0x84` is set to the current level as a snapshot on joining.

Verified example values: Bunyan HP+2/Pwr+2/Int−3; the Wyndia quadruplets
(Bais/Lang/Lee/Wynn) each +1 across every stat; Ladon HP−6/AP−6/Pwr+2/Def+2/Int+2;
Durandal is all-zero, the canonical joke master "?????".

The skill teach table sits at `0x801d4088`, stride 12 bytes = 6 × `[apprenticeLevel][skillId]`
pairs per master, sentinel `0x63ff` marks an empty slot. Skill ids were initially only 31
of 55 resolved, via the ability names at `0x801caa00`; after the engine id-space
correction above, every curriculum resolves with 0 unresolved entries — e.g. Mygas teaches
Frost/Meditation/Magic Ball/Typhoon, Deis teaches
Inferno/Blizzard/Myollnir/Sirocco/Celerity/Quake/Kyrie/Ragnarok/Foretell, both exactly
canonical.

**Level-up application (browser runtime).** `src/systems/partystate.ts` keeps live party
state in `localStorage` under key `bof3.party` via `loadParty()`/`saveParty()`/`resetParty()`;
without a saved state it starts from the disc's initial party. `grantExp(member, exp, chars)`
walks `level-growth.json → chars[].levels[]`, applying each level's `expToReach`, `growth`
(`hp`/`ap`/`pwr`/`def`/`agl`/`int`) and `skillGrant` one level at a time, not recomputed in
bulk, so multiple level-ups within one grant match the engine exactly. `applyBattleResult(
members, hpAp, exp)` writes back post-battle HP/AP and distributes EXP to all members,
including KO'd ones, matching original engine behavior. `battle.ts` loads the party through
`loadParty()` and calls `applyBattleResult` on victory, logging level-ups in the battle log.
Before this wiring, every battle rebuilt its party from a fixed RAM snapshot
(`party-record.json.liveExample`) and, on victory, wrote back only zenny and loot — earned
EXP was discarded and level/HP/AP reset on the next battle or reload.

### Items and equipment

Three lookup tables hold equipment stats: weapons at `0x801c9360` (24 entries), armor at
`0x801c9b28` (22 entries, shared by shield/helmet/body-armor via a subtype byte), and
accessories at `0x801ca100` (20 entries, two slots per character). Index `0` in any of the
three is the empty "Nothing" entry.

| Record | Offset | Field |
|---|---|---|
| weapon | `+0x10` | weight |
| weapon | `+0x12` | attack power |
| armor/shield/helmet | `+0x0e` | subtype (2 = shield, 3 = helmet, 4 = body armor) |
| armor/shield/helmet | `+0x0f` | weight |
| armor/shield/helmet | `+0x10` | defense |

Equip stat deltas, numerically exact across 6 of 6 tested characters
(`scratchpad/equip-verify.ts`):
- `ΔPwr = weapon atk (+0x12) + accessory bonuses` (example: Titan Belt +10 Pwr).
- `ΔDef = Σ armor def (+0x10)` over all equipped armor pieces.
- `ΔAgl = −(weapon weight (+0x10) + Σ armor weight (+0x0f))` — a weight system: heavier
  gear lowers agility.

`EQUIP_FIND` (`0x80166540`) counts equipped items per category across all 8 party records,
gated on party record `+0x07` bit0 (in-party flag).

Item descriptions link through a `u16LE` field inside the item record, masked with
`& 0x3fff`, indexing text Table B (455 item/skill/gene descriptions, see Menu assets and
fonts). The field's byte offset within the record depends on category: item `+2`, key item
`+0`, weapon `+8`, armor `+6`, accessory `+4`. Verified: Dagger → description 51, Clothing
→ 119, Flier → 437. Skill-to-description linkage is not linear; only window ids 76-110 map
as `id+139`.

### Menu assets and fonts

The system graphics — font, window frames, menu icons/labels — sit uncompressed as a
single `ctype3` VRAM page inside `FIRST.EMI` (4bpp, 128px wide); byte-identical copies ship
inside STATUS/SHOP/SISYOU/… as a shared sheet reused by every menu-bearing overlay.
`sub[3]` is the font (A-Z/a-z/0-9 in several sizes, plus kana, plus "Push Start/Select").
`sub[4]`/`sub[5]` are window frames, menu labels (HP/AP/Lv/EXP/Next), and icons. `ct6` is
the pBAV cell-position table; `ct8` is `(CLUT, TPage)` pairs. Extractor:
`extract/build-uifont.ts` → `public/entities/ui/{font,ui1,ui2}.png`.

The menu specifically uses the 8px raster rows of `sub[3]`: `O-Z@y148`, `A-N@y168`,
`♪,Z,a-m@y181`, `n-z@y193`, thin digits `@y136`, bold digits `@y216`; the larger rows
`y68-112` (`?!A-I/T-Z/J-S`) are a separate dialog font, not the menu font. Text CLUT,
derived by matching "Teepo/Ryu/Items" screen masks cell-by-cell onto the sheet: index 1
(gray 200) = opaque fill, index 2 (184) = bold fill, indices 3/5/6 (160/104/64) =
anti-aliasing, index 7 (32) = shadow. Atlas output: `menu-font.png`/`.json`, 86 glyphs.

**System text bank.** `FIRST sub[11]` = `AFLDKWA.EMI` at `0x8001a000` (see Master system
above for the same container's master-name block). Header: `[u32 baseA=8][u32 baseB]`;
each table is `[u16 offsets][0-terminated strings]`, offsets relative to that table's own
base. Table A holds 309 menu/system strings; entries A8-A14 are the 7 module labels
(status/items/equip/ability/tactics/config/camp). Table B holds 455 item/skill/gene
descriptions (see Items and equipment for the item-to-description link).

**UI icons.** `FIRST.EMI` font/UI lane, `sub@0x1e000200` → VRAM `(960,0)`, a 4bpp ribbon;
palettes come from the CLUT block `sub@0x80033800` → VRAM row 480, per-icon sub-palette 8
or 9. The menu bar and the battle command cross draw from this same source. Menu bar (7
icons, y39-42, from `menu-field.sav`): Items (selected, 22×22 scaled) = `(88,232)` palette
8; Ability = `(72,232)` palette 9; Equip = `(56,232)` palette 8; Tactics = `(216,232)`
palette 9; Status = `(184,232)` palette 8; Config = `(232,208)` palette 9 (the only icon
drawn from the `0x1c` UI band); Camp = `(200,232)` palette 9. Battle command cross
(`battlecap12.sav`): top/bottom = ability/items icons, left = `(104,232)` palette 9, right
= `(120,232)` palette 8; the 24×24 center icon is a battle-time composition from VRAM
`(256,480)`, known only from the savestate — its disc-side source is unidentified.
Extractor: `extract/build-ui-icons.ts` → `public/entities/ui/icons/` (10 named icons plus
`index.json` with UV/CLUT/proof per icon).

**Menu window themes.** The config option "Set window color" (`tableA[191]`) is a palette
switch — same texels, different CLUT column. Extractor: `extract/build-window-themes.ts`
→ `public/entities/ui/window-themes/`. FILL layer: `0x1a` UI band `@0x1a080200` → VRAM
`(832,256)`, byte-identical to the SHISU disc copy; tile region `uv(0,0)-(64,128)`. 4
material palettes sit on CLUT row 482 = sub-palettes `[3,5,4,6]` (code table `0x801df290`:
`GetClut(x∈{48,80,64,96}, 482)`) — wood, weathered-gray, dark-moss, green; menu tiles are
32×16 or 32×32, clut value `0x7883`. FRAME layer: `0x1c` UI band; piece region
`uv(96,128)-(256,232)`, a 9-slice set (TL corner `(120,144)` sized 24×16; edges `(144,144)`/
`(144,160)`/`(144,168)`; vertical strip `(152,144)` sized 8×56). 8 color styles sit on CLUT
row 481, `sub = 2·style + 1` (disassembly `0x801de158`: `x = style·32+16`); the field
default is style 5 / sub 11, brown-gold.

**Field menu reconstruction.** Key **X** opens `src/systems/gamemenu.ts`: main menu,
items, ability, and status+equip screens are built; tactics/config/camp remain stubs.
Ground truth comes from a DuckStation savestate captured with the menu open
(`references/gpudump/menu-field.sav`), located via the "savestate thumbnail" trick: the
zstd-compressed `frame[0]` embedded in every `.sav` is a ready 256×192 RGBA screenshot,
cheap to survey across many saves. Extractor: `extract/build-menu-assets.ts` →
`public/entities/ui/menu/` + `public/gamedata/system-text.json`.

GT crop assets: window frames as real 9-slices (party panel border 7px, small window
border 5px; the right panel edge is a mirrored copy of the left, since in GT the right edge
runs through the portrait); portraits for Teepo/Ryu at 46×46px; EXP/LV/HP/AP/TIME labels
(orange-cluster autocrop with an outline mask); 7 menu icons; a zenny display. Window
interiors are semi-transparent over the 3D scene in the original; the browser uses an RGBA
approximation, marked as such in the UI.

Documented approximations: the items screen shows the full compendium inventory (every
disc item) rather than the true save inventory, because the party record itself carries no
inventory or zenny data (those live separately, see The save format); TIME shows session
play time, not save playtime; zenny is hardcoded to 118 (the GT save's value); portraits
for characters other than Teepo/Ryu fall back to the field sprite; screen layouts beyond
the main menu are frame-style-faithful approximations, since ground truth exists only for
the main menu screen.

### The save format

`extract/read-savedata.ts` reads a BoF3 PSX memory-card file (`.mcd`) into
`public/gamedata/savedata-format.json` (a field table plus one fully decoded example save).

A PSX save is 1 block of 8192 bytes: a title frame at `+0x000` (`"SC"` magic, icon CLUT,
Shift-JIS title, 3 icon frames of 16×16 4bpp), followed by game data at `+0x200`. The game
data (`0x10b0` bytes) is a contiguous `memcpy` of live RAM `0x801448d8..0x80145988`, plus
header fields that live outside that RAM range (story bytes, area, position), plus a `u16`
checksum at `+0x070`. Conversion: `file_offset = 0x200 + (ram_address − 0x801448d8)`.

Save code disassembled from `SHOP.EMI sub[0]@0x801d0c00` (shared with STATUS/START):
prepare routine `0x801d69e0`, memory-card write `0x801d5220`, load-time verify
`0x801e5ae0` (checksum mismatch → error code 7).

| Data offset | Field | RAM address | Verified |
|---|---|---|---|
| `0x020` | `storyProgress[4]` | `0x80146864` | yes |
| `0x024` | `area` (u16) | `0x80143f00` | yes |
| `0x028`/`0x02c` | `playerX`/`playerY` (Q16 tile) | `0x80145ec4`/`0x80145ec8` | yes |
| `0x070` | checksum (u16) | — | yes |
| `0x090` | `partyRecords[8]` × 164 bytes | `0x80144968` | yes — the party record above |
| `0x678` | `zenny` (u32) | `0x80144f50` | yes |
| `0x682` | `partyOrder[8]` | `0x80144f5a` | — |
| `0x6e8` | `playTime` [H][M][S][tick] | `0x80144fc0` | yes |
| `0x774` | inventory item IDs (≤512, `0`-terminated) | `0x8014504c` | — |
| `0x974` | inventory item counts | `0x8014524c` | — |
| `~0x5c0..0xc90` | story flags, 7 `FLAG_SET` blocks | `0x80144eb8..` | yes |

⚠ The checksum is verified on load: any edit needs recalculation, a `u16` byte-sum over
`data[0..0x10b0]` with the checksum's own slot at `+0x70` treated as zero during the sum.

Verified end-to-end against an example save ("Breath of Fire 3(1) 00h41m Lv.01"): checksum
matched, and fields were plausible (Garr level 13 / 3000 EXP; Momo Pwr 30→88 with a cannon
equipped). The equipment and ability-list fields inside the record tail, once too sparse to
confirm from this debug save, are the party record's `+0x0e..+0x13` and `+0x5c..+0x83`
fields documented above. Dragon gene fields read as `0`/static in the one example examined,
so their exact layout inside the save is unconfirmed.

### Refuted approaches

- Equipment slot layout was reported "open" in one pass; it had already been solved and
  remained valid — the confusion came from misreading the `+0x5c` ability-bank values as
  equipment data, when that bank is the healing-magic list.
- `0x801d5744` was suspected to hold the equip logic; it is only the menu's item-name copy
  routine, not the equip mechanism.
- An early reading of `+0x66` as an equipment field was wrong; it is Momo's assist-magic
  list within the four ability lists.
- The engine ability-id table was first indexed from base `0x801caa06`; every ability
  consumer (master curricula, level grants, the four ability lists, enemy AI slots) actually
  indexes from `0x801ca98e`, six records (`0x78` bytes) earlier — the old base shifted every
  resolved name by −6.
- Skill ids `163-207` were suspected to be a separate "mage-master" id space; they are
  `type&3==3` duplicate entries (teach/transferable copies) inside the same corrected engine
  table, not a separate space.
- The system text bank parser (`references/re/afldkwa.json`) read per-table string offsets
  as absolute; they are relative to each table's own base, and the absolute reading produced
  spurious "7-character string" truncation artifacts.
- The menu font CLUT was first read with fill at index 7; the mapping is inverted — fill is
  index 1, shadow is index 7.
- The shared `ct6`/`ct7`/`ct8` block in `FIRST.EMI` was assumed to be the font graphic; it
  is an audio VAB (system/UI sound effects, byte-identical to `COMN_SE.EMI`) — the real
  font/UI graphic is the separate uncompressed `ctype3` page.
- Master assignment was suspected to live at RAM candidate `0x8014554c`; it is party record
  `+0x84` (level snapshot) and `+0x85..+0x8a` (`permStatBonus`).

### Open

- Accessory effect encoding: which bit or field of an accessory record selects its special
  effect is unresolved.
- The skill record's `+0x13` type byte and the high bits of the `+0x0e/+0x0f` flags word
  are read only as raw values; only the low 2 bits (bank selection on learn) are decoded.
- The master curriculum table (`0x801d4088`) is fully extracted but not verified per-master
  against canon; a repeated "Quads = Bunyan-set" pattern looks copied from the disc rather
  than a parsing error, unconfirmed.
- Master joining requirements (minimum stats) are not located.
- Party record `+0x07` bit1, suspected to be a "leader" flag, is unverified.
- Engine ability ids above 227 (a handful of outliers around `0xf0`) are not mapped; they
  share the id space with enemy AI skill slots.
- The general item-to-skill description linkage is not linear; only window ids 76-110
  (`= id+139`) are confirmed.
- UI icons for the item-list/equip screen (lane region V224-255) are unidentified — no
  screen packet has been captured for that screen, so raster position and palette are
  guesswork.
- Which FILL+FRAME theme combination each config "window color" selection sets is
  unconfirmed; needs one savestate per selection.
- The battle command-cross center icon's disc-side source blob is unidentified; it is known
  only from a savestate capture.
- Dragon gene fields in the save were `0`/static in the only example examined, so their
  layout is unconfirmed.
- Whether an actually fought (not EXP-injected) battle victory credits EXP end-to-end
  through `partystate.ts` was not confirmed; battle start could not be reliably triggered
  for an automated check.
- Equipment-change effects and field item use are not yet wired into the browser
  party-persistence layer.

