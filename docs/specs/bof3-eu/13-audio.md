> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 13. Audio: VAB, SEQ, SFX banks and XA streams

### Container formats

All music and effect audio uses the canonical Sony PSX SDK format (libsnd/libspu) — no
BoF3-specific encoding sits in the data path. Every `BGM*.EMI` (81 pieces, 19.4 MB) holds three
subfiles:

- **`ctype 6` `pBAV`** — VAB header (VABp): 32-byte header, 128×16-byte `ProgAtr` array,
  `numProg`×16×32-byte `ToneAtr` array (elsewhere called `VagAtr` — the same per-tone record),
  256×`u16` VAG size table. Verified against `BGM000`: VH = 7712 bytes = `0x20` ProgAtr +
  `0x820` tones (10×16) + `0x1C20` VAG table. The VAG table is located by the identity
  `Σ(size·8) == VB length` (`BGM000`: Σ = 233504 B == VB length). `ToneAtr` fields: `vol@+2`,
  `pan@+3`, `centerNote@+4`, `fine@+5`, `min@+6`, `max@+7`, `vagIdx@+0x16` (1-based).
- **`ctype 10` `pQES`** — SEQ (SEQp) sequence data, up to 4 tracks. Track 0 header (19 bytes):
  `'pQES'` · version · `PPQN@8` (`u16` BE) · `tempo@10` (`u24` BE) · `rhythm@13` ·
  `eventLen@15` (`u32` BE); events start at byte 19. Tracks 1-3 each carry their own 13-byte
  header: `id:2` · `PPQN:2` (BE) · `tempo:3` (BE) · `rhythm:2` · `eventLen:4` (BE); events start
  at `+13`. `eventLen` is the byte length of the event stream including the terminating
  `FF 2F 00`; chaining by `eventLen` consumes every file exactly (verified 81/81 files). Each
  track keeps its own tempo and PPQN.
- **`ctype 7`** — VB sample body: concatenated raw PSX 4-bit ADPCM samples, uncompressed.
  ⚠ Same ctype value as the unrelated, LZSS-blocked battle sprite codec — different content,
  do not conflate them. Decode with the standard nocash psx-spx algorithm: 5-filter set F0/F1
  per block, block flag bit 2 = loop start, bit 0 = loop end, bit 1 = loop repeat. Sample counts
  match the block arithmetic exactly, including the end flag.

SEQ event stream, `[Δ VLQ][event]`:

| Event byte | Meaning |
|---|---|
| `0x9n` | note-on (velocity 0 = note-off) |
| `0x8n` | note-off |
| `0xCn` | program change |
| `0xBn` | control change (CC7 volume, CC10 pan — see Pitch and envelope) |
| `0xEn` | pitch bend (see Pitch and envelope) |
| `0xAn` | aftertouch, ignored |
| `0xFF 2F` | end of track |
| `0xFF 51` | tempo change |

Spell-effect audio and general SFX reuse the identical VAB/VB pair inside their own EMI
containers (`ctype 6`/`ctype 7`); only the ownership differs (BMAGIC effect files, battle/system
SFX files). The container format needs no further reverse engineering — only the mapping from
SEQ program numbers and cue bytes to VAB instrument, pitch, and envelope does (below).

### Instrument/tone resolution

A VAB exposes 128 `ProgAtr` slots but usually fewer actual tone blocks; the tone blocks are
stored **packed**, in slot order (`VH length == 0x20 + ps·512 + 512`, holds for 82/82 BGM VABs,
where `ps` = number of occupied slots).

**Final rule: the SEQ program-change number addresses the `ProgAtr` SLOT, not the packed
tone-block index.** To find a program's tone data, scan the occupied-slot list for that slot
number — do not index the packed tone array directly with the program number.

Superseded reading: the first renderer indexed `programs[progNr]` straight into the packed
array, assuming program numbers were already packed contiguously. This fails whenever a VAB has
slot gaps. Proof (statistical, `extract/probe-bgm-audit.ts`): 27 of 82 VABs have slot gaps; for
25 of them the SEQ's used program numbers are *exactly* the set of occupied slots, and
simultaneously fall outside the packed range — impossible under the packed model by chance.
Concrete cases: `BGM053` has exactly one occupied slot (10), and its SEQ addresses exactly
program 10; `BGM060` has slots 8 and 12, and its SEQ uses exactly 8 and 12; `BGM032` has slots
2-7, and its SEQ uses exactly 2-7. `extract/build-fieldsfx.ts`, whose field SFX already sounded
correct, had always resolved cues by slot (`tones.set(pi, …)`, `pi` = ProgAtr slot) — independent
confirmation that the slot model, not the packed-index model, is right.

