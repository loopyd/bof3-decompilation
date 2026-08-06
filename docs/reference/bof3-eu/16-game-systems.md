> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 16. Game systems: masters, fairies, dragons, fishing, shops, casino

### Masters

**Teaching mechanic.** The `SISYOU` routine at `0x801d3c50` implements the full master-teaching gate. Each master's curriculum sits in a table at `0x801d4088`, one row per master at offset `masterId × 12`. A row holds up to six `[level:u8][skillId:u8]` slots. The same `masterId` also indexes a separately verified (17/17) per-master stat-modifier table, six entries per master, at `0x4154`. The active master for the record under test comes from `byte[0x801448ed]`.

A skill unlocks when both conditions hold: levels gained since joining the master reach the slot's `level`, and the skill is not already marked in the bitmask at `0x80144f80` (tested as word index `skillId >> 5`, bit `skillId & 0x1f`). That bitmask check is the disc-code form of the Prima guide's "no doubling up" rule.

**Curricula.** 16 of 17 masters match the "Learned From" column of the Prima strategy guide (pp. 131-132) exactly. US/EU naming differs for two skills: Chakura = Enlighten, Shadowalk = Shadowwalk. ⚠ The one deviation: Quads (Bais/Lang/Lee/Wynn) carries Risky Blow at level 5, Focus at 8, and Super Combo at 12 on the disc, mapped code-exact, while the Prima guide lists no skill for Quads at all. Quads' dialogue separately grants a hardcoded "Chain Form" (area037, message "learned Chain Form!"). Whether the disc-only Quads skills are reachable in normal play is unconfirmed.

**Join conditions.** Extracted dialogue text (`public/text/`) gives disc-verbatim join requirements, stored in `masters.json` as `requirement`/`requirementQuote`/`requirementSource`:

| Master | Area | Join requirement |
|---|---|---|
| Mygas | area061 | give up all zenny |
| D'lonzo | area068 | own 15 weapon types |
| Fahl | area050 | roughly 30 battles without resting |
| Hachio | area041 | bring Beef Jerky, Swallow Eye, Angler, and Martian Squid |
| Emitai | area116 | pay 10,000 zenny |
| Deis | area098 | address her as "Ma'am" |
| Meryleep | area091 | bring a Flower Jewel |
| Quads | area037 | win a game of Hide-and-Seek |
| Giotto | area113 | answer a fisherman's questions |
| Hondara | area074 | give a wise answer |
| Durandal | area059 | show persistence |
| Bunyan | area003 | pass a story gate |
| Yggdrasil, Ladon | not recorded | speak to them |

Ladon's join dialogue sits in AREA144 (Dragnier's underground section, see below); Yggdrasil's area is not given in these notes. Exact numeric thresholds behind conditions such as Fahl's fight counter live in SCENA scripts, not yet disassembled.

**Sprite mapping correction.** The masters compendium showed Garr's sprite for Ladon. The mapping had rested on a single "visually strong" match at AREA143, key `k629`, justified as a "golden dragon god." That sprite is actually 64×56 and depicts a muscular humanoid with a red mane, turquoise jewelry, and a purple cloth: Garr, the Guardian, not a dragon. The real Ladon sprite is in AREA144 (Dragnier, underground), key `k624`: a 199×133 lying dragon with violet wings, red-orange spikes, a green eye, a spiked tail, and claws. It animates over 18 idle frames (breathing, wing movement), versus Garr's single static frame. Contact sheets of every descriptor key in both Dragnier areas (17 keys in the upper area, 9 underground, via `probe-master-gallery.ts`) resolved the mixup by visual comparison; the fix is verified in the browser compendium.

A follow-up check rendered contact sheets for the other 16 masters. The only other suspicious case, Durandal (AREA059, key `k2`, a key value that recurs in the Dragnier areas), turned out correct: sprite keys are area-specific, and `k2` in AREA059 is an old man with gray hair and overalls, matching the title "Wanderer." Deis (serpent being), Ladon (dragon), and Meryleep (fairy) are unambiguous; the rest are plausible.

**Implemented.** The compendium's masters module shows each master's join condition, a disc-verbatim quote, and the source area (`references/screenshots/2026-07-04-meister-neu.jpeg`).

**Open.** SCENA-level numeric thresholds; in-game reachability of Quads' disc-only skills; Yggdrasil's join area.

### Fairy village (incl. casino)

These notes cover only the casino corner of the fairy village; gen-splicing and fairy-fusion mechanics are not addressed in this material.

A one-byte state at `0x801448f2` selects the active casino card-game block. Every sampled savestate from one ground-truth session, more than 130 of them, all early game, read `0`: block A. Later blocks are unconfirmed.

**Open.** Block selection beyond block A; fairy-village mechanics outside the casino.

### Dragons and genes

**Data.** The battle-time replacement of the UI band lives in `BATL_DRA.EMI`: two `ct3` graphics blocks at `0x1c080200` and `0x1a080200` (the same VRAM targets used by the portrait band at x=896/832, y=256), plus CLUT blocks at `0x80036c00` and `0x80036e00` feeding VRAM rows 506 and 507. The `GetClut(…, 506)` routine at `0x800b1dac`, table at `0x800b6d68`, belongs to this battle context. The gene players find lying in the field is `AREA` descriptor 633: a purple faceted crystal with a glowing core, 32×32, CLUT mode `b6=3` (64 colors), with a 4-frame idle animation.

