> Imported from the bof3js project (EU release, SLES_013.04). Addresses are
> EU address space — do NOT treat them as SLUS_004.22 facts; formats, record
> layouts, and rules carry over, addresses do not. Source of truth for a
> US-target fact remains our own evidence.

## 17. The browser client

### Module map

`src/` is the Three.js browser client. It renders the reconstructed world and hosts every
in-browser system, editor, and UI overlay. The table below maps the files this chapter covers
to their responsibility.

| Path | Responsibility |
|---|---|
| `src/main.ts` | Bootstrap and game loop; warp/section dispatch; wires compendium and drawer mount points; filters flame-wall quads out before `buildFeatures` runs |
| `src/world/loader.ts` | Loads areas and sections; BFS-unwraps mod-256-wrapped corner heights against already-solved neighbors |
| `src/render/terrain.ts` | Builds ground and wall meshes; alpha-tested texel transparency; wall-edge height lookup (`edgeAt`); `borrowRimKey` corner-texture patch |
| `src/render/features.ts` | Roof-cap and animated-seed overlays; billboard foot-height placement (`footY`) |
| `src/render/entities.ts` | Camera-facing object billboards, e.g. `buildTorches()` |
| `src/render/areabanner.ts` | Area-name banner overlay |
| `src/player/controls.ts` | `Keyboard` input state; ignores keydowns on form fields, clears keys on blur |
| `src/player/player.ts` | Player entity; exposes the ground height the follow/free camera anchors on |
| `src/ui/shell.ts` | Frame chrome: status line, icon bar, drawer host, help overlay, immersive toggle |
| `src/ui/playmenu.ts` | Play menu (key `Q`): tiles that start a system directly; `openEntry(id)` |
| `src/ui/charbar.ts` | Bottom bar: age switch, portrait strip, main accesses, zone corner, transformation button |
| `src/ui/i18n.ts` | Operating-UI translation catalog; `t()`, `bindText`/`bindTitle`, `onLang()` |
| `src/ui/theme.ts` | Shared color and font tokens consumed by every overlay |
| `src/ui/icons.ts` | Roughly 40 hand-drawn outline icons; `icon()`, `iconUrl()`, the `IconName` type |
| `src/ui/tooltip.ts` | Shared tooltip widget that replaces the native `title` attribute |
| `src/systems/compendium.ts` | Look-up database shell (key `G`); mounts per-module panels (`mountButtonIn`) |
| `src/systems/master.ts` | Masters module: field-sprite mapping and detail pane |
| `src/systems/dragon.ts` | Dragon-splicing module; resolves gene-splice form codes client-side |
| `src/systems/battle.ts` | Battle system and bestiary figures; `resolveEnemyFigure` fallback |
| `src/systems/party-widgets.ts` | Party module: click feedback, voice-clip buttons, standing-pose D-pad mode |
| `src/systems/slidedoors.ts` | Sliding-door leaf rendering; modulates parked vs. active leaf color |
| `src/systems/multiplayer.ts` | Multiplayer client; renders remote players; owns the drawer's `.panel` mount |

### Rendering paths

`src/render/terrain.ts` applies `alphaTest` to wall texels. A texel value of `0x0000` is
always transparent on the original PSX hardware. `terrain.ts` now matches that in the
browser — an earlier build painted it as opaque black instead, producing "curtain" artifacts.
Zero-opacity "drop" wall words are context-dependent, though: some render transparent over
water, others as a gray tread top over ground. The correct discriminator is whether the cell
`hasTop`, not the texel's own opacity. Treating every drop word as transparent regressed
several areas back into holes, and was reverted.

Walls render as one quad per edge, with a UV box scaled to the wall's map height. A theory
that some walls instead used a "stacked V-band" mode for four-unit-tall walls was refuted.
The apparent stack was three ordinary edges — a south wall, a pillar's east wall, and a north
wall — overlapping on screen. Wall bottom edges at void (unmapped) neighbors now read that
neighbor's own stored corner height, instead of defaulting to `0` (`edgeAt`). This corrects
platform-skirt heights next to void cells. Cells that carry more than one wall word on the
same edge are still only partly read (see Open).

`borrowRimKey` (`terrain.ts`) patches area 198's base ring, whose four rim corners carry no
wall words of their own and otherwise tear holes through the void guard. It borrows a
neighboring edge's texture for those corners instead, gated to area 198 specifically. A
general "borrow into any void edge" rule was tested and broke 127 other areas across 1561
edges. The fix stays area-specific until the area's actual crystal-skirt object geometry is
extracted.