Impact: 9299 of 135487 notes (6.9%) played the wrong instrument, and — because every instrument
carries its own `centerNote` — also the wrong pitch.

| BGM | notes on wrong instrument | title / location |
|---|---|---|
| BGM032, BGM060, BGM076, BGM153 | 100% | bridge / camp jingles |
| BGM143 | 89.5% | "For the Dragons" (main theme), Dragnier |
| BGM003 | 80.9% | Falling Green, Cedar Woods |
| BGM011 | 78.0% | — |
| BGM104 | 63.1% | — |
| BGM039, BGM075, BGM054 | 34-56% | Wyndia region, Steel Beach |
| 55 gapless VABs (incl. BGM000, BGMBAT00) | 0% | — McNeil always sounded right |

Cross-checked by chroma cross-correlation (36 chroma bins, 25-second window, best time
offset/semitone shift) against emulator captures and OST rips:

| Piece | Reference | before fix | after fix |
|---|---|---|---|
| BGM143 (89.5% wrong) | OST "For the Dragons" | 0.501 @ +0.33 semitone | 0.903 @ 0.00 semitone |
| BGM008 (20.7% wrong) | OST "Falling Green" | 0.439 | 0.577 |
| BGM003 (80.9% wrong) | OST "Falling Green" | 0.405 | 0.577 |
| BGM000 (control, 0%) | emulator capture | 0.756 | 0.783 |
| BGM000 (control) | OST "Country Living" | 0.776 | 0.801 |
| BGMBAT00 (control, 0%) | emulator capture | 0.673 | 0.692 |
| BGM037 (control) | OST "Wyndia" | 0.744 | 0.753 |
| BGM078 (control) | OST "The Champion" | 0.824 | 0.828 |

None of the six controls regresses. Full rebuild after the fix: 81/81 files render, 0 flagged
suspect, 173 MP3s total, no silent output and none under 2 seconds; every `area-bgm.json` target
resolves.

Two further tone-selection rules:

- With overlapping key-split tones (more than one tone claiming the same note range), pick the
  tone with `vol > 0` — otherwise the note renders silent (observed on `BGM117`).
- If a SEQ addresses a program slot the VAB has no tones for at all (`tones == 0`, the whole
  16-byte `ProgAtr` row zero), real hardware plays nothing. The renderer now leaves those notes
  silent and logs "N notes on unoccupied program slots". Superseded reading: earlier code
  substituted `prog % numProg` as a best guess for any out-of-range program. Affected SEQs:
  `BGM019` (programs 12/13, 193 notes), `BGM092` (program 14, 178 notes), `BGM043` (programs
  9/10/11, 174 notes), `BGM053` (program 0, 13 notes).

The silent-slot fix exposed a second bug in `BGM019`: its track 0 addresses *only* the
unplayable programs 12/13, so the whole block is mute. Block selection previously picked "track
0, else the first track with ≥50 notes" by raw note count; it now counts only *playable* notes
(notes on occupied, tone-bearing slots) — see Area-to-track mapping. For `BGM019` this selects
track 1 (435 notes), matching the separate observation that its main melody sits in track 1.
Residual loss in the other three affected SEQs, accepted as hardware-faithful but not
GT-verified: `BGM043` track 0 loses 174 of 1184 notes, `BGM092` track 2 loses 178 of 1553,
`BGM053` track 0 loses 13 of 28.

Remaining gap: 0.66% of all notes (882 of 134619), even on the correct program slot, fall
outside every tone's `min`/`max` range; those still fall back to `prog.tones[0]`
(`BGM092` is the worst case, 393 of 3014 notes).

Audit tool: `extract/probe-bgm-audit.ts` (`--json` flag) for VAB/SEQ structure; the chroma
matcher used for cross-checking lives in the scratchpad, not committed. Full rebuild:
`npm run bgm:build:all`, or directly `npx tsx extract/build-bgm-all.ts`.

