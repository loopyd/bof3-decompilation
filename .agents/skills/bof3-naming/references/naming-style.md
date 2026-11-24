# Naming and origin rules

Evidence first; raw `func_*`/`D_*` goes directly to reviewed semantic name, no
compatibility aliases. Never overlay-prefix raw name; choose a different name or a
suffix such as `D_80146864_BYTE`. Data camelCase with role suffix
`Table|Strings|State`. Locals/fields camelCase, types PascalCase, macros upper
snake case. Exact C symbols and schema literals retain case even when reference
filenames are lowercase.

## Name schema

A semantic name describes work in at most four camelCase tokens and 48
characters, lowercase-initial so identifier and filename agree. Its first token
names the functional category: `render`, `state`, `input`, `resource`, `audio`,
`text`, `ai`, `menu`, `save`, `math`, `table` and `boot` are the recorded
vocabulary; another word is legal when it names the work better. `func`,
`helper`, `sub`, `temp`, `misc` and `unknown` are refused.

Identity belongs to the address, the map row and the layout, so a name never
carries the target's composed token (`BmagicMagic00100`, `World02Area10814`,
`PlcharPlp46700`), another target's token, or a trailing `_801CE404`. A record
offset or width belongs to the typed view, not to the name. A raw placeholder
(`func_801CE404`, `D_801CE404`) is not a semantic name and stays as it is; an
unevidenced role keeps that placeholder rather than an address-anchored spelling.

One name per address, unique inside the target, and no two targets of one source
parent may use one spelling for different bodies. A clash takes a more specific
object word (`renderDrawQuadBand` / `renderDrawQuadRow`). An ordinal is the last
resort and is licensed only when the bodies it separates have no evidence
separating their roles *and* their instructions agree apart from an immediate
address or constant - a table base, a passed constant, a carried pointer - or the
bodies are byte-identical. `bin/harness naming check` proves that shape and reports
`ordinal without shape licence` when a sibling with the same stem differs.
`bin/harness naming list/check` report and enforce these rules.

Use `s8/u8/s16/u16/s32/u32/f32`, hex addresses/offsets/masks/encoded values, decimal
human quantities, braces, assembly/Splat function order. Data needs `@source` and
`@kind table|rodata|bss|data`; unknown until independent evidence. Preserve proven
kind during spelling-only change. Content kind is not storage section; bytes or
typedef alone do not prove either. Use C89 `/* */`, not `//`.

Every non-address map symbol except SDK needs one origin-tagged definition in lift,
header/source declaration or `WEAK_SYMBOL_AT`. Symbols check enforces raw-prefix/
origin, not kind/semantic approval. Keep pre-promotion `INFERRED:` beside owning
metadata, observation plus missing verification; hints never justify aliases.
Hand-maintained manifest-claimed support bindings need target-map address entry.
Different spelling at same address can be deliberate typed alias, not silent drift.

Shared-map data claims that address as data in every composing target. Keep local
unless recursive address/content/runtime-role proof holds. Equal overlay addresses
or PsyQ roles never prove ownership. Check the combined SDK map
(`config/sdk/psyq.txt`) before target row; the Splat adapter composes the shared,
target and projected SDK maps, each symbol in one map. Record verified archive
member in `[psyq.libraries]`. Reviewed data exclusion uses `Cd SIZE @ ADDRESS`; `af-` alone
is recreated by replay `aa`.

Splat stubs use boundary name, not authored `@source` basename. Collision-renamed
source keeps `source_dir/<boundary-name>.c` as stub projection. Source filenames
never substitute for metadata identity. Layout/classification/binding changes
require separate owning authority, not spelling cleanup.

## One row per address

A target map holds one name per address. Two rows at the same address are rejected
as an address collision, so a second binding for an existing cell — an aggregate
view over a scalar, an alias, a differently typed extern — cannot be added
alongside it. Two names at one address are legal only *across* layers, such as an
SDK row and a target row. When a lift needs a differently typed view of a mapped
cell, change the one row's type or leave the spelling alone; do not plan on a
parallel name.

## Migration draft rule

A migration wave may draft a name from the evidenced spelling before anyone reads
the body; the draft is a proposal, not a decision. Draft in this order:

1. Strip the trailing address and every composed target token from the name.
2. Split the remaining camelCase tokens and drop connectors: `Then`, `And`,
   `When`, `Else`, `Also`, `To`, `From`, `By`, `At`, `With`, `Of`, `For`, `The`,
   `A`, `An`, `In`, `On`, `As`, `If`, `Or`, `Repeat`.
3. Take the category word from the leading verb: draw/render/fill/plot ->
   `render`; call/invoke/dispatch -> `call`; seed/set/clear/reset/advance/apply/
   update/store -> `state`; find/get/scan/read/load/lookup -> `query`;
   count/test/check/compare/is -> `check`; init/create/build/alloc -> `setup`;
   add/sub/mul/scale/clamp/lerp/shift -> `math`; play/start/stop/sound -> `audio`;
   write/print/text -> `text`; table/index -> `table`; otherwise keep the first
   token as written.
4. Keep the category plus the next three content tokens in order.

Flag the draft for review when it drops three or more content tokens, collides in
its own target, or keeps fewer than two content words: that is where the
description usually loses its differentiator. A reviewed name may replace a
draft freely; a draft never replaces evidence.
