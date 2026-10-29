# Catalog records

The index schema is `bof3.textures-index/v8`. Its `resources` array contains
physical files and EMI entries. Its `records` array contains images, palettes,
animation program metadata and cells in validated sprite roots, programs and
cells in validated resident banks, and the qualified save-icon frames and palette.
Area payloads additionally expose directly selected packed words and their
coordinate-read spans. Qualified `0x800E3800` payloads additionally expose
eight-byte `sprite-definition` records. Other internal layouts remain unresolved.

Each `sprite-definition-table` resource has `sprite_definitions` metadata with
`definitions` and `tail_bytes` counts. Its storage is
`us-counted-sprite-definitions`. The byte count precedes the records in a
four-byte header; only its first byte is interpreted. Each record retains its
entry-payload offset and ID, CLUT selector, opaque byte, page, V base, depth byte
and group. The original `SLUS_004.22@0x8014DD3C` lookup selects the first matching
ID, so duplicate IDs remain distinct ordered records. Record uses identify their
table slot, ID and group without assigning an active entity or game category.

The original initializer at `0x8014EBB8` stores definition base `0x800E3804` and
resident base `0x800D3800`. These initial values do not establish current RAM
producers or runtime selection for every archive. The physical tables retain
unresolved `texture-source`, `palette`, `entity-selection` and `resident-base`
dependencies. Their four editable draw-state bytes do not establish complete
texel, CLUT, direction or loading associations. Verification retains
`partial-sprite-definitions`; IDs, groups and opaque bytes are immutable.

`area_surface` reports width, height, cell count, unique selected words and
coordinate-read spans. `surface-word` records have four entry-payload bytes.
`surface-coordinate` and `surface-coordinate-pair` records retain four- and
eight-byte reads, including overlapping views, as preservation-only records.
These reads are not complete table allocations. Cell ID zero selects no record;
unselected words, gaps and tails are not promoted to decoded geometry.

For qualified area archives, `area_surface.request` contains the original
16-bit `index`, request `slot` and disc `lba`. The selected original-disc extents
match the complete archive bytes. This mapping covers indices 0 through 199;
it does not establish an argument bound in the game. Query `--dependency
area-request`, `--dependency area-slot-lba` or `--dependency area-palette-stage`
to inspect the required GAME and EXE consumers. Missing consumers remain
unresolved; captured consumers retain partial status because execution and
intervening writes have not been established.

The conditional GAME path requests `(s0 & 0xFFFF) + 683`, waits for
`byte[0x80146494] == 3`, then calls `SLUS_004.22@0x8014E284`. On stable valid
memory, this routine clears `[0x80037000,0x80037800)` and copies 32 rows of
512 bytes from `0x80033800` to `0x80037800`. The final four copied rows are zero.
This static relationship does not identify a surface's palette source or prove
completed uploads, later palette mutations, CLUT selection or display.

The original `SLUS_004.22@0x80154294` consumer selects a word at
`grid + align_up(2 * width * height, 4) + 4 * cell_id`, where
`grid = 0x80104000 + 4 * u16[0x80104002]`. The selected word reaches the coordinate
consumer at `0x80155560`. Its CLUT and page calculations do not identify a
physical palette or texture source on their own. General dependencies, complete
consumer uses and runtime appearance remain unresolved and queryable.

One original `BIN/WORLD00/AREA024.EMI#13` surface word at payload offset 58300,
grid record 563, retains conditional `texture-source` and `palette` edges.
Query it with `--records --source AREA024 --kind surface-word --dependency
texture-source`. Entries 7 and 8 list the record in their `uses` fields.
The original word `0x5380007a` produces page `0x0097`, CLUT `0x7980` and UVs
`(160,112), (175,112), (160,127), (175,127)` at the immediate return boundary.
Recorded CPU writes connect the constructed fields to the main-loop DrawOTag
entry. The subsequent native GPU payload matches the submitted bytes in the same
execution. A zero texture-window command precedes the packet. The RAM recording
ends at the DrawOTag entry, so later RAM writes remain unobserved. The packet's
vertices lie above the captured draw area. Actual sampling remains unresolved.