### Pitch and envelope

Each voice is pitched relative to a 44100 Hz base rate: ratio `2^((note − center)/12)`, where
`center` is the tone's `centerNote` (`ToneAtr +4`). Refinement: the exponent also includes the
fine offset, `note − center + fine/128` (`fine` = `ToneAtr +5`) — confirmed by spectral
comparison against ground truth. Field/general SFX play at their own per-VAG base rate rather
than a shared fixed rate, with the same fine detune applied.

ADSR envelope, read directly from the SPU register fields (nocash psx-spx layout):

```
adsr1: b0-3 SustainLevel · b4-7 DecayRate · b8-14 AttackRate · b15 AttackMode(0=lin,1=exp)
adsr2: b0-4 ReleaseRate · b5 ReleaseMode · b6-12 SustainRate · b13 SustainDir · b14 SustainMode
Envelope tick 44100 Hz · step = (7−(rate&3)) << max(0, 11−(rate>>2)) · cycles = 1 << max(0,(rate>>2)−11)
Increase exp: above level > 0x6000 four times slower · Decrease exp: step × level/0x8000
```

`phaseTicks()` steps through the phases; `adsrOf()` converts them to attack/decay/release sample
counts plus sustain level, with a 4-second cap guarding `Ar`/`Rr` = 0 extremes. Decay and release
are 4-bit and 5-bit fields respectively, scaled to the full rate range with `<<2`.

Superseded reading: the original renderer used fixed constants — attack 4 ms, decay 90 ms,
release 60 ms — reading only the sustain level (`adsr1 & 0x0f`) from the VAB; `adsr2` was never
read. Measured across 6 BGM VABs (202 tones): 100% have a real, non-trivial decay rate, 100% are
flagged exponential attack mode (the renderer had computed linear), 12% have a slow attack. The
audible effect concentrates in release: measured real release times of 106-425 ms
(`BGM000` sample) versus the hardcoded 60 ms, so notes were cut off early. Decay is usually
inaudible in isolation (`Dr=15` with `Sl=15` effectively means "no decay"), which is why the bug
sounded intermittent rather than constant. Verified by re-render: `BGM000` MD5 changed, RMS of
the file's last 3 seconds up 10.2% (longer tails). All 168 BGMs were rebuilt with the corrected
envelope (`npm run bgm:build:all`).

⚠ SFX intentionally get no envelope: they are one-shot PCM played at their own per-VAG rate, with
no sustain phase to hold.

Two more SEQ-level omissions, fixed alongside the program-slot correction:

- **Pitch bend** (`0xEn`) was previously discarded; it occurs in 68 of 82 SEQs (`BGM016A` alone:
  3440 events), MSB distribution centered on 64 as in standard 14-bit MIDI. Bend range is stored
  per tone: `VagAtr +12` = `pbmin`, `+13` = `pbmax` (semitones). 2088 of 2694 tones have range
  0/0 (no bend); most of the rest are 12/12 or 2/2 — a real per-tone gate, not a flat global
  value. Implemented as a running per-channel time series during rendering.
- **CC7** (channel volume, 1157 occurrences) and **CC10** (channel pan, 3016 occurrences) were
  discarded; CC10 values span the full 0-127 range, so panorama information was completely lost.
  CC7 now runs continuously, CC10 is sampled at note-on, and tone pan is applied as an offset
  from center.
- `ProgAtr.mvol` (program base volume) was never read; 254 of 906 programs sit below full volume
  (127). It is now part of the gain chain. `ProgAtr.mpan` stays unused since 901 of 906 programs
  sit at center (64) anyway.

### Area-to-track mapping

A `pQES` SEQ resource can hold up to **4 independent songs**, not 4 mixed instrument tracks. Each
carries its own tempo/PPQN header; the engine plays exactly one. Confirmed from a live save
(McNeil): the player's slot array is `ptr = *(0x801907d8 + sys·4)`, `slot = ptr + track·188`;
only track 0 carries an active stream pointer (current/loop position at `0x8011xxxx` into the
loaded SEQ). Tracks 1-3 are only initialized (default volume), never advanced.

