# BOF3 textures

`bof3-textures` catalogs original US PlayStation graphics sources and exports
self-contained archive packages. It can edit raw VRAM upload words, their
4-bit or 8-bit index interpretations, exact color-word images, and qualified
palette-word banks within their original allocations.
It also edits qualified resident animation step pairs and five-byte cell records.
Recognized three-root sprite banks have editable index planes, palette words,
programs, and cells, with both compression layers bounded by their original spans.

Full graphics support is incomplete. Category discovery lists the intended game
roles. Most resource roles, CLUT associations, sprite composition,
reverse playback callers, unsupported bank variants, and runtime appearance remain unresolved.
Strict coverage and complete verification return exit 1. A successful raw upload
edit does not establish its appearance in the game.

The catalog retains qualified original audio containers, including auxiliary
containers, as `audio-owned-container` resources. Textures preserves their bytes.
Use the [audio tool](../bof3-audio/README.md) for audio decoding and edits. This ownership
disposition does not establish sample pairing, loading, playback, or the meaning
of unused bytes. Auxiliary containers remain opaque; their ownership does not
identify all bytes as envelope data.

## Build

Use Rust 1.98.1 with edition 2024. The crate declares Rust 1.98 as its minimum.
The lockfile uses dependencies already approved in this repository. Build from
the repository root with the installed toolchain and cached dependencies:

```sh
cargo build --offline --locked --release --manifest-path tools/rust/bof3-textures/Cargo.toml --target-dir build/tools/rust/bof3-textures
build/tools/rust/bof3-textures/release/bof3-textures --help
```

PNG interchange requires installed FFmpeg with the `png` codec and both `pal8`
and `rgba` pixel formats. Publication requires GNU coreutils `mv` with `--no-copy` and
`--no-clobber`. Commands do not install these tools.

## Repository commands

The repository exposes the component as `bin/harness textures`. Each operation
builds the Rust executable offline with its installed pinned toolchain, then
forwards arguments and the exit status. Discover operations and check setup with:

```sh
bin/harness textures --help
bin/harness setup --component textures
bin/harness doctor --component textures
```

Setup builds this component from cached dependencies. Doctor checks the existing
binary, source freshness, toolchain, and external tools without building or
installing them. The standalone executable also supports every operation below.

## Source package

Create a source ZIP with installed `zip` and `unzip`:

```sh
build/tools/rust/bof3-textures/release/bof3-textures package out/textures/bof3-textures-source.zip
```

The output must be new. The archive includes both local Rust crates, lockfiles,
the MIT license, release qualification metadata, and build instructions. It
excludes game media, exported images, and build outputs. It uses the source tree
where the executable was built; `--source-root REPOSITORY_ROOT` selects another
checkout containing the two crates and the qualification manifest.

Extract the archive, then use the build command above from its `bof3-textures`
directory. The build requires the declared Rust toolchain and the locked Cargo
dependencies. Offline builds require those dependencies in the local Cargo cache.
The package does not vendor them or install tools.

## Inputs and commands

Inputs must match the complete `SLUS_004.22` qualification manifest in
[config/original-us-disc.json](../../../config/original-us-disc.json). Supply an original EMI or a complete
extracted-disc root. Generated entry dumps are not additional original files.
Other releases and PSP data are unsupported. An individual archive whose bytes
occur at several original locations requires `--source` with its original path.
Tree qualification checks every directory except the generated entry directory
beside each original EMI. It ignores root XML, WAV, and `license_data.dat`
outputs, rejects other extra files and symlinks, and bounds inventory traversal
at 65,536 entries.

Use the existing disc extractor to create a fresh input tree. XA files must
contain 2336 bytes for every 2048-byte logical sector in their ISO extent.
Older extractions with truncated STR files fail qualification. The texture tool
does not repair inputs. Complete physical inputs do not establish complete
graphics support.