**Record format.** Band `0x1c` is an 8bpp gene-icon sheet in a ribbon layout (BPS-2, 128×256); an earlier BPS-4 reading rendered it block-scrambled. Its colors come from the `0x80036e00` CLUT block read as a 256-color palette, confirmed against ground truth (`references/spriters/dragon-genes-gt.png`). Band `0x1a` holds the dragon nameplates and window graphic; its palette mapping is still open. The icon sheet has no rigid grid (row pitch ≈24.3), so `extract/build-dragon-icons.ts` slices individual icons by projection-profile analysis instead of a fixed grid, producing a repaired `dragon_icons.png` plus 48 individual icon files under `public/entities/system/genes/`, with an index.

**Rules.** Descriptor 633 appears in exactly 13 areas: 054, 067, 075, 077, 093, 100, 103, 105, 114, 117, 123, 145, 146, the game's gene find spots. An earlier rule suppressing descriptor 633 from enemy processing, justified only as "a crystal object," is confirmed correct now that its role as a gene pickup is known.

**Implemented.** The dragon compendium module renders the animated field gene plus an icon gallery above the gene table, exported as `gene_field_f00-03.png`.

**Open.** ⚠ Mapping each of the 48 icons to its gene name needs a disassembly of the dragon menu. Band `0x1a`'s palette mapping is unresolved.

### Fishing

The browser reconstruction (`src/systems/fishing.ts`, a DOM overlay) already matches the disc for the catalog, bait and rod lists, HUD strings, and movement patterns; it still approximates the catch mechanic itself, flagged `_open` in `public/gamedata/fishing.json`.

#### BATE.EMI: everything but the live scene is static

All fishing data and code lives in `/BIN/ETC/BATE.EMI` (178,176 B):

| Sub | Type | Size | Target | Content |
|---|---|---|---|---|
| [0] | ct0 | 33,864 B | `0x801d0c00` | code (61 functions, `0x801d0da4`-`0x801d8508`) plus header data: format strings at `+0x04`, movement patterns at `+0x50` |
| [1] | ct3 | 32,768 B | VRAM `0x1c080200` | graphics (HUD/scene) |
| [2] | ct3 | 32,768 B | VRAM `0x1a080200` | graphics |
| [3] | ct0 | 512 B | `0x80033a00` | CLUT |
| [4] | ct0 | 512 B | `0x80033c00` | CLUT |
| [5] | ct6 | 3,616 B | - | cue/control data |
| [6] | ct8 | 44 B | - | SFX cue table |
| [7] | ct7 | 64,560 B | - | audio (pBAV) |

`sub[0]` is disassemblable code, not a data block. A statistical pass (`extract/probe-bate.ts`) finds 93-100% valid MIPS opcodes and, in every 4KB block, matching counts of `addiu $sp,$sp,-N` prologues and `jr $ra` returns (block +0: 14 prologues / 15 returns; +4096: 12/13; +8192: 9/9). Random data produces prologues without matching returns. The tool resolves 61 functions between `0x801d0da4` and `0x801d8508`, including HUD print paths (`lui $a1,0x801d` / `addiu $a1,0xc04`, addressing a format string at `0x801d0c04`) that call the resident print and sprite routines. Code and graphics need no emulator session at all; first-pass graphics sit at `references/re/bate/ct3_*.png` (4bpp, 1024×64: HUD text "CLEAR"/"TIME", digit sprites, fish symbols, with a still-arbitrary CLUT at that stage).

Tool usage:

```
npx tsx extract/probe-bate.ts                    # subfiles, function list, table @0x801d0c50
npx tsx extract/probe-bate.ts 801d1ca0 40        # disassembly from address
npx tsx extract/probe-bate.ts --xref 0x801d1234  # who calls/references this?
npx tsx extract/probe-bate.ts --rng              # all system calls by frequency
```

⚠ `jal` targets need `0x80000000 | (imm26<<2)`; without that transform, internal jumps wrongly land in the tool's "external call" list.

#### RAM layout

- Movement pattern table at `0x801d0c50`: ASCII names plus vectors, e.g. "Norm|al", "Atta|ck", vector `0xfff8fff8` = (-8,-8).
- Fishing work RAM `0x80148330`-`0x801483c6`.
- Entity array pointer `0x8014598c` (read 38 times by the overlay; its spawn routine sets size and position at `+0x4..+0xd`).
- RNG routine `0x8017e8c0`, called 40 times from the fishing code.
- Most frequent system calls from the fishing code: `0x801af524` ×65, `0x8014fa6c` ×45, `0x8017e8c0` ×40 (RNG), `0x8014e6f0` ×36, `0x80150304` ×32.

#### Global inventory arrays

The inventory consists of category arrays of 128 bytes each, index = item id, value = count (source: almarsguides.com CodeBreaker lists for NTSC-U; the same addresses apply unchanged to PAL SLES-01304, cross-checked against the zenny value and the item array):