Superseded reading: the initial renderer parsed all 4 tracks (each with its own tempo) and mixed
them into one buffer, each looping independently on its own period with shorter tracks repeated
up to the longest. This was based on the init disassembly at `0x8016b984`, which does touch every
track header during setup — that touch was mistaken for simultaneous playback. Consequence: 48 of
82 BGMs layered an unrelated song on top of the real one. Example: McNeil's `bgm000` gained a
20.2-second side-song looped 5.6 times over the real 112.8-second village theme; every battle
theme was affected too, just less noticeably.

Fix: `extract/build-bgm.ts` (`npm run bgm:build`) renders track 0 by default, or the track with
the most *playable* notes if track 0 is empty or unplayable (see Instrument/tone resolution).
`extract/build-bgm-all.ts` (`npm run bgm:build:all`) additionally exports other note-bearing
blocks as `bgmNNN_tN.mp3`; the compendium's music module labels these as song-block variants.
82/82 BGM tracks are listed; `public/bgm/index.json` (written by `build-bgm-all`) adds the 43
tracks with no area binding (battle, boss, event, fanfare) to the 39 area-bound ones.

Looping: `FF 2F` (end of track) triggers the SsSeq end handler at `0x8016d3e8`, which resets the
read pointer to the track start — the loop point is struct offset `+0x08`, populated at runtime.
Loop is always whole-track from tick 0; there is no intro/loop-point split. A whole-buffer Web
Audio loop is therefore correct.

Area-to-BGM-slot mapping lives in a resident RAM table, **not** present anywhere on disc: one
byte per area at `0x801ca7d7`, value = BGM slot number, covering all 200 areas. A disc-side table
was ruled out on three counts: SCENA scripts carry no BGM triggers, the EXE has no such table,
and no field in the per-area EMI overlay holds it. The table was located by cross-validating
three natural-save RAM anchors (`extract/bgm-find-table.ts`): exactly one offset satisfies
`T[7]=0`, `T[8]=3`, `T[33]=11`, and keeps all ~200 entries ≤ 80, in all three RAM captures.
Supporting evidence: unanchored entries also check out — `T[0]=0` for McNeil, `T[60]=26` for the
Overworld's own theme, `T[3]=1` for Cedar, all distinct.

`extract/bgm-find-table.ts` writes `public/bgm/area-bgm.json` (200 areas, 38 distinct themes);
browser-verified spot checks: area 000/007 → `bgm000`, 008 → `bgm003`, 033 → `bgm016`, 060 →
`bgm040`, 121 → `bgm013`. Slot to file: slot `N` is the `N`th `BGM*.EMI` in ISO order, per
`table[209+slot]` at `0x80182910`.

⚠ `0x80184460` is the SsSeq handle queue, not a request/trigger variable — a `bgm.ts` code
comment claiming otherwise is stale. Because `extract/warp.ts` does not load BGM (it leaves the
stale `BGM000` playing), ground truth for area-BGM mapping can only come from natural saves;
`extract/bgm-fingerprint.ts` (`npm run bgm:fingerprint`) fingerprints saves offline by matching
the loaded `pQES` SEQ against every BGM EMI.

### SFX bank system

Three independent SFX pipelines share the VAB/VB container:

| Pipeline | Source | Tool | Output | Coverage |
|---|---|---|---|---|
| Spell/effect sound | BMAGIC EMI `ctype 6`/`ctype 7` | — | `public/bmagic/sound/` | 221 WAVs for 128 of 144 effects |
| Shared battle bank | `BATL_SE` / `COMN_SE` | — | `public/sfx/` | fallback for effects with no own samples |
| General SFX library | battle/system SFX EMIs | `extract/build-sfx.ts` | `public/sfx/` | 4970 WAVs total |
| Field SFX cues | per-area cue tables | `extract/build-fieldsfx.ts` | field SFX playback | 131 distinct sets |