All declared entry 7 upload bytes match serialized VRAM. The nominal UV region
lies in chunk 54, and the selected CLUT row matches entry 8 words 768 through
1023. Upload history, source uniqueness and display remain unresolved.
Other palette rows contain differences whose causes and inverse edits remain
unresolved. Both edges stay conditional, and the record keeps its general
surface dependencies.

The surface word at payload offset 58940, grid record 723, also retains
conditional texture and palette edges. Its original word `0x13800095` reaches
the constructor with the complete metadata entry present in RAM. Recorded writes
connect its page `0x0097`, CLUT `0x7980` and UVs `(80,144), (95,144), (80,159),
(95,159)` to the matching GPU payload. Its encoded vertices lie inside the known
draw area with zero draw offset and texture window. Final serialized VRAM matches
all entry 7 upload bytes, including the nominal UV region in chunk 69, and entry
8's selected CLUT row. Final residency alone does not establish earlier texture
or palette reads, upload history or a unique source producer.

A combined edit replaces the 16x16 index patch with index 179 and entry 8 word
947 with `0x7c1f`. Independent original and edited media boots with the same
opening input schedule show a magenta surface patch in the opening mine. The
complete metadata entry and known packet remain equal in both final RAM captures.
This comparison does not trace the later GPU consumption or isolate the texel
and palette changes' contributions. Both edges remain conditional. Complete
shared uses, other palette-row differences and general surface dependencies
stay unresolved. See the [terrain edit guide](terrain-edit.md) for the supported
combined edit.

Resident metadata reports `decoded_programs` and `unresolved_programs` per
physical resource, counting unique program offsets. Partial banks retain
`animation-program` records for unresolved spans with `decode: unresolved-program`
and `pack: preservation-only`. Their original byte extents and refusal reasons
stay queryable through `animation-reference` dependencies. Decoded records keep
their structural references; `shared-program-impact` remains unresolved when
other programs have unknown RAM dependencies. Coverage sums these counts across
physical resources, including distinct original copies. It does not deduplicate
separate shipped locations or establish consumer reachability.

Qualified original US EMI types 6, 7, 8 and 10 have the physical kind
`audio-owned-container`, with storage `vab-header`, `vab-body`,
`audio-auxiliary-container` and `sep-sequence-container`, respectively.
Their source hashes, sizes and entry locators remain in
the catalog. The disposition requires an exact canonical source, including its
original TOC. Changed or unqualified sources are rejected.

These containers have `decode: delegated-to-audio`, `pack: preservation-only`
and `evidence_status: owner-qualified`. Textures preserves unused fields,
allocations, padding and opaque bytes. Ownership does not prove that every byte
is sound data or graphics-free. It does not establish bank pairing, loading,
runtime residency, or SFX and vocal roles. The original type-8 handler at
`0x801629F0` selects the audio slot's auxiliary destination. This establishes
ownership without identifying all auxiliary bytes as ADSR data. Archive gaps
and media streams retain their separate unresolved dispositions.

Use `query INDEX --kind audio-owned-container` to inspect these resources.
Coverage reports their container count and declared payload bytes separately;
they are not decoded graphics. Use the
[audio catalog](../../bof3-audio/README.md) for audio interpretation and supported
editing.

Use `query INDEX --records --kind animation-program` to find program records.
Omit `--records` to query physical resources. Both scopes support `--source`,
`--asset`, `--category`, `--storage`, `--kind`, `--palette`, `--animation`,
`--dependency` and `--evidence`. Source matches substrings; other values match
exactly. `--animation ID` matches the named program and its referenced cells.
`--palette ID` matches the palette itself and any verified palette references.
An unresolved draw CLUT does not create such a reference.

`query INDEX --dependency glyph-layout` finds the conditional 8x12 glyph mapping
for `FIRST.EMI` entry 3. The edge targets the original executable. It is
`unresolved` when that source is absent and `partial` when captured. Equal atlas
copies retain their own consumer dependencies.