`src/render/features.ts` floods roof holes with a cap quad cut from the brightest roof
texture. This covers overworld hip-roof huts whose roof is stored as sloped partial panels,
which leave the tile center uncovered. The cap quad sits at exactly the same world height as
the near-black texture underneath, so `polygonOffset` alone doesn't reliably win the
z-fight. `features.ts` also lifts cap quads by a constant `CAP_LIFT = 0.06` world units —
about half a height step, visually imperceptible — so the cap always wins.

Billboards (signs, star objects, and other feature props) place their foot at the bilinearly
sampled ground height under their anchor point: `footY = Math.max(ankerY,
grid.sampleY(cx, cz))`. This replaces placement at the raw tile's maximum corner height. The
corner-height approach had over-raised 41 of 1139 billboard placements whose map offset
pointed at a wall, furniture, or slope-top tile.

Torch flames are a 6-frame ROM animation. The extractor otherwise bakes them as a flat
frame-0 wall quad, but `src/render/entities.ts` renders them as upright, camera-facing
billboards via `buildTorches()`. `main.ts` filters the flame-wall quads out of the normal
geometry before `buildFeatures` runs, so they aren't drawn twice.

The follow and free cameras used to target `player.mesh.position.y` directly. That value's
vertical offset follows each animation frame's native pixel height — Ryu alone ranges from
39px facing down to 45px on a diagonal. Every direction change or idle fidget therefore
jolted the camera. The fix anchors the camera on `player.groundY + 0.72`, a fixed ground
height, in `src/player/player.ts`/`src/main.ts`. A 90-frame headless measurement went from
three −2px bob episodes to zero.

### World loading

Corner-height bytes in the map data store `h mod 256`. The original engine works
window-relative — a load-byte plus draw-offset rebase — so values that wrap past 255 simply
fall out of that window at runtime. `src/world/loader.ts` reconstructs true elevation with a
BFS unwrap: each corner's height is adjusted by ±256, in whichever direction brings it
closest to an already-solved neighboring corner. Component seeds are chosen as if the byte
were signed. An earlier "signed byte" reading (`248` taken as `−8`) is the special case of
this unwrap that only ever goes downward; it stays correct wherever no wrap occurs.

A `WorldGrid` can be constructed standalone, without full 3D setup, purely to answer "which
sections does this area have." `main.ts`'s `sectionsOf` callback uses this cached,
lightweight construction to feed the charbar's section list. This keeps the area catalog
itself free of world-rendering logic. `loadArea` expects an area's file name (`area000`),
not its numeric tag (`000`).

Walkability is evaluated per active section. `walkable()` only checks the currently active
section, so code that probes a different section first has to read `tileInfo().section` and
call `setSection()`. Walking onto a warp-coded tile (`0xa_`/`0xc0`) triggers the same
automatic warp the original game uses. Any code stepping through the grid programmatically
has to avoid those codes, or check `area.name` afterward to detect an unplanned area change.

### Systems

The Look-up compendium (key `G`) is a pure database: 16 modules, organized into three
meaning-groups — Combat & Characters, World & Content, Assets & Tech. This replaces an
earlier flat row of 16 equal-rank tabs. Each module renders disc-exact data and stays
explicit about what it can't show yet, rather than hiding the gap. Notable module-specific
behavior:

- **Texts** — full-text search across every area, an area-by-area text browser, and a SCENA
  dialogue browser that shows each line's resolution status. Reads `public/text/` (199
  areas, roughly 55,000 extracted strings, 1.4MB).
- **World map** — renders the `worldmap.json` graph: the main-world connected component plus
  gated side components, with their neighbor lists. An "→ enter" action quick-travels into
  the selected area via `__bof3.load`.
- **Skills/magic** — shows AP/power/element badges and each skill's master teacher.
  `renderMagic` splits spell cards into four honest classes (animated, shared texture sheet,
  shared status-text, choreography-only), instead of presenting a placeholder as a real
  per-spell animation, reading frames from `public/bmagic/anim/`.
- **Bestiary** — `renderBestiary` adds a `‹›` carousel, so a figure's different CLUT palette
  variants share one card instead of appearing as separate entries. Also exposes boss
  figures and monster sound-clip buttons.
