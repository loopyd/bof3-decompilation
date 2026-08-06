> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 9. Battle sprites, enemies and bosses

Battle sprites, enemy records, and boss figures share one resident sprite-program format
across field and battle rendering. Enemy and boss records each carry several
independently-decoded fields — name, graphics key, AI action slots, encounter-formation
membership — layered with CLUT and texture-page lookups per sprite cell. Named bosses map to
figures drawn from their own or a host area; the bestiary assembles both static and animated
figures from this data, cross-checked against the Prima strategy guide and backed by a
dedicated sound-extraction pipeline.

### Where the pixels live

Battle sprites use the same resident renderer as field graphics. Disassembly of `0x8014c62c`
— the POLY_FT4 sprite renderer, shared between field and battle — shows the sprite program
format is identical to field mesh groups and PLCHAR `ct1` data. A block holds `[vc:u8]`
followed by `vc` cell records, each packing `[flag][Xs:i8][Ys:i8][U:u8][V:u8]`, 5 bytes per
cell. The format needs no runtime capture to reconstruct: ENEMY019, rebuilt statically from
its stored records, matches ground truth on all 27 of 27 cells
(`references/screenshots/2026-07-03-enemy019-statisch.png`). Full format spec:
`scratchpad/BATTLE-SPRITE-FORMAT.md`.

Confirmed disc sources by sprite class:

- Bosses — `BOSS###.EMI` sub2, `ct0@0x800f0800`, uncompressed `[vc][cells]`.
- Party-side battle sprites — `BPLD###.EMI` ct1.
- Dragon transformation forms — dedicated EMIs (`RYUU`, `RYUD`, `BRTU`, `CRYU`, …), same
  format.
- Field furniture — the AREA EMI at `0x800d3800`.

Bosses, party battle sprites, and dragon forms — including the 11 previously-missing
transformations — are extractable directly and statically. ⚠ Only regular enemies
(`ENEMY###`, 200 files) stay open: their EMI holds audio and the `ct8` CLUT/texture-page
table but no cell geometry, so the `[vc][cells]` block is likely built at load time. One
synchronous active-battle dump would confirm its source.

### Descriptors

Each sprite cell's `flag` byte selects a size from table `0x8017fa08`: width =
`((flag&3)+1)·8`, height = `(((flag>>2)&3)+1)·8`. The stored `U` value splits by range to
pick both texture page and local U: below 40 maps to `page−1` with local U `+128`; 40 up to
160 maps to `page` unchanged; 161 and above maps to `page+1` with local U `−128` — the base
page comes from `ctx[0x25]`, held per object. `V` is `ctx[0x26]+V`. Screen position is
`ctx[0x2e/0x30] + Xs/Ys·ctx[0x40/0x44]`.

The enemy record carries several separately-decoded fields. Its name sits at fixed offset
`+0..+7`, exactly 8 bytes, null-terminated only if shorter. An earlier 12-byte reading is
superseded: it pulled overflow garbage from the next field ("NitemareE", "Assassin&",
"BossGbln2", "Thanotos)", "Egg Gangq", "Mikbaj"). Byte `+9` can be non-zero — Nitemare holds
`0x04` there — and the Prima guide's short forms confirm the 8-byte length, which together
refute the 12-byte reading. The fix propagated through `build-chardata`,
`build-enemy-figures-static`, `build-enemy-anims`, and `build-boss-figures`, with every index
rebuilt (ENEMY019 re-verified 27/27 unchanged). ⚠ A level field stays open: the earlier
candidate `u16@+8` collides with full 8-character names (it reads 1093 for Nitemare); a
candidate `u16@+0x0a` fits BossGbln (3) but not Goblin (18).

The graphics key sits at `+0x16` and maps `ENEMY<NNN>.EMI` files: `key mod 200` matched all
105 of 105 distinct keys. `bank = key/200` (0–3) selects a variant — Lizard k45, ArmorF k245,
and Hopper k645 all share `ENEMY045`; EyeGoo k105 and Audrey k505 share `ENEMY105`.

