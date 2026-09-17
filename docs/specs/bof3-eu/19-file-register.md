> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 19. File register: what produces what

Every asset under `public/` is generated. This table names the tool that writes it. The tools
live in `extract/`; `npm run bootstrap` drives them in eight stages.

| Asset under `public/` | Written by |
|---|---|
| `accession-transform` | `build-accession-vfx.ts`, `build-accession-video.ts` |
| `ambient.json` | `build-ambient.ts` |
| `area-catalog.json` | `build-area-catalog.ts` |
| `area-names-guide.json` | `build-area-catalog.ts`, `build-maptex-all.ts` |
| `area-names.json` | `build-area-catalog.ts`, `build-maptex-all.ts`, `build-worldmap.ts` |
| `area-titles.json` | `build-area-catalog.ts`, `build-fishing.ts`, `build-maptex-all.ts`, `build-titles.ts` … |
| `areas` | `bake-vertical-rt.ts`, `bgm-find-table.ts`, `bind-pages.ts`, `build-all.ts` … |
| `atlases` | `build.ts` |
| `battle-sprites` | `build-accession-video.ts`, `build-battle-sprites.ts`, `build-boss-figures.ts`, `build-dragon-anims.ts` … |
| `bgm` | `bgm-find-table.ts`, `bgm-gt-play.ts`, `build-battle-bgm.ts`, `build-bgm-all.ts` |
| `bmagic` | `build-bmagic-anim.ts`, `build-bmagic.ts`, `build-effect-vfx.ts`, `build-lineeffect-vfx.ts` … |
| `chests.json` | `build-chests.ts`, `build-npc-spawns.ts` |
| `clouds` | `build-cloud-tex.ts` |
| `deco` | `probe-w9-audit.ts` |
| `dialog` | `build-scena-flags.ts`, `scena-dialog.ts` |
| `emotes` | `build-emotes.ts` |
| `entities` | `build-blaze011.ts`, `build-chests.ts`, `build-dragon-icons.ts`, `build-ending-gfx.ts` … |
| `feattex` | `build-features.ts`, `build-runtime-geo.ts`, `probe-126-atlascrop.ts`, `probe-feattex-diff.ts` … |
| `feature-semi.json` | `build-feature-semi.ts` |
| `features` | `build-feature-semi.ts`, `build-features.ts`, `build-runtime-geo.ts`, `build-water-anim.ts` … |
| `fishing` | `build-fish-images.ts`, `build-fish-shadows.ts`, `build-fish-table.ts` |
| `fmv` | `build-fmv.ts` |
| `gamedata` | `build-bmagic-anim.ts`, `build-casino.ts`, `build-chardata.ts`, `build-dragon-moves.ts` … |
| `maptex` | `build-maptex-all.ts`, `build-maptex.ts`, `build-owwater-donor.ts`, `probe-ss-visual.ts` … |
| `masters` | `build-master-sprites.ts` |
| `meshes` | `build-meshes.ts`, `probe-w13-objmatch.ts`, `probe-w9-audit.ts` |
| `npc-spawns.json` | `build-npc-spawns.ts` |
| `npcscripts` | `build-ambient.ts`, `build-chests.ts`, `build-npc-spawns.ts`, `build-savepoints.ts` … |
| `npcsprites` | `build-npc-spawns.ts` |
| `objanchors` | `build-objanchors.ts`, `build-objinspect.ts`, `probe-w9-audit.ts` |
| `objinspect.json` | `build-objinspect.ts` |
| `overlays` | `build-overlays.ts`, `scene-compile.ts` |
| `recon` | `export-recon.ts` |
| `savepoints.json` | `build-savepoints.ts` |
| `scena-api.json` | `scena-api.ts` |
| `scene` | `scene-compile.ts` |
| `sfx` | `build-enemy-sounds.ts`, `build-enemy-voices.ts`, `build-fieldsfx.ts`, `build-party-cues.ts` … |
| `skyband` | `build-skyband.ts` |
| `skyband.json` | `build-skyband.ts` |
| `slidedoors` | `build-slidedoors.ts` |
| `slidedoors.json` | `build-slidedoors.ts` |
| `spawns.json` | `build-spawns.ts`, `build-worldmap.ts` |
| `spellfx` | `build-spell-replay.ts` |
| `texfix` | `scene-compile.ts` |
| `text` | `build-fishing.ts`, `build-objinspect.ts`, `build-texts.ts` |
| `tilepatches` | `build-tilepatch-tex.ts` |
| `walltex` | `build-walltex.ts`, `build-water-anim.ts`, `probe-w13-198front.ts`, `probe-w13-borrowcount.ts` … |
| `warps.json` | `build-warps.ts`, `build-worldmap.ts` |
| `water` | `build-owwater-donor.ts`, `build-water-anim.ts`, `probe-126-glyphs.ts` |
| `waterglanz` | `build-waterglanz.ts` |
| `weretiger-transform` | `build-weretiger-vfx.ts` |
| `worldmap-ui` | `build-worldmap-ui.ts` |
| `worldmap.json` | `build-worldmap.ts` |
| `xa` | `build-xa-cues.ts`, `build-xa.ts` |

### Bootstrap stages

Order matters: later stages consume what earlier ones produce. `npm run bootstrap -- --list`
prints this plan, `--from <n>` resumes at stage *n*.

**1. World geometry & tile textures (base for everything else)**  
   `build-maptex-all.ts` · `build-walltex.ts` · `build-owwater-donor.ts` · `build-meshes.ts` · `build-feature-semi.ts` · `build-cloud-tex.ts` · `build-waterglanz.ts`

**2. World content (spawns, warps, chests, objects, map)**  
   `build-area-catalog.ts` · `build-titles.ts` · `build-spawns.ts` · `build-warps.ts` · `build-chests.ts` · `build-ambient.ts` · `build-savepoints.ts` · `build-npc-spawns.ts` · `build-objinspect.ts` · `build-slidedoors.ts` · `build-smoke.ts` · `build-emotes.ts` · `build-worldmap.ts` · `build-worldmap-ui.ts` · `build-scena-flags.ts`

**3. Field characters & portraits**  
   `build-plchar-anim.ts` · `build-npc-sprites.ts` · `build-master-sprites.ts` · `build-portraits.ts` · `build-dragon-icons.ts`

**4. Battle figures (enemies, bosses, dragon forms)**  
   `build-enemy-pages.ts` · `build-enemy-sprites.ts` · `build-enemy-figures-static.ts` · `build-enemy-anims.ts` · `build-boss-figures.ts` · `build-battle-sprites.ts` · `build-dragon-sprites.ts`

**5. Effects (spell replays, transformations)**  
   `build-bmagic.ts` · `build-bmagic-anim.ts` · `build-weretiger-vfx.ts` · `build-accession-vfx.ts`

**6. Audio (BGM, SFX, voices, XA) — the longest section**  
   `build-bgm-all.ts` · `build-battle-bgm.ts` · `build-sfx.ts` · `build-fieldsfx.ts` · `build-enemy-sounds.ts` · `build-enemy-voices.ts` · `build-xa.ts` · `build-xa-cues.ts` · `build-track-names.ts`

**7. Data, texts & system graphics**  
   `build-texts.ts` · `build-chardata.ts` · `build-fishing.ts` · `build-fish-images.ts` · `build-fish-shadows.ts` · `build-fish-table.ts` · `build-fairies.ts` · `build-casino.ts` · `build-uifont.ts` · `build-etc-gfx.ts` · `build-ending-gfx.ts` · `build-system-screens.ts` · `build-fmv.ts`

**8. Replay curated layer (MUST run last)**  
   `enrich-community.ts` · `sync-data.ts`

Total: 62 steps.

### Tool catalogue

`extract/` holds 345 tools with a stated purpose: 107 pipeline steps,
161 research probes and 77 shared modules.

#### Pipeline steps (`build-*`)