```sh
bin/harness media disc extract -i 'inputs/external/Breath of Fire III (USA).cue' -o out/original-us
build/tools/rust/bof3-textures/release/bof3-textures categories --json
build/tools/rust/bof3-textures/release/bof3-textures index out/original-us --output out/textures/index.json
build/tools/rust/bof3-textures/release/bof3-textures query out/textures/index.json --category unclassified-graphics --json
build/tools/rust/bof3-textures/release/bof3-textures unpack out/original-us/BIN/BATTLE/BATL_DRA.EMI --output out/textures/dragon-upload
build/tools/rust/bof3-textures/release/bof3-textures pack out/textures/dragon-upload --output out/textures/packed --json
build/tools/rust/bof3-textures/release/bof3-textures coverage out/original-us --strict --json
build/tools/rust/bof3-textures/release/bof3-textures verify out/original-us --json
```

Create the output parent first. Each output must be new and outside its input.
Exit 0 means the requested operation succeeded, exit 1 means required coverage or
verification remains incomplete, and exit 2 reports invalid input or an operation
failure. JSON errors are written to standard error. Query results and reports use
JSON on standard output, including when `--json` is omitted.

## Catalog

The index schema is `bof3.textures-index/v5`. A resource ID combines release,
original path, and EMI slot or file locator. Equal hashes retain distinct original
locations. Each physical resource has its byte offset, size, hash, kind, game categories,
storage, decode and pack status, evidence, dependencies, known uses, and findings.
The catalog retains unknown entries and non-EMI files for further disposition.

Query supports `--category`, `--source`, `--asset`, `--storage`, `--kind`,
`--palette`, `--animation`, `--dependency`, and `--evidence`. Source matching uses
substrings. Other filters match exact identifiers. Filters combine with AND.
Resources remain in original-file and slot order. Empty results do not prove
that a category is absent from the game.

Qualified content mappings recognize glyphs, portraits, interface graphics and
logo or raster-label images. Use the `font`, `portrait`, `interface` and
`system-screen` category filters to discover these resources. Content recognition
also covers identical original copies of the `FIRST.EMI` entry 4 portrait atlas
and the `SHISU.EMI` entry 2 atlas of windows, arrows and controls. Both the payload
hash and upload layout must match the qualified source. Each copy keeps its
original locator and its own dependency evidence.
Equal bytes do not transfer palette, consumer or loading associations between
archives. The resources retain `unclassified-graphics` because their complete
logical regions and uses remain unresolved. An image can contain rasterized text
even when it also has another graphics role.

Atlases with qualified consumer evidence name their physical palette sources in
`dependencies`. Content recognition alone leaves the palette unresolved. A
`partial` palette association records original consumer coordinates and source
ranges; it does not establish runtime loading or the selected palette at every call.
`FIRST.EMI` entry 8 covers only 16 of the 8x8 renderer's 64 possible CLUT selectors.
The `SHISU.EMI` portrait mapping names both physical palette banks. Twelve bounded
selection records describe 40x48 rectangles and their palette columns on rows
481 and 482. They do not prove the complete valid selection domain or cover the
atlas's other interface regions.
The palette's `uses` field lists the associated original atlas IDs. Other palette,
page, resident-base, and upload-order dependencies remain visible and unresolved.

Use `--dependency archive-request` or `--dependency palette-stage` to find
qualified palette loading paths. These edges cover `FIRST.EMI` entries 8, 9, 10
and 13, plus `BATE.EMI` and `SHISU.EMI` entries 3 and 4. `archive-request` names
the original requester in the EXE or GAME entry 0; `palette-stage` names the EXE
copy routine. An edge is unresolved if its consumer source is outside the input
and partial when that source is captured.

The FIRST bootstrap copies 32 rows of 256 words from `0x80033800` to
`0x80037800`, after clearing source rows 28 through 31. Its four qualified
palette entries occupy rows 0, 1, 2 and 26. The BATE and SHISU initializers copy
rows 1 and 2 separately after their ready branches. These paths establish static
address mappings, not exclusive RAM producers or subsequent GPU execution.
Other initialization calls, overwrites, active palette selection and runtime
residency remain unresolved.

