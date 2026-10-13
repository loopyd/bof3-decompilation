# Naming and origin rules

Evidence first; raw `func_*`/`D_*` goes directly to reviewed semantic name, no
compatibility aliases. Functions verb-led camelCase, role-first, target prefix
only for collision. Never overlay-prefix raw name; choose different name or suffix
such as `D_80146864_BYTE`. Data camelCase with role suffix `Table|Strings|State`.
Locals/fields camelCase, types PascalCase, macros upper snake case. Exact C symbols
and schema literals retain case even when reference filenames are lowercase.

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
or PsyQ roles never prove ownership. Check shared SDK maps before target row;
Splat composes both, each symbol one map. Record verified archive member in
`[psyq.libraries]`. Reviewed data exclusion uses `Cd SIZE @ ADDRESS`; `af-` alone
is recreated by replay `aa`.

Splat stubs use boundary name, not authored `@source` basename. Collision-renamed
source keeps `source_dir/<boundary-name>.c` as stub projection. Source filenames
never substitute for metadata identity. Layout/classification/binding changes
require separate owning authority, not spelling cleanup.