AI action data sits at `+0x34`: four fixed 16-byte slots, each
`[tag][kind][id:u16][val:u16][0:u16][8-byte pattern]`. `kind` 0/5/6/7 are physical variants;
`kind 1` is a skill reference. `id<223` resolves against the skill table — 94 instances of
Leech Power on Goo-types and Nue confirm the mapping, plus 5 of Mind Sword. `id` 224–240
addresses a separate enemy-only skill space beyond the 222-entry table, which ends at the
level table `0x801cbb60`. Nue's `id226` falls in this unresolved range. `tag` behaves like a
weight or trigger, with `0x63` (99) dominant. The shared 8-byte patterns and the exact
per-turn selector remain open. `enemies.json` stores this as a structured `ai[]` array,
keeping the raw remainder as `aiTail`; the bestiary tags each move as a physical attack, Leech
Power, or a labeled special such as `#224`. `battle.ts` picks weighted real actions at
runtime: `kind=1` slots trigger the resolved skill (Leech Power drains AP, other skills apply
power damage; both approximated and labeled as such). A Playwright check confirms a MageGoo
battle uses Leech Power as expected.

Encounter formations occupy a 72-byte header at `0x800e4000`: 12 formations of 6 position
slots each, `u8` record index, `0xff` for empty. This validated disc-wide across 198 of 200
areas; AREA141 and AREA146 each have one outlier byte, treated as empty. Combinations read as
plausible groupings — Cedar mixes Ripper/EyeGoo/Goblin, AREA022-F10 pairs Zombie with Nue as
the boss formation. An open count of 772 slot references pointed to nameless records, status
unclear at the time: invisible participants, or leftover remnants. A closer pass on the
formation data found 108 references to nameless records that resolve to completely empty
entries, 136 bytes of null each — deactivated records that `build-chardata` filters out.
Formation data lands in `enemies.json` as `formations[]` plus a per-enemy `slot` field.
`battle.ts` picks a real formation from the area containing the chosen enemy, using A/B/C
suffixes for duplicates and a fallback copy heuristic. ⚠ Formation-selection weighting at
runtime is presumed random over non-empty slots but is unconfirmed.

### CLUT modes

| Asset | Format | CLUT source |
|---|---|---|
| Battle sprite cells (bosses, party, dragons, furniture) | uncompressed `[vc][cells]` | `ct8`: 16 entries of `[clut:u16][tpage:u16]` |
| `WARNING.EMI` piracy screen | raw 15-bit, 320×240×2 bytes | none — full-color frame, no codec |
| `DEMO.EMI` wall relief (opening fresco) | 8bpp, 1024px band, BPS=16 | CLUT 4 of the 4096-byte block at `0x80034000` |
| SHISU/BATE portrait and minigame-UI bands | 4bpp, 512px, BPS=4 | 16-entry sub-palette, column 0 of the second block at `0x80033c00` |

SHISU and BATE share the same two graphics bands, at `0x1c080200` and `0x1a080200`, with
CLUTs at `0x80033a00` and `0x80033c00`. Resolved at 4bpp, 512px, column 0, they hold character
portraits (Ryu, Nina, …) and the fairy-village/minigame UI ("Hunt/Clear/Build", "TIME"),
extracted to `entities/system/minigame_ui1.png` and `minigame_ui2.png`. ⚠ Correction: this is
not the 死す game-over screen once suspected — the field game-over image stays
runtime/dump-bound, while the battle game-over screen (`BATL_OVR`) is already solved.

### Bosses and host areas

Boss figure selection previously picked the largest coherent record across all animation
programs, which favored attack or effect frames over idle poses: Nue showed a claw-slash
instead of its chimera form, Golem a spin, Arwan a crescent moon, Sample 2 a lying pose,
Sample 4 scattered fragments. `build-boss-figures.ts` now applies `idleFirstPose`: the still
image is the `prog0`, `seq[0]` record whenever `op≥250 ∧ coh≥0.5`, falling back to a best-pose
scan only otherwise.