| Category | Base | Verified |
|---|---|---|
| Items | `0x8014524c` | savestate ids 0/1/2 = 1 each, the three starting items |
| Weapons | `0x801452cc` | writing takes effect in-game |
| Armor | `0x8014534c` | writing takes effect in-game |
| Options/Accessories | `0x801453cc` | writing takes effect in-game |
| Zenny (u32) | `0x80144f50` | reads 118, matches a known "5M Zenny" code |

Gameshark form: `30<address without the leading 80> 00<count>` (an 8-bit write), e.g. `3014524C 0063`. ⚠ DuckStation only picks up cheats on load; reload the savestate after poking, or nothing happens.

#### Fishing gear: rod and bait ownership

The angling gate is rod ownership, not the fish-jump trigger. At the Central Wyndia spot, the world-state transition `fish45 -> fishspot` plays the fish-jump animation regardless, but the minigame will not start without an owned rod. Early-game inventory there reads only three basic items (Plate, Beef Jerky, Healing Herb) from the item-category array. That session recorded its address as `0x8014504c`, almost certainly the same `0x8014524c` array above given identical ids 0-2 and count 1 each; the notes carry both forms. Poking an equipped-accessory slot (`+0x12 = 46`, the global rod id) was not enough by itself, which pointed at a still-unlocated gear array rather than the equip-slot system. The equip-lookup routine `EQUIP_FIND` (`0x80166540`) was flagged as a fallback disassembly anchor. The `0x80148330` work-RAM zone reads zero until the minigame actually starts.

A purchase diff (a savestate taken before and after buying gear) found the real location directly: exactly five consecutive bytes changed at `0x801453cc`:

```
0x801453cc  0 -> 1   Wooden Rod    (1 unit)
0x801453cd  0 -> 8   Worm          (8 units)
0x801453ce  0 -> 8   Toad
0x801453cf  0 -> 8   Old Popper
0x801453d0  0 -> 8   Sinker
```

This is a separate fishing-only numbering starting at 0 (0 = rod, 1..n = bait in catalog order), not the global accessory ids used elsewhere in `fishing.json` (46-51 for rods, 28-45 for bait). It coincidentally shares its base address with the global "Options/Accessories" array above, which is why filling that array by global id had no visible effect: the base was right, the index scheme was not. Gameshark: `301453CC 0001` (rod), `301453CD 0008` (bait), and so on. Reference savestate: `SLES-01304_fishgear.sav`.

#### Catch decision

At `0x801d1ca0`, one code path fills the fishing descriptor with fixed demo values: state `0x80148330 = 1`, `+0x04 = 0x140` (320), `+0x06 = 0x3e` (62, "Bass"). A second fixed write follows at `0x80148358`: base `= -0xaa`, `+0x02 = 0x3e`, `+0x08 = 7`. Two hardcoded Bass descriptors mark this as an initialization or default path, not the RNG catch logic. Only three fixed-address descriptor writes exist anywhere in the overlay (`0x801d1cc4`, `0x801d1cd0`, `0x801d1cdc`), all part of this constant path; the real catch write goes through a pointer instead, plausibly the entity array at `0x8014598c`.

The loop right after the constant path, `0x801d1ce4`-`0x801d1d18`, disassembles as:

```
801d1ce0  andi $v0,$a0,0xff      ; $a0 = run index
801d1cec  lbu  $v0,0x4f5a($at)   ; byte from 0x80144f5a + index
801d1cf4  beq  $v0,$zero,→hit    ; empty entry -> take it
801d1cfc  bne  $v0,$a2,→next     ; entry == $a2 -> take it
801d1d08  sb   $a0,0x0($a1)      ; store the found index
801d1d14  sltiu $v0,$v0,3        ; loop over exactly 3 entries
```

Three entries, "empty or equal to `$a2`," identify a 3-member party-slot search (table in the save/party region at `0x80144968 + 0x5f2`): it resolves which party member is fishing, not which bait is equipped.

Bait-to-fish selection therefore stays open, narrowed to the roughly 40 RNG call sites reachable via `probe-bate.ts --rng` (including `0x801d1844`, `0x801d18b8`, `0x801d18dc`, `0x801d1948`, `0x801d2460`, `0x801d24ac`), where a `rand()` result should fold into a range with `divu`/`mfhi` and compare against a table value rather than a constant.

#### Catch values and totals

Three ground-truth catches, read off the catch screen (`references/re/bate/fang-anzeige-gt.png` for the first):

| Catch | Fish | Size | Points | Total | Rank shown |
|---|---|---|---|---|---|
| 1 | Bass (id 62) | 20 CM | 200 PTS | 300 | novice++ |
| 2 | Jellyfish (id 56) | 23 CM | 70 PTS | - | - |
| 3 | MartianSquid | 47 CM | 300 PTS | 600 | rodman |

`TOTAL POINT` lives at `0x801ff488` (with stack copies at `0x801ff4a8` and `0x801ff4c0`), confirmed across catches 1 and 3 (300 -> 600). It is the only catch-related figure stored anywhere in RAM as a number. A full 2MB scan across all three catches, in every plausible encoding (u8, u16, u32, digit pairs, ASCII digits, and the fish name string), found no address holding the correct fish id, size, or points for more than one catch. An early guess that the 200/300-point readout might use BCD or split-digit encoding turned out moot for the same reason: the individual numbers are computed straight into the GPU display list and never stored as a discrete value, only the running total is kept. ⚠ Even a dense 40-sample series at 2-second intervals (`fs1`...`fs40`, with GPU dumps every third sample) caught no catch-display frame; the display window is shorter than the sampling interval, so a capture must trigger at the moment of the catch rather than poll.

