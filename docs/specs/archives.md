# Archive families

Per-archive entry manifests are generated during EMI extraction under
`out/extracted/BIN/**/emi.json`; they are consumed in memory by
`load_catalog` and are disposable generated evidence, not reviewed truth.

| Family | Stable role |
| --- | --- |
| `BATTLE` | battle code, graphics, and audio |
| `BENEMY` | enemy audio banks |
| `BGM` | VAB and sequence music bundles |
| `BMAGIC` | battle-effect code, graphics, and audio |
| `BOSS` | encounter code and resources |
| `BPLCHAR`, `PLCHAR` | player-character resources |
| `ETC` | shared frontend and system archives |
| `SCENARIO` | scenario controllers and resources |
| `WORLD00`–`WORLD04` | world and area code-data archives |

Notable `ETC` archives:

| Archive | Stable role |
| --- | --- |
| `SHOP.EMI` | shop program and data |
| `BATE.EMI` | battle-event content |
| `DEMO.EMI` | demo and cutscene content |
| `ENDKANJI.EMI` | end-credit text |
| `SHISU.EMI` | save-system content |
| `MTEST.EMI`, `RTEST.EMI` | development and test content |

Families describe shipped organization, not executable identity. Classify and
promote individual EMI entries.

# Data ownership and duplication

This page identifies owning archives. Byte offsets and record layouts remain in
the linked [data specs](data/index.md).

## Static data

| Domain | Owning archive | Duplicate or companion |
| --- | --- | --- |
| equipment, abilities, shops, levels | `BIN/ETC/GAME.EMI` | none confirmed |
| base character stats | `BIN/ETC/START.EMI` | byte-identical copy in `STATUS.EMI` |
| master skills and stat modifiers | `BIN/ETC/SISYOU.EMI` | none confirmed |
| master names | `BIN/ETC/AFLDKWA.EMI` | copy in `FIRST.EMI` |
| fairy gifts and exploration items | `BIN/ETC/COMMU00.EMI` | prizes in `COMMU02.EMI` |
| dragon growth data | `BIN/ETC/STATUS.EMI` | none confirmed |
| monsters and formations | each `WORLD*/AREA*.EMI` | area-local records |
| chests, genes, chrysms | referenced area/scenario archive | pointer-map locations |
| Manillo trade data | selected area archives | repeated trade tables |

## Programs

| Domain | Archive | Notes |
| --- | --- | --- |
| main battle program | `BIN/BATTLE/BATTLE.EMI` | shared battle implementation |
| battle copy | `BIN/BATTLE/BATTLE2.EMI` | duplicate payload group |
| boss battle programs | `BIN/BOSS/BOSS*.EMI` | battle implementation plus boss-local data |
| game frontend | `BIN/ETC/GAME.EMI` | confirmed code entries `0` and `1` |
| status menu | `BIN/ETC/STATUS.EMI` | confirmed code entry `0` |
| scenario controller | `BIN/SCENARIO/SCENA16.EMI` | confirmed code entry `0` |

Target identity remains archive path, entry slot, payload hash, and load
address. Duplicate bytes do not merge source ownership.

## Enemy audio

`BIN/BENEMY/ENEMY*.EMI` contains enemy audio banks rather than executable
overlays. The reference mapping uses monster ID `N` to select
`ENEMY{N-1}.EMI`; confirm the caller and bounds before promoting that relation
into code.

## Boot and media

| File | Role |
| --- | --- |
| `SYSTEM.CNF` | boot configuration |
| `SLUS_004.22` | main executable and loader |
| `LOGO/LOGO.EXE` | secondary logo executable |
| `LOGO/CAPCOM30.STR` | logo video and XA audio |
| `BIN/BMAG_XA/MAGIC00.STR` | battle-magic XA bank |
| `BIN/SCE_XA/S_XA00.STR` | scenario XA bank |
| `BIN/SCE_XA/VOICE.STR` | voice XA bank |

Per-archive entry records are generated into the disposable per-archive
manifests (`out/extracted/BIN/**/emi.json`) during extraction. Corpus counts
and duplicate groups are derived at runtime in the in-memory aggregate catalog
(`load_catalog`) and are not written back into those manifests; this page does
not duplicate either form of generated evidence.