A QA veto layer — `enrich-community.ts` §10, plus a `SUPPRESS` list inside
`build-boss-figures` — withdraws boss-name-to-figure assignments that are visually wrong,
leaving the raw findings in `references/re/boss-names.json` untouched:

- `002:95` / `024:95` — id95 is a brown sludge sprite, not the intended dam-worker human.
- `134:403`+`15` / `162:403` — correct elephant geometry, but its object texels are not loaded
  in the host area, producing a patchwork; a sweep across all 32 sub-palettes confirms this is
  not a palette error.
- `103:371`/`633` — a monkey/fire object, not Charyb or Gisshan.
- `198:780`/`781`/`792` — fragments of Myria: 780 is wings only, 781 renders as a
  foreign-VRAM "text tube".

New curated figures, visually verified against host-area candidates: Worker = `002:94`,
Operator = `002:21`, Miner = `024:89` (pickaxe), Engineer = `024:91` (shovel), Foreman =
`024:93`, and Myria = `198:779` — a full, animated 69×86 goddess-with-wings figure, replacing
the earlier wings-only partial. This gives Miner and Engineer a figure for the first time, and
lets Nue, Golem, Arwan, Weretiger, and Garr show their real shape.

23 bestiary entries stay honest placeholders. 19 are story-event enemies — Gary, Mogu, Rocky,
Pooch, Torast Guard, Bullies, Claw, Cawer, Patrio, Dodai, Stallion, Gaist, Torch — whose name
assignment lacks battle-script proof and stays guesswork. The other 4 are vetoed figures,
Charyb, Gisshan, Ammonite, and Sample 6, whose real texels are not statically present in the
host area, presumably a battle-time upload as with the cases above. Gazer, Shroom, and
HugeSlug look unusual but are code-key- or curation-backed and treated as correct; Nina
(`041:249`) stays uncertain. Diagnostic tool: `scratchpad/w4b-probe-desc.ts` (area plus
descriptor ID → `prog0` plus every record, rendered as a strip).

### Assembling animations

`build-boss-figures.ts` emits program-0 frames for every named or BOSS-keyed figure that
passes a coherence gate shared with `build-enemy-anims` — at least 2 distinct records, anchor
drift ≤45% — producing `public/battle-sprites/bosses-anim/`: 14 animated bosses, 87 frames
total (Nue 11, Balio/Sunder 6, Weretigri 6, Myria 4, …). The bestiary plays these through its
sprite-animation loop.

Figure selection for the full bestiary (`figureFor`) now favors instances with an animation
entry, a +1.5 score bonus, since the animation gate is the strongest completeness proof
available. That change completes the Ripper, Goo family, and Bat entries instead of leaving a
fragment still image. Non-animated candidates instead need `coh≥0.78 ∧ smooth≥0.62`. Visually
misassigned event figures — Worker, Guard — move to a `BROKEN_FIGURES` blacklist and render as
honest placeholders rather than wrong art.

Across all 168 bestiary cards: 103 regular figures are all animated, 42 boss figures split
into 34 animated and 8 honestly static (`prog0`, `nSteps=1`), and 23 are placeholders; `tsc`
reports 0 errors and ENEMY019 matches ground truth 27/27. For regular enemies, apparent
"fragments" in static contact sheets — Bomber/PipeBomb showing a smoke cloud, GooTitan showing
berries — are artifacts of the still-image fallback (`poseCells` → `bestPoseRecord`), which
the UI never shows for animated entries. The actual `prog0` animation frames are correct,
confirmed by montage review of PainWeed, NutArchr, Thrasher, Curr, and Ghoul. Zombie/Ghoul/
ZombieDr are legitimate palette variants, not errors; the egg/gold-egg "MULTI" design is an
intentional gang encounter; the shadow blob under figures is ground-truth-legitimate, present
in the battle1 dump.