#### Fish item table (resident RAM)

The fish are ordinary items in the resident item table at `0x801c9008` (`extract/build-fish-table.ts` -> `public/fishing/fish-table.json`). Record layout, 18 bytes: `u16 flag · u8 itemId · u8 type(0x40|0x41) · u16 value · char[12] name`. Text codec: `0xff` = space, `=` = hyphen (`Man=o=War` -> "Man-o-War").

The table's own `itemId` field runs a constant 103 higher than the fish id used everywhere else in this section (fishing.json, the catch descriptors, the image formula below): `itemId = fish id + 103`, confirmed independently because the name order matches the two ground-truth-confirmed ids (62 = Bass, 63 = MartianSquid):

| Fish id | Record itemId | Name | Sale value |
|---|---|---|---|
| 59 | 162* | Trout | 50 Z |
| 60 | 163 | Rainbow Trout | 80 Z |
| 61 | 164 | Red Catfish | 160 Z |
| 62 | 165 | Bass | 300 Z |
| 63 | 166 | MartianSquid | 100 Z |
| 64 | 167 | Black Bass | 200 Z |
| 65 | 168 | Barandy | 400 Z |
| 66 | 169 | Man-o-War | 2000 Z |
| 67 | 170 | Flying Fish | 10 Z |
| 68 | 171 | Blowfish | 50 Z |
| 69 | 172 | Sea Bream | 80 Z |
| 70 | 173 | Sea Bass | 160 Z |
| 71 | 174 | Black Porgy | 100 Z |
| 72 | 175 | Octopus | 400 Z |
| 73 | 176 | Angler | 300 Z |
| 74 | 177 | Devilfish | 400 Z |
| 75 | 178 | Spearfish | 1000 Z |
| 76 | 179 | Whale | 2000 Z |
| 77 | 180 | Mackerel | 4000 Z |

\* Trout's record reads itemId 230, not the expected 162; the block's first entry is likely misaligned by a truncated preceding foreign name, so only this one id is uncertain. Name and value are correct.

"Angler" (record itemId 176) names the anglerfish, not the in-game angling rank of the same word; the rank names are display sprites, never stored as text, which is why an earlier RAM text search for rank names came up empty. Immediately after this block, at item ids 200/201, sit the Manillo trade gifts: Part A through Part H, plus Horseradish, at 200 Z and 4,000 Z respectively. Sale value and catch points are independent: Bass sold for 300 Z but scored 200 points at 20 CM; MartianSquid sold for only 100 Z but scored 300 points at 47 CM, since points scale with size, not price.

#### Rods and bait (RAM name list)

A second block at `0x801ca300`-`0x801ca510` holds 20-byte records, each marked `0x32 0x40`, followed by 2 header bytes and a 12-byte name (same text codec as above). Unlike the fish block, this one does not follow the 18-byte item schema.

Rods (5): Wooden Rod, Bamboo Rod, Deluxe Rod, Angling Rod, Master's Rod. A foreign item, "Spanner," sits between two of these; the block is not homogeneous.

Bait (17): Spirit, Caro, Heavy Caro, Toad, Baby Frog, Frog, Fat Frog, Old Popper, Popper, Top, Dogwalker, Sinker, Float, Hanger, Deep Diver, Coin, Ding Frog.

The two header bytes preceding each name cycle through 20/50/200/244 and are not item ids; whether they encode a price component or an effect is open. The 20-byte record grid itself is certain, confirmed by three rod hits exactly 20 bytes apart.

#### Graphics: fish images

Working formula, GT-confirmed at both ends (Jellyfish 56 -> 0, Bass 62 -> 6):

```
i    = fishId - 56
UV   = ((i mod 4) * 64, floor(i/4) * 40)
CLUT = (i * 16, 491)
```

Sheet `pg768,256`, 4bpp (⚠ colorMode must read as 0; reading it as 8bpp produces sepia garbage), each image 64×40 with its own CLUT.

The fish graphics do not live in `BATE.EMI` itself. A byte fingerprint against dump VRAM (`extract/probe-bate-gfx.ts`) places them empirically, correcting the subfile table's nominal targets (`0x1c080200`/`0x1a080200` do not resolve to a readable location for this content):

| Sub | Content | VRAM location | Evidence |
|---|---|---|---|
| sub[1] | 32 KB image data | page 896,256 (64 halfwords wide) | 5 offsets, Δy matches Δoffset |
| sub[2] | 32 KB image data | page 832,256 | hit at +20480 -> (832,416) |
| sub[3] | 512 B, 16 palettes | CLUT row 481 | 512/512 B identical |
| sub[4] | 512 B, 16 palettes | CLUT row 482 | 512/512 B identical |

