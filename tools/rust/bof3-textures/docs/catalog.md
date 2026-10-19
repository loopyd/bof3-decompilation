# Catalog records

The index schema is `bof3.textures-index/v5`. Its `resources` array contains
physical files and EMI entries. Its `records` array contains images, palettes,
animation program metadata and cells in validated three-root banks, programs and
cells in validated resident banks, and the qualified save-icon frames and palette.
Other internal layouts remain unresolved.

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
contain 64 bytes in three-root banks and 32 bytes in save-icon sources. Program extents contain the header, steps and cell-offset table.
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