Qualified bank destinations in the `PL`, `BPLD` and `BPLU` source families have a
`stream_request` record. It identifies the original EXE request family, index,
loader slot, disc LBA and three selector identifiers. Only the 19 qualified
members of each family are mapped. The original request function accepts a byte
index without enforcing these family extents. The identifiers do not establish
character names, ages, actions or game roles.

Use `--dependency stream-request` or `--dependency root-selection` to find these
records. The edges name the original EXE and GAME entry 0 consumer. They remain
unresolved when those files are outside the captured input and partial when the
consumers are captured. A request does not prove that loading finished or that a
particular root is active. The `resident-base` edge retains those limits. Some
archives declare both raw and compressed writes to `0x8003b800`; an address match
does not establish the draw-time contents.

Use `--dependency entry-order` to find qualified same-archive transfer ordering.
These edges link entry 7's type-1 bank to entry 3's 16,384 zero bytes at
`0x8003b800..0x8003f800`. On the normal, unskipped path without an error and with
mode bit `0x20` clear, entry 3 finishes transferring before entry 7 is selected.
The intervening types 6, 8 and 7 retain their audio owner. Their complete RAM
effects, later writes, palette selection and draw-time state remain unresolved.
An edge is partial when the original EXE is captured and unresolved otherwise.
The zero-filled entry remains preservation-only; its `uses` field names the bank.
This order does not establish a decoder dependency on zero history, completed
loading or active residency.

Query checks source content and inventory plus the compiled schema, qualification,
format, package, and mapping policy. It refuses a stale or altered catalog and
prints a rebuild instruction. Timestamps do not establish freshness. Rebuild the
executable after changing its interpretation sources.

## Edit packages

The manifest schema is `bof3.textures-package/v4`. The package includes complete
snapshots of the captured inputs, raw resources, its bound catalog, and coverage. A
qualified raw upload also has `words.json` and indexed `indices-4.png` and
`indices-8.png` views. These PNGs use diagnostic grayscale. They do not identify
a game palette, texture mode, UV layout, or assembled sprite.

The same upload exports `colors-16.png`, an editable RGBA representation of its
16-bit words. Its width equals the VRAM word width. RGB channels use exact
5-bit expansion, `(value << 3) | (value >> 2)`. Alpha encodes the source word:
0 means word zero, 128 means STP is set, and 255 means a nonzero word without
STP. STP black therefore remains distinct from zero. Alpha is reconstruction
metadata here, not a simulation of GPU blending. This view does not establish
that a game consumer selects 16-bit color rendering.

Keep this PNG in 8-bit RGBA format with its original dimensions. Packing refuses
RGB channels outside the exact expansion, other alpha values, nonblack pixels
with alpha 0, and black pixels with alpha 255. It never rounds colors. Edits to
`colors-16.png`, `words.json`, raw bytes, and index views must agree wherever
their changed bytes overlap. The tool reports conflicting views and publishes
nothing. Older packages must be unpacked again with the current executable.

Qualified FIRST glyph, SHISU portrait, and DEMO logo and label atlases also
export read-only `preview-N.png` files. Each manifest `previews` entry names
its original `palette_resource`, `bits`, and `first_word` within that palette.
These are whole-atlas views using conditional static palette associations.
They do not assemble sprites, prove the active selector, or establish runtime
loading or appearance. Other atlas regions can require different palettes.

Previews retain the original snapshot's colors even when source views are edited.
Keep them unchanged. Edit the corresponding `indices-4.png` or `indices-8.png`
and the palette resource's `words.json`; packing rejects changes to a preview.
Exact palette words remain the authority for STP and blending metadata.

A qualified palette bank has an editable `words.json` containing its exact
little-endian PSX color words. Preserve the word count. These banks have proven
source and staging addresses, but their draw CLUT selections and texture uses
remain unresolved. Editing a bank does not assign it to a diagnostic image.

Use `--category save-icon` or `--kind save-icon-source` to find the embedded
memory-card icon data in `SHOP.EMI` entry 8 and `START.EMI` entry 9. Their equal
bytes retain separate resource IDs. The original header producer copies a
16-word palette and three 16x16 frames into an `SC` save header. The catalog's
`save_icon` field records the physical offsets; `--palette` and `--animation`
accept this resource ID.