The fish palettes themselves live in the area file of the fishing spot used for capture: `AREA030.EMI sub[14]`, starting at offset 4096, 32 B (16 colors) per fish slot. The dump CLUTs for slots 0-15 sit there at exactly `4096 + i×32`, 16/16 hits. `AREA030.EMI` carries all 22 fish palettes, while VRAM at capture time only held those relevant to that spot; AREA030 is freshwater, so Spearfish, Whale, and Mackerel never loaded, leaving CLUT columns 304/320/336 (row 491) empty in the dump and their images initially missing. Additionally, the dump CLUTs for slots 16-18 (Octopus, Angler, Devilfish) matched nowhere in `AREA030.EMI`; they were leftover VRAM from unrelated graphics, so the first-pass extraction had colored those three species wrong. The disc palette renders them correctly (anglerfish with a luminous organ, red or turquoise octopus).

Result (`extract/build-fish-images.ts`, `npm run extract:fishimg`): dump texels, present for all 22 slots, combined with disc palettes from `AREA030.EMI sub[14]`, give 22/22 fish images. 16 are byte-identical to the earlier dump-only extraction (confirming the method), 3 are corrected (Octopus, Angler, Devilfish), and 3 are new (Spearfish, Whale, Mackerel). Output: `public/fishing/fish/<id>_<Name>.png` plus `_sheet.png`.

#### Graphics: rank scale, font, HUD

**Ranks.** Sheet `pg320,256`, CLUT `(0,490)`, 8bpp. Ranks are finished sprites, not rendered text: novice, rodman, rodmaster, master of angling, THE FISH, plus a "New Record!" banner and "TOTAL POINT"/"PTS."/"CLS" labels with their display frames. Output: `public/fishing/ui/rang1…rang5*.png`, `newrecord.png` (crops still rough; full sheet at `references/re/bate/page_320-256_clut490_RAENGE.png`). The "++" suffixes seen in-game (e.g. "novice++") are drawn from the font, not baked into the rank sprite.

**Font.** Sheet `pg960,0`, CLUT `(0,480)`: digits 0-9 and A-Z, used to render catch size, points, and rank suffixes.

**HUD.** Sheet `pg576,256`, 8bpp. The gameplay CLUT is `(0,489)`, not the originally assumed `(0,490)`; `(0,490)` renders only on the catch-display frames, confirmed by checking which CLUT appears on which captured phase (`c490` only in `bate_p1`/`fs3`, `c489` on every gameplay-phase frame). The sheet holds the complete control graphics: the reel, a "CASTING POWER" bar, a "PLAYER vs FISH" tug-of-war bar, "TEC +" with levels 1-4, bar frames, and kg/cm labels. Output: `public/fishing/ui/hud_*.png`, full sheet `_hud_sheet.png`, in-game-CLUT version `_hud_sheet_ingame.png`. This is the concrete template for the original controls: a casting-power bar on cast, then a player-versus-fish tug-of-war with a 1-4 TEC level while reeling, none of which the current DOM overlay (`src/systems/fishing.ts`) yet reproduces.

Animation on this sheet moves by UV, not by a frame row: one sprite steps `uv(192,72)` -> `(208,72)` -> `(224,72)` (steps of 16) across successive dumps, a second steps more finely across `uv(166..172,104)` (steps of 2). The three 16×24 frames at `v=72` are extracted as `public/fishing/ui/anim_frame1..3.png` and show shore grass moving in the wind. General method: follow one prim's UV across multiple dumps to find an animation's frame grid, rather than scanning a sheet for a row of frames.

**Rod graphics.** None exist. All 15 prim classes of the equipment-menu dump (`bate_menu`) were reviewed; no rod icon appears anywhere. The original UI lists rods as plain text, which is why the browser's text list already sits close to the source. The only rod-related sprite is the reel inside the "CASTING POWER" display (`pg576,256`, CLUT `(0,489)`, 193×73) -> `public/fishing/ui/castingpower.png`.

#### Graphics: fish shadows (swimming animation)

Fishing plays out in the ordinary world scene, character on the shore, water rendered with the normal map texture (`references/re/bate/fang-anzeige-gt.png`), and the fish visible in the water are shadow sprites, not a HUD element. UV-tracking across 14 dumps first ruled out the wrong candidates:

| Class | Prims | Distinct UVs | Reading |
|---|---|---|---|
| `pg448,256` `c0,485` | 226 | 75 | terrain tiles, a regular 16×16 grid, no animation |
| `pg448,256` `c0,484` | 6251 | 17 | water surface, many quads, few textures |
| `pg704,256` `c96,483` (semi m3) | 126-150 | 1 | water surface (semi-transparent), one texture, only quad count varies |
| `pg960,0` `c0,480` | 258 | 36 | font |
| `pg704,0` `c32,499` | 112 | 25-53 | initially read as generic "UI small parts" |

That last class, `pg704,0` / CLUT `(32,499)`, is the fish shadows. A dump reconstruction (`npx tsx extract/render-gpudump.ts references/gpudump/bate_p5.psxgpu.zst`) shows 5-6 dark silhouettes standing in the water: elongated fish and rounder jellyfish or octopus shapes.

