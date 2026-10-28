# Catalog records

The index schema is `bof3.textures-index/v6`. Its `resources` array contains
physical files and EMI entries. Its `records` array contains images, palettes,
animation program metadata and cells in validated sprite roots, programs and
cells in validated resident banks, and the qualified save-icon frames and palette.
Other internal layouts remain unresolved.

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
