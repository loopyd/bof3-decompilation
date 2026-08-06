> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 18. Method: static disassembly, ground truth and verification

BoF3's engine is a small set of table-driven subsystems. All game code sits as code overlays
on the disc, loaded to fixed RAM addresses depending on the active mode — which makes almost
every subsystem statically disassemblable straight from the disc. Live RAM, GPU dumps, the
browser client, and audio exist to verify a static reading and to fill the handful of gaps a
static read cannot reach, mainly values an overlay assigns only at runtime.

### The toolchain

| Script | Purpose | Invocation |
|---|---|---|
| `extract/mips.ts` | Static MIPS disassembler for code overlays and raw RAM images | run against a RAM image, e.g. on `ram1.ram.bin` |
| `extract/parse-savestate.ts` | Parses a DuckStation savestate fully offline: complete RAM plus VRAM | reads a `.sav`; also accepts the newer `"DUCCT"` magic (a DuckStation version bump loosened the check to any `"DUCC"` prefix) |
| `extract/gpudump.ts` | Parses an F8 GPU dump: the primitive list as actually drawn that frame | reads a dump captured in-emulator with the `F8` hotkey; a slow-motion series captures an animation frame by frame |
| `extract/gdb.ts` (`GdbClient`) | Talks to DuckStation's built-in GDB server: breakpoints and register reads | `setBreakpoint`/`removeBreakpoint`/`waitStop`/`readRegisters` over the MIPS `g` packet (`r0..r31,sr,lo,hi,bad,cause,pc`) |
| `extract/hidkey-pid.py <AppName> <keycode> [hold]` | Posts synthetic pad/keyboard input straight to DuckStation's process, without window focus | Cross = key `K` (keycode `40`), Circle = `L`, D-pad = arrows (`settings.ini [Pad1]`); TogglePause = `Space` (keycode `49`) |
| `extract/warp.ts` | DuckStation cheat-warp into any area and tile, for deterministic ground-truth capture | pair immediately with an `F8` GPU dump once the warp lands |
| `extract/render-savestate.ts <sav> <out.png> [hlClutX hlClutY]` | Offline scene reconstruction from a savestate (RAM quads + VRAM), with an optional CLUT highlight | e.g. localizing which CLUT an entity draws from, when no `F8` dump exists |
| `extract/quads-from-savestate.ts` | Scans `POLY_FT4` frame quads directly out of a savestate's PSX RAM (9-word packets) | feeds `build-overlays.ts`; validated 315/324 = 97% against the `door2` dump |
| `extract/build-fieldchar.ts <dump> <out.png> [clutX clutY] [x0 y0 x1 y1]` | Extracts one CLUT's on-screen quads from a GPU dump, rasterized at their true bounding box | the optional screen-bbox filter isolates one object out of a multi-object CLUT |
| `extract/build-overlays.ts` | Builds map-decoration overlays from dump/savestate quads | applies the wall-vocabulary, rect-top, anchor-sanity, and repetition-discriminator filters (see GPU dumps) |
| `probe-wallverify.ts` | Statistically checks wall-package orientation and corner/top permutations across many dumps at once | modes `ends`, `corners`, `tops` |

### Static disassembly

Code overlays load at a handful of fixed addresses, so naming the address alone is not enough —
the currently active overlay has to be named with it:

| Overlay | Resident at | Contents |
|---|---|---|
| `GAME.EMI` ct0 | `0x80195a00` (229 KB) | Field engine: field FSM `0x80197178`, SCENA API helpers `0x8019fc28`/`0x801c00b8`, entity stepper in the `0x8019xxxx`/`0x801Bxxxx` band |
| `STATUS`/`SHOP`/`SISYOU`/`COMMU`/`BATE`/`SHISU`/`LOAD`/`BATTLE`/…`.EMI` | `0x801d0c00` | System modes: main menu, shop, masters, fairy village, fishing, game over, battle. Only the active mode is resident, so an address in this band means nothing without naming which overlay owns it |
| per-area init overlay | `0x801f2c00` | Warp table |
| per-area `SCENA##.EMI` | its own file | Compiled MIPS cutscene script |