- **Masters** — `renderMasters` shows each master's field-sprite mapping with a confidence
  footer (documented / visually strong / visual match), reading images from
  `public/masters/`.
- **Dragons** — `renderDragons` adds a swatch widget for a shape's color/element CLUT-row
  variants. `dragon.ts` resolves the active gene-splice result across 25 form codes,
  including element mirroring for hybrid forms.
- **Party** — `party-widgets.ts` repeats short animation cycles in whole increments, so
  poses stay visible regardless of native frame timing. It adds an active-state highlight to
  buttons, gives the voice-clip buttons (`▶0`-`▶5`) a green-border playing state, and exposes
  each character's standing-direction poses through a `⊙` D-pad mode.
- **Screens** — plays back the game's FMV cutscenes.

`src/systems/battle.ts` animates the party fully in combat: back-idle loops plus per-action
series triggered on `doPhysical`, replacing an earlier static image with a lunge. It falls
back to a generic boss figure through `resolveEnemyFigure` when no dedicated figure exists.

### User interface

Three top-level places cover the whole UI, one intent each, following the rule "one action,
one main location." **Play** (`src/ui/playmenu.ts`, key `Q`) starts systems directly from a
tile grid. Unavailable tiles gray out with a stated reason, e.g. "no enemies" for combat in
a town; `openEntry(id)` is a shared cross-reference entry point. **Look up** (the
compendium, key `G`) is a pure database with no launch capability of its own (see Systems).
**Tool** (the drawer, key `E`) holds settings and world/RE tooling. Cross-references between
places stay deliberately subtle — small secondary buttons, e.g. bestiary → "battle this
enemy," a spell entry → "cast in the world" — never a second main path.

**Shell chrome** (`src/ui/shell.ts`). A status line top left (`#bof3-status`) shows
location, section `i/n`, and `cols×rows`. The icon bar top right (`#bof3-rail`) holds three
buttons: language, sound, and immersive (`Tab`). An intermediate gear-flyout design was
tried and dropped (see Refuted approaches). The drawer (`#bof3-drawer`, 318px) slides over
the canvas without clipping it, closed by default. Its open/closed group state persists in
`localStorage['bof3.ui.groups']`, and the icon bar shifts left (`.bof3-shifted`, −318px)
while the drawer is open. `shell.group(id, key, icon)` returns a `{ body }` element that
other modules mount their controls into. The drawer currently groups World, Display, and
Tools. An earlier Character group (area/section/character pickers) and a catch-all Extras
group both dissolved as duplicates once the charbar absorbed their content (see Refuted
approaches). The `F1` key-bindings list is also reachable as a button inside the Tools
group.

The help overlay (`#bof3-help`, `F1` or `?`) replaces a permanent text line with four
groups — Movement/View/Play/Tools — built from the `HELP` constant in `shell.ts`; new
hotkeys must be added there. It sits at z-index 400, above every modal (compendium 100;
battle/menu/masters and similar overlays 300). Immersive mode (`Tab`) hides the bar, status
line, and mini-maps, and closes the drawer/help. `?ui=0` in the URL starts with no UI at
all, and `__bof3.ui(false)` toggles it at runtime; both exist for clean screenshots.
Mini-maps (`#bof3-minimaps`) are off by default. Shell keys are suppressed while an input
field has focus, or `isBlocked()` is true — compendium/menu/dialog/battle/transformation
open — but `F1` always stays reachable. A non-modal first-visit hint chip (bottom right, 6s)
is suppressed when `navigator.webdriver` is set, so automated screenshot tools never see it.

**Design system.** `src/ui/theme.ts` exports shared tokens —
`bg/panelBg/text/label/border/border2/btnBg/cardBg/accent/pos/neg/zero`, plus `F` fonts and
`OVERLAY/PANEL/HEAD/TITLE`. A module needing extra colors spreads the shared object:
`{ ...THEME_C, water0, water1 }`. The palette is warm-neutral anthracite
(`#17181c`/`#101115`), with gold `#c9a15a` as the sole accent for active switches, group
icons, checkbox ticks, and headers. Labels use system-ui; numbers and coordinates use
monospace.