144 − 128 = 16 BMAGIC effects carry no `ct6`/`ct7` subfile at all; their sound instead comes from
the shared battle bank. ⚠ The source list names 15: `KAIZAR_D`, `KAIZAR_F`, `KAIZAR_N`,
`MAGIC001`, `MAGIC002`, `MAGIC008`, `MAGIC010`, `MAGIC012`, `MAGIC015`, `MAGIC045`, `MAGIC060`,
`MAGIC065`, `MAGIC080`, `MAGIC112`, `MAGIC117` — one id is missing from the record against the
stated count of 16. The per-effect trigger that selects which shared sound plays for these is
unresolved; the SFX id lives in the effect's `ct0` script, with engine call candidate
`0x8014e6f0` operating on a structure at `0x80028fcc`.

Field SFX cue byte layout, read by the bank-handler routine at `0x8015ec48`:

| Field | Meaning |
|---|---|
| byte 2, high nibble (`b2>>4`) | tone index within the VAB |
| byte 2, low nibble (`b2&0xf`) | unused |
| byte 3 | volume, 0-127 |
| `fine` (from the resolved tone) | pitch detune, per Pitch and envelope |

**Final rule: byte 3 is volume; the sample plays at its own base (center) rate**, detuned only by
`fine`. An open finding preceded this fix: 646 of 1079 field SFX cues (60%) computed a rate at
the upper clamp (48000 Hz) in `build-fieldsfx.ts:117` (`Math.max(4000, Math.min(48000, …))`),
with 22 more pinned at the lower clamp (4000 Hz) — together 62% of all field SFX cues clamped,
showing the rate formula did not hold. The same byte-3 value was, at the same time, written into
the cue object under the field name `vol` — an internal contradiction pointing at the fix.

Four candidate readings were tested statistically across 131 field SFX sets (873 cues):

| Reading | in playable range | too low | too high | median rate |
|---|---|---|---|---|
| A — byte 3 as note (original) | 270 (31%) | 7 | 596 (68%) | 26222 Hz |
| B — note = center | 873 (100%) | 0 | 0 | 44100 Hz |
| C — `b2&0x0f` as note | 735 (84%) | 138 | 0 | 8751 Hz |
| D — `b3&0x3f` as note | 387 (44%) | 11 | 475 | 20812 Hz |

Reading B is the only one with no clamping; A/C/D are statistically excluded. After adopting B:
0 of 1079 field SFX cues sit at a clamp (previously 668, 62%); rates cluster at 44100 Hz with
fine-driven deviations up to 45866 Hz. ⚠ This is a plausibility conclusion from statistics, not a
confirmed emulator ground-truth match — a spectral comparison against a known effect (footstep,
door) would settle it.

A separate, smaller correction applies to `build-sfx.ts`: `ToneAtr +6`/`+7` hold a tone's
min/max note range per the Sony VAB spec. The renderer used to take `min` as the played note,
correct only for single-note key splits. Measured across 14 EMIs (198 tones): 172 have
`min==max` (unaffected), 26 have a wide range, of which 18 have `min=0` — previously producing
absurd rates (one BATTLE-bank tone, `vag9`, computed 193 Hz instead of roughly 44100, a 228×
error). Fix: `note = (min === max) ? min : center`. ⚠ In practice this changes none of the 4970
rendered WAVs, because `rateByVag` only consults the first tone per VAG, and that tone always has
`min==max` — the fix is precautionary, not an audible correction.

### XA streams

Coverage in this material is limited to a Playwright audit confirming 81 language/XA buttons and
31 XA clips play without error, alongside a separate check of screen and FMV video playback. No
cue table, sample rate, or decode address for XA streams is documented here; see Open.

### Playback in a browser

`src/audio.ts` (`Bgm` class) decodes each pre-rendered track with Web Audio
(`decodeAudioData`) and loops it via an `AudioBufferSourceNode` with `loop=true` — a gapless
whole-buffer loop, avoiding the gap an `<audio>` element's codec introduces at loop boundaries.
Autoplay unlock happens on the first user gesture via `AudioContext.resume()`. Track selection
per area comes from `public/bgm/area-bgm.json` (via `loadBgmMap`), falling back to a static
`AREA_BGM` table. A mute toggle lives in the UI panel; `__bof3.audio` exposes a debug hook.
Verified by Playwright plus ear: area 000/007 → `bgm000`, 008 → `bgm003`, 033 → `bgm016`; track
switches per area, playback and toggle both work.

