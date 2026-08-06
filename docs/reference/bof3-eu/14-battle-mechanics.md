> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 14. Battle mechanics

Battle logic lives in `BATTLE.EMI sub[3]@0x801d0c00` (118 KB), verified byte-identical against a
captured mid-battle RAM image (`references/gpudump/battlecap12.sav`, a Cedar-area encounter,
reached via `SLES-01304_resume.sav` and sampled live over GDB with `extract/battle-capture.ts` /
`scratchpad/formel-live.ts`). All addresses below are RAM addresses inside that overlay unless
stated otherwise.

### Call chain and phase handlers

**Battle entry.** The field FSM tick handler `0x80197178` (in `GAME.EMI`) zeroes
`0x80143b90`/`0x80143b92`, reads a request byte at `0x80143bb0`, and dispatches through table
`@0x801c7d8c` indexed by `b90 = bb0−1`. `table[4]` = warp (`bb0=5`), `table[8]` = battle
(`bb0=9`) — a phase loader that sub-dispatches through a second table `@0x801c7dd8` via
`0x80143b92`. Other `bb0` values: `1` = menu mode, `2` = cutscene/demo scene.

`bb0=9` alone does not start a battle. The random-encounter path `@0x801679b8` first checks the
encounter-enable flag (`0x80144956==0`) and the area encounter mask
(`0x8014625c & 0x80145ab6 ≠ 0`; the mask is 0 while standing in a village), then calls formation
selection `0x801c2754` and setup `0x801c4ecc`. The resulting battle type is stored at
`battleCtx[0x11c]` (`battleCtx = *(0x80146250)`, so the field resolves to `0x801460ec`):
`2`=random, `3`=script, `4`=unidentified.

**Command dispatch (menu → commit).** Command menu header `@0x801d0c00` (18 pointers, 3 category
stubs) → commit `0x801d5cb4` (usability check `0x8016697c`, action queues
`0x801d8bc0`/`0x801d8bd4`) → a 16-entry action-type table `@0x801ec400` (dispatcher `0x801d49f8`;
handlers from `0x801d4a44` = attack/skill/magic/item/defend/run executors).

**Action execution.** Once an action resolves, a second, distinct 16-entry table drives the
effect: the action dispatcher `0x801e3bd0` reads `fnTab[ctx[+3]]`, and the table itself is static
in the BATTLE overlay at `0x801eb424`. `type0 = 0x801e3c14` = physical attack → builds the damage
context; the other 15 types are not individually identified.

**Turn/phase state.** Current acting slot: `0x80144953`. Turn-phase byte: `0x80148651`
(19 distinct phase values observed).

**Actor structs.** Party battle actor: base `0x80145e90`, stride `0x140` (320 B), slots 0-2.

| Offset | Field |
|---|---|
| `+0x58` | remaining action time |
| `+0x74` | name |
| `+0x88` | HP (absolute `0x80145f18` for slot 0) |
| `+0x92..+0x9a` | stat row (`pwr@+0x94`; e.g. Teepo Lv1 = 20) |
| `ctx[0x4b]` | anim/program index (scratch-cache writer `0x801d6b04`) |

⚠ A separate pass anchors "party battle actor" at `0x80145f04+idx·320` (first `0xa4` bytes = copy
of the field party record `@0x80144968`, `+0xa4..` = sprite runtime) — 0x74 above the base listed
here, matching the `name` offset. The source material does not reconcile the two anchors; both are
given as found.

`ctx[0x4b]` program values: `6` = command idle (both Ryu and Teepo observed, `ctx[+8]=2`=party
side); `0x4e` = inactive slot; `0x12→0x13` = two-stage death sequence, set by the death helper.

Enemy battle block: base `0x801eb630`, stride `0x118` (280 B), slots 3+ (scratch writer
`0x801d16f0`). ⚠ The EXE-disassembly base `0x801fb2e8` is the FIELD spawn struct, not the battle
struct.

⚠ **E-block layout puzzle (open).** Spawn mapping writes record `+0x10`/`+0x12` → actor
`+0x84`/`+0x86`, HP → actor `+0x8a`; but live mid-battle dumps show `+0x94=25`, colliding with the
HP-apply target (see below). The enemy struct likely has phase-dependent occupancy or two
overlapping layouts. Reward and formula paths are independently measured and unaffected.

### Damage arithmetic

```
finalDamage = calcDamage(type, slot, raw)              // 0x801dc044
finalDamage *= (1 + m/2)   if flag 0x40 set             // buff multiplier; m zeroed after use
apply(a0, slot, finalDamage)                             // 0x801dbb78
writeHP(...)                                              // 0x801dbd6c
```

`raw=0xffff` passed into `calcDamage` means "value already computed by choreo precalculation", not
computed in this call.

