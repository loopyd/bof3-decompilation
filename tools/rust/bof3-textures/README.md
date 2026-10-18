# BOF3 textures

`bof3-textures` catalogs original US PlayStation graphics sources and exports
self-contained archive packages. It can edit raw VRAM upload words, their
4-bit or 8-bit index interpretations, and qualified palette-word banks within
their original allocations.
It also edits qualified resident animation step pairs and five-byte cell records.
Recognized three-root sprite banks have editable index planes, palette words,
programs, and cells, with both compression layers bounded by their original spans.

Full graphics support is incomplete. Category discovery lists the intended game
roles. Most resource roles, CLUT associations, sprite composition,
reverse playback callers, unsupported bank variants, and runtime appearance remain unresolved.
Strict coverage and complete verification return exit 1. A successful raw upload
edit does not establish its appearance in the game.

## Build

Use Rust 1.98.1 with edition 2024. The crate declares Rust 1.98 as its minimum.
The lockfile uses dependencies already approved in this repository. Build from
the repository root with the installed toolchain and cached dependencies:

```sh
cargo build --offline --locked --release --manifest-path tools/rust/bof3-textures/Cargo.toml --target-dir build/tools/rust/bof3-textures
build/tools/rust/bof3-textures/release/bof3-textures --help
```

PNG interchange requires installed FFmpeg with the `png` codec and `pal8` pixel
format. Publication requires GNU coreutils `mv` with `--no-copy` and
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

The index schema is `bof3.textures-index/v2`. A resource ID combines release,
original path, and EMI slot or file locator. Equal hashes retain distinct original
locations. Each record has its byte offset, size, hash, kind, game categories,
storage, decode and pack status, evidence, dependencies, known uses, and findings.
The catalog retains unknown entries and non-EMI files for further disposition.

Query supports `--category`, `--source`, `--asset`, `--storage`, `--kind`,
`--palette`, `--animation`, `--dependency`, and `--evidence`. Source matching uses
substrings. Other filters match exact identifiers. Filters combine with AND.
Resources remain in original-file and slot order. Empty results do not prove
that a category is absent from the game.

Qualified content mappings recognize glyphs, portraits, interface graphics and
logo or raster-label images in `FIRST.EMI`, `ENDKANJI.EMI`, `SHISU.EMI` and
`DEMO.EMI`. Use the `font`, `portrait`, `interface` and `system-screen` category
filters to discover these resources. Equal font, portrait and logo bytes keep
their separate original locators. They retain `unclassified-graphics` because
their complete logical regions and uses remain unresolved. An image can contain
rasterized text even when it also has another graphics role.

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

Query checks source content and inventory plus the compiled schema, qualification,
format, package, and mapping policy. It refuses a stale or altered catalog and
prints a rebuild instruction. Timestamps do not establish freshness. Rebuild the
executable after changing its interpretation sources.

## Edit packages

The manifest schema is `bof3.textures-package/v2`. The package includes complete
snapshots of the captured inputs, raw resources, its bound catalog, and coverage. A
qualified raw upload also has `words.json` and indexed `indices-4.png` and
`indices-8.png` views. These PNGs use diagnostic grayscale. They do not identify
a game palette, texture mode, UV layout, or assembled sprite.

A qualified palette bank has an editable `words.json` containing its exact
little-endian PSX color words. Preserve the word count. These banks have proven
source and staging addresses, but their draw CLUT selections and texture uses
remain unresolved. Editing a bank does not assign it to a diagnostic image.

A qualified outer compressed bank has a read-only `decoded.bin` view. Recognized
three-root banks also export `sprite.json` with schema `bof3.sprite-bank/v1`, plus
`root-N/image-N.png` views for mode-1 and mode-2 records. Query them with
`--kind sprite-bank`. The catalog's `sprite_bank` field reports the record counts
and image modes. PNG width is twice the record's VRAM word width; each pixel
preserves one decoded byte. Its grayscale palette is diagnostic.

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
do not override an edit. Both image streams and the outer bank must fit their
original consumed byte spans. Following pointers and
unconsumed bytes do not grant extra capacity. Packing preserves selection tails,
opaque flags, headers, pointer tables, codec metadata, and unrelated bytes.
The bank's `raw.bin` and `decoded.bin` are read-only; use its qualified views.
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

## Format and evidence limits

Verification reports structure, dependencies, decoding, unchanged reconstruction,
edited reconstruction, unrelated bytes, and runtime evidence separately. The
input verifier currently establishes captured structure, qualified raw-word
decoding, palette-word inverses, and outer-bank decompression. The pack operation
checks reconstruction and unrelated bytes for its own outputs. Neither report
establishes full category or runtime acceptance.
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
group's program indices at 256. Variants whose cell references select metadata
remain unsupported. Cell records follow the count byte at `0x8014c6f8` and the
five-byte stride at `0x8014c700`. Their size codes use the original dimension
table at `0x8017f53c`. Records must fit before the next distinct cell address or
the program end. Only existing step pairs and qualified cell records are
editable. A valid program or an edited record does not establish its use or
appearance.
Unresolved logical resources, categories, dependencies, inverse formats, and
runtime families keep full-support acceptance open.