| Property | Finding |
|---|---|
| Structure | each shadow is 1-4 quads (8×8, 16×16, 24×16) assembled into a silhouette |
| Texel | dithering-striped (every other column): PSX semi-transparency without an alpha channel |
| Animation | driven by screen position; there is no frame switching |
| Count | 0-4 visible at once, changing as fish swim into and out of frame (e.g. positions at -47/-55 sit half offscreen) |
| VRAM | byte-identical across every dump, so no texture upload occurs |

Measured positions across several dumps (`extract/build-fish-shadows.ts` -> `public/fishing/shadows/`, 28 tile sprites, 14 composited shadows, `index.json`):

```
bate_p1   0: —
bate_p3   4: (323,19)×2 (204,16)×3 (304,70)×3 (-47,252)×2
bate_p5   1: (50,301)×1
bate_p9   3: (377,185)×1 (369,246)×3 (51,376)×2
bate_p13  3: (258,196)×1 (346,202)×1 (332,257)×2
fs21      3: (300,186)×2 (128,271)×2 (242,277)×3
fs33      3: (357,-55)×1 (205,131)×4 (101,138)×1
```

Because a shadow's UV never changes, only its position, no UV-tracking pass could find it, which is why the class was first filed as "UI small parts" and, separately, as "water foam sprites" (`public/fishing/ui/wasserschaum.png`).

#### Graphics: sheet index

| Page / CLUT | bpp | Content |
|---|---|---|
| `pg320,256` / `(0,490)` | 8 | ranks, display frames, "New Record!" banner |
| `pg768,256` / `(i·16,491)` | 4 | fish images, 22 slots, 64×40 each, one CLUT per fish |
| `pg960,0` / `(0,480)` | 4/8 | font: digits 0-9 and A-Z |
| `pg576,256` / `(0,489)` gameplay, `(0,490)` catch screen | 8 | fishing HUD |
| `pg448,256` / `(0,484)` and `(0,485)` | 8 | terrain (grass/shore) and a green-palette HUD variant |
| `pg704,0` / `(32,499)` | - | fish shadow silhouettes |
| `pg704,256` / `(96,483)` | 4 | object sheet: trees, fish grid, portraits; CLUT undetermined, shows a green cast |
| `pg832,256` / `(0,486)` | 8 | second BATE graphics block; CLUT undetermined, noisy |

A region on `pg704,256` (u88-264, v180-252) initially looked like four fishing-reel animation frames. Checked across all 29 available dumps, it never draws within a single frame; it is leftover VRAM content from an unrelated overlay sharing the same memory. The real animated reel is the one already cut from the HUD sheet, above.

#### Rank thresholds

Sourced from the project's own community layer (`public/gamedata/fishing.json -> community.ranks`, a Supercheats FAQ by ElectroSpecter) and cross-checked disc-side at two points against the ground-truth catches: total 300 landed between the 200 and 600 thresholds and showed "novice++"; total 600 hit the threshold exactly and showed "rodman."

| Points | Rank | Reward |
|---|---|---|
| 0 | Novice | - |
| 100 | Novice+ | - |
| 200 | Novice++ | - |
| 600 | Rodman | - |
| 1000 | Rodman+ | - |
| 1500 | Rodman++ | - |
| 2000 | Rodmaster | training with Master Giotto becomes available |
| 3000 | Rodmaster+ | - |
| 4000 | Rodmaster++ | - |
| 5000 | Master of Angling | - |
| 7000 | Master of Angling+ | - |
| 9000 | Master of Angling++ | Master's Rod plus Ding Frog (fisherman west of Steel Beach, after completing the game) |
| 9500 | THE FISH | Fountain Pen, same NPC, grants a reusable Ink skill |

#### Ground-truth capture: method and tooling

The fish encyclopedia and spot catalog come straight from extracted game text. The fishing-mode areas (area030, area089, area129, identical copies) carry a 23-entry fish encyclopedia in item-id order 56-77, plus a 23rd Manillo entry ("Merchant who travels the world's seas," diameter 150cm), format `<desc> [Use: <use>] Av. Length: NNCM`. Pairing verified 22/22, e.g. Jellyfish "floats," Puffer "no poison," Whale "mammal 180CM," Mackerel "local favorite in Wyndia." These texts carry the official bait hints ("likes worms," "goes for any lure," "likes small fish"), stored as `fish[].desc/use/avgLengthCm`. The world-map spot catalog reports `<description> Target fish: <list>`, disc-verbatim from the overworld area texts: 17 spots across 7 overworlds (area016 ×3, area045 "Central Wyndia" ×3, area065 ×2, area087 "Rhapala Region" ×3, area088 "Urkan Region" ×3, area115 "Dauna Hills" ×2, area151 "Lost Shore" ×1), stored as `fishing.json spots[]` (Playwright: `references/screenshots/2026-07-04-angeln-neu.jpeg`).

Reaching the live minigame screen needed `extract/fish-capture.ts`, which captures while playing (F2/F8 in rhythm, copying dumps to `references/gpudump/bate_p*`). ⚠ Two measured pitfalls: DuckStation stores GPU dumps under `DuckStation/screenshots/`, not `gpudumps/`, so the wrong path makes the script silently report "no new dump"; and F8/F2 go to whichever app is in the foreground, so a background app (Signal, Chrome) swallows the keystrokes unless DuckStation is activated first (`osascript -e 'tell application "DuckStation" to activate'`). The movement-pattern marker `@0x801d0c50 == "Norm"` cannot serve as a run-detection signal during actual play (see Refuted approaches); a direct screen capture (`screencapture -R` on the window) is the reliable alternative.