`extract/mips.ts` reads these overlays without running the game at all, and it is the fallback
whenever heuristic parsing of a raw format stalls. PL034's field-sprite geometry is the clearest
case. A heuristic 5-byte vertex extraction produced only unstructured noise, misread as evidence
of a blocked or encrypted format. The actual problem was an assumed 3D-mesh structure — the real
format is a 2D sprite stack. Once that model was corrected, `probe-plchar-decode.ts` scanned
the whole ct1 container for valid sub-mesh headers and found 235 sub-meshes (2462 quads) with
coherent, recognizable character texels; `probe-plchar-grid.ts` montaged them individually for
inspection. Assembling those sub-meshes into one coherent frame stayed ambiguous from the data
alone: `probe-plchar-assemble.ts` tried two candidate record-entry layouts and got only fragments
from both. That ambiguity forced disassembly of the actual sprite interpreter (`0x8014c62c`, `0x8014c83c`)
and its animation sequencer (`0x8014d9e0`), which fully explained the mechanism. A
disassembly-faithful reconstruction (`probe-plchar-seq.ts`) then produced a coherent central
figure, plus a "cross" of parts still anchored on the wrong base. That remainder was resolved only
by the savestate read described in Savestates below.

Disassembly also locates writers that a black-box trace alone would misattribute. The battle
HP-apply writer sits at `0x801dbd6c`, inside the battle overlay: `HP_new = clamp(HP_old − $s1)`,
with `$s1` the damage and `$s0/320` the target actor index. The resident `clampAdd` at
`0x80165824` looked like the obvious candidate but never fires in battle at all — it belongs to
a different code path. Enemy HP has its own, separate writer. Ad-hoc tracer scripts
(`scratchpad/dmg-*.ts`, `battle-trace*.ts`) confirm a writer like this by catching it live, once
GDB is confirmed unpaused (see The warp cheat and deterministic capture). A Z0 instruction breakpoint on the
known-good `rand` call at `0x8017e8a0` confirmed the GDB breakpoint pipeline before it was
trusted against unverified addresses; Z0 breakpoints fire reliably, Z2 data watchpoints only
sporadically (see Pitfalls).

### Savestates

`extract/parse-savestate.ts` reads a DuckStation savestate fully offline: complete PSX RAM plus
VRAM, no running emulator required. It accepts both the original magic prefix and the newer
`"DUCCT"` format a DuckStation version bump introduced, loosened to match any `"DUCC"` prefix.

`extract/quads-from-savestate.ts` scans a savestate's RAM directly for `POLY_FT4` frame quads, in
9-word packets, with no GPU dump involved. Validated against the `door2` dump at 315/324 = 97%,
it feeds `build-overlays.ts` directly — every savestate becomes a source of overlay geometry,
without an F8 capture, a GUI, or a capture-walk.

`extract/render-savestate.ts` reconstructs a scene fully offline from a savestate's RAM quads and
VRAM, with an optional CLUT-highlight overlay. It is too noisy for a clean ground-truth
comparison — an `F8` GPU dump stays the better source for that — but it is useful for localizing
which CLUT one specific entity draws from.

`probe-ss-region.ts`/`probe-ss-visual.ts` check whether an overlay's screen registration can be
trusted. AREA008's outdoor forest scene failed this check: deep sightlines with no occluders
leave distant tiles perspectivally distorted, so the overlay pipeline's affine deprojection
scatters them (473 "floor" blocks, fragmented across a 40×38-tile area). Self-similar bark and
leaf texture then flattens the registration-score landscape, so a global fit and a per-component
fit disagree outright (`@37,45` vs. `@25,35`). AREA007's village scene, by contrast, has short
sightlines and visually distinct tiles, giving a sharp registration peak. ⚠ AREA008's overlays
stay uncommitted until an interior-style dump — short sight, distinctive tiles — replaces the
outdoor one. ⚠ A savestate's camera block also carries a `camTile` field; it is not the
registered map-tile center — a known-good door case measured it 50 tiles off — and must never be
used as a registration prior.