`apply` (`0x801dbb78`, args `a0, slot, dmg`): `slot<3` → party HUD block
`0x80145fb0+slot·320` (HP observed at `0x80145f18` for slot 0); `slot≥3` → enemy actor. Damage
context pointers (attacker/target): `0x8014639c`/`0x801463a0`. Display accumulator:
`0x801463d8`/`0x801463da`.

HP changes are pointer-based clamp-add, not a literal `sh damage,HP` (so naive write-pattern scans
miss it): `healHP` `0x80165534` (dual party/battle-actor path via a flag), `clampAdd`
`0x80165824`/`0x801657a8` (floors at 0; negative Δ subtracts).

Effective-stat formula (feeds both attack and defense; shared buff/debuff engine, also used by
the equipment system): `@0x80164cf8`, offsets relative to the field party record `@0x80144968`
(copied into the actor's first `0xa4` bytes):

```
effVal = base − round(base · mod / 10)      // magic constant 0x66666667 = ÷10
statsBase      @+0x3c
statsEffective @+0x1c
// 4 modifier passes applied in sequence
```

Level-up growth writer `0x801ae0a0` writes new maxHP to `+0x3c`/`+0x1c`/`+0x14` of that same
record.

The exact physical attack-vs-defense arithmetic itself is not disassembled. An empirical fit from
42 samples gives a uniform spread `[base/2 .. base]`; 6 live samples with full stat hexdumps
(damage 2-7, action types 0/1/4, attacker/target pointers `0x8014639c`/`0x8014a0`) are recorded in
`re-work/damage-trace/live-samples.json` but had too little variance to fit a closed form. A
power-driven, ÷10 damage-scale routine exists at `0x800a1270`, but it is the AI's damage
**preview** calculation, not the roll actually executed on hit.

A skill heal amount of `(int+100)/5` was found at `0x801e7934` — ⚠ this is a FIELD-overlay formula
on the field party record, misattributed to battle in an earlier pass against
`battleload.ram.bin`; it is not part of the battle heal path.

### Hit and crit

RNG source: BIOS `rand()`, invoked via kernel call `A0:0x2F` (battle-code stub `@0x8017e8a0`,
`jr 0xa0` with `t1=0x2f`; `srand` stub adjacent at `0x8017e8b0`). The generator itself (LCG,
`seed·0x41C64E6D+12345`, 15-bit return) lives inside the BIOS, not in RAM.

```
percent_roll = rand() % 100     // via magic constant 0x51EB851F, 22 call sites in battle code
                                  // (11 of the 22 call the rand stub directly)
```

Miss rate measured empirically at ≈7% (2 misses / 28 attacks, pooled across both sides). Agl
dependence of the miss chance could not be isolated with only one agl constellation sampled
(open; proposed method: an agl sweep analogous to the power sweep used for the damage fit).

No crit-hit formula (chance or damage multiplier) is present in the captured material. The
`specialProps` bit `0x08` (`luckyStrike`, see AI selection) marks an enemy as capable of a
distinguished attack, but its mechanical effect is not detailed anywhere in the source.

### Turn order

```
order = globalSort(agl)   // descending, both sides pooled, + enemy level jitter
```

Ground truth: sorted order at `0x8014630c` in `battlecap12` = `[3,1,0,4]`, with enemy agl‑27
acting first.

⚠ Superseded: an earlier reading (20+ rounds of anim-log observation) modeled this as two fixed
SIDE PHASES — party phase first (internal agl descending), then enemy phase (stable internal
order) — because the enemy agl values being compared (14-19) were read from a byte offset that a
later, more careful enemy-record pass (below) found to be shifted; the Cedar enemies' true agl is
2-5, so the party genuinely was faster and a global sort reproduces the same observed order.

### AI selection

`specialProps`, enemy record `+0x0e` (u8), governs both targeting behavior and which commands an
enemy may use:

| Bit | Meaning |
|---|---|
| `0x01` | reverse targeting |
| `0x02` | reprisal |
| `0x04` | dodge |
| `0x08` | lucky strike |
| `0x10` | target lowest HP |
| `0x20` | defend allowed |
| `0x40` | skills allowed |
| `0x80` | escape allowed |

(Bits `0x20`/`0x40`/`0x80` clear = that action is disabled for the enemy.)

Skill roulette, enemy record `+0x18` (8 B) = 8 eighth-slots of skill IDs — the "section-1" AI
selection pool. Example: Gary = `01 02 22 43 54 54 54 54` → NueStomp/Gambit/Steroids/Chill/Speed,
Speed weighted 4⁄8.

AI action list, enemy record `+0x34`, 4 slots × 16 B:

```
[0x63][kind][id][p][weight:u32][pattern:8B]
```

`kind`: `0x01` = skill (skill id in the `id` byte), `0x05`/`0x06` = physical. The trailing 8-byte
`pattern` is a further skill roulette in the same 8-eighth-slot format as `+0x18`; a header byte
at slot offset `+4` encodes an attack-type code (exact bit layout not further resolved). Raw
records are preserved per enemy in `enemies.json` under `aiTail`.

The runtime routine that evaluates these weights/patterns to pick an action each turn is not
disassembled — only the data layout is known (open).

### Status effects

`initStatus`, enemy record `+0x81` (u8), start-of-battle ailment bitfield (435/435 DB-verified):

| Bit | Status |
|---|---|
| `0x01` | regen |
| `0x04` | paralyze |
| `0x20` | confuse |
| `0x40` | sleep |
| `0x80` | poison |

`RESIST`, enemy record `+0x78`, 9 bytes, one per element, values 0-7, default 2 (holy default 5),
435/435 DB-verified:

`fire, ice, thunder, earth, wind, holy, psionic, status, death`

### Reward chain (EXP / zenny)

Enemy record: `+0x10` = zenny (u16), `+0x12` = EXP (u16).

Spawn copy `0x800ac32c`/`0x800ac348` copies these into actor `+0x84`/`+0x86`. ⚠ The raw
disassembly immediates `0xb6b4`/`0xb6b6` are not small offsets — they are the low halves of the
slot-3-absolute addresses `0x801fb630+0x84`/`+0x86`.

Kill routine `0x801e54ec` sets actor flag `|0x40`, zeroes actor `+0x84` (prevents double reward),
and accumulates into `0x8014632c` (EXP) / `0x80146330` (zenny). `BATL_END.EMI` (`@0x801eec00`)
pays out via `0x801ef818` → `addZenny`.

- `addZenny = 0x80166920` (shared capped-add, also used by the EXP/level-up path),
  `spendZenny = 0x801668f0`.
- Zenny variable `@0x80144f50` (u32, cap 9,999,999); lifetime accumulator `@0x80145030`.
- Bonus modifier `@0x8009cb10`:

```
bonus = reward × N × 100 / 1000     // ÷1000 via magic constant 0x10624dd3; nets reward × N / 10
```

Exact reward anchors (DB-verified):

| Enemy | EXP | Zenny |
|---|---|---|
| Ripper | 7 | 5 |
| Gonghead | 8 | 10 |
| GntRoach | 85 | 0 |

⚠ Values are PER-AREA-INSTANCE, not per species (e.g. Eye Goo in area003 = 2 EXP; other instances
carry different values — not a contradiction). `enemies.json` carries `exp`/`zenny` per instance.

### Data tables

**Enemy record**, 136 bytes total. Runtime base `0x800e4048+idx·136`; disc source
`AREA-EMI sub@0x800e4000`. Decode correlated 430/430 against the community enemy database.

| Offset | Size | Field | Notes |
|---|---|---|---|
| `+0x08` | u16 | enemyId | globally unique per variant |
| `+0x0a` | u8 | attackType | `0`=attack `1`=defend `2`=escape `3`=skills, `6`/`13`/`18`… = mixed forms |
| `+0x0e` | u8 | specialProps | bitfield, see AI selection |
| `+0x10` | u16 | zenny | reward |
| `+0x12` | u16 | EXP | reward |
| `+0x14` | u16 | level | |
| `+0x16` | u16 | gfxKey | `0xffff` = boss/graphic hosted in another area; key reader `0x800abce0`; ⚠ earlier read as "HP" (17/430 DB hits only) |
| `+0x18` | 8B | skill roulette | AI selection pool; ⚠ earlier read as `unk18`/`elemResist` |
| `+0x20` | u16 | HP | |
| `+0x22` | u16 | AP | |
| `+0x24` | u16 | Str | |
| `+0x26` | u16 | Def | |
| `+0x28` | u16 | Agl | |
| `+0x2a` | u16 | Int | |
| `+0x2c` | 4B | steal | `[itemId:u8][cat:u8][rateCode:u16]` |
| `+0x30` | 4B | drop | same layout as steal |
| `+0x34` | 4×16B | AI action list | see AI selection |
| `+0x74` | 4B | unk74 | default `00 00 ec ec`, bosses `28 28 f6 f6` — open |
| `+0x78` | 9B | RESIST | see Status effects |
| `+0x81` | u8 | initStatus | see Status effects |
| `+0x82..+0x87` | — | open | byte distributions documented in scratch `w1b`, not yet named |

`cat` (steal/drop): `0`=item `1`=weapon `2`=armor `3`=accessory.

`rateCode` → chance:

| Code | Chance |
|---|---|
| 1 | 1/256 |
| 2 | 1/128 |
| 3 | 1/64 |
| 4 | 1/32 |
| 5 | 1/8 |
| 6 | 1/2 |
| 7 | 1/1 |

DB verification counts:

| Fields | Match |
|---|---|
| attackType partition | 430/430 |
| HP/AP/Str/Def/Int | 428-430/437 |
| Agl | 413/437 (systematic community-DB off-by-one) |
| RESIST | 435/435 |
| initStatus | 435/435 |
| steal rateCode | 400/400 |
| drop rateCode | 343/343 |


### Battle operation catalogue

`references/re/battle-op-catalog.txt` holds two tables.

1. All 130 choreography op handlers from the table at `0x800b471c`, automatically classified by their `jal` target labels — `calcDamage`, `magicFormel`, `chanceRoll`, `stageApply`, `recompute`, `clearStatus`, and others. The classification tool's recipe is documented in the file's own header.
2. The complete phase-4 execute table at `0x800b44f8`: maps `skillId` to a start op, with names drawn from `skills.json`. 187 entries are occupied.

Together these form a lookup map from program-array index to game action. Every action resolves to its formula entry point. Examples:

| Skill | Start op |
|---|---|
| WardOfLight | 98 (direct stat stage) |
| Berserk | 121 |
| heal family | 7 |
| breath family | 64 |
| Super Combo | 103 |
| MeteorStrike | 124 |

Still open: the semantics of unlabeled choreography ops (an empty label means an animation/parameter op with no formula call), and multi-hit animation cosmetics.
### Refuted approaches

- **Turn order as fixed side-phases** (party-phase-then-enemy-phase, no global sort) — refuted; the
  real mechanism is a global agl sort with enemy level jitter, GT-proven via `battlecap12`. The
  side-phase illusion came from reading enemy agl at a shifted record offset.
- **`battleload.ram.bin` as the battle code source** — refuted; the real battle code is
  `BATTLE.EMI sub@0x801d0c00`, byte-identical in `battlecap12`. Addresses disassembled against
  `battleload.ram.bin` can describe foreign overlay code (the field-overlay heal formula
  `(int+100)/5@0x801e7934` was misattributed to battle this way).
- **Action-type handler table at `0x801fb424`** — a sign error; the correct static address is
  `0x801eb424` (candidates `0x801eb120`/`0x801eb1b4` considered in the same pass were superseded).
- **Enemy record field mislabels** (pre-W1-B correlation pass): `gfxKey@+0x16` was read as "HP"
  (only 17/430 DB hits); the skill roulette `@+0x18` was read as `unk18`/`elemResist`.
- **`0x80144f50` as "EXP total"** — wrong; it is the ZENNY variable, confirmed by shop code
  (`money -= price @0x801d3ee0`).
- **Damage-scale routine `@0x800a1270`** is the AI's damage preview, not the executed damage roll
  — do not conflate the two when reimplementing.
- **RAM LCG scan for the RNG state** — found nothing because the generator runs inside the BIOS
  (`rand()`/`srand()` via kernel call `A0:0x2F`), not as a RAM-resident LCG.
- **Enemy struct base `0x801fb2e8`** (from the EXE disassembly) — this is the FIELD spawn struct;
  the battle struct is `0x801eb630`.
- **EXP/zenny via a separate per-id table** — an earlier lead (`jalr 0x801e916c` reading a filler
  from `0x8014632c` before the count-up at `0x801eecd0`) suspected a table outside the 136-byte
  enemy record; superseded — they are enemy-record fields `+0x10`/`+0x12` directly, confirmed by
  the exact reward anchors above.

### Open

- Exact physical attack-vs-defense damage formula: only the call chain and an empirical
  `[base/2..base]` approximation exist; which of the 22 percent-roll sites (candidates in the
  `0x800a2xxx` choreo region) performs the damage roll is unidentified.
- Hit/miss agl-dependence: only a pooled ≈7% miss rate is known, no agl sweep performed.
- Crit-hit mechanic (chance, multiplier, trigger): absent from the captured material entirely;
  possibly tied to `specialProps` bit `0x08` (lucky strike).
- AI per-turn selection algorithm: the action-record/skill-roulette data layout is known, the
  runtime weighting/selection routine is not disassembled.
- E-block layout puzzle: `+0x94` collides between spawn-mapped fields and the live HP-apply
  target; struct may have phase-dependent occupancy or two overlapping layouts.
- `unk74` (enemy record `+0x74`, default `00 00 ec ec`, bosses `28 28 f6 f6`): purpose unknown.
- Enemy record `+0x82..+0x87`: byte distributions logged but fields not identified.
- Battle type `4` at `battleCtx[0x11c]`: meaning not identified (`2`=random and `3`=script are
  known).
- 16 action-execution types at `0x801eb424`: only `type0` (physical attack, `0x801e3c14`) is
  identified by name.