`extract/build-bgm.ts` (`npm run bgm:build`) renders one BGM completely: VAB programs/tones
become one voice per note, with pitch and ADSR envelope per Pitch and envelope above, and
pan/velocity/master volume applied. **Voice stealing per (channel, note)** cuts the previous
voice on a retrigger — otherwise overlapping release tails stack into an "echo"/wash. Mixed
output is normalized to −3 dB and written as WAV, or, with `--mp3`, encoded via `ffmpeg` at
128k. `--voices N` is a debug flag limiting rendered voice count. `extract/build-bgm-all.ts`
(`npm run bgm:build:all`) renders all 81 pieces to `public/bgm/`; all 81/81 produce audible
output.

Wider QA: a 16-module compendium sweep found 0 broken images, 0 undefined references, and 0 HTTP
404s across spell/SFX/music sound samples. A separate Playwright sweep found 0 errors across 56
SFX player widgets and 81 language/XA buttons with 31 clips.

⚠ Testing note: a Playwright tab left open after a test keeps playing the same BGM through the
system output, audible as an echo next to a manually opened tab — close the test browser
(navigate to `about:blank`) after audio tests.

### Refuted approaches

- **All 4 SEQ tracks mixed together.** The init disassembly at `0x8016b984` touches every track
  header at startup; that touch was mistaken for simultaneous 4-track playback. Only one track
  (song) is ever active — see Area-to-track mapping.
- **SEQ program number equals the packed tone-block index.** Wrong whenever a VAB has unoccupied
  slots; the number addresses the `ProgAtr` slot instead — see Instrument/tone resolution.
- **Out-of-range program numbers resolved via `prog % numProg`.** A guess with no hardware basis;
  the correct behavior is silence, since an empty program slot plays nothing on real hardware.
- **ADSR attack/decay/release as fixed constants (4/90/60 ms).** Only ever an approximation; real
  per-tone values, read from `adsr1`/`adsr2`, differ enough (release 106-425 ms) to be audible.
- **Field SFX cue byte 3 as the played note.** Produced impossible rates (68% above the 48 kHz
  clamp); byte 3 is volume, and the sample plays at its own center rate.
- **`0x80184460` as a BGM block-request/trigger variable.** It is the SsSeq handle queue; no
  block-selection trigger has been identified there.

### Open

- Which mechanism selects a non-default song block within a multi-song SEQ is untraced; current
  renderers use a static heuristic (track 0, else the track with the most playable notes), not a
  located trigger (originally logged against request variable `0x80184460`, since ruled out).
- No direct code reference has been located for the area→BGM slot table at `0x801ca7d7`;
  consistent with a pointer-based (non-literal-address) table writer, but unconfirmed.
- Slot 0 covers 75 areas. Whether high area IDs (113-199) assigned to slot 0 truly play the
  McNeil theme, or mean "keep current music", is unverified by ear.
- 0.66% of BGM notes (882 of 134619) fall outside every tone's min/max range even on the correct
  program slot; these still fall back to `prog.tones[0]`.
- `NRPN 20` with data entry (172 occurrences) is the libsnd loop-point mechanism in SEQ data; not
  yet connected to the whole-track-loop implementation.
- ADSR tick-stepping is implemented per the documented register/tick formulas but not verified
  sample-accurate against real hardware or emulator waveform output.
- A live in-browser sequencer (for example an AudioWorklet) instead of pre-rendered MP3 tracks
  remains unimplemented — an open architectural option, not a bug.
- Field SFX rate reading (byte 3 = volume) rests on statistics, not a confirmed emulator spectral
  capture of a known effect.
- The per-effect SFX trigger for the 16 shared-bank spell sounds is unresolved beyond the
  candidate engine call `0x8014e6f0` / structure `0x80028fcc`.
- XA stream cue layout, sample rate, and addressing are undocumented in this material beyond the
  passing UI audit (81 buttons, 31 clips).
- Two BGM title/area attributions were found swapped by chroma matching against OST rips:
  `track-names.json` lists "Wyndia" for `bgm039`, but the OST track correlates with `bgm037`
  (0.744 vs. 0.256); "The Champion" is listed at `bgm104`, but correlates with `bgm078`/`bgmspc`
  (0.824 vs. 0.146). Titles without a `confidence` field are unreliable.