`src/ui/icons.ts` draws roughly 40 hand-drawn outline icons on a 24-unit grid, stroke width
1.6, `currentColor`. They're used as an element (`icon()`) or a CSS data URI (`iconUrl()`).
No emoji appear anywhere in the operating UI: they render differently per platform, can't be
tinted, and look arbitrary. ⚠ The exported icon map `P` in `icons.ts` deliberately carries
no `Record<string,string>` type annotation, so `IconName` stays derived from its real keys.
Adding that annotation once let a call site pass a plain emoji string where an icon name was
expected, and the icon silently went missing.

`src/ui/tooltip.ts` reuses one tooltip element — 90ms delay, label plus key badge,
positioned from the target's bounding rect — instead of the native `title` attribute. That
attribute only appears after about a second, looks like OS chrome, and can't show a
shortcut. Tooltip text is passed as a function, so a language switch updates it without
re-registering. It only appears on `:focus-visible`, so clicking a button with the mouse
doesn't leave a tooltip stuck beside it. ⚠ `hideTooltip()` must be called explicitly before
opening a popover next to a tooltipped button, or the old tooltip stays stuck in place.

**Localization** (`src/ui/i18n.ts`). A flat catalog `{key: {de, en}}` backs
`t(key, vars)`, which supports `{placeholder}` substitution. The language choice persists in
`localStorage['bof3.lang']`, falling back to `navigator.language`. Switching languages never
reloads the page: long-lived elements register through `bindText`/`bindTitle`, composed
strings through `onLang(cb)`, and short-lived overlays just call `t()` while building their
DOM. The operating UI is fully bilingual: shell, drawer, selectors, toggles, help, messages,
dialog hints, shops, casino, the play-menu footnote, compendium tabs and frame, and overlay
headers all switch languages. Compendium and RE content text, the battle log, and RE notes
stay German-only, extendable through new catalog keys as needed. ⚠ `t` collides with an
existing time/text variable in some modules — `fishing.ts` uses it for a timestamp —
imported there as `import { t as tr }`.

**Figure bar** (`src/ui/charbar.ts`). Three centered rows: an age switch (child/adult) on
top, a portrait strip in the middle, and labeled main accesses on the bottom (area catalog,
play menu, cast spell, play, game systems). Clicking the portrait strip, or pressing
`[`/`]`, cycles the active world character. Centering uses CSS grid `1fr auto 1fr`, not
flexbox centering, so the middle content doesn't shift sideways when the right-hand
transformation button appears or disappears. That button — Accession for Ryu, Weretiger for
Rei, gold-accented, shown only when the active character has a transformation — becomes the
way back once transformed. Its label sits on the button itself, and its tooltip carries only
the explanation and the key. ⚠ Switching characters through the debug hook
`__bof3.setLeader()` must call `actionBar.refresh()`, or the button keeps showing the
previous character's transformation.