| Tool | Purpose |
|---|---|
| `build-accession-vfx.ts` | Ryu's dragon transformation VFX (skill 151 "Accession" → MAGIC151) |
| `build-accession-video.ts` | Reference MP4 of the child-Ryu Accession — 1:1 per the EMULATOR GT |
| `build-ambient.ts` | AMBIENT ENTITY SPAWNS from the init scripts — op 0xe0 (7 bytes): [e0][typ][00][col][00][row][param] |
| `build-area-catalog.ts` | AREA CATALOG: consolidates |
| `build-bate-gfx.ts` | Cut FISHING GRAPHICS from the GT dumps. The catch screen shows the fish image, HUD digits, and the rank graphic as their own prim |
| `build-bate-sheets.ts` | MAKE BATE GRAPHICS BLOCKS VISIBLE. sub[1]/[2] are 32 KB image data each, sub[3]/[4] are 512 B CLUT each (= 16 palettes of 16 colors |
| `build-battle-bgm.ts` | Boss→battle-theme table: BOSS###.EMI pQES matched byte-exact against BGMBAT* EMIs (35 bosses carry their battle BGM as their own pQES subfile; BGMBAT0… |
| `build-battle-sprites.ts` | Static battle sprite extractor. Sprites WITH ct1 (BPLCHAR/BOSS) |
| `build-bgm.ts` | BGM renderer: BoF3 BGM (PSX SEQp + VABp, purely from the disc) → stereo WAV. A BGM EMI has 3 subfiles |
| `build-blaze011.ts` | AREA011 fire flames (type-15 class): the burning McNeil treehouse (pd011b) draws its 7 fire columns as INDIVIDUAL 32×32 S1 quads with two flame cells … |
| `build-bmagic-anim.ts` | SPELL ANIMATION EXTRACTOR (frame decomposition). Complements build-bmagic.ts: that wrote the |
| `build-bmagic-palettes.ts` | the REAL 4bpp palette choice of the spell effects (audit finding #6 |
| `build-bmagic.ts` | SPELL VFX EXTRACTOR (RE roadmap). The 144 BMAGIC effects (141 MAGIC###.EMI + KAIZAR_{D,F,N}) are the battle spell/skill effects. Each EMI bundles thre… |
| `build-boss-figures.ts` | BOSS FIGURES static from the disc. ⚠ The BOSS###.EMI files contain NO graphics |
| `build-casino.ts` | faerie-village CASINO disc-exact (F round): COMMU02.EMI sub8 (ct0 overlay @0x801d0c00) carries the complete casino logic. Disc core (disasm evidence) |
| `build-chardata.ts` | extracts BoF3's battle/character DATA TABLES (RE roadmap B.3). Sources (disc-canonical) |
| `build-chests.ts` | W10i (§6.1 CHESTS SOLVED): chest/sack spawns from the INIT scripts of all areas. Mechanism (GT AREA102, chest102.sav + recon; KNOWLEDGE "W10i") |
| `build-cloud-tex.ts` | OVERWORLD CLOUDS from GT dump (audit loop, fish45_1): the world-map clouds are TWO-LAYERED, both layers from the same runtime texture class typ 11 |
| `build-doc-pages.ts` | Renders this reference to a standalone reading page (`KNOWLEDGE.html`) |
| `build-dragon-anims.ts` | DRAGON/SPECIAL-FORM BATTLE ANIMATIONS — COMPLETE (Rewrite). Previously only an IoU-clustered idle series + at most ONE "meaty" action was emitted |
| `build-dragon-icons.ts` | DRAGON-GENE ICONS disc-static. Source BATL_DRA.EMI (battle-time replacement of the |
| `build-dragon-moves.ts` | Dragon move menu table: the battle menu builder 0x800a78b8 reads formId = lbu [0x800b6f58] → BASE row = u8 TRIPLE @0x800b4ea4 + |
| `build-dragon-recipes.ts` | DRAGON BASE-FORM GENE RECIPES (Q2, disc-exact): the special-form resolver 0x800a6c2c checks 11 base-form records of the table @0x800b4d58 (3 B per for… |
| `build-dragon-sprites.ts` | DRAGON TRANSFORMATION SPRITES + SINGLE-CHARACTER BATTLE SPRITES (battle sprite format). |
| `build-dragons.ts` | BoF3's DRAGON-GENE system (RE roadmap phase C / "BoF3 specials"). |
| `build-effect-vfx.ts` | GENERALIZED extractor for BMAGIC effects WITHOUT a texture sheet (the ones |
| `build-emotes.ts` | emote/field-particle sprites 1:1 static from the disc (W7a). System (fully disassembled, GT-verified via DuckStation F8 dumps in AREA000) |
| `build-ending-gfx.ts` | graphics MISSED during the disc inventory (coverage audit, via |
| `build-enemy-anims.ts` | extracts ALL animation programs of the regular enemies STATICALLY from the disc. |
| `build-enemy-figures-static.ts` | assembles regular enemy FIGURES 1:1 STATICALLY from the disc. |
| `build-enemy-figures.ts` | ENEMY FIGURES (assembled) from battle GPU dumps — RE roadmap B.1c. ⚠ HONEST FINDING (rigorously verified, see dossier): the battle-enemy sprite ASSEMB… |
| `build-enemy-pages.ts` | ENEMY SPRITE PAGES (RE roadmap B.1, BREAKTHROUGH). The "blocked enemy sprite codec" that stood for years was a phantom — the enemy pixels sit RAW AND … |
| `build-enemy-sounds.ts` | MONSTER SOUNDS: the 200 /BIN/BENEMY/ENEMY###.EMI are pBAV VAB banks (format = BGM |
| `build-enemy-sprites.ts` | INDIVIDUAL ENEMY SPRITES (RE roadmap B.1b). Cuts the 200 encounter BANDS (public/battle-sprites/areas/areaNNN_p0.png, from build-enemy-pages.ts) into … |
| `build-enemy-voices.ts` | ENEMY BATTLE VOICES (Q1, disc-proven): the battle engine plays enemy voices as bank-6 cues via the UNIVERSAL ENEMY ct8 table (16 records, identical ac… |
| `build-etc-gfx.ts` | ETC/BATL graphics: statically decode the graphics EMIs classified during the disc inventory (ct3 stripe 256px @8bpp, BPS=4, like BMAGIC; CLUT = embedd… |
| `build-fairies.ts` | the FAERIE VILLAGE system of BoF3 — RE roadmap subsystem 12. BoF3 side system: a village of fairies ("faeries") that you assign jobs to; plus a casino… |
| `build-feature-semi.ts` | FEATURE SEMI HARVEST: the game draws some feature quads half-transparent (106 river water wf680b… = semi=3 additive ×¼ over dark map water) |
| `build-features.ts` | Exports the feature geometry per area for the browser (world formulas + texture system verified via MIPS RE — see references/KNOWLEDGE.md) |
| `build-fieldchar.ts` | Extracts a sprite (field character/NPC/decoration) from a GPU dump → transparent PNG (real bounding box, undistorted). Filter by clut cell; optionally… |
| `build-fieldsfx.ts` | Field SFX extractor: the engine holds 7 SFX banks with cue tables @0x801486a0+bank·0x7c (PlaySfx 0x8015e1cc: cue = flags\|bank(11-8)\|sample(7-0); |
| `build-fire.ts` | Extracts the chimney-fire sprite (runtime entity of the inn rooms, McNeil) from a GPU dump as an animated frame sequence. The fire is a 4bpp quad clut… |
| `build-fish-images.ts` | ALL 22 FISH IMAGES — texels from the GT dump, palettes from the disc. Why the mix? (proofs: probe-bate-gfx.ts / probe-bate-fish.ts) |
| `build-fish-shadows.ts` | FISH SHADOWS in the fishing water — the "swim animation". RESOLUTION: fishing runs IN THE WORLD SCENE (character on the shore, water = map texture). T… |
| `build-fish-table.ts` | FISH TABLE from the resident item segment. Finding: the fish are stored as ordinary ITEMS in the resident item table @0x801c8xxx. |
| `build-fishing.ts` | maps BoF3's FISHING system (RE roadmap: "fishing", previously open). FINDINGS OVERVIEW (verified vs. interpreted vs. open — see _ fields in the JSON) |
| `build-fmv.ts` | FMV extraction: /LOGO/CAPCOM30.STR = the ONLY FMV on the disc (the game intro is |
| `build-item-icons.ts` | MENU ITEM ICONS from the system font sheet. FIRST.EMI sub[3] ctype3 (4bpp, 128 px) |
| `build-lineeffect-vfx.ts` | GENERALIZATION of build-weretiger-vfx.ts: extracts the frames of arbitrary BMAGIC effects that emit their VFX as GPU LINES (no texture sheet) and ther… |
| `build-maptex-all.ts` | All areas as top-down per-tile-textured maps → public/maptex/*.png + index.json. |
| `build-maptex.ts` | Render an area's top-down tile map from the ROM (correct colors+textures, EMI-only). Uses the verified renderer in rom-tiles.ts. Output: public/maptex… |
| `build-mast.ts` | MAST decoration sprite (typ 1) (audit loop): typ 9 @045 (clut 16,483, pg 704,256, 4bpp |
| `build-master-sprites.ts` | MASTER FIELD SPRITES: renders each of the 17 masters (masters.json) its field sprite as a frame strip from the home area's EMI → public/masters/mID.pn… |
| `build-masters.ts` | the MASTER/TEACHER system of BoF3 (RE roadmap phase C). BoF3 core mechanic: characters join masters; as long as you follow them, the master MODIFIES t… |
| `build-menu-assets.ts` | FIELD MENU ASSETS (round 26a). Sources: 1. GROUND TRUTH: references/gpudump/menu-field.sav — DuckStation savestate with an OPEN field menu |
| `build-meshes.ts` | 3D object meshes (W5w, statically completed in W7c): exports the 40-B quad meshes (0x80117000 block + subfile-internal blocks) with instances from TWO… |
| `build-npc-spawns.ts` | L1.5: NPC SPAWNS with REAL sprite keys from the INIT scripts of all areas. Mechanism (GT mcneil_ent.sav ↔ disasm, KNOWLEDGE "L1.5") |
| `build-npc-sprites.ts` | NPC SPRITE ROSTER — more field NPC variety instead of the one village woman. The living NPCs (src/systems/npcvm.ts) previously ALL used public/entitie… |
| `build-objanchors.ts` | OBJECT-ANCHOR TABLE (RE roadmap L1.4). The per-area init overlay (RAM 0x801f2c00 |
| `build-objinspect.ts` | OBJECT INTERACTIONS (W14, §6.1 "param→mesh" SOLVED — as a gaze-interaction system): The object anchor table (init overlay, script struct +0x2c, count-… |
| `build-oceanwaves.ts` | WORLD-MAP WAVE textures (anim stage B): the overworld seas draw a RUNNING wave layer — huge additive S1 quads (pg704, CLUT(176,483) 4bpp = type 11) th… |
| `build-overlays.ts` | Extracts ENTITY-OVERLAY quads (doors, gates, facade decorations) from GPU dumps: registers the scene (like probe-wallverify), collects at every edge t… |
| `build-owwater-donor.ts` | Overworld deep-water waves via DONOR: area087/121 carry real GT water phases (W5s dump series, public/water). The remaining overworld areas use the SA… |
| `build-party-battle-anims.ts` | PARTY BATTLE ANIMATIONS COMPLETE. Extends build-party-battle-sprites.ts (which only |
| `build-party-battle-sprites.ts` | PARTY BATTLE SPRITES (battle sprite format). The BPLD/BPLU/BRTD/BRTU files in BIN/BPLCHAR bundle the party members as battle figures (D=front view, U=… |
| `build-party-cues.ts` | Party battle CUES: the BPLD VAB trios carry their official battle cue records [0][0x80\|prog][note][vol] in ct8 (banks 3-5 in battle = party slots). |
| `build-plantblink.ts` | PLANT-BLINK UNIT (audit loop): type 14 @049 „Plant" = 5 mini quads (clut 224,483 · pg 704,256 · 4bpp, UV region ~[192,64..208,86]) — small machine/ |
| `build-plchar-anim.ts` | Builds a CLEAN, directional field sprite set (Teepo + Ryu) from DuckStation savestates by decoding the PL034 ct1 geometry (the record DRAWN at runtime… |
| `build-plchar-anims.ts` | PARTY FIELD ANIMATIONS COMPLETE. Extends build-plchar-frames.ts: instead of ONE |
| `build-plchar-frames.ts` | FIELD CHARACTER SPRITESHEETS (RE roadmap L1.5 / sprite assets). FULLY STATIC from the |
| `build-plchar-model.ts` | Extract the PLCHAR field character model (PL034 = the player character loaded in AREA007). STATUS(honest partial success) |
| `build-plchar-sprite.ts` | Builds textured PL034 sprite-composite candidates: scans the decompressed ct1 for format-A sub-meshes (5-byte records [flag][Xs][Ys][U][V]), renders e… |
| `build-portraits.ts` | MENU PORTRAITS disc-static. The 40×48 character portraits of the field menu sit |
| `build-rgeo-from-grid.ts` | RGEO BAKE FROM THE WINDOW GRID: the "royal road" of the fork report on the 178/179 wall-stack mode as the bake path. The game fills the slot pool part… |
| `build-rgeo-scene-from-dump.ts` | RGEO SCENE SOLVER (W14, generalized from scratchpad/w13b-bridge-solve.ts): resolves the per-area CODE quads (target page/CLUT) of a GT dump into REAL … |
| `build-rgeo-seed-from-dump.ts` | Lift RGEO SEED from a GT GPU dump: filters the quads of a target page/CLUT (per-area CODE objects like 067 bridges / 055 Yggdrasil, which do NOT come … |
| `build-runtime-geo.ts` | Runtime geometry bake (W13b): per-area code objects (e.g. the 067 Maekyss bridge) that the game builds as GPU quads at runtime (pg/clut class WITHOUT … |
| `build-savepoints.ts` | W10k (§6.1 SAVE POINTS): the BoF3 save points are the round SAVE PADS ("Record your progress") — map geometry (red basin + 4 pillars, already rendered… |
| `build-scena-flags.ts` | SCENA FLAG HARVEST: which NPC/which cutscene sets which story flag? MOTIVATION: this exact mapping was missing for story chaining — "which cutscene se… |
| `build-sfx.ts` | SFX EXTRACTOR (RE roadmap L1.6). The sound-effect banks are pBAV VABs — EXACTLY the |
| `build-shops.ts` | SHOP INVENTORIES: SHOP.EMI overlay 0x801d0ff4 sets the shop-list |
| `build-skyband.ts` | SKYBAND BAKE: the 060 canyon sky is a 256×64 cloud band (pg(576,256) 8bpp, CLUT (0,486)) that the engine draws as 3 side-by-side screen quads |
| `build-slidedoors.ts` | CAER XHAN SLIDING DOORS. RE FINDING (GT tuer147/tuer147open + EMI scan, KNOWLEDGE "sliding doors") |
| `build-smoke.ts` | Extracts the McNeil chimney smoke (dark swirling column, clut(160,483)) from a GPU dump as a cleanly proportioned transparent PNG. Box = actual boundi… |
| `build-smokepuff.ts` | Extracts the ORIGINAL smoke-puff texture (32×32) of the McNeil chimney smoke from the AREA EMI |
| `build-spell-replay.ts` | SPELL-REPLAY extractor: BMAGIC spells as PLAYABLE choreography for the world (instead of pre-rasterized frame movies). The ct0 interpreter (references… |
| `build-system-screens.ts` | System screens (S4) — reconstruct the title screen + opening mural from GPU dumps. These screens (START.EMI/…) are drawn in-game NOT as a 32x32 tile t… |
| `build-teepo-adult.ts` | W9 §5: scan all sprite descriptor keys of the AREA011 EMI — find flame programs. Renders each program's frame 0 (+ frame count) as a PNG into the scra… |
| `build-texts.ts` | GAME TEXT EXPORT: ALL area text blocks from the disc → public/text/. Every AREA###.EMI carries |
| `build-tilepatch-tex.ts` | TILE PATCH TEXTURES (U3): renders the AFTER states of the [20] texture patches (cf. build-tilepatches.ts) as 16×16 tiles. Method = full engine semanti… |
| `build-tilepatches.ts` | MAP HEADER [20] TABLE = STATE TEXTURE PATCHES (T2, disc-exact): u16[20]·4 in the map sub (@0x80104000) points to GROUPS |
| `build-track-names.ts` | Track title mapping (W12 music): builds public/bgm/track-names.json — real OST/Special Box titles for the extracted BGM MP3 blocks, WITH confidence. |
| `build-ui-icons.ts` | UI ICONS (field menu + battle commands) disc-static & individual. SOURCE: FIRST.EMI sub3 @0x1e000200 = the field-resident font/UI band → VRAM (960,0),… |
| `build-uifont.ts` | UI/FONT SHEET EXTRACTOR (RE roadmap L1.3). The system graphics (font, window frame |
| `build-walltex.ts` | Wall textures per area (system v3) → public/walltex/area<NNN>.png (packed atlas, deduplicated |
| `build-warps.ts` | Extracts the IN-WORLD WARPS (door/passage → target area + position) from every AREA EMI. Source: subfile with RAM address 0x801f2c00 (per-area init ov… |
| `build-water-anim.ts` | Cell ANIMATION patches (W5s "The Middle Sea", generalized in W10): the game cyclically replaces the texels of fixed map cells via VRAM upload ([17]-ty… |
| `build-waterglanz.ts` | WATER-GLINT overlays (audit loop round 2): type-6 runtime quads (clut 96,483 · pg 704,256 · 4bpp · semi=3 = B+F/4, color 0x808080) — subtle additive |
| `build-weretiger-vfx.ts` | Rei's weretiger transformation VFX (skill 64 → MAGIC064) STATICALLY from the disc. |
| `build-weretiger.ts` | Rei's weretiger battle sprite STATICALLY from the disc (no emulator/dump). |
| `build-window-themes.ts` | WINDOW THEMES of the game menu, disc-static. Config option „Set window color" (tableA[191]): the menu recolors frame + fill via SUB-PALETTES |
| `build-windrad.ts` | WINDRAD (pinwheel) decoration sprite (audit loop): typ 9 @045 (clut 144,483, pg 704,256, 4bpp |
| `build-worldmap-ui.ts` | OVERWORLD HUD assets disc-static from AREA016.EMI (GT evidence: GPU dump, Yraall |
| `build-worldmap.ts` | Builds the WORLD GRAPH (L2.3): connects all 200 areas — including the region overworld maps — into |
| `build-wyndia-rad.ts` | LARGE WYNDIA WINDMILL WHEEL (user report): type 9 @069 (clut 144,483 · pg 704,256 · 4bpp) = multi-part runtime wheel on the town gables: 4 BLADES (32×… |
| `build-xa-cues.ts` | XA CUE CATALOG: the engine plays XA via XA_PLAY(cue) @0x80163954 → start routine 0x801639f8 |
| `build-xa.ts` | XA audio extractor (K4· FULL extractionevening). Decodes the STR/XA streams |

#### Research probes (`probe-*`)

A probe proves a finding; probes are not part of the bootstrap. They stay in the tree because
the evidence for a claim is the probe that produced it.

| Tool | Purpose |
|---|---|
| `probe-082-discriminate.ts` | F5/fix-B: discriminator search. For every 0x82 group (dstFilled>0): distinct 8bpp indices in |
| `probe-082-sweep.ts` | F5/Fix-B regression sweep: all 0x82-[17] groups of all areas. For each: dstFilled/srcFilled |
| `probe-126-atlascrop.ts` | F5: extract atlas regions of specific keys from feattex/areaNNN.png into a montage PNG (10x zoom) |
| `probe-126-glyphs.ts` | F5/fix-B diagnosis (126 SE water-line glyphs): replicates build-features.ts to clarify (a) which feature quads are the glyph boxes (VRAM source, opaci… |
| `probe-126-vram.ts` | F5: dump the VRAM window of AREA126-EMI as an 8bpp image (CLUT row selectable), montage. Clarifies whether the g3 source (48,208) is water or a font r… |
| `probe-addr-audit.ts` | ADDRESS AUDIT: check every RAM address constant in code and docs against the RAM dump. |
| `probe-anim-disasm.ts` | Probe 8: disassemble the Anim-CLUT routine 0x80159f8c (Y=481, <<5\|0x10) broadly, incl. the referenced anim counter variable @0x8014xxxx. Goal: read o… |
| `probe-anim-writer.ts` | Probe 7: find in RAM (ram1, AREA007) the routine that writes VRAM rows 480/481/482 (= palette animation, per the knowledge doc, runtime). VRAM-Y 480=0… |
| `probe-anim480.ts` | Probe 11: are VRAM rows 480/481/482 ANIMATED CLUTs, and which code produces them? - structure of the 3 rows (resident in all areas, identical): gradie… |
| `probe-animrows.ts` | Probe 6: examine VRAM rows 480-482 (per the knowledge doc "palette animation, runtime, not EMI") |
| `probe-areaload.ts` | AREA LOAD MECHANISM (offline RE from references/gpudump/ram1.ram.bin, SLES-01304). This probe documents + verifies the pokeable "load area N" chain an… |
| `probe-areaxref.ts` | Scanner: finds all code spots that reference a given RAM address. Tracks lui rt,hi → later lw/sw/lhu/sh/lbu/sb/addiu/ori rs==rt with matching offset. |
| `probe-bate-fish.ts` | FIND FISH IMAGES IN THE DISC. Reverse search: the fish texture visible in the dump VRAM (page 768,256) and its CLUT |
| `probe-bate-gfx.ts` | LOCATE BATE GRAPHICS STATICALLY. Goal: find the graphics bands (sub[1]/[2], ct3) and the CLUT blocks (sub[3]/[4]) of BATE.EMI |
| `probe-bate.ts` | STATICALLY ANALYZE the BATE OVERLAY (fishing minigame) — /BIN/ETC/BATE.EMI, sub[0] = code at RAM 0x801d0c00 (33,864 B ≈ 8466 instructions). This makes… |
| `probe-battle-formel.ts` | B.4 battle-formula verification: checks the DISC-EXACT disassembled physical damage formula against the 42 GT samples from re-work/damage-trace/sweep2… |
| `probe-bed.ts` | Measures the furniture quads around an anchor in tile space: decomposes each quad edge into |
| `probe-bestiary-cells.ts` | diagnostic for bestiary pixel errors (noise pixels/bar artifacts). |
| `probe-bgm-audit.ts` | Structural audit of all BGM EMIs: VAB program occupancy (slot vs. packed), program numbers used by the SEQ, note-range coverage, ignored MIDI events (… |
| `probe-bgm-blocks.ts` | PROBE (W12 music): block inventory of all BGM EMIs + bgmId table (tbl209) from the EXE. Purpose: title-mapping anchor — (file, block) → mp3 name (main… |
| `probe-bgm-pitch.ts` | looks for pitch risks in BGM playback. Checked per track and per SONG BLOCK |
| `probe-bill-float.ts` | F5/Fix-A-Sweep (corrected): Math.max(ankerY, groundTop) changes the foot height ONLY when groundTop > ankerY (bill gets RAISED). This set is affected … |
| `probe-bill-raw.ts` | Hexdump aller TYPE-0x27-Billboards einer Area: Header-Bytes, Frames, Spannen, Roh-Payload. npx tsx extract/probe-bill-raw.ts [area=008] |
| `probe-billwords.ts` | Lists all billboard texture words (TYPE 0x27) of an area: word, b3 decomposition per the exact texture routine 0x801557d4 (column=page&3, bit31=4bpp, … |
| `probe-blackcell.ts` | Searches a GPU dump for all textured prims that sample a specific 16×16 cell of a page window (e.g. the "black" slope cell p1:(0,0)) and shows CLUT |
| `probe-boss055-offsets.ts` | MIPS disasm of the BOSS055 ct0 choreography around the three descLookup |
| `probe-bossct0-keys.ts` | script evidence search for the keyless arena/event enemies (W11-P1) |
| `probe-callers.ts` | Finds all jal calls to a target address. Shows context (a0-a3 setup beforehand). npx tsx extract/probe-callers.ts 0x8019fca0 [ctxBefore] |
| `probe-clut-routine.ts` | Probe 4: broadly disassemble the candidate routines that use VRAM-Y 483 (0x1e3) as a CLUT upload target. Look for the pattern: frame counter / running… |
| `probe-clutcycle.ts` | Probe 3: search the resident RAM (ram1 = AREA007) for a CLUT-cycle/water-animation routine. Strategy |
| `probe-combo-polys.ts` | Lists all textured prims of a (pageX,pageY,clutX,clutY) combo from a GPU dump: screen bbox, UV rect, type — to understand HOW an object (e.g. the larg… |
| `probe-ctype8.ts` | Probe 12: decode ctype8 descriptors of AREA008. Per the battle docs: ctype8 = (TPage,CLUT) descriptors per graphics subfile/sprite group. |
| `probe-darktile.ts` | Diagnostic for near-black maptex tiles: texture entry, cell, CLUT content in EMI VRAM vs live VRAM (references/gpudump/vram-<area>.bin), pixel indices… |
| `probe-disasm-dec.ts` | Disassembles the decode routines from live battle RAM (deterministic, no guessing): 0x8014e820 dispatcher, 0x8014e948 mode-1 handler, 0x8014ea4c mode-… |
| `probe-disasm.ts` | Disassembles an arbitrary RAM range from ram1.ram.bin. npx tsx extract/probe-disasm.ts <hexAddr> [lines=60] |
| `probe-disptables.ts` | Reads the decode dispatcher tables from the live battle RAM: 0x80145a00 count, 0x80145a04 ptr[], 0x80145a54 w[], 0x80145a7c h[] |
| `probe-dragon-coverage.ts` | COMPLETENESS MATRIX of dragon splicing (W11 follow-up round). Enumerates ALL splices possible in the game (1-3 genes out of 18; fusion × 10 partner pa… |
| `probe-dragon-hybrid-disasm.ts` | static disasm of the hybrid partner resolver 0x800a805c (battle overlay in the battlecap12.sav RAM) + search for the transform sprite load path. |
| `probe-dragon-palettes.ts` | PROBE: dragon/special-form palette variants. Finding thesis: the element color variants of the dragon forms are CLUT ROWS within the same ct0 |
| `probe-dumpcells.ts` | Lists, per (TexPage,CLUT) combo, the used 16×16 cells (u0>>4,v0>>4) of all textured prims in a dump — for comparison with the map texture entries. |
| `probe-feattex-diff.ts` | F5: before/after atlas diff. Compares public/feattex/areaNNN.png against a backup, finds changed atlas keys (via features/areaNNN.json atlas positions… |
| `probe-features.ts` | Probe: parse the AREA000 feature block + check the texture-word hypothesis (idx → map textureData) against the UV bases of the GPU-dump object polys. |
| `probe-find-clut.ts` | PROBE: find the best CLUT row for the PL034 sprite page (640,0) 8bpp — across ALL VRAM rows. |
| `probe-findenqueue.ts` | Scans the live RAM code for all instructions that reference the decode-queue tables (immediate 0x5a00/0x5a04/0x5a54/0x5a7c) → finds the enqueue/setup … |
| `probe-heightcheck.ts` | Checks our map corner heights against the floor quads of a GPU dump: registers the 1×1 floor blocks (pipeline like probe-wallverify), fits screen = A·… |
| `probe-input-test.ts` | Diagnosis of live emulator control: (1) does the frame advance? (2) do arrow keys move Ryu? |
| `probe-item-desc-field.ts` | W12 items: thesis "every item record carries a DESC-INDEX u16 (idx & 0x3ff, flag 0x4000)" at a category-specific offset |
| `probe-item-descs.ts` | W12 items: calibration of the FIRST.EMI sub[11] description block (455 strings @0x8001b69c). Known: item id0-91 ↔ desc[0..91], skills id76-110 ↔ desc[… |
| `probe-items-abgleich.ts` | W12 items: EXPECTED/ACTUAL full cross-check of items+shops. EXPECTED: bof.fandom lists (MediaWiki API wikitext, scratchpad/fd_*.json) + Cosmo Canyon F… |
| `probe-keyless-hosts.ts` | diagnostics for the 11 keyless script/arena enemies (W11 follow-up P1) |
| `probe-livemap.ts` | Compares the LIVE map (ram1.ram.bin @0x80104000) with the AREA-EMI map subfile: header, heights, tileTexIdx and textureData entries — uncovers story/s… |
| `probe-master-gallery.ts` | PROBE (master sprites): frame-0 gallery of ALL sprite descriptor keys of an area |
| `probe-master-progs.ts` | PROBE (master sprites): renders ALL programs (0..15) for ONE sprite key of an area |
| `probe-mesh-match.ts` | Core evidence for mesh groups: project the vertex point clouds of the referenced groups at the |
| `probe-mesh-viz.ts` | Diagnostic image: renders frame quads flat + overlays projected mesh-group point clouds. Uses the same floor-fit pipeline as probe-mesh-match; world-t… |
| `probe-meshdecode.ts` | extract/probe-meshdecode.ts FINAL mesh-group decoder, decoded from the MIPS interpreter. RE RESULT (instruction-backed, ram1.ram.bin) |
| `probe-meshdisasm.ts` | Disassembles the mesh group interpreter region from ram1.ram.bin (cwd=project root). npx tsx extract/probe-meshdisasm.ts <startHex> [count=180] |
| `probe-mgdata.ts` | Raw decode of the format-A sub-meshes (01 01) from the AREA007 mesh group. Shows cmd1/cmd2/vc + the vc 5-byte vertices with flag byte, so the |
| `probe-myria.ts` | Myria giant form (AREA198, descs 780/781/792): programs, records, cell extents. |
| `probe-overlay-discrim.ts` | PROBE: does the "map texture discriminator" cleanly separate the overlay candidates rejected |
| `probe-owui-sheets.ts` | UI sheet regions of the overworld HUD from the GPU dump as PNG (8bpp, CLUT from VRAM row). |
| `probe-pairquads.ts` | Finds frame quads of a GPU dump via a UV window (for pair-wall forensics: read off the vertex↔UV correspondence without registration). |
| `probe-playerpos.ts` | Verifies the player/actor position addresses in PSX RAM. Goal: reliable source of the world/tile position of the field character for the autonomous |
| `probe-plchar-cell.ts` | PROBE: rasterize ONE field-character sprite cell from the savestate VRAM (paletted → real colors). |
| `probe-plchar-ct7.ts` | Probe: is the PLCHAR ctype7 decode the GENERIC (crackable) field loader (LZSS/5-bit, 0x8014e820) |
| `probe-plchar-final.ts` | FINAL: prove ct1 is parseable 3D geometry. Extract the decompressed PL034 model (RAM ground |
| `probe-plchar-montage.ts` | PROBE/DELIVERABLE: rasterize all pBAV cells of PL034 from the savestate VRAM (page 640,0, 8bpp |
| `probe-plchar-vram.ts` | PROBE: cut a field-character texture from the savestate VRAM (fallback for billboard NPCs). |
| `probe-plchar.ts` | Probe: PLCHAR ct1 (field character 3D model, EMI subfile @RAM 0x8003b800, ctype=1). FINDING |
| `probe-ptr.ts` | Searches the RAM for a 32-bit constant (function pointer / data value) as an LE word at 4-aligned offsets. |
| `probe-raminput.ts` | Finds ENEMY019's ctype7 INPUT (from disc) verbatim in the live battle RAM and searches via pointer scan |
| `probe-resume.ts` | Mini-Probe: GDB connect + read 0x80149308 + [resume-Methode], Tile ausgeben. Methode per arg: detach \| continue \| raw |
| `probe-roofoffs.ts` | Histogram of the xOff/yOff vertex values of roof (TYPE 0) and wall (0x10) features of an area |
| `probe-shop-cats.ts` | W12 items: definitive cat→table mapping of the item category lookup 0x80165ffc (jump table @0x80149f28, resident EXE SLES_013.04). The shop comparison… |
| `probe-shop-town.ts` | W12 items: Where does the shopId come from? Statically disassemble SHOP.EMI init 0x801d0ff4 → find the shopId source (global/argument); then find writ… |
| `probe-ss-region.ts` | VERIFIES the overlay registration of a savestate against the GROUND-TRUTH camera position: camTile sits precomputed in the camera block (RAM 0x801492e… |
| `probe-ss-visual.ts` | DEFINITIVE VISUAL VERIFICATION of a savestate's AREA registration: renders the floor tiles as the savestate shows them (texture from the savestate VRA… |
| `probe-star-match.ts` | Verifies the star-object ANCHOR formula (NW corner of tile (col-1,row)) against a GPU dump |
| `probe-swizzle.ts` | Rosetta stone: ENEMY019 ctype7 (input, from the disc) ↔ decoded VRAM page (output, from a savestate). |
| `probe-treetbl.ts` | Dumps the parameter tables of the star-object handler 0x801566a0 (TYPE 0x01..0x0E): 0x8017fc68 u8 count (planes per TYPE; loop s6) |
| `probe-treetype.ts` | Reads the feature dispatch table @0x8017fb34 from the RAM dump and disassembles the handlers of the small TYPEs (0x01..0x06 = suspected: large star tr… |
| `probe-unused-fields.ts` | UNUSED-DATA AUDIT: which extracted fields does the browser never read? REASON (lesson from the user after the Gaist finding): "If two pipelines read t… |
| `probe-usplit-gt.ts` | GT proof of the U-split/localU semantics (glitch-pixel diagnosis, bestiary). |
| `probe-vram-crop.ts` | PROBE: crop a region from the savestate VRAM as a BGR555 PNG (1:1, no CLUT — raw VRAM view). |
| `probe-vram-paletted.ts` | PROBE: VRAM-Page paletted (4/8bpp) mit gegebener CLUT-Zeile als PNG dekodieren (ganze Page sichten). |
| `probe-vramupload.ts` | Probe 13: is there a resident code routine that uploads to the MAP-texture VRAM region (Y 256-511, X 320-832) per frame (= water texture scroll/swap)?… |
| `probe-w12-apply-community.ts` | W12: idempotently enters the COMMUNITY detail blocks collected in the W12 research into fishing.json + fairy-village.json. |
| `probe-w12-fairyshops.ts` | search for the 6 FAIRY-MERCHANT INVENTORIES + antiques prices + |
| `probe-w12-fishspots.ts` | search for the per-spot FISH-POPULATION table disc-statically. |
| `probe-w12-fishspots2.ts` | W12 follow-up: decode the format of the per-spot fish-list table. |
| `probe-w12-master-gates.ts` | find the remaining master join gates disc-statically. W1-D found 7 gates (Fahl/D'lonzo/Emitai/Giotto/Mygas/Yggdrasil/Ladon) in the per-area INIT |
| `probe-w12-master-will.ts` | W12 masters audit, part 1: where does the game store the CURRENT |
| `probe-w12-party-defaults.ts` | W12 party comparison: where are the NEW-GAME default party records? |
| `probe-w12-sample40.ts` | W12 items: a 40-value sample across all categories as a markdown table (disc values from items.json; expected: fandom lists + Cosmo Canyon FAQ prices)… |
| `probe-w12-skill-usage.ts` | W12 SKILLS COMPARISON: enemy usage + acquisition relations |
| `probe-w13-045blank.ts` | 045: all referenced cells that are EMPTY in the probe045 live VRAM, but carry texels in our |
| `probe-w13-045ost.ts` | 045 east edge „confetti": referenced cells of the east columns + compare donor recon vs probe045 dump |
| `probe-w13-102chk.ts` | F1b: 102 chamber floor check — are the black-rendering, walkable index≠0 tiles of the two chambers empty BOTH in our recon AND in the GT dump? (=> GT-… |
| `probe-w13-108.ts` | F3: AREA108 placement inventory — how many objects/records ACTUALLY exist? List all plcBase entries + 0x117000 block size + ffff anchor candidates (in… |
| `probe-w13-198front.ts` | K3 198 diagnosis: why does the socket FRONT have a black gap (missing wall columns)? Dumps per-tile: index (solid?), 4 signed corner heights, walk, an… |
| `probe-w13-198gt.ts` | K3: x198flr GT analysis — which polygons form the base FRONT (skirt)? Lists all textured prims of the best frame with screen coordinates; we're lookin… |
| `probe-w13-198walls.ts` | G1: WHY do the socket rim corners lack walls (black gap) + is the cube too tall? Replicates collectAreaWalls' EXACT decision per tile: needS/needE (si… |
| `probe-w13-align.ts` | F2: determine the offset between GT dump VRAM and EMI reconstruction in the stripe region |
| `probe-w13-anim17.ts` | F2: Map-Header-[17]-Animations-Tabelle einer Area roh dekodieren (Format W9a) |
| `probe-w13-anim17b.ts` | F2: [17] groups of ALL types with the full 2nd header word, for several areas |
| `probe-w13-blacklid.ts` | G2: checks whether the maptex building lid is detectable via decodeTile as NEAR-BLACK (a reliable lid signal instead of the height heuristic). Decodes… |
| `probe-w13-borrowcount.ts` | G1 safety probe: how many edges would a "borrow adjacent same-face wall" corner-fill NEWLY render, across ALL areas? Replicates terrain.ts edgeAt exac… |
| `probe-w13-cellsheet.ts` | Comparison sheet: pg1 cells (0..1, 8..15) per source (area recons + dumps), decoded with the BlockA CLUT of a reference area. Output: PNG grid (source… |
| `probe-w13-coverage-g3.ts` | G3: Top-Down-Abdeckungskarte je Tile: maptex-Helligkeit + Roof-Quad-BBox + Cap-Abdeckung. |
| `probe-w13-dropop.ts` | K3: measure opaque% of every wall word per area, flagging renderWallImage-dropped (<5%) ones. |
| `probe-w13-dumpband.ts` | F2: GT dump VRAM band (page column, 8bpp, CLUT row) as PNG + UV list of the quads of a texpage/clut combo. npx tsx extract/probe-w13-dumpband.ts <dump… |
| `probe-w13-dumptex.ts` | F3: page-3 window (704,256) from the objpal108 dump VRAM with CLUT row 483 (column c) as PNG |
| `probe-w13-feat.ts` | F2 diagnosis: why build-features emits nothing for certain areas (081 + 404 family 086/102/103/105/107) resp. which roof/bill records affected areas c… |
| `probe-w13-flameinv.ts` | F3b: complete flame inventory — all TYPE-0x10 records with a 6-frame signature. Prints, per area, (col,row), w0 (=frame0 word → wall key 'w'+hex), ver… |
| `probe-w13-flamescan.ts` | F3: world-wide scan for the torch/campfire class: TYPE 0x10, size>=13: [4 verts][2 span][6 frames] (+ optional 2nd crossed-quad set) = flame object |
| `probe-w13-k1.ts` | K1 diagnostic: which referenced map texture cells of an area are empty/wrong in our EMI reconstruction (runtime upload window), and where would a seed… |
| `probe-w13-k3-browser.ts` | K3: browser wall simulation — replicates terrain.ts (edgeAt + emit) over the public/ assets |
| `probe-w13-k3-scan.ts` | K3: global scan — how many S/E edges fall into the classes (a) key present, atlas entry <50% opaque (transparency class → alphaTest fix visible) |
| `probe-w13-k3.ts` | K3 diagnostic: black areas on geometry (walls/caps). For an area, list all S/E wall edges (engine predicate, exactly like collectAreaWalls) and |
| `probe-w13-lidlist-g3.ts` | G3: lists, per area, the BUILDING-LID tiles (dark maptex tiles that are NOT part of a large dark area = water/border) with their coverage status. Clar… |
| `probe-w13-lists.ts` | F2: Draw-Listen 0x801459d0[mode] — Default-Mode-Byte + Verweise im EXE-Code finden |
| `probe-w13-objmatch.ts` | F3: Quad-genauer UV-Match: objpal108-Dump-Quads (pg704 clut48) ↔ plc[0]/plc[1]-Records |
| `probe-w13-objpal108.ts` | F3: objpal108-Dump — Boulder-Quads (Mesh-Klasse page(704,256)) mit CLUT + UV listen. npx tsx extract/probe-w13-objpal108.ts [dumpname] |
| `probe-w13-page.ts` | F2: ganzes Textur-Page-Fenster als PNG dekodieren (8bpp oder 4bpp, wählbare CLUT-Zeile) |
| `probe-w13-roofbury.ts` | G2: tests whether roof quads sit UNDER the maptex building lid (→ occluded = black). For every roof record: roof Y range (from z, /128) vs terrain til… |
| `probe-w13-roofcolorg3.ts` | G3: for each built roof record, print the RENDERED color (atlas avg × shade), grouped by key. Shows whether the "navy/purple" roof quad (a) is already… |
| `probe-w13-roofcrop-g3.ts` | G3: crop atlas tiles of given roof keys from public/feattex + save as an 8×-upscaled montage. Shows visually whether the "navy" roof texture is real s… |
| `probe-w13-roofdiag.ts` | F2b: roof diagnostic 065 (correct) vs 045/016/033 (black). For each dominant roof word |
| `probe-w13-roofg2.ts` | G2: roof-record raw-data dump for 016/033/045/065. For every TYPE-0x00 4-vert record: col/row, cond, texWord, the 4 vertices (z, xOff/yOff raw + signe… |
| `probe-w13-roofpair.ts` | F2: completion rule for the overworld roofs — unproject ow016hut dump quads and classify against the AREA016 roof entries (entry / mirror / cap). |
| `probe-w13-roofrender.ts` | G2: renders ONLY the roof quads of an area top-down (XZ), filled with their atlas average color — reveals coverage gaps (black lid showing through) in… |
| `probe-w13-scan82.ts` | F2: scan all areas for [17] type-0x82 groups (VRAM rect anims) |
| `probe-w13-shipcmp.ts` | Black-ship family: (a) dump window a131 vs a187 vs clut126_s01 hw equality per block, (b) residual equality of the recons among each other, (c) CLUT r… |
| `probe-w13-subfiles.ts` | F2: ct3-Subfiles einer Area mit Stripe-Platzierung listen |
| `probe-w13-tilestat-g3.ts` | G3: Status einzelner Tiles: maptex idx/lum/rgb + Dunkel-Region-Größe + Roof/Cap-Deckung. |
| `probe-w13-torch.ts` | F3: raw dump of the torch feature records (AREA120 et al.) — inspect TYPE-0x10 spanWords + |
| `probe-w13-verify82.ts` | F2: 0x82-Copy-These verifizieren — GT-Slot-Inhalt gegen EMI-Frame-Quellen matchen |
| `probe-w13-vram.ts` | F2: check the texel source of feature words — rect resolution + VRAM occupancy at the decoded spot (EMI reconstruct), incl. a scan of WHERE in the rec… |
| `probe-w8-disasm.ts` | W8 tool: MIPS disasm directly from a DuckStation savestate (RAM) — for EXE/overlay code without an emulator session. Created during world audit W8a/W8… |
| `probe-w8-dump.ts` | W8 tool: GPU dump forensics for the floor question — (a) does the character stand on floor quads? (b) which UV cells/screen sizes do the floor quads h… |
| `probe-w8-ft4scan.ts` | W8 tool: find POLY_FT4 packets in the savestate RAM (class-A packet buffer forensics). Finds packets by CLUT (+optional UV cell window), reports addre… |
| `probe-w8-holes.ts` | W8 tool: "walkable without floor" audit across all public/areas/*.json — the core metric of |
| `probe-w8-trace.ts` | W8 tool: GDB breakpoint tracer against DuckStation — collects registers per hit and builds histograms. Grew out of the world audit W8b (class-A drilli… |
| `probe-w9-audit.ts` | W9 tool: OFFLINE anomaly audit across all 200 areas — finds browser↔original suspicion cases WITHOUT an emulator (the user: "more systematic than eyeb… |
| `probe-wallbuilder.ts` | Searches the RAM dump for code spots that address the map block @0x80104000 — candidates for the map floor/wall builder (edge↔entry mapping). |
| `probe-wallchain.ts` | Raw texture-entry CHAIN (roof + follow-up entries up to the terminator) for given tiles — all fields incl. b1 (suspected: segment/mode field of the ma… |
| `probe-wallmatch.ts` | WALL-SYSTEM DECODING: assigns each vertical frame quad of a GPU dump to a map-tile EDGE (via edge matching against the registered floor quads) and com… |
| `probe-wallpredicate.ts` | Verifies the MIPS wall model (builder 0x80153344 + tile render 0x80154508) against the map data: block size (entries between texIdx and the next used … |
| `probe-wallquads.ts` | Vertical (wall) quads of a dump: UV base/span vs screen height — derives the original step/tile rule of the map-tile walls (for terrain.ts WALL_STEPS)… |
| `probe-wallverify.ts` | Verifies the NEW engine-exact wall assignment (collectAreaWalls: height predicates + sequential entry consumption) against a GPU dump: every assigned … |
| `probe-water-frames.ts` | Probe 9: search the AREA008 EMI for ALTERNATE water frames. (a) The two ctype3 pages (0a081000, 0e001000): which VRAM regions do they load into? Does |
| `probe-water-live.ts` | Probe 5: compare EMI-reconstructed water against live VRAM-008 (vram-008.bin). Plus: check whether the used water CLUT indices form a contiguous RAMP |
| `probe-water-png.ts` | Probe 10: render the water cell (and surroundings) as a PNG for viewing + decode the pBAV table (ctype6) of AREA008 to find animation cell records. |
| `probe-water-vram.ts` | Probe 2: water cell p2(2,9) in the reconstructed VRAM — are the 16x16 pixel indices uniform (1 color = CLUT cycle is enough) or structured (wave patte… |
| `probe-water008.ts` | Probe: AREA008 water tile (p2(2,9) pal4) — are there CLUT cycle frames in the EMI? Hypotheses: (A) CLUT cycling, (B) animation frames, (C) texture scr… |
| `probe-which-plchar.ts` | PROBE: which PLCHAR is loaded in the savestate RAM? Find pBAV occurrences in RAM + match the |
| `probe-zonecallers.ts` | Probe 2: finds all jal 0x801a4e40 (tile writer) in RAM + the base pointer @0x8014931c. Disassembles the call sites, folds the arguments (col,row,val),… |
| `probe-zonemap.ts` | Probe 3: (a) collect all 38 subfile writes (col,row,val) + compare against warps. (b) dump the live map @0x8010bd30 (cols*rows bytes) from RAM → value… |
| `probe-zoneoffset.ts` | Probe 8: exact verification: map subfile (idx12, @0x80104000) offset 0x7d30 == live map? And: is 0x7d30 derivable from the map header (pointer slot)? … |
| `probe-zonewalk.ts` | Probe 10: (a) do the warp SOURCE tiles sit in non-0x10 components (= are they reachable)? (b) value semantics: correlate map value with corner-height … |
| `probe-zonewriter.ts` | Probe: examines the subfile @0x801f2c00 (warp/init overlay, ctype0) of AREA007. Question: does its init code call a tile-byte writer 0x801a4e40 (base[… |

#### Shared modules and utilities

| Module | Purpose |
|---|---|
| `accession-capture.ts` | ACCESSION EMULATOR GT: battlecap12 + confirm hijack → real in-game transformation, with DuckStation MediaCapture (F9) as a video+audio dump. |
| `analyze-box.ts` | Solves with the real GTE camera (R,T from live RAM, formula from BOF3_KNOWLEDGE §10): 1. tile→world transform from floor correspondences: V=(S·col+ox,… |
| `analyze-gpudump.ts` | Analyzes the polygon geometry of ONE frame: do the ground tiles form a regular grid? Prints, per dominant page, the polys in draw order (screen quad, … |
| `analyze-ot.ts` | Locates the OT/packet structures in live RAM: for each textured quad of the dump frame, the POLY_FT4 packet (search via the uv0+clut word, verificatio… |
| `area-switch.ts` | AUTOMATED ZONE SWITCH via savestate edit (deterministic, without cheats UI/GDB). Patches the warp recipe found via disasm (field state machine, 6 RAM … |
| `atlas-from-dump.ts` | Extracts, from a GPU dump, every (page+CLUT) combination used by textured polys as a correctly colored 256×256 atlas (the scene's real tileset). Also … |
| `audit-prims.ts` | PRIM EXPLANATION AUDIT — the systematic gap finder: every drawn primitive of a GT dump gets assigned to a KNOWN source |
| `audit-visual.ts` | AUDIT-VISUAL: generates a contact sheet for each open audit candidate (area × CLUT slot): the recon crops around the candidate quads (from the area's … |
| `bake-vertical-rt.ts` | VERTICAL QUAD BAKE for runtime classes: runtime quads that are |
| `battle-capture.ts` | MID-BATTLE RAM CAPTURE via DuckStation cheat. Mechanism: the field FSM trigger 0x80143bb0 knows, besides 5 (=warp, see warp.ts), the value |
| `bgm-find-table.ts` | Searches for the area→BGM-slot engine table in RAM via cross-validation over several natural savestates. |
| `bgm-fingerprint.ts` | Fingerprints DuckStation savestates: which area + which BGM is loaded? Supplies (area, BGM slot) anchors for the area→BGM engine table lookup (see ref… |
| `bgm-gt-capture.ts` | captures the BGM OF ONE ZONE from the running game (ground truth). WHY: OST rips only work as a reference to a limited extent — they are arranged (dif… |
| `bgm-gt-fromsave.ts` | records the BGM from a SAVESTATE (ground truth). ⚠ WHY NOT VIA warp.ts: the cheat warp moves the character, but the engine does NOT load a |
| `bgm-gt-play.ts` | GROUND TRUTH for the BGM: have the emulator play ANY given track via the game |
| `bgm.ts` | Triggers a BGM in the running game via the BGM request variable 0x80184460 (RE) |
| `bind-pages.ts` | Binds my atlas ID (index bits 12-13) to the VRAM pages of the dump — via overlap of the used 16-cell blocks (no spatial matching needed). Also outputs… |
| `bmagic-sheet.ts` | BMAGIC SHEET/CLUT DECODER — the shared source of truth for all effect extractors (build-spell-replay.ts, references/re/vfx-interpreter/render-frames.t… |
| `bootstrap.ts` | builds `public/` COMPLETELY from your own disc. WHY THIS EXISTS: `public/` (sprites, music, textures, game texts) is derived |
| `capture-ram.ts` | Captures scene + FULL 2 MB RAM synchronously: load savestate → F8 GPU dump → GDB full read (pauses the emulation — hence after the dump; the frozen RA… |
| `capture-scene.ts` | Records a cutscene synchronously: load savestate → GPU dump (F8) + live RAM (GDB) in the same |
| `capture-walk.ts` | Autonomous walk capture: loads a savestate, steers Ryu via D-pad keys with GDB position feedback (camera block @0x801492e8 +0x38/0x3a) to a RELATIVE t… |
| `cell-clut.ts` | Checks: does the same texture cell (UV 16-cell) use different CLUTs? (= real per-tile palette) |
| `cheat-test.ts` | Test harness for DuckStation cheats (live RAM pokes per frame, without a GDB freeze). Writes a .cht with given Gameshark codes, activates it, starts D… |
| `ctype7.ts` | Decoder for BoF3 sprite-texture subfiles (EMI contentType 7, "ctype7"). Codec reverse-engineered from the PSX EXE decompressor at RAM 0x8014ea4c (EU S… |
| `derive-cluts.ts` | Derives the real (page column, cell) → CLUT row binding from the GPU dumps (ground truth against the pages 4-7 problem) and writes it as a TXTY-compat… |
| `diag-cells.ts` | Diagnose: Map-Roof-Zellen um camTile vs. Dump-Boden-Zellen im iso-Gitter — nebeneinander. npx tsx extract/diag-cells.ts [dump] [area] [camCol] [camRow… |
| `dump-featuv.ts` | Verifies the feature UV table theory (texture routine 0x801557d4): word modes: bits&0xf00==0 → 16×16 cell from nibbles; bit 0x800 → 8-byte table |
| `dump-series.ts` | GPU DUMP TIME SERIES: loads a named savestate into DuckStation (FOREGROUND — the emulator pauses in the background!) and pulls N F8 GPU dumps on a tim… |
| `emi.ts` | EMI container of Breath of Fire III. Layout (empirically verified against the disc: sum of the padded subfile sizes |
| `enrich-community.ts` | enters the COMMUNITY KNOWLEDGE LAYER (web cross-check 2026-07, see references/KNOWLEDGE.md) idempotently into the gamedata JSONs. |
| `export-recon.ts` | Exports the scene reconstructed from the GPU dump as walkable 3D assets: public/recon/<tag>.json — all textured quads as world 3D (floor flat; vertica… |
| `extract-anim-phases.ts` | W10 tool: extract cell ANIMATION PHASES of an area from time-shifted GPU dumps (the [17] type-0x80 class: the game cyclically copies frame-bank texels… |
| `extract-anim82-phases.ts` | [17] TYPE-0x82 RECT ANIMATIONS → anim-phases-<NNN>.json (consumer: build-water-anim.ts). Finding (pd104): 0x82 groups with MULTIPLE entry pairs are FR… |
| `extract-clut-phases.ts` | extract the CLUT ANIMATION PHASES of an area from a time-offset GPU dump SERIES. Finding (GT AREA005, KNOWLEDGE "W10d"): the river/water animation of … |
| `extract-runtime-water.ts` | Runtime tileset extractor: pulls the tile texels an area uploads at runtime from a GT GPU dump (full VRAM snapshot) → references/re/water-seed-<NNN>.j… |
| `features.ts` | Feature block parser (non-floor objects: roofs/walls/billboards/triggers) — freshly implemented from the reference doc bof3-3d-maps/docs/BOF3_KNOWLEDG… |
| `find-models.ts` | Probe object mesh RE: search UV fingerprints of the GPU-dump object polys in the AREA EMI subfiles. |
| `find-sharedbase.ts` | Finds the disc source of the shared-base textures: takes 64-byte row chunks from the live VRAM |
| `fish-capture.ts` | RECORD the fishing minigame while it's being played. Pulls F8 (GPU dump) + F2 (savestate) on a fixed cadence and drops both into |
| `fish-series.ts` | DENSE FISHING SERIES: samples a savestate (RAM+VRAM) at a ~2 s rate WHILE playing, plus every third one additionally as a GPU dump. Goal: catch the BI… |
| `fish-trigger.ts` | TRIGGER THE FISHING MINIGAME ITSELF — without pad injection. Finding: the field FSM jumps via table @0x801c7d8c with b90 = bb0−1 (0x80143bb0). |
| `floor-grid.ts` | Reconstructs the world grid of floor tiles from the dump — independent of the ROM decode. Each floor quad is the iso projection of a 1×1 world tile; i… |
| `gdb-read.ts` | Reads PSX RAM from the running DuckStation via its GDB server (port 2345). Usage: npm run gdb:read -- <hexaddr> <len> e.g. 80104000 64 |
| `gdb.ts` | Minimal GDB-RSP client for DuckStation's GDB server (port 2345) — promise API. |
| `gpudump.ts` | Parser for DuckStation GPU dumps (magic "PSXGPUDUMPv1"). Structure: 14-byte magic, then packets. Packet header = 1×u32 (LE) |
| `grid-tiles.ts` | GRID-TILE EXPORT: the savestate's window grid (0x8012c000, [tc][tr][slot], 56x28) IS the authoritative list of map cells drawn at the GT moment |
| `i18n-wrap.ts` | Tool: wrap German UI texts in `L(…)` and report the key inventory. `--check` (default) lists all L keys in the code and reports missing translations. |
| `inspect-gpudump.ts` | Inspects a DuckStation GPU dump: packet/primitive statistics + VRAM as a PNG. Call: npm run inspect:gpu -- <path/to/file.gpudump> |
| `inspect-subfiles.ts` | Diagnose: unbekannte AREA-EMI-Subfiles + Feature-Block des Map-Subfiles sichten. npx tsx extract/inspect-subfiles.ts [area=000] |
| `map.ts` | Map subfile of BoF3 (type 0x80104000 in the EMI). Layout (verified against the disc; sizes match exactly) |
| `match-features.ts` | Full verification of feature geometry against the GPU dump. 1. Deproject dump floor quads (including merged N×M blocks) into the iso grid. |
| `mesh-groups.ts` | Mesh-group table parser (3D models: furniture etc.) — freshly implemented following bof3-3d-maps/docs/BOF3_KNOWLEDGE.md §6, subfile RAM 0x800d3800. |
| `mips.ts` | Compact MIPS-I disassembler (R3000A + COP2/GTE transfers) for RAM code analysis. |
| `navigate.ts` | AUTONOMOUS A* NAVIGATOR through the emulated game. Loads a DuckStation savestate, reads the player position via GDB (lead cache 0x80149308/0c |
| `parse-savestate.ts` | Parses a DuckStation savestate (format "DUCCS", SaveStateCompression=Zstd) and extracts the |
| `quads-from-savestate.ts` | Reconstructs the textured frame quads (POLY_FT4) of a frame DIRECTLY from the PSX RAM of a DuckStation savestate — WITHOUT a GPU dump (no F8, no GUI, … |
| `read-savedata.ts` | parses a BoF3 PSX memory card (.mcd) and decodes the save format (RE roadmap phase C, solved). Output: public/gamedata/savedata-format.json |
| `register-dump.ts` | Which AREA does a GPU dump show? Registers the dump's floor surface (texture cells in the iso |
| `render-gpudump.ts` | Reconstructs the drawn frame from a GPU dump: rasterizes every textured polygon (affine UV mapping, 4/8/15bpp + CLUT from VRAM) in draw order (painter… |
| `render-savestate.ts` | Reconstructs the drawn scene OFFLINE from a DuckStation savestate (RAM frame quads + VRAM) — without GPU dump/F8. Like render-gpudump.ts, but quads fr… |
| `rom-tiles.ts` | Correct ROM tile renderer for BoF3 areas — ported from the verified Python version (bof3-3d-maps/test_mips_render.py `render_area_mips(path='default')… |
| `scena-api.ts` | SCENA ENGINE API CATALOG (RE roadmap L1.1). The SCENA##.EMI are compiled MIPS cutscene scripts (load to RAM 0x801f6c00): [u32 0x0100\|ID][u32 flags][N… |
| `scena-dialog.ts` | SCENA DIALOG MAPPING (RE roadmap L1.2; SCENA→area map). Builds on the |
| `scena-npc-scripts.ts` | NPC MOVEMENT/CUTSCENE BYTE VM (RE roadmap L1.7). Fully decoded from the MIPS code (ram1.ram.bin disassembly, extract/mips.ts) |
| `scena.ts` | SCENA spawn extractor: static MIPS analysis. Finds jal calls to the spawn helpers and resolves |
| `scene-compile.ts` | FAITHFUL SCENE-QUAD-COMPILER (Proof-of-Concept) — translates an area into the EXACT quad list that |
| `solve-vroof.ts` | Solves the 0x50/0x51 roof geometry conclusively: fits the floor DLT (ram1/AREA007, proven pipeline shortcut) and tests all plausible corner/axis inter… |
| `sprites-from-dump.ts` | Extracts whole, assembled 2D sprites (characters/enemies/dragons) from a battle GPU dump. Method: take textured quads of the main frame, exclude the 3… |
| `state-heights.ts` | STATE HEIGHTS EXPORT: runtime height-patch areas (060 bridge, 074 hill) deform the map in RAM — the judge browser would otherwise render the disc heig… |
| `state-movers.ts` | MOVER GT EXPORT: live object slots from the pd savestate for the judge — mover poses (094 ship gone, 172 lift-machine position, 086 crane, 045 windmil… |
| `sweep-compare.ts` | COMPARISON GALLERY — juxtaposes, per grid position, the game GPU recon (rz512 = ground truth from |
| `sweep-grid.ts` | SYSTEMATIC GROUND-TRUTH SWEEP — rasters an area from one corner and dumps every grid position |
| `sweep-mcneil.ts` | GROUND TRUTH SWEEP McNeil: warps the character deterministically to a LIST of positions and dumps |
| `sync-data.ts` | mirrors the HAND-MAINTAINED data from data/ into public/. WHY THIS EXISTS: `public/` is generated entirely from the disc and is therefore not in the r… |
| `warp.ts` | AUTONOMOUS ZONE SWITCH via DuckStation cheat (deterministic, without GDB/HID navigation). Mechanism (from ram1 disasm, verified): the field FSM tick 0… |
| `weretiger-capture.ts` | WERETIGER EMULATOR GT: battlecap12 + confirm hijack → real in-game transformation (Rei → Weretiger |