Each source exports `words.json` and `save-frame-0.png` through
`save-frame-2.png`. Edit the 16 unsigned 16-bit palette words or the frame indices
from 0 to 15. Keep PNG dimensions and palette metadata unchanged. PNGs preview
the original palette; packed color changes come from `words.json`. Packing
preserves producer code and the gaps between stored frames. Changes in `raw.bin`
must follow the same image-range restrictions and agree with changed views.

The source layout and archive edits are supported. Original save-header capture,
source loading, BIOS animation timing and edited icon display remain unresolved.
Memory-card import, export and save operations remain with the emulator's
memory-card owner.

A qualified outer compressed bank has a read-only `decoded.bin` view. Recognized
three-root banks export `sprite.json` with schema `bof3.sprite-bank/v1`, plus
`root-N/image-N.png` views for mode-1 and mode-2 records. Query them with
`--kind sprite-bank`. The catalog's `sprite_bank` field reports the record counts
and image modes. Add `--records` to query its logical images, palettes, programs
and cells by their own source-bound identities. PNG width is twice the record's VRAM word width; each pixel
preserves one decoded byte. Its grayscale palette is diagnostic.

Original `BIN/BPLCHAR/BPLU349.EMI#0` stores the same three-root layout as a raw
type-0 payload. Its storage is `us-raw-three-root-sprite-bank`, with `sprite_bank`
metadata and no outer `bank` metadata or `decoded.bin` view. Its JSON and PNG
editing rules are the same as those below. Other raw destinations remain
unresolved unless separately qualified.

In `sprite.json`, edit each palette record's 32 unsigned 16-bit `words`, existing
program step pairs, and existing cell parts. The step and part rules below also
apply here. Keep root order, section offsets, image descriptors, record extents,
and list lengths unchanged. Palette records do not establish a complete image
CLUT: decoded images can select indices above 31. Exact words preserve STP, but
the neighboring colors, draw CLUT, and runtime palette updates remain unresolved.

Edit PNG indices while preserving dimensions and the diagnostic palette.
Mode-1 records accept indices from 0 to 31; mode-2 records accept 0 to 255.
Changed aliases must agree on their complete palette, program, cell, or image
record. Conflicts name every changed view of the shared record. Unchanged aliases
do not override an edit. Image streams and any outer bank must fit their original
consumed byte spans. Raw banks retain their original payload size. Following pointers and
unconsumed bytes do not grant extra capacity. Packing preserves selection tails,
opaque flags, headers, pointer tables, codec metadata, and unrelated bytes.
The bank's `raw.bin` and any `decoded.bin` are read-only; use its qualified views.
Unsupported internal variants retain read-only decoded bytes and refusal findings.

A recognized resident animation bank has an editable `animation.json` document
with schema `bof3.resident-animation/v2`.
Each group retains its program pointers, byte extents, and existing step pairs.
Edit `duration` and `operand` values from 0 to 255. Keep the number of groups,
programs, and steps, and preserve all `offset` and `end` fields. Durations below
128 select cells. Durations of 128 or more select a forward step target. An edit
must preserve each step's frame or control role and keep every reference valid.
Programs shared by several table indices must receive agreeing edits. Unchanged
aliases do not override a changed view. Conflicts name every changed program view.

Some normal frame operands read a lookup word beyond the declared cell-offset
table and resolve to a zero-part cell inside that table. These metadata aliases
remain separate from declared offsets. Their programs' step pairs, headers,
declared offsets, lookup words and zero count bytes are read-only. Packing checks
these bytes against the original source after all cell, palette and image writes,
including edits submitted through a raw view. A part edit that overlaps a lookup
word is refused even when the changed lookup would remain within bounds.