A first real capture reached "Set rod & lure…": `references/gpudump/bate_menu.psxgpu.zst` (350 KB, 679 prims, 173 semi-transparent: water `pg448,256_c0,484` ×414, `pg704,256_c96,483` ×126, `pg448,256_c0,485` ×75, HUD `pg960,0_c0,480` ×25, font `pg768/832,0_c192,502`), alongside `SLES-01304_fishready.sav` (starting position AREA030, tile 15,60, direction 5) and `SLES-01304_bate_menu.sav`. A full session later produced 16 GPU dumps and 16 matching savestates (AREA030, FSM state `b90=8`): `bate_p1…p14` (cast/waiting/reeling, 606-687 prims, 141-208 semi-transparent), `bate_menu` (equipment screen), and `bate_catch` (catch result).

A full 1:1 scene reconstruction (rather than the DOM overlay) needs matching dump/savestate pairs, built with the same solver pipeline used for world areas:

1. Reach a fishing spot on the Wyndia coast (area045). Savestates `SLES-01304_fish45.sav`/`fishspot.sav` are already positioned there, or warp with `npx tsx extract/warp.ts 45 <col> <row>` (spot coordinates in `fishing.json spots`).
2. Start the minigame from the field menu's rod until the fishing screen is up.
3. Keep DuckStation in the foreground; it pauses when backgrounded. Take F8 GPU dumps across roughly 6-8 phases (cast, float drifting, bite, reeling, catch display, result/size screen) to `references/gpudump/bate_<phase>.psxgpu.zst`.
4. Save a matching state per dump (`SLES-01304_bate_<phase>.sav`); it carries RAM and VRAM, so descriptor values and texels can be checked offline.
5. Turn cheats off afterward.

Pad injection cannot substitute for manual play here: the controller input context lives in the scratchpad at `0x1f801c00`, unreachable for a Gameshark-style poke.

⚠ Tool lessons from the same sessions: always start DuckStation with the `.cue`, never the `.bin` (with the `.bin` the emulator reports "No Image" and the game stalls once it needs CD data; `extract/warp.ts` still carries a `.bin` constant, `TRACK1`, that needs checking). Dump savestates are not usable for playing: several place the character on tile (41,28) with walk code `0x10` (blocked, outside the world), others on warp tiles (`0xc0`). Use the user save slots (`SLES-01304_2..10.sav`) to actually play. Do not touch the controller type; Pad1 must stay `AnalogController`, switching to `DigitalController` does not fix input and only wastes time. Warp cheats need a guard that clears once the target area differs from the source area, otherwise they fire every frame and the game reloads endlessly.

#### Implemented

Catalog, bait, rod, and HUD text; per-spot species lists; the fish encyclopedia; movement patterns; all 22 fish images; the rank sprite sheet and thresholds; the font and HUD sheets including the in-game CLUT and grass animation; the fish shadow class; and the rod/bait ownership gate are disc-exact and extracted. The angling gate itself (rod ownership, confirmed by testing) and the ground-truth capture tooling are working end to end for every phase except the still-approximated catch mechanic.

#### Open

Bait-to-fish selection probability, narrowed to roughly 40 RNG call sites but not solved. Catch-size generation: which code writes descriptor `+0x04` outside the constant path (`--xref` on writers of `0x80148334` is the next concrete step). Per-spot fish population tables: not located; naive byte scans of the overworld EMIs and BATE turned up only pointer and code noise, with the SCENA arguments of each spot trigger as the next candidate. CLUT and bit depth for `pg704,256` and `pg832,256`, best resolved from a dump where that content is visibly active. The two mystery header bytes preceding each rod/bait name record. Trout's misaligned resident-table id. Manillo's trade prices (see Shops). A dedicated multi-phase ground-truth capture for the full 1:1 scene reconstruction.

### Shops

Shops remain largely unextracted; a dedicated reverse-engineering pass is needed for the items and shop module, including Manillo's fishing-gear trade prices.

The shop-id RAM zone stays at zero everywhere outside an open shop menu; values appear only once a shop screen is active, so no simple always-resident "current shop" flag exists.

A shopkeeper NPC in the AREA008 basement was captured in a corrupted state with no talk reaction during a ground-truth session, open, and possibly a capture artifact rather than a disc bug.

Ground truth for late-game shop states is reachable through the phase-advance poke: `warp.ts --poke 146871=80` before an area load increments the story phase (verified 1 to 2, chains up to 15). The savestate `mcneilp12` (AREA000 at story phase 12) confirms shops change with story phase by showing the McNeil weapon-shop sign.

**Open.** Shop inventories, prices, and the underlying RAM structures, entirely. The AREA008 NPC talk-reaction issue.

### Refuted approaches

**Masters.** A `confidence: 'visuell-stark'` tag on a sprite mapping meant one candidate matched expectations, not that every candidate in every relevant area had been compared. The Ladon/Garr mixup happened because the search only checked AREA143; Ladon's actual dialogue sprite sits in AREA144, underground. Check every area belonging to a location before accepting a visual match.