A savestate can also resolve a blocker that static disassembly leaves open. The PL034 field-sprite
interpreter was fully disassembled except for one runtime-assigned base pointer, set at load time
and absent from the static ct1 data. Testing a persistent, signature-scanned context struct
against a live savestate (`mcneil_ent.sav`, context at `0x80145e90`, `anim=4`) ruled that
hypothesis out too. The actual mechanism: the engine streams only the current frame's texture
into a single sprite-VRAM slot at runtime — confirmed because the down-walk sprite decodes clean
in a savestate captured mid-walk, but as garbage in one captured at a standstill. The resulting
builder, `extract/build-plchar-anim.ts`, turns decoded frames into four directional sprite
sheets; its `--probe <savestate>` mode harvests further mid-step captures for the directions that
still only have a standing pose (see Open).

### GPU dumps

An `F8` dump captures the exact primitive list the PSX GPU drew that frame. Every runtime
sprite — field characters, NPCs, and decor entities alike — sits in sprite VRAM at row `tp y=0`,
so scanning that row across a dump finds every drawn entity at once. Player and NPC sprites use
CLUT rows `495`/`496`/`499`/`500` (row `495` is the field-character CLUT, purple hair — this is
Teepo/PL034, not Ryu, correcting an earlier wrong assumption; row `496` is NPCs). Static decor
entities sit on their own CLUTs outside that range: a sign at `clut(192,483)`, a well at
`clut0,486`, and a plant/candle/stump group at `clut0,490` (one quad each, isolated individually
by bounding box). `extract/build-fieldchar.ts`'s optional `[x0 y0 x1 y1]` bbox filter cuts one
object out of a multi-object CLUT — a Cedar signpost isolated this way came out a clean 19×32
sprite.

Raw sprite rasters carry stray garbage texels — loose vertical stripes or dots next to the real
sprite, sampled from unrelated VRAM. The fix is connected-component labeling (8-neighborhood):
keep only components at least 30% of the largest component's size. That removed 12 stray pixels
from the Teepo extraction and 20 from an NPC extraction; decor sprites already came out at 0%
stray pixels. The foot shadow is not stray garbage — it is part of the original sprite and stays,
since it grounds the character visually.

`build-overlays.ts` turns raw dump/savestate quads into map-decoration overlays through four
filters, chained in sequence:

- **Rect-top filter** — an overlay whose UV box equals the area's own tile-top rect box is map
  geometry, not a decoration entity, and gets discarded.
- **Anchor-sanity filter** — compares an overlay's bottom edge (`hBot`) against the floor height
  at that edge: within 4 units snaps into place; buried more than 4 units below, or floating more
  than 16 units above, means mis-registration, and the candidate is discarded. Duplicate
  registrations of the same overlay at a neighboring offset are also deduplicated here.
- **Wall-vocabulary filter** — a candidate whose UV box matches a map-wall word on the same wall
  axis (south: same column; east: same row, within ±2) is a row-shifted registration of a real map
  wall, not a distinct overlay, and gets discarded.
- **Repetition discriminator** — a texture that recurs collinearly 3 or more times, or 5 or more
  times anywhere in the scene, is a tiled wall texture rather than a unique decoration, and gets
  discarded. This is what keeps dense village scenes from producing an explosion of false
  positives (see Open).

`probe-wallverify.ts` turns a whole corpus of dumps into one statistic instead of a handful of
spot checks. Mode `ends` counts which end of a wall texture package (`v0`) lines up with south
versus east across every sweep dump at once — 97 south hits against 2 outliers. Modes `corners`
(rotation-bit permutations) and `tops` (steep rect tops, including `rot=7`) came back without
exception correct. A separate full UV-corner check across 21 sweep/`area000` dumps closed a
"wrong door texture" doubt at 320 HIT / 0 MISS.

A short series of dumps, diffed against each other, also settles whether an animated CLUT belongs
to the map or to a runtime entity. Capturing a few frames with `extract/warp.ts` and diffing VRAM
across them showed that AREA008's room CLUT cycle (row 483, columns 97-110) belongs to a runtime
entity that no map tile actually references, while the Overworld sea (AREA060) uses a genuinely
static water CLUT (rows 484/485) — proof that map-tile water animation, where it exists at all,
is not a general shader effect waiting to be added.

Because every drawn entity sits at `tp y=0`, a scan of that row across the entire dump corpus can
prove a negative, not just find a positive: nothing beyond the player/NPC CLUT rows and the two
Cedar signs turned up anywhere. That is what closes a claim like "nothing else is missing" — a
corpus-wide scan, not the absence of further reports.