The document's `cells` list contains one entry per payload offset. Keep its order,
`offset`, `end`, and part count. Each part contains a `size_code` from 0 to 15,
`horizontal_flip`, signed `x` and `y` anchors from -128 to 127, and unsigned `u`
and `v` from 0 to 255. Width is `8 * (1 + (size_code & 3))`; height is
`8 * (1 + (size_code >> 2))`. Keep `opaque_flags` unchanged. It preserves bits
4 through 6 of the original record. `horizontal_flip` stores bit 7, whose effect
also depends on the entity's flags and drawing path. V also depends on the
entity's offset. Editing these records does not resolve texture pages or CLUTs.
Packing preserves headers, pointer tables, cell counts, padding, and unrelated
bytes. It applies the same restrictions to changes in `raw.bin`.

Query these banks with `--kind resident-animation-bank`. `--animation` accepts
their resource ID. Image and palette bindings, reverse playback callers, game roles, and
runtime appearance remain unresolved. Unrecognized resident variants have no
editable animation document and retain their refusal findings in the catalog.

Edit raw upload words as unsigned 16-bit values, or preserve a PNG's dimensions
and palette order while editing its indices. A 4-bit view accepts indices 0 to
15; an 8-bit view accepts indices 0 to 255. Keep unused holes zero. Changed views
that request different values for the same byte cause an edit-conflict error.
The error names all changed views at each conflicting byte. Unchanged views do
not override edits. Changes to preservation-only resources, allocation, source
snapshots, catalog descriptors, or diagnostic palettes fail.

Packing reconstructs from the package's snapshots without opening the former
source paths. It validates all members and outputs in a private staging directory,
then publishes a new tree with no-replace rename. It checks the resulting entry
bytes and every unrelated archive byte. The pack report records these checks
separately from runtime observations. Packed edits currently require their bound
package for interpretation; treating an edited archive as an original US input
is rejected.

The `bof3.textures-pack-report/v2` report lists changed physical resources in
`files[].edited_resources`. Its `potential_impact` lists those resources and their
known dependents across the package, including logical records and their recorded
uses. Records whose bytes remain unchanged can appear here because they depend
on a changed containing entry. Identities and uses are sorted, and cycles are
visited once. Equal bytes in separate original sources retain separate identities.

Each affected resource retains its dependency targets, evidence statuses and
unresolved reasons. `impact_complete` is true only when every dependency in this
impact set has a verified target inside the package catalog. An unchanged package
has an empty impact set. These fields describe catalog relationships; `runtime`
continues to report execution evidence separately. The package manifest contains
the graph before editing in `index.resources` and `index.records`, through their
`dependencies` and `uses` fields.

## Format and evidence limits

Verification uses `bof3.textures-verification/v2` and reports structure,
dependencies, decoding, unchanged reconstruction, edited reconstruction,
unrelated bytes, and runtime evidence separately. On original inputs it exercises
the supported word, index, palette, bank, sprite, resident-record and save-icon
inverses, then compares complete reconstructed files with the originals.
Unknown resources remain preserved bytes with unsupported inverse status.

To check packed outputs independently, retain the edit package and run:

```sh
build/tools/rust/bof3-textures/release/bof3-textures verify out/textures/packed --package out/textures/dragon-upload --json
```

The verifier reconstructs intended output from the package without publishing
files. It reads the actual output tree, decodes its supported views, compares
them with the intended edits, and checks every byte outside the intended change
positions. Header fields, archive padding and gaps inside edited resources are
included. The check refuses symlink traversal in output members. Missing files,
malformed archives, changed allocations and mismatched views produce failed
properties in the JSON report.

`byte_checks_passed` describes only these byte and source-view checks. For example,
an intact edited icon with damaged neighboring code can pass the edited-view
comparison while failing unrelated-byte preservation. The report retains file
and resource hashes and failure details. Neither byte equality nor decoded views
establish full category, palette-loading or runtime acceptance. Verification
returns exit 1 while full acceptance remains incomplete, even when its byte
checks pass. Invalid input or an invalid edit package returns exit 2.
Use `coverage --strict` to expose unresolved logical sources and dependencies;
its nonzero result is expected while those findings remain.