⚠ Remaining fine-tuning, found during sheet review: individual non-animated fragment poses on
ManTrap, Curr, Volt, Amalgam, Tricker, Phantom, LavaMan, Sleepy, Spiker, Dragnfly, FlyMan,
ToxicFly, Codger, and GntRoach, plus animations whose frame 0 is itself a partial frame
(Vulcan, D.Zombie, Reaper). Both categories need per-enemy pose/frame curation against the
Prima reference sprites.

### Extraction and coverage

`ENEMY<NNN>.EMI` is a pBAV-VAB sound bank, keyed by `graphics key mod 200`
(`enemy_record @+0x16`) — matching all 105 of 105 distinct keys — with `bank = key/200`
selecting the variant. `extract/build-enemy-sounds.ts` (`npm run extract:enemy-sounds`) fixes
two VAB quirks:

1. The VAG size table sits at the format position (`vh.length−512 = 0x820+numProg·512`) and
   also lists shared samples that sit outside the EMI's own VB — template remainder such as
   the "5744-er" sample present in 180 of 200 banks, unfindable in the common VAB and
   therefore never otherwise loaded or streamed. Since the EMI VB holds only the first `k`
   VAGs, sizes must be read cumulatively up to the VB end; the earlier `Σ·8==VB` search failed
   on exactly these banks.
2. SFX tones are single-note key splits: playback rate is
   `44100·2^((min−center−fine/128)/12)`, where `min==max` gives the note actually played.
   Tones with `vol=0` are mute placeholders, whose 352-byte VAGs decode to silence under a
   peak filter.

Result: 77 ENEMY banks hold real, distinct sounds (370 WAVs); 123 banks are complete dummies
that fall back to common battle SFX; a further 40 boss-own banks (113 WAVs) come from the
`addr==6` VAB inside the BOSS EMIs. Output: `public/sfx/{enemies,bosses}/`. The bestiary
exposes 217 sound buttons, keyed by `key%200`.

The Prima strategy guide's enemy compendium — pp. 124–129 of the local PDF,
`references/extmaps/misc/BoF3_Prima_…pdf`, legibly scanned — supplied a cross-check reference:
98 regular enemies with EXP, zenny, drops, learnable enemy skills, and sprite reference
images, transcribed into `public/gamedata/prima-bestiary.json`. No Prima enemy is missing from
the reconstructed roster: the 168 cards break down as the Prima 98 plus bosses plus event
figures. 72 names match directly; the rest resolve through short-form aliases (GntCrab → Giant
Crab, NutArchr → Nut Archer, Berserkr → Berserker, …). `enemies.json` now carries a
`prima {name, exp, zenny, skills, item1, item2}` block on 366 of 448 records. The bestiary
shows Prima EXP/zenny and learnable skills as a reference layer, pending a disc-exact
EXP-table reverse-engineering pass for which the Prima values themselves serve as ground
truth.

`WARNING.EMI`'s 153600-byte `ct0` is exactly one raw 320×240×2-byte 15-bit frame — the piracy
warning screen ("The unauthorised reproduction …") — which `build-etc-gfx.ts` extracts into
`entities/system/warning.png`, no codec or CLUT involved. `build-bgm.ts` now accepts `DEMO` as
a name over the same VAB+SEQ+VB trio, producing `public/bgm/demo.mp3`, 33.7 seconds of
attract-mode music. ⚠ Its graphics stay open: the 4 `ct3` bands read as noise under a
256px-stripe assumption, and the 262144-byte band is not raw16 512×256 either. An empirical
8-step sweep found the DEMO wall relief's format — 8bpp, 1024px, BPS=16, CLUT 4 of the block
at `0x80034000` — extracted to `entities/system/demo_mural.png`; 3 smaller 64KB bands beside
it stay unresolved (minor).