**Zone corner**, stacked left of the portraits, bottom to top: the area catalog ("where
to?"), the loaded area's name ("where am I?"), and the section selector ("which room?").
Clicking a catalog card loads that area immediately, with no intermediate section-chip step.
"Section `i/n`" opens a popover listing the loaded area's rooms (`BarAction.options()`); an
action with an empty option list simply isn't drawn, so single-room areas show no button.
The popover, not a modal, closes on an outside click or `Escape`, and calls `hideTooltip()`
when it opens. An area-catalog card can also expand its sections as chips — tile count plus
a "dark" marker; clicking one loads the area and jumps straight into that section. The
section list comes from the `sectionsOf` callback described in World loading, so the
catalog stays free of world logic. ⚠ `areaCatalog` must be initialized before
`createCharBar` runs, since the bar's first draw already needs the loaded area's name.

The area-name banner (`src/render/areabanner.ts`) originally sat at the screen row the
original game used, at the bottom edge. That overlapped the charbar on every area change, so
it now renders near the top, at a mirrored height instead. This is a deliberate, documented
deviation from the original layout, made because the charbar claims the bottom edge.

### Editors

While an editor is running, the shell shows a gold banner top-center (`#bof3-editbar`),
naming the editor and giving a concrete usage hint. This replaces what used to be a
permanently visible instruction line. The matching drawer button switches to a `.bof3-on`
state for as long as that editor stays active.

### Debug hooks

A global `window.__bof3` object exposes hooks for testing and ground-truth verification.
Several are driven by the extraction tooling's Playwright scripts, against a running dev
server (`npm run dev`).

| Hook | Effect |
|---|---|
| `__bof3.load` | Quick-travels into an area; used by the world-map compendium module's "→ enter" action |
| `__bof3.ui(false)` | Hides the entire operating UI at runtime; equivalent to starting with `?ui=0` in the URL |
| `__bof3.setLeader()` | Switches the active world character; must be followed by `actionBar.refresh()` |
| `window.__bof3.pickObj(nx, ny)` | Raycasts a screen point to its tile and reports which texture layer covers it (maptex, feature-texture cap, or wall texture) |
| `__bof3.setDLTCamera` | Rebuilds the browser camera as the exact DLT projection matrix solved from a ground-truth GPU dump |
| `captureDLT` | Grabs a frame synchronously in that DLT projection, for pixel-congruent comparison against emulator captures |

GT-comparison captures also hide the player sprite, force a black background, and turn off
the NPC layer, so only static map geometry is being compared. A non-hook gate,
`navigator.webdriver`, suppresses the first-visit hint chip, so automated sessions never
capture it by accident.

### Multiplayer

`src/systems/multiplayer.ts` drives the multiplayer client and renders remote players. Its
`.panel` used to mount inside the drawer's now-dissolved Extras group. That mount call
(`multiplayer.panel`), alongside the compendium's `mountButtonIn`, remains an unused hook in
`main.ts`'s group-wiring code. Multiplayer, like warps, cutscene control, and the debug
hooks, still reads and writes the `areaSel`/`secSel`/`leaderSel` state variables — even
though none of the three live in the drawer's DOM anymore.


### The user interface and its bilingual catalogue

**Shell.** The UI shell lives in `src/ui/shell.ts` (frame) and `src/ui/i18n.ts` (`de`/`en` catalog for the operating UI). `app.html` carries only `#app`.

**Navigation by intent.** Three places, one intent each.

| Place | Key | Role |
|---|---|---|
| Play (`src/ui/playmenu.ts`) | Q | 11 tiles, each starting its system directly. Unavailable tiles gray out with a reason (e.g. combat in a town: "no enemies"). `openEntry(id)` is the entry point for cross-references. |
| Look up (compendium) | G | Pure database. 16 modules across 3 meaning-groups: Combat & Characters, World & Content, Assets & Tech. |
| Tool (drawer) | E | Groups: World, Character, Display, Tools. The former Extras group is dissolved; compendium, shops, casino, multiplayer, and music moved out of it. |

Rule: one action, one main location. Cross-references remain only where they carry real content — bestiary entries link to "battle" against exactly that enemy, spell VFX links to "cast in the world," module headers link to "Play ▸." They are styled as subtle secondary buttons, never the main path.

**Shared tokens.** `src/ui/theme.ts` holds the color and font tokens every overlay shares. Key names match the old per-module constants (`bg`, `panelBg`, `text`, `label`, `border`, `border2`, `btnBg`, `cardBg`, `accent`, `pos`, `neg`, `zero`), so a module swaps its local constant block for the import. Modules needing extra colors spread it: `{ ...THEME_C, water0, water1 }`. Also exported: `F` (fonts — labels in system-ui, numbers/addresses in monospace) and `OVERLAY`/`PANEL`/`HEAD`/`TITLE`.

**Design.** Warm-neutral anthracite background (`#17181c`/`#101115`) with the original UI's gold (`#c9a15a`) as the sole accent color, used for active switches, group icons, checkbox ticks, and headers. Labels in system-ui, numbers/coordinates in monospace. Icons come from `src/ui/icons.ts`: about 40 hand-drawn outlines on a 24-unit grid, using `currentColor` and 1.6 stroke width. Available as a DOM element (`icon()`) or a CSS data URI (`iconUrl()`, used e.g. for the select-field arrow). No emoji anywhere in the UI — rendering differs per platform, emoji can't be tinted, and they look arbitrary next to the drawn icon set.

Caveat: the `P` icon-name map in `icons.ts` deliberately has no `Record<string,string>` type annotation. Only without it does `IconName` reflect the real, complete name list. With the annotation present, `shell.group(…, iconName)` once silently accepted a stray emoji string as a valid icon name, and the icon simply went missing.

**Tooltips.** `src/ui/tooltip.ts` replaces the native `title` attribute, which only appears after about 1 second, looks like an OS tooltip, and can't show a keyboard shortcut. One reused DOM element, 90 ms delay, shows a label plus a key badge, positioned from the target element's bounding rect. Tooltip text is passed as a function, so a language switch takes effect without re-registering. The tooltip only shows on `:focus-visible` — after a mouse click, the button keeps focus, and a tooltip lingering over its own button would just be in the way.

**Layout.** Full-screen canvas, with three elements above it. A status line top left (`#bof3-status`): location, section i/n, cols×rows. An icon bar top right (`#bof3-rail`, 7 buttons). A gold banner top center (`#bof3-editbar`) while an editor is active, showing the editor name plus a usage hint; the matching drawer button gets class `.bof3-on`. Mini-maps (`#bof3-minimaps`) are off by default.

**Drawer.** `#bof3-drawer`, 318 px wide, slides over the image without clipping it. Closed by default; opens on key `E` or the gear icon. Five `<details>` groups — World, Character, Display, Tools, Extras — with open/closed state persisted in `localStorage['bof3.ui.groups']`. While open, the icon bar gets class `.bof3-shifted` (−318 px) so it doesn't sit over the drawer's own selectors. `shell.group(id, key, icon)` returns `{ body }`; `main.ts` mounts its controls there (`compendium.mountButtonIn` and `multiplayer.panel` mount into the Extras body).

**Help overlay.** `#bof3-help`, opened by `F1` or `?`, replaces what used to be a permanent on-screen text line. z-index 400, above every modal (compendium is 100; battle/menu/masters/etc. are 300), so it stays visible from inside any open overlay. Four groups: Movement, View, Play, Tools. The key list is the constant `HELP` in `shell.ts` — new hotkeys must be added there.

**Immersive mode.** `Tab` hides the bar, status line, and mini-maps, and closes the drawer/help. For ground-truth or sweep screenshots, `http://127.0.0.1:5173/?ui=0` starts with no UI at all. `__bof3.ui(false)` toggles it off at runtime, giving a pixel-clean world image with no bar or corner element.

**Input handling.** Shell hotkeys don't fire while an input field has focus, or while `isBlocked()` is true (compendium, menu, dialog, battle, or transformation open); `F1` always stays reachable regardless. `Keyboard` (`player/controls.ts`) ignores keydown events on form elements and clears its held-key set on blur, so the character no longer keeps moving while the player types in the drawer, or after a window/tab switch.

**First-visit hint.** A non-modal chip bottom right, visible for 6 seconds, suppressed whenever `navigator.webdriver` is true — otherwise every Playwright/sweep screenshot would catch the hint in frame.

**Operating-UI translation, `i18n.ts`.** A flat catalog, `{key: {de, en}}`. `t(key, vars)` supports `{placeholder}` substitution. Language source: `localStorage['bof3.lang']`, falling back to `navigator.language`. No reload on switch: persistent elements register via `bindText`/`bindTitle`; composed texts use `onLang(cb)`, covering section, phase, cutscene, and character selectors. Short-lived overlays just call `t()` while building their DOM.

Naming caveat: `t` collides with an existing time/text variable in some modules — `fishing.ts` uses `t` for a timestamp — so those import it as `t as tr`.

Scope: the operating UI is fully bilingual. That includes the shell, drawer, selectors, toggles, help, messages, dialog hints, shops, casino, the play-menu footnote, compendium tabs and frame, and the overlay headers/close buttons of battle, fishing, masters, fairies, dragons, and Accession. Content and research text stays German by design: compendium module contents (including the "honest" RE-caveat blocks), battle log, RE notes. Extendable later via new catalog keys.

Evidence: `references/screenshots/ui-shell-2026-07-25/index.html`, 7 captures, DE and EN.

**Content translation, a second catalog: `ui/lang.ts` + `ui/lang/de-en.ts`.** Before this catalog existed, roughly 300 texts in code — compendium modules, masters/fairies/dragons/fishing, battle log, editor hints — stayed German by design, along with the RE notes embedded in the JSON game data. These sit outside the operating-UI catalog.

Why a second catalog: a key-based catalog like `i18n.ts` would need an invented key for every prose text, making the code unreadable and forcing the text to be maintained twice. Here the German text itself is the key:

```ts
el('div', css, L('Kampf'))                    // → 'Battle'
L`${n} distinkte Gegner · Stats aus den EMIs` // key: '{} distinkte Gegner · Stats aus den EMIs'
```

The tagged-template form replaces each inserted value with `{}`; the translation entry carries the same `{}` placeholders in the same order, so numbers, names, and disc values pass through unchanged. A missing catalog entry falls back to the German text — nothing breaks, the string just stays untranslated.

The trick for JSON data: master gates, dragon rules, fairy economy, and examine lists store their notes as German text inside `public/gamedata/*.json`, reaching code only as a variable. The central DOM helpers of each module — `el()` in compendium/master/fairy/dragon/fishing/battle, plus the cells built by `table()` — route every text through `L()`. This translates data-sourced text too, as long as it's in the catalog. Disc-original text (names, descriptions) isn't in the catalog and passes through unchanged.

Tools and rules: `npm run i18n:check` lists every `L` key in code with no catalog entry; target is 0. `npm run i18n:wrap <file…>` auto-wraps German display text in `L(…)`/`` L`…` ``. It conservatively skips lines with comparisons, object keys, console output, or CSS, since German-looking strings there are program values, e.g. `'c3:Erwachsen'`. Whoever changes a German text must update its catalog entry too, since the key IS the wording. The compendium redraws its open module on language switch (`onLang` → `select(activeId)`); otherwise content would stay stale until next opened.

Detector pitfall: an initial leftover scan searched the EN UI for German signal words and reported "0 remaining" — wrong. Strings like "Weltkarte," "→ Betreten," "Filter nach Area-Name/Nummer …" don't contain any of those words. The reliable test is a diff comparison: run the same click sequence in DE and EN, collect all visible text nodes, and intersect the two sets. Anything identical in both languages that isn't a disc name is untranslated. This method caught 15 more spots beyond the keyword pass; pattern in `scratchpad/scan-diff.mjs`.

Current state: 561 catalog entries; `i18n:check` reports 0 missing. A diff scan across the world, drawer, help, action book, play menu, all 16 compendium modules, fishing/masters/fairies/dragons/shops/casino, the field menu, and battle shows 0 untranslated texts. Live language switching with an open compendium is verified. Evidence: `references/screenshots/i18n-2026-07-30/index.html`.

**Area catalog.** The area dropdown used to show some areas with no name, and others with a dialog quote instead of a name — area 092 showed "Gonnatraingonnatrain…". The ROM title heuristic (`build-titles.ts`, first text-block string) is unreliable. The best available name dataset, `area-names-guide.json` with 70 Prima Guide names, wasn't even read by the browser: `friendlyName` only used titles `??` names.

Fix, two parts. First, `extract/build-area-catalog.ts` consolidates three sources into `public/area-catalog.json`. Priority order: guide names first (canonical; a trailing "(?)" flags uncertainty). Then community short form — the sentence fragment before the first comma, with the full sentence moving to the description. Then filtered ROM titles (quote detection). Then curated ground-truth cases. The ground-truth cases: area 002 = shipyard, 011 = burning treehouse, 025 = intro train, 053/090 = camp, 189 = endless plain. Result: 176 of 200 areas named; 24 stay unnamed, including 029, 040, 052, 092, 106, and 159-162.

Second, `src/ui/areacatalog.ts` is a catalog overlay in the Accession-picker style: one card per area, with a lazy-loaded maptex preview, number and dimensions, name, and a 2-line-clamped description. A search field filters by number, name, or description. The current area is outlined in gold. Escape or an outside click closes it. The drawer now shows a button labeled "NNN · Name" instead of the old dropdown (`areaSel` remains as an invisible sync element that warp/API paths still set). `friendlyName`, in `main.ts` and wall-lab, now reads the catalog, so the banner, status bar, and warp bubble stay consistent with each other.

Two UI traps found and fixed. Grid rows collapsed to 8.5 px, because `overflow:hidden` makes a card's automatic minimum size 0 while `align-content:stretch` distributes the container height; fixed with `grid-auto-rows:max-content; align-content:start`. Typing in the search field triggered global editor hotkeys (`O` opened the deco editor); fixed by having the overlay stop propagation of keydown/keyup/keypress, except Escape.

**In-browser editors and the local save server.** Every correction editor saves directly to disk. The save server (`vite.config.ts`, the `saveServer()` plugin) handles `POST /api/save/<kind>/<area>`, writing the JSON body to `public/<kind>/area<area>.json`. The kind is checked against a whitelist — `chimneys|deco|skipfill|texfix` — path-safe, dev-only. Client side, `persist(kind, data)` in `main.ts` writes to `localStorage` and POSTs on every edit, so the committable JSON is always current; `localStorage` is only the build-time fallback. Caveat: changing `vite.config.ts` requires a dev-server restart.

The tile texture editor (key `T`) fixes wrong interior textures: `buildTerrain(..., texOverride)` redirects a target tile's UV to a source tile's maptex region, leaving geometry and height untouched. Shift+click is the eyedropper, picking the source tile; a plain click applies the texture; clicking again resets it. Data shape `texOverride: Record<targetIdx, sourceIdx>` saves to `public/texfix/area<NNN>.json`. API: `__bof3.showTexFix()` / `__bof3.exportTexFix()`.

Editor key overview, all auto-saving to `public/<kind>/`:

| Key | Editor |
|---|---|
| K | skip fill |
| M | chimney |
| O | decoration (U = sprite) |
| T | tile texture |
### Refuted approaches

- **A permanently visible test UI** — a two-row hotkey wall top left, a 248px side panel,
  and two debug mini-maps bottom left — was replaced wholesale by the shell/drawer/
  help-overlay system. Mini-maps are off by default now, and the hotkey list lives behind
  `F1` instead of staying on screen.
- **The compendium as the launch ramp for interactive systems.** Fishing, masters, fairies,
  and gene-splicing used to open through one fat "interactive ▸" button inside the database
  overlay — an overlay inside an overlay. The three-places split (Play/Look up/Tool)
  replaced it; cross-reference buttons are now deliberately subtle, never a second main
  path.
- **A drawer "Extras" catch-all** for the compendium, shops, casino, multiplayer, and music
  was dissolved, once each was found a single proper main location. `compendium.mountButtonIn`
  and `multiplayer.panel` remain as unused mount hooks from that arrangement.
- **A ten-slot assignable action bar** — with its own action-book (key `F`), number
  hotkeys, and `localStorage` assignment — was removed outright. Every action it exposed
  already had a main location elsewhere: spells on `Z`, systems in the Play menu, lookup in
  the compendium. The bar only duplicated existing paths.
- **A gear flyout menu** for the top-right icon row was tried, then dropped once the row
  shrank to three buttons — a menu costs more clicks than it saves at that size.
- **Section-selector chips embedded in the area-catalog cards** were added, then removed
  again. Loading straight into the area, and picking a section afterward from the bar,
  proved simpler.
- **Drawer-hosted area/section/character pickers** were removed as duplicates of the
  charbar's zone corner and portrait strip. `areaSel`/`secSel`/`leaderSel` remain only as
  state carriers, read by warps, cutscenes, multiplayer, and debug hooks.
- **Raw pixel-difference comparison between ground truth and the browser** was a dead end.
  The PSX field camera is perspective, the browser's is orthographic, so scale and
  translation never line up between the two; phase correlation locked onto repeating village
  structures instead. Rebuilding the GT camera as a DLT projection inside the browser
  (`__bof3.setDLTCamera`) made both sides share one projection, making pixel comparison
  legitimate again as a self-test.
- **A "stacked V-band" wall mode** for four-unit-tall walls was refuted — see Rendering
  paths.

### Open

- Cells carrying more than one wall word on the same edge (e.g. two east-facing words on
  one cell) are only partly read: `collectAreaWalls` keeps a single slot per
  `[section][edge]`. Needs a data-model fix in `terrain.ts`.
- Area 198's crystal skirt still renders through the area-gated `borrowRimKey` patch,
  rather than its own object geometry. A general version of that borrow rule breaks 127
  other areas, so proper extraction of the skirt geometry remains outstanding.
- A second roof-quad rendering path is still missing for areas whose roofs are fully flat,
  solid covers, rather than the sloped-panel model `features.ts` already handles.
- Some interior levels render at a uniform height offset — about one map unit — from what
  the ground-truth camera expects. An automatic per-shot compensation papers over it,
  without a fixed root cause in the interior height model.
- VRAM animation cycles slower than roughly 80ms — CLUT cycling, sprite-frame torch and
  water ticks — fall outside the single-dump capture window the verification tooling uses.
  They stay effectively unverified against the browser's rendering.
- The "latent" object-visibility heuristic (hidden unless a save-state flag is set) is
  known to be too broad for a handful of areas, and needs per-area ground-truth
  clarification before it can be tightened.
- `public/bgm/` is missing the final-boss soundtrack — BOSS055.EMI carries its own audio
  set, not yet extracted — and still holds two stray, manifest-foreign files
  (`bgm004_t1.mp3`, `bgm004.tmp.wav`).