Raw upload placement uses `SLUS_004.22@0x80162698` and the existing EMI image
owner. Each chunk contains 32 by 32 little-endian VRAM words. Descriptor masks
are `0x3f` for X, `0x1f` for Y, and `0x3f` for row span, with 32-word coordinates.
The consumer at `SLUS_004.22@0x80163378` submits a chunk only when its whole
rectangle fits within 1024 by 512 VRAM words. Catalog `skipped_chunks` lists the
zero-based chunks it does not submit. In `BIN/ETC/FIRST.EMI` entry 6, only chunks
0 and 1 are submitted; chunks 2 through 15 remain in the package and round trip
exactly. Word and PNG views retain the complete logical source grid, including
skipped chunks. Skipped chunks do not wrap to the top of VRAM.

The library preserves PSX palette words, including duplicate colors and STP.
Only color word zero is transparent in the diagnostic color conversion. STP
black remains distinct. Blending also depends on consumer draw state and the
background. A PNG cannot replace exact palette-word metadata.

Palette banks are type-0 spans wholly inside `0x80033800..0x80037000`, with
32-byte alignment and length. The original entry loader at `0x80162BAC` and
sector path at `0x801625E4` bind the source bytes to RAM. The staging routine at
`0x8014E284` copies this buffer for the upload at `0x8014E22C`. All addresses are
in `SLUS_004.22`. The staging routine clears the following four rows, so those
rows are excluded. A bank can contain several palettes; word preservation does
not establish CLUT boundaries, upload timing, or the appearance of an edited asset.

The library's `packed` module implements the mode-1 byte expansion consumed at
`SLUS_004.22@0x8014E7F8`. Six five-bit values occupy bit positions 0, 5, 10, 16,
21, and 26 in each little-endian word. Encoding preserves bits 15 and 31 and
unused tail values. The three-root owner below binds these records to qualified bank sections.

The `lzss` module implements the mode-2 byte stream consumed at
`SLUS_004.22@0x8014E8FC`. It retains the four-byte opaque prefix and uses
LSB-first flags with a 512-byte dictionary. The initial cursor is `0x1ee`;
only dictionary offsets below it are initialized to zero. Reading an unwritten
offset fails. Final-reference bytes beyond the declared image length are
reported separately and preserved during encoding. Unchanged streams remain
exact, and edited streams must fit the original allocation. The three-root owner supplies the source extents used by editable packages.

The `bank` module implements the outer stream consumed at
`SLUS_004.22@0x80164064`, with forward overlapping copies at `0x801642F4`.
Qualified type-1 entries loaded at `0x8003b800` have a six-byte header and an
`0xff` terminator. Decoded size must match the first header word and cannot
exceed 1 MiB. The decoder rejects references before the output start. The library
inverse preserves unchanged streams exactly, retains opaque header and trailing
bytes, and requires edited output to fit the original consumed span. This byte
inverse does not validate logical sprite edits. Packages validate their record
edits through the three-root owner before encoding the outer bank.
The qualified raw `BPLU349.EMI#0` payload bypasses this outer codec. Original
type-0 dispatch at `SLUS_004.22@0x801622A8` selects `0x801625E4`, which copies
sectors into the entry's destination through `0x80162C14`. Packing edits its
three-root records directly and preserves bytes outside the edited spans.

The `bank_image` library reads the eight-byte records consumed at
`SLUS_004.22@0x8014e6d0`. The header stores a 16-bit width in VRAM words,
a 16-bit height, and a 32-bit mode. Mode 1 uses `packed`, mode 2 uses `lzss`,
and other modes submit raw words. This mode does not identify GPU bit depth.
Dimensions must fit 1024 words by 512 rows, within a 1 MiB decoded bank.
Callers supply qualified record extents. Encoding preserves dimensions and mode,
the surrounding bank, and codec metadata. Edited streams must fit a separately
qualified allocation. Without proven reusable tail bytes, callers must bound
encoding at the original consumed length. A following image or animation pointer
does not establish free space. The library preserves mode-2 final-token overshoot
separately from image bytes. The three-root owner supplies these extents for package edits. Draw CLUT
bindings and runtime appearance remain unresolved.