| Tool | Purpose / output |
|---|---|
| `extract/build-enemy-sounds.ts` (`npm run extract:enemy-sounds`) | Enemy scream banks → `public/sfx/enemies/`; boss-own banks → `public/sfx/bosses/` |
| `build-boss-figures.ts` | Boss still image (`idleFirstPose`) and program-0 animation frames → `public/battle-sprites/bosses-anim/` |
| `build-enemy-figures-static` / `build-enemy-anims` | Regular-enemy still images and animation frames |
| `build-chardata.ts` | Assembles `enemies.json` — name, graphics key, AI slots, formations, Prima cross-reference |
| `enrich-community.ts` §10 | Applies the QA veto layer against `references/re/boss-names.json` |
| `build-etc-gfx.ts` | `WARNING.EMI` piracy screen → `entities/system/warning.png` |
| `build-bgm.ts` | `DEMO.EMI` attract-mode music → `public/bgm/demo.mp3` |
| `scratchpad/qa-sheets.ts` | Bestiary QA contact sheets, replicating the live UI view |
| `scratchpad/w4b-probe-desc.ts` | Diagnostic: area + descriptor ID → `prog0` and all records as a strip |
| `scratchpad/BATTLE-SPRITE-FORMAT.md` | Full sprite-cell format spec |

### Refuted approaches

- **Battle sprite assembly as capture-bound or runtime-only.** Refuted by disassembling the
  resident sprite renderer at `0x8014c62c`: the format is static and disc-resident, identical
  to field mesh groups and PLCHAR `ct1` programs.
- **12-byte enemy name field.** Superseded by the 8-byte reading at offset `+0..+7`. The
  12-byte version pulled overflow garbage from the adjacent field ("NitemareE", "Assassin&",
  "BossGbln2", "Thanotos)", "Egg Gangq", "Mikbaj"); refuted once byte `+9` proved non-zero for
  Nitemare (`0x04`) and the Prima guide confirmed 8-byte short names.
- **Largest-coherent-record boss pose selection.** Superseded by `idleFirstPose`. Picking the
  largest coherent record across all animation programs favored attack and effect frames over
  idle poses (Nue's claw-slash, Golem's spin, Arwan's crescent moon, Sample 2's lying pose,
  Sample 4's scattered fragments).
- **SHISU-ct3 as the 死す game-over image.** Refuted once the band resolved to character
  portraits and fairy-village/minigame UI text at 4bpp/512px. The real field game-over image
  stays runtime/dump-bound; the battle game-over screen (`BATL_OVR`) was already solved
  separately.

### Open

- Regular enemy (`ENEMY###`) sprite geometry: the EMI holds only audio and the `ct8`
  CLUT/texture-page table, no `[vc][cells]` block. A synchronous active-battle dump would
  confirm whether geometry is built at load time.
- Formation-selection weighting at runtime is presumed random over non-empty slots but
  unconfirmed.
- 19 story-event enemies (Gary, Mogu, Rocky, Pooch, Torast Guard, Bullies, Claw, Cawer, Patrio,
  Dodai, Stallion, Gaist, Torch) lack battle-script proof for their name assignment.
- 4 vetoed figures — Charyb, Gisshan, Ammonite, Sample 6 — have no statically-resident texels
  in their host area, presumably a battle-time upload; Nina (`041:249`) stays uncertain.
- Enemy AI: `id` 224–240 addresses an unresolved enemy-only skill space (e.g. Nue's `id226`);
  the shared 8-byte patterns and the exact per-turn selector are undocumented.
- Enemy record level field has no confirmed offset: `u16@+8` collides with 8-character names,
  and candidate `u16@+0x0a` fits BossGbln but not Goblin.
- Per-enemy pose/frame curation remains for ManTrap, Curr, Volt, Amalgam, Tricker, Phantom,
  LavaMan, Sleepy, Spiker, Dragnfly, FlyMan, ToxicFly, Codger, and GntRoach (fragment poses),
  and for Vulcan, D.Zombie, and Reaper (partial frame 0).
- `DEMO.EMI` graphics (4 `ct3` bands) are unresolved, reading as noise under current
  stripe-width assumptions; 3 small 64KB bands near the DEMO wall relief are unresolved too
  (minor).