### The warp cheat and deterministic capture

`extract/warp.ts` pokes a DuckStation cheat that warps into any area and any tile on demand, then
pairs immediately with an `F8` GPU dump. This replaces the manual capture-walk — reaching a
specific tile no longer means navigating the game by hand — and makes ground truth for any
location reproducible on request, not something to stumble into.

Focus-free input removes the second manual bottleneck. `extract/hidkey-pid.py <AppName> <keycode>
[hold]` posts keyboard/pad events with `CGEventPostToPid`, straight to DuckStation's process ID,
reaching pad input without window focus. The earlier `hidkey.py`, built on `kCGHIDEventTap`,
needed the window focused and blocked the operator's machine for the duration. Focus-free input
unlocks every later capture automation — menu navigation, battles, story progress — without
blocking the machine it runs on.

⚠ **DuckStation's GDB server halts emulation the instant a client connects**, and neither a `D`
detach nor a `c` (continue) plus destroying the connection reliably releases it. This was the
cause of every earlier sporadic live trace: the game sat still for most of the capture window.
Traces read as inconsistent for reasons that had nothing to do with the thing being measured.
The reliable fix is DuckStation's own `TogglePause` hotkey, `Space` (keycode `49`), sent through
`hidkey-pid`. Recipe: set the needed GDB breakpoints or writes, detach, send `Space` via
`hidkey-pid`, then let emulation run.

⚠ A warp into `area0` needs DuckStation `pkill`-ed and relaunched with `--wait 28`, to guarantee a
fresh door state — reusing an already-running instance leaves stale state behind. ⚠ zsh does not
split an unquoted `$var` the way a loop over `key:value` triples assumes; parse each triple with
`${spec%%:*}` instead of relying on word-splitting.

### Verification rules and what counts as proven

The standing verification loop cross-checks a finding across five independent channels: static
decode, live RAM (savestates), GPU dumps (an `F8` capture, or a slow-motion series for
animation), the browser client itself (driven with Playwright), and — for audio — listening
directly. Order of work is fixed: run `probe-*.ts` forensics first, only then write a productive
`build-*.ts`, and only then document the result. A task counts as done when another model can
implement it from the data plus the documentation alone, without repeating the
reverse-engineering.

Concrete rules that follow from this loop:

- **A single dump is an anecdote; a corpus is evidence.** The wall-package orientation rule
  ("south and east are built identically") looked correct from a handful of examples, but
  `probe-wallverify` mode `ends`, run across every sweep dump at once, found 97 south-end hits
  against only 2 outliers for east walls — the earlier rule was backwards. A door-texture doubt
  was closed the same way, at 320 HIT / 0 MISS across 21 dumps.
- **A correction must regression-test against fixed ground truth before it replaces the old
  output.** `extract/build-worldmap.ts` replaced `build-warps.ts` as the canonical warp-table
  reader only after it reproduced AREA007's known 17/17 ground-truth warps with zero
  regressions, while also restoring 142 warp edges the old reader had silently dropped.
- **"Verified 1:1" is a claim about specific checked spots, not a blanket state.** Confirming a
  pond and a few houses against recon images is not the same as confirming an area. A proper
  recheck goes section by section instead — every section warped, dumped, and compared against
  both its recon image and the browser render in turn — rather than stopping once the sections
  with existing recons pass.
- **Before reporting something missing, zoom in and pan the camera first.** Reported gaps have
  repeatedly turned out to be framing errors — an object rendered correctly but sat outside a
  tight close-up crop. Confirming genuine absence requires ruling this out first, every time.
- **A negative result is proved the same way a positive one is.** A missing screen or asset gets
  confirmed by reproducing the exact conditions that should trigger it and observing what
  actually happens, not by failing to find the asset in the data and stopping there.
- **A single clean sample is not a verified formula.** A writer address being pinned exactly does
  not mean the formula around it is solved — a formula fit needs multiple independent samples,
  and staying at one sample has to be reported as open, not as done.
- **Check whether static map data already explains an observation before crediting a runtime
  entity.** A dump-derived "sprite" can turn out to be a duplicate of already-decoded static map
  geometry, drawn from a different angle — the simpler, static explanation wins once it is
  checked.