The ordinary-byte branch at `SLUS_004.22@0x8014FA58` computes `d = byte - 48`,
`q = trunc(d / 31)` and `r = d - 31*q`. It stores the low eight bits of `8*r`
as U and `72 + 12*q + crop_top` as V. Width is 8 and height is
`12 - crop_reduction`. Initial flag bits 4 through 7 select one of 16 crop records at
`0x8017F580`; each record stores `crop_top` followed by `crop_reduction`.
The low four palette flag bits select CLUT `0x7800..0x780f` in FIRST entry 8.
These formulas require nonaliasing source and packet storage and preservation
of the relevant saved registers across calls.

Control `0x15` takes a separate two-byte glyph path. It computes
`n = next_byte + 91`, `q = trunc(n / 21)` and `r = n - 21*q`. U is the low eight
bits of `12*r`; V is the low eight bits of `12*q + crop_top`, without the
ordinary path's `+72`. Both paths emit width 8 and advance X by 8 despite the
extended path's 12-pixel UV pitch. Controls `0x05` and `0x06` change the
low-nibble CLUT state without reloading the initial crop.

Controls `0x00`, `0x01`, `0x05`, `0x06`, `0x07`, `0x15` and `0xff` have separate
dispatch paths. `0xff` advances X without emitting a rectangle. The edge
describes source selection, not text decoding or complete glyph enumeration.
Active texture-window state, GPU submission and display remain unresolved.
Existing raw atlas and palette views control image edits.

Each record ID contains its physical parent ID, coordinate space, kind and byte
offset. Equal bytes at different offsets have different IDs. Repeated table
pointers merge into one record with all table-slot uses. `parent` identifies
the physical resource, and `coordinate_source_sha256` binds the full byte array
that owns the offset. `sha256` binds the exact record bytes.

`decoded-bank` offsets address the decompressed bank, including its root table.
They are not archive offsets. `entry-payload` offsets address the qualified raw
bank, resident-bank or save-icon payload. The parent retains the archive offset
and physical source hash.
Image extents include only consumed header and stream bytes. Palette extents
contain 64 bytes in sprite roots and 32 bytes in save-icon sources. Decoded program extents contain the header, steps and cell-offset table.
Unresolved program extents include the entire immutable allocation up to the next
distinct program or group boundary.
Cell extents contain the count byte and five bytes per part. Allocation tails
remain opaque.

Program-to-cell dependencies are verified structural table references. Cell-to-program
dependencies let a cell identify the programs that reference it. These
edges do not establish execution, selection domains, image residency, draw CLUTs,
texture pages or upload order. Selection tails are not enumerated as records.
The save-icon frame-to-palette references follow the original header producer.
A palette query returns the palette and its three source frames. Each original
archive copy retains its own IDs. These source relationships do not establish
BIOS timing or edited display. Record edits remain subject to the parent
package's supported edit rules.

Coverage schema `bof3.textures-coverage/v3` reports physical resources separately
from logical records and their kinds. Each category has `physical_resources`
and `logical_records`, sorted and deduplicated by ID, with
`physical_resource_count` and `logical_record_count` respectively. Each member
retains its source, entry, extent, hash, evidence status, evidence references and
dependencies. Physical members retain their findings; logical members retain
their parent, coordinate space and coordinate-source hash. The release and
qualified inventory remain identified by the coverage report.

These lists expose existing catalog assignments. Structural record evidence does
not prove category enumeration or consumer coverage. Category dispositions remain
`unresolved`, including categories with no members; an empty list does not prove
absence. Logical extents may overlap, so their sizes must not be added to physical
source bytes. Strict coverage still fails while logical enumeration, game roles,
consumer bindings or runtime acceptance remain incomplete. Old indexes and
packages must be regenerated with the current executable.

Catalog files and package manifests have a 512 MiB read limit. Each original
source file keeps its separate 128 MiB limit. Readers refuse files above these limits and reads whose size changes. The larger document limit allows the full corpus to retain
individual records without changing source-file limits.