**Fishing, code identity.** `sub[0]` of `BATE.EMI` was first read as non-code: a fishing savestate showed only the bytes `16 00 00 00 25 32 64 00` (the ASCII format string "%2d") at its target address `0x801d0c00`, because the overlay had already unloaded by the time the state was captured. A block missing from one savestate only proves when the capture happened, not what the block contains; disassembly later showed `sub[0]` is 61 functions of ordinary code.

**Fishing, trigger state.** The minigame's launch trigger was read as `bb0=13`; the measured FSM state during an actual session is `b90=8`, and the `bb0=13` reading is refuted.

**Fishing, run-detection marker.** The value at `0x801d0c50`, part of the movement-pattern table and normally reading "Norm...", was proposed as a marker for "minigame currently running." It is unusable for that: live gameplay overwrites it with unrelated data.

**Fishing, hooked-fish descriptor.** `0x80148330` (`+0x04` size, `+0x06` fish id) was read as the live hooked-fish descriptor, as still stated in `fishing.json`'s `mechanic.hookedFish`. Disassembly shows only three fixed-address writes to it anywhere in the overlay, all part of one constant/demo path (state 1, size 320, fish id 62 "Bass"); the real catch write goes through a pointer instead, which makes this field a display or handoff buffer rather than the catch decision.

**Fishing, bait-slot lookup.** The three-entry table lookup at `0x80144f5a`, reached right after the constant descriptor writes, looked like an equipment or bait-slot check. Its disassembly shows a 3-member party-slot search instead, which party member is fishing, reusing the party table at `0x80144968 + 0x5f2`.

**Fishing, bait-affinity flag.** The fish item record's `flag` field was tested as a bait-affinity bitmask; correlation against the community bait/fish matrix reached only 58-84%, chance level. It is the item's usage-flags field, unrelated to bait.

**Fishing, catch-value storage.** Individual catch results were assumed to sit somewhere in RAM as a small descriptor. For the first catch (Bass, id 62, 20cm, 200pts), plausible matches for the fish id turned up at `0x80144002`, `0x80146491`, `0x801469f2`, and `0x80147112`. Candidates for the size turned up at `0x80144fea`, `0x8014599e`, `0x801459a2`, `0x801459be`, `0x801459c2`, `0x80146b1e`, and `0x80146fe2`. A second catch with disjoint values (Jellyfish, id 56, 23cm, 70pts) matched none of them, checked as u8, u16, digit pairs, ASCII text, and the fish name, across the full 2MB of RAM. A third catch confirmed the pattern. Only the accumulated `TOTAL POINT` exists as a stored number.

**Fishing, gear location.** Rod and bait ownership was assumed to live in the six standard 128-byte item-category arrays (`0x8014524c`...`0x801454cc`); filling them with test counts left the in-game rod list empty. A byte search for global accessory ids 46-51 found only random hits, and an (id, count) heuristic surfaced only the index table at `0x801426c2`. A poke on `0x80144f5a` did nothing, and a memory-card comparison used a mismatched save. The real gear slots sit at `0x801453cc`, found instead by diffing a savestate across an actual purchase.

**Fishing, sheet identity.** The class `pg704,0` / CLUT `(32,499)` was filed first as generic "UI small parts," then relabeled "water foam sprites." Both were wrong: it holds the fish shadows, which move by screen position with a constant UV, so no UV-tracking pass could ever implicate it.

**Fishing, reel animation.** A region on `pg704,256` (u88-264, v180-252) looked like four fishing-reel animation frames. Checked across 29 dumps, it never draws within a single frame; it is leftover VRAM content from an unrelated overlay sharing the same memory. The real animated reel is part of the HUD sheet (`pg576,256`).

**Fishing, swimming-animation mechanism.** Byte-identical VRAM across every dump first suggested a cell-upload animation, the same class used for the world map's water tiles. The actual mechanism repositions fixed-texel shadow quads on screen; no texture upload happens at all.

### Open

**Masters.** Exact numeric thresholds behind conditions such as Fahl's fight counter, stored in SCENA scripts rather than the disassembled tables. Whether Quads' disc-only curriculum (Risky Blow, Focus, Super Combo) is reachable in normal play. Yggdrasil's join area is not recorded in these notes.

**Fairy village.** Gen-splicing and fairy-fusion mechanics are not covered by this material. Casino block selection beyond the one observed block A (`0x801448f2 = 0`).

**Dragons and genes.** Which gene name belongs to which of the 48 icons, needing a dragon-menu disassembly. Band `0x1a`'s (dragon nameplate/window) palette mapping.

**Fishing.** Bait-to-fish selection probability, narrowed to about 40 RNG call sites. Catch-size generation source. Per-spot fish population tables. CLUT and bit depth for `pg704,256` and `pg832,256`. The meaning of the two header bytes preceding each rod/bait name record. Trout's misaligned resident-table id. A dedicated multi-phase ground-truth capture for a full 1:1 scene reconstruction.

**Shops.** Inventories, prices (including Manillo's fishing-gear trade prices), and the underlying RAM structures, entirely unextracted. The corrupted AREA008 basement NPC with no talk reaction.