- **Proving completeness needs a corpus-wide scan, not case-by-case chasing.** "Nothing else is
  missing" is only provable by scanning every dump for every instance of the relevant signature
  (e.g. every sprite-VRAM entity row) at once.

"Browser-verified" in tool output specifically means driven and observed through Playwright
against a running `npm run dev` server — not a visual guess from a single screenshot.

### Pitfalls

- **An assumed blocker is usually not the real one.** The resident `clampAdd` looked like the
  battle HP writer but never fired in battle (see Static disassembly); PL034's animation base was
  wrong on its third distinct hypothesis in a row before a savestate settled it (see Savestates).
  Trace a suspected blocker live before building anything around it.
- **DuckStation's GDB server pauses emulation on connect, and does not reliably resume on
  detach.** This produced every earlier sporadic live trace before the fix (send `Space` via
  `hidkey-pid` after detaching) was found — see The warp cheat and deterministic capture.
- **Z2 data watchpoints are unreliable in DuckStation's gdbstub** — they fire only sporadically.
  Z0 instruction breakpoints fire promptly and should be preferred wherever the target can be
  expressed as one.
- **A small sample can flip a rule's polarity.** The warp table's `b9` byte was read as a
  2-value constant (`{2,3}`) from limited examples; the resulting reader kept only the longest run
  matching that assumption and silently cut every inter-region overworld warp, fracturing the
  walkable world into 19 disconnected clusters without any error being raised.
- **Deep sightlines and self-similar texture defeat savestate overlay registration** (AREA008,
  see Savestates) — outdoor scenes like it need a different capture, not a better fit algorithm.
- **A savestate's `camTile` field is not a map-tile registration prior** (see Savestates) — a
  known-good door case measured it 50 tiles off the actual registered tile.
- **Noise from a heuristic parse is not proof of a blocked or encrypted format** (see Static
  disassembly) — it can just as easily mean the wrong structural model. Guessing harder at
  PL034's byte layout never worked; disassembling the interpreter did.
- **A GPU-dump "sprite" can be a duplicate of already-decoded static geometry.** A dump quad at
  the shop staircases briefly looked like a runtime mesh-group entity. All four staircases are
  ordinary `walk=0x60` rect-top slope quads, drawn in their own staircase strip behind a void row
  — the dump quad was just the map geometry, seen again.

### Open

- The battle damage formula has one clean sample (`dmg=3`) against the exactly-pinned HP-apply
  writer `0x801dbd6c`. A multi-sample fit stays open: the resolver computes `$s1` before the
  apply fires, and one turn-based automation run currently yields roughly one usable hit — slow,
  but workable with the GDB space-unpause fix.
- AREA008-style outdoor overlay registration (deep sight, self-similar texture) has no committed
  fix. It needs an interior-style dump — short sight, visually distinct tiles — in place of the
  outdoor one already captured.
- Full static PL034/PLCHAR sprite reconstruction is blocked on four separate items: per-quad VRAM
  page-bit selection (quads with `U≥232` read a second VRAM page, leaving 2 of 8 frames green),
  the `animTable`→direction/group mapping, undecoded ct1 leaf control blocks (`80 00 10 00 01
  00`), and the runtime-set UV texture window (`ctx[0x26]`/`ctx[0x28]`). Ryu and Rei have never
  appeared as party leader in any existing dump, so their sprites still need a fresh capture with
  a reordered party.
- PL034's walk-cycle animation exists only for the down direction; up/left/right have a standing
  pose but no walk frames, since each needs its own mid-step savestate capture.
  `extract/build-plchar-anim.ts --probe <savestate>` is already built to harvest these once more
  captures exist.
- Overlay coverage beyond AREA007's clean set stays partial: dense central-village dumps keep
  producing unpredicted map-wall false positives (12 clean overlays growing to 45 candidates,
  roughly 33 of them false), with no working discriminator yet for that specific case.
- The battle sprite ctype7 codec is still blocked. An earlier "entropy difference" that looked
  like a distinguishing signal against ENEMY019 turned out to be a measurement artifact.
- McNeil's entrance-gate posts and crossbeam are a multi-part entity, discarded from the committed
  overlay set as poorly anchored; they still need their own reconstruction. Overlay
  `hTop`/`hBot` screen-fit remains the pipeline's weakest point in general.

