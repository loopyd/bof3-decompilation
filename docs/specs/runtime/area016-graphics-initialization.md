# AREA016 graphics initialization

Reviewed original-byte finding for US SLUS graphics initialization with the
AREA016 map resident. This is a path-specific side-effect proof, not acceptance
of a compiled lift or proof of the complete area-loading lifecycle.

## Evidence identity

| Original payload | Load address | SHA-256 |
| --- | --- | --- |
| SLUS executable payload | `0x80096800` | `677754d0d22c88151a5022cd98b8e89af1b0882177d9850faf62676eb7089eff` |
| `BIN/WORLD00/AREA016.EMI#12`, 53324 bytes | `0x80104000` | `0891444cb193cd79b5eb84a10d26e345ad60315fd365694b942f2d0261881f8f` |

Addresses below refer to SLUS code or RAM containing this pinned map. Independent
review on 2026-09-17, session `01a0b049-d30a-7e40-be2e-e8bacaaf2d53`, verified
the zero-index invariant, callback effects and all eight combinations of the
three map-record conditions. Reproduce from these original payloads, not cached
disassembly or source status metadata.

## Zero-index invariant

Initializer `SLUS@0x80153DE8` clears the index halfword at offset `+2` of
each of 56 by 28 four-byte cells starting at `0x8012C000`. The clearing store
is at `0x80154108`. Leaf `0x801541C0..0x80154294` writes only cell bytes
`+0/+1`, preserving all 1568 zero indices through the loop. The initializer
sets ring offsets `0x80149324=27` and `0x80149326=55` before list processing.

Lookup `0x801558F0..0x801559AC` rejects projected coordinates outside
`0..55`, otherwise wraps them to row `0..55`, column `0..27` and returns
the low 12 bits of the cell's index halfword. Rejected coordinates return
zero too. With those initialized indices and offsets, every lookup is zero.

## Intervening map processing

Offsets in this table are relative to the map payload. Header offsets contain
little-endian halfword offsets measured in four-byte units.

| Header field | First record | Records | Zero terminator |
| --- | --- | --- | --- |
| `+0x20` | `0xBE20` | 2 | `0xBE38` |
| `+0x24` | `0xBF04` | 162 | `0xD048` |
| `+0x28` | `0xBE78` | 3 | `0xBF00` |

The preceding patcher `0x80154438` skips stores for the three `+0x28`
condition headers `0x021F,0x2800,0x2800`; none equals its required `0x8001`.
The `+0x20` callbacks selected through `0x8017F8F0` are `0x801581B4` and
`0x801583BC`. Their palette, dirty-flag and map-header writes preserve the
grid indices and ring offsets. Helper `0x801580A4` also performs no stores
for this map's `+0x22` record types `0x81/0x82` (it mutates type `0x80`).

Consumer `0x801576FC` writes nine words at `0x8010EF3C,0x8010EF40,
0x8010EF44,0x8010EF48,0x8010EF4C,0x8010EF54,0x8010EF58,0x8010EF60,
0x8010EF64` and navigation bytes at `0x8010B04A/0x8010B04B`. These addresses
are within the map, disjoint from the grid, ring offsets, record lists and
tile-index cells used for later destination calculations. Conditions select
values, not these destinations; their evaluators introduce no relevant writes.

Consequently each type-0 entry's lookup returns zero, and the branch at
`0x80157838` skips the graphics-update call at `0x801578DC` to `0x80155560`.
Induction over the entries preserves the zero-index invariant throughout
this initial traversal. Type-3 entries do not take that update path.

## Limits

The proof assumes the pinned map and initialized state remain resident,
ordinary valid stack execution without aliasing the reviewed storage, and
no external mutation during this interval. It does not cover earlier loader
yields, scenario selection/dispatch, later runtime traversals or arbitrary maps.
Keep the conditional graphics-update branch in a faithful lift: later indices
can be nonzero. This result alone cannot establish an entity's initial mode,
a global three-mode enum, shared storage ownership or a native byte match.

See [runtime layout](runtime-layout.md) for independent load regions and
[graphics format](../formats/graphics.md) for texture/palette representation.