The `sprite_bank` owner recognizes the three-root table selected by
`BIN/ETC/GAME.EMI#0@0x801c1254`. Each root contains ordered image, palette,
program, and selection sections. The first relative offset delimits each table;
image and palette tables use 32-bit offsets, and program tables use 16-bit
offsets. Each table has at most 256 indices. The palette consumer at
`SLUS_004.22@0x80166c48` copies 32 words per selected record. The selection
consumer at `0x8014dc08` can skip an image upload according to entity state.
Selection tails stay opaque: contained triples do not establish valid runtime
indices or editable records. Recognizing these sections does not assign a bank
to a field or battle category.

Input files are bounded at 128 MiB, indexed images at 1,048,576 pixels, and each
codec process at 20 seconds. Invalid extents, malformed images, changed snapshots,
escaped package paths, missing members, and existing destinations cause failure.

The [composition library](docs/composition.md) computes qualified rectangle and
quad geometry from an explicit entity and graph type. It preserves the original
flip dispatch, scale rounding, UV wrapping, texture-page correction and encoded
CLUT fields. It does not infer source residency or render an assembled frame.

The [selection library](docs/selection.md) resolves one explicit root selector
through the original consumer. It preserves the image-skip path, independent
program selection and raw mirror byte. It does not infer a valid selector range
or make the surrounding tail editable.

The `animation` library reads bounded program headers, step pairs, and cell
offsets using the US consumers at `0x8014d910`, `0x8014d978`, and `0x8014daec`.
It preserves cell payloads and allocation while re-encoding existing step pairs.
Forward transitions branch once on a duration byte at or above `0x80`, then fetch
the target directly. Initialization and reverse playback differ. The library
models the two forward variants and reverse loop in consumer updates, without
assuming a frame rate. `PlaybackMode` selects `ForwardLoop`, `ForwardStop`, or
`ReverseLoop`. Switching modes retains the literal byte cursor returned by
`Playback::cursor()`.

The reverse consumer at `0x8014da24` decrements the byte countdown, wraps a zero
cursor to the step-table end, and directly fetches the preceding token. High
durations remain durations in this path. Switching from forward initialization
to reverse first fetches the current token again. A zero duration lasts 256
consumer updates. Missing cell references cause an error, including high-token
operands that are valid forward targets but exceed the cell table in reverse.
Caller reachability, game direction mapping, and rendered composition remain
unresolved. The three-root owner binds forward programs to compressed banks.

The `resident` module binds the recognized type-0 `0x800d3800` table variant to
the original base initialization at `0x8014ebb8` and program selection at
`0x8014d878`. A 32-bit relative group table leads to 16-bit relative program
tables. Each table's first offset delimits its entries. References may be
repeated or out of order. Each program and its cells must fit before the
next distinct program address, within its group and payload. The parser bounds
sources at 1 MiB, group indices and total program indices at 65,536, and each
group's program indices at 256. A normal frame operand beyond the declared cell
count is supported only when its lookup word fits inside the same program and
selects a zero count byte inside the declared offset table. Parsed programs expose
these source-bound lookups separately as `metadata_cells`; they do not increase
the declared cell count. Other metadata references remain unsupported. The setup
call at `0x8014c6fc` precedes the zero-part branch, so a zero-part cell does not
establish an invisible frame or absence of GPU effects. Cell records follow the
count byte at `0x8014c6f8` and the
five-byte stride at `0x8014c700`. Their size codes use the original dimension
table at `0x8017f53c`. Records must fit before the next distinct cell address or
the program end. Only existing step pairs and qualified cell records are
editable. A valid program or an edited record does not establish its use or
appearance.
Unresolved logical resources, categories, dependencies, inverse formats, and
runtime families keep full-support acceptance open.


## Logical bank records

`query INDEX --records` queries individual records in validated three-root and
resident banks, plus qualified save-icon frames and palettes.
The default query reports physical files and EMI entries. Both scopes support the
same filters. [Catalog records](docs/catalog.md) explains identities, byte
coordinates, structural references and coverage units.
